#pragma once

#include "CoreMinimal.h"
#include "ItemStruct.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Props      UMETA(DisplayName = "Props"),
	PlotProp   UMETA(DisplayName = "PlotProp")
};

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    int32 ItemID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FString ItemName = TEXT("新物品");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FString Description = TEXT("这里是物品介绍...");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    int32 CurrentCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    int32 MaxStackCount = 99;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    bool bCanStack = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    UTexture2D* ItemIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    UStaticMesh* ItemMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemType ItemType;
};