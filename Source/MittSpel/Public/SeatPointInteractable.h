// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "SeatPointInteractable.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ASpaceship;
class AMyCharacter;

UCLASS()
class MITTSPEL_API ASeatPointInteractable : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASeatPointInteractable();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* InteractionMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Setup")
	ASpaceship* OwningShip;

public:	
	virtual void Interact_Implementation(AMyCharacter* Player) override;
	virtual bool CanInteract_Implementation(AMyCharacter* Player) const override;
	virtual FText GetInteractText_Implementation() const override;
};
