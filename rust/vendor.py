#!/usr/bin/env python3
"""Copies the C sources and the license of WAH into the crates, as `cargo package` and `cargo publish` need them
(they only package the files under each crate). Run this before either: `python vendor.py && cargo publish -p wah-sys`.
The copies are ignored by git; builds within the repository use the originals (warning if the copies are stale)."""

import shutil
from pathlib import Path

rust = Path(__file__).resolve().parent
root = rust.parent

copies = [
    ("wah.h", "wah-sys/vendor/wah.h"),
    ("wahjit.h", "wah-sys/vendor/wahjit.h"),
    ("LICENSE.txt", "wah-sys/LICENSE.txt"),
    ("LICENSE.txt", "wah/LICENSE.txt"),
]

for src, dst in copies:
    src, dst = root / src, rust / dst
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, dst)
    print(f"{src.relative_to(root)} -> {dst.relative_to(root)}")
