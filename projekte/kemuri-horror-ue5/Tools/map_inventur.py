"""
KEMURI – Map-Inventur fuer Unreal Engine 5 (Editor-Python).

Zweck: Herausfinden, was in der gekauften Japan-Map steckt (NPC-Modelle mit Schirm und Fahrrad,
Animationen, Automaten, Schreine, Licht, Nebel, Mass-/ZoneGraph-Crowd), damit die KEMURI-Systeme
passend angeschlossen werden koennen.

Ausfuehren:
  1. Plugin "Python Editor Script Plugin" aktivieren (Edit -> Plugins), Editor neu starten.
  2. Die Japan-Map oeffnen.
  3. Tools -> Execute Python Script... -> diese Datei waehlen.
     (alternativ im Output Log auf "Python" umschalten und: py "<Pfad>/map_inventur.py")
  4. Ergebnis: <Projekt>/Saved/KemuriInventur.md  -> Inhalt an Claude schicken.

Das Skript liest nur und veraendert nichts am Projekt.
"""

import collections
import os

import unreal

KEYWORDS = {
    "NPC / Figuren": ["npc", "crowd", "ped", "pedestrian", "human", "character", "people", "civilian", "man", "woman", "metahuman"],
    "Schirm": ["umbrella", "kasa", "parasol"],
    "Fahrrad / Roller": ["bike", "bicycle", "cycle", "jitensha", "scooter", "moped"],
    "Automaten / Konbini": ["vending", "jihanki", "konbini", "store", "shop", "cigarette", "tobacco", "smok", "kitsuen"],
    "Schrein / Tempel": ["shrine", "torii", "temple", "jinja", "hokora", "lantern", "toro", "shimenawa"],
    "Licht": ["light", "lamp", "neon", "sign"],
    "Wetter / Nebel": ["rain", "fog", "mist", "puddle", "wet", "weather", "cloud", "sky"],
    "Fahrzeuge": ["car", "truck", "taxi", "vehicle", "train"],
}

INTERESTING_ACTOR_CLASSES = [
    "ExponentialHeightFog", "LocalFogVolume", "SkyAtmosphere", "VolumetricCloud", "DirectionalLight",
    "SkyLight", "PostProcessVolume", "NavMeshBoundsVolume", "ZoneShape", "MassSpawner",
    "LevelSequenceActor", "NiagaraActor", "LandscapeProxy", "Landscape", "WorldPartitionHLOD",
]


def classify(name):
    lowered = name.lower()
    hits = []
    for group, words in KEYWORDS.items():
        if any(word in lowered for word in words):
            hits.append(group)
    return hits


def asset_class_name(asset_data):
    # UE 5.1+: asset_class_path, aeltere Versionen: asset_class
    path = getattr(asset_data, "asset_class_path", None)
    if path is not None:
        return str(path.asset_name)
    return str(getattr(asset_data, "asset_class", "Unknown"))


def main():
    lines = ["# KEMURI Map-Inventur", ""]

    # ---------- Level-Actoren ----------
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_subsystem.get_all_level_actors()
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    lines.append(f"Level: `{world.get_name() if world else '?'}`  ·  Actoren: {len(actors)}")
    lines.append("")

    class_counts = collections.Counter(actor.get_class().get_name() for actor in actors)
    lines.append("## 1 Actor-Klassen im Level (Top 60)")
    lines.append("")
    lines.append("| Klasse | Anzahl |")
    lines.append("|---|---|")
    for cls, count in class_counts.most_common(60):
        lines.append(f"| {cls} | {count} |")
    lines.append("")

    lines.append("## 2 Wichtige Engine-Actoren")
    lines.append("")
    for wanted in INTERESTING_ACTOR_CLASSES:
        found = [a for a in actors if a.get_class().get_name().startswith(wanted)]
        if found:
            names = ", ".join(a.get_actor_label() for a in found[:8])
            lines.append(f"- **{wanted}** ({len(found)}): {names}")
    lines.append("")

    grouped = collections.defaultdict(list)
    for actor in actors:
        label = actor.get_actor_label()
        cls = actor.get_class().get_name()
        for group in classify(label + " " + cls):
            grouped[group].append((label, cls, actor.get_actor_location()))

    lines.append("## 3 Actoren nach Thema (Name oder Klasse)")
    lines.append("")
    for group in KEYWORDS:
        entries = grouped.get(group, [])
        lines.append(f"### 3.{list(KEYWORDS).index(group) + 1} {group} ({len(entries)})")
        lines.append("")
        for label, cls, loc in entries[:40]:
            lines.append(f"- `{label}` · {cls} · ({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})")
        if len(entries) > 40:
            lines.append(f"- … und {len(entries) - 40} weitere")
        lines.append("")

    # Pawns/Characters mit Controller-Klasse (fuer autonome NPCs entscheidend)
    lines.append("## 4 Pawns und Characters")
    lines.append("")
    pawns = [a for a in actors if isinstance(a, unreal.Pawn)]
    for pawn in pawns[:60]:
        try:
            ai_class = pawn.get_editor_property("ai_controller_class")
            ai_name = ai_class.get_name() if ai_class else "-"
        except Exception:
            ai_name = "?"
        lines.append(f"- `{pawn.get_actor_label()}` · {pawn.get_class().get_name()} · AI-Controller: {ai_name}")
    if not pawns:
        lines.append("- Keine Pawns im Level. NPCs sind vermutlich Static/Skeletal-Mesh-Actoren oder Blueprints ohne Pawn-Basis.")
    lines.append("")

    # ---------- Assets im Projekt ----------
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = registry.get_assets_by_path("/Game", recursive=True)
    asset_counts = collections.Counter(asset_class_name(a) for a in assets)
    lines.append(f"## 5 Assets unter /Game ({len(assets)})")
    lines.append("")
    lines.append("| Typ | Anzahl |")
    lines.append("|---|---|")
    for cls, count in asset_counts.most_common(30):
        lines.append(f"| {cls} | {count} |")
    lines.append("")

    lines.append("## 6 Relevante Assets nach Thema")
    lines.append("")
    relevant_types = {"Blueprint", "SkeletalMesh", "StaticMesh", "AnimSequence", "AnimMontage", "AnimBlueprint",
                      "NiagaraSystem", "MaterialInstanceConstant", "LevelSequence", "SoundWave", "SoundCue", "MetaSoundSource"}
    by_group = collections.defaultdict(list)
    for asset in assets:
        cls = asset_class_name(asset)
        if cls not in relevant_types:
            continue
        name = str(asset.asset_name)
        for group in classify(name):
            by_group[group].append((cls, str(asset.package_name)))
    for index, group in enumerate(KEYWORDS, start=1):
        entries = by_group.get(group, [])
        lines.append(f"### 6.{index} {group} ({len(entries)})")
        lines.append("")
        for cls, package in sorted(entries)[:50]:
            lines.append(f"- {cls}: `{package}`")
        if len(entries) > 50:
            lines.append(f"- … und {len(entries) - 50} weitere")
        lines.append("")

    # Ordnerstruktur (erste Ebene unter /Game) zeigt meist den Paketnamen der Map
    folders = sorted({str(a.package_path).split("/")[2] for a in assets if str(a.package_path).count("/") >= 2})
    lines.append("## 7 Ordner direkt unter /Game")
    lines.append("")
    lines.append(", ".join(f"`{f}`" for f in folders))
    lines.append("")

    out_path = os.path.join(unreal.Paths.project_saved_dir(), "KemuriInventur.md")
    with open(out_path, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))
    unreal.log(f"KEMURI-Inventur geschrieben: {out_path}")


main()
