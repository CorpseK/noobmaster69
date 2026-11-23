"""Audit and conservatively fix texture paths in a Skyrim SE replacer mod.

Usage (Windows, Python 3.9+):
    python fix_nif_paths.py "D:\\MO2\\mods\\My Replacer" "D:\\MO2\\mods\\My Replacer - NIF fixes"

The mod folder is only read. Fixed NIFs are written to the output folder with the
same relative paths, so the output can be installed as a separate mod that loads
after the original (MO2: put it below). A report is written to <output>/report.txt.

Fixes applied automatically (only when the new target file exists in the mod):
  1. FaceGen tint slot pointing to another plugin / another NPC / a typo
     -> textures\\actors\\character\\facegendata\\facetint\\<plugin>\\<formid>.dds
  2. Paths with a "data\\" prefix, an absolute path, or no "textures\\" prefix -> normalised.
  3. Texture that does not exist in the mod, while a file with the same name exists
     in the texture folder that mirrors the mesh folder (meshes\\A\\B\\x.nif <-> textures\\A\\B\\),
     or exactly one same-named file exists in the mod and the path is not a known hair-pack path.
     Vanilla paths are never redirected (they exist in the game BSAs).
Everything else is only reported.
"""

import collections
import ntpath
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from esp import load as load_esp, subrecords  # noqa: E402
from niftex import Nif, TEXTURE_SLOTS  # noqa: E402

TINT_SLOT = TEXTURE_SLOTS.index("inner/multilayer")
VANILLA_ACTOR_DIRS = {
    "female", "male", "eyes", "mouth", "femalebrows", "malebrows", "hair", "beards", "femaleorc", "maleorc",
    "femaleold", "maleold", "argonianfemale", "argonianmale", "khajiitfemale", "khajiitmale", "facegendata",
    "bretonfemale", "bretonmale", "darkelffemale", "darkelfmale", "highelffemale", "highelfmale",
    "imperialfemale", "imperialmale", "nordfemale", "nordmale", "redguardfemale", "redguardmale",
    "woodelffemale", "woodelfmale", "orcfemale", "orcmale", "elderfemale", "eldermale", "children",
}
HAIR_PACKS = {"sghairs": "SG Hairs", "ks hairdo's": "KS Hairdo's", "sgbrows": "SG Brows", "sgeyes": "SG Eyes"}


def key(path):
    """Normalise to 'textures\\...' lower-case, or None if it cannot be made relative."""
    p = path.replace("/", "\\").strip().lower()
    m = re.search(r"(^|\\)textures\\", p)
    if m:
        return "textures\\" + p[m.end():]
    if re.match(r"^[a-z]:\\", p) or p.startswith("data\\"):
        return None
    return "textures\\" + p


def category(k):
    parts = k.split("\\")
    if len(parts) > 2 and parts[1] == "cubemaps":
        return "vanilla"
    if len(parts) > 3 and parts[1] == "actors" and parts[2] == "character":
        if parts[3] in VANILLA_ACTOR_DIRS:
            return "vanilla"
        if parts[3] in HAIR_PACKS:
            return "pack:" + HAIR_PACKS[parts[3]]
    if len(parts) > 1 and parts[1] in HAIR_PACKS:
        return "pack:" + HAIR_PACKS[parts[1]]
    return "missing"


def find_dir(root, name):
    for d in os.listdir(root):
        if d.lower() == name and os.path.isdir(os.path.join(root, d)):
            return os.path.join(root, d)
    return None


def main(mod, out):
    lines = []
    log = lines.append
    tex_root = find_dir(mod, "textures")
    mesh_root = find_dir(mod, "meshes")
    if not mesh_root:
        sys.exit("No meshes folder in " + mod)

    textures = {}
    if tex_root:
        for dp, _, files in os.walk(tex_root):
            for f in files:
                if f.lower().endswith(".dds"):
                    rel = os.path.relpath(os.path.join(dp, f), mod).replace("/", "\\")
                    textures[rel.lower()] = rel
    by_name = collections.defaultdict(list)
    for k in textures:
        by_name[ntpath.basename(k)].append(k)

    # --- ESP: NPC records for FaceGen checks --------------------------------
    npcs, owners_seen = {}, set()
    for f in os.listdir(mod):
        if not f.lower().endswith((".esp", ".esm", ".esl")):
            continue
        masters, _, recs = load_esp(os.path.join(mod, f))
        owners = masters + [f]
        hdpt = {}
        for t, fid, _, body in recs:
            if t == "HDPT":
                for st, sd in subrecords(body):
                    if st == "EDID":
                        hdpt[fid] = sd[:-1].decode("cp1252")
        for t, fid, _, body in recs:
            if t != "NPC_":
                continue
            e = {"edid": "", "parts": [], "template": False, "plugin": f}
            for st, sd in subrecords(body):
                if st == "EDID":
                    e["edid"] = sd[:-1].decode("cp1252")
                elif st == "PNAM":
                    e["parts"].append(hdpt.get(struct.unpack("<I", sd)[0]))
                elif st == "TPLT":
                    e["template"] = True
            idx = fid >> 24
            owner = owners[idx] if idx < len(owners) else f
            npcs[(owner.lower(), fid & 0xFFFFFF)] = e
            owners_seen.add(owner.lower())

    fixed_files = 0
    report = collections.defaultdict(list)
    facegen_dir = "actors\\character\\facegendata\\facegeom"
    facegen_seen = set()

    for dp, _, files in os.walk(mesh_root):
        for f in files:
            if not f.lower().endswith(".nif"):
                continue
            full = os.path.join(dp, f)
            rel = os.path.relpath(full, mod).replace("/", "\\")
            rel_mesh = os.path.relpath(full, mesh_root).replace("/", "\\").lower()
            try:
                nif = Nif(full)
            except Exception as ex:  # noqa: BLE001
                report["unreadable"].append(f"{rel}: {ex}")
                continue

            facegen = None
            if rel_mesh.startswith(facegen_dir + "\\"):
                owner = ntpath.basename(ntpath.dirname(rel_mesh))
                try:
                    facegen = (owner, int(os.path.splitext(f)[0], 16) & 0xFFFFFF)
                    facegen_seen.add(facegen)
                except ValueError:
                    report["facegen-bad-name"].append(rel)

            changes = {}
            for shape, kind, slot, path, blk in nif.report():
                if kind != "lighting":
                    if (key(path) or "") not in textures and category(key(path) or "x\\x") == "missing":
                        report["missing-effect-texture"].append(f"{rel} | {shape} | {slot} | {path}")
                    continue
                slot_i = TEXTURE_SLOTS.index(slot) if slot in TEXTURE_SLOTS else int(slot)
                k = key(path)
                new = None

                if facegen and slot_i == TINT_SLOT:
                    want = f"textures\\actors\\character\\facegendata\\facetint\\{facegen[0]}\\{facegen[1]:08x}.dds"
                    if path.replace("/", "\\").lower() == want and want not in textures:
                        report["facegen-tint-needs-ck"].append(f"{rel} -> {path} (tint file not in mod)")
                    if path.replace("/", "\\").lower() != want:
                        if want in textures:
                            new = textures[want]
                        else:
                            report["facegen-tint-needs-ck"].append(f"{rel} -> {path} (expected {want}, not in mod)")
                    if new is None:
                        continue
                elif k is None or path.replace("/", "\\").lower() != k:
                    if k and (k in textures or category(k) != "missing"):
                        new = textures.get(k, path[path.lower().find("textures"):] if "textures" in path.lower() else k)
                    else:
                        report["bad-path"].append(f"{rel} | {shape} | {slot} | {path}")
                        continue
                elif k in textures:
                    continue
                else:
                    cat = category(k)
                    cands = by_name.get(ntpath.basename(k), [])
                    mirror = "textures\\" + ntpath.dirname(rel_mesh)
                    mirrored = [c for c in cands if ntpath.dirname(c) == mirror]
                    if cat == "vanilla":
                        # Exists in the vanilla BSAs, so it is not purple. A same-named copy next to the
                        # mesh may be intended (or used via a TXST swap in the ESP): report, do not change.
                        if mirrored:
                            report["vanilla-path-with-local-copy"].append(f"{rel} | {shape} | {slot} | {path} | local copy: {textures[mirrored[0]]}")
                        continue
                    if mirrored:
                        new = textures[mirrored[0]]
                    elif cat == "missing" and len(cands) == 1:
                        new = textures[cands[0]]
                    elif cat == "missing":
                        report["missing-texture"].append(f"{rel} | {shape} | {slot} | {path}" + (f" | candidates: {cands[:3]}" if cands else ""))
                        continue
                    elif cat.startswith("pack:"):
                        report["needs-" + cat[5:]].append(k)
                        continue
                    else:
                        continue

                if new and new.lower() != path.lower():
                    changes.setdefault(blk, {})[slot_i] = new
                    report["fixed"].append(f"{rel} | {shape} | {slot} | {path}  ->  {new}")

            if changes:
                for blk, slots in changes.items():
                    paths = nif.texture_set(blk)
                    for i, p in slots.items():
                        paths[i] = p
                    nif.set_texture_set(blk, paths)
                dest = os.path.join(out, rel)
                os.makedirs(os.path.dirname(dest), exist_ok=True)
                nif.save(dest)
                Nif(dest)  # re-read to make sure the written file parses
                fixed_files += 1

            if facegen and facegen in npcs:
                names = {n.lower() for n in nif.shape_names() if n}
                miss = [p for p in npcs[facegen]["parts"] if p and p.lower() not in names]
                if miss:
                    report["facegen-headpart-mismatch"].append(
                        f"{rel} ({npcs[facegen]['edid']}): ESP has {miss}, FaceGen NIF does not -> re-export FaceGen in CK (Ctrl+F4)")

    for k, e in npcs.items():
        if k not in facegen_seen and not e["template"]:
            report["npc-without-facegen"].append(f"{k[0]}\\{k[1]:08X} {e['edid']}")
    for k in facegen_seen:
        if k not in npcs:
            report["facegen-without-npc-record"].append(
                f"{k[0]}\\{k[1]:08X}: FaceGen shipped but no NPC record in the ESP -> dark face unless another plugin provides the matching record")

    order = ["fixed", "facegen-tint-needs-ck", "facegen-headpart-mismatch", "facegen-without-npc-record",
             "npc-without-facegen", "bad-path", "missing-texture", "missing-effect-texture", "unreadable", "facegen-bad-name"]
    order += sorted(k for k in report if k not in order)
    log(f"Mod: {mod}\nNIFs rewritten: {fixed_files} (in {out})\n")
    for name in order:
        items = report.get(name)
        if not items:
            continue
        if name.startswith("needs-"):
            items = sorted(set(items))
            log(f"== {name}: {len(items)} texture paths from that hair pack, not shipped in this mod (install the pack or they show purple)")
        else:
            log(f"== {name}: {len(items)}")
        for it in items:
            log("  " + it)
        log("")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "report.txt"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))
    print("\n".join(l for l in lines if l.startswith(("==", "Mod", "NIFs"))))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
