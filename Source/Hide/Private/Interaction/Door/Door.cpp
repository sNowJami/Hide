// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Door/Door.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Curves/CurveVector.h"


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

    // 【核心修复】完全剥夺玩家 Character 的控制权，而不仅仅是 DisableInput
    ACharacter* PlayerChar = Cast<ACharacter>(CachedPC->GetPawn());
    if (PlayerChar)
    {
        // 彻底无视键盘移动输入和鼠标视角输入
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
    // 如果处于活跃事件（正在播惊吓动画），绝对不理会输入
    if (bIsEventActive) return;

    // 容错：如果事件刚播完，状态还没来得及变回来，这里自动帮它变回来
    if (CurrentState == EDoorState::EventTriggered && !bIsEventActive)
    {
        CurrentState = EDoorState::Pushing;
    }

    // 正常推门拦截
    if (CurrentState != EDoorState::Pushing || AxisValue <= 0.0f) return;

    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentProgress += DeltaTime * PushSpeed * AxisValue;

    if (CurrentProgress > 60.0f) CurrentProgress = 60.0f;

    UpdateDoorVisuals();

    // 实时监测随机事件触发点
    if (!bHasEventTriggered && CurrentProgress >= TriggerValue)
    {
        TriggerRandomEvent();
        // 注意：如果 TriggerRandomEvent 触发了真正的事件，它会把 CurrentState 变成 EventTriggered
        // 如果触发的是空事件，它会直接返回，下面的 60 门槛判定就能在同一帧无缝执行！
    }

    // 达到 60 阈值，直接无缝进入自动全开阶段
    if (CurrentProgress >= 60.0f)
    {
        StartAutoComplete();
    }
}

void ADoor::TryPullBack()
{
    // 只有在 Pushing 状态且没有处于活跃事件中，才接受 S 键输入
    if (CurrentState != EDoorState::Pushing || bIsEventActive) return;

    // 只有在进度完全为 0 或极小时允许退出交互
    if (FMath::IsNearlyZero(CurrentProgress))
    {
        CurrentState = EDoorState::Normal;

        if (CachedPC)
        {
            // 1. 获取玩家 Character
            ACharacter* PlayerChar = Cast<ACharacter>(CachedPC->GetPawn());
            if (PlayerChar)
            {
                // 2. 【核心】恢复玩家角色的全部键盘、鼠标、移动控制
                PlayerChar->EnableInput(CachedPC);

                // 3. 将相机平滑切回玩家角色视角
                CachedPC->SetViewTargetWithBlend(PlayerChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
            }

            // 4. 【核心】门自己立刻交出控制权，不再拦截任何后续按键
            this->DisableInput(CachedPC);
        }
    }
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

        // 2. 相机轨迹：根据当前的进度比例 (0.0 - 1.0)，从自定义曲线中采样
        if (CameraMovementCurve)
        {
            float TimePosition = CurrentProgress / 60.0f; // 映射到 0~1 的时间轴
            FVector CurveOffset = CameraMovementCurve->GetVectorValue(TimePosition);


            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow,
                    FString::Printf(TEXT("Progress: %.2f | Time: %.2f | Offset: %s"), CurrentProgress, TimePosition, *CurveOffset.ToString()));
            }


            // 相机相对位置 = 初始位置 + 曲线定义的偏移量
            DoorCamera->SetRelativeLocation(InitialCameraLocation + CurveOffset);
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