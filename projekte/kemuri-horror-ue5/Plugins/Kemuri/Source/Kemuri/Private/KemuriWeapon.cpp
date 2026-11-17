#include "KemuriWeapon.h"
#include "KemuriWeaponData.h"
#include "KemuriEnemies.h"
#include "KemuriInventoryComponent.h"
#include "KemuriHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"

AKemuriWeapon::AKemuriWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	SetRootComponent(Mesh);
}

void AKemuriWeapon::InitFromData(UKemuriWeaponData* InData)
{
	Data = InData;
	if (!Data)
	{
		return;
	}
	if (Data->WeaponMesh)
	{
		Mesh->SetSkeletalMeshAsset(Data->WeaponMesh);
	}
	AmmoInMagazine = Data->MagazineSize;
	AmmoReserve = Data->MaxReserve;
	OnAmmoChanged.Broadcast(AmmoInMagazine, AmmoReserve);
}

void AKemuriWeapon::StartFire()
{
	if (!Data)
	{
		return;
	}
	bWantsToFire = true;
	const float Interval = 60.f / FMath::Max(1.f, Data->RoundsPerMinute);
	const double Now = GetWorld()->GetTimeSeconds();
	const float Wait = FMath::Max(0.f, static_cast<float>(LastFireTime + Interval - Now));

	if (Wait <= 0.f)
	{
		FireOnce();
	}
	if (Data->bAutomatic)
	{
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AKemuriWeapon::FireOnce, Interval, true, Wait > 0.f ? Wait : Interval);
	}
}

void AKemuriWeapon::StopFire()
{
	bWantsToFire = false;
	GetWorldTimerManager().ClearTimer(AutoFireTimer);
}

void AKemuriWeapon::FireOnce()
{
	if (!Data || bReloading)
	{
		return;
	}
	if (AmmoInMagazine <= 0)
	{
		if (Data->DryFireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Data->DryFireSound, GetActorLocation());
		}
		StopFire();
		Reload();
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (!OwnerController)
	{
		return;
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
	--AmmoInMagazine;

	FVector ViewLoc;
	FRotator ViewRot;
	OwnerController->GetPlayerViewPoint(ViewLoc, ViewRot);
	const float HalfConeRad = FMath::DegreesToRadians(Data->SpreadDegrees * 0.5f);
	for (int32 i = 0; i < Data->PelletCount; ++i)
	{
		TraceShot(ViewLoc, FMath::VRandCone(ViewRot.Vector(), HalfConeRad));
	}

	if (Data->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->FireSound, GetActorLocation());
	}
	if (Data->MuzzleFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(Data->MuzzleFX, Mesh, Data->MuzzleSocket, FVector::ZeroVector,
			FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}

	// Schuesse sind laut: Stalker und Nebelgestalten hoeren das.
	UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), Data->NoiseLoudness, OwnerPawn, Data->NoiseRange, FName(TEXT("Schuss")));

	OwnerPawn->AddControllerPitchInput(-Data->RecoilPitch);
	OnFired();
	OnAmmoChanged.Broadcast(AmmoInMagazine, AmmoReserve);

	if (!Data->bAutomatic)
	{
		bWantsToFire = false;
	}
	if (AmmoInMagazine <= 0)
	{
		StopFire();
		Reload();
	}
}

void AKemuriWeapon::TraceShot(const FVector& Origin, const FVector& Direction)
{
	const FVector End = Origin + Direction * Data->Range;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KemuriWeapon), true, this);
	Params.AddIgnoredActor(GetOwner());
	Params.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, TraceChannel, Params))
	{
		return;
	}

	if (Data->ImpactFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Data->ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return;
	}

	const bool bHeadshot = Hit.BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase);
	const float Damage = Data->Damage * (bHeadshot ? Data->HeadshotMultiplier : 1.f);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* InstigatorController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	UGameplayStatics::ApplyPointDamage(HitActor, Damage, Direction, Hit, InstigatorController, this, Data->DamageType);

	if (AKemuriEnemyBase* Enemy = Cast<AKemuriEnemyBase>(HitActor))
	{
		if (AmmoMod != EKemuriAmmoMod::None && FMath::FRand() < Data->AmmoModProcChance)
		{
			Enemy->ApplyAmmoMod(AmmoMod, InstigatorController, this, Hit.ImpactPoint);
		}

		const bool bKilled = Enemy->IsDead();
		if (bKilled && bHeadshot)
		{
			if (UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(OwnerPawn))
			{
				Inv->AddMon(Enemy->HeadshotKillBonus);
			}
		}
		OnHitEnemy(Enemy, bHeadshot, bKilled);
	}
}

void AKemuriWeapon::Reload()
{
	if (!Data || bReloading || AmmoReserve <= 0 || AmmoInMagazine >= Data->MagazineSize)
	{
		return;
	}
	bReloading = true;
	StopFire();
	const float Duration = Data->ReloadTime * FMath::Max(0.1f, ReloadSpeedMultiplier);
	if (Data->ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->ReloadSound, GetActorLocation());
	}
	OnReloadStarted(Duration);
	GetWorldTimerManager().SetTimer(ReloadTimer, this, &AKemuriWeapon::FinishReload, Duration, false);
}

void AKemuriWeapon::FinishReload()
{
	bReloading = false;
	if (!Data)
	{
		return;
	}
	const int32 Needed = Data->MagazineSize - AmmoInMagazine;
	const int32 Taken = FMath::Min(Needed, AmmoReserve);
	AmmoInMagazine += Taken;
	AmmoReserve -= Taken;
	OnReloadFinished();
	OnAmmoChanged.Broadcast(AmmoInMagazine, AmmoReserve);
}

void AKemuriWeapon::RefillAmmo()
{
	if (!Data)
	{
		return;
	}
	AmmoInMagazine = Data->MagazineSize;
	AmmoReserve = Data->MaxReserve;
	OnAmmoChanged.Broadcast(AmmoInMagazine, AmmoReserve);
}
