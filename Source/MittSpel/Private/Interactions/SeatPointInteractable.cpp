// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactions/SeatPointInteractable.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Ships/Spaceship.h"
#include "Characters/MyCharacter.h"

// Sets default values
ASeatPointInteractable::ASeatPointInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InteractionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionMesh"));
	InteractionMesh->SetupAttachment(Root);

	InteractionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ASeatPointInteractable::Interact_Implementation(AMyCharacter* Player)
{
	if (!OwningShip)
	{
		UE_LOG(LogTemp, Warning, TEXT("SeatPointInteractable has no OwningShip"));
		return;
	}

	OwningShip->SitInSeat(Player);
}

bool ASeatPointInteractable::CanInteract_Implementation(AMyCharacter* Player) const
{
	return OwningShip != nullptr;
}

FText ASeatPointInteractable::GetInteractText_Implementation() const
{
	return FText::FromString(TEXT("Sitt Down"));
}

