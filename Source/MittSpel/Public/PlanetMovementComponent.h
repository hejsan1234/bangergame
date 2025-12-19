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

	bool CheckGrounded(const FVector& Up, float ProbeDistance, FHitResult& OutHit);

private:
	bool bAnchoredToPlanet = false;
	float bPrev = 0;
	float AnchorEnterMargin = 5000.f;
	float AnchorExitMargin = 5000.f;
};
