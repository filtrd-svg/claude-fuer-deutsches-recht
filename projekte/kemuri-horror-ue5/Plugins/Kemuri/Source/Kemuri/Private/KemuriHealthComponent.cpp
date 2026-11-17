#include "KemuriHealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"

UKemuriHealthComponent::UKemuriHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UKemuriHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UKemuriHealthComponent::HandleTakeAnyDamage);
	}
}

void UKemuriHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDead || !bRegenerates || Health >= MaxHealth)
	{
		return;
	}
	TimeSinceDamage += DeltaTime;
	if (TimeSinceDamage >= RegenDelay)
	{
		Health = FMath::Min(MaxHealth, Health + RegenPerSecond * DeltaTime);
	}
}

void UKemuriHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bDead || bInvulnerable || Damage <= 0.f)
	{
		return;
	}

	Health -= Damage;
	TimeSinceDamage = 0.f;

	const bool bLethalAllowed = !OnlyLethalDamageType || (DamageType && DamageType->IsA(OnlyLethalDamageType));
	if (!bLethalAllowed)
	{
		Health = FMath::Max(Health, 1.f);
	}

	OnDamaged.Broadcast(Damage, DamageType, InstigatedBy, DamageCauser, Health);

	if (Health <= 0.f)
	{
		Health = 0.f;
		bDead = true;
		OnDied.Broadcast(InstigatedBy, DamageType);
	}
}

void UKemuriHealthComponent::Heal(float Amount)
{
	if (!bDead)
	{
		Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	}
}

void UKemuriHealthComponent::SetMaxHealth(float NewMax, bool bFill)
{
	MaxHealth = FMath::Max(1.f, NewMax);
	Health = bFill ? MaxHealth : FMath::Min(Health, MaxHealth);
}

void UKemuriHealthComponent::Revive(float HealthPercent)
{
	bDead = false;
	Health = FMath::Clamp(HealthPercent, 0.05f, 1.f) * MaxHealth;
	TimeSinceDamage = 0.f;
}
