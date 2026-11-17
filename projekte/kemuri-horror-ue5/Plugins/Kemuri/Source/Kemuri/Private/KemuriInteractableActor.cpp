#include "KemuriInteractableActor.h"
#include "KemuriInventoryComponent.h"
#include "KemuriGlassesComponent.h"
#include "KemuriWeaponHolderComponent.h"
#include "KemuriWorldSubsystem.h"
#include "KemuriTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#define LOCTEXT_NAMESPACE "Kemuri"

AKemuriInteractableActor::AKemuriInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

bool AKemuriInteractableActor::CanInteract_Implementation(AActor* Interactor)
{
	return !IsHidden() && IsRequiredFlagSet();
}

void AKemuriInteractableActor::Interact_Implementation(AActor* Interactor)
{
	PlayInteractSound();
	OnInteracted(Interactor);
}

FText AKemuriInteractableActor::GetInteractPrompt_Implementation(AActor* Interactor)
{
	return Prompt;
}

bool AKemuriInteractableActor::IsRequiredFlagSet() const
{
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	return !Sub || Sub->IsFlagSatisfied(RequiredFlag);
}

void AKemuriInteractableActor::PlayInteractSound() const
{
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractSound, GetActorLocation());
	}
}

void AKemuriInteractableActor::Deny(AActor* Interactor)
{
	if (DeniedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeniedSound, GetActorLocation());
	}
	OnDenied(Interactor);
}

UKemuriInventoryComponent* AKemuriInteractableActor::GetInventory(const AActor* Actor)
{
	return UKemuriInventoryComponent::FindOn(Actor);
}

FText AKemuriInteractableActor::FormatPrice(const FText& Label, int32 Price)
{
	return FText::Format(LOCTEXT("PriceFmt", "{0}  [{1} Mon]"), Label, FText::AsNumber(Price));
}

// ===================== Pickup =====================

AKemuriPickup::AKemuriPickup()
{
	Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AKemuriPickup::BeginPlay()
{
	Super::BeginPlay();
	if (bScannable)
	{
		if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
		{
			Sub->RegisterScanTarget(this);
		}
	}
}

void AKemuriPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->UnregisterScanTarget(this);
	}
	Super::EndPlay(EndPlayReason);
}

FText AKemuriPickup::GetInteractPrompt_Implementation(AActor* Interactor)
{
	if (!Prompt.IsEmpty())
	{
		return Prompt;
	}
	const FText Name = DisplayName.IsEmpty() ? FText::FromName(ItemId) : DisplayName;
	return FText::Format(LOCTEXT("PickupFmt", "Aufheben: {0}"), Name);
}

void AKemuriPickup::Interact_Implementation(AActor* Interactor)
{
	if (WeaponData)
	{
		if (UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor))
		{
			Holder->GiveWeapon(WeaponData);
		}
	}

	if (!ItemId.IsNone())
	{
		if (ItemId == KemuriItems::Battery)
		{
			if (UKemuriGlassesComponent* Glasses = UKemuriGlassesComponent::FindOn(Interactor))
			{
				Glasses->AddBattery(BatteryAmount * Count);
			}
		}
		else if (UKemuriInventoryComponent* Inv = GetInventory(Interactor))
		{
			Inv->AddItem(ItemId, Count);
		}
	}

	if (UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		Sub->SetFlag(FlagOnPickup);
		Sub->UnregisterScanTarget(this);
	}

	PlayInteractSound();
	OnInteracted(Interactor);
	OnPickedUp(Interactor);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}

#undef LOCTEXT_NAMESPACE
