# KEMURI – Kaufen oder selbst bauen

Recherchestand: 17.11.2026. Preise, Versionen und Lizenzen ändern sich. Vor jedem Kauf auf der Listing-Seite die unterstützte Engine-Version (Ziel: 5.8) und die Lizenz prüfen. Fab-Assets mit Standardlizenz dürfen in eigenen Spielen verwendet werden.

## 1 Ergebnis in einem Satz

Kaufen lohnt sich für Kunst und Klang (Wetter, Animationen, Waffenmodelle, Sounds, Wald- und Schrein-Umgebung, Druckermodell). Die Spielsysteme selbst gibt es in dieser Kombination nicht fertig zu kaufen; sie stecken vollständig im Plugin `Kemuri`.

## 2 Spielsysteme: Markt gegen KEMURI-Plugin

| System | Kommerziell verfügbar? | Empfehlung |
|---|---|---|
| Waffen im Zombie-Modus-Stil (Punkte, Wandkauf, Zufallskiste, Upgrade, Perks) | Teilweise: [Multiplayer Zombie Survival Template](https://www.fab.com/listings/8ed61cc4-2b0b-4a20-bdc9-032248b308a7) mit Zufallskiste und Punkten, dazu [Multiplayer Perk System](https://www.fab.com/listings/843e2b1e-9fb7-4904-83fd-83be69ec36c5). Wandkauf und Upgrade fehlen; das Template ist alt und auf Mehrspieler ausgelegt. | Plugin verwenden. Ein Template würde mit Brille, Nebel und Stalker kollidieren. |
| Stalker-KI mit Sicht, Gehör, Verfolgung | Ja, in Horror-Templates: [Ultimate Horror Template](https://www.fab.com/listings/79445fc7-a754-47c7-a7a0-e35315de030b), [The Horror Framework](https://www.fab.com/listings/acae9820-9b81-421a-b497-518801e94cf8), [First Person Horror Kit](https://www.fab.com/listings/c87594f7-33a0-4f8f-9d04-316f69409674) | Plugin verwenden (Leinen-Prinzip, Raucherbereich, Lockruf, Betäubung). Ein Template kaufen nur dann, wenn du fertige Türen und Verstecke willst. |
| Nachtsicht/IR als Post-Process | Ja: Fab-Pakete wie „Thermal Vision & Night Vision – Ultimate Post Process Material“ und „Tactical Vision System“ ([Fab-Suche](https://www.fab.com/search?q=night%20vision%20thermal)) | Optional kaufen, wenn das eigene Material (Rezept in `04_Atmosphaere_Technik.md`) nicht reicht. Das Material wird im Brillen-Bauteil eingehängt. |
| Brille mit T-Menü, IR-only-Räume, Live-Übersetzung | Nein | Plugin |
| Nebel als Uhr mit Spawns | Nein | Plugin |
| Informant mit Rauch-Ökonomie | Nein | Plugin |
| 3D-Drucker-Simulation | Nein | Plugin |
| Fahrbarer E-Scooter | Nein. Es gibt nur das Modell [Electric Scooter](https://www.fab.com/listings/04c4f57a-cb81-45a0-afee-5bdfb9082958) ohne Fahrlogik. | Modell kaufen, Fahrlogik aus dem Plugin |

## 3 Assets zum Kaufen

### 3.1 Himmel, Regen, Blitze

[Ultra Dynamic Sky](https://www.fab.com/search?q=Ultra%20Dynamic%20Sky) enthält Ultra Dynamic Weather mit Regen, Blitzen, Pfützen und Wetterzonen. Laut Fab-Suchergebnis kostet es etwa 200 USD; die Freigabe für 5.8 war nicht bestätigt. Es ist die schnellste Lösung für glaubwürdigen Starkregen mit nassen Oberflächen. Wichtig: Den Nebel steuert der KEMURI-Nebel-Direktor. Im Ultra Dynamic Sky daher den eigenen Nebel deaktivieren oder dessen Height-Fog-Actor im Direktor eintragen.

### 3.2 Wald und Schreine

| Asset | Inhalt | Link |
|---|---|---|
| Mountain Temple (Japanese Shrine Temple Environment) | Ginkgo- und Ahornwälder mit Nanite, Moos, Steinlaternen in verwitterten Varianten, Torii mit Seilen und Glocken | [Fab](https://www.fab.com/listings/b50950cd-12c9-4a31-b083-dc7e1f4174c8) |
| Japanese Modular Shrine (With Interiors), Jemini Studio | Haupthalle, kleine Schreine, Glockenturm, Tore, mit Innenräumen | [Forum-Vorstellung mit Fab-Link](https://forums.unrealengine.com/t/jemini-studio-japanese-modular-shrine-with-interiors/2753765) |

### 3.3 Waffen und Arme

Ein Fab-Paket mit realistischen Waffen (Pistole, Pump-Schrotflinte, Maschinenpistole, Gewehre, Messer) und über 400 Animationen für Ich- und Third-Person-Ansicht ist auf das UE5-Skelett geriggt und MetaHuman-kompatibel: [Fab-Listing](https://www.fab.com/listings/70ce0c19-5efc-486a-bbef-b5a79e382953). Die Waffenmeshes kommen in die Datenassets `DA_Waffe_*`; die Animationen hängen an den Blueprint-Events `OnFired` und `OnReloadStarted`.

### 3.4 Animationen für Stalker und Nebelgestalten

| Paket | Warum | Link |
|---|---|---|
| Jiffy – Haunted Hosts | Geloopte Mocap für Horror-NPCs, Epic-Skelett, MetaHuman-tauglich | [Forum mit Fab-Link](https://forums.unrealengine.com/t/jiffy-haunted-hosts-looped-mocap-for-zombies-horror-npcs/2841997) |
| Hyper Zombie Chase / Zombie-Mocap-Pakete | Sprint, Kriechen, Greifen, Tod; IK-Retargeter für Manny | [Fab-Suche](https://www.fab.com/search?q=zombie%20mocap) |
| MetaHuman Cinema Pack: Zombie Mocap Vol.1 | 172 Animationen, MetaHuman-ready | [Fab-Suche](https://www.fab.com/search?q=MetaHuman%20Cinema%20Pack%20Zombie) |

Für Kurodas Rauchen und Gesten ist keine Kaufanimation nötig: Die reale Person spielt sie vor der Webcam, und das kostenlose Markerless-Mocap-Plugin von Epic überträgt sie (siehe `03_Figuren_aus_Fotos.md`).

### 3.5 Sound

| Paket | Inhalt | Link |
|---|---|---|
| Dark Fantasy Studio: NERVE | 162 Horror-Effekte mit nahtlosen Loops | [Forum](https://forums.unrealengine.com/t/dark-fantasy-studio-1-nerve-162-horror-sound-effects/2846111) |
| Dark Fantasy Studio: PSYCHOSIS | Flüsternde Drones, ferne Echos | [Forum](https://forums.unrealengine.com/t/dark-fantasy-studio-1-psychosis-35-horror-sound-effects/2847121) |
| Dark Fantasy Studio: COMPLEX CINEMATIC HORROR | 52 Impacts und Steigerungen für Jumpscares | [Forum](https://forums.unrealengine.com/t/dark-fantasy-studio-1-complex-cinematic-horror-52-impact-sound-effects/2841867) |
| Horror Game Sound Basics / Jumpscare Pack X | Schritte, Objekte, Stinger | [Fab-Suche](https://www.fab.com/search?q=horror%20sound) |

Regen-Atmo, Bahnübergang, Automatensummen und Krähen klingen aufgenommen am besten. Diese Aufnahmen lassen sich auch selbst machen, das Handy reicht für Ambiences.

### 3.6 Druckermodell in maximalem Detail

Nanite verarbeitet auch sehr dichte Modelle. Deshalb ist ein detailliertes, nicht spielfertiges Modell hier die bessere Wahl als ein Low-Poly-Drucker.

| Modell | Eigenschaften | Link |
|---|---|---|
| CGTrader „3d printer model design“ | Rahmen, Bett, Extruder, Schienen, Bedienfeld, Motoren, Kabelwege | [CGTrader](https://www.cgtrader.com/3d-models/vehicle/military-vehicle/3d-printer-model-design) |
| Superhive „Fully Animated FDM 3D Printer“ | Bereits animiert, gut als Referenz für Achsen | [Superhive](https://superhivemarket.com/products/fully-animated-fdm-3d-printer) |
| RenderHub „FDM 3D Printer“ | In Unreal getestet, LODs, animiert | [RenderHub](https://www.renderhub.com/zarrarabbas73/fdm-3d-printer) |

Für die Simulation muss das Modell in Einzelteile zerlegt sein: Rahmen, Druckbett, Hotend, Motoren, Mainboard, Netzteil. Das Hotend wird an `PrintheadAnchor` gehängt.

## 4 Werkzeuge für Figuren aus Fotos

| Werkzeug | Rolle | Kosten |
|---|---|---|
| MetaHuman (in UE 5.8 integriert) | Zielformat für beide Figuren, Rig, Gesicht, Haare | kostenlos |
| KeenTools KeenFace | Fotos zu Kopf in MetaHuman-Topologie, seit Oktober 2026 | Preis beim Anbieter prüfen |
| KeenTools FaceBuilder für Blender | Bewährter Weg mit vier bis acht Fotos | Preis beim Anbieter prüfen |
| Reallusion Headshot 3 (für Character Creator 5) | Foto zu Kopf mit KI-Aufbereitung, Export nach Unreal | rund 199 USD plus CC5 |
| Meshy oder Tripo | Bild zu 3D für Körper, Kleidung, Schirm | Abo |
| MetaHuman Animator + Markerless Mocap (Fab, experimentell) | Gesicht und Körper aus Webcam-Video | kostenlos |

Die Abläufe stehen in `03_Figuren_aus_Fotos.md`.

## 5 Kostenrahmen

Mit Wetter, Wald-Umgebung, Waffenpaket, einem Animationspaket, zwei Soundpaketen und dem Druckermodell liegst du grob zwischen 400 und 700 USD. Ohne Ultra Dynamic Sky (Regen mit Niagara selbst bauen) und mit nur einem Soundpaket geht es auch deutlich günstiger. Alle Zahlen sind Schätzungen aus Suchergebnissen und keine geprüften Preise.
