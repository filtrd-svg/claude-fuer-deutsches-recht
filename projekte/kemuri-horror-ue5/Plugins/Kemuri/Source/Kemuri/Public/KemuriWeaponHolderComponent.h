#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KemuriTypes.h"
#include "KemuriWeaponHolderComponent.generated.h"

class AKemuriWeapon;
class UKemuriWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriWeaponChanged, AKemuriWeapon*, NewWeapon);

/**
 * Waffenplaetze wie im Zombie-Modus: zwei Plaetze, mit Mittsu-Ramune drei.
 * Kaufen bei voller Belegung ersetzt die aktuelle Waffe.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriWeaponHolderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriWeaponHolderComponent();

	static UKemuriWeaponHolderComponent* FindOn(const AActor* Actor);

	/** Woran Waffen befestigt werden (Arm-Mesh mit Socket oder Kamera). */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void SetAttachParent(USceneComponent* InParent) { AttachParent = InParent; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	AKemuriWeapon* GiveWeapon(UKemuriWeaponData* Data);

	/** Besitzt der Spieler diese Waffe (auch geweiht)? */
	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffen")
	AKemuriWeapon* FindWeapon(const UKemuriWeaponData* Data) const;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void Equip(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void Reload();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffen")
	AKemuriWeapon* GetCurrentWeapon() const;

	/** Nimmt die aktuelle Waffe fuer den Weihe-Schrein weg und gibt ihre Daten zurueck. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	UKemuriWeaponData* TakeCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void SetReloadSpeedMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Waffen")
	void SetMaxSlots(int32 NewMax) { MaxSlots = FMath::Max(1, NewMax); }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffen")
	int32 GetMaxSlots() const { return MaxSlots; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Waffen")
	TArray<AKemuriWeapon*> GetWeapons() const;

	/** Startwaffe, z. B. die Pistole. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Waffen")
	TObjectPtr<UKemuriWeaponData> StartingWeapon;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Waffen")
	FKemuriWeaponChanged OnWeaponChanged;

protected:
	virtual void BeginPlay() override;

private:
	void AttachWeapon(AKemuriWeapon* Weapon) const;

	UPROPERTY()
	TArray<TObjectPtr<AKemuriWeapon>> Weapons;

	UPROPERTY()
	TObjectPtr<USceneComponent> AttachParent;

	int32 CurrentIndex = INDEX_NONE;
	int32 MaxSlots = 2;
	float ReloadSpeedMultiplier = 1.f;
};
