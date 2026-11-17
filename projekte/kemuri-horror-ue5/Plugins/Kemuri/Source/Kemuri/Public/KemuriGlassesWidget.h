#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KemuriTypes.h"
#include "KemuriGlassesWidget.generated.h"

/**
 * Basisklasse fuer das Brillen-Display. Es gibt kein HUD: Alles, was der Spieler an
 * Text sieht, ist eine AR-Einblendung der Brille (Uebersetzung, Prompts, Kameramodus, Menue).
 * Im Editor: Widget Blueprint "WBP_KemuriGlasses" anlegen, Parent Class = KemuriGlassesWidget,
 * und die Events unten gestalten. Das Widget ist im Normalzustand leer und transparent.
 */
UCLASS(Abstract, Blueprintable)
class KEMURI_API UKemuriGlassesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** T gehalten: App-Menue einblenden. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnMenuOpened(const TArray<EKemuriGlassesApp>& Apps, int32 SelectedIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnMenuSelectionChanged(int32 SelectedIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnMenuClosed();

	/** App wurde gestartet: Karte, Hinweise, Bauteile, Akku zeigen hier eine Seite fuer einige Sekunden. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnAppActivated(EKemuriGlassesApp App);

	/** Kameramodus (IR) an/aus: REC-Punkt, Zeitstempel, Rauschen, Linsenverzerrung. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnCameraModeChanged(bool bCameraMode);

	/** Live-Uebersetzung: Untertitel fuer japanische Sprache. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnSubtitle(const FText& Speaker, const FText& Text, float Duration);

	/** Interaktions-Prompt. Leerer Text = ausblenden. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnPrompt(const FText& Prompt);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnHintLogged(const FText& Hint);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnBatteryChanged(float BatteryPercent);

	/** Akku fast leer: Bildfehler, Flackern, Ausfaelle. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Brille")
	void OnBatteryCritical(bool bCritical);
};
