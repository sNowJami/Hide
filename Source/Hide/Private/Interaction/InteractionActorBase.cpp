// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/InteractionActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AInteractionActorBase::AInteractionActorBase()
{
    PrimaryActorTick.bCanEverTick = true;
    USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
    RootComponent = DummyRoot;
    ActorCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    ActorCollision->SetupAttachment(RootComponent); 
    ActorCollision->SetSphereRadius(100.f);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent); 
    MeshComponent->SetRelativeLocation(FVector::ZeroVector);

    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
}

void AInteractionActorBase::BeginPlay()
{
    Super::BeginPlay();
}

void AInteractionActorBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}