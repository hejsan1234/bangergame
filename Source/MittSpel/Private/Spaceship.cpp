// Fill out your copyright notice in the Description page of Project Settings.


#include "Spaceship.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ASpaceship::ASpaceship()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    ShipMesh->SetupAttachment(Root);
    ShipMesh->SetMobility(EComponentMobility::Movable);

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(ShipMesh);
    DoorMesh->SetMobility(EComponentMobility::Movable);

    SeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPoint"));
    SeatPoint->SetupAttachment(ShipMesh);
}

// Called when the game starts or when spawned
void ASpaceship::BeginPlay()
{
	Super::BeginPlay();
	
    DoorClosedLocation = DoorMesh->GetRelativeLocation();
    DoorOpenLocation = DoorClosedLocation + DoorOpenOffset;
}


void ASpaceship::OpenDoor()
{
    if (bDoorOpen)
    {
        bDoorOpen = false;

        DoorMesh->SetRelativeLocation(DoorClosedLocation);
        DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        UE_LOG(LogTemp, Warning, TEXT("Ship door Closed"));
        return;
    }

    bDoorOpen = true;

    DoorMesh->SetRelativeLocation(DoorOpenLocation);
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Warning, TEXT("Ship door opened"));
}

void ASpaceship::SitInSeat(AMyCharacter* Player)
{
    if (!Player || !SeatPoint) {
        UE_LOG(LogTemp, Warning, TEXT("Cannot sit in seat: Player or SeatPoint is null"));
		return;
    }

    if (isSeated) {
        isSeated = false;
        Player->SetActorLocation(SeatPoint->GetComponentLocation());
        Player->SetActorRotation(SeatPoint->GetComponentRotation());

        if (UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement())
        {
            Player->SetIsSeated(false);
        }

        Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        return;
	}

    isSeated = true;

	Player->SetActorLocation(SeatPoint->GetComponentLocation());
	Player->SetActorRotation(SeatPoint->GetComponentRotation());

    if (UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement())
    {
		Player->SetIsSeated(true);
	}

	Player->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
}

