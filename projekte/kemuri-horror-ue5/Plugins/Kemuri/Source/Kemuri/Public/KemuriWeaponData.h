#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KemuriWeaponData.generated.h"

class USkeletalMesh;
class USoundBase;
class UNiagaraSystem;
class UDamageType;
class AKemuriWeapon;

/**
 * Datenblatt einer Waffe. Im Editor: Rechtsklick -> Miscellaneous -> Data Asset -> KemuriWeaponData.
 * Jede Waffe hat optional eine geweihte Version (Weihe-Schrein, entspricht dem Upgrade-Automaten).
 */
UCLASS(BlueprintType)
class KEMURI_API UKemuriWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waffe")
	FText DisplayName;

	/** Optional eigene Waffenklasse (Blueprint mit Animationen). Leer = AKemuriWeapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waffe")
	TSubclassOf<AKemuriWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waffe")
	TObjectPtr<USkeletalMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waffe")
	FName AttachSocket = TEXT("GripPoint");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waffe")
	FTransform AttachOffset;

	// ---------- Schaden ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden")
	float Damage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden")
	float HeadshotMultiplier = 2.5f;

	/** Schrotflinte: mehrere Projektile pro Schuss. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden", meta = (ClampMin = "1"))
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden")
	float SpreadDegrees = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden")
	float Range = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schaden")
	TSubclassOf<UDamageType> DamageType;

	// ---------- Handhabung ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	float RoundsPerMinute = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	int32 MagazineSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	int32 MaxReserve = 64;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	float ReloadTime = 1.6f;

	/** Kameraruck nach oben pro Schuss (Grad). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	float RecoilPitch = 0.8f;

	/** Lautstaerke fuer die Stalker-KI (0 = lautlos, 1 = Schuss). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	float NoiseLoudness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handhabung")
	float NoiseRange = 4000.f;

	// ---------- Weihe / Endwaffe ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weihe")
	TObjectPtr<UKemuriWeaponData> UpgradedVersion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weihe")
	bool bIsUpgraded = false;

	/** Gedruckte HARAI-Waffe: kann nicht geweiht werden, toetet den Stalker. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weihe")
	bool bIsFinalWeapon = false;

	/** Chance pro Treffer, die Munitionsmodifikation auszuloesen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weihe", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmmoModProcChance = 0.15f;

	// ---------- Audio / FX ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> MuzzleFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> ImpactFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FName MuzzleSocket = TEXT("Muzzle");
};
