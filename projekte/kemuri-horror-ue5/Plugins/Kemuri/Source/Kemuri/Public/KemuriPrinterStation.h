#pragma once

#include "CoreMinimal.h"
#include "KemuriInteractableActor.h"
#include "KemuriTypes.h"
#include "KemuriPrinterStation.generated.h"

class UKemuriWeaponData;
class AKemuriFogDirector;
class UAudioComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKemuriPrinterStateChanged, EKemuriPrinterState, NewState);

/**
 * Der 3D-Drucker. Ablauf: Bauteile einsetzen -> kalibrieren (Minispiel im Blueprint) ->
 * Spezialfilament einlegen -> drucken (Belagerung, Strom halten) -> HARAI-Waffe entnehmen.
 * Der Druckkopf faehrt prozedural im Infill-Zickzack, Schicht fuer Schicht; das gedruckte Teil
 * waechst ueber den Materialparameter "PrintProgress" (Rezept in docs/04).
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriPrinterStation : public AKemuriInteractableActor
{
	GENERATED_BODY()

public:
	AKemuriPrinterStation();

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	/** Vom Kalibrier-Minispiel im Blueprint aufrufen, wenn alle vier Ecken justiert sind. */
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Drucker")
	void CompleteCalibration();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Drucker")
	EKemuriPrinterState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Drucker")
	float GetPrintProgress() const { return PrintProgress; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Drucker")
	TArray<FName> GetMissingParts() const;

	// ---------- Komponenten ----------
	/** Hier den Hotend-/Druckkopf-Mesh anhaengen. Wird prozedural bewegt. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Drucker")
	TObjectPtr<USceneComponent> PrintheadAnchor;

	/** Ursprung des Druckbetts (linke vordere Ecke, Oberseite). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Drucker")
	TObjectPtr<USceneComponent> BedOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Drucker")
	TObjectPtr<UStaticMeshComponent> PrintedPart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Drucker")
	TObjectPtr<UAudioComponent> MotorAudio;

	// ---------- Einstellungen ----------
	/** Item-IDs der Bauteile, z. B. Bauplan_1..3, Mainboard, Netzteil, Hotend, Schrittmotoren, Druckbett. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	TArray<FName> RequiredParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	FName FilamentItem = TEXT("Filament_Shimenawa");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	TObjectPtr<UKemuriWeaponData> FinalWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	float PrintDuration = 180.f;

	/** Ohne dieses Flag pausiert der Druck (Generator, Stromkasten). Leer = immer Strom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	FName PowerFlag = TEXT("Strom_An");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	TObjectPtr<AKemuriFogDirector> FogDirector;

	/** Alle X Sekunden wird der Stalker zum Drucker gelockt. 0 = nie. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker")
	float LureStalkerInterval = 35.f;

	/** Bettgroesse in cm (X, Y). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker|Bewegung")
	FVector2D BedSize = FVector2D(25.f, 25.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker|Bewegung")
	int32 LayerCount = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker|Bewegung")
	float LayerHeight = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker|Bewegung")
	int32 LinesPerLayer = 14;

	/** Abstand Duesenspitze zum Anker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Drucker|Bewegung")
	float NozzleOffsetZ = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "Kemuri|Drucker")
	FKemuriPrinterStateChanged OnStateChanged;

	// ---------- Blueprint-Hooks ----------
	/** Bauteil-Mesh einblenden, Einrast-Sound. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Drucker")
	void OnPartInstalled(FName PartId);

	/** Kalibrier-Minispiel starten (vier Rad-Schrauben, Papiertest). Danach CompleteCalibration(). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Drucker")
	void OnStartCalibration(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Drucker")
	void OnFilamentLoaded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Drucker")
	void OnPrintProgress(float Progress, int32 Layer);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Drucker")
	void OnPrintFinished();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetState(EKemuriPrinterState NewState);
	void UpdatePrinthead();
	bool HasPower() const;

	UPROPERTY(VisibleInstanceOnly, Category = "Kemuri|Drucker")
	EKemuriPrinterState State = EKemuriPrinterState::Incomplete;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PrintedPartMID;

	TSet<FName> InstalledParts;
	float PrintProgress = 0.f;
	float LureTimer = 0.f;
	int32 LastLayer = -1;
};
