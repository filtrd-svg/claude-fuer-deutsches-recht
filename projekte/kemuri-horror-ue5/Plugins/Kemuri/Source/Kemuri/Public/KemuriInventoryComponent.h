#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KemuriInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKemuriItemChanged, FName, ItemId, int32, NewCount);

/**
 * Einfaches Inventar auf Basis von Item-IDs. Die Waehrung "Mon" (Punkte wie im Zombie-Modus)
 * ist ebenfalls ein Item, damit Shops, Informant und Drucker dieselbe Logik nutzen.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriInventoryComponent();

	static UKemuriInventoryComponent* FindOn(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Inventar")
	void AddItem(FName ItemId, int32 Count = 1);

	/** Gibt false zurueck und entfernt nichts, wenn nicht genug vorhanden ist. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Inventar")
	bool RemoveItem(FName ItemId, int32 Count = 1);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Inventar")
	int32 GetCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Kemuri|Inventar")
	bool HasItem(FName ItemId, int32 Count = 1) const { return GetCount(ItemId) >= Count; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Inventar")
	int32 GetMon() const;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Inventar")
	void AddMon(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Inventar")
	bool SpendMon(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Inventar")
	TMap<FName, int32> GetAllItems() const { return Items; }

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Inventar")
	FKemuriItemChanged OnItemChanged;

	/** Startinventar, z. B. Mon = 500 und eine nasse Zigarettenschachtel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Inventar")
	TMap<FName, int32> StartItems;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Kemuri|Inventar")
	TMap<FName, int32> Items;
};
