// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include "SolarSystemsManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h" // Add this include to resolve the incomplete type error for ACharacter
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacter.h"

// Add this as a member variable in your UPlanetMovementComponent class (in the header file, e.g., PlanetMovementComponent.h):

UPlanetMovementComponent::UPlanetMovementComponent()
{
    PlanetGravityScale = 1.0f;
}

void UPlanetMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (!IsValid(SolarSystemManager))
    {
        SolarSystemManager = Cast<ASolarSystemManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ASolarSystemManager::StaticClass())
        );
    }

    AAPlanetActor* GravPlanet = IsValid(SolarSystemManager)
        ? SolarSystemManager->GetActiveGravityBody()
        : nullptr;

    if (!GravPlanet)
    {
        return;
    }

    if (!CharacterOwner || !UpdatedComponent || !Planet)
    {
		UE_LOG(LogTemp, Warning, TEXT("PlanetMovementComponent: Missing required references, falling back to default physics."));
        PhysFalling(DeltaTime, Iterations);
        return;
    }

    bGrounded = false;

    AMyCharacter* MyChar = Cast<AMyCharacter>(CharacterOwner);

    const FVector AnchorSim = IsValid(SolarSystemManager)
        ? SolarSystemManager->GetAnchorSimPos()
        : FVector::ZeroVector;

    const FVector Center = Planet->GetCenterInFrame(AnchorSim);
    const FVector Pos = UpdatedComponent->GetComponentLocation();

    FVector ToCenter = Center - Pos;
    float Distance = ToCenter.Size();

    const float Surface = Planet->GetPlanetRadiusWS();
    const float Altitude = Distance - Surface;

    if (Distance < KINDA_SMALL_NUMBER)
        return;

    FVector DirToCenter = ToCenter / Distance;
    FVector Up = -DirToCenter;

    // 1) Läs input
    FVector Input = GetLastInputVector();
    Input = Input.GetClampedToMaxSize(1.0f);

    // 2) Projekt på tangentplanet
    FVector MoveDir = FVector::VectorPlaneProject(Input, Up);
    MoveDir = MoveDir.GetSafeNormal();

    const float MaxSpeed = GetMaxSpeed();
    const float MaxAccel = GetMaxAcceleration();

	const float GravityStrength = Planet->GravityStrength;

    if (!bGrounded) {
        Velocity += DirToCenter * GravityStrength * PlanetGravityScale * DeltaTime;
    }
	// print out velocity for debugging

    // 7) Flytta kapseln
    FVector Delta = Velocity * DeltaTime;
    FHitResult Hit;

    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        const float Dot = FVector::DotProduct(Hit.Normal, Up);
        if (Dot > 0.7f)
        {
			bGrounded = true;
        }
        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }

    if (bGrounded)
    {
        const float StickSpeed = 30.f;

        Velocity = FVector::VectorPlaneProject(Velocity, Up)
            - Up * StickSpeed;
    }

    FVector TangentVel = FVector::VectorPlaneProject(Velocity, Up);

    // 3) Acceleration från input
    FVector Accel = FVector::ZeroVector;
    if (!MoveDir.IsNearlyZero())
    {
        if (!bGrounded)
        {
            const float AirAccelScale = 0.5f;
            const float AirBrakeScale = 0.5f;

            const float TangentSpeed = TangentVel.Size();

            const bool bIsBrakingInput = (TangentSpeed > KINDA_SMALL_NUMBER) && (FVector::DotProduct(MoveDir, TangentVel.GetSafeNormal()) < 0.0f);
            const FVector TestVel = TangentVel + (MoveDir * MaxAccel * AirAccelScale * DeltaTime);
            const float TestSpeed = TestVel.Size();

            if (bIsBrakingInput)
            {
                Accel = MoveDir * MaxAccel * AirBrakeScale;
            }
            else if (TestSpeed < MaxSpeed) {
                Accel = MoveDir * MaxAccel * AirAccelScale;
            }
        }
        else {
            Accel = MoveDir * MaxAccel;
        }
    }

    // 4) Uppdatera Velocity
    Velocity += Accel * DeltaTime;

    if (bGrounded)
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

	FVector DebugVel = Velocity * DeltaTime;

    // 5) Bromsa om ingen inpup
    FVector RadialVel = Velocity - TangentVel;
    if (bGrounded && MoveDir.IsNearlyZero())
    {
        const FVector OldVel = Velocity;
        Velocity = TangentVel;
        ApplyVelocityBraking(DeltaTime, BrakingFriction, BrakingDecelerationWalking);
        TangentVel = Velocity;
        Velocity = TangentVel + RadialVel;
    }

    if (!bGrounded && MoveDir.IsNearlyZero())
    {
        const FVector OldVel = Velocity;
        Velocity = TangentVel;
        ApplyVelocityBraking(DeltaTime, BrakingFriction, BrakingDecelerationFlying);
        TangentVel = Velocity;
        Velocity = TangentVel + RadialVel;
	}

    // 6) Clampa toppfart

    if (MyChar && MyChar->IsPlanetJumping() && bGrounded)
    {
        float JumpHeight = CharacterOwner->GetCharacterMovement()->JumpZVelocity;
        float JumpVelocityMagnitude = FMath::Sqrt(2 * GravityStrength * PlanetGravityScale * JumpHeight);
        Velocity += Up * JumpVelocityMagnitude;
    }

	//FVector LookDir = CharacterOwner->GetControlRotation().Vector();

// Ta bort grav-komponenten så vi bara får rörelse längs ytan
    TangentVel = FVector::VectorPlaneProject(Velocity, Up);

	FVector Forward = FVector::VectorPlaneProject(GetCharacterOwner()->GetActorForwardVector(), Up).GetSafeNormal();

    // om vi står nästan still: behåll nuvarande forward men se till att den ligger på tangentplanet
    if (Forward.IsNearlyZero())
    {
        Forward = FVector::VectorPlaneProject(CharacterOwner->GetActorForwardVector(), Up).GetSafeNormal();
    }

    const FRotator TargetRot = FRotationMatrix::MakeFromXZ(Forward, Up).Rotator();
    const FRotator NewRot = FMath::RInterpTo(CharacterOwner->GetActorRotation(), TargetRot, DeltaTime, 12.f);
    CharacterOwner->SetActorRotation(NewRot);

    //Sätt planetAnkare
    
	bPrev += DeltaTime;
    if (IsValid(SolarSystemManager) && bPrev > 0.05)
    {
		bPrev = 0;
        if (!bAnchoredToPlanet)
        {

            if (bGrounded || Altitude < AnchorEnterMargin)
            {
                bAnchoredToPlanet = true;
                SolarSystemManager->RequestAnchor(Planet);
            }
            else
            {
                //SolarSystemManager->SetAnchor(Planet);
                //SolarSystemManager->ClearAnchor();
            }
        }
        else
        {

            if (Altitude > AnchorExitMargin)
            {
                bAnchoredToPlanet = false;
                //SolarSystemManager->SetAnchor(Planet);
                SolarSystemManager->ClearAnchorRequest();
            }
            else
            {
                //SolarSystemManager->SetAnchor(Planet);
            }
        }
    }
}

bool UPlanetMovementComponent::CheckGrounded(
    const FVector& Up,
    float ProbeDistance,
    FHitResult& OutHit
)
{
    if (!CharacterOwner)
        return false;

    UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
    if (!Capsule)
        return false;

    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    const FVector Start = UpdatedComponent->GetComponentLocation() - Up * (HalfHeight - 1.f);
    const FVector End = Start - Up * ProbeDistance;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CharacterOwner);

    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        Start,
        End,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeCapsule(
            Capsule->GetScaledCapsuleRadius(),
            Capsule->GetScaledCapsuleHalfHeight()
        ),
        Params
    );

    if (!bHit)
        return false;

    const float Dot = FVector::DotProduct(OutHit.Normal, Up);
    return Dot > 0.7f;
}
