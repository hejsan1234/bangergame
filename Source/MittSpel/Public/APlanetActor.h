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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	FVector OrbitAxis = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float PhaseDeg = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	bool bEnableOrbit = false;

	// Spawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	USceneComponent* SpawnPoint;

	UFUNCTION(BlueprintPure, Category = "Spawn")
	FTransform GetSpawnTransform() const;


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateOrbit(float DeltaTime);
	float OrbitAngleDeg = 0.f;
	void UpdateSpawnPoint();
};
