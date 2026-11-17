#include "KemuriAtmosphereComponents.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriEnemies.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"

// ============================================================================
// Angst
// ============================================================================

UKemuriFearComponent::UKemuriFearComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UKemuriFearComponent::BeginPlay()
{
	Super::BeginPlay();
	if (HeartbeatLoop)
	{
		HeartbeatComp = UGameplayStatics::SpawnSound2D(this, HeartbeatLoop, 0.01f, 1.f, 0.f, nullptr, false, false);
	}
	if (BreathingLoop)
	{
		BreathingComp = UGameplayStatics::SpawnSound2D(this, BreathingLoop, 0.01f, 1.f, 0.f, nullptr, false, false);
	}
}

void UKemuriFearComponent::AddScareSpike(float Amount)
{
	Spike = FMath::Clamp(Spike + Amount, 0.f, 1.f);
}

float UKemuriFearComponent::ComputeTargetFear() const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (!Player || !Sub)
	{
		return Spike;
	}
	const FVector PlayerLoc = Player->GetActorLocation();

	float Proximity = 0.f;
	if (const AKemuriStalker* Stalker = Sub->GetStalker())
	{
		if (!Stalker->IsDead())
		{
			const float Dist = FVector::Dist(Stalker->GetActorLocation(), PlayerLoc);
			Proximity = FMath::Square(1.f - FMath::Clamp(Dist / StalkerFearRadius, 0.f, 1.f));

			if (Proximity > 0.f)
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(KemuriFearLOS), false, Stalker);
				Params.AddIgnoredActor(Player);
				const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Stalker->GetActorLocation() + FVector(0, 0, 60),
					PlayerLoc + FVector(0, 0, 60), ECC_Visibility, Params);
				if (bBlocked)
				{
					Proximity *= 0.6f;
				}
			}
			if (Stalker->IsStunned())
			{
				Proximity *= 0.4f;
			}
		}
	}

	const float FogFear = Sub->GetFogAmountAt(PlayerLoc) * FogFearWeight;
	return FMath::Clamp(FMath::Max(Proximity, FogFear) + Spike, 0.f, 1.f);
}

void UKemuriFearComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Spike = FMath::Max(0.f, Spike - SpikeDecayPerSecond * DeltaTime);
	const float Target = ComputeTargetFear();
	// Angst steigt schnell und faellt langsam.
	const float Speed = Target > Fear ? FearInterpSpeed * 2.5f : FearInterpSpeed * 0.5f;
	Fear = FMath::FInterpTo(Fear, Target, DeltaTime, Speed);

	// Effekte wirken auf die aktive Kamera (zu Fuss oder auf dem Scooter).
	if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Camera = Player->FindComponentByClass<UCameraComponent>();
	}
	ApplyEffects();

	ShakeCooldown -= DeltaTime;
	if (PanicShake && Fear >= PanicThreshold && ShakeCooldown <= 0.f)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->ClientStartCameraShake(PanicShake, Fear);
		}
		ShakeCooldown = 2.5f;
	}
}

void UKemuriFearComponent::ApplyEffects()
{
	if (Camera)
	{
		FPostProcessSettings& PP = Camera->PostProcessSettings;
		PP.bOverride_VignetteIntensity = true;
		PP.VignetteIntensity = BaseVignette + Fear * MaxExtraVignette;
		PP.bOverride_FilmGrainIntensity = true;
		PP.FilmGrainIntensity = 0.15f + Fear * MaxFilmGrain;
		PP.bOverride_SceneFringeIntensity = true;
		PP.SceneFringeIntensity = Fear * MaxFringe;
		PP.bOverride_ColorSaturation = true;
		const float Sat = 1.f - Fear * MaxDesaturation;
		PP.ColorSaturation = FVector4(Sat, Sat, Sat, 1.f);
	}

	if (AtmosphereCollection)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, AtmosphereCollection, TEXT("Kemuri_Fear"), Fear);
	}

	if (HeartbeatComp)
	{
		HeartbeatComp->SetVolumeMultiplier(FMath::Max(0.001f, Fear));
		HeartbeatComp->SetPitchMultiplier(0.85f + Fear * 0.6f);
		HeartbeatComp->SetFloatParameter(TEXT("Fear"), Fear);
	}
	if (BreathingComp)
	{
		BreathingComp->SetVolumeMultiplier(FMath::Max(0.001f, FMath::SmoothStep(0.3f, 1.f, Fear)));
		BreathingComp->SetFloatParameter(TEXT("Fear"), Fear);
	}
}

// ============================================================================
// Flackern
// ============================================================================

UKemuriFlickerComponent::UKemuriFlickerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.033f;
}

void UKemuriFlickerComponent::BeginPlay()
{
	Super::BeginPlay();
	NoiseOffset = FMath::FRandRange(0.f, 1000.f);
	TArray<ULightComponent*> Found;
	if (GetOwner())
	{
		GetOwner()->GetComponents<ULightComponent>(Found);
	}
	for (ULightComponent* Light : Found)
	{
		Lights.Add(Light);
		BaseIntensities.Add(Light->Intensity);
	}
}

void UKemuriFlickerComponent::KillLight(float Duration)
{
	KillTimeLeft = FMath::Max(KillTimeLeft, Duration);
}

void UKemuriFlickerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player && FVector::DistSquared(Player->GetActorLocation(), GetOwner()->GetActorLocation()) > FMath::Square(8000.f))
	{
		return; // Weit weg: keine Kosten.
	}

	if (Sub && !PowerOutageFlag.IsNone() && Sub->HasFlag(PowerOutageFlag))
	{
		SetBrightness(0.f);
		return;
	}
	if (KillTimeLeft > 0.f)
	{
		KillTimeLeft -= DeltaTime;
		SetBrightness(0.f);
		return;
	}

	float Strength = IdleFlicker;
	if (bReactToStalker && Sub)
	{
		if (const AKemuriStalker* Stalker = Sub->GetStalker())
		{
			const float Dist = FVector::Dist(Stalker->GetActorLocation(), GetOwner()->GetActorLocation());
			Strength = FMath::Max(Strength, 1.f - FMath::Clamp(Dist / StalkerReactRadius, 0.f, 1.f));
		}
	}
	const float Photo = Sub ? Sub->PhotosensitivityScale : 1.f;
	Strength *= Photo;
	if (Strength <= KINDA_SMALL_NUMBER)
	{
		SetBrightness(1.f);
		return;
	}

	const float Time = GetWorld()->GetTimeSeconds();
	const float N = 0.5f + 0.5f * FMath::PerlinNoise1D(Time * FlickerSpeed + NoiseOffset);

	if (BurstTimeLeft > 0.f)
	{
		BurstTimeLeft -= DeltaTime;
		SetBrightness(0.04f);
		return;
	}
	if (FMath::FRand() < Strength * 1.4f * DeltaTime)
	{
		BurstTimeLeft = FMath::FRandRange(0.04f, 0.2f);
	}
	SetBrightness(1.f - Strength * (1.f - N) * 0.85f);
}

void UKemuriFlickerComponent::SetBrightness(float Alpha)
{
	if (FMath::IsNearlyEqual(Alpha, CurrentAlpha, 0.01f))
	{
		return;
	}
	CurrentAlpha = Alpha;
	for (int32 i = 0; i < Lights.Num(); ++i)
	{
		if (ULightComponent* Light = Lights[i].Get())
		{
			Light->SetIntensity(BaseIntensities[i] * Alpha);
		}
	}
	if (!EmissiveParameter.IsNone() && GetOwner())
	{
		TArray<UPrimitiveComponent*> Prims;
		GetOwner()->GetComponents<UPrimitiveComponent>(Prims);
		for (UPrimitiveComponent* Prim : Prims)
		{
			Prim->SetScalarParameterValueOnMaterials(EmissiveParameter, Alpha);
		}
	}
}

// ============================================================================
// IR-Sichtbarkeit
// ============================================================================

UKemuriIRRevealComponent::UKemuriIRRevealComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKemuriIRRevealComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->OnIRChanged.AddDynamic(this, &UKemuriIRRevealComponent::HandleIRChanged);
		Sub->OnFlagChanged.AddDynamic(this, &UKemuriIRRevealComponent::HandleFlagChanged);
	}
	Refresh();
}

void UKemuriIRRevealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->OnIRChanged.RemoveDynamic(this, &UKemuriIRRevealComponent::HandleIRChanged);
		Sub->OnFlagChanged.RemoveDynamic(this, &UKemuriIRRevealComponent::HandleFlagChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UKemuriIRRevealComponent::HandleIRChanged(bool bIRActive)
{
	Refresh();
}

void UKemuriIRRevealComponent::HandleFlagChanged(FName Flag, bool bIsSet)
{
	if (Flag == RequiredFlag)
	{
		Refresh();
	}
}

void UKemuriIRRevealComponent::Refresh()
{
	AActor* Owner = GetOwner();
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (!Owner || !Sub)
	{
		return;
	}
	const bool bIR = Sub->IsIRActive();
	const bool bShow = (bVisibleOnlyInIR ? bIR : !bIR) && Sub->IsFlagSatisfied(RequiredFlag);

	Owner->SetActorHiddenInGame(!bShow);
	if (bAffectCollision)
	{
		Owner->SetActorEnableCollision(bShow);
	}

	TArray<UPrimitiveComponent*> Prims;
	Owner->GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* Prim : Prims)
	{
		const bool bGlow = bShow && bIR;
		Prim->SetRenderCustomDepth(bGlow);
		Prim->SetCustomDepthStencilValue(bGlow ? IRStencilValue : 0);
	}
}

// ============================================================================
// Im Nebel verschwinden
// ============================================================================

UKemuriFogVanishComponent::UKemuriFogVanishComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f;
}

void UKemuriFogVanishComponent::BeginPlay()
{
	Super::BeginPlay();
	Delay = FMath::FRandRange(0.f, MaxRandomDelay);
}

void UKemuriFogVanishComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (!Owner || !Sub)
	{
		return;
	}
	if (Sub->GetFogAmountAt(Owner->GetActorLocation()) < FogThreshold)
	{
		FoggedTime = 0.f;
		return;
	}
	FoggedTime += DeltaTime;
	if (FoggedTime < Delay || Owner->WasRecentlyRendered(0.5f))
	{
		return;
	}

	OnVanished.Broadcast(Owner);
	if (bDestroyActor)
	{
		Owner->Destroy();
		return;
	}
	Owner->SetActorHiddenInGame(true);
	Owner->SetActorEnableCollision(false);
	Owner->SetActorTickEnabled(false);
	SetComponentTickEnabled(false);
}

// ============================================================================
// Spline-Folger
// ============================================================================

UKemuriSplineFollowerComponent::UKemuriSplineFollowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKemuriSplineFollowerComponent::BeginPlay()
{
	Super::BeginPlay();
	Spline = SplineActor ? SplineActor->FindComponentByClass<USplineComponent>() : nullptr;
	Distance = StartDistance;
	CurrentSpeed = Speed;
}

void UKemuriSplineFollowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Spline || !Owner || Owner->IsHidden())
	{
		return;
	}

	float TargetSpeed = Speed;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector ToPlayer = Player->GetActorLocation() - Owner->GetActorLocation();
		if (ToPlayer.Size() < StopForPlayerDistance && FVector::DotProduct(ToPlayer.GetSafeNormal(), Owner->GetActorForwardVector()) > 0.5f)
		{
			TargetSpeed = 0.f;
		}
	}
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, 2.5f);

	const float Length = Spline->GetSplineLength();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	Distance += CurrentSpeed * DeltaTime;
	if (bLoop)
	{
		Distance = FMath::Fmod(Distance, Length);
		if (Distance < 0.f)
		{
			Distance += Length;
		}
	}
	else
	{
		Distance = FMath::Clamp(Distance, 0.f, Length);
	}

	const FVector Loc = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	const FRotator Rot = Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	Owner->SetActorLocationAndRotation(Loc, Rot);
}
