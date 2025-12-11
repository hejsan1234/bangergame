// Fill out your copyright notice in the Description page of Project Settings.


#include "APlanetActor.h"

// Sets default values
AAPlanetActor::AAPlanetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAPlanetActor::BeginPlay()
{
	Super::BeginPlay();
	
	FVector Origin;
	FVector Extents;
	GetActorBounds(true, Origin, Extents);

	PlanetRadius = Extents.GetMax();

	UE_LOG(LogTemp, Warning, TEXT("Planet radius set to: %f"), PlanetRadius);
}

// Called every frame
void AAPlanetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AAPlanetActor::GetGravityDirection(const FVector& Location) const
{
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();
	return -Direction;
}

