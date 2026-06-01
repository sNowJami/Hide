// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractionActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SphereComponent.h"



// Sets default values
AInteractionActorBase::AInteractionActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
void AInteractionActorBase::BeginPlay()
{
	Super::BeginPlay();
   
  
}

void AInteractionActorBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
