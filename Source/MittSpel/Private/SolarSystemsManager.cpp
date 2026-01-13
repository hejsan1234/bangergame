// Fill out your copyright notice in the Description page of Project Settings.

#include "SolarSystemsManager.h"
#include <Kismet/GameplayStatics.h>
#include "M_Skysphere.h"
#include "GameFramework/Character.h"

ASolarSystemManager::ASolarSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASolarSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AnchorBody != RequestedAnchorBody)
    {
        AnchorBody = RequestedAnchorBody;
        if (IsValid(SkySphereActor)) {
            SkySphereActor->SetActiveBody(AnchorBody);
        }
    }

    // Uppdatera simpos
    const FVector OldAnchorSimPos = AnchorSimPos;
    AnchorSimPos = AnchorBody ? AnchorBody->SimPos : FVector::ZeroVector;

    AAPlanetActor* Best = nullptr;
    float BestDistSq = TNumericLimits<float>::Max();

    const FVector PlayerPos = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)
        ? UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation()
        : FVector::ZeroVector;

    for (AAPlanetActor* Body : Bodies)
    {
        if (!Body) continue;

        const FVector Center = Body->GetCenterInFrame(AnchorSimPos); // render-frame
        const float DistSq = FVector::DistSquared(PlayerPos, Center);

        const float Range = Body->GravityRange;
        const float RangeSq = Range * Range;

        if (DistSq <= RangeSq && DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
			//UE_LOG(LogTemp, Warning, TEXT("SolarSystemManager: Body %s is within gravity range (dist sq: %f, range sq: %f)"), *Body->GetName(), DistSq, RangeSq);
            Best = Body;
        }
    }

    ActiveGravityBody = Best;

    const bool bAnchorChanged = (PrevAnchorBody != AnchorBody);
    PrevAnchorBody = AnchorBody;

    if (bAnchorChanged)
    {
        const FVector Shift = OldAnchorSimPos - AnchorSimPos;

        if (!Shift.IsNearlyZero())
        {
            UE_LOG(LogTemp, Warning, TEXT("SolarSystemManager: Applying world shift %s"), *Shift.ToString());

            if (ACharacter* C = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                C->AddActorWorldOffset(Shift, false, nullptr, ETeleportType::TeleportPhysics);
            }

            if (IsValid(SkySphereActor))
            {
                SkySphereActor->AddActorWorldOffset(Shift, false, nullptr, ETeleportType::TeleportPhysics);
            }
        }
    }

    // Rendera bodies i ankarets frame (kan fortfarande uppdateras varje tick)
    for (AAPlanetActor* Body : Bodies)
    {
        if (!Body) continue;
        Body->SetActorLocation(Body->SimPos - AnchorSimPos);
    }
}



void ASolarSystemManager::RequestAnchor(AAPlanetActor* Body)
{
	RequestedAnchorBody = Body;
}

void ASolarSystemManager::ClearAnchorRequest()
{
	RequestedAnchorBody = nullptr;
}


