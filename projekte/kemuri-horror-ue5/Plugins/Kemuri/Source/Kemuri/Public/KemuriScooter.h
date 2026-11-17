#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KemuriInteractable.h"
#include "KemuriScooter.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UCameraComponent;
class USpotLightComponent;
class UAudioComponent;
class UInputMappingContext;
class UInputAction;
class ACharacter;
struct FInputActionValue;

/**
 * Elektro-Scooter mit eigener, robuster Fahrphysik (kein Chaos-Vehicle noetig):
 * Bodenabtastung, Ausrichtung an Hang und Kurven, Schraeglage, Kollision mit Gleiten,
 * Akku, Scheinwerfer, Motorgeraeusch (lockt den Stalker an).
 * Steuerung: W/S Gas/Bremse, A/D lenken, Leertaste Vollbremsung, E absteigen, F Licht.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriScooter : public APawn, public IKemuriInteractable
{
	GENERATED_BODY()

public:
	AKemuriScooter();

	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Scooter")
	void Mount(ACharacter* InRider);

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Scooter")
	void Dismount();

	UFUNCTION(BlueprintPure, Category = "Kemuri|Scooter")
	ACharacter* GetRider() const { return Rider; }

	UFUNCTION(BlueprintPure, Category = "Kemuri|Scooter")
	float GetSpeedKmh() const { return Speed * 0.036f; }

	UFUNCTION(BlueprintCallable, Category = "Kemuri|Scooter")
	void AddBattery(float Percent) { Battery = FMath::Clamp(Battery + Percent, 0.f, 100.f); }

	// ---------- Komponenten ----------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<UBoxComponent> Collision;

	/** Dreht sich fuer die Schraeglage; Body-Mesh und Licht haengen hier. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<USceneComponent> LeanPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<UCameraComponent> RiderCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<USpotLightComponent> Headlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri|Scooter")
	TObjectPtr<UAudioComponent> MotorAudio;

	// ---------- Fahrwerte ----------
	/** cm/s. 900 = ca. 32 km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float MaxSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float MaxReverseSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float Acceleration = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float BrakeDeceleration = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float CoastDeceleration = 140.f;

	/** Grad/s bei niedriger bzw. hoher Geschwindigkeit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	FVector2D TurnRate = FVector2D(110.f, 42.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float MaxLeanDegrees = 16.f;

	/** Bergauf langsamer, bergab schneller. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float SlopeInfluence = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float RideHeight = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float GroundTraceLength = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float Gravity = 1400.f;

	/** Ab dieser Aufprallgeschwindigkeit: Sturz-Event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Fahren")
	float CrashSpeed = 650.f;

	// ---------- Akku und Laerm ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Akku")
	float Battery = 100.f;

	/** Prozent pro Sekunde bei Vollgas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Akku")
	float BatteryDrainAtFullThrottle = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Akku")
	float NoiseRange = 2600.f;

	// ---------- Eingabe ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Axis2D: Y = Gas/Bremse, X = Lenken (gleiche Aktion wie Laufen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputAction> DismountAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Scooter|Eingabe")
	TObjectPtr<UInputAction> LightAction;

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Scooter")
	void OnMounted(ACharacter* NewRider);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Scooter")
	void OnDismounted(ACharacter* OldRider);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Scooter")
	void OnCrash(float ImpactSpeed);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri|Scooter")
	void OnBatteryEmpty();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	void InputMove(const FInputActionValue& Value);
	void InputMoveReleased(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);
	void InputBrakePressed();
	void InputBrakeReleased();
	void InputDismount();
	void InputToggleLight();

	void UpdateGround(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<ACharacter> Rider;

	float Speed = 0.f;
	float VerticalVelocity = 0.f;
	float Throttle = 0.f;
	float Steer = 0.f;
	bool bBraking = false;
	bool bGrounded = false;
	bool bBatteryEmptyNotified = false;
	float CurrentLean = 0.f;
	float LookYaw = 0.f;
	float LookPitch = 0.f;
	float NoiseTimer = 0.f;
	FVector GroundNormal = FVector::UpVector;
};
