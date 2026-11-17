#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "KemuriInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriFocusChanged, AActor*, NewFocus);

/**
 * Sucht per Blickstrahl nach Objekten mit IKemuriInteractable und zeigt den Prompt
 * ueber die Brille an. E ruft TryInteract().
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriInteractionComponent();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Interaktion")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Interaktion")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Interaktion")
	float InteractDistance = 230.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Interaktion")
	float TraceRadius = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Interaktion")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Fokussiertes Objekt bekommt Custom Depth (fuer einen dezenten Umriss im Post-Process). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Interaktion")
	bool bHighlightFocus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Interaktion")
	int32 HighlightStencilValue = 1;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Interaktion")
	FKemuriFocusChanged OnFocusChanged;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AActor* FindFocus() const;
	void SetHighlight(AActor* Actor, bool bOn) const;
	void RefreshPrompt();

	TWeakObjectPtr<AActor> FocusedActor;
	FText LastPrompt;
};
