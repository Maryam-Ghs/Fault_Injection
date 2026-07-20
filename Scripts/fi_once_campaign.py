#!/usr/bin/env python3
"""Compile and run deterministic, once-only ADAD/ADAF fault campaigns."""

from __future__ import annotations

import argparse
import csv
import hashlib
import os
import random
import re
import signal
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = ROOT / "Code_source" / "generated_codes"
DEFAULT_OUTPUT = ROOT / "Output" / "FI_once"
LIBRARY = ROOT / "adad" / "Adad_FI_once"

RESULT_FIELDS = [
    "campaign", "source", "program", "location_id", "kind", "operation",
    "address", "source_line", "source_lines", "source_fraction",
    "golden_first_event", "golden_total_events", "first_event_fraction",
    "target_occurrence", "target_total_occurrences", "occurrence_policy",
    "random_seed", "fault_mode", "bit_index", "golden_runtime_ns",
    "faulty_runtime_ns", "injection_elapsed_ns", "injection_global_event",
    "event_fraction", "time_fraction", "injected", "outcome", "returncode",
    "golden_output_bytes", "faulty_output_bytes", "before_bits", "after_bits",
]

STATUS_FIELDS = [
    "campaign", "source", "state", "detail", "compile_runtime_ns",
    "golden_runtime_ns", "discovery_runtime_ns", "locations", "total_events",
]


@dataclass
class CommandResult:
    returncode: int
    stdout: bytes
    stderr: bytes
    runtime_ns: int
    timed_out: bool = False


@dataclass
class Location:
    location_id: int
    kind: str
    operation: str
    count: int
    first_event: int
    description: str
    address: str
    source_line: int | None


class CsvAppender:
    def __init__(self, path: Path, fields: list[str]):
        path.parent.mkdir(parents=True, exist_ok=True)
        self.file = path.open("a", newline="", encoding="utf-8")
        self.writer = csv.DictWriter(self.file, fieldnames=fields)
        if path.stat().st_size == 0:
            self.writer.writeheader()
            self.file.flush()

    def append(self, row: dict[str, object]) -> None:
        self.writer.writerow(row)
        self.file.flush()

    def close(self) -> None:
        self.file.close()


def run_command(command: list[str], timeout_seconds: float) -> CommandResult:
    started = time.perf_counter_ns()
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        start_new_session=True,
        env={**os.environ, "LC_ALL": "C"},
    )
    try:
        stdout, stderr = process.communicate(timeout=timeout_seconds)
        return CommandResult(
            process.returncode, stdout, stderr, time.perf_counter_ns() - started
        )
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        stdout, stderr = process.communicate()
        return CommandResult(
            124, stdout, stderr, time.perf_counter_ns() - started, timed_out=True
        )


def replace_main(text: str, replacement: str) -> str:
    transformed, count = re.subn(
        r"\bint\s+main\s*\(", replacement + "(", text, count=1
    )
    if count != 1:
        raise ValueError("no conventional int main(...) was found")
    return transformed


def transform_integer(source: Path, relative: str) -> str:
    text = source.read_text(encoding="utf-8", errors="replace")
    text = re.sub(r"\binline\b", "", text)
    text = replace_main(text, "PINT pain")
    text = re.sub(r"\(\s*int\s*\)", "(PINT)", text)
    replacements = [
        (r"\bunsigned\s+long\s+long\s+int\b", "adad"),
        (r"\bunsigned\s+long\s+long\b", "adad"),
        (r"\blong\s+long\s+int\b", "adad"),
        (r"\blong\s+long\b", "adad"),
        (r"\bunsigned\s+long\s+int\b", "adad"),
        (r"\bunsigned\s+long\b", "adad"),
        (r"\bunsigned\s+int\b", "adad"),
        (r"\blong\s+int\b", "adad"),
        (r"\bunsigned\b", "adad"),
        (r"\bint\b", "adad"),
    ]
    for pattern, replacement in replacements:
        text = re.sub(pattern, replacement, text)
    text = re.sub(r"\badad\s+double\b", "long double", text)
    label = relative.replace("\\", "\\\\").replace('"', '\\"')
    return f'#include "adad.h"\n#define PINT int\n#line 1 "{label}"\n{text}'


def transform_float(source: Path, relative: str) -> str:
    text = source.read_text(encoding="utf-8", errors="replace")
    text = re.sub(r"\b(inline|constexpr|const)\b", "", text)
    text = replace_main(text, "int pain")
    text = re.sub(r"\(\s*(float|double)\s*\)", "(PINT)", text)
    replacements = [
        (r"\bunsigned\s+long\s+long\s+float\b", "adaf"),
        (r"\blong\s+long\s+float\b", "adaf"),
        (r"\bunsigned\s+long\s+float\b", "adaf"),
        (r"\bunsigned\s+float\b", "adaf"),
        (r"\blong\s+float\b", "adaf"),
        (r"\bfloat\b", "adaf"),
        (r"\bunsigned\s+long\s+long\s+double\b", "adaf"),
        (r"\blong\s+long\s+double\b", "adaf"),
        (r"\bunsigned\s+long\s+double\b", "adaf"),
        (r"\bunsigned\s+double\b", "adaf"),
        (r"\blong\s+double\b", "adaf"),
        (r"\bdouble\b", "adaf"),
    ]
    for pattern, replacement in replacements:
        text = re.sub(pattern, replacement, text)
    label = relative.replace("\\", "\\\\").replace('"', '\\"')
    return f'#include "adaf.h"\n#define PINT float\n#line 1 "{label}"\n{text}'


def parse_injection(stderr: bytes) -> dict[str, str]:
    for line in stderr.decode("utf-8", errors="replace").splitlines():
        if line.startswith("FI_INJECT\t"):
            return dict(
                field.split("=", 1)
                for field in line.split("\t")[1:]
                if "=" in field
            )
    return {}


def resolve_line(binary: Path, address: str) -> int | None:
    if not address:
        return None
    result = run_command(
        ["addr2line", "-C", "-f", "-p", "-e", str(binary), address], 5
    )
    match = re.search(rb":([0-9]+)(?:\s|$)", result.stdout)
    return int(match.group(1)) if match else None


def parse_locations(output: bytes, binary: Path) -> tuple[list[Location], int]:
    locations: list[Location] = []
    total_events = 0
    active = False
    for line in output.decode("utf-8", errors="replace").splitlines():
        if line == "FI_LOCATIONS_BEGIN":
            active = True
            continue
        if line == "FI_LOCATIONS_END":
            active = False
            continue
        if line.startswith("FI_TOTAL_EVENTS\t"):
            total_events = int(line.split("\t", 1)[1])
            continue
        if not active or line.startswith("id\t"):
            continue
        fields = line.split("\t", 5)
        if len(fields) != 6:
            continue
        location_id, kind, operation, count, first_event, description = fields
        address_match = re.search(r"\(\+(0x[0-9a-fA-F]+)\)", description)
        address = address_match.group(1) if address_match else ""
        locations.append(
            Location(
                int(location_id), kind, operation, int(count), int(first_event),
                description, address, resolve_line(binary, address),
            )
        )
    return locations, total_events


def write_locations(
    path: Path, locations: Iterable[Location], total_events: int, source_lines: int
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        fields = [
            "id", "kind", "operation", "count", "first_event",
            "first_event_fraction", "address", "line", "source_lines",
            "source_fraction", "description", "golden_total_events",
        ]
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for location in locations:
            writer.writerow({
                "id": location.location_id,
                "kind": location.kind,
                "operation": location.operation,
                "count": location.count,
                "first_event": location.first_event,
                "first_event_fraction": ratio(location.first_event, total_events),
                "address": location.address,
                "line": location.source_line or "",
                "source_lines": source_lines,
                "source_fraction": ratio(location.source_line, source_lines),
                "description": location.description,
                "golden_total_events": total_events,
            })


def ratio(numerator: int | str | None, denominator: int | str | None) -> str:
    try:
        if numerator is None or denominator is None or int(denominator) == 0:
            return ""
        return f"{int(numerator) / int(denominator):.9f}"
    except (TypeError, ValueError):
        return ""


def fault_modes(kind: str) -> tuple[str, ...]:
    if kind == "integer":
        return ("int_msb", "int_lsb", "int_middle")
    if kind == "float":
        return ("float_mantissa_middle", "float_exponent_middle")
    if kind == "boolean":
        return ("boolean",)
    return ()


def bit_for_mode(mode: str) -> int:
    return {
        "int_lsb": 0,
        "int_middle": 15,
        "int_msb": 31,
        "float_mantissa_middle": 11,
        "float_exponent_middle": 26,
        "boolean": -1,
    }[mode]


def reproducible_occurrence(
    source: str, campaign: str, location_id: int, count: int, seed: int
) -> int:
    material = f"{seed}\0{campaign}\0{source}\0{location_id}".encode()
    stable_seed = int.from_bytes(hashlib.sha256(material).digest()[:8], "big")
    return random.Random(stable_seed).randint(1, count)


def load_result_keys(path: Path) -> set[tuple[str, str, str, str]]:
    keys: set[tuple[str, str, str, str]] = set()
    if not path.exists():
        return keys
    with path.open(newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            keys.add((
                row["campaign"], row["source"], row["location_id"],
                row["fault_mode"] + ":" + row["target_occurrence"],
            ))
    return keys


def load_completed(path: Path) -> set[tuple[str, str]]:
    completed: set[tuple[str, str]] = set()
    if not path.exists():
        return completed
    with path.open(newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            if row.get("state") == "complete":
                completed.add((row["campaign"], row["source"]))
    return completed


def save_failure_logs(
    output_root: Path, campaign: str, program: str, location_id: int,
    mode: str, occurrence: int, result: CommandResult,
) -> None:
    directory = output_root / "fault_logs" / campaign
    directory.mkdir(parents=True, exist_ok=True)
    prefix = directory / f"{program}__id{location_id}__{mode}__c{occurrence}"
    prefix.with_suffix(".stdout").write_bytes(result.stdout)
    prefix.with_suffix(".stderr").write_bytes(result.stderr)


def process_program(
    source: Path,
    campaign: str,
    args: argparse.Namespace,
    results: CsvAppender,
    statuses: CsvAppender,
    result_keys: set[tuple[str, str, str, str]],
) -> None:
    relative = source.relative_to(args.source_dir).as_posix()
    program = relative.removesuffix(".cpp").replace("/", "__")
    source_lines = sum(1 for _ in source.open("rb"))
    transformed_path = args.output_dir / "transformed" / campaign / f"{program}.cpp"
    binary = args.output_dir / "binaries" / campaign / f"{program}.out"
    transformed_path.parent.mkdir(parents=True, exist_ok=True)
    binary.parent.mkdir(parents=True, exist_ok=True)

    try:
        transformed = (
            transform_integer(source, relative)
            if campaign == "integer"
            else transform_float(source, relative)
        )
        transformed_path.write_text(transformed, encoding="utf-8")
    except Exception as error:
        statuses.append(status_row(campaign, relative, "transform_failed", str(error)))
        return

    operator_source = "adad.cpp" if campaign == "integer" else "adaf.cpp"
    compile_result = run_command([
        "g++", "-std=c++17", "-g", "-O0", "-fno-inline",
        "-fno-omit-frame-pointer", "-rdynamic", f"-I{LIBRARY}",
        str(transformed_path), str(LIBRARY / "main.cpp"),
        str(LIBRARY / operator_source), str(LIBRARY / "fault.cpp"),
        "-lm", "-o", str(binary),
    ], args.compile_timeout)
    if compile_result.returncode != 0:
        log = args.output_dir / "compile_logs" / campaign / f"{program}.log"
        log.parent.mkdir(parents=True, exist_ok=True)
        log.write_bytes(compile_result.stderr)
        statuses.append(status_row(
            campaign, relative, "compile_failed", f"returncode={compile_result.returncode}",
            compile_ns=compile_result.runtime_ns,
        ))
        return

    golden = run_command([str(binary)], args.timeout)
    golden_dir = args.output_dir / "golden" / campaign
    golden_dir.mkdir(parents=True, exist_ok=True)
    (golden_dir / f"{program}.stdout").write_bytes(golden.stdout)
    (golden_dir / f"{program}.stderr").write_bytes(golden.stderr)
    if golden.returncode != 0:
        statuses.append(status_row(
            campaign, relative, "golden_failed",
            "timeout" if golden.timed_out else f"returncode={golden.returncode}",
            compile_result.runtime_ns, golden.runtime_ns,
        ))
        return

    discovery = run_command([str(binary), "-l"], args.timeout)
    if discovery.returncode != 0:
        statuses.append(status_row(
            campaign, relative, "discovery_failed",
            "timeout" if discovery.timed_out else f"returncode={discovery.returncode}",
            compile_result.runtime_ns, golden.runtime_ns, discovery.runtime_ns,
        ))
        return

    locations, total_events = parse_locations(discovery.stdout, binary)
    write_locations(
        args.output_dir / "locations" / campaign / f"{program}.csv",
        locations, total_events, source_lines,
    )
    if args.max_locations:
        locations = locations[: args.max_locations]

    for location in locations:
        for mode in fault_modes(location.kind):
            occurrence = (
                1 if args.occurrence == "first" else reproducible_occurrence(
                    relative, campaign, location.location_id, location.count, args.seed
                )
            )
            key = (
                campaign, relative, str(location.location_id), f"{mode}:{occurrence}"
            )
            if args.resume and key in result_keys:
                continue

            faulty = run_command([
                str(binary), "-p", f"{location.location_id},1", "-c",
                str(occurrence), "-t", mode,
            ], args.timeout)
            injection = parse_injection(faulty.stderr)
            injected = bool(injection)
            if faulty.timed_out:
                outcome = "timeout"
            elif faulty.returncode != 0:
                outcome = "crash_or_error"
            elif not injected:
                outcome = "not_injected"
            elif faulty.stdout == golden.stdout:
                outcome = "matched"
            else:
                outcome = "not_matched"

            if faulty.returncode != 0 or not injected:
                save_failure_logs(
                    args.output_dir, campaign, program, location.location_id,
                    mode, occurrence, faulty,
                )

            injection_event = injection.get("global_event", "")
            injection_elapsed = injection.get("elapsed_ns", "")
            row = {
                "campaign": campaign,
                "source": relative,
                "program": program,
                "location_id": location.location_id,
                "kind": location.kind,
                "operation": location.operation,
                "address": location.address,
                "source_line": location.source_line or "",
                "source_lines": source_lines,
                "source_fraction": ratio(location.source_line, source_lines),
                "golden_first_event": location.first_event,
                "golden_total_events": total_events,
                "first_event_fraction": ratio(location.first_event, total_events),
                "target_occurrence": occurrence,
                "target_total_occurrences": location.count,
                "occurrence_policy": args.occurrence,
                "random_seed": args.seed if args.occurrence == "random" else "",
                "fault_mode": mode,
                "bit_index": bit_for_mode(mode),
                "golden_runtime_ns": golden.runtime_ns,
                "faulty_runtime_ns": faulty.runtime_ns,
                "injection_elapsed_ns": injection_elapsed,
                "injection_global_event": injection_event,
                "event_fraction": ratio(injection_event, total_events),
                "time_fraction": ratio(injection_elapsed, golden.runtime_ns),
                "injected": 1 if injected else 0,
                "outcome": outcome,
                "returncode": faulty.returncode,
                "golden_output_bytes": len(golden.stdout),
                "faulty_output_bytes": len(faulty.stdout),
                "before_bits": injection.get("before", ""),
                "after_bits": injection.get("after", ""),
            }
            results.append(row)
            result_keys.add(key)

    statuses.append(status_row(
        campaign, relative, "complete", "", compile_result.runtime_ns,
        golden.runtime_ns, discovery.runtime_ns, len(locations), total_events,
    ))


def status_row(
    campaign: str, source: str, state: str, detail: str,
    compile_ns: int | str = "", golden_ns: int | str = "",
    discovery_ns: int | str = "", locations: int | str = "",
    total_events: int | str = "",
) -> dict[str, object]:
    return {
        "campaign": campaign,
        "source": source,
        "state": state,
        "detail": detail,
        "compile_runtime_ns": compile_ns,
        "golden_runtime_ns": golden_ns,
        "discovery_runtime_ns": discovery_ns,
        "locations": locations,
        "total_events": total_events,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--campaign", choices=("integer", "float", "both"), default="both"
    )
    parser.add_argument(
        "--occurrence", choices=("first", "random"), default="first",
        help="inject at the first occurrence or one reproducibly random occurrence",
    )
    parser.add_argument("--seed", type=int, default=1, help="random policy seed")
    parser.add_argument("--source-dir", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--compile-timeout", type=float, default=60.0)
    parser.add_argument("--limit", type=int, default=0, help="limit source files (testing)")
    parser.add_argument(
        "--max-locations", type=int, default=0,
        help="limit locations per program (testing)",
    )
    parser.add_argument("--filter", default="", help="source-path substring filter")
    parser.add_argument(
        "--resume", action=argparse.BooleanOptionalAction, default=True
    )
    args = parser.parse_args()
    args.source_dir = args.source_dir.resolve()
    args.output_dir = args.output_dir.resolve()
    return args


def main() -> int:
    args = parse_args()
    if not args.source_dir.is_dir():
        print(f"source directory does not exist: {args.source_dir}", file=sys.stderr)
        return 2

    sources = sorted(args.source_dir.rglob("*.cpp"))
    if args.filter:
        sources = [source for source in sources if args.filter in str(source)]
    if args.limit:
        sources = sources[: args.limit]
    campaigns = (
        ("integer", "float") if args.campaign == "both" else (args.campaign,)
    )

    results_path = args.output_dir / "results.csv"
    statuses_path = args.output_dir / "program_status.csv"
    result_keys = load_result_keys(results_path) if args.resume else set()
    completed = load_completed(statuses_path) if args.resume else set()
    results = CsvAppender(results_path, RESULT_FIELDS)
    statuses = CsvAppender(statuses_path, STATUS_FIELDS)

    try:
        total = len(sources) * len(campaigns)
        current = 0
        for campaign in campaigns:
            for source in sources:
                current += 1
                relative = source.relative_to(args.source_dir).as_posix()
                if args.resume and (campaign, relative) in completed:
                    print(f"[{current}/{total}] skip complete {campaign} {relative}")
                    continue
                print(f"[{current}/{total}] {campaign} {relative}", flush=True)
                process_program(
                    source, campaign, args, results, statuses, result_keys
                )
    except KeyboardInterrupt:
        print("interrupted; completed rows are safe and --resume is enabled", file=sys.stderr)
        return 130
    finally:
        results.close()
        statuses.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
