// Fill out your copyright notice in the Description page of Project Settings.
#include "GameFramework/CharacterMovementComponent.h"

#include "MyEnemy.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "MyEnemyMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AMyEnemy::AMyEnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMyEnemyMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	const float SIZE = 120.f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);

	if (MannyMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MannyMesh.Object);
	}

	GetCapsuleComponent()->InitCapsuleSize(34.f, 120.f);
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetupAttachment(GetCapsuleComponent());

	MeshComp->SetRelativeScale3D(FVector(1.45f, 1.45f, 1.45f)); // testa 1.2–1.4

	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -140.f));

	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AMyEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bRunPhysicsWithNoController = true;
		Move->SetMovementMode(MOVE_Custom);
	}
}

// Called every frame
void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Normalize();

	if (UMyEnemyMovementComponent* Move = Cast<UMyEnemyMovementComponent>(GetCharacterMovement()))
	{
		Move->SetAIInput(ToPlayer);
	}

	const float Dist = FVector::Dist(Player->GetActorLocation(), GetActorLocation());

	const float KillDistance = 200.f;

	if (Dist < KillDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player caught!"));

		// Stäng spelet
		UKismetSystemLibrary::QuitGame(
			GetWorld(),
			nullptr,
			EQuitPreference::Quit,
			true
		);
	}
}

// Called to bind functionality to input
void AMyEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

