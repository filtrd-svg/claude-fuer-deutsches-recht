#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "KemuriTypes.h"
#include "KemuriAIControllers.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class AKemuriStalker;

/**
 * Einfache Jaeger-KI fuer Nebelgestalten: laeuft zum Spieler und greift an.
 */
UCLASS()
class KEMURI_API AKemuriEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AKemuriEnemyAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|KI")
	float ThinkInterval = 0.3f;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	void Think();

	FTimerHandle ThinkTimer;
};

/**
 * Stalker-KI als Zustandsautomat (kein Behavior-Tree-Asset noetig).
 * Umherstreifen mit "Leine": Ohne Kontakt zieht sich der Suchradius um den Spieler langsam zu,
 * so dass der Stalker immer wieder in die Naehe kommt, ohne zu schummeln.
 * Sicht (Taschenlampe verraet), Gehoer (Schuesse, Sprinten, Scooter), Raucherbereich = Rueckzug.
 */
UCLASS()
class KEMURI_API AKemuriStalkerAIController : public AAIController
{
	GENERATED_BODY()

public:
	AKemuriStalkerAIController();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Stalker")
	EKemuriStalkerState GetState() const { return State; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float ThinkInterval = 0.25f;

	/** Leine: Radius um den Spieler, in dem Streifziele gewaehlt werden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float LeashRadiusMax = 4500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float LeashRadiusMin = 900.f;

	/** Sekunden ohne Kontakt, bis die Leine ganz eng ist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float LeashTightenTime = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float SearchDuration = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float NoiseMemory = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float RetreatDistance = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float RetreatDuration = 25.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Stalker")
	TObjectPtr<UAIPerceptionComponent> Perception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Stalker")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Stalker")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void HandlePerception(AActor* Actor, FAIStimulus Stimulus);

	void Think();
	void SetState(EKemuriStalkerState NewState);
	bool PickNavPoint(const FVector& Origin, float Radius, FVector& Out) const;
	bool PickPointAwayFrom(const FVector& Threat, float Distance, FVector& Out) const;
	APawn* GetPlayerPawn() const;
	AKemuriStalker* GetStalker() const;

	EKemuriStalkerState State = EKemuriStalkerState::Roam;

	bool bSeesTarget = false;
	FVector LastSeenLocation = FVector::ZeroVector;
	double LastSeenTime = -1000.0;
	FVector LastNoiseLocation = FVector::ZeroVector;
	double LastNoiseTime = -1000.0;
	double LastContactTime = 0.0;
	double RetreatUntil = 0.0;
	bool bHasRoamGoal = false;

	FTimerHandle ThinkTimer;
};

/**
 * Passanten mit Schirm: schlendern zu zufaelligen Punkten, fliehen vor dem Stalker.
 * Mit UKemuriFogVanishComponent verschwinden sie spaeter im Nebel.
 */
UCLASS()
class KEMURI_API AKemuriCivilianAIController : public AAIController
{
	GENERATED_BODY()

public:
	AKemuriCivilianAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Passanten")
	float WanderRadius = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Passanten")
	FVector2D WaitTimeRange = FVector2D(2.f, 7.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Passanten")
	float FleeRadius = 1600.f;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	void Think();

	FVector HomeLocation = FVector::ZeroVector;
	double NextMoveTime = 0.0;
	FTimerHandle ThinkTimer;
};
