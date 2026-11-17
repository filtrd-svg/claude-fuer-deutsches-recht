#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "KemuriWorldSubsystem.generated.h"

class AKemuriStalker;
class AKemuriFogDirector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKemuriFlagChanged, FName, Flag, bool, bIsSet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriIRChanged, bool, bIRActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriPlayerCaught, AActor*, Stalker);

/**
 * Zentrale Drehscheibe pro Level: Story-Flags, IR-Zustand, Registrierung von Stalker,
 * Nebel-Direktor und Scanner-Zielen. Aus Blueprints: "Get KemuriWorldSubsystem".
 */
UCLASS()
class KEMURI_API UKemuriWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UKemuriWorldSubsystem* Get(const UObject* WorldContext);

	// ---------- Flags ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Flags")
	void SetFlag(FName Flag);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Flags")
	void ClearFlag(FName Flag);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Flags")
	bool HasFlag(FName Flag) const;

	/** Leeres Flag zaehlt als erfuellt. */
	UFUNCTION(BlueprintPure, Category = "Kemuri|Flags")
	bool IsFlagSatisfied(FName Flag) const { return Flag.IsNone() || HasFlag(Flag); }

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Flags")
	FKemuriFlagChanged OnFlagChanged;

	// ---------- IR-Kamera ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|IR")
	void SetIRActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Kemuri|IR")
	bool IsIRActive() const { return bIRActive; }

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|IR")
	FKemuriIRChanged OnIRChanged;

	// ---------- Registrierungen ----------
	void RegisterStalker(AKemuriStalker* InStalker) { Stalker = InStalker; }

	UFUNCTION(BlueprintPure, Category = "Kemuri")
	AKemuriStalker* GetStalker() const { return Stalker.Get(); }

	void RegisterFogDirector(AKemuriFogDirector* InDirector) { FogDirector = InDirector; }

	UFUNCTION(BlueprintPure, Category = "Kemuri")
	AKemuriFogDirector* GetFogDirector() const { return FogDirector.Get(); }

	/** 0 = klar, 1 = voller Nebel an dieser Stelle. */
	UFUNCTION(BlueprintPure, Category = "Kemuri|Nebel")
	float GetFogAmountAt(const FVector& Location) const;

	void RegisterScanTarget(AActor* Target);
	void UnregisterScanTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Scanner")
	AActor* GetNearestScanTarget(const FVector& From, float MaxDistance, float& OutDistance) const;

	// ---------- Spielerzustand ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri")
	void SetPlayerInSafeZone(bool bInside);

	UFUNCTION(BlueprintPure, Category = "Kemuri")
	bool IsPlayerInSafeZone() const { return bPlayerInSafeZone; }

	UPROPERTY(BlueprintAssignable, Category = "Kemuri")
	FKemuriPlayerCaught OnPlayerCaught;

	/**
	 * Barrierefreiheit: 1 = volle Blitz- und Flackereffekte, 0 = keine schnellen Helligkeitswechsel.
	 * Im Optionsmenue anbieten (Photosensitivitaet).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Optionen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhotosensitivityScale = 1.0f;

private:
	UPROPERTY()
	TSet<FName> Flags;

	bool bIRActive = false;
	bool bPlayerInSafeZone = false;

	TWeakObjectPtr<AKemuriStalker> Stalker;
	TWeakObjectPtr<AKemuriFogDirector> FogDirector;
	TArray<TWeakObjectPtr<AActor>> ScanTargets;
};
