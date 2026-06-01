// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/PickUp/InteractionActor_PickUp.h"
#include "Characters/PlayerCharacter.h"

void AInteractionActor_PickUp::BeginPlay()
{
	Super::BeginPlay();
}

AInteractionActor_PickUp::AInteractionActor_PickUp()
{
	// 开启碰撞
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 可以被射线检测到
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AInteractionActor_PickUp::OnInteract(APlayerCharacter* InteractingCharacter)
{
	if (!InteractingCharacter) return;

	InteractingCharacter->AddItemToInventory(ItemData);

	Destroy();
}