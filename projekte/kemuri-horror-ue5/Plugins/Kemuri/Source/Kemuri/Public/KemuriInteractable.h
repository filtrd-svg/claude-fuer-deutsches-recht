#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KemuriInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UKemuriInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Alles, was der Spieler mit E benutzen kann.
 * Blueprints koennen das Interface ebenfalls implementieren (Class Settings -> Interfaces).
 */
class KEMURI_API IKemuriInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kemuri|Interaktion")
	bool CanInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kemuri|Interaktion")
	void Interact(AActor* Interactor);

	/** Kurzer Text, den die Brille als AR-Einblendung zeigt (kein HUD). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kemuri|Interaktion")
	FText GetInteractPrompt(AActor* Interactor);
};
