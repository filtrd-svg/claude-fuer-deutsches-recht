#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KemuriInteractable.h"
#include "KemuriTypes.h"
#include "KemuriInformant.generated.h"

class UCapsuleComponent;
class USoundBase;

/**
 * Der Raucher im Raucherbereich. Kennt die Gegend, raucht gern.
 * Zigarette -> vager Hinweis. Weed -> er ist eine Weile high und gibt praezise Hinweise
 * (setzt Flags, die IR-Markierungen in der Welt sichtbar machen).
 * Spricht Japanisch; die Brille zeigt die Live-Uebersetzung.
 *
 * MetaHuman: MetaHuman-Blueprint oeffnen -> Class Settings -> Parent Class = KemuriInformant.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriInformant : public AActor, public IKemuriInteractable
{
	GENERATED_BODY()

public:
	AKemuriInformant();

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	/** Unsichtbare Kapsel als Ziel fuer den Interaktionsstrahl. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Informant")
	TObjectPtr<UCapsuleComponent> InteractVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant")
	FText SpeakerName;

	/** Reihenfolge = Story-Reihenfolge. Es gilt der erste offene Hinweis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant")
	TArray<FKemuriHint> Hints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant")
	float HighDuration = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant", meta = (MultiLine = true))
	FText AskForSmokeLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant", meta = (MultiLine = true))
	TArray<FText> IdleLines;

	/** Wird gesagt, wenn er high ist und es nichts Neues gibt (Lore, Wahnsinn, Humor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant", meta = (MultiLine = true))
	TArray<FText> HighRamblings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant")
	float SubtitleSecondsPerChar = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Informant")
	FName FlagOnFirstTalk = TEXT("Nebel_Start");

	UFUNCTION(BlueprintPure, Category = "Kemuri|Informant")
	bool IsHigh() const { return bHigh; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Informant")
	void Say(const FText& Text, USoundBase* Voice, AActor* Listener);

	// ---------- Blueprint-Hooks: Animation, Rauch-Niagara, Lachen ----------
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Informant")
	void OnLightCigarette();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Informant")
	void OnLightJoint();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Informant")
	void OnHighChanged(bool bIsHigh);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Informant")
	void OnSpeak(const FText& Text, bool bPrecise);

protected:
	int32 FindCurrentHint() const;
	void GiveHint(AActor* Listener, bool bPrecise);
	void EndHigh();

	bool bHigh = false;
	bool bTalkedOnce = false;
	FTimerHandle HighTimer;
};
