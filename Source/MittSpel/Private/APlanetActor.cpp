#include "APlanetActor.h"
#include "Components/StaticMeshComponent.h"

AAPlanetActor::AAPlanetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetMesh"));
	RootComponent = PlanetMesh;

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootComponent);
}

void AAPlanetActor::BeginPlay()
{
	Super::BeginPlay();
	OrbitAngleDeg = PhaseDeg;
	OrbitAxis = OrbitAxis.GetSafeNormal();
	if (bEnableOrbit && ParentBody)
	{
		UpdateOrbit(0.f);
	}
	UpdateSpawnPoint();
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

void AAPlanetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bEnableOrbit && ParentBody)
	{
		UpdateOrbit(DeltaTime);
	}
}

void AAPlanetActor::UpdateOrbit(float DeltaTime)
{
	OrbitAngleDeg = FMath::Fmod(OrbitAngleDeg + OrbitSpeedDegPerSec * DeltaTime, 360.f);

	const FVector ParentPos = ParentBody->GetActorLocation();
	const FQuat Q(OrbitAxis, FMath::DegreesToRadians(OrbitAngleDeg));

	const FVector LocalOffset = Q.RotateVector(FVector(OrbitRadius, 0.f, 0.f));
	SetActorLocation(ParentPos + LocalOffset);
}

FTransform AAPlanetActor::GetSpawnTransform() const
{
	return SpawnPoint
		? SpawnPoint->GetComponentTransform()
		: GetActorTransform();
}

void AAPlanetActor::UpdateSpawnPoint()
{
	if (!SpawnPoint) return;

	const float Rws = GetPlanetRadiusWS();
	const float MarginWS = 100.f;

	const FVector Scale = GetActorScale3D();
	const float Sz = FMath::Max(Scale.Z, KINDA_SMALL_NUMBER);

	const float Rlocal = Rws / Sz;
	const float MarginLocal = MarginWS / Sz;

	SpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, Rlocal + MarginLocal));
}
