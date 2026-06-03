// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "ItemStruct.h"
#include "PlayerCharacter.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class HIDE_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;





protected:
	virtual void BeginPlay() override;

private:
};