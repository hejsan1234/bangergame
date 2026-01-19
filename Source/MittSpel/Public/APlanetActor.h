#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlanetActor.generated.h"

class UStaticMeshComponent;
class AAPlanetActor;

UCLASS()
class MITTSPEL_API AAPlanetActor : public AActor
{
	GENERATED_BODY()

public:
	AAPlanetActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	float GravityStrength = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	float GravityRange = 50000.f; // cm

	// Valfritt: låt dig override:a radie manuellt vid behov
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	bool bOverridePlanetRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties", meta = (EditCondition = "bOverridePlanetRadius"))
	float PlanetRadius = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	UStaticMeshComponent* PlanetMesh;

	UFUNCTION(BlueprintCallable, Category = "Planet Properties")
	float GetPlanetRadiusWS() const;

	UFUNCTION(BlueprintCallable, Category = "Planet Properties")
	FVector GetGravityDirection(const FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Properties")
	UStaticMeshComponent* GetPlanetMesh() const { return PlanetMesh; }

	// Orbiting properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	AAPlanetActor* ParentBody = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitRadius = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitSpeedDegPerSec = 5.f;

	UFUNCTION(BluePrintCallable, Category = "Orbit")
	float GetOrbitSpeedDegPerSec() const { return OrbitSpeedDegPerSec; }

	UFUNCTION(BluePrintCallable, Category = "Orbit")
	void SetOrbitSpeedDegPerSec(float NewOrbitSpeed) { OrbitSpeedDegPerSec = NewOrbitSpeed; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	FVector OrbitAxis = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float PhaseDeg = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	bool bEnableOrbit = false;

	// Spinning properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spin")
	float SpinSpeedDegPerSec = 10.f;

	UFUNCTION(BluePrintCallable, Category = "spin")
	float GetSpinDegPerSec() const { return SpinSpeedDegPerSec; }

	UFUNCTION(BluePrintCallable, Category = "spin")
	void SetSpinSpeedDegPerSec(float NewSpinSpeed) { SpinSpeedDegPerSec = NewSpinSpeed; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin")
	float SpinAngleDeg = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin")
	FQuat SimRot = FQuat::Identity;

	UFUNCTION(BlueprintCallable, Category = "Spin")
	void UpdateSpin(float DeltaTime);

	// Spawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	USceneComponent* SpawnPoint;

	UFUNCTION(BlueprintPure, Category = "Spawn")
	FTransform GetSpawnTransform() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation")
	FVector SimPos = FVector::ZeroVector;

	UFUNCTION(BlueprintPure, Category = "Simulation")
	FVector GetCenterInFrame(const FVector& AnchorSimPos) const
	{
		return SimPos - AnchorSimPos;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateOrbit(float DeltaTime);
	float OrbitAngleDeg = 0.f;
	void UpdateSpawnPoint();
};
