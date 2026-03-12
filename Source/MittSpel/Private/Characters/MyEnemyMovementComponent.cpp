#include "Characters/MyEnemyMovementComponent.h"
#include "GameFramework/Character.h"

void UMyEnemyMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* EnemyActor = GetOwner();
    AActor* PlanetActor = EnemyActor ? EnemyActor->GetOwner() : nullptr; // owner satt vid spawn

    Planet = Cast<AAPlanetActor>(PlanetActor);

    AAPlanetActor* GravPlanet = GetActivePlanet();
}

void UMyEnemyMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (!UpdatedComponent) { return; }

    EnsureSolarSystemManager();

    UPlanetMovementComponent::FPlanetFrame Frame;
    if (!BuildPlanetFrame(Frame))
        return;

    const FVector& Up = Frame.Up;

	FVector MoveDir = ReadAIMoveDirOnTangent(Up);

    FHitResult GroundHit;

    bGrounded = CheckGrounded(Up, 1.f, GroundHit);

    const float MaxSpeed = GetMaxSpeed() * 2;
    const float MaxAccel = GetMaxAcceleration();

    MoveCapsuleAndResolveCollisions(Up, DeltaTime, GroundHit, Frame);

    FVector TangentVel = FVector::VectorPlaneProject(Velocity, Up);

    const FVector Accel = ComputeInputAcceleration(MoveDir, TangentVel, MaxAccel, MaxSpeed, DeltaTime);
    Velocity += Accel * DeltaTime;

    if (bGrounded)
    {
        ClampGroundTangentSpeed(Up, MaxSpeed);
    }

    AlignCharacterToSurface(Up, DeltaTime);
}

FVector UMyEnemyMovementComponent::ReadAIMoveDirOnTangent(const FVector& Up) const
{
    FVector Input = AIInput.GetClampedToMaxSize(1.0f);
    FVector MoveDir = FVector::VectorPlaneProject(AIInput, Up);
    return MoveDir.GetSafeNormal();
}