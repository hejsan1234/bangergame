// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlanetActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "SolarSystemsManager.generated.h"

class ADirectionalLight;
class AM_Skysphere;

UCLASS()
class MITTSPEL_API ASolarSystemManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<AAPlanetActor*> Bodies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	AAPlanetActor* AnchorBody = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Anchor")
	AAPlanetActor* RequestedAnchorBody = nullptr;

	void RequestAnchor(AAPlanetActor* Body);
	void ClearAnchorRequest();
	ASolarSystemManager();

	UFUNCTION(BlueprintPure)
	FVector GetAnchorSimPos() const { return AnchorSimPos; }

	UPROPERTY(BlueprintReadOnly, Category = "Gravity")
	AAPlanetActor* ActiveGravityBody = nullptr;

	UFUNCTION(BlueprintPure, Category = "Gravity")
	AAPlanetActor* GetActiveGravityBody() const { return ActiveGravityBody; }

	UPROPERTY(EditAnywhere, Category = "World Shift")
	AM_Skysphere* SkySphereActor = nullptr;

	// --- Lighting ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	ADirectionalLight* SunDirectionalLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	AAPlanetActor* SunBody = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	bool bLightTargetsAnchor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float LightYawOffsetDeg = 0.f;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateSunLightDirection();

	FVector AnchorSimPos = FVector::ZeroVector;
	FVector PrevAnchorSimPos = FVector::ZeroVector;
	AAPlanetActor* PrevAnchorBody = nullptr;
	float PrevSpin = 0.f;
	float PrevOrbit = 0.f;
};

