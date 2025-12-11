// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MoveSpeed = 600.0f;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (auto* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
	}
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveBackward", this, &AMyCharacter::MoveBackward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveLeft", this, &AMyCharacter::MoveLeft);

	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyCharacter::StopJump);

}

// Move the character

void AMyCharacter::MoveForward(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyCharacter::MoveBackward(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyCharacter::MoveRight(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyCharacter::MoveLeft(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyCharacter::Turn(float Value) {
	if (Value != 0.0f) {
		Value *= Sensitivity;
		AddControllerYawInput(Value);
	}
}

void AMyCharacter::LookUp(float Value) {
	if (Value != 0.0f) {
		Value *= Sensitivity;
		AddControllerPitchInput(Value);
	}
}

void AMyCharacter::StartJump() {
	Jump();
}

void AMyCharacter::StopJump() {
	StopJumping();
}
