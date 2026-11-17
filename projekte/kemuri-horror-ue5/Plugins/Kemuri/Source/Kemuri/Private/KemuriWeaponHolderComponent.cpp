#include "KemuriWeaponHolderComponent.h"
#include "KemuriWeapon.h"
#include "KemuriWeaponData.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

UKemuriWeaponHolderComponent::UKemuriWeaponHolderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UKemuriWeaponHolderComponent* UKemuriWeaponHolderComponent::FindOn(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UKemuriWeaponHolderComponent>() : nullptr;
}

void UKemuriWeaponHolderComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!AttachParent && GetOwner())
	{
		AttachParent = GetOwner()->FindComponentByClass<UCameraComponent>();
	}
	if (StartingWeapon)
	{
		GiveWeapon(StartingWeapon);
	}
}

AKemuriWeapon* UKemuriWeaponHolderComponent::FindWeapon(const UKemuriWeaponData* Data) const
{
	if (!Data)
	{
		return nullptr;
	}
	for (AKemuriWeapon* Weapon : Weapons)
	{
		if (!Weapon || !Weapon->GetData())
		{
			continue;
		}
		const UKemuriWeaponData* Owned = Weapon->GetData();
		if (Owned == Data || Owned == Data->UpgradedVersion || Owned->UpgradedVersion == Data)
		{
			return Weapon;
		}
	}
	return nullptr;
}

AKemuriWeapon* UKemuriWeaponHolderComponent::GiveWeapon(UKemuriWeaponData* Data)
{
	if (!Data || !GetOwner())
	{
		return nullptr;
	}

	if (AKemuriWeapon* Existing = FindWeapon(Data))
	{
		if (Existing->GetData() == Data)
		{
			Existing->RefillAmmo();
			return Existing;
		}
		// Gleiche Waffe in anderer Stufe: ersetzen.
		Weapons.Remove(Existing);
		Existing->Destroy();
		CurrentIndex = INDEX_NONE;
	}

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* WeaponClass = Data->WeaponClass ? Data->WeaponClass.Get() : AKemuriWeapon::StaticClass();
	AKemuriWeapon* Weapon = GetWorld()->SpawnActor<AKemuriWeapon>(WeaponClass, GetOwner()->GetActorTransform(), Params);
	if (!Weapon)
	{
		return nullptr;
	}
	Weapon->InitFromData(Data);
	Weapon->ReloadSpeedMultiplier = ReloadSpeedMultiplier;
	AttachWeapon(Weapon);

	if (Weapons.Num() >= MaxSlots && Weapons.IsValidIndex(CurrentIndex))
	{
		AKemuriWeapon* Old = Weapons[CurrentIndex];
		Weapons[CurrentIndex] = Weapon;
		if (Old)
		{
			Old->Destroy();
		}
		Equip(CurrentIndex);
	}
	else
	{
		Weapons.Add(Weapon);
		Equip(Weapons.Num() - 1);
	}
	return Weapon;
}

void UKemuriWeaponHolderComponent::AttachWeapon(AKemuriWeapon* Weapon) const
{
	if (!Weapon || !AttachParent)
	{
		return;
	}
	const UKemuriWeaponData* Data = Weapon->GetData();
	const FName Socket = (Data && AttachParent->DoesSocketExist(Data->AttachSocket)) ? Data->AttachSocket : NAME_None;
	Weapon->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	if (Data)
	{
		Weapon->SetActorRelativeTransform(Data->AttachOffset);
	}
}

void UKemuriWeaponHolderComponent::Equip(int32 Index)
{
	if (!Weapons.IsValidIndex(Index))
	{
		return;
	}
	if (AKemuriWeapon* Current = GetCurrentWeapon())
	{
		Current->StopFire();
	}
	CurrentIndex = Index;
	for (int32 i = 0; i < Weapons.Num(); ++i)
	{
		if (Weapons[i])
		{
			Weapons[i]->SetActorHiddenInGame(i != CurrentIndex);
		}
	}
	OnWeaponChanged.Broadcast(GetCurrentWeapon());
}

void UKemuriWeaponHolderComponent::NextWeapon()
{
	if (Weapons.Num() > 1)
	{
		Equip((CurrentIndex + 1) % Weapons.Num());
	}
}

void UKemuriWeaponHolderComponent::StartFire()
{
	if (AKemuriWeapon* Weapon = GetCurrentWeapon())
	{
		Weapon->StartFire();
	}
}

void UKemuriWeaponHolderComponent::StopFire()
{
	if (AKemuriWeapon* Weapon = GetCurrentWeapon())
	{
		Weapon->StopFire();
	}
}

void UKemuriWeaponHolderComponent::Reload()
{
	if (AKemuriWeapon* Weapon = GetCurrentWeapon())
	{
		Weapon->Reload();
	}
}

AKemuriWeapon* UKemuriWeaponHolderComponent::GetCurrentWeapon() const
{
	return Weapons.IsValidIndex(CurrentIndex) ? Weapons[CurrentIndex].Get() : nullptr;
}

UKemuriWeaponData* UKemuriWeaponHolderComponent::TakeCurrentWeapon()
{
	AKemuriWeapon* Weapon = GetCurrentWeapon();
	if (!Weapon)
	{
		return nullptr;
	}
	UKemuriWeaponData* Data = Weapon->GetData();
	Weapons.RemoveAt(CurrentIndex);
	Weapon->Destroy();
	CurrentIndex = INDEX_NONE;
	if (Weapons.Num() > 0)
	{
		Equip(0);
	}
	else
	{
		OnWeaponChanged.Broadcast(nullptr);
	}
	return Data;
}

void UKemuriWeaponHolderComponent::SetReloadSpeedMultiplier(float Multiplier)
{
	ReloadSpeedMultiplier = Multiplier;
	for (AKemuriWeapon* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->ReloadSpeedMultiplier = Multiplier;
		}
	}
}

TArray<AKemuriWeapon*> UKemuriWeaponHolderComponent::GetWeapons() const
{
	TArray<AKemuriWeapon*> Result;
	for (AKemuriWeapon* Weapon : Weapons)
	{
		Result.Add(Weapon);
	}
	return Result;
}
