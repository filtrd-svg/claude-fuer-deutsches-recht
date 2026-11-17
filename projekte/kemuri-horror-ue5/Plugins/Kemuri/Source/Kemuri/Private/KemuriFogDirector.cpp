#include "KemuriFogDirector.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriEnemies.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

AKemuriFogDirector::AKemuriFogDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AKemuriFogDirector::BeginPlay()
{
	Super::BeginPlay();
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->RegisterFogDirector(this);
	}
	bRunning = StartFlag.IsNone();
	for (AActor* Zone : FogZoneActors)
	{
		if (Zone)
		{
			Zone->SetActorHiddenInGame(true);
		}
	}
	UpdateVisuals();
}

float AKemuriFogDirector::GetSafeRadius() const
{
	return FMath::Lerp(StartSafeRadius, EndSafeRadius, Progress);
}

float AKemuriFogDirector::GetFogAmountAt(const FVector& Location) const
{
	const float Dist = FVector::Dist2D(Location, GetActorLocation());
	const float Safe = GetSafeRadius();
	return FMath::SmoothStep(Safe - EdgeWidth * 0.5f, Safe + EdgeWidth * 0.5f, Dist);
}

void AKemuriFogDirector::PushBack(float Amount)
{
	PushBackAmount += FMath::Max(0.f, Amount);
}

void AKemuriFogDirector::Dissipate(float Seconds)
{
	bDissipating = true;
	bRunning = false;
	bSiege = false;
	DissipateSpeed = Progress / FMath::Max(1.f, Seconds);
}

void AKemuriFogDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRunning && !bDissipating && !StartFlag.IsNone())
	{
		if (const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
		{
			bRunning = Sub->HasFlag(StartFlag);
		}
	}

	if (bDissipating)
	{
		Progress = FMath::Max(0.f, Progress - DissipateSpeed * DeltaSeconds);
	}
	else if (bRunning)
	{
		Elapsed += DeltaSeconds;
		const float TimeAlpha = FMath::Clamp(Elapsed / FMath::Max(1.f, FullFogDuration), 0.f, 1.f);
		const float Curved = ProgressCurve ? ProgressCurve->GetFloatValue(TimeAlpha) : TimeAlpha;
		Progress = FMath::Clamp(Curved - PushBackAmount, 0.f, 1.f);
	}

	const int32 NewStage = FMath::Clamp(FMath::FloorToInt(Progress * StageCount), 0, StageCount);
	if (NewStage != CurrentStage)
	{
		CurrentStage = NewStage;
		OnStageChanged.Broadcast(CurrentStage);
	}

	UpdateVisuals();
	if (!bDissipating)
	{
		UpdateSpawning(DeltaSeconds);
	}
}

void AKemuriFogDirector::UpdateVisuals()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const float FogAtPlayer = Player ? GetFogAmountAt(Player->GetActorLocation()) : 0.f;

	// Hoehennebel ist global: Dichte folgt dem Nebel am Spieler, damit man den Rand sieht.
	if (HeightFog)
	{
		if (UExponentialHeightFogComponent* Comp = HeightFog->GetComponent())
		{
			const float Global = FMath::Max(FogAtPlayer, Progress * 0.35f);
			Comp->SetFogDensity(FMath::Lerp(ClearFogDensity, FullFogDensity, Global));
			Comp->SetVolumetricFogExtinctionScale(FMath::Lerp(ClearVolumetricExtinction, FullVolumetricExtinction, Global));
		}
	}

	if (AtmosphereCollection)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, AtmosphereCollection, TEXT("Kemuri_FogProgress"), Progress);
		UKismetMaterialLibrary::SetScalarParameterValue(this, AtmosphereCollection, TEXT("Kemuri_SafeRadius"), GetSafeRadius());
		UKismetMaterialLibrary::SetScalarParameterValue(this, AtmosphereCollection, TEXT("Kemuri_FogAtPlayer"), FogAtPlayer);
		const FVector C = GetActorLocation();
		UKismetMaterialLibrary::SetVectorParameterValue(this, AtmosphereCollection, TEXT("Kemuri_FogCenter"), FLinearColor(C.X, C.Y, C.Z, 0.f));
	}

	for (AActor* Zone : FogZoneActors)
	{
		if (Zone)
		{
			const bool bFogged = GetFogAmountAt(Zone->GetActorLocation()) > 0.3f;
			if (Zone->IsHidden() == bFogged)
			{
				Zone->SetActorHiddenInGame(!bFogged);
			}
		}
	}
}

void AKemuriFogDirector::UpdateSpawning(float DeltaSeconds)
{
	if (!WraithClass || (!bRunning && !bSiege))
	{
		return;
	}
	Wraiths.RemoveAll([](const TWeakObjectPtr<AKemuriEnemyBase>& W) { return !W.IsValid() || W->IsDead(); });

	const float Mult = bSiege ? SiegeMultiplier : 1.f;
	const int32 MaxAlive = FMath::RoundToInt(FMath::Lerp(static_cast<float>(MaxWraithsAtStart), static_cast<float>(MaxWraithsAtFull), Progress) * Mult);
	const float Interval = FMath::Lerp(SpawnIntervalAtStart, SpawnIntervalAtFull, Progress) / Mult;

	SpawnTimer += DeltaSeconds;
	if (SpawnTimer < Interval || Wraiths.Num() >= MaxAlive)
	{
		return;
	}
	if (TrySpawnWraith())
	{
		SpawnTimer = 0.f;
	}
}

bool AKemuriFogDirector::TrySpawnWraith()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Player || !Nav)
	{
		return false;
	}
	FVector ViewLoc;
	FRotator ViewRot;
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);
	}
	else
	{
		ViewLoc = Player->GetActorLocation();
		ViewRot = Player->GetActorRotation();
	}

	for (int32 Attempt = 0; Attempt < 8; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Dist = FMath::FRandRange(SpawnDistance.X, SpawnDistance.Y);
		const FVector Desired = Player->GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Dist;

		FNavLocation NavLoc;
		if (!Nav->ProjectPointToNavigation(Desired, NavLoc, FVector(500.f, 500.f, 800.f)))
		{
			continue;
		}
		if (GetFogAmountAt(NavLoc.Location) < MinFogForSpawn)
		{
			continue;
		}
		// Nicht im Blickfeld erscheinen lassen.
		const FVector ToSpawn = (NavLoc.Location - ViewLoc).GetSafeNormal();
		if (FVector::DotProduct(ToSpawn, ViewRot.Vector()) > 0.35f)
		{
			continue;
		}

		const float HalfHeight = WraithClass->GetDefaultObject<AKemuriEnemyBase>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		const FRotator FacePlayer = (Player->GetActorLocation() - NavLoc.Location).GetSafeNormal2D().Rotation();
		AKemuriEnemyBase* Wraith = GetWorld()->SpawnActor<AKemuriEnemyBase>(WraithClass, NavLoc.Location + FVector(0, 0, HalfHeight + 5.f), FacePlayer, Params);
		if (Wraith)
		{
			Wraiths.Add(Wraith);
			OnWraithSpawned(Wraith);
			return true;
		}
	}
	return false;
}
