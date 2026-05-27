// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"





APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bUseControllerRotationPitch = false; //设为true 角色模型会跟随鼠标上下移动而"上下点头"，false 会通过摄像机旋转实现 不需要模型去倾斜
	bUseControllerRotationYaw = true; //设为true 角色模型会始终面朝摄像机看去的方向, false 你的角色身体会像个转台一样跟着鼠标左右转
	bUseControllerRotationRoll = false; //设为true 你的角色会随鼠标左右晃动而向侧面倾斜。false 除非你有特殊的飞行或极度摇晃的视角需求，否则开启它会让角色看起来非常奇怪。

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;

	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.f;
	  
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}
