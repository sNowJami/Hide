// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractionActorBase.h"
#include "InteractionActor_PickUp.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class HIDE_API AInteractionActor_PickUp : public AInteractionActorBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	AInteractionActor_PickUp();



};
