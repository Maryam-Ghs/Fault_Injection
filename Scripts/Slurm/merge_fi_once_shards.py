#!/usr/bin/env python3
"""Merge independent bwUniCluster shard CSVs into durable campaign reports."""

from __future__ import annotations

import argparse
import csv
import os
from collections import Counter
from pathlib import Path


def read_csv(path: Path) -> tuple[list[str], list[dict[str, str]]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise ValueError(f"missing CSV header: {path}")
        rows = list(reader)
        if any(None in row for row in rows):
            raise ValueError(f"row width does not match header: {path}")
        return reader.fieldnames, rows


def atomic_write(path: Path, fields: list[str], rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.tmp")
    with temporary.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(temporary, path)


def merge_files(
    paths: list[Path], destination: Path, key_fields: tuple[str, ...]
) -> tuple[int, list[str]]:
    expected_header: list[str] | None = None
    merged: dict[tuple[str, ...], dict[str, str]] = {}
    for path in paths:
        header, rows = read_csv(path)
        if expected_header is None:
            expected_header = header
        elif header != expected_header:
            raise ValueError(f"schema mismatch: {path}")
        for row in rows:
            merged[tuple(row.get(field, "") for field in key_fields)] = row
    if expected_header is None:
        return 0, []
    ordered = sorted(merged.values(), key=lambda row: tuple(row.get(k, "") for k in key_fields))
    atomic_write(destination, expected_header, ordered)
    return len(ordered), expected_header


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-root", type=Path, required=True)
    parser.add_argument("--shard-count", type=int, required=True)
    parser.add_argument("--seed-start", type=int, required=True)
    parser.add_argument("--seed-count", type=int, required=True)
    args = parser.parse_args()

    run_root = args.run_root.resolve()
    shard_root = run_root / "shards"
    merged_root = run_root / "merged"
    expected = [
        shard_root / f"seed_{seed}" / f"shard_{shard}"
        for seed in range(args.seed_start, args.seed_start + args.seed_count)
        for shard in range(args.shard_count)
    ]
    missing_results = [path for path in expected if not (path / "results.csv").is_file()]
    missing_status = [path for path in expected if not (path / "program_status.csv").is_file()]

    result_paths = sorted(shard_root.glob("seed_*/shard_*/results.csv"))
    status_paths = sorted(shard_root.glob("seed_*/shard_*/program_status.csv"))
    result_count, _ = merge_files(
        result_paths,
        merged_root / "results.csv",
        ("campaign", "source", "location_id", "fault_mode", "target_occurrence",
         "occurrence_policy", "random_seed"),
    )
    status_count, _ = merge_files(
        status_paths,
        merged_root / "program_status.csv",
        ("campaign", "source", "occurrence_policy", "random_seed"),
    )

    outcomes: Counter[str] = Counter()
    merged_results = merged_root / "results.csv"
    if merged_results.exists():
        _, rows = read_csv(merged_results)
        outcomes.update(row.get("outcome", "unknown") for row in rows)

    summary = merged_root / "merge_summary.txt"
    with summary.open("w", encoding="utf-8") as handle:
        handle.write(f"result_rows={result_count}\n")
        handle.write(f"status_rows={status_count}\n")
        handle.write(f"expected_shards={len(expected)}\n")
        handle.write(f"missing_result_shards={len(missing_results)}\n")
        handle.write(f"missing_status_shards={len(missing_status)}\n")
        for outcome, count in sorted(outcomes.items()):
            handle.write(f"outcome_{outcome}={count}\n")
        for path in missing_results:
            handle.write(f"missing_results={path.relative_to(run_root)}\n")

    print(summary.read_text(encoding="utf-8"), end="")
    return 0 if not missing_results and not missing_status else 1


if __name__ == "__main__":
    raise SystemExit(main())
