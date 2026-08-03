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


def concatenate_files(paths: list[Path], destination: Path) -> tuple[int, list[str]]:
    """Stream disjoint shard rows without retaining the full campaign in RAM."""
    expected_header: list[str] | None = None
    count = 0
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_name(f".{destination.name}.tmp")
    with temporary.open("w", newline="", encoding="utf-8") as output:
        writer: csv.DictWriter[str] | None = None
        for path in paths:
            with path.open(newline="", encoding="utf-8") as handle:
                reader = csv.DictReader(handle)
                if reader.fieldnames is None:
                    raise ValueError(f"missing CSV header: {path}")
                if expected_header is None:
                    expected_header = reader.fieldnames
                    writer = csv.DictWriter(output, fieldnames=expected_header)
                    writer.writeheader()
                elif reader.fieldnames != expected_header:
                    raise ValueError(f"schema mismatch: {path}")
                assert writer is not None
                for row in reader:
                    if None in row:
                        raise ValueError(f"row width does not match header: {path}")
                    writer.writerow(row)
                    count += 1
        output.flush()
        os.fsync(output.fileno())
    if expected_header is None:
        temporary.unlink()
        return 0, []
    os.replace(temporary, destination)
    return count, expected_header


def shard_is_complete(path: Path) -> bool:
    """Accept the new marker or a successful manifest from older workers."""
    if (path / "complete.marker").is_file():
        return True
    manifest = path / "job_manifest.txt"
    if not manifest.is_file():
        return False
    values: dict[str, str] = {}
    for line in manifest.read_text(encoding="utf-8", errors="replace").splitlines():
        key, separator, value = line.partition("=")
        if separator:
            values[key] = value
    return (
        values.get("exit_status") == "0"
        and (path / "results.csv").is_file()
        and (path / "program_status.csv").is_file()
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-root", type=Path, required=True)
    parser.add_argument("--shard-count", type=int, required=True)
    parser.add_argument("--seed-list", help="colon-separated explicit seed list")
    parser.add_argument("--seed-start", type=int)
    parser.add_argument("--seed-count", type=int)
    args = parser.parse_args()

    if args.seed_list:
        seeds = [int(value) for value in args.seed_list.split(":")]
    elif args.seed_start is not None and args.seed_count is not None:
        seeds = list(range(args.seed_start, args.seed_start + args.seed_count))
    else:
        parser.error("provide --seed-list or both --seed-start and --seed-count")
    if not seeds:
        parser.error("seed list must not be empty")

    run_root = args.run_root.resolve()
    shard_root = run_root / "shards"
    merged_root = run_root / "merged"
    merged_root.mkdir(parents=True, exist_ok=True)
    expected = [
        shard_root / f"seed_{seed}" / f"shard_{shard}"
        for seed in seeds
        for shard in range(args.shard_count)
    ]
    missing_results = [path for path in expected if not (path / "results.csv").is_file()]
    missing_status = [path for path in expected if not (path / "program_status.csv").is_file()]
    incomplete = [path for path in expected if not shard_is_complete(path)]

    result_paths = [path / "results.csv" for path in expected if (path / "results.csv").is_file()]
    status_paths = [
        path / "program_status.csv"
        for path in expected
        if (path / "program_status.csv").is_file()
    ]
    # Sources are partitioned disjointly between shard files, and the campaign
    # runner prevents duplicate result keys while resuming. Stream this large
    # table to keep merge memory bounded.
    result_count, _ = concatenate_files(
        result_paths, merged_root / "results.csv"
    )
    status_count, _ = merge_files(
        status_paths,
        merged_root / "program_status.csv",
        ("campaign", "source", "occurrence_policy", "random_seed"),
    )

    outcomes: Counter[str] = Counter()
    merged_results = merged_root / "results.csv"
    if merged_results.exists():
        with merged_results.open(newline="", encoding="utf-8") as handle:
            outcomes.update(
                row.get("outcome", "unknown") for row in csv.DictReader(handle)
            )

    summary = merged_root / "merge_summary.txt"
    with summary.open("w", encoding="utf-8") as handle:
        handle.write(f"result_rows={result_count}\n")
        handle.write(f"status_rows={status_count}\n")
        handle.write(f"expected_shards={len(expected)}\n")
        handle.write(f"missing_result_shards={len(missing_results)}\n")
        handle.write(f"missing_status_shards={len(missing_status)}\n")
        handle.write(f"incomplete_shards={len(incomplete)}\n")
        for outcome, count in sorted(outcomes.items()):
            handle.write(f"outcome_{outcome}={count}\n")
        for path in missing_results:
            handle.write(f"missing_results={path.relative_to(run_root)}\n")
        for path in incomplete:
            handle.write(f"incomplete={path.relative_to(run_root)}\n")

    print(summary.read_text(encoding="utf-8"), end="")
    return 0 if not missing_results and not missing_status and not incomplete else 1


if __name__ == "__main__":
    raise SystemExit(main())
