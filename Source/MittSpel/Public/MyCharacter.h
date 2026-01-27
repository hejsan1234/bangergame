// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class AAPlanetActor;
class UPlanetMovementComponent;
class UCameraComponent;
class USceneComponent;

UENUM()
enum class EControlMode : uint8
{
	Planet,
	Space
};

UCLASS()
class MITTSPEL_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	bool IsPlanetJumping() const { return bIsJumping; }

	UPROPERTY(BlueprintReadOnly)
	EControlMode ControlMode = EControlMode::Planet;

	UFUNCTION(BlueprintCallable)
	void SetControlMode(EControlMode NewMode);

	bool IsSpaceMode() const { return ControlMode == EControlMode::Space; }

	UCameraComponent* GetCameraComponent() const { return Camera; }
	USceneComponent* GetCameraPivot() const { return CameraPivot; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Sensitivity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Gravity")
	AAPlanetActor* PlanetRef = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USceneComponent* CameraPivot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsJumping = false;

	UFUNCTION(BlueprintCallable, Category = "Planet Gravity")
	AAPlanetActor* GetCurrentPlanet() const;

	float PitchDeg = 0.f;

	FQuat CameraOrientation = FQuat::Identity;
	FVector SpaceUp = FVector::UpVector;

	FVector PendingLookDirWorld = FVector::ForwardVector;

	bool bHoldSpaceCamInPlanet = false;
	int32 HoldCamTicks = 0;
	FQuat HoldSpaceCamWorld = FQuat::Identity;

	bool  bBlendToPlanetCam = false;
	float BlendTime = 0.f;
	float BlendDuration = 0.7f;

	FQuat BlendStartCamWorld = FQuat::Identity;
	FQuat BlendTargetCamWorld = FQuat::Identity;

	FQuat PendingActorTargetQuat = FQuat::Identity;
	float PendingPlanetPitchDeg = 0.f;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);

	void Turn(float Value);
	void LookUp(float Value);

	void StartJump();
	void StopJump();
};
