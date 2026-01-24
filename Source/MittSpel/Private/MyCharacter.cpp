// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"

#include "PlanetMovementComponent.h"
#include "APlanetActor.h"

// Sets default values
AMyCharacter::AMyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlanetMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	MoveSpeed = 1600.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPivot"));
	CameraPivot->SetupAttachment(GetCapsuleComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraPivot);
	Camera->bUsePawnControlRotation = false;

	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	if (auto* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
		Move->bUseControllerDesiredRotation = false;
		Move->MaxCustomMovementSpeed = MoveSpeed;
		Move->BrakingDecelerationWalking = 7048.f;
		Move->MaxAcceleration = 6048.f;
	}
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MoveCompBase = GetCharacterMovement())
	{
		MoveCompBase->MaxWalkSpeed = MoveSpeed;
		MoveCompBase->JumpZVelocity = JumpHeight;
		MoveCompBase->GravityScale = 0.0f;
		MoveCompBase->SetMovementMode(MOVE_Custom);
	}

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (!PlanetRef) return;

			const FTransform SpawnTM = PlanetRef->GetSpawnTransform();
			TeleportTo(SpawnTM.GetLocation(), SpawnTM.Rotator(), false, true);
		});

	if (UPlanetMovementComponent* PlanetMove = Cast<UPlanetMovementComponent>(GetCharacterMovement()))
	{
		PlanetMove->Planet = PlanetRef;
	}

	if (Camera && CameraPivot)
	{
		Camera->AttachToComponent(CameraPivot, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

AAPlanetActor* AMyCharacter::GetCurrentPlanet() const
{
	if (const UPlanetMovementComponent* PlanetMove = Cast<UPlanetMovementComponent>(GetCharacterMovement()))
	{
		return PlanetMove->Planet;
	}
	return nullptr;
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsSpaceMode())
	{
		// 1) Behåll kamerans forward (från input-uppdaterad quat)
		const FVector Forward = CameraOrientation.GetForwardVector().GetSafeNormal();

		// 2) Försök låta SpaceUp följa kamerans up
		FVector DesiredUp = CameraOrientation.GetUpVector().GetSafeNormal();

		// 3) Skydd nära polen (Forward ~ Up) => bygg en stabil Up från en referens
		if (FMath::Abs(FVector::DotProduct(Forward, DesiredUp)) > 0.98f)
		{
			const FVector Ref = (FMath::Abs(Forward.Z) < 0.9f)
				? FVector::UpVector
				: FVector::ForwardVector;

			const FVector Right = FVector::CrossProduct(Ref, Forward).GetSafeNormal();
			DesiredUp = FVector::CrossProduct(Forward, Right).GetSafeNormal();
		}

		// 4) Uppdatera SpaceUp varje tick
		SpaceUp = DesiredUp;

		// 5) Rebuild orientation från Forward + SpaceUp (detta “nollar roll” enligt SpaceUp)
		const FRotator R = FRotationMatrix::MakeFromXZ(Forward, SpaceUp).Rotator();
		CameraOrientation = R.Quaternion();

		// 6) Applicera på kameran
		Camera->SetWorldRotation(CameraOrientation);
	}
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AMyCharacter::MoveUp);

	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyCharacter::StopJump);
}

// Movement input
void AMyCharacter::MoveForward(float Value)
{
	if (Value == 0.0f) return;

	if (IsSpaceMode())
	{
		const FVector Fwd = CameraOrientation.GetForwardVector().GetSafeNormal();
		AddMovementInput(Fwd, Value);
	}
	else
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Value == 0.f) return;

	if (IsSpaceMode())
	{
		const FVector Right = CameraOrientation.GetRightVector().GetSafeNormal();
		AddMovementInput(Right, Value);
	}
	else
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyCharacter::MoveUp(float Value)
{
	if (Value == 0.f) return;
	if (!IsSpaceMode()) return;

	const FVector Up = CameraOrientation.GetUpVector().GetSafeNormal();
	AddMovementInput(Up, Value);
}

void AMyCharacter::Turn(float Value)
{
	if (Value == 0.f) return;

	if (!IsSpaceMode())
	{
		AAPlanetActor* Planet = GetCurrentPlanet();
		if (!Planet) return;

		const FVector Up = (GetActorLocation() - Planet->GetActorLocation()).GetSafeNormal();
		const float YawSpeedDegPerSec = 180.f;

		const float DeltaYawRad = FMath::DegreesToRadians(YawSpeedDegPerSec * Value * GetWorld()->GetDeltaSeconds());
		const FQuat Delta = FQuat(Up, DeltaYawRad);
		SetActorRotation((Delta * GetActorQuat()).Rotator());
	}
	else
	{
		const float Speed = 120.f;
		const float AngleRad = FMath::DegreesToRadians(Value * Speed * GetWorld()->GetDeltaSeconds());

		const FVector Up = SpaceUp.GetSafeNormal();
		const FQuat DeltaYaw(Up, AngleRad);

		CameraOrientation = (DeltaYaw * CameraOrientation).GetNormalized();
	}
}

void AMyCharacter::LookUp(float Value)
{
	if (Value == 0.f) return;

	if (!IsSpaceMode())
	{
		PitchDeg = FMath::Clamp(PitchDeg + Value * Sensitivity * 2, -85.f, 85.f);
		CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));
	}
	else
	{
		const float Speed = 120.f;
		const float AngleRad = FMath::DegreesToRadians(Value * Speed * GetWorld()->GetDeltaSeconds());

		const FVector Right = CameraOrientation.GetRightVector();
		const FQuat DeltaPitch(Right, -AngleRad);

		CameraOrientation = (DeltaPitch * CameraOrientation).GetNormalized();
	}
}

void AMyCharacter::StartJump()
{
	bIsJumping = true;
}

void AMyCharacter::StopJump()
{
	bIsJumping = false;
}

void AMyCharacter::SetControlMode(EControlMode NewMode)
{
	if (ControlMode == NewMode) return;
	ControlMode = NewMode;

	UE_LOG(LogTemp, Warning, TEXT("Switched to control mode: %s"), 
		(ControlMode == EControlMode::Space) ? TEXT("Space") : TEXT("Planet"));

	// vi kör manuellt i båda, så håll av
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (ControlMode == EControlMode::Space)
	{
		if (Camera)
			CameraOrientation = Camera->GetComponentQuat();
		SpaceUp = GetActorUpVector().GetSafeNormal();
	}
	else
	{
		PitchDeg = 0.f;
		if (CameraPivot)
			CameraPivot->SetRelativeRotation(FRotator::ZeroRotator);
	}
}
