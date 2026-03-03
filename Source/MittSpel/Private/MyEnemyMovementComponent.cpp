#include "MyEnemyMovementComponent.h"
#include "GameFramework/Character.h"

void UMyEnemyMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* EnemyActor = GetOwner();
    AActor* PlanetActor = EnemyActor ? EnemyActor->GetOwner() : nullptr; // owner satt vid spawn

    Planet = Cast<AAPlanetActor>(PlanetActor);

    UE_LOG(LogTemp, Warning, TEXT("EnemyMovement Planet = %s"), *GetNameSafe(Planet));
}

void UMyEnemyMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    AAPlanetActor* GravPlanet = GetActivePlanet();
    if (!UpdatedComponent) { return; }

    UPlanetMovementComponent::FPlanetFrame Frame;
    if (!BuildPlanetFrame(Frame))
        return;

    const FVector& Up = Frame.Up;

    AlignCharacterToSurface(Up, DeltaTime);
}