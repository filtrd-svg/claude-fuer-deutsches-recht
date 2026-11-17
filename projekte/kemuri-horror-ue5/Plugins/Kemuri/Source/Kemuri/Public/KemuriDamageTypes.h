#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "KemuriDamageTypes.generated.h"

/** Basisklasse aller KEMURI-Schadenstypen. */
UCLASS(Blueprintable)
class KEMURI_API UKemuriDamageType : public UDamageType
{
	GENERATED_BODY()
};

/** Nur dieser Schaden (die gedruckte HARAI-Waffe) kann den Stalker toeten. */
UCLASS(Blueprintable)
class KEMURI_API UKemuriFinalDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class KEMURI_API UKemuriMeleeDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class KEMURI_API UKemuriFireDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class KEMURI_API UKemuriLightningDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class KEMURI_API UKemuriFrostDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class KEMURI_API UKemuriPurifyDamageType : public UKemuriDamageType
{
	GENERATED_BODY()
};
