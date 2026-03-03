#pragma once

#include "CoreMinimal.h"
#include "PlanetMovementComponent.h"
#include "MyEnemyMovementComponent.generated.h"

UCLASS()
class MITTSPEL_API UMyEnemyMovementComponent : public UPlanetMovementComponent
{
	GENERATED_BODY()

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	virtual void BeginPlay() override;
public:

};