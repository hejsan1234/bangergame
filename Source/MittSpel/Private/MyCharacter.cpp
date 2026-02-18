// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include <Kismet/GameplayStatics.h>

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

	CameraPivot->SetRelativeLocation(FVector(15.f, 0.f, 120.f));
	Camera->SetRelativeLocation(FVector::ZeroVector); // org var 0 0 64
	GetCapsuleComponent()->InitCapsuleSize(34.f, 120.f); // org var egenltigen borta men 34 88

	PendingLookDirWorld = FVector::ForwardVector;

	if (auto* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
		Move->bUseControllerDesiredRotation = false;
		Move->MaxCustomMovementSpeed = MoveSpeed;
		Move->BrakingDecelerationWalking = 7048.f;
		Move->MaxAcceleration = 6048.f;
	}

	// CHARACTER MESH
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetupAttachment(GetCapsuleComponent());

	MeshComp->SetRelativeScale3D(FVector(1.45f, 1.45f, 1.45f)); // testa 1.2–1.4

	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -120.f));

	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASolarSystemManager* SSM = Cast<ASolarSystemManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASolarSystemManager::StaticClass())
	);

	if (SSM)
	{
		AddTickPrerequisiteActor(SSM);
	}

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

	// Load the skeletal mesh asset at runtime
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const TCHAR* MannyRef =
			TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple'");

		USkeletalMesh* Manny = LoadObject<USkeletalMesh>(nullptr, MannyRef);
		if (Manny)
		{
			UE_LOG(LogTemp, Warning, TEXT("Loaded Manny_Simple via LoadObject: %s"), *GetNameSafe(Manny));
			MeshComp->SetSkeletalMesh(Manny);
			MeshComp->SetVisibility(true, true);
			MeshComp->SetHiddenInGame(false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LoadObject failed for MannyRef"));
		}
	}

	GetMesh()->HideBoneByName(HeadBoneName, EPhysBodyOp::PBO_None);
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

	// DEBUG

	// Capsule (collision)
	//const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	//const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	//DrawDebugCapsule(
	//	GetWorld(),
	//	GetCapsuleComponent()->GetComponentLocation(),
	//	CapsuleHalfHeight,
	//	CapsuleRadius,
	//	GetCapsuleComponent()->GetComponentQuat(),
	//	FColor::Blue,
	//	false,
	//	0.f,
	//	0,
	//	2.f
	//);

	//// Mesh bounds (visual size)
	//if (USkeletalMeshComponent* M = GetMesh())
	//{
	//	const FBoxSphereBounds B = M->Bounds;
	//	DrawDebugBox(
	//		GetWorld(),
	//		B.Origin,
	//		B.BoxExtent,
	//		M->GetComponentQuat(),
	//		FColor::Green,
	//		false,
	//		0.f,
	//		0,
	//		2.f
	//	);
	//}

	if (IsSpaceMode())
	{
		const FVector Forward = CameraOrientation.GetForwardVector().GetSafeNormal();
		FVector DesiredUp = CameraOrientation.GetUpVector().GetSafeNormal();

		if (FMath::Abs(FVector::DotProduct(Forward, DesiredUp)) > 0.98f)
		{
			const FVector Ref = (FMath::Abs(Forward.Z) < 0.9f) ? FVector::UpVector : FVector::ForwardVector;
			const FVector Right = FVector::CrossProduct(Ref, Forward).GetSafeNormal();
			DesiredUp = FVector::CrossProduct(Forward, Right).GetSafeNormal();
		}

		SpaceUp = DesiredUp;

		const FRotator R = FRotationMatrix::MakeFromXZ(Forward, SpaceUp).Rotator();
		CameraOrientation = R.Quaternion();

		if (Camera)
		{
			Camera->SetWorldRotation(CameraOrientation);

			// BAD WAY TO DO THIS, BUT TEMPORARY # TODO: SET ACTORROTATION AND MAKE CAMERA RELATIVE TO ACTOR IN SPACE MODE INSTEAD OF SETTING WORLD ROTATION
			SetActorRotation(CameraOrientation);
		}
	}

	if (!IsSpaceMode() && bHoldSpaceCamInPlanet && Camera)
	{
		Camera->SetWorldRotation(HoldSpaceCamWorld);

		if (HoldCamTicks > 0)
		{
			HoldCamTicks--;
			return;
		}

		bHoldSpaceCamInPlanet = false;

		const FVector Up = GetActorUpVector().GetSafeNormal();
		const FVector Fwd = PendingLookDirWorld.GetSafeNormal();

		FVector PlanarFwd = FVector::VectorPlaneProject(Fwd, Up).GetSafeNormal();
		if (PlanarFwd.IsNearlyZero())
		{
			const FVector Ref = (FMath::Abs(Up.Z) < 0.9f) ? FVector::UpVector : FVector::ForwardVector;
			PlanarFwd = FVector::CrossProduct(Up, Ref).GetSafeNormal();
		}

		PendingActorTargetQuat = FRotationMatrix::MakeFromXZ(PlanarFwd, Up).ToQuat();

		const float Sin = FVector::DotProduct(Fwd, Up);
		const float Cos = FVector::DotProduct(Fwd, PlanarFwd);
		const float PitchRad = FMath::Atan2(Sin, Cos);
		PendingPlanetPitchDeg = FMath::Clamp(FMath::RadiansToDegrees(PitchRad), -85.f, 85.f);

		const FQuat SavedActor = GetActorQuat();
		const float SavedPitch = PitchDeg;

		SetActorRotation(PendingActorTargetQuat);
		PitchDeg = PendingPlanetPitchDeg;
		if (CameraPivot) CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
		Camera->UpdateComponentToWorld();

		BlendTargetCamWorld = Camera->GetComponentQuat();

		SetActorRotation(SavedActor);
		PitchDeg = SavedPitch;
		if (CameraPivot) CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));
		Camera->SetWorldRotation(HoldSpaceCamWorld);

		bBlendToPlanetCam = true;
		BlendTime = 0.f;
		BlendDuration = 0.7f;
		BlendStartCamWorld = HoldSpaceCamWorld;

		return;
	}

	if (!IsSpaceMode() && bBlendToPlanetCam && Camera)
	{
		BlendTime += DeltaTime;
		const float A = FMath::Clamp(BlendTime / BlendDuration, 0.f, 1.f);
		const float S = A * A * (3.f - 2.f * A);

		const FQuat Q = FQuat::Slerp(BlendStartCamWorld, BlendTargetCamWorld, S).GetNormalized();
		Camera->SetWorldRotation(Q);

		if (A >= 1.f)
		{
			bBlendToPlanetCam = false;

			SetActorRotation(PendingActorTargetQuat);
			PitchDeg = PendingPlanetPitchDeg;
			if (CameraPivot) CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));

			Camera->SetRelativeRotation(FRotator::ZeroRotator);
		}

		return;
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
	auto* Move = GetCharacterMovement();
	if (Value == 0.f) {
		if (Move)
		{
			Move->MaxCustomMovementSpeed = 1600.f;
		}
		return;
	}

	if (!IsSpaceMode()) {
		if (Value > 0.1f)
			Move->MaxCustomMovementSpeed = 15000.f;
	}
	else {
		const FVector Up = CameraOrientation.GetUpVector().GetSafeNormal();
		AddMovementInput(Up, Value);
	}
}

void AMyCharacter::Turn(float Value)
{
	if (Value == 0.f) return;

	if (bHoldSpaceCamInPlanet || bBlendToPlanetCam) return;

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

	if (bHoldSpaceCamInPlanet || bBlendToPlanetCam) return;

	if (!IsSpaceMode())
	{
		PitchDeg = FMath::Clamp(PitchDeg + Value * Sensitivity * 2, -85.f, 85.f);
		if (CameraPivot)
		{
			CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));
		}
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

	const EControlMode OldMode = ControlMode;
	ControlMode = NewMode;

	// Show/hide mesh based on mode
	if (USkeletalMeshComponent* M = GetMesh())
	{
		const bool bShow = (NewMode == EControlMode::Planet);

		M->SetHiddenInGame(!bShow);
		M->SetVisibility(bShow, true);
		M->SetCastShadow(bShow);

		M->bPauseAnims = !bShow;
		M->SetComponentTickEnabled(bShow);
	}

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (OldMode == EControlMode::Planet && NewMode == EControlMode::Space)
	{
		UE_LOG(LogTemp, Warning, TEXT("Switching to Space Mode, pausing game to prevent physics issues"));
		//GetWorld()->bDebugPauseExecution = true;
	}

	if (NewMode == EControlMode::Space)
	{
		if (Camera)
		{
			CameraOrientation = Camera->GetComponentQuat();
		}
		SpaceUp = GetActorUpVector().GetSafeNormal();

		if (auto* Move = GetCharacterMovement())
		{
			Move->BrakingDecelerationWalking = 17048.f;
			Move->MaxAcceleration = 16048.f;
		}

		return;
	}

	if (OldMode == EControlMode::Space && NewMode == EControlMode::Planet)
	{
		PendingLookDirWorld = CameraOrientation.GetForwardVector().GetSafeNormal();

		HoldSpaceCamWorld = CameraOrientation;
		bHoldSpaceCamInPlanet = true;
		HoldCamTicks = 3;
		if (auto* Move = GetCharacterMovement())
		{
			Move->BrakingDecelerationWalking = 7048.f;
			Move->MaxAcceleration = 6048.f;
		}
	}
}
