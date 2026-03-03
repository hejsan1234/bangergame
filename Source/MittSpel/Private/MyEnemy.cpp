// Fill out your copyright notice in the Description page of Project Settings.
#include "GameFramework/CharacterMovementComponent.h"

#include "MyEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "MyEnemyMovementComponent.h"

// Sets default values
AMyEnemy::AMyEnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMyEnemyMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
}

// Called every frame
void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMyEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

