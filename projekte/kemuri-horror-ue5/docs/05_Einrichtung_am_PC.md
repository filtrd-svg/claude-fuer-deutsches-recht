# KEMURI – Einrichtung am PC, Schritt für Schritt

Diese Anleitung bringt das Plugin in dein Japan-Map-Projekt und baut ein erstes spielbares Grundgerüst. Rechne für Abschnitt 1 bis 4 mit etwa einer Stunde, für Abschnitt 5 mit einem Abend.

## 1 Voraussetzungen

Empfohlen ist Unreal Engine 5.8. Das Plugin verwendet nur Schnittstellen, die seit 5.4 stabil sind, und sollte daher auch mit 5.6 und 5.7 bauen. Für den Build brauchst du Visual Studio 2022 mit der Arbeitslast „Spieleentwicklung mit C++“. Welche Visual-Studio-Version Epic für 5.8 genau verlangt, steht in den Release Notes der Engine. Wenn Epic dort eine neuere Version vorschreibt, gilt diese.

Lege vor allem anderen eine Sicherheitskopie des Projektordners an oder richte Git mit LFS bzw. Diversion ein. Die Fab-Map selbst änderst du nie direkt, sondern nur über eigene Blueprints und Kopien.

## 2 Projekt auf C++ umstellen

Falls dein Projekt bisher nur Blueprints hat:

1. Projekt im Editor öffnen.
2. Tools → New C++ Class → None (leere Klasse) → Name `KemuriBootstrap` → Create Class.
3. Der Editor erzeugt den Ordner `Source` und die Visual-Studio-Projektmappe. Danach den Editor schließen.

## 3 Plugin einbauen und bauen

1. Den Ordner `Plugins/Kemuri` aus diesem Paket nach `<DeinProjekt>/Plugins/Kemuri` kopieren.
2. Rechtsklick auf die `.uproject`-Datei → Generate Visual Studio project files.
3. Die `.sln` öffnen, Konfiguration „Development Editor“ und „Win64“ wählen, dann Build → Build Solution.
4. Projekt starten (F5 in Visual Studio oder Doppelklick auf `.uproject`).
5. Unter Edit → Plugins prüfen, dass „KEMURI Horror Kit“, „Enhanced Input“ und „Niagara“ aktiv sind.

Der Code wurde nicht mit einer Unreal-Installation kompiliert, weil hier keine verfügbar war. Wenn der Build Fehler meldet, kopiere die ersten Fehlermeldungen aus dem Ausgabefenster (Registerkarte „Error List“ oder Output → Build) und schick sie mir. Meist sind es Einzeiler.

## 4 Die Map vermessen

1. Edit → Plugins → „Python Editor Script Plugin“ aktivieren und den Editor neu starten.
2. Die Japan-Map öffnen.
3. Tools → Execute Python Script… → `Tools/map_inventur.py` aus diesem Paket wählen.
4. Die Datei `<DeinProjekt>/Saved/KemuriInventur.md` an mich schicken.

Daran sehe ich, wie die NPCs mit Schirm und Fahrrad gebaut sind (Pawns mit KI, Blueprints, Mass Crowd oder einfache Meshes). Davon hängt ab, ob sie `KemuriCivilianAIController`, `KemuriSplineFollower` oder eine Anpassung an das vorhandene System bekommen.

## 5 Grundgerüst im Editor

### 5.1 Ordner

Unter `Content` den Ordner `Kemuri` mit den Unterordnern `Blueprints`, `Data`, `Input`, `Materials`, `UI`, `Audio` und `Sequences` anlegen.

### 5.2 Eingabe

In `Kemuri/Input` die folgenden Input Actions (Rechtsklick → Input → Input Action) und einen Input Mapping Context `IMC_Kemuri` anlegen:

| Input Action | Value Type | Belegung in IMC_Kemuri |
|---|---|---|
| IA_Move | Axis2D | W (Swizzle YXZ), S (Swizzle YXZ + Negate), D, A (Negate) |
| IA_Look | Axis2D | Maus XY (Negate Y) |
| IA_Jump | Digital | Leertaste |
| IA_Sprint | Digital | Linke Umschalttaste |
| IA_Crouch | Digital | Linke Strg-Taste |
| IA_Fire | Digital | Linke Maustaste |
| IA_Aim | Digital | Rechte Maustaste |
| IA_Reload | Digital | R |
| IA_Interact | Digital | E |
| IA_SwitchWeapon | Digital | Mausrad hoch, Q |
| IA_Melee | Digital | V |
| IA_Glasses | Digital | T |
| IA_Flashlight | Digital | F |

Der Scooter nutzt dieselben Aktionen: IA_Move zum Fahren, IA_Look zum Umsehen, IA_Jump zum Bremsen, IA_Interact zum Absteigen und IA_Flashlight für den Scheinwerfer.

### 5.3 Spielfigur und GameMode

1. Blueprint-Klasse von `KemuriPlayerCharacter` anlegen: `BP_KemuriPlayer`.
2. In `BP_KemuriPlayer` bei `Arms` ein Arm-Mesh zuweisen (aus dem Waffenpaket oder die Arme aus dem First-Person-Template). Das Mesh braucht einen Socket `GripPoint` für Waffen.
3. Unter Kemuri → Eingabe `IMC_Kemuri` und alle Input Actions zuweisen.
4. Unter Glasses → `Widget Class` das Widget `WBP_KemuriGlasses` und unter `IR Post Process Material` das Material `M_PP_IRCamera` eintragen (beides nach `04_Atmosphaere_Technik.md` bauen).
5. Unter Fear → Herzschlag- und Atem-Loop sowie `MPC_KemuriAtmosphere` eintragen.
6. Unter Weapon Holder → `Starting Weapon` den Dienstrevolver als Datenasset eintragen.
7. Blueprint-GameMode `BP_KemuriGameMode` anlegen, Default Pawn Class = `BP_KemuriPlayer`, und in den World Settings der Map als GameMode Override setzen.

### 5.4 Waffen-Datenassets

Rechtsklick → Miscellaneous → Data Asset → `KemuriWeaponData`. Startwerte:

| Asset | Schaden | Kugeln | Streuung | Schuss/min | Auto | Magazin | Reserve | Nachladen |
|---|---|---|---|---|---|---|---|---|
| DA_Waffe_Revolver | 70 | 1 | 1,0 | 200 | nein | 6 | 48 | 2,2 |
| DA_Waffe_Revolver_Geweiht | 160 | 1 | 0,8 | 240 | nein | 8 | 96 | 1,8 |
| DA_Waffe_Schrotflinte | 28 | 8 | 7,0 | 70 | nein | 6 | 36 | 3,0 |
| DA_Waffe_MP | 34 | 1 | 2,5 | 750 | ja | 30 | 180 | 2,0 |
| DA_Waffe_Jagdgewehr | 220 | 1 | 0,3 | 50 | nein | 5 | 30 | 3,2 |
| DA_Waffe_HARAI | 900 | 1 | 0,2 | 40 | nein | 3 | 6 | 4,0 |

Bei jeder Basiswaffe `Upgraded Version` auf die geweihte Variante setzen, bei der geweihten `Is Upgraded` anhaken. Bei HARAI `Is Final Weapon` anhaken und `Damage Type` auf `KemuriFinalDamageType` stellen.

### 5.5 Welt bestücken

| Actor | Wo | Einstellungen |
|---|---|---|
| NavMeshBoundsVolume | Über das gesamte spielbare Gebiet inklusive Wald | Ohne Navmesh bewegen sich weder Stalker noch Gestalten noch Passanten |
| KemuriFogDirector | Mitte des Raucherbereichs | Height Fog aus der Map eintragen, MPC, Fog Zone Actors, Wraith Class = `BP_Nebelgestalt` |
| KemuriSafeZone | Um das Raucherhäuschen | Box-Größe an das Häuschen anpassen |
| BP_Kuroda (MetaHuman, Parent `KemuriInformant`) | Im Raucherhäuschen | Hints eintragen (Texte in `01_Spielkonzept.md`, 4.1) |
| BP_Stalker (Parent `KemuriStalker`) | Am Rand des Viertels | Mesh, Animation Blueprint, Stinger-Sound |
| BP_Nebelgestalt (Parent `KemuriEnemyBase`) | Nicht platzieren, der Direktor spawnt | Mesh, Animation Blueprint, Gesundheit 150 |
| KemuriWallBuy | An Wänden | Datenasset, Preis; Mesh = Kreideumriss als Decal-Plane |
| KemuriOmikujiBox | Drei bis vier Orte, genau einer mit `Starts Active` | Pool mit Gewichten |
| KemuriPerkVending | An vorhandenen Getränkeautomaten | Perk, Getränkename, Preis, `Power Flag = Strom_An` |
| KemuriUpgradeShrine | Am Okumiya im Wald | Preise |
| KemuriPrinterStation | Werkstatt (später Raucherbereich) | Final Weapon = `DA_Waffe_HARAI`, Fog Director eintragen |
| KemuriPickup | Bauteile, Reliquien, Zigaretten, Weed, Akkus | `Item Id` exakt wie in Spielkonzept und Code, z. B. `Bauplan_1`, `Reliquie_Raijin`, `Zigaretten`, `Weed`, `Akku` |
| KemuriScooter | Ladesäule am Bahnhof | Body-Mesh aus dem Scooter-Paket, Input-Assets wie beim Spieler |
| KemuriScareTrigger | An den Orten aus `01_Spielkonzept.md`, Abschnitt 5.2 | Bedingungen und Auslösung je Szene |

IR-Inhalte bekommen die Komponente `KemuriIRReveal`: Schriftzeichen, versteckte Weed-Pakete, Fußspuren, Kurodas Markierungen (mit `Required Flag`). Lampen bekommen `KemuriFlicker`. NPCs mit Schirm bekommen `KemuriFogVanish`, Radfahrer zusätzlich `KemuriSplineFollower` mit einem Spline-Actor als Route.

### 5.6 Speichern

Das Speichern ist bewusst nicht im Plugin, weil es stark vom Projekt abhängt. Lege eine SaveGame-Klasse an, die Inventar, Flags (über `Get KemuriWorldSubsystem`), Waffen und Spielerposition speichert. Auslöser ist das Rauchen im Raucherbereich: ein Interaktionspunkt, der eine Zigarette verbraucht.

## 6 Erster Testlauf

1. Spiel starten, T tippen: Das Bild wechselt in den IR-Modus, und der Akku sinkt.
2. T halten und die Maus bewegen: Das App-Menü erscheint, beim Loslassen startet die App.
3. Zu Kuroda gehen und E drücken: Er fragt nach einer Zigarette. Mit einem Pickup `Zigaretten` erneut versuchen: Der Untertitel erscheint, und der Nebel beginnt.
4. Den Stalker im Gameplay Debugger beobachten (Apostroph-Taste im Spiel, Kategorie AI): Zustand, Wahrnehmung und Ziel prüfen.
5. Schießen: Mon steigt bei Treffern auf Nebelgestalten. Den Stand im Inventar-Debugging oder per Print String aus `Get Mon` prüfen.
6. Scooter: E zum Aufsteigen, fahren, E zum Absteigen unter 9 km/h.

## 7 Claude direkt im Editor arbeiten lassen (optional)

UE 5.8 bringt laut mehreren Drittanbieter-Anleitungen ein experimentelles MCP-Plugin mit, über das Claude Code den Editor steuern kann. Epic selbst habe ich dazu nicht gelesen; prüfe die Schritte deshalb gegen die aktuelle Epic-Dokumentation.

1. Edit → Plugins: „Unreal MCP“ und das zugehörige Toolset-Plugin aktivieren, Editor neu starten.
2. Editor Preferences → Model Context Protocol → Auto Start Server, alternativ in der Konsole `ModelContextProtocol.StartServer`.
3. In der Konsole `ModelContextProtocol.GenerateClientConfig ClaudeCode` ausführen. Das schreibt eine `.mcp.json` in den Projektordner.
4. Die Datei `Vorlagen/CLAUDE.md` aus diesem Paket in den Projektordner kopieren und den Ordner `docs` nach `<DeinProjekt>/Docs/KEMURI` kopieren, damit die Verweise darin stimmen.
5. Im Projektordner `claude` starten, den Server bestätigen und mit `/mcp` prüfen.

Danach kann Claude Code Blueprints, Datenassets und Actors nach diesen Dokumenten anlegen. Vor größeren Änderungen immer speichern und einen Commit machen.

## 8 Häufige Probleme

| Problem | Ursache und Lösung |
|---|---|
| „Missing modules, rebuild?“ beim Start | Projekt ist noch nicht C++ oder Visual Studio fehlt. Abschnitt 2 und 3 ausführen. |
| Schüsse treffen Gegner nicht | Das Mesh blockt den Kanal Visibility nicht. Im Gegner-Blueprint am Mesh Collision Presets → Custom → Visibility = Block, und ein Physik-Asset zuweisen. |
| Stalker oder Gestalten bewegen sich nicht | Kein Navmesh. NavMeshBoundsVolume platzieren, mit P im Editor die grüne Fläche prüfen. |
| Keine Nebelgestalten | `Wraith Class` im Direktor leer, Nebel noch nicht gestartet (Flag `Nebel_Start` kommt beim ersten Gespräch mit Kuroda) oder keine Navmesh-Punkte im Nebel. |
| IR-Bild bleibt normal | `IR Post Process Material` leer oder Blendable Location falsch; Custom Depth-Stencil Pass auf „Enabled with Stencil“ stellen. |
| Brillen-Text erscheint nicht | `Widget Class` leer oder Events im Widget nicht implementiert. |
