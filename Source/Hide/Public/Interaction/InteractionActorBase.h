// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionActorBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class HIDE_API AInteractionActorBase : public AActor
{
    GENERATED_BODY()

public:
    AInteractionActorBase();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* ActorCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* MeshComponent;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};