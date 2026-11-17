#include "KemuriInformant.h"
#include "KemuriInventoryComponent.h"
#include "KemuriGlassesComponent.h"
#include "KemuriWorldSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "Kemuri"

AKemuriInformant::AKemuriInformant()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractVolume"));
	InteractVolume->InitCapsuleSize(45.f, 95.f);
	InteractVolume->SetCollisionProfileName(TEXT("NoCollision"));
	InteractVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractVolume->SetHiddenInGame(true);
	SetRootComponent(InteractVolume);

	SpeakerName = LOCTEXT("InformantName", "Kuroda");
	AskForSmokeLine = LOCTEXT("AskSmoke", "Hast du was zu rauchen? Ohne Rauch rede ich nicht.");
}

bool AKemuriInformant::CanInteract_Implementation(AActor* Interactor)
{
	return true;
}

FText AKemuriInformant::GetInteractPrompt_Implementation(AActor* Interactor)
{
	const UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(Interactor);
	if (bHigh)
	{
		return FText::Format(LOCTEXT("TalkHigh", "Mit {0} reden"), SpeakerName);
	}
	if (Inv && Inv->HasItem(KemuriItems::Weed))
	{
		return FText::Format(LOCTEXT("GiveWeed", "{0} Weed geben"), SpeakerName);
	}
	if (Inv && Inv->HasItem(KemuriItems::Cigarettes))
	{
		return FText::Format(LOCTEXT("GiveCig", "{0} eine Zigarette geben"), SpeakerName);
	}
	return FText::Format(LOCTEXT("Talk", "Mit {0} reden"), SpeakerName);
}

void AKemuriInformant::Interact_Implementation(AActor* Interactor)
{
	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (!bTalkedOnce)
	{
		bTalkedOnce = true;
		if (Sub)
		{
			Sub->SetFlag(FlagOnFirstTalk);
		}
	}

	if (bHigh)
	{
		GiveHint(Interactor, true);
		return;
	}

	UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(Interactor);
	if (Inv && Inv->RemoveItem(KemuriItems::Weed))
	{
		bHigh = true;
		OnLightJoint();
		OnHighChanged(true);
		GetWorldTimerManager().SetTimer(HighTimer, this, &AKemuriInformant::EndHigh, HighDuration, false);
		GiveHint(Interactor, true);
		return;
	}
	if (Inv && Inv->RemoveItem(KemuriItems::Cigarettes))
	{
		OnLightCigarette();
		GiveHint(Interactor, false);
		return;
	}
	Say(AskForSmokeLine, nullptr, Interactor);
}

int32 AKemuriInformant::FindCurrentHint() const
{
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (!Sub)
	{
		return INDEX_NONE;
	}
	for (int32 i = 0; i < Hints.Num(); ++i)
	{
		const FKemuriHint& Hint = Hints[i];
		const bool bOpen = Hint.CompletedFlag.IsNone() || !Sub->HasFlag(Hint.CompletedFlag);
		if (Sub->IsFlagSatisfied(Hint.RequiredFlag) && bOpen)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void AKemuriInformant::GiveHint(AActor* Listener, bool bPrecise)
{
	const int32 Index = FindCurrentHint();
	if (!Hints.IsValidIndex(Index))
	{
		const TArray<FText>& Pool = (bHigh && HighRamblings.Num() > 0) ? HighRamblings : IdleLines;
		if (Pool.Num() > 0)
		{
			Say(Pool[FMath::RandRange(0, Pool.Num() - 1)], nullptr, Listener);
		}
		return;
	}

	const FKemuriHint& Hint = Hints[Index];
	UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	if (bPrecise)
	{
		Say(Hint.PreciseText, Hint.PreciseVoice, Listener);
		if (Sub)
		{
			Sub->SetFlag(Hint.FlagOnVague);
			Sub->SetFlag(Hint.FlagOnPrecise);
		}
	}
	else
	{
		Say(Hint.VagueText, Hint.VagueVoice, Listener);
		if (Sub)
		{
			Sub->SetFlag(Hint.FlagOnVague);
		}
	}
	OnSpeak(bPrecise ? Hint.PreciseText : Hint.VagueText, bPrecise);

	if (UKemuriGlassesComponent* Glasses = UKemuriGlassesComponent::FindOn(Listener))
	{
		Glasses->LogHint(bPrecise ? Hint.PreciseText : Hint.VagueText);
	}
}

void AKemuriInformant::Say(const FText& Text, USoundBase* Voice, AActor* Listener)
{
	if (Text.IsEmpty())
	{
		return;
	}
	if (Voice)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Voice, GetActorLocation());
	}
	const float Duration = FMath::Clamp(Text.ToString().Len() * SubtitleSecondsPerChar, 2.5f, 12.f);
	if (UKemuriGlassesComponent* Glasses = UKemuriGlassesComponent::FindOn(Listener))
	{
		Glasses->ShowSubtitle(SpeakerName, Text, Duration);
	}
}

void AKemuriInformant::EndHigh()
{
	bHigh = false;
	OnHighChanged(false);
}

#undef LOCTEXT_NAMESPACE
