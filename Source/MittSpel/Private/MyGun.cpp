#include "MyGun.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AMyGun::AMyGun()
{
    PrimaryActorTick.bCanEverTick = false;

    GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
    RootComponent = GunMesh;

    GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GunMesh->SetOnlyOwnerSee(true);
    GunMesh->CastShadow = false;
}

void AMyGun::Fire(const FVector& Start, const FVector& Direction)
{
    FVector End = Start + Direction * Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    DrawDebugLine(
        GetWorld(),
        Start,
        bHit ? Hit.Location : End,
        FColor::Red,
        false,
        1.f,
        0,
        1.f
    );

    if (bHit && Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(),
            Damage,
            Direction,
            Hit,
            GetInstigatorController(),
            this,
            nullptr
        );
    }
    
    GunMesh->SetRelativeLocation(FVector(150.f, 30.f, -30.f));
    GunMesh->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
    GunMesh->SetRelativeScale3D(FVector(5.f));
}
