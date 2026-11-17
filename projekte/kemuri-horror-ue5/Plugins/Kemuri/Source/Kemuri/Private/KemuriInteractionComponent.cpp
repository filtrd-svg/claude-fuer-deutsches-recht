#include "KemuriInteractionComponent.h"
#include "KemuriInteractable.h"
#include "KemuriGlassesComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UKemuriInteractionComponent::UKemuriInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.08f;
}

AActor* UKemuriInteractionComponent::FindFocus() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->GetController())
	{
		return nullptr;
	}

	FVector ViewLoc;
	FRotator ViewRot;
	Pawn->GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);
	const FVector End = ViewLoc + ViewRot.Vector() * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(KemuriInteract), false, Pawn);
	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(Hit, ViewLoc, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius), Params);
	if (!bHit || !Hit.GetActor())
	{
		return nullptr;
	}

	AActor* Candidate = Hit.GetActor();
	// Auch Elternactor pruefen (z. B. Child Actor in einem MetaHuman-Blueprint).
	for (int32 Depth = 0; Candidate && Depth < 3; ++Depth)
	{
		if (Candidate->GetClass()->ImplementsInterface(UKemuriInteractable::StaticClass()))
		{
			if (IKemuriInteractable::Execute_CanInteract(Candidate, GetOwner()))
			{
				return Candidate;
			}
			return nullptr;
		}
		Candidate = Candidate->GetAttachParentActor();
	}
	return nullptr;
}

void UKemuriInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* NewFocus = FindFocus();
	if (NewFocus != FocusedActor.Get())
	{
		if (bHighlightFocus)
		{
			SetHighlight(FocusedActor.Get(), false);
			SetHighlight(NewFocus, true);
		}
		FocusedActor = NewFocus;
		OnFocusChanged.Broadcast(NewFocus);
	}
	RefreshPrompt();
}

void UKemuriInteractionComponent::RefreshPrompt()
{
	FText Prompt = FText::GetEmpty();
	if (AActor* Focus = FocusedActor.Get())
	{
		Prompt = IKemuriInteractable::Execute_GetInteractPrompt(Focus, GetOwner());
	}
	if (!Prompt.EqualTo(LastPrompt))
	{
		LastPrompt = Prompt;
		if (UKemuriGlassesComponent* Glasses = UKemuriGlassesComponent::FindOn(GetOwner()))
		{
			Glasses->ShowPrompt(Prompt);
		}
	}
}

bool UKemuriInteractionComponent::TryInteract()
{
	AActor* Focus = FocusedActor.Get();
	if (!Focus || !IKemuriInteractable::Execute_CanInteract(Focus, GetOwner()))
	{
		return false;
	}
	IKemuriInteractable::Execute_Interact(Focus, GetOwner());
	RefreshPrompt();
	return true;
}

void UKemuriInteractionComponent::SetHighlight(AActor* Actor, bool bOn) const
{
	if (!Actor)
	{
		return;
	}
	TArray<UPrimitiveComponent*> Prims;
	Actor->GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* Prim : Prims)
	{
		if (Prim && Prim->IsVisible())
		{
			Prim->SetRenderCustomDepth(bOn);
			Prim->SetCustomDepthStencilValue(bOn ? HighlightStencilValue : 0);
		}
	}
}
