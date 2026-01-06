// Fill out your copyright notice in the Description page of Project Settings.


#include "M_Skysphere.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
// Sets default values
AM_Skysphere::AM_Skysphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	RootComponent = SphereMesh;

	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereMesh->SetCastShadow(false);
	SphereMesh->SetMobility(EComponentMobility::Movable);

}

// Called when the game starts or when spawned
void AM_Skysphere::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AM_Skysphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Updating Skysphere location to camera location: %s"), *Cam->GetCameraLocation().ToString());
			//SetActorLocation(Cam->GetCameraLocation());
		}
	}
}

