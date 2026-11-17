#include "KemuriInventoryComponent.h"
#include "KemuriTypes.h"
#include "GameFramework/Actor.h"

UKemuriInventoryComponent::UKemuriInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	StartItems.Add(KemuriItems::Mon, 500);
}

UKemuriInventoryComponent* UKemuriInventoryComponent::FindOn(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UKemuriInventoryComponent>() : nullptr;
}

void UKemuriInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	for (const TPair<FName, int32>& Pair : StartItems)
	{
		AddItem(Pair.Key, Pair.Value);
	}
}

void UKemuriInventoryComponent::AddItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return;
	}
	int32& Value = Items.FindOrAdd(ItemId);
	Value += Count;
	OnItemChanged.Broadcast(ItemId, Value);
}

bool UKemuriInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	int32* Value = Items.Find(ItemId);
	if (!Value || *Value < Count)
	{
		return false;
	}
	*Value -= Count;
	const int32 NewCount = *Value;
	if (NewCount <= 0)
	{
		Items.Remove(ItemId);
	}
	OnItemChanged.Broadcast(ItemId, NewCount);
	return true;
}

int32 UKemuriInventoryComponent::GetCount(FName ItemId) const
{
	const int32* Value = Items.Find(ItemId);
	return Value ? *Value : 0;
}

int32 UKemuriInventoryComponent::GetMon() const
{
	return GetCount(KemuriItems::Mon);
}

void UKemuriInventoryComponent::AddMon(int32 Amount)
{
	AddItem(KemuriItems::Mon, Amount);
}

bool UKemuriInventoryComponent::SpendMon(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	return RemoveItem(KemuriItems::Mon, Amount);
}
