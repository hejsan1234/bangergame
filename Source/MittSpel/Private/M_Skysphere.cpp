// Fill out your copyright notice in the Description page of Project Settings.


#include "M_Skysphere.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SolarSystemsManager.h"
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

	OrbitAngleDeg = FMath::Fmod(OrbitAngleDeg + OrbitSpeedDegPerSec * DeltaTime, 360.f);

	const FVector Axis = OrbitAxis.GetSafeNormal();
	const FQuat Q(Axis, FMath::DegreesToRadians(-OrbitAngleDeg));

	SphereMesh->SetRelativeRotation(Q);

	//SphereMesh->AddLocalRotation(FRotator(0.f, 0.f, 1.f * DeltaTime));

	//UE_LOG(LogTemp, Warning, TEXT("Skysphere Tick: ActiveBody = %s"), ActiveBody ? *ActiveBody->GetName() : TEXT("None"));

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Updating Skysphere location to camera location: %s"), *Cam->GetCameraLocation().ToString());
			//SetActorLocation(Cam->GetCameraLocation());
		}
	}
}

void AM_Skysphere::SetActiveBody(AAPlanetActor* NewActiveBody) {
	ActiveBody = NewActiveBody;
}

