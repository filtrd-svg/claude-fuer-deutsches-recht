# KEMURI 煙 – Horror-Escape auf der Japan-Map (Unreal Engine 5)

Ein Mann sucht im verregneten japanischen Viertel einen Ort zum Rauchen, trifft einen Informanten, der nur für Tabak redet, und wird von etwas gejagt, das nur eine selbst gedruckte Waffe töten kann. Kein HUD, eine AI-Brille mit IR-Kamera, Nebel als Uhr und ein Waffensystem im Stil des Zombie-Modus.

## 1 Inhalt

| Pfad | Inhalt |
|---|---|
| `docs/01_Spielkonzept.md` | Story, Figuren, Akte, Informant, Brille, Nebel, Stalker, Waffen, Drucker, Scooter, Wald, Schreckmomente, Steuerung |
| `docs/02_Einkaufsliste.md` | Was es fertig gibt (Fab und andere) und was das Plugin abdeckt, mit Links |
| `docs/03_Figuren_aus_Fotos.md` | Informant und Stalker nach Fotos realer Personen: KeenFace, FaceBuilder, Headshot 3, MetaHuman, Markerless Mocap |
| `docs/04_Atmosphaere_Technik.md` | Projekteinstellungen, Licht, Nebel, Regen, IR-Material, Druck-Material, Brillen-Display, Klang, Leistung |
| `docs/05_Einrichtung_am_PC.md` | Schritt für Schritt vom Kopieren bis zum ersten Testlauf, dazu Claude im Editor über MCP |
| `Plugins/Kemuri/` | C++-Plugin mit allen Spielsystemen |
| `Tools/map_inventur.py` | Editor-Skript, das die gekaufte Map vermisst (NPCs, Schirme, Fahrräder, Licht, Nebel) |
| `Config/DefaultEngine_Kemuri.ini` | Render-Einstellungen zum Einfügen |
| `Vorlagen/CLAUDE.md` | Regeln für Claude Code im Unreal-Projekt |

## 2 Systeme im Plugin

| Klasse | Aufgabe |
|---|---|
| `KemuriPlayerCharacter` | Ich-Perspektive, Sprint mit Ausdauer, Schleichen, Taschenlampe, Nahkampf, Geräusche für die KI, Perks |
| `KemuriGlassesComponent` + `KemuriGlassesWidget` | AI-Brille: T tippen = IR-Kamera, T halten = App-Menü, Akku, Resonanz-Scanner, Live-Übersetzung, Prompts |
| `KemuriIRRevealComponent` | Objekte und Räume, die nur im IR-Bild existieren (oder nur ohne) |
| `KemuriFogDirector` | Nebel als Uhr: schrumpfender sicherer Kreis, Höhennebel, Nebel-Zonen, Nebelgestalten |
| `KemuriStalker` + `KemuriStalkerAIController` | Unsterblicher Jäger mit Sicht, Gehör, Leine, Suche, Betäubung, Rückzug, Zugriff |
| `KemuriEnemyBase` + `KemuriEnemyAIController` | Nebelgestalten mit Munitionseffekten (Blitz, Feuer, Frost, Salz) |
| `KemuriCivilianAIController`, `KemuriSplineFollowerComponent`, `KemuriFogVanishComponent` | Passanten und Radfahrer, die im Nebel verschwinden |
| `KemuriWeapon`, `KemuriWeaponData`, `KemuriWeaponHolderComponent` | Hitscan-Waffen, Datenassets, Waffenplätze |
| `KemuriWallBuy`, `KemuriOmikujiBox`, `KemuriUpgradeShrine`, `KemuriPerkVending` | Wandkauf, Zufallsschrein, Weihe und Munitionssegen, Getränkeautomaten |
| `KemuriInformant` | Raucher mit Hinweisen gegen Zigaretten (vage) oder Weed (präzise, setzt IR-Markierungen) |
| `KemuriPrinterStation` | 3D-Drucker: Bauteile, Kalibrierung, Filament, Druck mit prozeduralem Druckkopf, Belagerung |
| `KemuriScooter` | Fahrbarer E-Scooter mit eigener Fahrphysik, Akku, Licht und Lärm |
| `KemuriFearComponent`, `KemuriFlickerComponent` | Angst ohne HUD (Bildeffekte, Herzschlag, Atem), flackernde Lampen |
| `KemuriSafeZone`, `KemuriScareTrigger` | Raucherbereich als sicherer Ort, dosierte Schreckmomente |
| `KemuriWorldSubsystem` | Story-Flags, IR-Zustand, Registrierungen, Photosensitivitäts-Regler |

## 3 Schnellstart

1. `docs/05_Einrichtung_am_PC.md` Abschnitt 1 bis 3: Projekt auf C++ umstellen, Plugin kopieren, bauen.
2. Abschnitt 4: `Tools/map_inventur.py` in der Map ausführen und das Ergebnis zurückschicken.
3. Abschnitt 5: Spielfigur, Eingabe, Brille, Nebel, Kuroda und Stalker einrichten.
4. Abschnitt 6: Erster Testlauf.

## 4 Stand

Der C++-Code ist vollständig geschrieben und strukturell geprüft: Klammern, Includes und für alle 263 deklarierten Funktionen eine Implementierung. Mit einer Unreal-Installation kompiliert wurde er nicht, weil in der Arbeitsumgebung keine verfügbar war. Der erste Build auf deinem PC ist daher der eigentliche Test. Fehlermeldungen bitte unverändert zurückschicken.
