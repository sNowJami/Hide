// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemStruct.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HIDE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

    // 背包容量：12个格子
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 InventorySize = 12;

    // 背包的内容数组（存放当前格子的数据）
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FItemData> InventoryItems;

    // 核心函数：尝试将物品加入背包
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FItemData NewItem);

    // UI 刷新事件
   UPROPERTY(BlueprintAssignable, Category = "Inventory")
   FOnInventoryUpdated OnInventoryUpdated;

protected:
    virtual void BeginPlay() override;
 
};
