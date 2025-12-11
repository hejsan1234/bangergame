// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UCLASS()
class MITTSPEL_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Sensitivity = 0.5f;

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
