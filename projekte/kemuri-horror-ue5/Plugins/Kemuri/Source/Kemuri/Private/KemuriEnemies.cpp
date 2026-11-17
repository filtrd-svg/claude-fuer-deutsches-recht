#include "KemuriEnemies.h"
#include "KemuriAIControllers.h"
#include "KemuriHealthComponent.h"
#include "KemuriInventoryComponent.h"
#include "KemuriDamageTypes.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriScooter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// AKemuriEnemyBase
// ============================================================================

AKemuriEnemyBase::AKemuriEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	Health = CreateDefaultSubobject<UKemuriHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 150.f;

	AIControllerClass = AKemuriEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	bUseControllerRotationYaw = false;

	// Treffer per Waffe: Mesh muss den Waffen-Kanal blocken (Physik-Asset fuer Kopftreffer).
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void AKemuriEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	Health->OnDamaged.AddDynamic(this, &AKemuriEnemyBase::HandleDamaged);
	Health->OnDied.AddDynamic(this, &AKemuriEnemyBase::HandleDied);
	UpdateSpeed();
}

bool AKemuriEnemyBase::IsDead() const
{
	return Health && Health->IsDead();
}

void AKemuriEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsDead())
	{
		return;
	}

	if (BurnTimeLeft > 0.f)
	{
		BurnTimeLeft -= DeltaSeconds;
		UGameplayStatics::ApplyDamage(this, BurnDPS * DeltaSeconds, BurnInstigator.Get(), this, UKemuriFireDamageType::StaticClass());
	}

	if (SlowTimeLeft > 0.f)
	{
		SlowTimeLeft -= DeltaSeconds;
		if (SlowTimeLeft <= 0.f)
		{
			UpdateSpeed();
		}
	}
}

void AKemuriEnemyBase::UpdateSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * GetSpeedMultiplier();
}

bool AKemuriEnemyBase::TryAttack(AActor* Target)
{
	if (!Target || IsDead())
	{
		return false;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown)
	{
		return false;
	}
	LastAttackTime = Now;
	PendingAttackTarget = Target;
	OnAttack(Target);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AKemuriEnemyBase::ResolveAttack, FMath::Max(0.01f, AttackHitDelay), false);
	return true;
}

void AKemuriEnemyBase::ResolveAttack()
{
	AActor* Target = PendingAttackTarget.Get();
	if (!Target || IsDead())
	{
		return;
	}
	// Ausweichen ist moeglich: nur treffen, wenn das Ziel noch in Reichweite ist.
	if (FVector::Dist(Target->GetActorLocation(), GetActorLocation()) <= AttackRange * 1.25f)
	{
		UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UKemuriMeleeDamageType::StaticClass());
	}
}

void AKemuriEnemyBase::GivePoints(AController* To, int32 Amount) const
{
	if (!To || Amount <= 0)
	{
		return;
	}
	if (UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(To->GetPawn()))
	{
		Inv->AddMon(Amount);
	}
	else if (const AKemuriScooter* Scooter = Cast<AKemuriScooter>(To->GetPawn()))
	{
		if (UKemuriInventoryComponent* RiderInv = UKemuriInventoryComponent::FindOn(Scooter->GetRider()))
		{
			RiderInv->AddMon(Amount);
		}
	}
}

void AKemuriEnemyBase::HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth)
{
	if (InstigatedBy && InstigatedBy->IsPlayerController())
	{
		GivePoints(InstigatedBy, PointsPerHit);
	}
	OnHit(Damage, DamageCauser);
}

void AKemuriEnemyBase::HandleDied(AController* Killer, const UDamageType* DamageType)
{
	if (Killer && Killer->IsPlayerController())
	{
		GivePoints(Killer, PointsPerKill);
	}
	GetWorldTimerManager().ClearTimer(AttackTimer);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetachFromControllerPendingDestroy();
	OnDied();
	SetLifeSpan(CorpseLifeSpan);
}

void AKemuriEnemyBase::ApplyBurn(float DamagePerSecond, float Duration, AController* InstigatorController)
{
	BurnDPS = DamagePerSecond;
	BurnTimeLeft = FMath::Max(BurnTimeLeft, Duration);
	BurnInstigator = InstigatorController;
}

void AKemuriEnemyBase::ApplySlow(float Factor, float Duration)
{
	SlowFactor = FMath::Clamp(Factor, 0.05f, 1.f);
	SlowTimeLeft = FMath::Max(SlowTimeLeft, Duration);
	UpdateSpeed();
}

void AKemuriEnemyBase::ApplyAmmoMod(EKemuriAmmoMod Mod, AController* InstigatorController, AActor* Causer, FVector HitLocation)
{
	if (IsDead())
	{
		return;
	}
	OnAmmoModTriggered(Mod, HitLocation);

	switch (Mod)
	{
	case EKemuriAmmoMod::Raijin:
	{
		// Kettenblitz auf Gegner in der Naehe.
		TArray<AActor*> Found;
		TArray<TEnumAsByte<EObjectTypeQuery>> Types;
		Types.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		TArray<AActor*> Ignore;
		UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), RaijinChainRadius, Types, AKemuriEnemyBase::StaticClass(), Ignore, Found);
		int32 Hits = 0;
		for (AActor* Actor : Found)
		{
			if (Hits >= RaijinMaxTargets)
			{
				break;
			}
			AKemuriEnemyBase* Other = Cast<AKemuriEnemyBase>(Actor);
			if (Other && !Other->IsDead())
			{
				UGameplayStatics::ApplyDamage(Other, RaijinDamage, InstigatorController, Causer, UKemuriLightningDamageType::StaticClass());
				++Hits;
			}
		}
		break;
	}
	case EKemuriAmmoMod::Kitsunebi:
		ApplyBurn(KitsunebiDPS, KitsunebiDuration, InstigatorController);
		break;
	case EKemuriAmmoMod::Yuki:
		ApplySlow(YukiSlowFactor, YukiDuration);
		break;
	case EKemuriAmmoMod::Shio:
		if (bPurgedByShio)
		{
			UGameplayStatics::ApplyDamage(this, Health->GetHealth() + 1.f, InstigatorController, Causer, UKemuriPurifyDamageType::StaticClass());
		}
		break;
	default:
		break;
	}
}

// ============================================================================
// AKemuriStalker
// ============================================================================

AKemuriStalker::AKemuriStalker()
{
	AIControllerClass = AKemuriStalkerAIController::StaticClass();
	Health->MaxHealth = 3000.f;
	Health->OnlyLethalDamageType = UKemuriFinalDamageType::StaticClass();
	PointsPerHit = 5;
	PointsPerKill = 5000;
	HeadshotKillBonus = 0;
	bPurgedByShio = false;
	BaseWalkSpeed = WalkSpeed;
	AttackRange = 160.f;
}

void AKemuriStalker::BeginPlay()
{
	BaseWalkSpeed = WalkSpeed;
	Super::BeginPlay();
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->RegisterStalker(this);
	}
}

void AKemuriStalker::SetChasing(bool bChasing)
{
	if (bChasing && !bChasingState)
	{
		OnSpottedPlayer();
		if (StingerOnSpotted)
		{
			UGameplayStatics::PlaySound2D(this, StingerOnSpotted);
		}
	}
	bChasingState = bChasing;

	float FogFactor = 0.f;
	if (const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		FogFactor = Sub->GetFogAmountAt(GetActorLocation());
	}
	BaseWalkSpeed = (bChasing ? ChaseSpeed : WalkSpeed) * (1.f + FogSpeedBonus * FogFactor);
	UpdateSpeed();
}

void AKemuriStalker::CatchTarget(AActor* Target)
{
	if (!Target || bStunned || IsDead())
	{
		return;
	}
	// Vom Scooter reissen.
	if (AKemuriScooter* Scooter = Cast<AKemuriScooter>(Target))
	{
		AActor* Rider = Scooter->GetRider();
		Scooter->Dismount();
		Target = Rider;
		if (!Target)
		{
			return;
		}
	}

	OnCatch(Target);
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->OnPlayerCaught.Broadcast(this);
	}
	UGameplayStatics::ApplyDamage(Target, CatchDamage, GetController(), this, UKemuriMeleeDamageType::StaticClass());
}

void AKemuriStalker::LureTo(FVector Location)
{
	bHasLure = true;
	LureLocation = Location;
}

bool AKemuriStalker::ConsumeLure(FVector& OutLocation)
{
	if (!bHasLure)
	{
		return false;
	}
	bHasLure = false;
	OutLocation = LureLocation;
	return true;
}

void AKemuriStalker::HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth)
{
	Super::HandleDamaged(Damage, DamageType, InstigatedBy, DamageCauser, NewHealth);
	const bool bFinal = DamageType && DamageType->IsA(UKemuriFinalDamageType::StaticClass());
	if (!bFinal)
	{
		AddStunBuildup(Damage);
	}
}

void AKemuriStalker::AddStunBuildup(float Amount)
{
	if (bStunned || IsDead())
	{
		return;
	}
	StunBuildup += Amount;
	if (StunBuildup >= StunThreshold)
	{
		bStunned = true;
		StunBuildup = 0.f;
		StunThreshold *= StunThresholdGrowth;
		GetCharacterMovement()->StopMovementImmediately();
		OnStunned(StunDuration);
		GetWorldTimerManager().SetTimer(StunTimer, this, &AKemuriStalker::Recover, StunDuration, false);
	}
}

void AKemuriStalker::Recover()
{
	bStunned = false;
	OnRecovered();
}

void AKemuriStalker::ApplyAmmoMod(EKemuriAmmoMod Mod, AController* InstigatorController, AActor* Causer, FVector HitLocation)
{
	OnAmmoModTriggered(Mod, HitLocation);
	switch (Mod)
	{
	case EKemuriAmmoMod::Shio:
		AddStunBuildup(StunThreshold * 0.1f * ShioStunMultiplier);
		break;
	case EKemuriAmmoMod::Yuki:
		ApplySlow(0.6f, YukiDuration);
		break;
	case EKemuriAmmoMod::Raijin:
		AddStunBuildup(RaijinDamage);
		break;
	case EKemuriAmmoMod::Kitsunebi:
		ApplyBurn(KitsunebiDPS * 0.5f, KitsunebiDuration, InstigatorController);
		break;
	default:
		break;
	}
}

void AKemuriStalker::HandleDied(AController* Killer, const UDamageType* DamageType)
{
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->SetFlag(KemuriFlags::StalkerDefeated);
	}
	OnDefeated();
	Super::HandleDied(Killer, DamageType);
	SetLifeSpan(0.f); // Leiche bleibt fuer das Ende liegen.
}
