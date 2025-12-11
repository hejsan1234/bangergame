// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlanetActor.generated.h"

UCLASS()
class MITTSPEL_API AAPlanetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAPlanetActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	float GravityStrength = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Properties")
	float PlanetRadius = 500.0f;

	FVector GetGravityDirection(const FVector& Location) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
