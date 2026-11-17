#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KemuriFogDirector.generated.h"

class AExponentialHeightFog;
class UMaterialParameterCollection;
class UCurveFloat;
class AKemuriEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriFogStage, int32, Stage);

/**
 * Der Nebel ist die Uhr des Spiels (kein HUD-Timer).
 * Ein "sicherer Kreis" um den Raucherbereich schrumpft ueber die Spielzeit; ausserhalb ist Nebel.
 * Der Direktor steuert Hoehennebel, aktiviert Nebel-Actoren (Local Fog Volumes, Niagara-Schwaden),
 * schreibt Werte in eine Material Parameter Collection und spawnt Nebelgestalten im Nebel.
 * Einmal pro Level platzieren, Position = Zentrum (Raucherbereich).
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriFogDirector : public AActor
{
	GENERATED_BODY()

public:
	AKemuriFogDirector();

	/** 0 = klar, 1 = voller Nebel. Weicher Rand ueber EdgeWidth. */
	UFUNCTION(BlueprintPure, Category = "Kemuri|Nebel")
	float GetFogAmountAt(const FVector& Location) const;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Nebel")
	float GetProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Nebel")
	float GetSafeRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Nebel")
	void StartFog() { bRunning = true; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Nebel")
	void SetPaused(bool bPaused) { bRunning = !bPaused; }

	/** Drueckt den Nebel zurueck (Laternen entzuendet, Schrein gereinigt). Amount in Fortschritt (0..1). */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Nebel")
	void PushBack(float Amount);

	/** Drucker-Belagerung: mehr Nebelgestalten, schneller. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Nebel")
	void SetSiegeMode(bool bEnable) { bSiege = bEnable; }

	/** Ende: Nebel loest sich auf. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Nebel")
	void Dissipate(float Seconds = 20.f);

	// ---------- Zeitachse ----------
	/** Sekunden, bis der Nebel das Viertel ganz eingenommen hat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float FullFogDuration = 1800.f;

	/** Startet erst, wenn dieses Flag gesetzt wird (z. B. nach dem ersten Gespraech). Leer = sofort. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	FName StartFlag = TEXT("Nebel_Start");

	/** Optional: Verlauf von Zeit (0..1) zu Fortschritt (0..1), z. B. langsam beginnen und dann kippen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	TObjectPtr<UCurveFloat> ProgressCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float StartSafeRadius = 16000.f;

	/** Bei vollem Nebel bleibt nur noch der Raucherbereich frei. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float EndSafeRadius = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float EdgeWidth = 1500.f;

	/** Anzahl Stufen fuer OnStageChanged (Sounds, Story-Ereignisse). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	int32 StageCount = 5;

	// ---------- Darstellung ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	TObjectPtr<AExponentialHeightFog> HeightFog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	float ClearFogDensity = 0.015f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	float FullFogDensity = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	float ClearVolumetricExtinction = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	float FullVolumetricExtinction = 12.f;

	/** Parameter: Kemuri_FogProgress, Kemuri_SafeRadius, Kemuri_FogAtPlayer, Kemuri_FogCenter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	TObjectPtr<UMaterialParameterCollection> AtmosphereCollection;

	/** Actoren (Local Fog Volumes, Niagara-Schwaden), die sichtbar werden, sobald der Nebel sie erreicht. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Darstellung")
	TArray<TObjectPtr<AActor>> FogZoneActors;

	// ---------- Nebelgestalten ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	TSubclassOf<AKemuriEnemyBase> WraithClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	int32 MaxWraithsAtStart = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	int32 MaxWraithsAtFull = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	float SpawnIntervalAtStart = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	float SpawnIntervalAtFull = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	float SiegeMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	FVector2D SpawnDistance = FVector2D(1400.f, 3200.f);

	/** Gestalten erscheinen nur, wo mindestens so viel Nebel ist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel|Gestalten")
	float MinFogForSpawn = 0.55f;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Nebel")
	FKemuriFogStage OnStageChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Nebel")
	void OnWraithSpawned(AKemuriEnemyBase* Wraith);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void UpdateVisuals();
	void UpdateSpawning(float DeltaSeconds);
	bool TrySpawnWraith();

	float Elapsed = 0.f;
	float Progress = 0.f;
	float PushBackAmount = 0.f;
	bool bRunning = false;
	bool bSiege = false;
	bool bDissipating = false;
	float DissipateSpeed = 0.f;
	int32 CurrentStage = 0;
	float SpawnTimer = 0.f;

	UPROPERTY()
	TArray<TWeakObjectPtr<AKemuriEnemyBase>> Wraiths;
};
