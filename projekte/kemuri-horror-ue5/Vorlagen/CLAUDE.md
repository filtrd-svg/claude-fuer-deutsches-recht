# CLAUDE.md – KEMURI (Unreal-Engine-Projekt)

Diese Datei gehört in den Wurzelordner des Unreal-Projekts. Sie gilt für Claude Code, wenn es in diesem Projekt arbeitet, mit oder ohne Unreal-MCP-Verbindung zum Editor.

## Projekt

KEMURI ist ein First-Person-Survival-Horror-Spiel auf einer gekauften Japan-Map von Fab. Das Spielkonzept steht in `Docs/KEMURI/01_Spielkonzept.md`, die Technik in `Docs/KEMURI/04_Atmosphaere_Technik.md` und die Einrichtung in `Docs/KEMURI/05_Einrichtung_am_PC.md`. Alle Gameplay-Systeme liegen im C++-Plugin `Plugins/Kemuri`.

## Regeln

- Antworten und Kommentare auf Deutsch, Bezeichner im Code auf Englisch.
- Die Inhalte der Fab-Map und anderer gekaufter Pakete nie direkt verändern. Änderungen erfolgen über eigene Blueprints in `Content/Kemuri`, über Kopien oder über Level-Instanzen.
- Neue Gameplay-Logik zuerst im Plugin als C++ mit Blueprint-Events anlegen und erst das Feintuning in Blueprints. Keine Logik in Level Blueprints, außer für einmalige Skriptszenen.
- Es gibt kein HUD. Text erscheint nur über `UKemuriGlassesComponent` bzw. das Widget `WBP_KemuriGlasses`.
- Story-Zustand läuft ausschließlich über Flags im `UKemuriWorldSubsystem`. Neue Flags in `01_Spielkonzept.md`, Abschnitt 3, nachtragen.
- Item-IDs exakt wie in `KemuriTypes.h` (`KemuriItems`) und im Spielkonzept schreiben.
- Lichtblitze und Flackern immer über `PhotosensitivityScale` dämpfbar halten.

## Arbeitsweise mit dem Editor (MCP)

- Vor jeder größeren Änderung alle Assets speichern und einen Git-Commit anlegen lassen.
- Pro Auftrag eine überschaubare Änderung, danach im Spiel testen (PIE) und das Ergebnis kurz berichten.
- Keine Assets löschen, ohne vorher die Referenzen zu prüfen und nachzufragen.
- Nach C++-Änderungen den Editor schließen, in Visual Studio bauen und neu starten, oder Live Coding (Strg+Alt+F11) nur für Änderungen in .cpp-Dateien verwenden.

## Build

- Konfiguration: Development Editor, Win64.
- Bei Compilerfehlern zuerst die erste Fehlermeldung beheben; Folgefehler verschwinden meist mit.
