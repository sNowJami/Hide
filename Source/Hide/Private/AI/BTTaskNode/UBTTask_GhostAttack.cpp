// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTaskNode/UBTTask_GhostAttack.h"
#include "AIController.h"
#include "Characters/Ghost/GhostCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GhostTypes.h"

UBTTask_GhostAttack::UBTTask_GhostAttack()
{
    // 在虚幻引擎行为树编辑器中，该节点显示的节点方块名称
    NodeName = "Ghost Attack";
}

EBTNodeResult::Type UBTTask_GhostAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 1. 安全检查：获取当前的 AI 控制器与绑定的黑板组件
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BBComp)
    {
        return EBTNodeResult::Failed;
    }

    // 2. 尝试获取 AI 当前正在控制的 Pawn，并安全转换为女鬼类
    AGhostCharacter* Ghost = Cast<AGhostCharacter>(AIController->GetPawn());
    if (!Ghost)
    {
        return EBTNodeResult::Failed;
    }

    // 3. 验证女鬼肉身身上是否在编辑器中指派了有效的攻击动画资产
    if (Ghost->AttackMontage)
    {
        // 播放攻击动画蒙太奇，该函数会返回该动画资产的实际播放时长（单位：秒）
        float AnimDuration = Ghost->PlayAnimMontage(Ghost->AttackMontage);

        // 关键逻辑：立即开启黑板中的攻击冷却锁（IsAttackCooldown = true）
        // 行为树分支上的 Decorator 会因为这个变量变为 true，在接下来的时间里拦截并禁止再次进入攻击分支
        BBComp->SetValueAsBool(BBKeys::IsAttackCooldown, true);

        // 4. 使用引擎世界定时器（TimerManager），设计一个延迟回调，用于在攻击动画播完且冷却结束后重置状态
        FTimerHandle CooldownTimerHandle;

        // 恐怖游戏经典手感控制：总冷却时间 = 动画时长 + 2.0秒的空档呆滞期
        float TotalCooldownTime = AnimDuration + 2.0f;

        // 利用 C++ Lambda 表达式进行无缝延迟回调
        AIController->GetWorldTimerManager().SetTimer(CooldownTimerHandle, [BBComp]()
            {
                // 此 Lambda 块将在 TotalCooldownTime 秒后在主线程被异步调用
                if (BBComp)
                {
                    // 解开冷却锁，允许下一次攻击
                    BBComp->SetValueAsBool(BBKeys::IsAttackCooldown, false);

                    // 将女鬼当前的全局黑板状态强行切回 Idle（闲置观察）
                    // 这样做是为了让行为树的分支Selector在下一帧重新评估：是继续追玩家，还是转入搜寻
                    BBComp->SetValueAsEnum(BBKeys::CurrentState, static_cast<uint8>(EGhostState::Idle));
                }
            }, TotalCooldownTime, false);

        // 告诉行为树：该任务已经成功启动，行为树可以继续向下或者成功结束此 Sequence
        return EBTNodeResult::Succeeded;
    }

    // 如果策划忘记在蓝图里给女鬼拖入 AttackMontage 动画，则走兜底逻辑宣告节点失败
    return EBTNodeResult::Failed;
}