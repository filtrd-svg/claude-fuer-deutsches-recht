#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KemuriInteractable.h"
#include "KemuriInteractableActor.generated.h"

class UStaticMeshComponent;
class USoundBase;
class UKemuriInventoryComponent;
class UKemuriWeaponData;

/**
 * Basis fuer alles Benutzbare: Shops, Schreine, Drucker, Pickups.
 * Blueprints erben davon und setzen Mesh, Prompt und Effekte.
 */
UCLASS(Abstract, Blueprintable)
class KEMURI_API AKemuriInteractableActor : public AActor, public IKemuriInteractable
{
	GENERATED_BODY()

public:
	AKemuriInteractableActor();

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	FText Prompt;

	/** Nur benutzbar, wenn dieses Flag gesetzt ist (leer = immer). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	FName RequiredFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	TObjectPtr<USoundBase> InteractSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri")
	TObjectPtr<USoundBase> DeniedSound;

	/** Wird nach jeder erfolgreichen Interaktion aufgerufen (Animationen, Effekte). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnInteracted(AActor* Interactor);

	/** Zu wenig Mon oder Bedingung nicht erfuellt. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnDenied(AActor* Interactor);

protected:
	bool IsRequiredFlagSet() const;
	void PlayInteractSound() const;
	void Deny(AActor* Interactor);

	static UKemuriInventoryComponent* GetInventory(const AActor* Actor);
	static FText FormatPrice(const FText& Label, int32 Price);
};

/**
 * Aufhebbares Objekt: Zigaretten, Weed, Bauteile, Reliquien, Akkus, Filament.
 * Akku-Items laden direkt die Brille.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriPickup : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	AKemuriPickup();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	FText DisplayName;

	/** Optional: Waffe direkt aufnehmen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	TObjectPtr<UKemuriWeaponData> WeaponData;

	/** Prozent Akku, wenn ItemId = Akku. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	float BatteryAmount = 35.f;

	/** Der Resonanz-Scanner der Brille piept auf dieses Objekt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	bool bScannable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	FName FlagOnPickup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Pickup")
	bool bDestroyOnPickup = true;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Pickup")
	void OnPickedUp(AActor* Interactor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
