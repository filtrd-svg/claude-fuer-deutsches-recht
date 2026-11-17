#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KemuriTypes.h"
#include "KemuriEnemies.generated.h"

class UKemuriHealthComponent;
class UDamageType;
class USoundBase;

/**
 * Basis fuer Gegner. Die Nebelgestalten sind direkt diese Klasse (Blueprint-Kind mit Mesh),
 * der Stalker ist AKemuriStalker. Statuseffekte der Munitionsmodifikationen laufen hier.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AKemuriEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Gegner")
	TObjectPtr<UKemuriHealthComponent> Health;

	// ---------- Punkte (Mon) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Mon")
	int32 PointsPerHit = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Mon")
	int32 PointsPerKill = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Mon")
	int32 HeadshotKillBonus = 40;

	// ---------- Angriff ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Angriff")
	float AttackDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Angriff")
	float AttackRange = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Angriff")
	float AttackCooldown = 1.4f;

	/** Verzoegerung zwischen Ausholen (Animation) und Treffer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Angriff")
	float AttackHitDelay = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner")
	float BaseWalkSpeed = 330.f;

	/** Nebelgestalten zerfallen sofort durch Shio-Munition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner")
	bool bPurgedByShio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner")
	float CorpseLifeSpan = 6.f;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Gegner")
	bool TryAttack(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Gegner")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Gegner")
	float GetSpeedMultiplier() const { return SlowTimeLeft > 0.f ? SlowFactor : 1.f; }

	/** Effekt einer Munitionsmodifikation. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Gegner")
	virtual void ApplyAmmoMod(EKemuriAmmoMod Mod, AController* InstigatorController, AActor* Causer, FVector HitLocation);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Gegner")
	void ApplyBurn(float DamagePerSecond, float Duration, AController* InstigatorController);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Gegner")
	void ApplySlow(float Factor, float Duration);

	// ---------- Blueprint-Hooks fuer Animation/FX ----------
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Gegner")
	void OnAttack(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Gegner")
	void OnHit(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Gegner")
	void OnDied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Gegner")
	void OnAmmoModTriggered(EKemuriAmmoMod Mod, FVector HitLocation);

	// ---------- Munitionsmodifikationen: Tuning ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float RaijinChainRadius = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	int32 RaijinMaxTargets = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float RaijinDamage = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float KitsunebiDPS = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float KitsunebiDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float YukiSlowFactor = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Gegner|Munition")
	float YukiDuration = 4.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth);

	UFUNCTION()
	virtual void HandleDied(AController* Killer, const UDamageType* DamageType);

	void GivePoints(AController* To, int32 Amount) const;
	void ResolveAttack();
	void UpdateSpeed();

	TWeakObjectPtr<AActor> PendingAttackTarget;
	double LastAttackTime = -1000.0;
	FTimerHandle AttackTimer;

	float BurnDPS = 0.f;
	float BurnTimeLeft = 0.f;
	TWeakObjectPtr<AController> BurnInstigator;

	float SlowFactor = 1.f;
	float SlowTimeLeft = 0.f;
};

/**
 * Der Stalker (Ame-Otoko bzw. Ame-Onna). Nicht toetbar ausser durch die gedruckte HARAI-Waffe.
 * Genug Schaden betaeubt ihn; jede Betaeubung macht die naechste schwerer.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriStalker : public AKemuriEnemyBase
{
	GENERATED_BODY()

public:
	AKemuriStalker();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float WalkSpeed = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float ChaseSpeed = 430.f;

	/** Zuschlag auf das Tempo bei vollem Nebel (0.2 = +20 %). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float FogSpeedBonus = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float StunThreshold = 700.f;

	/** Jede Betaeubung erhoeht die Schwelle um diesen Faktor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float StunThresholdGrowth = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float StunDuration = 8.f;

	/** Shio-Munition zaehlt mehrfach fuer die Betaeubung. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float ShioStunMultiplier = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float CatchRadius = 120.f;

	/** Schaden beim Zugriff. Standard: toedlich (Mamori-Mizu rettet einmal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	float CatchDamage = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Stalker")
	TObjectPtr<USoundBase> StingerOnSpotted;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Stalker")
	bool IsStunned() const { return bStunned; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Stalker")
	void SetChasing(bool bChasing);

	/** Wird vom AI-Controller aufgerufen, wenn der Spieler erreicht wurde. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Stalker")
	void CatchTarget(AActor* Target);

	/** Lenkt den Stalker zu einem Ort (Drucker-Belagerung, Skript-Szenen). */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Stalker")
	void LureTo(FVector Location);

	bool ConsumeLure(FVector& OutLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Stalker")
	void OnStunned(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Stalker")
	void OnRecovered();

	/** Jumpscare: Greif-Montage, Kamera-Sequenz, Schrei. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Stalker")
	void OnCatch(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Stalker")
	void OnSpottedPlayer();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Stalker")
	void OnDefeated();

	virtual void ApplyAmmoMod(EKemuriAmmoMod Mod, AController* InstigatorController, AActor* Causer, FVector HitLocation) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth) override;
	virtual void HandleDied(AController* Killer, const UDamageType* DamageType) override;

	void AddStunBuildup(float Amount);
	void Recover();

	bool bStunned = false;
	bool bChasingState = false;
	float StunBuildup = 0.f;
	FTimerHandle StunTimer;

	bool bHasLure = false;
	FVector LureLocation = FVector::ZeroVector;
};
