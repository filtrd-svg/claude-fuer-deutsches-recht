# KEMURI – Licht, Nebel, Regen, IR-Kamera, Klang

Ziel ist ein Bild, das wie eine verregnete Nachtaufnahme mit alter Kameralinse wirkt: tiefe Schwärzen, wenige, harte Lichtquellen, Natriumdampf-Orange und kaltes Neon im Nebel, nasse Spiegelungen überall.

## 1 Projekteinstellungen (UE 5.8)

| Einstellung | Wert | Warum |
|---|---|---|
| Rendering → Global Illumination | Lumen | Indirektes Licht von Neon, Automaten und Taschenlampe |
| Rendering → Reflections | Lumen | Nasse Straßen spiegeln die Lichter |
| Rendering → Lumen → Hardware Ray Tracing | an, wenn die GPU es kann | Saubere Spiegelungen in Pfützen |
| Rendering → MegaLights | an | In 5.8 produktionsreif. Viele flackernde Lampen mit Schatten werden bezahlbar. |
| Rendering → Volumetric Fog | an | Lichtkegel im Nebel |
| Rendering → Custom Depth-Stencil Pass | Enabled with Stencil | IR-Glühen (Stencil 2), Fokus-Umriss (Stencil 1), Stalker-Augen (Stencil 3) |
| Rendering → Virtual Shadow Maps | an | Scharfe Schatten von Laternen |
| Rendering → Default Settings → Auto Exposure | aus oder stark begrenzt | Dunkelheit soll dunkel bleiben |
| Rendering → Default Settings → Motion Blur | aus | Klare Bewegungen, weniger Übelkeit |

Die Datei `Config/DefaultEngine_Kemuri.ini` enthält die passenden Zeilen zum Einfügen.

## 2 Grundlicht

Den Mond als Directional Light mit sehr geringer Intensität (etwa 0,05 bis 0,2 Lux), leicht blau, mit Volumetric Scattering Intensity 2. Die Sky Light Intensität minimal halten, damit Schatten wirklich schwarz sind. Straßenlaternen als Spot Lights mit Natriumdampf-Farbe (Temperatur etwa 2.000 K) und Volumetric Scattering Intensity 3 bis 5, damit sie im Nebel Kegel werfen. Automaten, Konbini und Neonschilder als Rect Lights mit kalter Farbe. Jede zweite Lampe bekommt `KemuriFlicker`; die Komponente lässt Lampen in der Nähe des Stalkers verrückt spielen.

Der Grundlook im Post Process Volume (unbound): Film Grain 0,15, Vignette 0,45, leichte chromatische Aberration (die Angst-Komponente erhöht diese Werte live), Teal-Orange-Grading mit abgesenkten Mitten, Bloom gering, Lens Flares aus.

## 3 Nebel

Der Nebel hat drei Schichten. Der Exponential Height Fog bildet die Grundschicht; seine Dichte steuert der `KemuriFogDirector` nach dem Nebel am Spieler. Volumetric Fog im Height Fog aktivieren, Extinction Scale steuert ebenfalls der Direktor. Local Fog Volumes setzen dichte Bänke in Gassen, Unterführungen und am Waldrand. Diese Actors trägst du im Direktor unter `FogZoneActors` ein; sie erscheinen, sobald der Nebel sie erreicht. Niagara-Schwaden bringen bodennahe Bewegung, besonders vor Lichtquellen, und gehören ebenfalls in `FogZoneActors`.

Für die Material Parameter Collection `MPC_KemuriAtmosphere` folgende Skalarparameter anlegen: `Kemuri_FogProgress`, `Kemuri_SafeRadius`, `Kemuri_FogAtPlayer`, `Kemuri_Fear`. Dazu den Vektorparameter `Kemuri_FogCenter`. Materialien können daraus nasse Ränder, Kondenswasser auf Scheiben oder sterbende Neonschilder ableiten.

## 4 Regen

Mit Ultra Dynamic Weather: Regen auf Starkregen stellen, Wetness und Pfützen an, Blitze selten (alle 40 bis 90 Sekunden), aber hell. Blitze sind die besten Momente, um den Stalker in der Ferne zu zeigen.

Ohne Kaufpaket: ein Niagara-System mit GPU-Partikeln in einer Box um die Kamera, Spritzer per Kollision mit Depth Buffer und ein Material-Layer für Nässe (Roughness runter, Basisfarbe dunkler) über einen Parameter in der MPC.

Wichtig ist Regen auf der Brille: ein dezentes Overlay im Brillen-Widget mit Tropfen, die bei hoher Angst schneller laufen. Nimm keine Tropfen auf dem Bildschirm selbst, die Brille ist die Linse.

## 5 IR-Kamera-Material (M_PP_IRCamera)

Neues Material anlegen: Material Domain = Post Process, Blendable Location = Before Tonemapping. Knoten in dieser Reihenfolge:

1. `SceneTexture: PostProcessInput0` liefert das Bild. Daraus mit `Desaturation` (Fraction 1) Graustufen machen.
2. Mit einem Skalarparameter `Gain` (Standard 6) multiplizieren, damit der IR-Strahler dunkle Räume aufhellt.
3. Rauschen addieren: `Noise`-Knoten oder eine Rausch-Textur, deren UV mit `Time` verschoben wird, Stärke 0,08.
4. Kontrast: `Power` mit Exponent 1,4, danach `Saturate`.
5. Tönung: mit Weiß-Grün (0,75; 1,0; 0,8) multiplizieren. Für den Look einer Smart-Brille statt eines Militärgeräts passt ein neutrales Weiß-Grau besser.
6. IR-Glühen: `SceneTexture: CustomStencil` lesen. Ist der Wert 2 (IR-Objekte), wird mit einem `If`-Knoten das Ergebnis mit 2,5 multipliziert. Ist er 3 (Stalker-Augen), wird reines Weiß ausgegeben.
7. Linse: radialer Verlauf (`RadialGradientExponential` auf `ScreenPosition`) als Vignette und leichte Tonnenverzeichnung über verschobene UVs am Rand.
8. Zeilen: `sin(ScreenPosition.y * 900)` mit 0,03 Stärke addieren.
9. Ergebnis in Emissive Color.

Das Material im Brillen-Bauteil des Spieler-Blueprints unter `IRPostProcessMaterial` eintragen. REC-Punkt, Zeitstempel und Akkuwarnung gehören ins Brillen-Widget, nicht ins Material.

## 6 Material für das gedruckte Teil (M_PrintedPart)

Das gedruckte HARAI-Teil wächst von unten nach oben. Im Material einen Skalarparameter `PrintProgress` anlegen (der Drucker setzt ihn), dazu die Höhe des Modells als Parameter `PartHeight`.

1. `LocalPosition` (oder `Object Position` abgezogen von `Absolute World Position`) liefert die Höhe Z jedes Pixels.
2. `Z / PartHeight` mit `PrintProgress` vergleichen: Pixel oberhalb werden über die Opacity Mask (Blend Mode Masked) verworfen.
3. Ein schmales Band direkt unter der Schnittkante leuchtet orange (frisch geschmolzenes Filament) und kühlt über die nächsten 2 Prozent ab.
4. Feine horizontale Rillen im Normal-Kanal mit `frac(Z / 0,02)` simulieren die Schichtlinien.

## 7 Brillen-Display (WBP_KemuriGlasses)

Das Widget erbt von `KemuriGlassesWidget`. Im Normalzustand ist es vollständig leer. Gestaltung: eine dünne, schmale Schrift in kaltem Weiß-Cyan mit leichter Transparenz, ein sehr leichter Doppelbild-Versatz von 1 Pixel (Brillenglas) und Elemente nur im unteren Drittel und am rechten Rand, so wie ein echtes Display im Brillenglas. Kein Element bleibt dauerhaft sichtbar.

| Event | Darstellung |
|---|---|
| `OnSubtitle` | Sprechername klein, Text darunter, einblenden, nach `Duration` ausblenden |
| `OnPrompt` | Eine Zeile rechts unten, z. B. „Waffe weihen [5000 Mon]“ |
| `OnMenuOpened` / `OnMenuSelectionChanged` | Horizontale App-Leiste mit sechs Symbolen, gewählte App größer |
| `OnAppActivated` | Karte, Hinweise, Bauteile oder Akku für vier Sekunden zeigen |
| `OnCameraModeChanged` | REC-Punkt, Uhrzeit, kleiner Akkustand, leichter Rand-Glow |
| `OnBatteryCritical` | Bildfehler, Zeilenversatz, kurzes Schwarz |

## 8 Klang

Für den Herzschlag eine MetaSound Source mit Float-Eingang `Fear` anlegen. Bei 0 ist kaum etwas zu hören, ab 0,5 wird es tiefer und schneller, und ab 0,8 kommt ein Tinnitus-Ton hinzu. Für den Atem einen zweiten MetaSound mit `Fear` bauen, gekoppelt an Ausdauer über das Blueprint-Event `OnOutOfBreath`.

Regen in Schichten aufbauen: eine Grundschicht über die ganze Map, Audio Volumes mit eigenem Reverb und eigener Regenschicht für Gassen, Unterführungen und Innenräume sowie punktuelle Emitter an Blechdächern, Schirmen und Regenrinnen. Okklusion für alle 3D-Sounds aktivieren. Mit Kopfhörern wirkt ein HRTF-Spatialisierungs-Plugin enorm; prüfe in den Projekteinstellungen unter Plattformen → Windows → Audio, welches in deiner Version verfügbar ist.

Der Stalker hat eigene Schritte mit Wasserplatschen. Nahe beim Spieler wird der Regen über eine Sound Class leiser gedreht, während Tropfen weiterfallen; dieser Kontrast ist ein starkes Warnsignal.

## 9 Leistung

Zielwert sind 60 Bilder pro Sekunde in 1440p auf einer Grafikkarte der Mittelklasse aktueller Generation. Die größten Kostentreiber sind Volumetric Fog, Lumen-Reflexionen und Regenpartikel. Für schwächere Rechner Lumen Lite (neu in 5.8) als Option anbieten und `r.VolumetricFog.GridPixelSize` von 4 auf 8 erhöhen. Nebelgestalten über `MaxWraithsAtFull` begrenzen.
