#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlanetActor.generated.h"

class UStaticMeshComponent;

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

protected:
	virtual void BeginPlay() override;
};
