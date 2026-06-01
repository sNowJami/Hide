// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

APlayerCharacter::APlayerCharacter()
{
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(200.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

void APlayerCharacter::AddItemToInventory(const FItemData& NewItem)
{
    // 如果物品允许堆叠
    if (NewItem.bCanStack)
    {
        // 遍历背包
        for (FItemData& Item : Inventory)
        {
            // 找相同ID
            if (Item.ItemID == NewItem.ItemID)
            {
                // 增加数量
                Item.Quantity += NewItem.Quantity;

                UE_LOG(LogTemp, Warning, TEXT("物品堆叠成功"));

                return;
            }
        }
    }
    // 没找到相同物品
    Inventory.Add(NewItem);

    UE_LOG(LogTemp, Warning, TEXT("新增物品"));
}



void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}