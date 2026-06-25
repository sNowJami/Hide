// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GhostTypes.h"
#include "GhostCharacter.generated.h"

class UCapsuleComponent;
class UAudioComponent;
class UAIPerceptionStimuliSourceComponent;

UCLASS()
class HIDE_API AGhostCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    AGhostCharacter();

protected:
    virtual void BeginPlay() override;

    /* =====================================================================
    * 基础组件 (Components)
    * ===================================================================== */

public:
    // 1. 攻击范围/伤害检测碰撞体 (例如挂在手部，或者作为身体前方的判定区)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | Components")
    UCapsuleComponent* AttackCollisionComp;

    // 2. 尖叫/喘息音效组件 (方便在 C++ 中随时控制声音的播放、停止或切换参数)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | Components")
    UAudioComponent* AudioComp;

    // 3. 感知刺激源组件：这是关键！它让女鬼自己也能被其他 AI 察觉，或者标记自身团队属性
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost | Components")
    UAIPerceptionStimuliSourceComponent* StimuliSourceComp;


    /* =====================================================================
    * 动画资产 (Montages)
    * ===================================================================== */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost | Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost | Animation")
    UAnimMontage* SearchMontage;

    // 开门杀/突袭专属动画
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost | Animation")
    UAnimMontage* JumpScareMontage;


    /* =====================================================================
    * 战斗与碰撞事件回调
    * ===================================================================== */

    // 碰撞开始回调：当攻击判定舱接触到物体时触发
    UFUNCTION()
        void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
            bool bFromSweep, const FHitResult& SweepResult);

public:
    // 开启/关闭攻击碰撞（通常由 AnimNotify 在动画挥爪时调用）
    UFUNCTION(BlueprintCallable, Category = "Ghost | Combat")
        void SetAttackCollisionEnabled(bool bEnabled);
};
