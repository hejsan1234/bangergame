// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMovementComponent.h"
#include "APlanetActor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h" // Add this include to resolve the incomplete type error for ACharacter

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

	//FVector LookDir = CharacterOwner->GetControlRotation().Vector();
	//LookDir = FVector::VectorPlaneProject(LookDir, Up).GetSafeNormal();

 //   if (LookDir.IsNearlyZero())
 //   {
 //       LookDir = FVector::VectorPlaneProject(CharacterOwner->GetActorForwardVector(), Up).GetSafeNormal();
	//}

	//const FRotator TargetRot = FRotationMatrix::MakeFromXZ(LookDir, Up).Rotator();
	//const FRotator NewRot = FMath::RInterpTo(CharacterOwner->GetActorRotation(), TargetRot, DeltaTime, 12.0f);

	//CharacterOwner->SetActorRotation(NewRot);

    //Karaktär riktning och upp-vektor

	//static bool bDidRotate = false;
 //   if (!bDidRotate) {
	//	FRotator Rot = CharacterOwner->GetActorRotation();
	//	UE_LOG(LogTemp, Warning, TEXT("Before Adjusting Actor Rotation: %s"), *Rot.ToString());
	//	Rot.Yaw += 90.0f * DeltaTime; // Justera yaw med 90 grader
	//	CharacterOwner->SetActorRotation(Rot);
	//	FRotator NewRot = CharacterOwner->GetActorRotation();
	//	UE_LOG(LogTemp, Warning, TEXT("After Adjusting Actor Rotation: %s"), *NewRot.ToString());
	//	//bDidRotate = true;
 //   }

	//UE_LOG(LogTemp, Warning, TEXT("Actor Rotation: %s"), *NewRot.ToString());

    // Gravitation mot planetens centrum

    // --- ROTERA GUBBEN EFTER RÖRELSERIKTNING (utan mus) ---

// Ta bort grav-komponenten så vi bara får rörelse längs ytan
    FVector TangentVel = FVector::VectorPlaneProject(Velocity, Up);

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