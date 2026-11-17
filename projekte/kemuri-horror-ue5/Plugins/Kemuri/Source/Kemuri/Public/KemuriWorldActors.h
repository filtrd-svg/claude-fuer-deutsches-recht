#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KemuriWorldActors.generated.h"

class UBoxComponent;
class USoundBase;
class ALevelSequenceActor;
class UCameraShakeBase;

/**
 * Raucherbereich: Hier ist der Spieler sicher (Speicherraum-Prinzip). Der Stalker zieht sich zurueck.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriSafeZone : public AActor
{
	GENERATED_BODY()

public:
	AKemuriSafeZone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UBoxComponent> Volume;

	/** Ist dieses Flag gesetzt, ist die Zone NICHT mehr sicher (Finale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	FName DisabledByFlag = TEXT("Druck_Gestartet");

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnPlayerEntered();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnPlayerLeft();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleFlagChanged(FName Flag, bool bIsSet);

	bool IsPlayer(const AActor* Actor) const;
	bool bPlayerInside = false;
};

/**
 * Schreckmoment-Ausloeser. Bedingungen: einmalig, Zufall, Abklingzeit, nur im IR-Modus,
 * nur wenn der Spieler auf ein Ziel schaut, nur mit Flag. Ausloesung: Level Sequence,
 * Stinger-Sound, Lichter aus, Kamerawackeln, Angst-Spitze, Blueprint-Event.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriScareTrigger : public AActor
{
	GENERATED_BODY()

public:
	AKemuriScareTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Schreck")
	TObjectPtr<UBoxComponent> Volume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	bool bOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Chance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	float Cooldown = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	bool bRequireIRMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	FName RequiredFlag;

	/** Nur ausloesen, wenn der Spieler in Richtung dieses Actors schaut. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	TObjectPtr<AActor> LookAtTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	float LookAtAngle = 25.f;

	/** Kein Schreck, wenn die Angst schon hoch ist (Dosierung: Spannung braucht Taeler). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Bedingungen")
	float MaxFearToTrigger = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	TObjectPtr<ALevelSequenceActor> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	TObjectPtr<USoundBase> Stinger;

	/** Actoren mit Lichtern, die kurz ausgehen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	TArray<TObjectPtr<AActor>> LightsToKill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	float LightsOffDuration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	float FearSpike = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Schreck|Ausloesung")
	FName FlagOnTrigger;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Schreck")
	void OnScare(AActor* Player);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Schreck")
	void Fire(AActor* Player);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	bool ConditionsMet(AActor* Player) const;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TWeakObjectPtr<AActor> PlayerInside;
	bool bFired = false;
	double LastFireTime = -1000.0;
};
