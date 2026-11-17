#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "KemuriTypes.h"
#include "KemuriWeapon.generated.h"

class UKemuriWeaponData;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKemuriAmmoChanged, int32, InMagazine, int32, Reserve);

/**
 * Hitscan-Waffe. Alle Werte stammen aus UKemuriWeaponData.
 * Animationen und Sounds am Arm-Mesh haengen in Blueprints an OnFired / OnReloadStarted.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriWeapon : public AActor
{
	GENERATED_BODY()

public:
	AKemuriWeapon();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void InitFromData(UKemuriWeaponData* InData);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void RefillAmmo();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffe")
	void SetAmmoMod(EKemuriAmmoMod NewMod) { AmmoMod = NewMod; OnAmmoModChanged(NewMod); }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffe")
	UKemuriWeaponData* GetData() const { return Data; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffe")
	EKemuriAmmoMod GetAmmoMod() const { return AmmoMod; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffe")
	bool IsReloading() const { return bReloading; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffe")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffe")
	int32 GetAmmoReserve() const { return AmmoReserve; }

	/** Wird vom Waffenhalter gesetzt (Hayai-Kohi). */
	UPROPERTY(BlueprintReadWrite, Category = "Kemuri|Waffe")
	float ReloadSpeedMultiplier = 1.f;

	/** Kollisionskanal fuer Treffer. Gegner-Meshes muessen diesen Kanal blocken. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Waffe")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Waffe")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Waffe")
	FKemuriAmmoChanged OnAmmoChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Waffe")
	void OnFired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Waffe")
	void OnReloadStarted(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Waffe")
	void OnReloadFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Waffe")
	void OnHitEnemy(AActor* Enemy, bool bHeadshot, bool bKilled);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Waffe")
	void OnAmmoModChanged(EKemuriAmmoMod NewMod);

protected:
	void FireOnce();
	void FinishReload();
	void TraceShot(const FVector& Origin, const FVector& Direction);

	UPROPERTY()
	TObjectPtr<UKemuriWeaponData> Data;

	int32 AmmoInMagazine = 0;
	int32 AmmoReserve = 0;
	EKemuriAmmoMod AmmoMod = EKemuriAmmoMod::None;
	bool bReloading = false;
	bool bWantsToFire = false;
	double LastFireTime = -1000.0;

	FTimerHandle AutoFireTimer;
	FTimerHandle ReloadTimer;
};
