# KEMURI – Figuren nach Fotos echter Personen

Beide Figuren entstehen als MetaHuman. MetaHuman ist ab UE 5.6 direkt im Editor eingebaut und liefert Rig, Gesichtsanimation und Haare in der Qualität, die mit Lumen und Nanite zusammenpasst. Für die Ähnlichkeit mit der realen Person wird zuerst ein Kopf aus Fotos erzeugt und dann in MetaHuman eingepasst.

## 1 Welcher Weg für wen

| Weg | Ähnlichkeit | Aufwand | Wann |
|---|---|---|---|
| KeenFace (KeenTools, seit Oktober 2026) | hoch | gering | Standardweg: Fotos hochladen, Kopf in nativer MetaHuman-Topologie zurückbekommen |
| FaceBuilder für Blender | hoch | mittel | Wenn Fotos lokal bleiben sollen oder KeenFace nicht passt |
| Headshot 3 in Character Creator 5 | hoch, KI-gestützt | mittel | Wenn du ohnehin CC5 nutzt; Ergebnis ist ein CC-Charakter statt MetaHuman |
| Photogrammetrie mit RealityScan | sehr hoch | hoch | Wenn die Person für 40 bis 80 Fotos rundherum stillhält |
| Meshy oder Tripo | mittel | gering | Für Körper, Kleidung und Schirm, nicht für das Gesicht |

Für KEMURI empfehle ich: KeenFace oder FaceBuilder für beide Köpfe, MetaHuman Creator für Körper und Kleidung, Markerless Mocap für Kurodas Bewegungen.

## 2 Fotos richtig aufnehmen

Gleichmäßiges, weiches Licht ohne harte Schatten, am besten bewölkter Himmel oder ein Fenster. Neutraler Gesichtsausdruck, Mund geschlossen, keine Brille, Haare aus Stirn und Ohren. Kamera auf Augenhöhe, mindestens 12 Megapixel, kein Weitwinkel (50 bis 85 mm Kleinbild-Äquivalent). Fünf bis acht Ansichten: frontal, je 45 Grad links und rechts, beide Profile, leicht von oben und leicht von unten. Zusätzlich ein Ganzkörperfoto in A-Pose für Proportionen und Kleidung.

## 3 Ablauf Kuroda (Informant)

### 3.1 Kopf

KeenFace: Fotos hochladen, Kopf und Texturen in MetaHuman-Topologie herunterladen. In UE 5.8 im MetaHuman Creator ein neues MetaHuman anlegen und den Weg „Conform from Template“ bzw. „From Custom Mesh“ wählen. Danach Basisfarb- und Normal-Map aus KeenFace zuweisen.

FaceBuilder: In Blender den Kopf aus den Fotos aufbauen, die MetaHuman-kompatible Textur erzeugen, als FBX exportieren und in Unreal über „From Custom Mesh“ mit Auto Solve einpassen.

### 3.2 Körper, Kleidung, Rauchen

Im MetaHuman Creator Körpertyp, Größe und Kleidung wählen: Arbeitsjacke, Kapuzenpulli oder Hemd mit offenem Kragen. Kleidung, die MetaHuman nicht bietet, über Meshy aus dem Ganzkörperfoto erzeugen und an den Körper anpassen. Die Zigarette ist ein kleines Static Mesh am Hand-Socket, die Glut ein emissives Material, der Rauch ein Niagara-System mit Wind aus der Regenrichtung.

### 3.3 Bewegung und Stimme

Die reale Person spielt vor einer Webcam: rauchen, aufs Handy schauen, lachen, high in die Luft starren, nach der Zigarette fragen. Das kostenlose Markerless-Motion-Capture-Plugin von Epic (in UE 5.8 experimentell, über Fab) erzeugt daraus Körperanimation. MetaHuman Animator macht aus demselben Video die Gesichtsanimation, die Videoquelle läuft über Live Link Hub. Forenberichte erwähnen Fehler bei manchen MP4-Dateien in 5.8.0 bis 5.8.2. Falls das Einlesen scheitert, das Video mit konstanter Bildrate neu exportieren.

Die Stimme nimmt die Person selbst auf, auf Japanisch oder Deutsch. Spricht sie kein Japanisch, kann sie deutsch sprechen, und die Brille „übersetzt“ trotzdem; das bleibt glaubwürdig, weil die Untertitel ohnehin auf Deutsch erscheinen.

### 3.4 In das Plugin einhängen

Den fertigen MetaHuman-Blueprint öffnen, unter Class Settings als Parent Class `KemuriInformant` wählen und die Komponente `InteractVolume` über die Sitzfigur legen. Hinweise trägst du im Details-Panel unter `Hints` ein; die Texte stehen in `01_Spielkonzept.md`, Abschnitt 4.1.

## 4 Ablauf Stalker

### 4.1 Grundlage

Den Kopf genauso wie bei Kuroda aus den Fotos der zweiten Person erzeugen. Die Verfremdung beginnt erst danach, damit die Person erkennbar bleibt; genau das macht den Horror aus.

### 4.2 Verfremdung

Im MetaHuman Creator die Körpergröße auf das Maximum stellen, Arme und Hals verlängern, die Schultern schmal und abfallend halten. Mit der MetaHuman-Bearbeitung in Blender oder im Creator den Kiefer leicht verlängern, die Augen eine Spur zu weit auseinander setzen und die Ohren anlegen. Haare: lang, nass, strähnig, über das Gesicht fallend. Haut: ein eigenes Material mit fahler, leicht bläulicher Farbe, sichtbaren Adern an den Schläfen, Nässe-Glanz über einen Wetness-Parameter und Wassertropfen als Niagara am ganzen Körper.

Die Augen bekommen im IR-Modus ein starkes Retroreflex-Glühen. Dazu im Augenmaterial einen Emissive-Anteil hinzufügen, der nur bei gesetztem Custom-Stencil-Wert 3 aufleuchtet, und den Stalker-Mesh im IR-Modus auf Stencil 3 setzen (Blueprint-Event `OnCameraModeChanged` der Brille).

### 4.3 Bewegung

Animationen aus einem Zombie- oder Horror-Mocap-Paket (siehe `02_Einkaufsliste.md`) über den IK-Retargeter auf das MetaHuman-Skelett übertragen. Gehen: langsam, mit leicht verzögerten Gelenken. Verfolgung: viel schneller, als der Körper es zulassen sollte. Betäubung: auf die Knie fallen und den Kopf zu weit nach hinten neigen. Zugriff: eine Greif-Montage mit Kamerafahrt als Level Sequence, gestartet über das Blueprint-Event `OnCatch`.

### 4.4 In das Plugin einhängen

Blueprint-Kind `BP_Stalker` von `KemuriStalker` anlegen. Bei einem MetaHuman ist es am einfachsten, die Körper- und Gesichts-Meshes als Komponenten in `BP_Stalker` zu übernehmen (Body an das Character-Mesh, Face per Leader Pose Component). Die Kollision des Body-Mesh muss den Kanal Visibility blocken, und das Physik-Asset braucht einen Kopf-Body namens `head`, damit Kopftreffer erkannt werden.

## 5 Nebelgestalten

Die Nebelgestalten brauchen keine Ähnlichkeit. Ein MetaHuman ohne Gesichtsdetails mit einem Material, das die Silhouette über Fresnel betont und nach innen in Nebel übergeht, reicht aus. Dazu ein Schirm in der Hand und Mocap-Gang. Als Performance-Spar-Variante bietet sich ein Skeletal Mesh mit niedriger Polygonzahl an, weil davon bis zu zwölf gleichzeitig unterwegs sind.

## 6 Einwilligung und Daten

Du hast die Einwilligung der beiden Personen; halte sie schriftlich fest. Die Einwilligung sollte ausdrücklich abdecken, dass die Person als Informant bzw. als Monster dargestellt wird, ob das Spiel veröffentlicht oder nur privat gespielt wird, und dass die Fotos an Cloud-Dienste wie KeenFace oder Meshy gehen. Für Bildnisse gilt bei einer Veröffentlichung § 22 KUG (Recht am eigenen Bild). Gesichtsfotos sind personenbezogene Daten; bei Cloud-Diensten deshalb deren Speicher- und Trainingsbedingungen prüfen oder den lokalen Weg über FaceBuilder bzw. RealityScan wählen.
