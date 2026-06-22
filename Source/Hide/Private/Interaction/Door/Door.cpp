 // Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Door/Door.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values 
ADoor::ADoor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;


    // 初始化组件
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(RootComp);

    DoorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DoorCamera"));
    DoorCamera->SetupAttachment(RootComp);

    // 初始化状态
    CurrentState = EDoorState::Normal;
    CurrentProgress = 0.0f;
    bHasEventTriggered = false;
    bIsEventActive = false;
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
    Super::BeginPlay();


    // 配置自动化 Timeline
    if (OpenCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleTimelineProgress"));
        AutoCompleteTimeline.AddInterpFloat(OpenCurve, ProgressFunction);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
        AutoCompleteTimeline.SetTimelineFinishedFunc(FinishedFunction);
    }

}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 每帧递增 Timeline（仅在 AutoComplete 状态下有效）
    if (CurrentState == EDoorState::AutoComplete)
    {
        AutoCompleteTimeline.TickTimeline(DeltaTime);
    }

}

void ADoor::TryInteract(APlayerController* PC)
{
    if (!PC || CurrentState != EDoorState::Normal) return;
    CachedPC = PC;

    // 检查全局静态变量（示例采用模拟函数，实际可以访问你的GameInstance）
    if (GetGlobalSkipPushStage()) { StartAutoComplete(); return; }
    // 切换到推门状态
    if (DoorCamera)
    {
        DoorCamera->Activate(true);
        DoorCamera->bUsePawnControlRotation = false;

        InitialCameraLocation = DoorCamera->GetRelativeLocation();
    }

    CurrentState = EDoorState::Pushing;

    bHasEventTriggered = false;
    bIsEventActive = false;
    CurrentProgress = 0.0f;
    // 随机生成 30-50 之间的触发点
    TriggerValue = FMath::FRandRange(30.0f, 50.0f);


    ACharacter* PlayerChar = Cast<ACharacter>(CachedPC->GetPawn());
    if (PlayerChar)
    {
        // 1. 强行清空角色的速度（瞬间刹车），防止脚步声和惯性动画被卡住
        PlayerChar->GetCharacterMovement()->Velocity = FVector::ZeroVector;  //后续发现SoundBug是因为蓝图的逻辑导致，  如后续出现新Bug 删掉这一条

        // 2. 强行停止所有移动组件的正在进行的加速度计算
        PlayerChar->GetCharacterMovement()->StopMovementImmediately();   // 以上

        // 3. 【安全清空】在这两步做完、速度彻底归零之后，再切断玩家输入   //以上
        PlayerChar->DisableInput(CachedPC);
    }
    // 让门本身开始允许接收输入（这样门才能在玩家断开后，单独接收 W/S 键）
    this->EnableInput(CachedPC);


    // 锁定玩家常规输入并平滑切视角
    CachedPC->DisableInput(CachedPC);
    CachedPC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
}

void ADoor::PushDoor(float AxisValue)
{
    // 1. 如果处于活跃事件（正在播惊吓动画），绝对不理会任何输入（包括S键）
    if (bIsEventActive) return;

    // 2. 容错：如果事件刚播完，状态还没变回来，自动帮它变回来
    if (CurrentState == EDoorState::EventTriggered && !bIsEventActive)
    {
        CurrentState = EDoorState::Pushing;
    }

    // 3. 拦截：如果不是 Pushing 状态，或者没有任何输入，就不处理
    if (CurrentState != EDoorState::Pushing || AxisValue == 0.0f) return;

    // 获取帧时间
    float DeltaTime = GetWorld()->GetDeltaSeconds();

    //// 【防瞬移核心保护】
    //if (DeltaTime > 0.05f) DeltaTime = 0.016f;    //后续发现是CameraFov问题  如后续有问题则去掉注释

    // 计算这一帧的真实理论增量（AxisValue 为正时往前推，为负时往回拉）
    float ProgressIncrement = DeltaTime * PushSpeed * AxisValue;

  
    // 强制卡死：任何一帧，门板进度的单帧变化绝对不能超过 1.5，也绝不能低于 -1.5，彻底解决 FOV 或时间断层引发的闪现
    ProgressIncrement = FMath::Clamp(ProgressIncrement, -1.5f, 1.5f);


    // ───A：处理往回拉（S键，即 AxisValue < 0） ───
    if (AxisValue < 0.0f)
    {
        // 只要在触发惊吓事件前（TriggerValue - 1.0f 之前），都允许玩家按 S 把门往回拉
        if (CurrentProgress < (TriggerValue - 1.0f))
        {
            // 累加负增量（因为 ProgressIncrement 此时是负数，所以这里实际上是在逐步扣减 CurrentProgress）
            CurrentProgress += ProgressIncrement;

            // 如果玩家一路把门拉回到了原点（进度归零或变负）
            if (CurrentProgress <= 0.0f)
            {
                CurrentProgress = 0.0f;
                UpdateDoorVisuals(); // 确保门板视觉、FOV 刷回 0 度的完全关闭状态

                UE_LOG(LogTemp, Log, TEXT("Door: Player pulled the door back to 0. Exiting interaction."));

                // 门完全拉回闭合位置，平滑退出交互，交还控制权给玩家
                ExitInteraction();
                return;
            }
        }
        else
        {
            // 如果进度已经超过了 (TriggerValue - 1.0f)，说明门缝推得太大，已经进入了无法挽回的惊吓临界区
            // 此时按 S 键将没有任何效果，门无法被拉回
            return;
        }
    }
    // ───正常按 W 往前推门（AxisValue > 0） ───
    else
    {
        // 累加正增量
        CurrentProgress += ProgressIncrement;

        // 限制最大推门进度到 60
        if (CurrentProgress > 60.0f) CurrentProgress = 60.0f;

        // 实时监测随机事件触发点
        if (!bHasEventTriggered && CurrentProgress >= TriggerValue)
        {
            TriggerRandomEvent();
            return; // 触发事件后直接返回，防止同一帧继续往下走
        }

        // 达到 60 阈值，直接无缝进入自动全开阶段
        if (CurrentProgress >= 60.0f)
        {
            StartAutoComplete();
            return;
        }
    }

    // 无论前进还是后退，只要进度变了，就实时刷新门的旋转和相机 FOV
    UpdateDoorVisuals();
}



// ─────────────────────
// 表现层更新：基于进度控制门旋转和相机微调
// ─────────────────────



void ADoor::UpdateDoorVisuals()
{
    if (CurrentProgress <= 60.0f)
    {
        // 1. 门的旋转保持不变
        float CurrentAngle = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 60.0f), FVector2D(0.0f, MaxPushAngle), CurrentProgress);
        DoorMesh->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));

        // 2. 替代方案：不调位置，直接调视野 FOV 
        if (DoorCamera)
        {
            float Alpha = CurrentProgress / 60.0f; // 0.0 ~ 1.0

            // 默认 FOV 是 90（大视野），随着推门，平滑缩到 60（极度聚焦门缝）
            // 你也可以在这里采样你的曲线：float CurveVal = CameraMovementCurve->GetVectorValue(Alpha).X; 然后用 CurveVal 来当 FOV
            float TargetFOV = FMath::Lerp(90.0f, 60.0f, Alpha);

            DoorCamera->SetFieldOfView(TargetFOV);
        }
    }
}

void ADoor::TriggerRandomEvent()
{
    bHasEventTriggered = true; // 确保这一趟交互只触发一次事件

    // 随机 1~3
    int32 EventIndex = FMath::RandRange(1, 3);

    if (EventIndex == 3)
    {
        // 1. 如果是空事件：什么都不发生，让玩家继续无缝推门（保持你之前的流畅体验）
        UE_LOG(LogTemp, Log, TEXT("Door: Empty Event skipped. Continuity preserved."));
        return;
    }

    // 2. 如果随机到了 1 或 2（真正的惊吓事件）：
    // 改变状态为事件激活，防止玩家此时再通过键盘控制门
    CurrentState = EDoorState::EventTriggered;
    bIsEventActive = true;

    // 触发全局静态变量（以后别的门直接略过推门，本门不需要，但留作机制备份）
    SetGlobalSkipPushStage(true);

    // 3.发出蓝图事件通知，让美术（音效、震屏、鬼影）在这一帧炸裂开来
    if (EventIndex == 1)
    {
        OnPlayScareA(); // 对应蓝图里的惊吓 A（比如一声巨响）
    }
    else if (EventIndex == 2)
    {
        OnPlayScareB(); // 对应蓝图里的惊吓 B（比如女鬼闪过）
    }

    // 4.不需要用 Timer 等了  直接把门踢进第四阶段（强制自动全开）
    // 这个函数会启动你之前写好的门的 Timeline，让门自动弹开，并把相机 Lerp 回复位位置
    StartAutoComplete();
}

void ADoor::ExitInteraction()
{
    // 1. 状态变回 Normal，代表现在没人跟这扇门交互了
    CurrentState = EDoorState::Normal;

    if (CachedPC)
    {
        ACharacter* PlayerChar = Cast<ACharacter>(CachedPC->GetPawn());
        if (PlayerChar)
        {
            // 2. 【核心】恢复玩家角色的全部键盘、鼠标、移动控制
            PlayerChar->EnableInput(CachedPC);

            // 3. 将相机平滑切回玩家角色视角
            CachedPC->SetViewTargetWithBlend(PlayerChar, 0.3f, EViewTargetBlendFunction::VTBlend_Cubic);
        }

        // 4. 【核心】门自己立刻交出控制权，不再拦截任何后续按键
        this->DisableInput(CachedPC);
    }
}

void ADoor::EndCurrentEvent()
{
    bIsEventActive = false;
    // 事件结束后，退回到 Pushing 状态，允许玩家重新获得 W 键控制权
    CurrentState = EDoorState::Pushing;
}

void ADoor::StartAutoComplete()
{
    CurrentState = EDoorState::AutoComplete;

    if (OpenCurve)
    {
        AutoCompleteTimeline.PlayFromStart();
    }
    else
    {
        // 容错处理：如果没有配置 Curve，直接瞬间完成
        OnTimelineFinished();
    }
}




void ADoor::HandleTimelineProgress(float Value)
{
    // 门旋转：从当前推到的角度，平滑插值到全开 TargetOpenAngle (如 90°)
    float CurrentAngle = DoorMesh->GetRelativeRotation().Yaw;
    float NewAngle = FMath::Lerp(CurrentAngle, TargetOpenAngle, Value);
    DoorMesh->SetRelativeRotation(FRotator(0.0f, NewAngle, 0.0f));

    // 相机复位：从当前微调过后的位置，平滑 Lerp 回初始相对位置
    FVector CurrentCamLoc = DoorCamera->GetRelativeLocation();
    FVector NewCamLoc = FMath::Lerp(CurrentCamLoc, InitialCameraLocation, Value);
    DoorCamera->SetRelativeLocation(NewCamLoc);
}

void ADoor::OnTimelineFinished()
{
    // 将状态标记为完全打开，交互彻底结束
    CurrentState = EDoorState::AutoComplete;
    bIsEventActive = false;

    if (CachedPC)
    {
        // 1. 获取玩家的 Character
        ACharacter* PlayerChar = Cast<ACharacter>(CachedPC->GetPawn());
        if (PlayerChar)
        {
            // 2. 核心：完全恢复玩家角色的全部键盘移动和鼠标视角输入
            PlayerChar->EnableInput(CachedPC);

            // 3. 核心：将摄像机从门的 DoorCamera 平滑地混成（Blend）切回玩家的角色视角
            // 这里的 0.4f 是切回来的时间，你可以改成 0.2f 让它更快、更惊悚地拉回视角
            CachedPC->SetViewTargetWithBlend(PlayerChar, 0.4f, EViewTargetBlendFunction::VTBlend_Cubic);
        }

        // 4. 核心：门自己立刻全面交出（Disable）输入权，以后玩家按 W/S 门都绝对不再响应
        this->DisableInput(CachedPC);
    }

    // 停止 Tick，省下性能
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Log, TEXT("Door: Auto Open Complete. Control returned to Character."));
}


bool ADoor::GetGlobalSkipPushStage() const
{
    // 这里仅做代码示意。实际开发中应该：
     // UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
     // return GI ? GI->bGlobalSkipPushStage : false;
    return false;
}
void ADoor::SetGlobalSkipPushStage(bool bSkip)
{
    // UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    // if(GI) GI->bGlobalSkipPushStage = bSkip;
}