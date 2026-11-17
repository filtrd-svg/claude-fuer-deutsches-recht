#pragma once

#include "CoreMinimal.h"
#include "KemuriInteractableActor.h"
#include "KemuriTypes.h"
#include "KemuriShops.generated.h"

class UKemuriWeaponData;

/**
 * Wandkauf: Kreideumriss einer Waffe an einer Wand. Kaufen oder Munition nachkaufen.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriWallBuy : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Wandkauf")
	TObjectPtr<UKemuriWeaponData> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Wandkauf")
	int32 Price = 750;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Wandkauf")
	int32 AmmoPrice = 375;

	/** Munition fuer die geweihte Version ist teurer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Wandkauf")
	int32 UpgradedAmmoPrice = 2500;
};

USTRUCT(BlueprintType)
struct FKemuriBoxEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	TObjectPtr<UKemuriWeaponData> Weapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	float Weight = 1.f;
};

UENUM(BlueprintType)
enum class EKemuriBoxState : uint8
{
	Idle, Rolling, Offering, Moving, Inactive
};

/**
 * Omikuji-Schrein: Zufallswaffe gegen Mon (entspricht der Zufallskiste).
 * "Kyo" (schlechtes Los) laesst den Schrein an einen anderen Ort wandern.
 * Mehrere im Level platzieren; genau einer hat bStartsActive.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriOmikujiBox : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	AKemuriOmikujiBox();

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	TArray<FKemuriBoxEntry> Pool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	int32 Price = 950;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	bool bStartsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	float RollDuration = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	float OfferDuration = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	int32 MinUsesBeforeKyo = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KyoChance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Omikuji")
	float MoveDelay = 8.f;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Omikuji")
	EKemuriBoxState GetBoxState() const { return BoxState; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Omikuji")
	void SetBoxActive(bool bActive);

	/** Waehrend des Rollens: Waffe kurz zeigen (Hologramm/Zettel). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Omikuji")
	void OnRollTick(UKemuriWeaponData* ShownWeapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Omikuji")
	void OnOffer(UKemuriWeaponData* Weapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Omikuji")
	void OnKyo();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Omikuji")
	void OnActiveChanged(bool bActive);

protected:
	virtual void BeginPlay() override;

	UKemuriWeaponData* PickWeapon(const AActor* For) const;
	void RollTick();
	void FinishRoll();
	void OfferExpired();
	void MoveAway();

	EKemuriBoxState BoxState = EKemuriBoxState::Inactive;
	int32 Uses = 0;
	float RollElapsed = 0.f;

	UPROPERTY()
	TObjectPtr<UKemuriWeaponData> OfferedWeapon;

	TWeakObjectPtr<AActor> Roller;
	FTimerHandle RollTimer;
	FTimerHandle OfferTimer;
	FTimerHandle MoveTimer;
};

/**
 * Weihe-Schrein im Wald: Waffe weihen (Upgrade, entspricht dem Upgrade-Automaten).
 * Bei geweihter Waffe: Munitionsmodifikation wechseln, sofern die passende Reliquie vorhanden ist.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriUpgradeShrine : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Weihe")
	int32 UpgradePrice = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Weihe")
	int32 AmmoModPrice = 2500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Weihe")
	float UpgradeDuration = 5.f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Weihe")
	void OnUpgradeStarted(UKemuriWeaponData* Weapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Weihe")
	void OnUpgradeReady(UKemuriWeaponData* UpgradedWeapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Weihe")
	void OnAmmoModApplied(EKemuriAmmoMod Mod);

protected:
	void FinishUpgrade();
	TArray<EKemuriAmmoMod> GetUnlockedMods(const AActor* For) const;

	bool bBusy = false;
	bool bReady = false;

	UPROPERTY()
	TObjectPtr<UKemuriWeaponData> PendingWeapon;

	FTimerHandle UpgradeTimer;
};

/**
 * Getraenkeautomat mit dauerhaftem Effekt (entspricht den Perk-Automaten).
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriPerkVending : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Automat")
	EKemuriPerk Perk = EKemuriPerk::TetsuCha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Automat")
	FText DrinkName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Automat")
	int32 Price = 2500;

	/** Automat braucht Strom (Flag). Leer = immer an. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Automat")
	FName PowerFlag;
};
