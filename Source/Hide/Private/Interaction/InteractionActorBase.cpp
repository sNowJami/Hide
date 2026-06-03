// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/InteractionActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AInteractionActorBase::AInteractionActorBase()
{
    PrimaryActorTick.bCanEverTick = true;
    ActorCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(ActorCollision);
    RootComponent = ActorCollision;
    ActorCollision->SetSphereRadius(100.f);
    MeshComponent->SetRelativeLocation(FVector::ZeroVector);

    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void AInteractionActorBase::BeginPlay()
{
    Super::BeginPlay();
}

void AInteractionActorBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}