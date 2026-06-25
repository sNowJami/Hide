// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UBTTask_GhostAttack.generated.h"

/**
 * 
 */
UCLASS()
class HIDE_API UBTTask_GhostAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_GhostAttack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
