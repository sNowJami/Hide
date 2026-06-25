// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GhostTypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "AGhostAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class HIDE_API AAGhostAIController : public AAIController
{
    GENERATED_BODY()

public:
    AAGhostAIController();

    // 当控制器“附身”到具体的女鬼 Character 时触发
    virtual void OnPossess(APawn* InPawn) override;

    // 当控制器“解除附身”时触发，用于清理绑定
    virtual void OnUnPossess() override;

    /* =====================================================================
    * 外部调用接口 (剧情触发 / 遭遇机制)
    * ===================================================================== */

    // 强制触发遭遇：可由关卡蓝图、过场动画 Sequence 或门体触发器调用
    UFUNCTION(BlueprintCallable, Category = "Ghost | AI")
    void ForceTriggerEncounter(AActor* PlayerActor, EGhostState InitialState, bool bPlayJumpScare);

    // 核心状态切换函数：安全地修改黑板上的当前状态枚举
    UFUNCTION(BlueprintCallable, Category = "Ghost | AI")
    void SetGhostState(EGhostState NewState);

protected:
    /* =====================================================================
    * AI 核心组件
    * ===================================================================== */

    // 行为树组件：控制 AI 的决策树执行
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | AI")
    UBehaviorTreeComponent* BehaviorTreeComp;

    // 黑板组件：存储 AI 的记忆数据（变量）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | AI")
    UBlackboardComponent* BlackboardComp;

    // 感知组件：AI 的耳目，用来统一挂载视觉、听觉等配置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | AI")
    UAIPerceptionComponent* PerceptionComp;

    /* =====================================================================
    * 感知具体配置 (可在蓝图子类中微调参数)
    * ===================================================================== */

    // 视觉配置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | AI | Perception")
    UAISenseConfig_Sight* SightConfig;

    // 听觉配置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | AI | Perception")
    UAISenseConfig_Hearing* HearingConfig;

    /* =====================================================================
    * 资产引用
    * ===================================================================== */

    // 在编辑器中指派的具体行为树资产
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ghost | AI")
    UBehaviorTree* GhostBehaviorTree;

private:
    /* =====================================================================
    * 感知系统核心回调
    * ===================================================================== */

    // 当感知系统刷新时（看到、丢掉、听到），此函数会被自动调用
    UFUNCTION()
    void HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // 内部辅助函数：计算背离玩家的撤退点
    FVector CalculateRetreatLocation();
};