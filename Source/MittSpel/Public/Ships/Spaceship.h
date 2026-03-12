// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spaceship.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AMyCharacter;

UCLASS()
class MITTSPEL_API ASpaceship : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceship();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(EditAnywhere, Category = "Door")
    FVector DoorOpenOffset = FVector(650.f, 0.f, 0.f);

    FVector DoorClosedLocation = FVector::ZeroVector;
    FVector DoorOpenLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SeatPoint;

    bool bDoorOpen = false;
	bool isSeated = false;

public:	
    void OpenDoor();
    void CloseDoor();
	bool IsDoorOpen() const { return bDoorOpen; }

	void SitInSeat(AMyCharacter* Player);
};
