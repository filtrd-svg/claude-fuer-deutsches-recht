#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KemuriHealthComponent.generated.h"

class UDamageType;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FKemuriDamaged, float, Damage, const UDamageType*, DamageType, AController*, InstigatedBy, AActor*, DamageCauser, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKemuriDied, AController*, Killer, const UDamageType*, DamageType);

/**
 * Lebenspunkte fuer Spieler und Gegner. Hoert auf das Engine-Schadenssystem (ApplyDamage).
 * Spieler: regeneriert nach kurzer Pause (wie im Zombie-Modus). Stalker: kann nur durch
 * OnlyLethalDamageType sterben.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben")
	bool bRegenerates = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben", meta = (EditCondition = "bRegenerates"))
	float RegenDelay = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben", meta = (EditCondition = "bRegenerates"))
	float RegenPerSecond = 35.f;

	/** Wenn gesetzt, faellt das Leben durch andere Schadenstypen nie unter 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben")
	TSubclassOf<UDamageType> OnlyLethalDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Leben")
	bool bInvulnerable = false;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Leben")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Leben")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Leben")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Leben")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Leben")
	void SetMaxHealth(float NewMax, bool bFill = true);

	/** Holt den Besitzer aus dem Tod zurueck (Mamori-Mizu, Checkpoint). */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Leben")
	void Revive(float HealthPercent = 0.5f);

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Leben")
	FKemuriDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Leben")
	FKemuriDied OnDied;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:
	float Health = 100.f;
	float TimeSinceDamage = 0.f;
	bool bDead = false;
};
