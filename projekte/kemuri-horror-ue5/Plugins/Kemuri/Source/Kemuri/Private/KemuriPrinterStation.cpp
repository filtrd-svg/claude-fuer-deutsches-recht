#include "KemuriPrinterStation.h"
#include "KemuriInventoryComponent.h"
#include "KemuriWeaponHolderComponent.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriFogDirector.h"
#include "KemuriEnemies.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "Kemuri"

AKemuriPrinterStation::AKemuriPrinterStation()
{
	PrimaryActorTick.bCanEverTick = true;

	BedOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("BedOrigin"));
	BedOrigin->SetupAttachment(Root);

	PrintheadAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PrintheadAnchor"));
	PrintheadAnchor->SetupAttachment(BedOrigin);

	PrintedPart = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrintedPart"));
	PrintedPart->SetupAttachment(BedOrigin);
	PrintedPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MotorAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("MotorAudio"));
	MotorAudio->SetupAttachment(Root);
	MotorAudio->bAutoActivate = false;

	RequiredParts = {
		TEXT("Bauplan_1"), TEXT("Bauplan_2"), TEXT("Bauplan_3"),
		TEXT("Mainboard"), TEXT("Netzteil"), TEXT("Hotend"),
		TEXT("Schrittmotoren"), TEXT("Druckbett") };
}

void AKemuriPrinterStation::BeginPlay()
{
	Super::BeginPlay();
	PrintedPartMID = PrintedPart->CreateAndSetMaterialInstanceDynamic(0);
	if (PrintedPartMID)
	{
		PrintedPartMID->SetScalarParameterValue(TEXT("PrintProgress"), 0.f);
	}
	PrintedPart->SetVisibility(false);
	UpdatePrinthead();
}

TArray<FName> AKemuriPrinterStation::GetMissingParts() const
{
	TArray<FName> Missing;
	for (const FName& Part : RequiredParts)
	{
		if (!InstalledParts.Contains(Part))
		{
			Missing.Add(Part);
		}
	}
	return Missing;
}

bool AKemuriPrinterStation::HasPower() const
{
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	return !Sub || Sub->IsFlagSatisfied(PowerFlag);
}

void AKemuriPrinterStation::SetState(EKemuriPrinterState NewState)
{
	if (State == NewState)
	{
		return;
	}
	State = NewState;
	OnStateChanged.Broadcast(State);
}

bool AKemuriPrinterStation::CanInteract_Implementation(AActor* Interactor)
{
	return State != EKemuriPrinterState::Printing && State != EKemuriPrinterState::Paused && Super::CanInteract_Implementation(Interactor);
}

FText AKemuriPrinterStation::GetInteractPrompt_Implementation(AActor* Interactor)
{
	switch (State)
	{
	case EKemuriPrinterState::Incomplete:
		return FText::Format(LOCTEXT("PrinterParts", "Bauteile einsetzen ({0} fehlen)"), FText::AsNumber(GetMissingParts().Num()));
	case EKemuriPrinterState::Assembled:
		return LOCTEXT("PrinterCalibrate", "Druckbett kalibrieren");
	case EKemuriPrinterState::Calibrated:
		return LOCTEXT("PrinterFilament", "Filament einlegen");
	case EKemuriPrinterState::Ready:
		return LOCTEXT("PrinterStart", "Druck starten");
	case EKemuriPrinterState::Done:
		return LOCTEXT("PrinterTake", "HARAI entnehmen");
	default:
		return FText::GetEmpty();
	}
}

void AKemuriPrinterStation::Interact_Implementation(AActor* Interactor)
{
	UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(Interactor);
	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);

	switch (State)
	{
	case EKemuriPrinterState::Incomplete:
	{
		bool bInstalledAny = false;
		for (const FName& Part : GetMissingParts())
		{
			if (Inv && Inv->RemoveItem(Part))
			{
				InstalledParts.Add(Part);
				OnPartInstalled(Part);
				bInstalledAny = true;
			}
		}
		if (!bInstalledAny)
		{
			Deny(Interactor);
			return;
		}
		if (GetMissingParts().Num() == 0)
		{
			SetState(EKemuriPrinterState::Assembled);
		}
		break;
	}
	case EKemuriPrinterState::Assembled:
		OnStartCalibration(Interactor);
		break;
	case EKemuriPrinterState::Calibrated:
		if (!Inv || !Inv->RemoveItem(FilamentItem))
		{
			Deny(Interactor);
			return;
		}
		OnFilamentLoaded();
		SetState(EKemuriPrinterState::Ready);
		break;
	case EKemuriPrinterState::Ready:
		PrintProgress = 0.f;
		PrintedPart->SetVisibility(true);
		SetState(HasPower() ? EKemuriPrinterState::Printing : EKemuriPrinterState::Paused);
		if (Sub)
		{
			Sub->SetFlag(KemuriFlags::PrintStarted);
		}
		if (FogDirector)
		{
			FogDirector->SetSiegeMode(true);
		}
		MotorAudio->Play();
		break;
	case EKemuriPrinterState::Done:
		if (FinalWeapon)
		{
			if (UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor))
			{
				Holder->GiveWeapon(FinalWeapon);
			}
		}
		PrintedPart->SetVisibility(false);
		SetActorTickEnabled(false);
		break;
	default:
		return;
	}

	PlayInteractSound();
	OnInteracted(Interactor);
}

void AKemuriPrinterStation::CompleteCalibration()
{
	if (State == EKemuriPrinterState::Assembled)
	{
		SetState(EKemuriPrinterState::Calibrated);
	}
}

void AKemuriPrinterStation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (State != EKemuriPrinterState::Printing && State != EKemuriPrinterState::Paused)
	{
		return;
	}

	// Strom faellt aus -> Pause (Stalker zerstoert den Generator, Spieler muss reparieren).
	const bool bPower = HasPower();
	if (!bPower && State == EKemuriPrinterState::Printing)
	{
		SetState(EKemuriPrinterState::Paused);
		MotorAudio->SetPaused(true);
	}
	else if (bPower && State == EKemuriPrinterState::Paused)
	{
		SetState(EKemuriPrinterState::Printing);
		MotorAudio->SetPaused(false);
	}
	if (State != EKemuriPrinterState::Printing)
	{
		return;
	}

	PrintProgress = FMath::Min(1.f, PrintProgress + DeltaSeconds / FMath::Max(1.f, PrintDuration));
	UpdatePrinthead();

	if (PrintedPartMID)
	{
		PrintedPartMID->SetScalarParameterValue(TEXT("PrintProgress"), PrintProgress);
	}
	const int32 Layer = FMath::FloorToInt(PrintProgress * LayerCount);
	if (Layer != LastLayer)
	{
		LastLayer = Layer;
		OnPrintProgress(PrintProgress, Layer);
	}

	// Belagerung: Stalker wird regelmaessig angelockt.
	if (LureStalkerInterval > 0.f)
	{
		LureTimer += DeltaSeconds;
		if (LureTimer >= LureStalkerInterval)
		{
			LureTimer = 0.f;
			if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
			{
				if (AKemuriStalker* Stalker = Sub->GetStalker())
				{
					Stalker->LureTo(GetActorLocation());
				}
			}
		}
	}

	if (PrintProgress >= 1.f)
	{
		SetState(EKemuriPrinterState::Done);
		MotorAudio->Stop();
		if (FogDirector)
		{
			FogDirector->SetSiegeMode(false);
		}
		if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
		{
			Sub->SetFlag(KemuriFlags::PrintDone);
		}
		OnPrintFinished();
	}
}

void AKemuriPrinterStation::UpdatePrinthead()
{
	// Infill: pro Schicht LinesPerLayer Bahnen im Zickzack, Schichten abwechselnd in X und Y (Kreuzschraffur).
	const int32 TotalLines = FMath::Max(1, LayerCount * LinesPerLayer);
	const float LineF = PrintProgress * TotalLines;
	const int32 Line = FMath::Min(FMath::FloorToInt(LineF), TotalLines - 1);
	const float T = FMath::Clamp(LineF - Line, 0.f, 1.f);
	const int32 Layer = Line / FMath::Max(1, LinesPerLayer);
	const int32 LineInLayer = Line % FMath::Max(1, LinesPerLayer);

	const float Across = (LinesPerLayer > 1) ? static_cast<float>(LineInLayer) / (LinesPerLayer - 1) : 0.5f;
	const float Along = (LineInLayer % 2 == 0) ? T : 1.f - T;

	float X, Y;
	if (Layer % 2 == 0)
	{
		X = Along * BedSize.X;
		Y = Across * BedSize.Y;
	}
	else
	{
		X = Across * BedSize.X;
		Y = Along * BedSize.Y;
	}
	const float Z = Layer * LayerHeight + NozzleOffsetZ;
	PrintheadAnchor->SetRelativeLocation(FVector(X, Y, Z));

	if (MotorAudio && MotorAudio->IsPlaying())
	{
		// Schrittmotor-Ton: am Bahnende kurz tiefer (Richtungswechsel).
		const float Edge = FMath::Min(T, 1.f - T);
		MotorAudio->SetPitchMultiplier(0.85f + FMath::Clamp(Edge * 4.f, 0.f, 0.4f));
	}
}

#undef LOCTEXT_NAMESPACE
