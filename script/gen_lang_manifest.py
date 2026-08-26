#!/usr/bin/env python3
"""
Generate per-version language manifest and compressed language files.

Usage:
  python3 script/gen_lang_manifest.py --product gravitymon --version 2.6.0 --src path/to/langjson --out bin/

Notes:
 - Injects `compatible_firmware` (major.minor) into each language JSON file.
 - Writes gzipped files and computes CRC32 (of the uncompressed data, equals gzip CRC) for manifest.
 - Writes output directly to `out/` (no subdirectory nesting).
 - Does not perform publishing; CI or release tooling should handle publishing `out/` contents.
"""
import argparse
import gzip
import json
import os
import shutil
import sys
import tempfile
import time
from pathlib import Path
import zlib


def iso_now():
    return time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())


LANGUAGE_NAMES = {
    'da': 'Dansk',
    'de': 'Deutsch',
    'es': 'Español',
    'fr': 'Français',
    'it': 'Italiano',
    'nl': 'Nederlands',
    'no': 'Norsk',
    'pl': 'Polski',
    'pt': 'Português',
    'sv': 'Svenska',
    'zh': '中文',
}


def major_minor(ver: str) -> str:
    parts = ver.split('.')
    if len(parts) < 2:
        raise ValueError('version must be at least major.minor')
    return f"{parts[0]}.{parts[1]}"


def atomic_write(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=str(path.parent), prefix=f".{path.name}.")
    try:
        with os.fdopen(fd, 'wb') as f:
            f.write(data)
        os.replace(tmp, str(path))
    finally:
        if os.path.exists(tmp):
            os.remove(tmp)


def process_lang_file(src_path: Path, out_dir: Path, compat_ver: str, full_version: str):
    # Read JSON
    raw = src_path.read_bytes()
    try:
        obj = json.loads(raw.decode('utf-8'))
    except Exception as e:
        raise RuntimeError(f"Failed to parse JSON {src_path}: {e}")

    # Serialize back (no injection needed; version info lives in manifest header)
    new_bytes = json.dumps(obj, ensure_ascii=False, separators=(',', ':')).encode('utf-8')

    # Compute CRC32 of uncompressed data (gzip CRC is of uncompressed data)
    crc = zlib.crc32(new_bytes) & 0xFFFFFFFF

    # Compress to gzip bytes
    gz_bytes = gzip.compress(new_bytes, compresslevel=6)

    # Write gz to out_dir with same basename + .gz
    out_name = 'lang_' + src_path.name + '.gz'
    out_path = out_dir / out_name
    atomic_write(out_path, gz_bytes)

    return {
        'lang': src_path.stem,
        'name': LANGUAGE_NAMES.get(src_path.stem, src_path.stem),
        'filename': out_name,
        'crc32': format(crc, '08x'),
    }


def main(argv=None):
    p = argparse.ArgumentParser(description='Generate per-version language manifest (bin output).')
    p.add_argument('--product', required=True)
    p.add_argument('--version', required=True, help='Full semantic version, e.g. 2.6.0')
    p.add_argument('--src', required=True, help='Directory containing language JSON files (e.g. en.json)')
    p.add_argument('--out', default='bin', help='Output directory (default: bin)')
    args = p.parse_args(argv)

    product = args.product
    version = args.version
    src = Path(args.src)
    out_dir = Path(args.out)

    if not src.exists() or not src.is_dir():
        print(f"Source directory does not exist: {src}", file=sys.stderr)
        return 2

    compat = major_minor(version)

    out_dir.mkdir(parents=True, exist_ok=True)

    packs = []
    for entry in sorted(src.iterdir()):
        if entry.is_file() and entry.suffix.lower() == '.json' and len(entry.stem) == 2 and entry.stem != 'en':
            print(f"Processing {entry.name}")
            info = process_lang_file(entry, out_dir, compat, version)
            packs.append(info)
    # Keep pack `url` as relative paths (e.g. ./en.json.gz).
    # Publishing tooling or client resolves to absolute host URL using `languageURL`.

    manifest = {
        'product': product,
        'version': version,
        'generated_at': iso_now(),
        'packs': packs,
    }

    manifest_bytes = json.dumps(manifest, indent=2, ensure_ascii=False).encode('utf-8')
    manifest_path = out_dir / 'lang_manifest.json'
    atomic_write(manifest_path, manifest_bytes)

    print(f"Wrote {len(packs)} packs to {out_dir}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
