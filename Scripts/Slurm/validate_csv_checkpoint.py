#!/usr/bin/env python3
"""Validate that a copied campaign CSV ends on a complete, rectangular row."""

from __future__ import annotations

import csv
import sys
from pathlib import Path


def main() -> int:
    path = Path(sys.argv[1])
    data = path.read_bytes()
    if not data or not data.endswith(b"\n"):
        return 1
    with path.open(newline="", encoding="utf-8") as handle:
        rows = csv.reader(handle)
        header = next(rows, [])
        if not header:
            return 1
        count = 0
        for line_number, row in enumerate(rows, 2):
            if len(row) != len(header):
                print(
                    f"{path}: row {line_number} has {len(row)} fields; "
                    f"expected {len(header)}",
                    file=sys.stderr,
                )
                return 1
            count += 1
    print(count)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
