#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "ShipDoorInteractable.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ASpaceship;
class AMyCharacter;

UCLASS()
class MITTSPEL_API AShipDoorInteractable : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AShipDoorInteractable();

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