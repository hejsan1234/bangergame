// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SolarSystemsManager.h"
#include "PlanetMovementComponent.generated.h"

class AAPlanetActor;

/**
 * 
 */
UCLASS()
class MITTSPEL_API UPlanetMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UPlanetMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	AAPlanetActor* Planet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float SurfaceEpsilon = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float PlanetGravityScale = 1.0f;
	bool bGrounded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	ASolarSystemManager* SolarSystemManager = nullptr;


protected:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void PhysFree(float deltaTime, int32 Iterations);

	bool CheckGrounded(const FVector& Up, float ProbeDistance, FHitResult& OutHit);

	//static void DrawCharacterDebug(
	//	UWorld* World,
	//	ACharacter* Char,
	//	const FVector& FrameUp,
	//	const FVector& FrameCenter
	//);

private:
	bool bAnchoredToPlanet = false;
	float JumpGroundIgnoreTime = 0.f;
	float bPrev = 0;
	float AnchorEnterMargin = 8000.f;
	float AnchorExitMargin = 8000.f;

	bool EnsureSolarSystemManager();
	AAPlanetActor* GetActivePlanet() const;
	bool EnsureMovementPrereqs(float DeltaTime, int32 Iterations);

	struct FPlanetFrame
	{
		FVector AnchorSim = FVector::ZeroVector;
		FVector Center = FVector::ZeroVector;
		FVector Pos = FVector::ZeroVector;

		FVector ToCenter = FVector::ZeroVector;
		float Distance = 0.f;

		float Surface = 0.f;
		float Altitude = 0.f;

		FVector DirToCenter = FVector::ZeroVector;
		FVector Up = FVector::UpVector;
	};

	bool BuildPlanetFrame(FPlanetFrame& OutFrame) const;

	FVector ReadMoveDirOnTangent(const FVector& Up) const;

	void MoveCapsuleAndResolveCollisions(const FVector& Up, float DeltaTime, FHitResult GroundHit, FPlanetFrame Frame);

	FVector ComputeInputAcceleration(
		const FVector& MoveDir,
		const FVector& TangentVel,
		float MaxAccel,
		float MaxSpeed,
		float DeltaTime
	) const;

	void ClampGroundTangentSpeed(const FVector& Up, float MaxSpeed);

	void ApplyNoInputBraking(
		const FVector& MoveDir,
		const FVector& Up,
		float DeltaTime,
		FVector& InOutTangentVel
	);

	void TryApplyPlanetJump(const FVector& Up, float GravityStrength);
	void AlignCharacterToSurface(const FVector& Up, float DeltaTime);

	void UpdateAnchorStateMachine(float Altitude, float DeltaTime);

	void OnEnterPlanet(const FPlanetFrame& Frame, float DeltaTime);

	FVector PrevUp = FVector::UpVector;
	FVector PrevFwd = FVector::ForwardVector;
	bool bHasFrame = false;
	int FrameAnchoredCount = 0;
	bool bWasAnchoredToPlanet = false;
};
