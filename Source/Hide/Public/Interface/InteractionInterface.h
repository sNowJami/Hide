// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

class APlayerCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HIDE_API IInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void IFInteract(APlayerCharacter* BP_Player);

	// 2. 如果是“获取信息”，直接把返回值写在函数定义里，蓝图会自动识别为返回值
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void IFCanInteract(bool& bCaninteract) const;

	// 3. 如果需要返回多个值，继续使用引用参数，但建议参数名与函数名区分开
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void IFGetInteractPrompt(bool& bOutShowPrompt, FName& OutName) const;
};
