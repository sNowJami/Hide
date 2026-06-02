// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractionActorBase.h"
#include "ItemStruct.h"
#include "InteractionActor_PickUp.generated.h"

class USphereComponent;
class APlayerCharacter;

UCLASS()
class HIDE_API AInteractionActor_PickUp : public AInteractionActorBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AInteractionActor_PickUp();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemData ItemData;
	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemData GetItemData() const { return ItemData; }
};