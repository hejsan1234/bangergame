// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include "SolarSystemsManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h" // Add this include to resolve the incomplete type error for ACharacter
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacter.h"

// Add this as a member variable in your UPlanetMovementComponent class (in the header file, e.g., PlanetMovementComponent.h):

UPlanetMovementComponent::UPlanetMovementComponent()
{
    PlanetGravityScale = 1.0f;
}

bool UPlanetMovementComponent::EnsureSolarSystemManager()
{
    if (IsValid(SolarSystemManager))
        return true;

    SolarSystemManager = Cast<ASolarSystemManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ASolarSystemManager::StaticClass())
    );

    return IsValid(SolarSystemManager);
}

AAPlanetActor* UPlanetMovementComponent::GetActivePlanet() const
{
    return IsValid(SolarSystemManager)
        ? SolarSystemManager->GetActiveGravityBody()
        : nullptr;
}

bool UPlanetMovementComponent::EnsureMovementPrereqs(float DeltaTime, int32 Iterations)
{
    EnsureSolarSystemManager();
    AMyCharacter* Mychar = Cast<AMyCharacter>(CharacterOwner);
    AAPlanetActor* GravPlanet = GetActivePlanet();
    if (!GravPlanet)
    {
        if (Mychar) Mychar->SetControlMode(EControlMode::Space);
        PhysFree(DeltaTime, Iterations);
        return false;
    }

    Planet = GravPlanet;

    if (!CharacterOwner || !UpdatedComponent || !Planet)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlanetMovementComponent: Missing required references, falling back to default physics."));
        PhysFalling(DeltaTime, Iterations);
        return false;
    }

    return true;
}

bool UPlanetMovementComponent::BuildPlanetFrame(FPlanetFrame& OutFrame) const
{
    OutFrame.AnchorSim = IsValid(SolarSystemManager)
        ? SolarSystemManager->GetAnchorSimPos()
        : FVector::ZeroVector;

    OutFrame.Center = Planet->GetCenterInFrame(OutFrame.AnchorSim);
    OutFrame.Pos = UpdatedComponent->GetComponentLocation();

    //UE_LOG(LogTemp, Warning, TEXT("POS: %s"), *OutFrame.Pos.ToString());

    OutFrame.ToCenter = OutFrame.Center - OutFrame.Pos;
    OutFrame.Distance = OutFrame.ToCenter.Size();

    OutFrame.Surface = Planet->GetPlanetRadiusWS();
    OutFrame.Altitude = OutFrame.Distance - OutFrame.Surface;

    if (OutFrame.Distance < KINDA_SMALL_NUMBER)
        return false;

    OutFrame.DirToCenter = OutFrame.ToCenter / OutFrame.Distance;

    OutFrame.Up = -OutFrame.DirToCenter;

    return true;
}

FVector UPlanetMovementComponent::ReadMoveDirOnTangent(const FVector& Up) const
{
    // 1) Läs input
    FVector Input = GetLastInputVector();
    Input = Input.GetClampedToMaxSize(1.0f);

    // 2) Projekt på tangentplanet
    FVector MoveDir = FVector::VectorPlaneProject(Input, Up);
    return MoveDir.GetSafeNormal();
}

void UPlanetMovementComponent::MoveCapsuleAndResolveCollisions(const FVector& Up, float DeltaTime, FHitResult GroundHit, FPlanetFrame Frame)
{
    FVector Delta = Velocity * DeltaTime;
    FHitResult Hit;

    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        const float Dot = FVector::DotProduct(Hit.Normal, Up);
        if (Dot > 0.7f)
        {
        }

        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }

    if (bGrounded && JumpGroundIgnoreTime <= 0.f)
    {
        const FVector N = GroundHit.Normal.GetSafeNormal();

        // 1) Ta bort all vel in/ut från marken
        Velocity = FVector::VectorPlaneProject(Velocity, N);

        // 2) Snap om vi är ovanför
        const float DesiredGap = 2.f;

        //VID BUGG KAN BYTA TILL DETTA DÅ DET KANSKE ÄR MER EXAKT
  //      const float Dist = FVector::DotProduct(UpdatedComponent->GetComponentLocation() - GroundHit.Location, N);
		//UE_LOG(LogTemp, Warning, TEXT("Dist to ground: %f"), Dist);

		const float Dist = GroundHit.Distance;

        const float Error = Dist - DesiredGap;

        if (Error > 0.f)
        {
            FHitResult SnapHit;
            SafeMoveUpdatedComponent(-N * Error, UpdatedComponent->GetComponentQuat(), true, SnapHit);
        }
    }
    else {
		// Apply gravity when not grounded
        const FVector& DirToCenter = Frame.DirToCenter;
        const float GravityStrength = Planet->GravityStrength;
        Velocity += DirToCenter * GravityStrength * PlanetGravityScale * DeltaTime;
    }
}

FVector UPlanetMovementComponent::ComputeInputAcceleration(
    const FVector& MoveDir,
    const FVector& TangentVel,
    float MaxAccel,
    float MaxSpeed,
    float DeltaTime
) const
{
    FVector Accel = FVector::ZeroVector;

    if (MoveDir.IsNearlyZero())
        return Accel;

    const float TangentSpeed = TangentVel.Size();
    const bool bIsBrakingInput =
        (TangentSpeed > KINDA_SMALL_NUMBER) &&
        (FVector::DotProduct(MoveDir, TangentVel.GetSafeNormal()) < 0.0f);

    if (!bGrounded)
    {
        const float AirAccelScale = 0.2f;
        const float AirBrakeScale = 0.2f;

        const FVector TestVel = TangentVel + (MoveDir * MaxAccel * AirAccelScale * DeltaTime);
        const float TestSpeed = TestVel.Size();

        if (bIsBrakingInput)
        {
            Accel = MoveDir * MaxAccel * AirBrakeScale;
        }
        else if (TestSpeed < MaxSpeed)
        {
            Accel = MoveDir * MaxAccel * AirAccelScale;
        }
    }
    else
    {
        if (bIsBrakingInput)
        {
            Accel = MoveDir * MaxAccel * 3;
        }
        else {
            Accel = MoveDir * MaxAccel;
        }
    }

    return Accel;
}

void UPlanetMovementComponent::ClampGroundTangentSpeed(const FVector& Up, float MaxSpeed)
{
    FVector TangentVel2 = FVector::VectorPlaneProject(Velocity, Up);
    FVector RadialVel2 = Velocity - TangentVel2;

    const float TangentSpeed = TangentVel2.Size();
    if (TangentSpeed > MaxSpeed)
    {
        TangentVel2 *= (MaxSpeed / TangentSpeed);
        Velocity = TangentVel2 + RadialVel2;
    }
}

void UPlanetMovementComponent::ApplyNoInputBraking(
    const FVector& MoveDir,
    const FVector& Up,
    float DeltaTime,
    FVector& InOutTangentVel
)
{
    // ersätter båda dina block: om ingen input, bromsa tangent-velocity men behåll radial-del
    if (!MoveDir.IsNearlyZero())
        return;

    const FVector RadialVel = Velocity - InOutTangentVel;

    Velocity = InOutTangentVel;

    if (bGrounded)
    {
        ApplyVelocityBraking(DeltaTime, BrakingFriction, BrakingDecelerationWalking);
        const float StopSpeed = 2.f;
        if (Velocity.SizeSquared() < FMath::Square(StopSpeed))
        {
            Velocity = FVector::ZeroVector;
        }
    }
    else
    {
        ApplyVelocityBraking(DeltaTime, BrakingFriction, BrakingDecelerationFlying);
    }

    InOutTangentVel = Velocity;
    Velocity = InOutTangentVel + RadialVel;
}

void UPlanetMovementComponent::TryApplyPlanetJump(const FVector& Up, float GravityStrength)
{
    AMyCharacter* MyChar = Cast<AMyCharacter>(CharacterOwner);
    if (!MyChar || !MyChar->IsPlanetJumping() || !bGrounded)
        return;

    float JumpHeight = CharacterOwner->GetCharacterMovement()->JumpZVelocity;
    float JumpVelocityMagnitude = FMath::Sqrt(2.f * GravityStrength * PlanetGravityScale * JumpHeight);

    Velocity += Up * JumpVelocityMagnitude;
	JumpGroundIgnoreTime = 0.12f;
}

void UPlanetMovementComponent::AlignCharacterToSurface(const FVector& Up, float DeltaTime)
{
    const FVector UpN = Up.GetSafeNormal();

    FVector Forward = FVector::VectorPlaneProject(
        CharacterOwner->GetActorForwardVector(),
        UpN
    );

    if (Forward.IsNearlyZero())
    {
        Forward = FVector::VectorPlaneProject(
            CharacterOwner->GetActorForwardVector(),
            UpN
        );
    }

    Forward = Forward.GetSafeNormal();

    const FQuat TargetQuat =
        FRotationMatrix::MakeFromXZ(Forward, UpN).ToQuat();

    const FQuat CurrentQuat = CharacterOwner->GetActorQuat();

    const float Speed = 12.f;
    const float Alpha = FMath::Clamp(Speed * DeltaTime, 0.f, 1.f);

    const FQuat NewQuat =
        FQuat::Slerp(CurrentQuat, TargetQuat, Alpha).GetNormalized();

    CharacterOwner->SetActorRotation(NewQuat);
}

void UPlanetMovementComponent::UpdateAnchorStateMachine(float Altitude, float DeltaTime)
{
    bPrev += DeltaTime;

    if (!IsValid(SolarSystemManager) || bPrev <= 0.05f)
        return;

    bPrev = 0.f;

    if (!bAnchoredToPlanet)
    {
        if (bGrounded || Altitude < AnchorEnterMargin)
        {
            bAnchoredToPlanet = true;
            SolarSystemManager->RequestAnchor(Planet);
        }
    }
    else
    {
        if (Altitude > AnchorExitMargin)
        {
            bAnchoredToPlanet = false;
            SolarSystemManager->ClearAnchorRequest();
        }
    }
}


void UPlanetMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (!EnsureMovementPrereqs(DeltaTime, Iterations))
        return;

    //bGrounded = false;

    const FVector Pos = UpdatedComponent->GetComponentLocation();

    //UE_LOG(LogTemp, Warning, TEXT("Character position: %s"), *Pos.ToString());
    //UE_LOG(LogTemp, Warning, TEXT("bGrounded: %d"), bGrounded);

    AMyCharacter* MyChar = Cast<AMyCharacter>(CharacterOwner);

    FPlanetFrame Frame;
    if (!BuildPlanetFrame(Frame))
        return;

	const FVector& DirToCenter = Frame.DirToCenter;
	const FVector& Up = Frame.Up;
	const float Altitude = Frame.Altitude;

    UpdateAnchorStateMachine(Altitude, DeltaTime);

    if (!bAnchoredToPlanet) {
        if (MyChar) MyChar->SetControlMode(EControlMode::Space);
        PhysFree(DeltaTime, Iterations);
        return;
    }
    else {
        if (MyChar) MyChar->SetControlMode(EControlMode::Planet);
    }

    JumpGroundIgnoreTime = FMath::Max(0.f, JumpGroundIgnoreTime - DeltaTime);

    FHitResult GroundHit;

    if (JumpGroundIgnoreTime > 0.f)
    {
        bGrounded = false;
    }
    else {
        bGrounded = CheckGrounded(Up, 1.f, GroundHit);
    }
	//UE_LOG(LogTemp, Warning, TEXT("bOnGround: %s"), bOnGround ? TEXT("true") : TEXT("false"));

    FVector MoveDir = ReadMoveDirOnTangent(Up);

    //Gravitation
    const float MaxSpeed = GetMaxSpeed();
    const float MaxAccel = GetMaxAcceleration();

	const float GravityStrength = Planet->GravityStrength;

    MoveCapsuleAndResolveCollisions(Up, DeltaTime, GroundHit, Frame);

    FVector TangentVel = FVector::VectorPlaneProject(Velocity, Up);

    const FVector Accel = ComputeInputAcceleration(MoveDir, TangentVel, MaxAccel, MaxSpeed, DeltaTime);
    Velocity += Accel * DeltaTime;

    if (bGrounded)
    {
        ClampGroundTangentSpeed(Up, MaxSpeed);
    }

    TangentVel = FVector::VectorPlaneProject(Velocity, Up);

    ApplyNoInputBraking(MoveDir, Up, DeltaTime, TangentVel);
    //UE_LOG(LogTemp, Warning, TEXT("Velocity efter ApplyNoInputBraking: %f"), Velocity.Size());

    TryApplyPlanetJump(Up, GravityStrength);

    AlignCharacterToSurface(Up, DeltaTime);
}

void UPlanetMovementComponent::PhysFree(float DeltaTime, int32 Iterations)
{
    const FVector MoveDir = GetLastInputVector().GetClampedToMaxSize(1.f);


    const float MaxSpeed = GetMaxSpeed() * 1000;
    const float MaxAccel = GetMaxAcceleration();

    const FVector Accel = ComputeInputAcceleration(MoveDir, Velocity, MaxAccel, MaxSpeed, DeltaTime);
    Velocity += Accel * DeltaTime;

    const float Speed = Velocity.Size();
    if (Speed > MaxSpeed)
        Velocity *= (MaxSpeed / Speed);

    FHitResult Hit;
    const FVector Delta = Velocity * DeltaTime;
    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
        Velocity = FVector::VectorPlaneProject(Velocity, Hit.Normal);
    }

    //UE_LOG(LogTemp, Warning, TEXT("Free speed: %f"), Velocity.Size());
}

bool UPlanetMovementComponent::CheckGrounded(
    const FVector& Up,
    float ProbeDistance,
    FHitResult& OutHit
)
{
    if (!CharacterOwner || !UpdatedComponent) return false;

    UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
    if (!Capsule) return false;

    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    const float Radius = Capsule->GetScaledCapsuleRadius();

    const float StartOffset = 35.f; // testa 1,2,5
    const FVector Start = UpdatedComponent->GetComponentLocation() - Up * (HalfHeight - StartOffset);
    const FVector End = Start - Up * 10.f;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CharacterOwner);


    // Kanske lägg minska från radius till 2.f
    const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, 1.f);

    const FQuat Rot = UpdatedComponent->GetComponentQuat();

    const bool bHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, ECC_WorldStatic, Shape, Params);
    if (!bHit) return false;

    return FVector::DotProduct(OutHit.Normal, Up) > 0.7f && OutHit.Distance < 4.f;
}
