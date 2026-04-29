#!/usr/bin/env python3
import argparse
import re
import sys
from pathlib import Path


RUNNING_RE = re.compile(r"^## Running (test_[^.\s]+)\.c\.\.\.")
GENERATED_RE = re.compile(r"^Generated binary \((\d+) bytes\):")
HEX_RE = re.compile(r"(?<![0-9A-Fa-f])([0-9A-Fa-f]{2})(?![0-9A-Fa-f])")


def save_binary(out_dir, counts, test_name, data, expected_size):
    if len(data) != expected_size:
        raise RuntimeError(
            f"{test_name}: expected {expected_size} bytes, parsed {len(data)} bytes"
        )

    seq = counts.get(test_name, 0) + 1
    counts[test_name] = seq
    path = out_dir / f"{test_name}.{seq}.wasm"
    path.write_bytes(bytes(data))
    return path


def main():
    parser = argparse.ArgumentParser(
        description="Capture WAH binary spec DSL dumps from test output into wasm files."
    )
    parser.add_argument("output_dir", help="directory to write corpus files into")
    parser.add_argument(
        "--clean",
        action="store_true",
        help="delete existing *.wasm files in output_dir before capturing",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="do not mirror input lines to stdout",
    )
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    if args.clean:
        for path in out_dir.glob("*.wasm"):
            path.unlink()

    counts = {}
    current_test = "unknown"
    capturing = False
    expected_size = 0
    data = bytearray()
    saved = 0

    try:
        for raw_line in sys.stdin:
            line = raw_line.rstrip("\r\n")
            if not args.quiet:
                print(line, flush=True)

            running = RUNNING_RE.match(line)
            if running:
                current_test = running.group(1)
                continue

            generated = GENERATED_RE.match(line)
            if generated:
                if capturing:
                    save_binary(out_dir, counts, current_test, data, expected_size)
                    saved += 1
                capturing = True
                expected_size = int(generated.group(1))
                data = bytearray()
                if expected_size == 0:
                    save_binary(out_dir, counts, current_test, data, expected_size)
                    saved += 1
                    capturing = False
                continue

            if not capturing:
                continue

            hexes = HEX_RE.findall(line)
            if not hexes:
                if data:
                    save_binary(out_dir, counts, current_test, data, expected_size)
                    saved += 1
                    capturing = False
                    expected_size = 0
                    data = bytearray()
                continue

            for h in hexes:
                data.append(int(h, 16))

            if len(data) == expected_size:
                save_binary(out_dir, counts, current_test, data, expected_size)
                saved += 1
                capturing = False
                expected_size = 0
                data = bytearray()
            elif len(data) > expected_size:
                raise RuntimeError(
                    f"{current_test}: parsed {len(data)} bytes, exceeded expected "
                    f"{expected_size} bytes"
                )

        if capturing:
            save_binary(out_dir, counts, current_test, data, expected_size)
            saved += 1

    except BrokenPipeError:
        return 1
    except Exception as exc:
        print(f"capture_corpus.py: error: {exc}", file=sys.stderr)
        return 1

    print(f"capture_corpus.py: captured {saved} wasm binaries into {out_dir}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
