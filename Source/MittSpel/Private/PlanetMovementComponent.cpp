// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h" // Add this include to resolve the incomplete type error for ACharacter
#include "MyCharacter.h"

UPlanetMovementComponent::UPlanetMovementComponent()
{
    PlanetGravityScale = 1.0f;
}

void UPlanetMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{

    if (!CharacterOwner || !UpdatedComponent || !Planet)
    {
        PhysFalling(DeltaTime, Iterations);
        return;
    }

    AMyCharacter* MyChar = Cast<AMyCharacter>(CharacterOwner);

    const FVector Center = Planet->GetActorLocation();
    const FVector Pos = UpdatedComponent->GetComponentLocation();

    FVector ToCenter = Center - Pos;
    float Distance = ToCenter.Size();
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
	Velocity += DirToCenter * GravityStrength * PlanetGravityScale * DeltaTime;
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
    else {
		bGrounded = false;
    }

    FVector TangentVel = FVector::VectorPlaneProject(Velocity, Up);

    if (bGrounded)
    {
        Velocity = FVector::VectorPlaneProject(Velocity, Up);
    }

    // 3) Acceleration från input
    FVector Accel = FVector::ZeroVector;
    if (!MoveDir.IsNearlyZero())
    {
        FVector tempVelocity = Velocity;
        FVector tempAccel = Accel;
        
        //TangentSpeed < MaxSpeed
        if (!bGrounded) {
            tempAccel = MoveDir * MaxAccel; // mindre kontroll i luften
			tempVelocity += tempAccel * DeltaTime;
            if (tempVelocity.Size() < Velocity.Size()) {
                Accel = MoveDir * MaxAccel * 0.2f;
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
	UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"), *DebugVel.ToString());

    // 5) Bromsa om ingen inpup
    FVector RadialVel = Velocity - TangentVel;
    if (bGrounded && MoveDir.IsNearlyZero())
    {
        const FVector OldVel = Velocity;
        Velocity = TangentVel;
        ApplyVelocityBraking(DeltaTime, BrakingFriction, BrakingDecelerationWalking);
        TangentVel = Velocity;

        // sätt tillbaka radial
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
}