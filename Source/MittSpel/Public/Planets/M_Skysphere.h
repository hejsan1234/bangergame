// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "APlanetActor.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M_Skysphere.generated.h"

UCLASS()
class MITTSPEL_API AM_Skysphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AM_Skysphere();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	AAPlanetActor* ActiveBody = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	UStaticMeshComponent* SphereMesh;

	void SetActiveBody(AAPlanetActor* NewActiveBody);

	UPROPERTY(EditAnywhere, Category = "Sky")
	float OrbitAngleDeg = 0.f;

	UPROPERTY(EditAnywhere, Category = "Sky")
	float OrbitSpeedDegPerSec = 2.f;

	UFUNCTION(BlueprintCallable, Category = "Sky")
	void SetOrbitSpeedDegPerSec(float NewOrbitSpeed) { OrbitSpeedDegPerSec = NewOrbitSpeed; }

	UPROPERTY(EditAnywhere, Category = "Sky")
	FVector OrbitAxis = FVector(0.f, 0.f, 1.f);
};
