// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveComponent.h"
#include "InteractableInterface.h"
#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UInteractiveComponent::UInteractiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TraceForInteractable();
}

AMyCharacter* UInteractiveComponent::GetPlayerCharacter() const
{
	return Cast<AMyCharacter>(GetOwner());
}

void UInteractiveComponent::TraceForInteractable()
{
	AMyCharacter* Player = GetPlayerCharacter();
	if (!Player) {
		CurrentInteractableActor = nullptr;
		return;
	}

	UCameraComponent* Camera = Player->GetCameraComponent();
	if (!Camera) {
		CurrentInteractableActor = nullptr;
		return;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.f, 0, 1.f);

	if (!bHit || !Hit.GetActor()) {
		CurrentInteractableActor = nullptr;
		return;
	}

	AActor* HitActor = Hit.GetActor();

	if (HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		bool bCanInteract = IInteractableInterface::Execute_CanInteract(HitActor, Player);
		CurrentInteractableActor = bCanInteract ? HitActor : nullptr;
	}
	else
	{
		CurrentInteractableActor = nullptr;
	}
}

void UInteractiveComponent::TryInteract()
{
	AMyCharacter* Player = GetPlayerCharacter();
	if (!Player) {
		CurrentInteractableActor = nullptr;
		return;
	}

	if (!CurrentInteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("No interactable actor in focus"));
		return;
	}


	if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		IInteractableInterface::Execute_Interact(CurrentInteractableActor, Player);
	}
}

