// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Ghost/GhostCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Optional.h"

// Sets default values
AGhostCharacter::AGhostCharacter()
{
    PrimaryActorTick.bCanEverTick = false; // AI 逻辑走行为树，角色自身通常不需要 Tick，优化性能

    // ---------------------------------------------------------------------
    // 1. 初始化音效组件并挂载到根组件（CapsuleComponent）
    // ---------------------------------------------------------------------
    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComp->SetupAttachment(RootComponent);
    AudioComp->bAutoActivate = false; // 不要一出生就自动播放，由代码/行为树控制

    // ---------------------------------------------------------------------
    // 2. 初始化攻击检测碰撞体，并挂载到网格体（Mesh）上
    // ---------------------------------------------------------------------
    AttackCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AttackCollisionComponent"));
    // 实际开发中，可以将其附加到手部骨骼：例如 GetMesh(), FName("Hand_R_Socket")
    AttackCollisionComp->SetupAttachment(GetMesh());
    AttackCollisionComp->SetCapsuleHalfHeight(40.f);
    AttackCollisionComp->SetCapsuleRadius(20.f);

    // 初始关闭碰撞，只有在播放攻击动画的伤害帧时才开启
    AttackCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AttackCollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AttackCollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // ---------------------------------------------------------------------
    // 3. 初始化 AI 感知刺激源 (Stimuli Source)
    // ---------------------------------------------------------------------
    StimuliSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
    // 注册为视觉刺激源（这样如果以后有多个 AI，或者队友 AI，能互相看见）
    StimuliSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSourceComp->RegisterWithPerceptionSystem();
}

void AGhostCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 绑定碰撞重叠事件
    if (AttackCollisionComp)
    {
        AttackCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AGhostCharacter::OnAttackOverlapBegin);
    }
}

void AGhostCharacter::SetAttackCollisionEnabled(bool bEnabled)
{
    if (AttackCollisionComp)
    {
        if (bEnabled)
        {
            AttackCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            AttackCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void AGhostCharacter::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(TEXT("Player")))
    {
        // 在这里应用伤害（Apply Damage）
        // UGameplayStatics::ApplyDamage(OtherActor, 50.f, GetController(), this, UDamageType::StaticClass());

        // 命中后立即关闭碰撞，防止单次攻击多段重复伤害
        SetAttackCollisionEnabled(false);
    }
}