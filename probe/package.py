#!/usr/bin/env python3
import argparse
from pathlib import Path
import shutil
import stat
import zipfile

ROOT = Path(__file__).resolve().parent
PORT = ROOT / "port"

def add_tree(zf: zipfile.ZipFile, root: Path, prefix: str = ""):
    for p in sorted(root.rglob("*")):
        if p.is_file():
            arc = str(Path(prefix) / p.relative_to(root))
            zf.write(p, arc)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--binary", required=True, type=Path)
    ap.add_argument("--out", default=ROOT / "dist", type=Path)
    args = ap.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    stage = args.out / "stage"
    if stage.exists():
        shutil.rmtree(stage)
    (stage / "r36sprobe").mkdir(parents=True)
    shutil.copy2(PORT / "R36S Hardware Probe.sh", stage / "R36S Hardware Probe.sh")
    shutil.copy2(PORT / "port.json", stage / "port.json")
    shutil.copy2(PORT / "gameinfo.xml", stage / "gameinfo.xml")
    shutil.copy2(PORT / "README.md", stage / "r36sprobe" / "README.md")
    shutil.copy2(args.binary, stage / "r36sprobe" / "r36s-hwprobe.aarch64")
    for p in [stage / "R36S Hardware Probe.sh", stage / "r36sprobe" / "r36s-hwprobe.aarch64"]:
        p.chmod(p.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    out = args.out / "r36s-hardware-probe.zip"
    with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        add_tree(zf, stage)
    print(out)

if __name__ == "__main__":
    main()
