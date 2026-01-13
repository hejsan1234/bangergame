// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "GameFramework/Character.h"
#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

#include "MyCharacter.generated.h"

class AAPlanetActor;
class UPlanetMovementComponent;

UCLASS()
class MITTSPEL_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	bool IsPlanetJumping() const { return bIsJumping; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 4600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight = 9000.0f;

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

	UFUNCTION(BlueprintCallable, Category="Planet Gravity")
	AAPlanetActor* GetCurrentPlanet() const;

	float PitchDeg = 0.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveBackward(float Value);
	void MoveRight(float Value);
	void MoveLeft(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartJump();
	void StopJump();
};
