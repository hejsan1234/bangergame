#include "APlanetActor.h"
#include "Components/StaticMeshComponent.h"

AAPlanetActor::AAPlanetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetMesh"));
	RootComponent = PlanetMesh;
}

void AAPlanetActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Planet radius (WS): %f"), GetPlanetRadiusWS());
}

float AAPlanetActor::GetPlanetRadiusWS() const
{
	if (bOverridePlanetRadius)
		return PlanetRadius;

	if (!PlanetMesh)
		return PlanetRadius;

	return PlanetMesh->Bounds.SphereRadius; // world space, scale inräknad
}

FVector AAPlanetActor::GetGravityDirection(const FVector& Location) const
{
	return (GetActorLocation() - Location).GetSafeNormal();
}
