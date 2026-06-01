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
    // Sets default values for this actor's properties
    AInteractionActorBase();
protected:

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* ActorCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category = "Components")
    UStaticMeshComponent* MeshComponent; 



protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

   

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    

};