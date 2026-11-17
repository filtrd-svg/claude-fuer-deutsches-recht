#include "KemuriScooter.h"
#include "KemuriGlassesComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"

#define LOCTEXT_NAMESPACE "Kemuri"

AKemuriScooter::AKemuriScooter()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetBoxExtent(FVector(60.f, 18.f, 40.f));
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SetRootComponent(Collision);

	LeanPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeanPivot"));
	LeanPivot->SetupAttachment(Collision);
	LeanPivot->SetRelativeLocation(FVector(0.f, 0.f, -40.f));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(LeanPivot);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Headlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Headlight"));
	Headlight->SetupAttachment(LeanPivot);
	Headlight->SetRelativeLocation(FVector(45.f, 0.f, 95.f));
	Headlight->SetRelativeRotation(FRotator(-6.f, 0.f, 0.f));
	Headlight->IntensityUnits = ELightUnits::Candelas;
	Headlight->Intensity = 1200.f;
	Headlight->AttenuationRadius = 4500.f;
	Headlight->OuterConeAngle = 28.f;
	Headlight->InnerConeAngle = 14.f;
	Headlight->SetVisibility(false);

	RiderCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("RiderCamera"));
	RiderCamera->SetupAttachment(Collision);
	RiderCamera->SetRelativeLocation(FVector(-15.f, 0.f, 125.f));
	RiderCamera->bUsePawnControlRotation = false;

	MotorAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("MotorAudio"));
	MotorAudio->SetupAttachment(Collision);
	MotorAudio->bAutoActivate = false;

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

// ---------- Interaktion ----------

bool AKemuriScooter::CanInteract_Implementation(AActor* Interactor)
{
	return Rider == nullptr && Cast<ACharacter>(Interactor) != nullptr;
}

FText AKemuriScooter::GetInteractPrompt_Implementation(AActor* Interactor)
{
	return FText::Format(LOCTEXT("ScooterMount", "Aufsteigen  [Akku {0} %]"), FText::AsNumber(FMath::RoundToInt(Battery)));
}

void AKemuriScooter::Interact_Implementation(AActor* Interactor)
{
	Mount(Cast<ACharacter>(Interactor));
}

void AKemuriScooter::Mount(ACharacter* InRider)
{
	if (!InRider || Rider)
	{
		return;
	}
	AController* RiderController = InRider->GetController();
	if (!RiderController)
	{
		return;
	}

	Rider = InRider;
	if (UKemuriGlassesComponent* Glasses = UKemuriGlassesComponent::FindOn(Rider))
	{
		Glasses->SetCameraMode(false); // IR-Bild liegt auf der Kopfkamera, nicht auf dem Scooter.
	}
	Rider->GetCharacterMovement()->DisableMovement();
	Rider->SetActorEnableCollision(false);
	Rider->SetActorHiddenInGame(true);
	Rider->AttachToComponent(LeanPivot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	LookYaw = 0.f;
	LookPitch = 0.f;
	RiderController->Possess(this);
	Headlight->SetVisibility(true);
	MotorAudio->Play();
	OnMounted(Rider);
}

void AKemuriScooter::Dismount()
{
	if (!Rider)
	{
		return;
	}
	ACharacter* OldRider = Rider;
	AController* MyController = GetController();

	FVector ExitLoc = GetActorLocation() + GetActorRightVector() * 110.f + FVector(0.f, 0.f, 60.f);
	const FRotator ExitRot(0.f, GetActorRotation().Yaw, 0.f);
	GetWorld()->FindTeleportSpot(OldRider, ExitLoc, ExitRot);

	OldRider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OldRider->SetActorLocationAndRotation(ExitLoc, ExitRot);
	OldRider->SetActorHiddenInGame(false);
	OldRider->SetActorEnableCollision(true);
	OldRider->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	Rider = nullptr;
	Throttle = 0.f;
	Steer = 0.f;
	bBraking = false;
	MotorAudio->Stop();

	if (MyController)
	{
		MyController->Possess(OldRider);
		MyController->SetControlRotation(ExitRot);
	}
	OnDismounted(OldRider);
}

// ---------- Eingabe ----------

void AKemuriScooter::NotifyControllerChanged()
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

void AKemuriScooter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}
	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKemuriScooter::InputMove);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &AKemuriScooter::InputMoveReleased);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKemuriScooter::InputLook);
	}
	if (BrakeAction)
	{
		EIC->BindAction(BrakeAction, ETriggerEvent::Started, this, &AKemuriScooter::InputBrakePressed);
		EIC->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AKemuriScooter::InputBrakeReleased);
	}
	if (DismountAction)
	{
		EIC->BindAction(DismountAction, ETriggerEvent::Started, this, &AKemuriScooter::InputDismount);
	}
	if (LightAction)
	{
		EIC->BindAction(LightAction, ETriggerEvent::Started, this, &AKemuriScooter::InputToggleLight);
	}
}

void AKemuriScooter::InputMove(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	Throttle = FMath::Clamp(V.Y, -1.f, 1.f);
	Steer = FMath::Clamp(V.X, -1.f, 1.f);
}

void AKemuriScooter::InputMoveReleased(const FInputActionValue& Value)
{
	Throttle = 0.f;
	Steer = 0.f;
}

void AKemuriScooter::InputLook(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	LookYaw = FMath::Clamp(LookYaw + V.X, -110.f, 110.f);
	LookPitch = FMath::Clamp(LookPitch - V.Y, -55.f, 40.f);
}

void AKemuriScooter::InputBrakePressed()
{
	bBraking = true;
}

void AKemuriScooter::InputBrakeReleased()
{
	bBraking = false;
}

void AKemuriScooter::InputDismount()
{
	if (FMath::Abs(Speed) < 250.f)
	{
		Dismount();
	}
}

void AKemuriScooter::InputToggleLight()
{
	Headlight->SetVisibility(!Headlight->IsVisible());
}

// ---------- Fahrphysik ----------

void AKemuriScooter::UpdateGround(float DeltaSeconds)
{
	const FVector Loc = GetActorLocation();
	const float HalfHeight = Collision->GetScaledBoxExtent().Z;
	const FVector Fwd = FRotator(0.f, GetActorRotation().Yaw, 0.f).Vector();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(KemuriScooterGround), false, this);
	if (Rider)
	{
		Params.AddIgnoredActor(Rider);
	}

	int32 Hits = 0;
	float SumZ = 0.f;
	FVector SumNormal = FVector::ZeroVector;
	for (const float Offset : { 45.f, -45.f })
	{
		const FVector Start = Loc + Fwd * Offset + FVector(0.f, 0.f, 30.f);
		const FVector End = Start - FVector(0.f, 0.f, 30.f + HalfHeight + RideHeight + GroundTraceLength);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.ImpactNormal.Z > 0.45f)
		{
			++Hits;
			SumZ += Hit.ImpactPoint.Z;
			SumNormal += Hit.ImpactNormal;
		}
	}

	if (Hits > 0)
	{
		const float TargetZ = SumZ / Hits + HalfHeight + RideHeight;
		const bool bCloseToGround = Loc.Z - TargetZ < 18.f;
		if (bCloseToGround || VerticalVelocity <= 0.f)
		{
			if (Loc.Z - TargetZ < 18.f)
			{
				bGrounded = true;
				VerticalVelocity = 0.f;
				GroundNormal = SumNormal.GetSafeNormal();
				SetActorLocation(FVector(Loc.X, Loc.Y, FMath::FInterpTo(Loc.Z, TargetZ, DeltaSeconds, 20.f)));
				return;
			}
		}
	}
	bGrounded = false;
	GroundNormal = FVector::UpVector;
	VerticalVelocity -= Gravity * DeltaSeconds;
}

void AKemuriScooter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float EffThrottle = (Rider && Battery > 0.f) ? Throttle : 0.f;
	if (Battery <= 0.f && !bBatteryEmptyNotified)
	{
		bBatteryEmptyNotified = true;
		OnBatteryEmpty();
	}

	// Geschwindigkeit
	if (bBraking)
	{
		Speed = FMath::FInterpConstantTo(Speed, 0.f, DeltaSeconds, BrakeDeceleration);
	}
	else if (EffThrottle > 0.05f)
	{
		Speed += (Speed < 0.f ? BrakeDeceleration : Acceleration * EffThrottle) * DeltaSeconds;
	}
	else if (EffThrottle < -0.05f)
	{
		Speed -= (Speed > 0.f ? BrakeDeceleration * -EffThrottle : Acceleration * 0.5f * -EffThrottle) * DeltaSeconds;
	}
	else
	{
		Speed = FMath::FInterpConstantTo(Speed, 0.f, DeltaSeconds, CoastDeceleration);
	}
	if (bGrounded)
	{
		Speed -= GetActorForwardVector().Z * SlopeInfluence * DeltaSeconds;
	}
	Speed = FMath::Clamp(Speed, -MaxReverseSpeed, MaxSpeed);
	const float SpeedAlpha = FMath::Clamp(FMath::Abs(Speed) / MaxSpeed, 0.f, 1.f);

	// Lenkung
	const float Rate = FMath::Lerp(TurnRate.X, TurnRate.Y, SpeedAlpha);
	const float YawDelta = Steer * Rate * DeltaSeconds * FMath::Clamp(FMath::Abs(Speed) / 80.f, 0.f, 1.f) * FMath::Sign(Speed);

	// Boden und Ausrichtung
	UpdateGround(DeltaSeconds);
	const FRotator Current = GetActorRotation();
	const float Yaw = Current.Yaw + YawDelta;
	FRotator Target;
	if (bGrounded)
	{
		const FVector Fwd = FVector::VectorPlaneProject(FRotator(0.f, Yaw, 0.f).Vector(), GroundNormal).GetSafeNormal();
		Target = FRotationMatrix::MakeFromXZ(Fwd, GroundNormal).Rotator();
	}
	else
	{
		Target = FRotator(Current.Pitch * 0.98f, Yaw, Current.Roll * 0.98f);
	}
	FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaSeconds, 10.f);
	NewRot.Yaw = Yaw;
	SetActorRotation(NewRot);

	// Bewegung mit Gleiten an Waenden
	const FVector Delta = GetActorForwardVector() * Speed * DeltaSeconds + FVector(0.f, 0.f, VerticalVelocity * DeltaSeconds);
	FHitResult Hit;
	AddActorWorldOffset(Delta, true, &Hit);
	if (Hit.bBlockingHit)
	{
		const float Facing = FMath::Abs(FVector::DotProduct(GetActorForwardVector(), -Hit.ImpactNormal));
		const float ImpactSpeed = FMath::Abs(Speed) * Facing;
		if (Hit.ImpactNormal.Z < 0.6f && ImpactSpeed > CrashSpeed)
		{
			OnCrash(ImpactSpeed);
		}
		const FVector Slide = FVector::VectorPlaneProject(Delta * (1.f - Hit.Time), Hit.ImpactNormal);
		AddActorWorldOffset(Slide, true);
		if (Hit.ImpactNormal.Z < 0.6f)
		{
			Speed *= (1.f - Facing * 0.8f);
		}
		if (Hit.ImpactNormal.Z > 0.6f && VerticalVelocity < 0.f)
		{
			VerticalVelocity = 0.f;
		}
	}

	// Schraeglage und Kamera
	const float TargetLean = -Steer * MaxLeanDegrees * SpeedAlpha;
	CurrentLean = FMath::FInterpTo(CurrentLean, TargetLean, DeltaSeconds, 6.f);
	LeanPivot->SetRelativeRotation(FRotator(0.f, 0.f, CurrentLean));
	if (FMath::Abs(Speed) > 200.f)
	{
		LookYaw = FMath::FInterpTo(LookYaw, 0.f, DeltaSeconds, 1.2f);
	}
	RiderCamera->SetRelativeRotation(FRotator(LookPitch, LookYaw, CurrentLean * 0.5f));

	// Motor, Akku, Laerm
	if (MotorAudio && MotorAudio->IsPlaying())
	{
		MotorAudio->SetPitchMultiplier(0.6f + SpeedAlpha * 0.9f);
		MotorAudio->SetVolumeMultiplier(0.25f + FMath::Abs(EffThrottle) * 0.75f);
	}
	Battery = FMath::Max(0.f, Battery - FMath::Abs(EffThrottle) * BatteryDrainAtFullThrottle * DeltaSeconds);

	NoiseTimer += DeltaSeconds;
	if (NoiseTimer >= 0.5f && FMath::Abs(Speed) > 50.f)
	{
		NoiseTimer = 0.f;
		UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 0.4f + 0.6f * SpeedAlpha, this, NoiseRange, FName(TEXT("Scooter")));
	}
}

#undef LOCTEXT_NAMESPACE
