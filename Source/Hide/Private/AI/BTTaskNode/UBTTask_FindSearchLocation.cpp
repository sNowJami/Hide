// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTaskNode/UBTTask_FindSearchLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GhostTypes.h"                      
#include "NavigationSystem.h"

UBTTask_FindSearchLocation::UBTTask_FindSearchLocation()
{
    // 在虚幻引擎行为树编辑器中，该节点显示的节点方块名称
    NodeName = "Find Ghost Search Location";
}

EBTNodeResult::Type UBTTask_FindSearchLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 1. 获取核心组件
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();

    if (!BBComp || !AIController)
    {
        return EBTNodeResult::Failed;
    }

    // 2. 恐怖游戏高智能 AI 逻辑：决策寻路的几何圆心
    // 优先：去女鬼刚刚听到的声音点（LastNoiseLocation）
    FVector CenterLocation = BBComp->GetValueAsVector(BBKeys::LastNoiseLocation);

    // 如果声音坐标为 (0, 0, 0)，代表此时没有新的噪音，则退而求其次，前往玩家最后消失的视线死角（LastKnownPlayerLocation）
    if (CenterLocation.IsZero())
    {
        CenterLocation = BBComp->GetValueAsVector(BBKeys::LastKnownPlayerLocation);
    }

    // 兜底：如果游戏刚开局或者两个坐标全是空，则以女鬼自身当前所处的物理坐标作为圆心在附近巡逻
    if (CenterLocation.IsZero() && AIController->GetPawn())
    {
        CenterLocation = AIController->GetPawn()->GetActorLocation();
    }

    // 3. 获取虚幻引擎当前关卡的全局导航网格寻路系统 (NavMeshV1)
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    // 声明一个虚幻专用的导航点存储结构，用于接收寻路算法传回的结果
    FNavLocation CalculatedResultLocation;

    // 4. 调用引擎底层的寻路函数：在设定的 CenterLocation 圆心周围 600码（即6米）半径内，
    // 随机抽取一个经过 NavMesh 烘焙拓扑校准过的、女鬼的双脚绝对能走过去的真实物理点。
    const float SearchRadius = 600.f;
    if (NavSys->GetRandomReachablePointInRadius(CenterLocation, SearchRadius, CalculatedResultLocation))
    {
        // 5. 【核心操作】：将计算成功的 3D 向量坐标写入黑板专用的 TargetLocation 中
        // 这样，紧随其后的原生 MoveTo 节点就能直接读取该键并进行平滑移步
        BBComp->SetValueAsVector(BBKeys::TargetLocation, CalculatedResultLocation.Location);

        // 6. 【擦除听觉缓存】：这行非常关键！
        // 既然女鬼已经成功消耗并响应了这次听到的声音，必须把 LastNoiseLocation 重置为 (0,0,0)。
        // 否则，如果不擦除，下一次进入此节点时，AI 还会死循环般地在这个声音点附近无限打转。
        BBComp->SetValueAsVector(BBKeys::LastNoiseLocation, FVector::ZeroVector);

        // 任务大功告成，通知行为树控制流立刻向后执行 MoveTo 节点
        return EBTNodeResult::Succeeded;
    }

    // 如果由于地形极其刁钻，或者女鬼卡在了 NavMesh 寻路网格之外导致算法没抽到有效点，返回 Failed 报错
    return EBTNodeResult::Failed;
}