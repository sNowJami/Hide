#pragma once

#include "CoreMinimal.h"
#include "GhostTypes.generated.h"

UENUM(BlueprintType)
enum class EGhostState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Searching   UMETA(DisplayName = "Searching"),
    Retreating  UMETA(DisplayName = "Retreating")
};


namespace BBKeys
{
    const FName TargetActor = TEXT("TargetActor");
    const FName CurrentState = TEXT("CurrentState");
    const FName LastKnownPlayerLocation = TEXT("LastKnownPlayerLocation");
    const FName LastNoiseLocation = TEXT("LastNoiseLocation");
    const FName IsAttackCooldown = TEXT("IsAttackCooldown");
    const FName TargetLocation = TEXT("TargetLocation");
}