// Fill out your copyright notice in the Description page of Project Settings.

// TODO FIXA SÅ ATT OM EN PLANET HAR SAMMA ROTATION SOM OMLOPPSBANA SÄTT ROTATION TILL NOLL OCH ROTERA SKYSPHEREN RUNT ORBITAXIS ISTÄLLET   

#include "SolarSystemsManager.h"
#include <Kismet/GameplayStatics.h>
#include "M_Skysphere.h"
#include "GameFramework/Character.h"
#include <Kismet/KismetMathLibrary.h>
#include "Engine/DirectionalLight.h"
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
        UE_LOG(LogTemp, Warning, TEXT("ANCHOR CHANGED)"));
        if (PrevAnchorBody != nullptr)
        {
            PrevAnchorBody->SetSpinSpeedDegPerSec(PrevSpin);
            PrevAnchorBody->SetOrbitSpeedDegPerSec(PrevOrbit);
        }

        if (AnchorBody != nullptr)
        {
            float AnchorSpinDeg = AnchorBody->GetSpinDegPerSec();
            float AnchorOrbitSpeedDeg = AnchorBody->GetOrbitSpeedDegPerSec();

            PrevSpin = AnchorSpinDeg;
            PrevOrbit = AnchorOrbitSpeedDeg;

            AnchorBody->SetSpinSpeedDegPerSec(0.f);
            AnchorBody->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg - AnchorSpinDeg);
			if (AnchorBody->isMoon)
                SkySphereActor->SetOrbitSpeedDegPerSec(-AnchorOrbitSpeedDeg / 4);
            else
                SkySphereActor->SetOrbitSpeedDegPerSec(AnchorOrbitSpeedDeg/4);
		}
        else {
            SkySphereActor->SetOrbitSpeedDegPerSec(0.f);
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
    }

    PrevAnchorBody = AnchorBody;

    // Rendera bodies i ankarets frame (kan fortfarande uppdateras varje tick)
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
    if (!SunDirectionalLight || !SunBody) return;

    APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!P) return;

    const FVector SunLoc = SunBody->GetActorLocation();
    const FVector PlayerLoc = P->GetActorLocation();

    FVector Dir = (PlayerLoc - SunLoc).GetSafeNormal();
    if (Dir.IsNearlyZero()) return;

    FRotator SunRot = Dir.Rotation();
    SunRot.Yaw += LightYawOffsetDeg;
    SunDirectionalLight->SetActorRotation(SunRot);

    if (SunFillLight)
    {
        FVector PlanetCenter = FVector::ZeroVector;
        if (AnchorBody)
        {
            PlanetCenter = AnchorBody->GetActorLocation();
        }
        FVector Up = (PlayerLoc - PlanetCenter).GetSafeNormal();
        if (Up.IsNearlyZero()) Up = FVector::UpVector;

        FVector Axis = FVector::CrossProduct(Dir, Up).GetSafeNormal();

        if (Axis.IsNearlyZero())
        {
            Axis = FVector::CrossProduct(Dir, FVector::RightVector).GetSafeNormal();
            if (Axis.IsNearlyZero())
                Axis = FVector::UpVector;
        }

        const float AngleDeg = -FillLightAngleOffset;
        FVector FillDir = Dir.RotateAngleAxis(AngleDeg, Axis);

        FRotator FillRot = FillDir.Rotation();
        FillRot.Yaw += LightYawOffsetDeg;

        SunFillLight->SetActorRotation(FillRot);
    }
}


