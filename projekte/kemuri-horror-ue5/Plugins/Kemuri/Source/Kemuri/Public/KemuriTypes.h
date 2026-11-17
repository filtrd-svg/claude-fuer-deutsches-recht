#pragma once

#include "CoreMinimal.h"
#include "KemuriTypes.generated.h"

class USoundBase;

/** Munitionsmodifikation, freigeschaltet durch Schrein-Reliquien. */
UENUM(BlueprintType)
enum class EKemuriAmmoMod : uint8
{
	None		UMETA(DisplayName = "Keine"),
	Raijin		UMETA(DisplayName = "Raijin (Kettenblitz)"),
	Kitsunebi	UMETA(DisplayName = "Kitsunebi (Fuchsfeuer)"),
	Yuki		UMETA(DisplayName = "Yuki (Frost)"),
	Shio		UMETA(DisplayName = "Shio (Reinigungssalz)")
};

/** Getraenke aus Automaten mit dauerhaftem Effekt. */
UENUM(BlueprintType)
enum class EKemuriPerk : uint8
{
	TetsuCha	UMETA(DisplayName = "Tetsu-Cha (mehr Leben)"),
	HayaiKohi	UMETA(DisplayName = "Hayai-Kohi (schnelles Nachladen)"),
	MittsuRamune UMETA(DisplayName = "Mittsu-Ramune (dritter Waffenplatz)"),
	MamoriMizu	UMETA(DisplayName = "Mamori-Mizu (Selbstwiederbelebung)"),
	KazeSoda	UMETA(DisplayName = "Kaze-Soda (leise und schnell)")
};

/** Apps der Brille im T-Menue. */
UENUM(BlueprintType)
enum class EKemuriGlassesApp : uint8
{
	Camera		UMETA(DisplayName = "Kamera (IR)"),
	Map			UMETA(DisplayName = "Karte"),
	Notes		UMETA(DisplayName = "Hinweise"),
	Parts		UMETA(DisplayName = "Bauteile"),
	Resonance	UMETA(DisplayName = "Resonanz-Scanner"),
	Battery		UMETA(DisplayName = "Akku")
};

UENUM(BlueprintType)
enum class EKemuriStalkerState : uint8
{
	Roam		UMETA(DisplayName = "Umherstreifen"),
	Investigate	UMETA(DisplayName = "Geraeusch untersuchen"),
	Chase		UMETA(DisplayName = "Verfolgung"),
	Search		UMETA(DisplayName = "Suche"),
	Stunned		UMETA(DisplayName = "Betaeubt"),
	Retreat		UMETA(DisplayName = "Rueckzug"),
	Catch		UMETA(DisplayName = "Zugriff")
};

UENUM(BlueprintType)
enum class EKemuriPrinterState : uint8
{
	Incomplete	UMETA(DisplayName = "Teile fehlen"),
	Assembled	UMETA(DisplayName = "Zusammengebaut, unkalibriert"),
	Calibrated	UMETA(DisplayName = "Kalibriert, Filament fehlt"),
	Ready		UMETA(DisplayName = "Druckbereit"),
	Printing	UMETA(DisplayName = "Druckt"),
	Paused		UMETA(DisplayName = "Pausiert (kein Strom)"),
	Done		UMETA(DisplayName = "Fertig")
};

/** Eine Hinweis-Zeile des Informanten. Zigarette = vage, high = praezise. */
USTRUCT(BlueprintType)
struct FKemuriHint
{
	GENERATED_BODY()

	/** Hinweis gilt erst, wenn dieses Flag gesetzt ist (leer = sofort). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	FName RequiredFlag;

	/** Hinweis entfaellt, sobald dieses Flag gesetzt ist (z. B. Aufgabe erledigt). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	FName CompletedFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis", meta = (MultiLine = true))
	FText VagueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis", meta = (MultiLine = true))
	FText PreciseText;

	/** Wird beim vagen Hinweis gesetzt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	FName FlagOnVague;

	/** Wird beim praezisen Hinweis gesetzt, z. B. um eine IR-Markierung sichtbar zu machen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	FName FlagOnPrecise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	TObjectPtr<USoundBase> VagueVoice = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hinweis")
	TObjectPtr<USoundBase> PreciseVoice = nullptr;
};

/** Zentrale Item-IDs. Frei erweiterbar; nur diese werden im Code direkt verwendet. */
namespace KemuriItems
{
	static const FName Mon(TEXT("Mon"));
	static const FName Battery(TEXT("Akku"));
	static const FName Cigarettes(TEXT("Zigaretten"));
	static const FName Weed(TEXT("Weed"));
	static const FName RelicRaijin(TEXT("Reliquie_Raijin"));
	static const FName RelicKitsune(TEXT("Reliquie_Kitsune"));
	static const FName RelicYuki(TEXT("Reliquie_Yuki"));
	static const FName RelicShio(TEXT("Reliquie_Shio"));
}

/** Zentrale Flags, die der Code selbst setzt. */
namespace KemuriFlags
{
	static const FName PlayerInSafeZone(TEXT("Spieler_ImRaucherbereich"));
	static const FName StalkerDefeated(TEXT("Stalker_Besiegt"));
	static const FName PrintStarted(TEXT("Druck_Gestartet"));
	static const FName PrintDone(TEXT("Druck_Fertig"));
}
