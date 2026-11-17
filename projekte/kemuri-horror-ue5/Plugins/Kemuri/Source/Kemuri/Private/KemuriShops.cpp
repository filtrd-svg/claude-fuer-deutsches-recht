#include "KemuriShops.h"
#include "KemuriInventoryComponent.h"
#include "KemuriWeaponHolderComponent.h"
#include "KemuriWeapon.h"
#include "KemuriWeaponData.h"
#include "KemuriPlayerCharacter.h"
#include "KemuriWorldSubsystem.h"
#include "EngineUtils.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "Kemuri"

// ============================================================================
// Wandkauf
// ============================================================================

FText AKemuriWallBuy::GetInteractPrompt_Implementation(AActor* Interactor)
{
	if (!Weapon)
	{
		return FText::GetEmpty();
	}
	const UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor);
	if (const AKemuriWeapon* Owned = Holder ? Holder->FindWeapon(Weapon) : nullptr)
	{
		const bool bUpgraded = Owned->GetData() && Owned->GetData()->bIsUpgraded;
		return FormatPrice(LOCTEXT("WallAmmo", "Munition kaufen"), bUpgraded ? UpgradedAmmoPrice : AmmoPrice);
	}
	return FormatPrice(FText::Format(LOCTEXT("WallBuy", "{0} kaufen"), Weapon->DisplayName), Price);
}

void AKemuriWallBuy::Interact_Implementation(AActor* Interactor)
{
	UKemuriInventoryComponent* Inv = GetInventory(Interactor);
	UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor);
	if (!Inv || !Holder || !Weapon)
	{
		return;
	}

	if (AKemuriWeapon* Owned = Holder->FindWeapon(Weapon))
	{
		const bool bUpgraded = Owned->GetData() && Owned->GetData()->bIsUpgraded;
		if (!Inv->SpendMon(bUpgraded ? UpgradedAmmoPrice : AmmoPrice))
		{
			Deny(Interactor);
			return;
		}
		Owned->RefillAmmo();
	}
	else
	{
		if (!Inv->SpendMon(Price))
		{
			Deny(Interactor);
			return;
		}
		Holder->GiveWeapon(Weapon);
	}
	Super::Interact_Implementation(Interactor);
}

// ============================================================================
// Omikuji-Schrein
// ============================================================================

AKemuriOmikujiBox::AKemuriOmikujiBox()
{
	Prompt = LOCTEXT("OmikujiPrompt", "Omikuji ziehen");
}

void AKemuriOmikujiBox::BeginPlay()
{
	Super::BeginPlay();
	SetBoxActive(bStartsActive);
}

void AKemuriOmikujiBox::SetBoxActive(bool bActive)
{
	BoxState = bActive ? EKemuriBoxState::Idle : EKemuriBoxState::Inactive;
	OnActiveChanged(bActive);
}

bool AKemuriOmikujiBox::CanInteract_Implementation(AActor* Interactor)
{
	return (BoxState == EKemuriBoxState::Idle || (BoxState == EKemuriBoxState::Offering && Roller.Get() == Interactor))
		&& Super::CanInteract_Implementation(Interactor);
}

FText AKemuriOmikujiBox::GetInteractPrompt_Implementation(AActor* Interactor)
{
	if (BoxState == EKemuriBoxState::Offering && OfferedWeapon)
	{
		return FText::Format(LOCTEXT("OmikujiTake", "{0} nehmen"), OfferedWeapon->DisplayName);
	}
	return BoxState == EKemuriBoxState::Idle ? FormatPrice(Prompt, Price) : FText::GetEmpty();
}

UKemuriWeaponData* AKemuriOmikujiBox::PickWeapon(const AActor* For) const
{
	const UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(For);
	float Total = 0.f;
	TArray<const FKemuriBoxEntry*> Candidates;
	for (const FKemuriBoxEntry& Entry : Pool)
	{
		if (Entry.Weapon && Entry.Weight > 0.f && (!Holder || !Holder->FindWeapon(Entry.Weapon)))
		{
			Candidates.Add(&Entry);
			Total += Entry.Weight;
		}
	}
	if (Candidates.Num() == 0)
	{
		return Pool.Num() > 0 ? Pool[FMath::RandRange(0, Pool.Num() - 1)].Weapon.Get() : nullptr;
	}
	float Roll = FMath::FRandRange(0.f, Total);
	for (const FKemuriBoxEntry* Entry : Candidates)
	{
		Roll -= Entry->Weight;
		if (Roll <= 0.f)
		{
			return Entry->Weapon;
		}
	}
	return Candidates.Last()->Weapon;
}

void AKemuriOmikujiBox::Interact_Implementation(AActor* Interactor)
{
	if (BoxState == EKemuriBoxState::Offering)
	{
		if (UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor))
		{
			Holder->GiveWeapon(OfferedWeapon);
		}
		GetWorldTimerManager().ClearTimer(OfferTimer);
		OfferedWeapon = nullptr;
		BoxState = EKemuriBoxState::Idle;
		Super::Interact_Implementation(Interactor);
		return;
	}

	UKemuriInventoryComponent* Inv = GetInventory(Interactor);
	if (!Inv || !Inv->SpendMon(Price))
	{
		Deny(Interactor);
		return;
	}
	Roller = Interactor;
	++Uses;
	RollElapsed = 0.f;
	BoxState = EKemuriBoxState::Rolling;
	GetWorldTimerManager().SetTimer(RollTimer, this, &AKemuriOmikujiBox::RollTick, 0.15f, true);
	Super::Interact_Implementation(Interactor);
}

void AKemuriOmikujiBox::RollTick()
{
	RollElapsed += 0.15f;
	OnRollTick(PickWeapon(Roller.Get()));
	if (RollElapsed >= RollDuration)
	{
		GetWorldTimerManager().ClearTimer(RollTimer);
		FinishRoll();
	}
}

void AKemuriOmikujiBox::FinishRoll()
{
	// Kyo: schlechtes Los, Geld zurueck, Schrein wandert.
	if (Uses >= MinUsesBeforeKyo && FMath::FRand() < KyoChance)
	{
		if (UKemuriInventoryComponent* Inv = GetInventory(Roller.Get()))
		{
			Inv->AddMon(Price);
		}
		BoxState = EKemuriBoxState::Moving;
		OnKyo();
		GetWorldTimerManager().SetTimer(MoveTimer, this, &AKemuriOmikujiBox::MoveAway, MoveDelay, false);
		return;
	}
	OfferedWeapon = PickWeapon(Roller.Get());
	BoxState = EKemuriBoxState::Offering;
	OnOffer(OfferedWeapon);
	GetWorldTimerManager().SetTimer(OfferTimer, this, &AKemuriOmikujiBox::OfferExpired, OfferDuration, false);
}

void AKemuriOmikujiBox::OfferExpired()
{
	OfferedWeapon = nullptr;
	BoxState = EKemuriBoxState::Idle;
	OnOffer(nullptr);
}

void AKemuriOmikujiBox::MoveAway()
{
	TArray<AKemuriOmikujiBox*> Others;
	for (TActorIterator<AKemuriOmikujiBox> It(GetWorld()); It; ++It)
	{
		if (*It != this)
		{
			Others.Add(*It);
		}
	}
	SetBoxActive(false);
	Uses = 0;
	if (Others.Num() > 0)
	{
		Others[FMath::RandRange(0, Others.Num() - 1)]->SetBoxActive(true);
	}
	else
	{
		SetBoxActive(true);
	}
}

// ============================================================================
// Weihe-Schrein
// ============================================================================

TArray<EKemuriAmmoMod> AKemuriUpgradeShrine::GetUnlockedMods(const AActor* For) const
{
	TArray<EKemuriAmmoMod> Mods;
	if (const UKemuriInventoryComponent* Inv = UKemuriInventoryComponent::FindOn(For))
	{
		if (Inv->HasItem(KemuriItems::RelicRaijin))  { Mods.Add(EKemuriAmmoMod::Raijin); }
		if (Inv->HasItem(KemuriItems::RelicKitsune)) { Mods.Add(EKemuriAmmoMod::Kitsunebi); }
		if (Inv->HasItem(KemuriItems::RelicYuki))    { Mods.Add(EKemuriAmmoMod::Yuki); }
		if (Inv->HasItem(KemuriItems::RelicShio))    { Mods.Add(EKemuriAmmoMod::Shio); }
	}
	return Mods;
}

bool AKemuriUpgradeShrine::CanInteract_Implementation(AActor* Interactor)
{
	if (!Super::CanInteract_Implementation(Interactor) || bBusy)
	{
		return false;
	}
	if (bReady)
	{
		return true;
	}
	const UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor);
	const AKemuriWeapon* Weapon = Holder ? Holder->GetCurrentWeapon() : nullptr;
	return Weapon && Weapon->GetData() && !Weapon->GetData()->bIsFinalWeapon;
}

FText AKemuriUpgradeShrine::GetInteractPrompt_Implementation(AActor* Interactor)
{
	if (bReady)
	{
		return LOCTEXT("ShrineTake", "Geweihte Waffe nehmen");
	}
	const UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor);
	const AKemuriWeapon* Weapon = Holder ? Holder->GetCurrentWeapon() : nullptr;
	if (!Weapon || !Weapon->GetData())
	{
		return FText::GetEmpty();
	}
	if (Weapon->GetData()->bIsUpgraded)
	{
		return GetUnlockedMods(Interactor).Num() > 0
			? FormatPrice(LOCTEXT("ShrineMod", "Munition segnen"), AmmoModPrice)
			: LOCTEXT("ShrineNoRelic", "Keine Reliquie");
	}
	return Weapon->GetData()->UpgradedVersion
		? FormatPrice(LOCTEXT("ShrineUpgrade", "Waffe weihen"), UpgradePrice)
		: LOCTEXT("ShrineCannot", "Diese Waffe kann nicht geweiht werden");
}

void AKemuriUpgradeShrine::Interact_Implementation(AActor* Interactor)
{
	UKemuriInventoryComponent* Inv = GetInventory(Interactor);
	UKemuriWeaponHolderComponent* Holder = UKemuriWeaponHolderComponent::FindOn(Interactor);
	if (!Inv || !Holder)
	{
		return;
	}

	if (bReady)
	{
		AKemuriWeapon* NewWeapon = Holder->GiveWeapon(PendingWeapon);
		if (NewWeapon)
		{
			NewWeapon->RefillAmmo();
		}
		PendingWeapon = nullptr;
		bReady = false;
		Super::Interact_Implementation(Interactor);
		return;
	}

	AKemuriWeapon* Weapon = Holder->GetCurrentWeapon();
	UKemuriWeaponData* Data = Weapon ? Weapon->GetData() : nullptr;
	if (!Data)
	{
		return;
	}

	if (Data->bIsUpgraded)
	{
		// Naechste freigeschaltete Modifikation durchschalten.
		const TArray<EKemuriAmmoMod> Mods = GetUnlockedMods(Interactor);
		if (Mods.Num() == 0 || !Inv->SpendMon(AmmoModPrice))
		{
			Deny(Interactor);
			return;
		}
		const int32 CurrentIdx = Mods.IndexOfByKey(Weapon->GetAmmoMod());
		const EKemuriAmmoMod Next = Mods[(CurrentIdx + 1) % Mods.Num()];
		Weapon->SetAmmoMod(Next);
		Weapon->RefillAmmo();
		OnAmmoModApplied(Next);
		Super::Interact_Implementation(Interactor);
		return;
	}

	if (!Data->UpgradedVersion || !Inv->SpendMon(UpgradePrice))
	{
		Deny(Interactor);
		return;
	}
	PendingWeapon = Data->UpgradedVersion;
	Holder->TakeCurrentWeapon();
	bBusy = true;
	OnUpgradeStarted(Data);
	GetWorldTimerManager().SetTimer(UpgradeTimer, this, &AKemuriUpgradeShrine::FinishUpgrade, UpgradeDuration, false);
	Super::Interact_Implementation(Interactor);
}

void AKemuriUpgradeShrine::FinishUpgrade()
{
	bBusy = false;
	bReady = true;
	OnUpgradeReady(PendingWeapon);
}

// ============================================================================
// Getraenkeautomat
// ============================================================================

bool AKemuriPerkVending::CanInteract_Implementation(AActor* Interactor)
{
	const AKemuriPlayerCharacter* Player = Cast<AKemuriPlayerCharacter>(Interactor);
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);
	const bool bPower = !Sub || Sub->IsFlagSatisfied(PowerFlag);
	return Player && bPower && !Player->HasPerk(Perk) && Super::CanInteract_Implementation(Interactor);
}

FText AKemuriPerkVending::GetInteractPrompt_Implementation(AActor* Interactor)
{
	return FormatPrice(FText::Format(LOCTEXT("PerkBuy", "{0} ziehen"), DrinkName), Price);
}

void AKemuriPerkVending::Interact_Implementation(AActor* Interactor)
{
	AKemuriPlayerCharacter* Player = Cast<AKemuriPlayerCharacter>(Interactor);
	UKemuriInventoryComponent* Inv = GetInventory(Interactor);
	if (!Player || !Inv || !Inv->SpendMon(Price))
	{
		Deny(Interactor);
		return;
	}
	Player->ApplyPerk(Perk);
	Super::Interact_Implementation(Interactor);
}

#undef LOCTEXT_NAMESPACE
