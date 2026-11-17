#include "KemuriAIControllers.h"
#include "KemuriEnemies.h"
#include "KemuriWorldSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// Nebelgestalten
// ============================================================================

AKemuriEnemyAIController::AKemuriEnemyAIController()
{
}

void AKemuriEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	const float FirstDelay = FMath::FRandRange(0.05f, ThinkInterval);
	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AKemuriEnemyAIController::Think, ThinkInterval, true, FirstDelay);
}

void AKemuriEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);
	Super::OnUnPossess();
}

void AKemuriEnemyAIController::Think()
{
	AKemuriEnemyBase* Enemy = Cast<AKemuriEnemyBase>(GetPawn());
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Enemy || Enemy->IsDead() || !Player)
	{
		return;
	}

	const float Dist = FVector::Dist(Enemy->GetActorLocation(), Player->GetActorLocation());
	if (Dist <= Enemy->AttackRange)
	{
		StopMovement();
		SetFocus(Player);
		Enemy->TryAttack(Player);
		return;
	}
	ClearFocus(EAIFocusPriority::Gameplay);
	MoveToActor(Player, Enemy->AttackRange * 0.6f, true, true, false, {}, true);
}

// ============================================================================
// Stalker
// ============================================================================

AKemuriStalkerAIController::AKemuriStalkerAIController()
{

	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2400.f;
	SightConfig->LoseSightRadius = 2900.f;
	SightConfig->PeripheralVisionAngleDegrees = 65.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->SetMaxAge(4.f);

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3200.f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->SetMaxAge(6.f);

	Perception->ConfigureSense(*SightConfig);
	Perception->ConfigureSense(*HearingConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*Perception);
}

void AKemuriStalkerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Perception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AKemuriStalkerAIController::HandlePerception);
	LastContactTime = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AKemuriStalkerAIController::Think, ThinkInterval, true);
}

void AKemuriStalkerAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);
	Super::OnUnPossess();
}

APawn* AKemuriStalkerAIController::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

AKemuriStalker* AKemuriStalkerAIController::GetStalker() const
{
	return Cast<AKemuriStalker>(GetPawn());
}

void AKemuriStalkerAIController::HandlePerception(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* Player = GetPlayerPawn();
	if (!Player || !Actor)
	{
		return;
	}
	// Spieler zu Fuss oder auf dem Scooter (dann ist der Scooter der Pawn).
	const bool bIsPlayer = (Actor == Player);
	const double Now = GetWorld()->GetTimeSeconds();

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (!bIsPlayer)
		{
			return;
		}
		bSeesTarget = Stimulus.WasSuccessfullySensed();
		LastSeenLocation = Stimulus.StimulusLocation;
		if (bSeesTarget)
		{
			LastSeenTime = Now;
			LastContactTime = Now;
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			LastNoiseLocation = Stimulus.StimulusLocation;
			LastNoiseTime = Now;
			LastContactTime = Now;
		}
	}
}

bool AKemuriStalkerAIController::PickNavPoint(const FVector& Origin, float Radius, FVector& Out) const
{
	const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation Result;
	if (Nav && Nav->GetRandomReachablePointInRadius(Origin, Radius, Result))
	{
		Out = Result.Location;
		return true;
	}
	return false;
}

bool AKemuriStalkerAIController::PickPointAwayFrom(const FVector& Threat, float Distance, FVector& Out) const
{
	const APawn* MyPawn = GetPawn();
	const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!MyPawn || !Nav)
	{
		return false;
	}
	FVector Dir = MyPawn->GetActorLocation() - Threat;
	Dir.Z = 0.f;
	Dir = Dir.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}
	FNavLocation Result;
	const FVector Desired = MyPawn->GetActorLocation() + Dir * Distance;
	if (Nav->ProjectPointToNavigation(Desired, Result, FVector(800.f, 800.f, 600.f)))
	{
		Out = Result.Location;
		return true;
	}
	return PickNavPoint(Desired, 1200.f, Out);
}

void AKemuriStalkerAIController::SetState(EKemuriStalkerState NewState)
{
	if (State == NewState)
	{
		return;
	}
	State = NewState;
	bHasRoamGoal = false;
	if (AKemuriStalker* Stalker = GetStalker())
	{
		Stalker->SetChasing(NewState == EKemuriStalkerState::Chase);
	}
}

void AKemuriStalkerAIController::Think()
{
	AKemuriStalker* Stalker = GetStalker();
	APawn* Player = GetPlayerPawn();
	if (!Stalker || Stalker->IsDead() || !Player)
	{
		return;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	const FVector MyLoc = Stalker->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const float DistToPlayer = FVector::Dist(MyLoc, PlayerLoc);
	const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this);

	// 1) Betaeubt: nichts tun.
	if (Stalker->IsStunned())
	{
		StopMovement();
		SetState(EKemuriStalkerState::Stunned);
		return;
	}
	if (State == EKemuriStalkerState::Stunned)
	{
		// Nach Betaeubung zieht er sich zurueck und kommt dann wieder.
		RetreatUntil = Now + RetreatDuration * 0.5f;
		SetState(EKemuriStalkerState::Retreat);
	}

	// 2) Skript-Lockruf (Drucker-Belagerung).
	FVector Lure;
	if (Stalker->ConsumeLure(Lure))
	{
		LastNoiseLocation = Lure;
		LastNoiseTime = Now;
	}

	// 3) Raucherbereich: Spieler ist sicher, Stalker zieht sich zurueck.
	if (Sub && Sub->IsPlayerInSafeZone())
	{
		if (State != EKemuriStalkerState::Retreat)
		{
			RetreatUntil = Now + RetreatDuration;
			SetState(EKemuriStalkerState::Retreat);
		}
	}

	if (State == EKemuriStalkerState::Retreat)
	{
		if (Now < RetreatUntil || (Sub && Sub->IsPlayerInSafeZone()))
		{
			if (!bHasRoamGoal || GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				FVector Away;
				if (PickPointAwayFrom(PlayerLoc, RetreatDistance, Away))
				{
					MoveToLocation(Away, 100.f);
					bHasRoamGoal = true;
				}
			}
			return;
		}
		SetState(EKemuriStalkerState::Roam);
	}

	// 4) Zugriff.
	if (bSeesTarget && DistToPlayer <= Stalker->CatchRadius)
	{
		SetState(EKemuriStalkerState::Catch);
		StopMovement();
		SetFocus(Player);
		Stalker->CatchTarget(Player);
		RetreatUntil = Now + RetreatDuration;
		SetState(EKemuriStalkerState::Retreat);
		return;
	}

	// 5) Verfolgung bei Sicht.
	if (bSeesTarget)
	{
		SetState(EKemuriStalkerState::Chase);
		SetFocus(Player);
		MoveToActor(Player, Stalker->CatchRadius * 0.5f, true, true, false, {}, true);
		return;
	}
	ClearFocus(EAIFocusPriority::Gameplay);

	// 6) Suche am letzten Sichtpunkt.
	if (Now - LastSeenTime < SearchDuration)
	{
		if (State != EKemuriStalkerState::Search)
		{
			SetState(EKemuriStalkerState::Search);
			MoveToLocation(LastSeenLocation, 80.f);
			bHasRoamGoal = true;
		}
		else if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			FVector Next;
			if (PickNavPoint(LastSeenLocation, 700.f, Next))
			{
				MoveToLocation(Next, 80.f);
			}
		}
		return;
	}

	// 7) Geraeusch untersuchen.
	if (Now - LastNoiseTime < NoiseMemory)
	{
		if (State != EKemuriStalkerState::Investigate)
		{
			SetState(EKemuriStalkerState::Investigate);
			MoveToLocation(LastNoiseLocation, 120.f);
			bHasRoamGoal = true;
		}
		return;
	}

	// 8) Umherstreifen an der Leine.
	SetState(EKemuriStalkerState::Roam);
	if (!bHasRoamGoal || GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		const float Alpha = FMath::Clamp(static_cast<float>((Now - LastContactTime) / FMath::Max(1.f, LeashTightenTime)), 0.f, 1.f);
		const float Leash = FMath::Lerp(LeashRadiusMax, LeashRadiusMin, Alpha);
		FVector Goal;
		if (PickNavPoint(PlayerLoc, Leash, Goal))
		{
			MoveToLocation(Goal, 150.f);
			bHasRoamGoal = true;
		}
	}
}

// ============================================================================
// Passanten
// ============================================================================

AKemuriCivilianAIController::AKemuriCivilianAIController()
{
}

void AKemuriCivilianAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	HomeLocation = InPawn ? InPawn->GetActorLocation() : FVector::ZeroVector;
	NextMoveTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(0.f, 3.f);
	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AKemuriCivilianAIController::Think, 0.5f, true, FMath::FRandRange(0.f, 0.5f));
}

void AKemuriCivilianAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);
	Super::OnUnPossess();
}

void AKemuriCivilianAIController::Think()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn || MyPawn->IsHidden())
	{
		return;
	}
	const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const double Now = GetWorld()->GetTimeSeconds();

	// Flucht vor dem Stalker.
	if (const UKemuriWorldSubsystem* Sub = UKemuriWorldSubsystem::Get(this))
	{
		if (const AKemuriStalker* Stalker = Sub->GetStalker())
		{
			const FVector Threat = Stalker->GetActorLocation();
			if (FVector::Dist(Threat, MyPawn->GetActorLocation()) < FleeRadius && Nav)
			{
				FVector Dir = (MyPawn->GetActorLocation() - Threat).GetSafeNormal2D();
				FNavLocation Result;
				if (Nav->ProjectPointToNavigation(MyPawn->GetActorLocation() + Dir * 1500.f, Result, FVector(600.f, 600.f, 400.f)))
				{
					MoveToLocation(Result.Location, 80.f);
					NextMoveTime = Now + 4.0;
					return;
				}
			}
		}
	}

	if (Now < NextMoveTime || GetMoveStatus() != EPathFollowingStatus::Idle || !Nav)
	{
		return;
	}
	FNavLocation Result;
	if (Nav->GetRandomReachablePointInRadius(HomeLocation, WanderRadius, Result))
	{
		MoveToLocation(Result.Location, 60.f);
	}
	NextMoveTime = Now + FMath::FRandRange(WaitTimeRange.X, WaitTimeRange.Y);
}
