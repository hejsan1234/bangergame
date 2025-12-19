// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "APlanetActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlanetMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "MyGun.h"

// Sets default values
AMyCharacter::AMyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlanetMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MoveSpeed = 600.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPivot"));
	CameraPivot->SetupAttachment(GetCapsuleComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraPivot);
	Camera->bUsePawnControlRotation = false;

	// valfritt: placera kameran
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 100.f));

	if (auto* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
		Move->bUseControllerDesiredRotation = false;
	}
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Pawn in use: %s"), *GetClass()->GetName());


	// Bas-inställningar för movement
	if (UCharacterMovementComponent* MoveCompBase = GetCharacterMovement())
	{
		MoveCompBase->MaxWalkSpeed = MoveSpeed;
		MoveCompBase->JumpZVelocity = JumpHeight;

		// Vi använder custom fysik, så vanlig -Z-gravity ska inte styra
		MoveCompBase->GravityScale = 0.0f;

		// Viktigt: sätt custom movement mode så PhysCustom() kallas
		MoveCompBase->SetMovementMode(MOVE_Custom);
	}

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (!PlanetRef) return;

			const FTransform SpawnTM = PlanetRef->GetSpawnTransform();
			TeleportTo(SpawnTM.GetLocation(), SpawnTM.Rotator(), false, true);

			UE_LOG(LogTemp, Warning, TEXT("Initial teleport done via next tick"));
	});


	// Koppla planeten till vår custom movement component
	if (UPlanetMovementComponent* PlanetMove = Cast<UPlanetMovementComponent>(GetCharacterMovement()))
	{
		PlanetMove->Planet = PlanetRef;
	}

	if (Camera && CameraPivot)
	{
		Camera->AttachToComponent(CameraPivot, FAttachmentTransformRules::KeepRelativeTransform);
	}
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    if (!GunClass)
    {
        UE_LOG(LogTemp, Error, TEXT("GunClass is not set on MyCharacter!"));
        return;
    }

    Gun = GetWorld()->SpawnActor<AMyGun>(GunClass, SpawnParams);

    if (Gun)
    {
        Gun->AttachToComponent(
            Camera,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );
    }
    
    if (!PlanetRef)
    {
        UE_LOG(LogTemp, Error, TEXT("PlanetRef is NOT set on MyCharacter"));
    }
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    if (!PlanetRef)
        {
            return;
        }
    
	const FVector PlayerLoc = GetActorLocation();
	const FVector PlanetLoc = PlanetRef->GetActorLocation();
	const float Dist = FVector::Dist(PlayerLoc, PlanetLoc);
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
    
    PlayerInputComponent->BindAction(
        "Fire",
        IE_Pressed,
        this,
        &AMyCharacter::Fire
    );

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
	if (Value == 0.f || !PlanetRef) return;

	const FVector Up = (GetActorLocation() - PlanetRef->GetActorLocation()).GetSafeNormal();

	// grader per sekund (justera!)
	const float YawSpeedDegPerSec = 180.f;

	const float DeltaYawRad = FMath::DegreesToRadians(YawSpeedDegPerSec * Value * GetWorld()->GetDeltaSeconds());
	const FQuat Delta = FQuat(Up, DeltaYawRad);

	SetActorRotation((Delta * GetActorQuat()).Rotator());
}

void AMyCharacter::LookUp(float Value) {
	if (Value == 0.f) return;

	PitchDeg = FMath::Clamp(PitchDeg + Value * Sensitivity*2, -85.f, 85.f);
	CameraPivot->SetRelativeRotation(FRotator(PitchDeg, 0.f, 0.f));
}

void AMyCharacter::StartJump() {
	bIsJumping = true;
}

void AMyCharacter::StopJump() {
	bIsJumping = false;
}

void AMyCharacter::Fire()
{
    if (!Gun || !Camera) return;

    Gun->Fire(
        Camera->GetComponentLocation(),
        Camera->GetForwardVector()
    );
    
    UE_LOG(LogTemp, Error, TEXT("Fire!!!!"));
}
