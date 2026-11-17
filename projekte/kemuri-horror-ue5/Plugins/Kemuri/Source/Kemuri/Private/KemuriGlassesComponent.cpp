#include "KemuriGlassesComponent.h"
#include "KemuriGlassesWidget.h"
#include "KemuriWorldSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundBase.h"

UKemuriGlassesComponent::UKemuriGlassesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	Apps = {
		EKemuriGlassesApp::Camera, EKemuriGlassesApp::Map, EKemuriGlassesApp::Notes,
		EKemuriGlassesApp::Parts, EKemuriGlassesApp::Resonance, EKemuriGlassesApp::Battery };
}

UKemuriGlassesComponent* UKemuriGlassesComponent::FindOn(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UKemuriGlassesComponent>() : nullptr;
}

void UKemuriGlassesComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ViewCamera)
	{
		SetViewCamera(GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr);
	}
	EnsureWidget();
}

void UKemuriGlassesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCameraMode)
	{
		if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
		{
			Sub->SetIRActive(false);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void UKemuriGlassesComponent::SetViewCamera(UCameraComponent* InCamera)
{
	ViewCamera = InCamera;
	if (!ViewCamera || !GetOwner())
	{
		return;
	}

	// Unsichtbarer IR-Strahler: nur im Kameramodus an, damit dunkle Raeume im IR-Bild erscheinen.
	if (!IRIlluminator)
	{
		IRIlluminator = NewObject<USpotLightComponent>(GetOwner());
		IRIlluminator->SetupAttachment(ViewCamera);
		IRIlluminator->RegisterComponent();
	}
	else
	{
		IRIlluminator->AttachToComponent(ViewCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	IRIlluminator->SetIntensityUnits(ELightUnits::Candelas);
	IRIlluminator->SetIntensity(IRIlluminatorIntensity);
	IRIlluminator->SetAttenuationRadius(IRIlluminatorRange);
	IRIlluminator->SetInnerConeAngle(18.f);
	IRIlluminator->SetOuterConeAngle(38.f);
	IRIlluminator->SetCastShadows(true);
	IRIlluminator->SetVisibility(bCameraMode);

	if (IRPostProcessMaterial && IRBlendableIndex == INDEX_NONE)
	{
		ViewCamera->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(0.f, IRPostProcessMaterial));
		IRBlendableIndex = ViewCamera->PostProcessSettings.WeightedBlendables.Array.Num() - 1;
	}
}

void UKemuriGlassesComponent::EnsureWidget()
{
	if (Widget || !WidgetClass)
	{
		return;
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		Widget = CreateWidget<UKemuriGlassesWidget>(PC, WidgetClass);
		if (Widget)
		{
			Widget->AddToViewport(50);
			Widget->OnBatteryChanged(Battery);
		}
	}
}

void UKemuriGlassesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Widget)
	{
		EnsureWidget();
	}

	// T gehalten -> Menue
	if (bInputHeld && !bMenuOpen)
	{
		HoldTime += DeltaTime;
		if (HoldTime >= HoldThreshold)
		{
			OpenMenu();
		}
	}

	// Akku im Kameramodus
	if (bCameraMode)
	{
		if (!DrainBattery(CameraDrainPerSecond * DeltaTime))
		{
			SetCameraMode(false);
		}
	}

	// Resonanz-Scanner: Pieptoene, die naeher am Ziel schneller werden
	if (ScannerTimeLeft > 0.f)
	{
		ScannerTimeLeft -= DeltaTime;
		ScannerNextPing -= DeltaTime;
		if (ScannerNextPing <= 0.f)
		{
			ScannerPing();
		}
	}
}

// ---------- Eingabe ----------

void UKemuriGlassesComponent::GlassesInputStarted()
{
	bInputHeld = true;
	HoldTime = 0.f;
}

void UKemuriGlassesComponent::GlassesInputCompleted()
{
	bInputHeld = false;
	if (bMenuOpen)
	{
		CloseMenu(true);
	}
	else
	{
		ToggleCameraMode();
	}
	HoldTime = 0.f;
}

bool UKemuriGlassesComponent::ConsumeLookInput(float DeltaX)
{
	if (!bMenuOpen || Apps.Num() == 0)
	{
		return false;
	}
	MenuLookAccum += DeltaX;
	if (FMath::Abs(MenuLookAccum) >= MenuStepThreshold)
	{
		const int32 Step = MenuLookAccum > 0.f ? 1 : -1;
		SelectedIndex = (SelectedIndex + Step + Apps.Num()) % Apps.Num();
		MenuLookAccum = 0.f;
		PlayUISound(MenuTickSound);
		if (Widget)
		{
			Widget->OnMenuSelectionChanged(SelectedIndex);
		}
	}
	return true;
}

void UKemuriGlassesComponent::OpenMenu()
{
	bMenuOpen = true;
	MenuLookAccum = 0.f;
	SelectedIndex = FMath::Clamp(SelectedIndex, 0, FMath::Max(0, Apps.Num() - 1));
	PlayUISound(MenuTickSound);
	if (Widget)
	{
		Widget->OnMenuOpened(Apps, SelectedIndex);
	}
}

void UKemuriGlassesComponent::CloseMenu(bool bConfirm)
{
	bMenuOpen = false;
	if (Widget)
	{
		Widget->OnMenuClosed();
	}
	if (bConfirm && Apps.IsValidIndex(SelectedIndex))
	{
		const EKemuriGlassesApp App = Apps[SelectedIndex];
		ActivateApp(App);
		OnAppActivated.Broadcast(App);
		if (Widget)
		{
			Widget->OnAppActivated(App);
		}
	}
}

void UKemuriGlassesComponent::ActivateApp_Implementation(EKemuriGlassesApp App)
{
	switch (App)
	{
	case EKemuriGlassesApp::Camera:
		ToggleCameraMode();
		break;
	case EKemuriGlassesApp::Resonance:
		if (DrainBattery(ScannerBatteryCost))
		{
			ScannerTimeLeft = ScannerDuration;
			ScannerNextPing = 0.f;
		}
		break;
	default:
		// Karte, Hinweise, Bauteile, Akku: Darstellung im Widget (OnAppActivated).
		break;
	}
}

// ---------- Kameramodus ----------

void UKemuriGlassesComponent::SetCameraMode(bool bEnable)
{
	if (bEnable && Battery <= 0.f)
	{
		bEnable = false;
	}
	if (bCameraMode == bEnable)
	{
		return;
	}
	bCameraMode = bEnable;
	ApplyCameraModeVisuals();
	PlayUISound(bCameraMode ? CameraOnSound : CameraOffSound);

	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->SetIRActive(bCameraMode);
	}
	if (Widget)
	{
		Widget->OnCameraModeChanged(bCameraMode);
	}
	OnCameraModeChanged.Broadcast(bCameraMode);
}

void UKemuriGlassesComponent::ApplyCameraModeVisuals()
{
	if (IRIlluminator)
	{
		IRIlluminator->SetVisibility(bCameraMode);
	}
	if (ViewCamera && ViewCamera->PostProcessSettings.WeightedBlendables.Array.IsValidIndex(IRBlendableIndex))
	{
		ViewCamera->PostProcessSettings.WeightedBlendables.Array[IRBlendableIndex].Weight = bCameraMode ? 1.f : 0.f;
	}
}

// ---------- Akku ----------

void UKemuriGlassesComponent::AddBattery(float Percent)
{
	Battery = FMath::Clamp(Battery + Percent, 0.f, 100.f);
	const bool bCritical = Battery <= CriticalBatteryThreshold;
	if (Widget)
	{
		Widget->OnBatteryChanged(Battery);
		if (bCritical != bBatteryCritical)
		{
			Widget->OnBatteryCritical(bCritical);
		}
	}
	bBatteryCritical = bCritical;
}

bool UKemuriGlassesComponent::DrainBattery(float Percent)
{
	if (Battery <= 0.f)
	{
		return false;
	}
	const int32 OldWhole = FMath::FloorToInt(Battery);
	Battery = FMath::Max(0.f, Battery - Percent);
	const bool bCritical = Battery <= CriticalBatteryThreshold;
	if (Widget && (FMath::FloorToInt(Battery) != OldWhole || bCritical != bBatteryCritical))
	{
		Widget->OnBatteryChanged(Battery);
		if (bCritical != bBatteryCritical)
		{
			Widget->OnBatteryCritical(bCritical);
		}
	}
	bBatteryCritical = bCritical;
	return Battery > 0.f;
}

// ---------- Anzeige ----------

void UKemuriGlassesComponent::ShowSubtitle(const FText& Speaker, const FText& Text, float Duration)
{
	if (Widget)
	{
		Widget->OnSubtitle(Speaker, Text, Duration);
	}
}

void UKemuriGlassesComponent::ShowPrompt(const FText& Prompt)
{
	if (Widget)
	{
		Widget->OnPrompt(Prompt);
	}
}

void UKemuriGlassesComponent::LogHint(const FText& Hint)
{
	if (Hint.IsEmpty())
	{
		return;
	}
	HintLog.Add(Hint);
	if (Widget)
	{
		Widget->OnHintLogged(Hint);
	}
}

// ---------- Scanner ----------

void UKemuriGlassesComponent::ScannerPing()
{
	float Distance = -1.f;
	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	const AActor* Target = Sub && GetOwner() ? Sub->GetNearestScanTarget(GetOwner()->GetActorLocation(), ScannerRange, Distance) : nullptr;
	if (!Target)
	{
		ScannerNextPing = 2.5f;
		return;
	}
	const float Alpha = FMath::Clamp(Distance / ScannerRange, 0.f, 1.f);
	ScannerNextPing = FMath::Lerp(0.15f, 2.0f, Alpha);
	if (ScannerPingSound)
	{
		UGameplayStatics::PlaySound2D(this, ScannerPingSound, FMath::Lerp(1.f, 0.35f, Alpha), FMath::Lerp(1.4f, 0.8f, Alpha));
	}
}

void UKemuriGlassesComponent::PlayUISound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
