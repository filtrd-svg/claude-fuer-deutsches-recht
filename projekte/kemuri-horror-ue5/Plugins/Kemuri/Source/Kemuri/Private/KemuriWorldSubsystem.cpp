#include "KemuriWorldSubsystem.h"
#include "KemuriFogDirector.h"
#include "KemuriTypes.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UKemuriWorldSubsystem* UKemuriWorldSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext || !GEngine)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetSubsystem<UKemuriWorldSubsystem>();
	}
	return nullptr;
}

void UKemuriWorldSubsystem::SetFlag(FName Flag)
{
	if (Flag.IsNone() || Flags.Contains(Flag))
	{
		return;
	}
	Flags.Add(Flag);
	OnFlagChanged.Broadcast(Flag, true);
}

void UKemuriWorldSubsystem::ClearFlag(FName Flag)
{
	if (Flags.Remove(Flag) > 0)
	{
		OnFlagChanged.Broadcast(Flag, false);
	}
}

bool UKemuriWorldSubsystem::HasFlag(FName Flag) const
{
	return Flags.Contains(Flag);
}

void UKemuriWorldSubsystem::SetIRActive(bool bActive)
{
	if (bIRActive == bActive)
	{
		return;
	}
	bIRActive = bActive;
	OnIRChanged.Broadcast(bIRActive);
}

float UKemuriWorldSubsystem::GetFogAmountAt(const FVector& Location) const
{
	if (const AKemuriFogDirector* Director = FogDirector.Get())
	{
		return Director->GetFogAmountAt(Location);
	}
	return 0.f;
}

void UKemuriWorldSubsystem::RegisterScanTarget(AActor* Target)
{
	if (Target)
	{
		ScanTargets.AddUnique(Target);
	}
}

void UKemuriWorldSubsystem::UnregisterScanTarget(AActor* Target)
{
	ScanTargets.Remove(Target);
}

AActor* UKemuriWorldSubsystem::GetNearestScanTarget(const FVector& From, float MaxDistance, float& OutDistance) const
{
	AActor* Best = nullptr;
	float BestDistSq = FMath::Square(MaxDistance);
	for (const TWeakObjectPtr<AActor>& Weak : ScanTargets)
	{
		const AActor* Target = Weak.Get();
		if (!Target || Target->IsHidden())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(From, Target->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = const_cast<AActor*>(Target);
		}
	}
	OutDistance = Best ? FMath::Sqrt(BestDistSq) : -1.f;
	return Best;
}

void UKemuriWorldSubsystem::SetPlayerInSafeZone(bool bInside)
{
	bPlayerInSafeZone = bInside;
	if (bInside)
	{
		SetFlag(KemuriFlags::PlayerInSafeZone);
	}
	else
	{
		ClearFlag(KemuriFlags::PlayerInSafeZone);
	}
}
