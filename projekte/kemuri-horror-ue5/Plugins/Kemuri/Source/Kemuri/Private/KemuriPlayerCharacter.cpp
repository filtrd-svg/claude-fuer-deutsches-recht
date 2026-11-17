#include "KemuriPlayerCharacter.h"
#include "KemuriInventoryComponent.h"
#include "KemuriHealthComponent.h"
#include "KemuriInteractionComponent.h"
#include "KemuriGlassesComponent.h"
#include "KemuriAtmosphereComponents.h"
#include "KemuriWeaponHolderComponent.h"
#include "KemuriEnemies.h"
#include "KemuriDamageTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"

AKemuriPlayerCharacter::AKemuriPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 92.f);
	BaseEyeHeight = 70.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-5.f, 0.f, 70.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(DefaultFOV);

	Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	Arms->SetupAttachment(FirstPersonCamera);
	Arms->SetOnlyOwnerSee(true);
	Arms->SetCastShadow(false);
	Arms->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetOwnerNoSee(true);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FirstPersonCamera);
	Flashlight->SetRelativeLocation(FVector(10.f, 18.f, -12.f));
	Flashlight->IntensityUnits = ELightUnits::Candelas;
	Flashlight->Intensity = 450.f;
	Flashlight->AttenuationRadius = 2400.f;
	Flashlight->InnerConeAngle = 12.f;
	Flashlight->OuterConeAngle = 30.f;
	Flashlight->SetVisibility(false);

	Inventory = CreateDefaultSubobject<UKemuriInventoryComponent>(TEXT("Inventory"));
	Health = CreateDefaultSubobject<UKemuriHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 100.f;
	Health->bRegenerates = true;
	Interaction = CreateDefaultSubobject<UKemuriInteractionComponent>(TEXT("Interaction"));
	Glasses = CreateDefaultSubobject<UKemuriGlassesComponent>(TEXT("Glasses"));
	Fear = CreateDefaultSubobject<UKemuriFearComponent>(TEXT("Fear"));
	WeaponHolder = CreateDefaultSubobject<UKemuriWeaponHolderComponent>(TEXT("WeaponHolder"));

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = 150.f;
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->BrakingDecelerationWalking = 1600.f;
	Move->AirControl = 0.2f;
}

void AKemuriPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Vor BeginPlay der Komponenten: Waffen am Arm-Mesh (Socket GripPoint) oder an der Kamera befestigen.
	if (WeaponHolder)
	{
		USceneComponent* Parent = FirstPersonCamera.Get();
		if (Arms && Arms->GetSkeletalMeshAsset())
		{
			Parent = Arms.Get();
		}
		WeaponHolder->SetAttachParent(Parent);
	}
	// Die Brille findet die Kamera in ihrem BeginPlay selbst (einzige Kamera am Spieler).
}

void AKemuriPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Stamina = MaxStamina;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	Health->OnDamaged.AddDynamic(this, &AKemuriPlayerCharacter::HandleDamaged);
	Health->OnDied.AddDynamic(this, &AKemuriPlayerCharacter::HandleDied);
}

void AKemuriPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void AKemuriPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}
	auto BindStarted = [EIC, this](UInputAction* Action, void (AKemuriPlayerCharacter::*Func)())
	{
		if (Action)
		{
			EIC->BindAction(Action, ETriggerEvent::Started, this, Func);
		}
	};
	auto BindCompleted = [EIC, this](UInputAction* Action, void (AKemuriPlayerCharacter::*Func)())
	{
		if (Action)
		{
			EIC->BindAction(Action, ETriggerEvent::Completed, this, Func);
		}
	};

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKemuriPlayerCharacter::InputMove);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKemuriPlayerCharacter::InputLook);
	}
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	BindStarted(SprintAction, &AKemuriPlayerCharacter::InputSprintStart);
	BindCompleted(SprintAction, &AKemuriPlayerCharacter::InputSprintStop);
	BindStarted(CrouchAction, &AKemuriPlayerCharacter::InputCrouch);
	BindStarted(FireAction, &AKemuriPlayerCharacter::InputFireStart);
	BindCompleted(FireAction, &AKemuriPlayerCharacter::InputFireStop);
	BindStarted(AimAction, &AKemuriPlayerCharacter::InputAimStart);
	BindCompleted(AimAction, &AKemuriPlayerCharacter::InputAimStop);
	BindStarted(ReloadAction, &AKemuriPlayerCharacter::InputReload);
	BindStarted(InteractAction, &AKemuriPlayerCharacter::InputInteract);
	BindStarted(SwitchWeaponAction, &AKemuriPlayerCharacter::InputSwitchWeapon);
	BindStarted(MeleeAction, &AKemuriPlayerCharacter::InputMelee);
	BindStarted(GlassesAction, &AKemuriPlayerCharacter::InputGlassesStart);
	BindCompleted(GlassesAction, &AKemuriPlayerCharacter::InputGlassesStop);
	BindStarted(FlashlightAction, &AKemuriPlayerCharacter::InputFlashlight);
}

// ---------- Eingabe ----------

void AKemuriPlayerCharacter::InputMove(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), V.Y);
	AddMovementInput(GetActorRightVector(), V.X);
}

void AKemuriPlayerCharacter::InputLook(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>() * LookSensitivity;
	// Offenes Brillen-Menue: Maus waehlt die App, Blick bleibt fast stehen.
	if (Glasses && Glasses->ConsumeLookInput(V.X))
	{
		AddControllerYawInput(V.X * 0.1f);
		AddControllerPitchInput(V.Y * 0.1f);
		return;
	}
	AddControllerYawInput(V.X);
	AddControllerPitchInput(V.Y);
}

void AKemuriPlayerCharacter::InputSprintStart()
{
	bWantsSprint = true;
}

void AKemuriPlayerCharacter::InputSprintStop()
{
	bWantsSprint = false;
}

void AKemuriPlayerCharacter::InputCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AKemuriPlayerCharacter::InputFireStart()
{
	if (!bSprinting && WeaponHolder)
	{
		WeaponHolder->StartFire();
	}
}

void AKemuriPlayerCharacter::InputFireStop()
{
	if (WeaponHolder)
	{
		WeaponHolder->StopFire();
	}
}

void AKemuriPlayerCharacter::InputAimStart()
{
	bAiming = true;
}

void AKemuriPlayerCharacter::InputAimStop()
{
	bAiming = false;
}

void AKemuriPlayerCharacter::InputReload()
{
	if (WeaponHolder)
	{
		WeaponHolder->Reload();
	}
}

void AKemuriPlayerCharacter::InputInteract()
{
	if (Interaction)
	{
		Interaction->TryInteract();
	}
}

void AKemuriPlayerCharacter::InputSwitchWeapon()
{
	if (WeaponHolder)
	{
		WeaponHolder->NextWeapon();
	}
}

void AKemuriPlayerCharacter::InputGlassesStart()
{
	if (Glasses)
	{
		Glasses->GlassesInputStarted();
	}
}

void AKemuriPlayerCharacter::InputGlassesStop()
{
	if (Glasses)
	{
		Glasses->GlassesInputCompleted();
	}
}

void AKemuriPlayerCharacter::InputFlashlight()
{
	if (!Flashlight)
	{
		return;
	}
	const bool bOn = !Flashlight->IsVisible();
	if (bOn && Glasses && Glasses->GetBattery() <= 0.f)
	{
		return; // Powerbank leer.
	}
	Flashlight->SetVisibility(bOn);
}

void AKemuriPlayerCharacter::InputMelee()
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastMeleeTime < MeleeCooldown)
	{
		return;
	}
	LastMeleeTime = Now;
	OnMelee();

	FVector ViewLoc;
	FRotator ViewRot;
	if (!GetController())
	{
		return;
	}
	GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KemuriMelee), false, this);
	if (GetWorld()->SweepSingleByChannel(Hit, ViewLoc, ViewLoc + ViewRot.Vector() * MeleeRange, FQuat::Identity,
		ECC_Visibility, FCollisionShape::MakeSphere(20.f), Params) && Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), MeleeDamage, ViewRot.Vector(), Hit, GetController(), this, UKemuriMeleeDamageType::StaticClass());
	}
	UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 0.3f * NoiseMultiplier, this, 900.f, FName(TEXT("Nahkampf")));
}

// ---------- Tick: Ausdauer, Laerm, Taschenlampe, Sichtfeld ----------

void AKemuriPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UCharacterMovementComponent* Move = GetCharacterMovement();
	const float GroundSpeed = GetVelocity().Size2D();
	const bool bMoving = GroundSpeed > 20.f && Move->IsMovingOnGround();

	// Sprinten mit Ausdauer. Keine Anzeige: Atem und Herzschlag sagen alles.
	const bool bCanSprint = bWantsSprint && bMoving && !bIsCrouched && !bOutOfBreath && !bAiming;
	bSprinting = bCanSprint;
	if (bSprinting)
	{
		Stamina = FMath::Max(0.f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
		StaminaRegenTimer = 0.f;
		if (Stamina <= 0.f)
		{
			bOutOfBreath = true;
			OnOutOfBreath();
			if (Fear)
			{
				Fear->AddScareSpike(0.25f);
			}
		}
	}
	else
	{
		StaminaRegenTimer += DeltaSeconds;
		if (StaminaRegenTimer >= StaminaRegenDelay)
		{
			Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * DeltaSeconds);
		}
		if (bOutOfBreath && Stamina >= MaxStamina * 0.35f)
		{
			bOutOfBreath = false;
		}
	}
	Move->MaxWalkSpeed = bSprinting ? SprintSpeed : (bAiming ? WalkSpeed * 0.6f : WalkSpeed);

	// Schritte -> Geraeusch fuer die KI.
	if (bMoving)
	{
		StepTimer += DeltaSeconds;
		const float Interval = bSprinting ? SprintStepInterval : WalkStepInterval;
		if (StepTimer >= Interval)
		{
			StepTimer = 0.f;
			const float Loudness = (bIsCrouched ? CrouchNoise : (bSprinting ? SprintNoise : WalkNoise)) * NoiseMultiplier;
			UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), Loudness, this, StepNoiseRange, FName(TEXT("Schritt")));
			OnFootstep(bSprinting);
		}
	}

	// Taschenlampe: zieht Akku aus der Powerbank und verraet die Position.
	if (Flashlight && Flashlight->IsVisible())
	{
		if (Glasses && !Glasses->DrainBattery(FlashlightDrainPerSecond * DeltaSeconds))
		{
			Flashlight->SetVisibility(false);
		}
		BeaconTimer += DeltaSeconds;
		if (BeaconTimer >= 1.5f)
		{
			BeaconTimer = 0.f;
			UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), FlashlightBeaconLoudness, this, FlashlightBeaconRange, FName(TEXT("Licht")));
		}
	}

	// Zielen = engeres Sichtfeld.
	const float TargetFOV = bAiming ? AimFOV : DefaultFOV;
	FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(FirstPersonCamera->FieldOfView, TargetFOV, DeltaSeconds, 10.f));
}

// ---------- Perks ----------

void AKemuriPlayerCharacter::ApplyPerk(EKemuriPerk Perk)
{
	if (Perks.Contains(Perk))
	{
		return;
	}
	Perks.Add(Perk);
	switch (Perk)
	{
	case EKemuriPerk::TetsuCha:
		Health->SetMaxHealth(250.f, true);
		break;
	case EKemuriPerk::HayaiKohi:
		WeaponHolder->SetReloadSpeedMultiplier(0.5f);
		break;
	case EKemuriPerk::MittsuRamune:
		WeaponHolder->SetMaxSlots(3);
		break;
	case EKemuriPerk::MamoriMizu:
		bSelfReviveAvailable = true;
		break;
	case EKemuriPerk::KazeSoda:
		SprintSpeed *= 1.15f;
		NoiseMultiplier = 0.4f;
		break;
	}
	OnPerkApplied(Perk);
}

// ---------- Schaden und Tod ----------

void AKemuriPlayerCharacter::HandleDamaged(float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, float NewHealth)
{
	if (Fear)
	{
		Fear->AddScareSpike(FMath::Clamp(Damage / 100.f, 0.1f, 0.6f));
	}
	OnPlayerDamaged(Damage, Health->GetHealthPercent());
}

void AKemuriPlayerCharacter::HandleDied(AController* Killer, const UDamageType* DamageType)
{
	if (WeaponHolder)
	{
		WeaponHolder->StopFire();
	}
	if (bSelfReviveAvailable)
	{
		bSelfReviveAvailable = false;
		Perks.Remove(EKemuriPerk::MamoriMizu);
		GetWorldTimerManager().SetTimer(ReviveTimer, this, &AKemuriPlayerCharacter::DoSelfRevive, 2.5f, false);
		return;
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	OnPlayerDied();
}

void AKemuriPlayerCharacter::DoSelfRevive()
{
	Health->Revive(0.6f);
	OnSelfRevived();
}
