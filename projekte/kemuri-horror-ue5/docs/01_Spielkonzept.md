# KEMURI 煙 – Spielkonzept

Arbeitstitel **KEMURI** (japanisch „Rauch“). First-Person-Survival-Horror mit Escape-Struktur, Spielzeit etwa 60 bis 90 Minuten.

Ein Mann sucht im verregneten japanischen Viertel nur einen Ort zum Rauchen. Er findet einen Raucher, der alles weiß und für Tabak redet. Er findet einen Nebel, der das Viertel Straße für Straße verschluckt. Und er findet etwas, das ihn durch die Gassen jagt und nur mit einer Waffe stirbt, die er selbst drucken muss.

## 1 Leitlinien

### 1.1 Kein HUD

Es gibt keine Lebensleiste, keinen Munitionszähler, keine Minikarte und keinen Timer. Alles, was der Spieler wissen muss, steckt in der Welt: Atem und Herzschlag zeigen Ausdauer und Angst, das Magazin ist am Waffenmodell sichtbar, und die AI-Brille blendet Text nur dann ein, wenn eine echte Brille das auch täte (Übersetzung, Kamerabild, App-Menü).

### 1.2 Sehen kostet

Licht verrät dich. Die Taschenlampe lockt den Stalker an, die IR-Kamera der Brille dagegen nicht, aber sie leert die Powerbank. Dunkelheit ist also sicher und gleichzeitig blind. Diese Abwägung ist der Kern jeder Minute.

### 1.3 Der Nebel ist die Uhr

Statt eines Countdowns rückt der Nebel vor. Ein sicherer Kreis um den Raucherbereich schrumpft über die Spielzeit, Passanten verschwinden, und in den Schwaden entstehen Nebelgestalten. Wer trödelt, sieht es an der Straße, nicht auf einer Anzeige.

### 1.4 Wissen gegen Rauch

Informationen kosten Zigaretten oder Weed. Eine Zigarette bringt einen vagen Hinweis, Weed macht den Informanten für einige Minuten high und gesprächig, und er nennt dann genaue Orte, die als IR-Markierung in der Welt erscheinen. Rauchwaren sind knapp und konkurrieren mit dem Speichern.

### 1.5 Bauen statt finden

Normale Waffen verletzen den Stalker nur und betäuben ihn kurz. Töten kann ihn ausschließlich die gedruckte Waffe HARAI-01. Das Spiel ist deshalb eine Sammel- und Bauaufgabe unter Druck.

## 2 Figuren

### 2.1 Der Spieler

Ein namenloser Mann, der mit dem letzten Zug in das Viertel kommt. Seine Zigaretten sind im Regen durchgeweicht, und genau deshalb sucht er den Raucherbereich am Bahnhof. Er trägt eine AI-Brille mit Kamera, Live-Übersetzung und kleinem Display im Brillenglas.

### 2.2 Kuroda, der Informant

Kuroda ist Arbeitstitel; die Figur wird nach dem Foto der realen Person modelliert, die eingewilligt hat. Er sitzt im gläsernen Raucherhäuschen (Kitsuenjo) neben dem Bahnhof und verlässt es nie. Er kennt jede Gasse, raucht Kette und redet nur, wenn er etwas zu rauchen bekommt. Nüchtern ist er wortkarg und spricht in Andeutungen. High wird er redselig, genau und ein wenig unheimlich, als wüsste er zu viel.

Seine Regel ist auch die Lore des Spiels: „Der Rauch hält es fern.“ Das Ding im Nebel meidet echten Rauch. Deshalb ist der Raucherbereich der einzige sichere Ort, und deshalb heißt der Nebel bei ihm „falscher Rauch“.

Er spricht Japanisch. Die Brille übersetzt live, und die Untertitel erscheinen im Brillendisplay.

### 2.3 Der Stalker: Ame-Otoko bzw. Ame-Onna

Im Japanischen nennt man einen Menschen, der immer den Regen mitbringt, Ame-Otoko (Regenmann) oder Ame-Onna (Regenfrau). Der Stalker wird nach dem Foto der zweiten realen Person modelliert und dann verfremdet. Es ist jemand aus dem Viertel, der im Nebel des letzten Jahres verschwand und zurückkam.

Die Figur ist rund zwanzig Zentimeter zu groß, hält den Kopf schief und trieft ununterbrochen, auch unter Dächern. Sie trägt einen zerbrochenen durchsichtigen Schirm, dessen Streben wie Finger abstehen. In der IR-Kamera reflektieren ihre Augen grellweiß. Ihre Ankündigungen sind flackernde Laternen, Tropfgeräusche ohne Regen und Rauschen im Brillenton.

### 2.4 Nebelgestalten (Kiri-bito)

Das sind die Menschen, die im Nebel verschwunden sind: gesichtslose Silhouetten mit Schirmen, langsam und schwach, aber zahlreich. Sie sterben durch Kugeln, zerfallen zu Dunst und liefern die Währung Mon. Sie sind der Grund, warum das Waffensystem im Stil des Zombie-Modus funktioniert.

### 2.5 Passanten und Radfahrer

Die NPCs der Map mit Schirmen und Fahrrädern bewegen sich zu Beginn ganz normal. Mit dem Nebel verschwinden sie einer nach dem anderen, sobald niemand hinsieht. Zurück bleiben ihre Schirme auf dem nassen Asphalt und Fahrräder, deren Rad sich noch dreht.

## 3 Ablauf

Die Flags in der rechten Spalte sind die Namen, die der Code und die Hinweise des Informanten verwenden.

| Abschnitt | Ziel | Wichtige Flags |
|---|---|---|
| Prolog | Raucherbereich finden, Kuroda treffen | `Nebel_Start` |
| Akt 1 | Werkstatt finden, Baupläne und Elektronik sammeln | `Werkstatt_Gefunden`, `Bauplan_1..3` |
| Akt 2 | Mit dem Scooter in den Wald, Reliquien und Filament holen | `Wald_Offen`, `Reliquie_*`, `Filament_Gefunden` |
| Akt 3 | Drucker aufbauen, kalibrieren, drucken, Stalker töten | `Druck_Gestartet`, `Druck_Fertig`, `Stalker_Besiegt` |

### 3.1 Prolog: Ein Ort zum Rauchen (5 bis 8 Minuten)

Es ist kurz nach Mitternacht, Starkregen, das Viertel wirkt normal. Passanten mit Schirmen gehen vorbei, ein Fahrrad klingelt, die Automaten leuchten. Der Spieler folgt dem Raucherzeichen zum Kitsuenjo. Kuroda bittet um eine Zigarette, doch die eigene Schachtel ist durchweicht.

Die erste Aufgabe lehrt alles Grundlegende. Der Zigarettenautomat verlangt eine Alterskarte, die in einem liegengelassenen Portemonnaie im Fahrradkorb vor dem Konbini steckt. Über die Brille lernt der Spieler die Übersetzung von Schildern kennen. Mit der ersten Zigarette beginnt das Gespräch, und das Flag `Nebel_Start` startet die Uhr. Beim Rückweg zeigt ein Blitz am Ende der Straße eine zu große Gestalt mit Schirm. Beim nächsten Blitz ist sie weg.

### 3.2 Akt 1: Das Viertel (20 bis 25 Minuten)

Kuroda erzählt vom Nebel, der jedes Jahr kommt, und von einem Mann, der in seiner Werkstatt „Dinge gedruckt hat, die Kami vertreiben“. Die Werkstatt ist ein geschlossener Elektronikladen mit Maker-Ecke. Im Normalbild ist der Hinterraum schwarz und leer. Erst die IR-Kamera zeigt den Drucker-Rahmen, Schriftzeichen an der Wand und die Spur nasser Fußabdrücke.

Der Spieler sammelt im Viertel: drei Bauplan-Fragmente, das Mainboard, das Netzteil, das Hotend, die Schrittmotoren und das Druckbett. Gleichzeitig lernt er das Waffensystem kennen: eine Startpistole, Kreideumrisse von Waffen an Wänden (Wandkauf), den Omikuji-Schrein (Zufallswaffe) und Getränkeautomaten mit dauerhaften Effekten. Der Stalker streift ab jetzt durch das Viertel.

Ein Stromkasten versorgt Werkstatt und Automaten. Fällt er aus, flackern die Lichter und die Automaten verstummen; die Reparatur ist eine wiederkehrende Aufgabe unter Druck.

### 3.3 Akt 2: Der Wald (20 bis 30 Minuten)

Kuroda, high, verrät, dass gewöhnliches Filament die Waffe nicht trägt. Gebraucht wird ein Strang aus dem heiligen Seil (Shimenawa) des inneren Schreins im Wald. Am Bahnhof steht ein E-Scooter an einer Ladesäule. Der Spieler fährt über eine einsame Landstraße in die Hügel.

Das Umland ist weit, still und melancholisch: nebelverhangene Hänge, umgestürzte Strommasten, Moos auf allem, ein Fluss, Krähen und das ferne Klingeln eines Bahnübergangs ohne Zug. Die Landschaft lässt Raum und Einsamkeit wirken, bevor der nächste Schrecken kommt.

Im Wald liegen vier kleine Schreine (Hokora) und der innere Schrein (Okumiya). Jeder Hokora birgt eine Reliquie, die eine Munitionsmodifikation freischaltet. Jede Reliquie verlangt eine eigene Hürde:

| Hokora | Reliquie | Hürde |
|---|---|---|
| Donnerschrein | Raijin (Kettenblitz) | Drei Glocken in der Reihenfolge läuten, die nur im IR sichtbar an den Bäumen steht |
| Fuchsschrein | Kitsunebi (Fuchsfeuer) | Sieben Steinlaternen entzünden; jede drückt den Nebel ein Stück zurück |
| Schneeschrein | Yuki (Frost) | Einen Bachlauf entlang im Dunkeln gehen, nur mit IR, ohne Licht |
| Salzschrein | Shio (Reinigungssalz) | Eine Zigarette als Opfergabe hinlegen und das Gebet bis zum Ende abwarten, während etwas näher kommt |

Am Okumiya steht der Weihe-Schrein, an dem Waffen geweiht und Munition gesegnet werden. Dort hängt auch das Shimenawa, aus dem das Filament gewonnen wird. Beim Abschneiden reißt der Nebel auf, und der Stalker steht zwischen dem Spieler und dem Scooter.

### 3.4 Akt 3: Der Druck (10 bis 15 Minuten)

Zurück im Viertel ist fast alles Nebel. Kuroda lässt den Drucker in den Raucherbereich tragen, weil nur dort noch Rauch hängt. Der Spieler setzt die Bauteile ein, kalibriert das Bett und lädt das Filament. Mit dem Start des Drucks endet der Schutz des Raucherbereichs (`Druck_Gestartet`). Drei Minuten lang wächst die Waffe Schicht für Schicht, während Nebelgestalten angreifen, der Stalker zum Drucker gelockt wird und der Strom ausfallen kann.

Ist HARAI-01 fertig, beginnt das Finale. Shio-Munition betäubt den Stalker, und drei Treffer mit HARAI töten ihn.

### 3.5 Enden

Im Standardende lichtet sich der Nebel im Morgengrauen. Kuroda zündet sich eine letzte Zigarette an und sagt: „Nächstes Jahr bringst du deine eigenen mit.“

Das versteckte Ende erreicht, wer Kuroda alle vier Portionen Weed gegeben hat. Im letzten Rausch erzählt er, dass er selbst letztes Jahr im Nebel war und nur durch den Rauch zurückfand. In der Schlussszene steht er im Morgenlicht, aber in der IR-Kamera reflektieren seine Augen grellweiß.

## 4 Systeme

### 4.1 Informant und Rauch-Ökonomie

Zigaretten finden sich in Automaten (Alterskarte nötig), in Konbini-Regalen und bei verschwundenen Passanten. Weed gibt es genau viermal im Spiel. Es liegt in Verstecken, die nur in der IR-Kamera sichtbar sind, weil Kurodas Dealer sie mit IR-Farbe markiert hat.

Kuroda gibt immer den Hinweis zur ersten offenen Aufgabe. Ein vager Hinweis setzt `FlagOnVague`. Ein präziser Hinweis setzt zusätzlich `FlagOnPrecise`, und dieses Flag schaltet eine IR-Markierung am Zielort frei (Komponente `KemuriIRReveal` mit `RequiredFlag`).

| Aufgabe | Vage (Zigarette) | Präzise (high) |
|---|---|---|
| Werkstatt | „Der alte Mann mit den Maschinen. Seine Tür ist da, wo die Bahn quietscht.“ | „Elektroladen hinter dem Bahnübergang, Hintertür. Der Raum ist nur dunkel, nicht leer. Sieh mit deiner Kamera hin.“ |
| Bauplan 1 | „Papier wird nass. Er hat seine Pläne dort versteckt, wo es trocken bleibt und Geld liegt.“ | „Im Konbini, in der Kasse unter der Schublade. Der Kassierer ist weg, die Kasse nicht.“ |
| Bauplan 2 | „Ganz oben, wo die Wäsche hängt.“ | „Dach des Wohnhauses mit der roten Feuertreppe. Unter dem Wassertank.“ |
| Bauplan 3 | „Bei den Toten liegt Papier immer gut.“ | „Friedhof hinter dem Tempel, drittes Grab links, in der Vase für Räucherstäbchen.“ |
| Elektronik | „Was er brauchte, hat er sich zusammengeklaut. Fahrradladen, Spielhalle, Bahnhof.“ | „Mainboard im Spielautomaten der Spielhalle, Netzteil im Stellwerk am Bahnhof, Hotend und Motoren beim Fahrradhändler.“ |
| Wald | „Gewöhnliches Plastik trägt das nicht. Es braucht etwas, das gebunden ist.“ | „Das Seil am inneren Schrein im Wald. Nimm den Roller am Bahnhof. Fahr nicht mit Licht, wenn du es nicht brauchst.“ |
| Reliquien | „Vier kleine Schreine, vier Wetter.“ | „Donner hört man, Feuer zündet man, Schnee sieht man nicht, Salz gibt man. Die Reihenfolge ist egal, die Geduld nicht.“ |
| Finale | „Wenn es druckt, bin ich nicht mehr sicher. Du auch nicht.“ | „Drei Treffer, wenn es kniet. Vorher bringt dir das Ding gar nichts. Salz zwingt es auf die Knie.“ |

Speichern funktioniert wie ein Farbband im Survival-Horror: Im Raucherbereich eine Zigarette rauchen speichert das Spiel. Jede Zigarette ist damit entweder ein Hinweis oder ein Speicherstand.

### 4.2 Die Brille

Die Brille ist das einzige Interface. T kurz tippen schaltet den Kameramodus (IR) ein und aus. T halten öffnet das App-Menü; die Maus wählt, und Loslassen startet die App. Das Spiel pausiert dabei nicht.

| App | Funktion |
|---|---|
| Kamera | IR-Nachtsicht mit unsichtbarem Strahler. Zeigt IR-Inhalte, verrät dich nicht, kostet Akku. Bildschirm mit REC-Punkt, Zeitstempel und Rauschen. |
| Karte | Nur besuchte Orte und präzise Hinweise von Kuroda, einige Sekunden lang eingeblendet. |
| Hinweise | Protokoll aller Aussagen Kurodas in Übersetzung. |
| Bauteile | Checkliste der Druckerteile ohne Fundorte. |
| Resonanz | Zwanzig Sekunden lang Pieptöne, die zur nächsten Reliquie oder zum nächsten Bauteil schneller werden. Nur Ton, keine Anzeige. |
| Akku | Zeigt den Ladestand der Powerbank, die Brille und Taschenlampe gemeinsam speist. |

Die IR-Kamera hat feste Spielfunktionen. Sie macht Räume sichtbar, die im Normalbild nur schwarz sind, sie zeigt Schriftzeichen, Weed-Verstecke und die nassen Spuren des Stalkers, und sie lässt Trugbilder verschwinden, die nur ohne IR existieren. Unterhalb von zwölf Prozent Akku zeigt das Display Bildfehler und fällt sporadisch aus.

### 4.3 Nebel als Uhr

Der Nebel braucht etwa dreißig Minuten, bis er das Viertel ganz eingenommen hat. Fünf Stufen markieren den Verlauf:

| Stufe | Welt |
|---|---|
| 0 | Leichter Dunst, Passanten unterwegs, Automaten leuchten |
| 1 | Nebelbänke am Ortsrand, erste Passanten verschwinden, vereinzelt Nebelgestalten |
| 2 | Hauptstraße im Nebel, Radfahrer fehlen, Schirme liegen herum, der Stalker wird schneller |
| 3 | Nur noch Bahnhofsumfeld klar, Gestalten in Gruppen |
| 4 | Sicht unter zwanzig Metern außer am Raucherbereich |
| 5 | Nur der Raucherbereich ist frei |

Entzündete Steinlaternen am Fuchsschrein drücken den Nebel zurück (`PushBack`). Das ist die einzige Möglichkeit, Zeit zu kaufen.

### 4.4 Der Stalker

Der Stalker streift an einer unsichtbaren Leine durch das Viertel: Ohne Kontakt zieht sich sein Suchradius um den Spieler langsam zusammen, sodass er immer wieder nah kommt, ohne zu schummeln. Er sieht dich im Licht und in einem Sichtkegel von etwa 130 Grad. Er hört Schüsse, Sprinten, den Scooter und die Taschenlampe.

Gegenmittel sind Dunkelheit, die IR-Kamera, Schleichen, der Raucherbereich, Shio-Munition und das Getränk Kaze-Soda. Genug Schaden betäubt ihn für acht Sekunden; jede Betäubung macht die nächste schwerer. Erreicht er dich, ist das ein Jumpscare und der Tod. Nur Mamori-Mizu rettet einmal.

### 4.5 Waffen im Stil des Zombie-Modus

Die Währung heißt Mon. Ein Treffer bringt 10 Mon, ein Kill 60 Mon und ein Kopfschuss-Kill 40 Mon zusätzlich; ein Treffer am Stalker bringt 5 Mon. Zu Beginn hat der Spieler 500 Mon.

| Wandkauf | Preis | Munition | Geweihte Version |
|---|---|---|---|
| Dienstrevolver | 500 | 250 | Kami-no-Koe |
| Pump-Schrotflinte | 1.000 | 500 | Raiden-Hō |
| Maschinenpistole | 1.200 | 600 | Hachi-no-Su |
| Jagdgewehr | 1.500 | 750 | Tsukuyomi |

Der Omikuji-Schrein kostet 950 Mon und enthält zusätzlich Sonderwaffen: eine Nagelpistole aus dem Baumarkt, eine Signalpistole, eine Harpune vom Fischmarkt und eine Armbrust. Nach vier Zügen kann ein Kyō (schlechtes Los) erscheinen; dann gibt es das Geld zurück, und der Schrein wandert an einen anderen Ort.

Der Weihe-Schrein im Wald weiht eine Waffe für 5.000 Mon. Für 2.500 Mon segnet er die Munition einer geweihten Waffe mit einer freigeschalteten Modifikation; jeder weitere Kauf schaltet zur nächsten:

| Modifikation | Reliquie | Wirkung auf Gestalten | Wirkung auf den Stalker |
|---|---|---|---|
| Raijin | Donnerschrein | Kettenblitz auf bis zu fünf Gegner | Zusätzliche Betäubung |
| Kitsunebi | Fuchsschrein | Blaues Feuer, Schaden über Zeit | Brennt, aber ohne tödliche Wirkung |
| Yuki | Schneeschrein | Starke Verlangsamung | Verlangsamung um 40 Prozent |
| Shio | Salzschrein | Sofortige Auflösung | Dreifache Betäubung, Pflicht im Finale |

Getränkeautomaten verkaufen dauerhafte Effekte:

| Getränk | Preis | Effekt |
|---|---|---|
| Tetsu-Cha (Eisentee) | 2.500 | 250 statt 100 Lebenspunkte |
| Hayai-Kōhī (schneller Kaffee) | 3.000 | Doppelt so schnelles Nachladen |
| Mittsu-Ramune | 4.000 | Dritter Waffenplatz |
| Mamori-Mizu (Schutzwasser) | 1.500 | Einmalige Selbstwiederbelebung |
| Kaze-Soda | 2.000 | Schnellerer Sprint, Schritte 60 Prozent leiser |

### 4.6 Der 3D-Drucker

Der Drucker ist eine Simulation mit möglichst vielen echten Handgriffen. Der Spieler setzt Rahmen, Druckbett, Motoren, Hotend, Mainboard und Netzteil ein; jedes Teil rastet sichtbar und hörbar ein. Beim Kalibrieren dreht er an vier Ecken die Rändelschrauben, bis ein Blatt Papier zwischen Düse und Bett gerade noch kratzt. Danach heizt er die Düse auf, schiebt das Shimenawa-Filament ein und sieht zu, wie die erste Schmelze austritt. Auf der Brille wählt er die Datei `HARAI-01.gcode` aus Kurodas Bauplänen.

Während des Drucks fährt der Druckkopf prozedural im Zickzack-Infill, Schicht für Schicht, mit wechselnder Richtung pro Schicht. Das Teil wächst über einen Materialparameter von unten nach oben, und der Schrittmotor-Ton ändert sich an jedem Bahnende. Optional gilt: Wer schlecht kalibriert, riskiert Spaghetti, also einen Fehldruck, der zum Neustart zwingt.

HARAI-01 ist eine gedruckte Spulenwaffe, die mit Salz und Papier-Ofuda gefüllte Flechetten verschießt. Das Magazin fasst drei Schuss, die Reserve sechs.

### 4.7 E-Scooter

Der Scooter steht an einer Ladesäule am Bahnhof. Er fährt bis etwa 32 km/h, wird bergauf langsamer, legt sich in Kurven und verliert Akku proportional zum Gas. Sein Motor ist hörbar und lockt den Stalker an, und der Scheinwerfer ist im Wald überlebenswichtig, aber auffällig. Ein harter Aufprall löst einen Sturz aus, und der Stalker kann den Spieler vom Scooter reißen.

## 5 Horror-Regie

### 5.1 Grundsätze

Angst braucht Täler. Ein großer Schreckmoment kommt höchstens alle fünf bis sieben Minuten, dazwischen nur Unbehagen. Die Komponente `KemuriScareTrigger` löst nicht aus, wenn die Angst schon über 0,7 liegt. Vor jedem Schrecken wird es stiller: Regen und Musik fallen ab, und nur noch der Atem ist zu hören. Kein Schrecken wiederholt sich in derselben Form. Die stärksten Momente nutzen die IR-Kamera, weil der Spieler sie freiwillig einschaltet.

### 5.2 Schreckmomente

| Nr. | Ort | Auslöser | Was passiert |
|---|---|---|---|
| 1 | Hauptstraße | Blitz im Prolog | Silhouette mit Schirm am Straßenende, beim nächsten Blitz fort |
| 2 | Konbini | Spieler schaut zur Kühltheke | Alle Neonröhren erlöschen nacheinander auf ihn zu, dann wieder an, und ein Passant steht zu nah |
| 3 | Werkstatt | Erstes Einschalten der IR-Kamera | Im Bild steht jemand direkt neben dem Drucker; im Normalbild ist dort niemand |
| 4 | Wohnhausdach | Bauplan 2 aufheben | Wäsche hängt reglos im Sturm, dann bewegt sich ein Laken in die falsche Richtung |
| 5 | Friedhof | Nur im IR | Alle Grabsteine haben Augenreflexe |
| 6 | Bahnübergang | Spieler wartet | Die Schranke senkt sich, ein Zug rauscht vorbei, und dahinter steht der Stalker, viel näher als zuvor |
| 7 | Spielhalle | Mainboard aus dem Automaten ziehen | Alle Automaten starten gleichzeitig mit einer Melodie, die ein Kinderlied rückwärts spielt |
| 8 | Landstraße | Scooter-Akku unter 20 Prozent | Der Scheinwerfer flackert und zeigt bei jedem Aufblitzen eine Gestalt näher am Straßenrand |
| 9 | Schneeschrein | Bachlauf im Dunkeln | Etwas folgt im Wasser im gleichen Schritt und bleibt stehen, wenn du stehen bleibst |
| 10 | Okumiya | Seil abschneiden | Der Nebel reißt auf, alle Laternen gehen aus, und Atem ist direkt hinter der Kamera zu hören |
| 11 | Raucherbereich | Druckstart | Kuroda ist kurz nicht da, nur seine Zigarette glimmt auf der Bank |
| 12 | Finale | Erster HARAI-Treffer | Der Stalker schreit mit der Stimme der Person aus dem Foto |

### 5.3 Klang

Regen ist die Grundschicht und klingt je nach Oberfläche unterschiedlich: auf Schirmen, Blech, Laub und Asphalt. Nahe dem Stalker hört der Regen auf, aber Tropfen fallen weiter. Der Herzschlag folgt der Angst, der Atem der Ausdauer. Die Brille rauscht leise, wenn der Stalker in der Nähe ist. Musik gibt es nur im Raucherbereich: ein leises Radio mit altem City-Pop, das zum Speicherraum-Gefühl gehört.

### 5.4 Barrierefreiheit

Vor dem Start erscheint eine Warnung vor Lichtblitzen. Im Optionsmenü regelt ein Regler für Lichtblitze von 0 bis 100 Prozent den Wert `PhotosensitivityScale`; bei 0 flackert und blitzt nichts mehr. Untertitel sind immer an, weil sie Teil der Brille sind.

## 6 Steuerung

| Taste | Zu Fuß | Scooter |
|---|---|---|
| WASD | Laufen | Gas, Bremse, Lenken |
| Maus | Umsehen | Kopf drehen |
| Shift | Sprinten | – |
| Strg | Schleichen | – |
| Leertaste | Springen | Vollbremsung |
| E | Benutzen, Kaufen, Aufsteigen | Absteigen |
| Linke Maustaste | Schießen | – |
| Rechte Maustaste | Zielen | – |
| R | Nachladen | – |
| Mausrad oder Q | Waffe wechseln | – |
| V | Nahkampf | – |
| F | Taschenlampe | Scheinwerfer |
| T tippen | IR-Kamera an/aus | – |
| T halten | Brillen-Apps | – |

## 7 Umfang und Reihenfolge

Zuerst entsteht ein spielbarer Ausschnitt aus Prolog und erster Hälfte von Akt 1: Raucherbereich, Kuroda mit zwei Hinweisen, Nebel, Stalker, ein Wandkauf, die IR-Werkstatt und zwei Bauteile. Erst wenn dieser Ausschnitt Angst macht, folgen der Wald, der Scooter und der Drucker. Jedes System ist im Plugin bereits vorhanden, sodass der Aufwand vor allem im Leveldesign, in den Figuren und im Klang liegt.
