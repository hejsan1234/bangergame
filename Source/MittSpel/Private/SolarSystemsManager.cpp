// Fill out your copyright notice in the Description page of Project Settings.

// TODO FIXA SÅ ATT OM EN PLANET HAR SAMMA ROTATION SOM OMLOPPSBANA SÄTT ROTATION TILL NOLL OCH ROTERA SKYSPHEREN RUNT ORBITAXIS ISTÄLLET   

#include "SolarSystemsManager.h"
#include <Kismet/GameplayStatics.h>
#include "M_Skysphere.h"
#include "GameFramework/Character.h"
#include <Kismet/KismetMathLibrary.h>
#include "Engine/DirectionalLight.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "APlanetActor.h" 

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

        const FVector Center = Body->GetCenterInFrame(AnchorSimPos);
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

    if (bAnchorChanged)
    {
        for (AAPlanetActor* Body : Bodies)
        {
            if (!Body) continue;

            if (AnchorBody != nullptr) {
                float AnchorSpinDeg = AnchorBody->GetSpinDegPerSec();
                float AnchorOrbitSpeedDeg = AnchorBody->GetOrbitSpeedDegPerSec();
                if (AnchorBody->isMoon) {
                    if (Body == AnchorBody) {
                        Body->SetSpinSpeedDegPerSec(0.f);
                        Body->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg - AnchorSpinDeg);

                        SkySphereActor->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg / 4);
                    }
                    else {
						if (Body->isMoon) {
                            Body->SetOrbitSpeedDegPerSec(Body->GetOrgOrbitSpeed() - AnchorSpinDeg);
                            Body->SetSpinSpeedDegPerSec(Body->GetOrgSpinSpeed() - AnchorSpinDeg);
						}
                        else {
                            Body->SetOrbitSpeedDegPerSec(-(Body->GetOrgOrbitSpeed()));
                            Body->SetSpinSpeedDegPerSec(-(Body->GetOrgSpinSpeed()));
                        }
                    }
                }
                else
                {
                    if (Body == AnchorBody) {
                        Body->SetSpinSpeedDegPerSec(0.f);
                        Body->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg - AnchorSpinDeg);

                        SkySphereActor->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg / 4);
                    }
                    else {
                        Body->SetOrgOrbitSpeedDegPerSec();
                        Body->SetOrgSpinSpeedDegPerSec();
                    }
                }
            }
            else {
                Body->SetOrgOrbitSpeedDegPerSec();
                Body->SetOrgSpinSpeedDegPerSec();
                SkySphereActor->SetOrbitSpeedDegPerSec(0.f);
            }
        }

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
        if (AnchorBody && AnchorBody->GetHasEnemy())
        {
            AnchorBody->bSpawnEnemy = true;
        }
    }

    PrevAnchorBody = AnchorBody;

    // Rendera bodies i ankarets frame
    for (AAPlanetActor* Body : Bodies)
    {
        if (!Body) continue;
        Body->SetActorLocation(Body->SimPos - AnchorSimPos);

        if (Body == AnchorBody) {
            continue;
        }
        Body->UpdateSpin(DeltaTime);
		Body->SetActorRotation(Body->SimRot);
    }

    UpdateSunLightDirection();

    if (AnchorBody && AnchorBody->bSpawnEnemy)
    {
        AnchorBody->SpawnEnemy();
        AnchorBody->bSpawnEnemy = false;
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

void ASolarSystemManager::UpdateSunLightDirection()
{
    AAPlanetActor* TargetBody = bLightTargetsAnchor ? AnchorBody : ActiveGravityBody;

    if (!SunDirectionalLight || !SunBody || !TargetBody)
        return;

    const FVector SunLoc = SunBody->GetActorLocation();
    const FVector TargetLoc = TargetBody->GetActorLocation();

    if (SunLoc.Equals(TargetLoc, 0.01f))
        return;

    FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(SunLoc, TargetLoc);
    LookRot.Yaw += LightYawOffsetDeg;

    SunDirectionalLight->SetActorRotation(LookRot);
}


