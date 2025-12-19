#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGun.generated.h"

class USkeletalMeshComponent;

UCLASS()
class MITTSPEL_API AMyGun : public AActor
{
    GENERATED_BODY()

public:
    AMyGun();

    void Fire(const FVector& Start, const FVector& Direction);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gun")
    USkeletalMeshComponent* GunMesh;

    UPROPERTY(EditAnywhere, Category="Gun")
    float Range = 10000.f;

    UPROPERTY(EditAnywhere, Category="Gun")
    float Damage = 20.f;
};
