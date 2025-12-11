// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h" // Add this include to resolve the incomplete type error for ACharacter

void UPlanetMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (!CharacterOwner || !UpdatedComponent || !Planet)
    {
        PhysFalling(DeltaTime, Iterations);
        return;
    }

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

    // 3) Acceleration från input
    FVector Accel = FVector::ZeroVector;
    if (!MoveDir.IsNearlyZero())
    {
        Accel = MoveDir * MaxAccel;
    }

    // 4) Uppdatera Velocity
    Velocity += Accel * DeltaTime;

	const float GravityStrength = Planet->GravityStrength;
	Velocity += DirToCenter * GravityStrength * PlanetGravityScale * DeltaTime;
	// print out velocity for debugging
    UE_LOG(LogTemp, Warning, TEXT("GravityStrength: %s"), *Velocity.ToString());
    UE_LOG(LogTemp, Warning, TEXT("GravityScale: %f"), PlanetGravityScale);

    // 5) Bromsa om ingen input
    if (MoveDir.IsNearlyZero())
    {
        ApplyVelocityBraking(
            DeltaTime,
            BrakingFriction,
            BrakingDecelerationWalking
        );
    }

    // 6) Clampa toppfart
    const float Speed = Velocity.Size();
    if (Speed > MaxSpeed)
    {
        Velocity *= (MaxSpeed / Speed);
    }

    // 7) Flytta kapseln
    FVector Delta = Velocity * DeltaTime;
    FHitResult Hit;
    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }
}