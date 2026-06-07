// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Door.generated.h"

// 定义门和玩家交互的4个状态
UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Normal              UMETA(DisplayName = "Normal"),
	Pushing             UMETA(DisplayName = "Pushing"),
	EventTriggered      UMETA(DisplayName = "EventTriggered"),
	AutoComplete        UMETA(DisplayName = "AutoComplete")
};

UCLASS()
class HIDE_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 外部调用：玩家按下交互键
	UFUNCTION(BlueprintCallable, Category = "Door|Interaction")
	void TryInteract(APlayerController* PC);

	// 外部调用：玩家输入推进（按住W时每帧调用，或通过增强输入传入Axis值）
	UFUNCTION(BlueprintCallable, Category = "Door|Interaction")
	void PushDoor(float AxisValue);




protected:
    // ─── 组件 ───
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraComponent* DoorCamera;

    // ─── 核心变量 ───
    UPROPERTY(EditAnywhere, Category = "Door|Config")
    UCurveVector* CameraMovementCurve; // 允许你在蓝图里自由绘制相机的 XYZ 轨迹

    UPROPERTY(BlueprintReadOnly, Category = "Door|State")
    EDoorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Door|Variables")
    float CurrentProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Door|Variables")
    float TriggerValue;

    UPROPERTY(BlueprintReadOnly, Category = "Door|Variables")
    bool bHasEventTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Door|Variables")
    bool bIsEventActive;

    // ─── 配置参数 ───
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
    float PushSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
    float MaxPushAngle = 45.0f; // 0-60进度时门的最大旋转角度

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
    float TargetOpenAngle = 90.0f; // 门完全打开的角度

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Config")
    FVector CameraPushOffset = FVector(10.0f, 0.0f, 0.0f); // 推进时相机的最大位移

    // ─── 自动完成的时间轴 ───
    FTimeline AutoCompleteTimeline;

    UPROPERTY(EditAnywhere, Category = "Door|Timeline")
    UCurveFloat* OpenCurve;

    UFUNCTION()
    void HandleTimelineProgress(float Value);

    UFUNCTION()
    void OnTimelineFinished();

    // ─── 内部逻辑函数 ───
    void UpdateDoorVisuals();
    void TriggerRandomEvent();
    void ExitInteraction();

    UFUNCTION(BlueprintCallable, Category = "Door|Events")
    void EndCurrentEvent(); // 事件结束时由蓝图/C++调用恢复控制

    void StartAutoComplete();


    // 声明惊吓事件 A，允许在蓝图里复写
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|ScareEvents")
    void OnPlayScareA();

    // 声明惊吓事件 B，允许在蓝图里复写
    UFUNCTION(BlueprintImplementableEvent, Category = "Door|ScareEvents")
    void OnPlayScareB();


    // 缓存最初的相机相对位置
    FVector InitialCameraLocation;

    // 声明一个用来控制事件结束的定时器句柄
    FTimerHandle EventTimerHandle;

private:
    UPROPERTY()
    APlayerController* CachedPC;

    // 辅助函数：模拟获取全局变量，实际建议写在你的自定义GameInstance中
    bool GetGlobalSkipPushStage() const;
    void SetGlobalSkipPushStage(bool bSkip);
};

