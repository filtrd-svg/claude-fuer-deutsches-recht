#include "KemuriWorldActors.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriAtmosphereComponents.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraShakeBase.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"

// ============================================================================
// Raucherbereich
// ============================================================================

AKemuriSafeZone::AKemuriSafeZone()
{
	PrimaryActorTick.bCanEverTick = false;
	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	Volume->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Volume);
}

void AKemuriSafeZone::BeginPlay()
{
	Super::BeginPlay();
	Volume->OnComponentBeginOverlap.AddDynamic(this, &AKemuriSafeZone::HandleBeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &AKemuriSafeZone::HandleEndOverlap);
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->OnFlagChanged.AddDynamic(this, &AKemuriSafeZone::HandleFlagChanged);
	}
}

bool AKemuriSafeZone::IsPlayer(const AActor* Actor) const
{
	return Actor && Actor == UGameplayStatics::GetPlayerPawn(this, 0);
}

void AKemuriSafeZone::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPlayer(OtherActor))
	{
		return;
	}
	bPlayerInside = true;
	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (Sub && Sub->IsFlagSatisfied(DisabledByFlag) && !DisabledByFlag.IsNone())
	{
		return; // Finale: kein Schutz mehr.
	}
	if (Sub)
	{
		Sub->SetPlayerInSafeZone(true);
	}
	OnPlayerEntered();
}

void AKemuriSafeZone::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsPlayer(OtherActor))
	{
		return;
	}
	bPlayerInside = false;
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->SetPlayerInSafeZone(false);
	}
	OnPlayerLeft();
}

void AKemuriSafeZone::HandleFlagChanged(FName Flag, bool bIsSet)
{
	if (Flag == DisabledByFlag && bIsSet && bPlayerInside)
	{
		if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
		{
			Sub->SetPlayerInSafeZone(false);
		}
	}
}

// ============================================================================
// Schreckmoment
// ============================================================================

AKemuriScareTrigger::AKemuriScareTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetBoxExtent(FVector(200.f, 200.f, 150.f));
	Volume->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Volume);
}

void AKemuriScareTrigger::BeginPlay()
{
	Super::BeginPlay();
	Volume->OnComponentBeginOverlap.AddDynamic(this, &AKemuriScareTrigger::HandleBeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &AKemuriScareTrigger::HandleEndOverlap);
}

void AKemuriScareTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
	{
		PlayerInside = OtherActor;
	}
}

void AKemuriScareTrigger::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == PlayerInside.Get())
	{
		PlayerInside.Reset();
	}
}

bool AKemuriScareTrigger::ConditionsMet(AActor* Player) const
{
	if (!Player || (bOnce && bFired))
	{
		return false;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastFireTime < Cooldown)
	{
		return false;
	}
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (Sub)
	{
		if (!Sub->IsFlagSatisfied(RequiredFlag) || (bRequireIRMode && !Sub->IsIRActive()))
		{
			return false;
		}
	}
	if (const UKemuriFearComponent* Fear = Player->FindComponentByClass<UKemuriFearComponent>())
	{
		if (Fear->GetFear() > MaxFearToTrigger)
		{
			return false;
		}
	}
	if (LookAtTarget)
	{
		const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC)
		{
			return false;
		}
		FVector ViewLoc;
		FRotator ViewRot;
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);
		const FVector ToTarget = (LookAtTarget->GetActorLocation() - ViewLoc).GetSafeNormal();
		if (FVector::DotProduct(ToTarget, ViewRot.Vector()) < FMath::Cos(FMath::DegreesToRadians(LookAtAngle)))
		{
			return false;
		}
	}
	return true;
}

void AKemuriScareTrigger::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* Player = PlayerInside.Get();
	if (Player && ConditionsMet(Player))
	{
		// Wuerfeln nur einmal pro Betreten-Fenster, damit Chance < 1 nicht sofort durchrutscht.
		if (FMath::FRand() <= Chance)
		{
			Fire(Player);
		}
		LastFireTime = GetWorld()->GetTimeSeconds();
		if (bOnce)
		{
			bFired = true;
		}
	}
}

void AKemuriScareTrigger::Fire(AActor* Player)
{
	bFired = true;
	LastFireTime = GetWorld()->GetTimeSeconds();

	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	const float Photo = Sub ? Sub->PhotosensitivityScale : 1.f;

	if (Sequence)
	{
		if (ULevelSequencePlayer* SeqPlayer = Sequence->GetSequencePlayer())
		{
			SeqPlayer->Play();
		}
	}
	if (Stinger)
	{
		UGameplayStatics::PlaySound2D(this, Stinger);
	}
	if (Photo > 0.f)
	{
		for (AActor* LightActor : LightsToKill)
		{
			if (!LightActor)
			{
				continue;
			}
			if (UKemuriFlickerComponent* Flicker = LightActor->FindComponentByClass<UKemuriFlickerComponent>())
			{
				Flicker->KillLight(LightsOffDuration);
			}
		}
	}
	if (CameraShake)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->ClientStartCameraShake(CameraShake, FMath::Max(0.2f, Photo));
		}
	}
	if (Player)
	{
		if (UKemuriFearComponent* Fear = Player->FindComponentByClass<UKemuriFearComponent>())
		{
			Fear->AddScareSpike(FearSpike);
		}
	}
	if (Sub)
	{
		Sub->SetFlag(FlagOnTrigger);
	}
	OnScare(Player);
}
