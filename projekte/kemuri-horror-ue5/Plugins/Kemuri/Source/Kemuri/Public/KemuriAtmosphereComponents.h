#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KemuriAtmosphereComponents.generated.h"

class UCameraComponent;
class UAudioComponent;
class USoundBase;
class UMaterialParameterCollection;
class UCameraShakeBase;
class ULightComponent;
class USplineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriVanished, AActor*, VanishedActor);

/**
 * Angst-Indikator ohne HUD. Naehe und Sichtlinie zum Stalker, Nebel am Spieler und
 * Schreckmomente treiben Vignette, Koernung, Farbsaum, Entsaettigung, Herzschlag und Atem.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriFearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriFearComponent();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Angst")
	float GetFear() const { return Fear; }

	/** Kurzer Schock (Jumpscare): springt hoch und klingt ab. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Angst")
	void AddScareSpike(float Amount = 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float StalkerFearRadius = 2600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float FogFearWeight = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float FearInterpSpeed = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float SpikeDecayPerSecond = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float BaseVignette = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float MaxExtraVignette = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float MaxFilmGrain = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float MaxFringe = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float MaxDesaturation = 0.55f;

	/** Loop-Sound Herzschlag (MetaSound mit Float-Parameter "Fear" empfohlen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	TObjectPtr<USoundBase> HeartbeatLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	TObjectPtr<USoundBase> BreathingLoop;

	/** Optional: Skalar "Kemuri_Fear" fuer Materialien (Regentropfen auf der Linse, Adern usw.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	TObjectPtr<UMaterialParameterCollection> AtmosphereCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	TSubclassOf<UCameraShakeBase> PanicShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Angst")
	float PanicThreshold = 0.82f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float ComputeTargetFear() const;
	void ApplyEffects();

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BreathingComp;

	float Fear = 0.f;
	float Spike = 0.f;
	float ShakeCooldown = 0.f;
};

/**
 * Flackerndes Licht. Reagiert auf die Naehe des Stalkers (Lampen spielen verrueckt, bevor er kommt)
 * und auf Stromausfall-Flags. Respektiert die Photosensitivitaets-Einstellung.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriFlickerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriFlickerComponent();

	/** Grundflackern (0 = ruhig, 1 = stark). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IdleFlicker = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern")
	float FlickerSpeed = 9.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern")
	bool bReactToStalker = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern")
	float StalkerReactRadius = 1800.f;

	/** Ist dieses Flag gesetzt, bleibt das Licht aus (Stromausfall). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern")
	FName PowerOutageFlag;

	/** Optional: Material-Parameter fuer leuchtende Meshes (Neonschild). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Flackern")
	FName EmissiveParameter = TEXT("EmissiveStrength");

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Flackern")
	void KillLight(float Duration);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetBrightness(float Alpha);

	TArray<TWeakObjectPtr<ULightComponent>> Lights;
	TArray<float> BaseIntensities;
	float NoiseOffset = 0.f;
	float BurstTimeLeft = 0.f;
	float KillTimeLeft = 0.f;
	float CurrentAlpha = 1.f;
};

/**
 * Macht den Besitzer nur im IR-Kameramodus sichtbar (oder umgekehrt nur ohne).
 * Fuer versteckte Raeume, Schriftzeichen, nasse Fussspuren des Stalkers, Markierungen des Informanten.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriIRRevealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriIRRevealComponent();

	/** true = nur im IR sichtbar. false = nur OHNE IR sichtbar (Trugbilder, die im IR verschwinden). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|IR")
	bool bVisibleOnlyInIR = true;

	/** Kollision folgt der Sichtbarkeit (geheime Wege). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|IR")
	bool bAffectCollision = false;

	/** Sichtbar erst, wenn dieses Flag gesetzt ist (z. B. praeziser Hinweis des Informanten). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|IR")
	FName RequiredFlag;

	/** Im IR leuchtet das Objekt ueber Custom Stencil (Wert im IR-Material abfragen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|IR")
	int32 IRStencilValue = 2;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|IR")
	void Refresh();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleIRChanged(bool bIRActive);

	UFUNCTION()
	void HandleFlagChanged(FName Flag, bool bIsSet);
};

/**
 * Passanten und Radfahrer verschwinden im Nebel, sobald niemand hinsieht.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriFogVanishComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriFogVanishComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float FogThreshold = 0.6f;

	/** Zusaetzliche Zufallsverzoegerung, damit nicht alle gleichzeitig verschwinden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	float MaxRandomDelay = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nebel")
	bool bDestroyActor = false;

	/** Z. B. den Schirm des Passanten auf der Strasse liegen lassen. */
	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Nebel")
	FKemuriVanished OnVanished;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float Delay = 0.f;
	float FoggedTime = 0.f;
};

/**
 * Bewegt den Besitzer entlang eines Splines (Fahrradfahrer, Lieferroller, Krähe).
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriSplineFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriSplineFollowerComponent();

	/** Actor, der eine Spline-Komponente enthaelt (z. B. leerer Actor mit Spline). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Spline")
	TObjectPtr<AActor> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Spline")
	float Speed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Spline")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Spline")
	float StartDistance = 0.f;

	/** Haelt an, wenn der Spieler im Weg steht. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Spline")
	float StopForPlayerDistance = 250.f;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Spline")
	float GetCurrentSpeed() const { return CurrentSpeed; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<USplineComponent> Spline;

	float Distance = 0.f;
	float CurrentSpeed = 0.f;
};
