#include "Interactions/ShipDoorInteractable.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Ships/Spaceship.h"
#include "Characters/MyCharacter.h"

AShipDoorInteractable::AShipDoorInteractable()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    InteractionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionMesh"));
    InteractionMesh->SetupAttachment(Root);

    InteractionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AShipDoorInteractable::Interact_Implementation(AMyCharacter* Player)
{
    if (!OwningShip)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShipDoorInteractable has no OwningShip"));
        return;
    }

    OwningShip->OpenDoor();
}

bool AShipDoorInteractable::CanInteract_Implementation(AMyCharacter* Player) const
{
    return OwningShip != nullptr;
}

FText AShipDoorInteractable::GetInteractText_Implementation() const
{
    return FText::FromString(TEXT("Open Door"));
}