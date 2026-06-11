// Fill out your copyright notice in the Description page of Project Settings.
//

#include "Component/InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

bool UInventoryComponent::AddItem(FItemData NewItem)
{
    // 安全检查
    if (NewItem.ItemID == 0) return false;

    // 逻辑 1：如果可以堆叠，先寻找现有的相同 ID 物品
    if (NewItem.bCanStack)
    {
        for (int32 i = 0; i < InventoryItems.Num(); i++)
        {
            if (InventoryItems[i].ItemID == NewItem.ItemID)
            {
                // 检查是否没达到堆叠上限
                if (InventoryItems[i].CurrentCount < InventoryItems[i].MaxStackCount)
                {
                    // 计算可以塞进去多少数量
                    int32 AvailableSpace = InventoryItems[i].MaxStackCount - InventoryItems[i].CurrentCount;

                    if (NewItem.CurrentCount <= AvailableSpace)
                    {
                        InventoryItems[i].CurrentCount += NewItem.CurrentCount;
                        OnInventoryUpdated.Broadcast(); // 通知 UI 刷新
                        return true;
                    }
                    else
                    {
                        // 现有的格子堆满了，消耗掉一部分，剩下的继续找新格子
                        InventoryItems[i].CurrentCount = InventoryItems[i].MaxStackCount;
                        NewItem.CurrentCount -= AvailableSpace;
                    }
                }
            }
        }
    }

    // 逻辑 2：如果是新编号物体，或者无法堆叠，或者堆叠满了剩下的数量，寻找第一个空闲格子插入
    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        // 假设 ItemID == 0 代表这个格子是空的
        if (InventoryItems[i].ItemID == 0)
        {
            InventoryItems[i] = NewItem;
            OnInventoryUpdated.Broadcast(); // 通知 UI 刷新
            return true;
        }
    }

    // 背包满了
    return false;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventoryItems.SetNum(InventorySize);

	// ...
	
}


// Called every frame


