#include "AI/AIController/AGhostAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"



AAGhostAIController::AAGhostAIController()
{
    // 强制此 Actor 不要 Tick，全权交由行为树 Tick，节约大体量 AI 运行时的 CPU 开销
    PrimaryActorTick.bCanEverTick = false;

    // 1. 实例化行为树与黑板组件
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // 2. 实例化感知组件
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // 3. 详细配置【视觉系统】
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.f;                  // 视距：15米内能直接看到玩家
    SightConfig->LoseSightRadius = 2000.f;              // 丢视距：超过20米彻底跟丢
    SightConfig->PeripheralVisionAngleDegrees = 60.f;   // 视角的半角（即左右各60度，总计120度扇形区域）
    SightConfig->SetMaxAge(5.f);                        // 感知记忆寿命：感知消失5秒后自动清除标记

    // 必须勾选以下三项，否则 AI 无法识别世界中的任何阵营实体
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 将视觉配置应用到感知组件
    PerceptionComp->ConfigureSense(*SightConfig);

    // 4. 详细配置【听觉系统】
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.f; // 直接赋值              // 听力范围：12米内能听到声音（无视墙壁阻挡）
    HearingConfig->SetMaxAge(3.f);                      // 声音记忆寿命

    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 将听觉配置应用到感知组件
    PerceptionComp->ConfigureSense(*HearingConfig);

    // 5. 设置主导感知：当视觉和听觉冲突时，优先信任视觉
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AAGhostAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 确保资产有效
    if (GhostBehaviorTree && GhostBehaviorTree->BlackboardAsset)
    {
        // 1. 初始化并运行黑板
        BlackboardComp->InitializeBlackboard(*GhostBehaviorTree->BlackboardAsset);

        // 2. 预设初始状态为 Idle（闲置状态）
        BlackboardComp->SetValueAsEnum(BBKeys::CurrentState, static_cast<uint8>(EGhostState::Idle));
        BlackboardComp->SetValueAsBool(BBKeys::IsAttackCooldown, false);

        // 3. 启动行为树
        RunBehaviorTree(GhostBehaviorTree);
    }

    // 4. 动态绑定感知刷新事件
    if (PerceptionComp)
    {
        PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AAGhostAIController::HandlePerceptionUpdated);
    }
}

void AAGhostAIController::OnUnPossess()
{
    // 解除附身时及时解绑，防止内存泄漏和野指针调用
    if (PerceptionComp)
    {
        PerceptionComp->OnPerceptionUpdated.RemoveDynamic(this, &AAGhostAIController::HandlePerceptionUpdated);
    }

    Super::OnUnPossess();
}

void AAGhostAIController::HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    if (!BlackboardComp) return;

    for (AActor* Actor : UpdatedActors)
    {
        // 恐怖游戏核心安全检查：只对拥有 "Player" 标签的玩家角色做出反应
        if (!Actor || !Actor->ActorHasTag(TEXT("Player"))) continue;

        FActorPerceptionBlueprintInfo PerceptionInfo;
        PerceptionComp->GetActorsPerception(Actor, PerceptionInfo);

        // 遍历所有激发的感官刺激（可能是刚看到，也可能是刚听到）
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            // -----------------------------------------------------------------
            // A. 处理【视觉】刺激
            // -----------------------------------------------------------------
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    // 核心逻辑：【看见玩家】
                    BlackboardComp->SetValueAsObject(BBKeys::TargetActor, Actor);
                    SetGhostState(EGhostState::Chasing); // 直接切入癫狂追逐状态
                }
                else
                {
                    // 核心逻辑：【跟丢玩家/玩家躲入视线死角】
                    BlackboardComp->SetValueAsObject(BBKeys::TargetActor, nullptr);

                    // 记录最后已知玩家在视线里消失的位置
                    BlackboardComp->SetValueAsVector(BBKeys::LastKnownPlayerLocation, Stimulus.StimulusLocation);

                    // 状态保护：只有当前正在追逐时跟丢了，才转入搜寻状态
                    uint8 CurrentState = BlackboardComp->GetValueAsEnum(BBKeys::CurrentState);
                    if (CurrentState == static_cast<uint8>(EGhostState::Chasing))
                    {
                        SetGhostState(EGhostState::Searching);
                    }
                }
            }
            // -----------------------------------------------------------------
            // B. 处理【听觉】刺激
            // -----------------------------------------------------------------
            else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
            {
                // 如果女鬼听到了有效声响（玩家跑步、开门、弄出噪音）
                if (Stimulus.WasSuccessfullySensed())
                {
                    uint8 CurrentState = BlackboardComp->GetValueAsEnum(BBKeys::CurrentState);

                    // 听觉决策分支：如果女鬼在闲置(Idle)或已经在搜寻(Searching)，听到声音会跑去查看
                    // 注意：如果女鬼正在睁眼瞎追逐(Chasing)，则无视外界杂音，专心追逐
                    if (CurrentState == static_cast<uint8>(EGhostState::Idle) || CurrentState == static_cast<uint8>(EGhostState::Searching))
                    {
                        // 写入黑板，标记噪音来源点
                        BlackboardComp->SetValueAsVector(BBKeys::LastNoiseLocation, Stimulus.StimulusLocation);
                        SetGhostState(EGhostState::Searching);
                    }
                }
            }
        }
    }
}

void AAGhostAIController::ForceTriggerEncounter(AActor* PlayerActor, EGhostState InitialState, bool bPlayJumpScare)
{
    if (!BlackboardComp || !PlayerActor) return;

    // 1. 强行锁定目标玩家
    BlackboardComp->SetValueAsObject(BBKeys::TargetActor, PlayerActor);

    // 2. 强行修改黑板决策状态
    SetGhostState(InitialState);

    // 3. 如果是突发开门杀，强制执行物理表现
    if (bPlayJumpScare)
    {
        ACharacter* GhostChar = Cast<ACharacter>(GetPawn());
        if (GhostChar)
        {
            // 通过获取女鬼身上之前挂载的动画资产直接进行播放
            // 实际开发中，可以在此处触发全屏后处理滤镜、响亮音效等
            SetGhostState(EGhostState::Attacking);
        }
    }
}

void AAGhostAIController::SetGhostState(EGhostState NewState)
{
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(BBKeys::CurrentState, static_cast<uint8>(NewState));
    }
}

FVector AAGhostAIController::CalculateRetreatLocation()
{
    APawn* GhostPawn = GetPawn();
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!GhostPawn || !PlayerPawn) return FVector::ZeroVector;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation ResultLocation;

    // 计算一条背离玩家方向的逃跑线
    FVector AwayDirection = (GhostPawn->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
    // 往背离玩家的方向延伸 15 米作为逃跑预设中心点
    FVector SearchCenter = GhostPawn->GetActorLocation() + (AwayDirection * 1500.f);

    if (NavSys && NavSys->GetRandomReachablePointInRadius(SearchCenter, 500.f, ResultLocation))
    {
        return ResultLocation.Location;
    }
    return GhostPawn->GetActorLocation(); // 兜底：找不到就原地待命
}