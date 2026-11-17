#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KemuriTypes.h"
#include "KemuriPlayerCharacter.generated.h"

class UCameraComponent;
class USpotLightComponent;
class UKemuriInventoryComponent;
class UKemuriHealthComponent;
class UKemuriInteractionComponent;
class UKemuriGlassesComponent;
class UKemuriFearComponent;
class UKemuriWeaponHolderComponent;
class UInputMappingContext;
class UInputAction;
class UDamageType;
struct FInputActionValue;

/**
 * Spielfigur aus der Ich-Perspektive. Kein HUD: Zustand ueber Atem, Herzschlag, Bildeffekte,
 * Brille und Waffe selbst (Magazin sichtbar am Modell).
 * Im Editor: Blueprint-Kind "BP_KemuriPlayer" anlegen, Arm-Mesh, Input-Assets und Sounds zuweisen,
 * im GameMode als Default Pawn setzen.
 */
UCLASS(Blueprintable)
class KEMURI_API AKemuriPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKemuriPlayerCharacter();

	// ---------- Komponenten ----------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<USkeletalMeshComponent> Arms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriInteractionComponent> Interaction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriGlassesComponent> Glasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriFearComponent> Fear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kemuri")
	TObjectPtr<UKemuriWeaponHolderComponent> WeaponHolder;

	// ---------- Eingabe (Enhanced Input) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> MeleeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> GlassesAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Eingabe")
	TObjectPtr<UInputAction> FlashlightAction;

	// ---------- Bewegung ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float WalkSpeed = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float SprintSpeed = 520.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float StaminaDrainPerSecond = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float StaminaRegenPerSecond = 11.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float StaminaRegenDelay = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float DefaultFOV = 88.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float AimFOV = 62.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Bewegung")
	float LookSensitivity = 1.f;

	// ---------- Laerm (fuer den Stalker) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float WalkStepInterval = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float SprintStepInterval = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float WalkNoise = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float SprintNoise = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float CrouchNoise = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float StepNoiseRange = 1800.f;

	/** Eingeschaltete Taschenlampe verraet die Position (Stalker "sieht" das Licht). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float FlashlightBeaconLoudness = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float FlashlightBeaconRange = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Laerm")
	float FlashlightDrainPerSecond = 0.35f;

	// ---------- Nahkampf ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nahkampf")
	float MeleeDamage = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nahkampf")
	float MeleeRange = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kemuri|Nahkampf")
	float MeleeCooldown = 0.9f;

	// ---------- Perks ----------
	UFUNCTION(BlueprintCallable, Category = "Kemuri|Perks")
	void ApplyPerk(EKemuriPerk Perk);

	UFUNCTION(BlueprintPure, Category = "Kemuri|Perks")
	bool HasPerk(EKemuriPerk Perk) const { return Perks.Contains(Perk); }

	UFUNCTION(BlueprintPure, Category = "Kemuri")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category = "Kemuri")
	bool IsSprinting() const { return bSprinting; }

	// ---------- Blueprint-Hooks ----------
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnFootstep(bool bSprintingStep);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnMelee();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnPlayerDamaged(float Damage, float HealthPercent);

	/** Endgueltiger Tod: Ladebildschirm, Checkpoint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnPlayerDied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnSelfRevived();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnPerkApplied(EKemuriPerk Perk);

	UFUNCTION(BlueprintImplementableEvent, Category = "Kemuri")
	void OnOutOfBreath();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	void InputMove(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);
	void InputSprintStart();
	void InputSprintStop();
	void InputCrouch();
	void InputFireStart();
	void InputFireStop();
	void InputAimStart();
	void InputAimStop();
	void InputReload();
	void InputInteract();
	void InputSwitchWeapon();
	void InputMelee();
	void InputGlassesStart();
	void InputGlassesStop();
	void InputFlashlight();

	UFUNCTION()
	void HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth);

	UFUNCTION()
	void HandleDied(AController* Killer, const UDamageType* DamageType);

	void DoSelfRevive();

	TSet<EKemuriPerk> Perks;
	bool bSelfReviveAvailable = false;
	float NoiseMultiplier = 1.f;

	bool bWantsSprint = false;
	bool bSprinting = false;
	bool bAiming = false;
	bool bOutOfBreath = false;
	float Stamina = 100.f;
	float StaminaRegenTimer = 0.f;
	float StepTimer = 0.f;
	float BeaconTimer = 0.f;
	double LastMeleeTime = -1000.0;
	FTimerHandle ReviveTimer;
};
