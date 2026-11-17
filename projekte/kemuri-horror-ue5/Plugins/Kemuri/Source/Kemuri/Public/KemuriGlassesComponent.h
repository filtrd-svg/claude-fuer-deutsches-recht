#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KemuriTypes.h"
#include "KemuriGlassesComponent.generated.h"

class UKemuriGlassesWidget;
class UCameraComponent;
class USpotLightComponent;
class UMaterialInterface;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriCameraModeChanged, bool, bCameraMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriAppActivated, EKemuriGlassesApp, App);

/**
 * Die AI-Brille des Spielers.
 * - T kurz tippen: Kameramodus (IR) an/aus.
 * - T halten: App-Menue. Maus links/rechts waehlt, T loslassen startet die App.
 * - IR-Kamera: Post-Process-Material + unsichtbarer IR-Strahler, macht IR-Objekte sichtbar
 *   (siehe UKemuriIRRevealComponent). Verbraucht Akku.
 * - Live-Uebersetzung, Prompts und Hinweise laufen ueber das Brillen-Widget.
 */
UCLASS(ClassGroup = (Kemuri), meta = (BlueprintSpawnableComponent))
class KEMURI_API UKemuriGlassesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKemuriGlassesComponent();

	static UKemuriGlassesComponent* FindOn(const AActor* Actor);

	// ---------- Eingabe (vom Charakter aufgerufen) ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void GlassesInputStarted();

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void GlassesInputCompleted();

	/** Mausbewegung waehrend das Menue offen ist. Gibt true zurueck, wenn die Eingabe verbraucht wurde. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	bool ConsumeLookInput(float DeltaX);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Brille")
	bool IsMenuOpen() const { return bMenuOpen; }

	// ---------- Kameramodus ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void SetCameraMode(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void ToggleCameraMode() { SetCameraMode(!bCameraMode); }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Brille")
	bool IsCameraMode() const { return bCameraMode; }

	// ---------- Akku ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void AddBattery(float Percent);

	/** Zusaetzlicher Verbrauch durch andere Geraete (Taschenlampe). */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	bool DrainBattery(float Percent);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Brille")
	float GetBattery() const { return Battery; }

	// ---------- Anzeige ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void ShowSubtitle(const FText& Speaker, const FText& Text, float Duration = 4.f);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void ShowPrompt(const FText& Prompt);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void LogHint(const FText& Hint);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Brille")
	TArray<FText> GetHintLog() const { return HintLog; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Brille")
	UKemuriGlassesWidget* GetWidget() const { return Widget; }

	/** Kamera des Spielers, auf die Post-Process und IR-Strahler wirken. Wird vom Charakter gesetzt. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Brille")
	void SetViewCamera(UCameraComponent* InCamera);

	// ---------- Einstellungen ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	TSubclassOf<UKemuriGlassesWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	TArray<EKemuriGlassesApp> Apps;

	/** Ab dieser Haltedauer oeffnet T das Menue statt den Kameramodus umzuschalten. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	float HoldThreshold = 0.28f;

	/** Mausweg, der einen Menue-Schritt ausloest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	float MenuStepThreshold = 6.f;

	/** IR-Post-Process-Material (Domain Post Process). Rezept siehe docs/04. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|IR")
	TObjectPtr<UMaterialInterface> IRPostProcessMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|IR")
	float IRIlluminatorIntensity = 1500.f; // Candela

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|IR")
	float IRIlluminatorRange = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Akku", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Battery = 100.f;

	/** Prozent pro Sekunde im Kameramodus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Akku")
	float CameraDrainPerSecond = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Akku")
	float CriticalBatteryThreshold = 12.f;

	// ---------- Resonanz-Scanner (nur Ton, keine Anzeige) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Scanner")
	TObjectPtr<USoundBase> ScannerPingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Scanner")
	float ScannerRange = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Scanner")
	float ScannerDuration = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille|Scanner")
	float ScannerBatteryCost = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	TObjectPtr<USoundBase> MenuTickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	TObjectPtr<USoundBase> CameraOnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Brille")
	TObjectPtr<USoundBase> CameraOffSound;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Brille")
	FKemuriCameraModeChanged OnCameraModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Brille")
	FKemuriAppActivated OnAppActivated;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Standardverhalten der Apps; in Blueprints ueberschreibbar. */
	UFUNCTION(BlueprintNativeEvent, Category = "Kemuri|Brille")
	void ActivateApp(EKemuriGlassesApp App);
	virtual void ActivateApp_Implementation(EKemuriGlassesApp App);

private:
	void OpenMenu();
	void CloseMenu(bool bConfirm);
	void EnsureWidget();
	void ApplyCameraModeVisuals();
	void ScannerPing();
	void PlayUISound(USoundBase* Sound) const;

	UPROPERTY()
	TObjectPtr<UKemuriGlassesWidget> Widget;

	UPROPERTY()
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> IRIlluminator;

	TArray<FText> HintLog;

	bool bInputHeld = false;
	bool bMenuOpen = false;
	bool bCameraMode = false;
	bool bBatteryCritical = false;
	float HoldTime = 0.f;
	float MenuLookAccum = 0.f;
	int32 SelectedIndex = 0;
	int32 IRBlendableIndex = INDEX_NONE;

	float ScannerTimeLeft = 0.f;
	float ScannerNextPing = 0.f;
};
