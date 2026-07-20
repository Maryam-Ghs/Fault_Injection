# Once-only deterministic fault campaign

`Run_FI_once.sh` reads every C++ file below `Code_source/generated_codes` by
default. Integer and floating-point instrumentation are separate campaigns;
Boolean comparison inversion is included in both campaigns.

## Fault modes

- Integer: bit 31 (MSB), bit 0 (LSB), and bit 15 (middle).
- Float: bit 11 (middle of mantissa) and bit 26 (middle of exponent).
- Boolean: invert the result of an instrumented comparison.

Each faulty process injects at most one fault. The default campaign injects on
the first execution of each location:

```bash
Scripts/Run_FI_once.sh
```

To select one reproducibly random occurrence between 1 and the location's
golden occurrence count:

```bash
Scripts/Run_FI_once.sh --occurrence random --seed 2026
```

Both commands can use the same output directory. Resume keys include the
occurrence policy and random seed.

Before a full run, a small validation campaign is useful:

```bash
Scripts/Run_FI_once.sh --limit 2 --max-locations 3
```

Other useful selections:

```bash
Scripts/Run_FI_once.sh --campaign integer
Scripts/Run_FI_once.sh --campaign float
Scripts/Run_FI_once.sh --filter 2_Opt_Heuristic_TSP_v10
Scripts/Run_FI_once.sh --timeout 10 --compile-timeout 120
```

Runs are resumable by default. A completed source/configuration is skipped, and
individual result rows are written immediately. Use `--no-resume` only when
duplicate reruns are intentionally wanted.

To remove all previous `Output/FI_once` data and immediately start a completely
new campaign:

```bash
Scripts/Run_FI_once.sh --clean
```

`--clean` removes results, per-program reports, binaries, transformed sources,
golden outputs, and logs in the selected `--output-dir`. To repair a result file
written across the older and current CSV schemas without running new faults:

```bash
Scripts/Run_FI_once.sh --repair-only
```

The repair is atomic and retains the original as a timestamped
`results.csv.schema_backup_*` file. Future runs validate CSV headers before
appending and stop instead of producing shifted columns.

## Outputs

Everything is written below `Output/FI_once`:

- `results.csv`: one faulty execution per row.
- `FI_Reports/<campaign>/*_results.csv`: old-style per-program result files
  containing the same expanded columns as the consolidated CSV.
- `program_status.csv`: transformation, compilation, golden-run, discovery, or
  completion state for every source and campaign.
- `locations/`: discovered locations, occurrence counts, source positions, and
  first dynamic event positions.
- `golden/`: golden standard output and runtime metadata.
- `transformed/` and `binaries/`: retained reproducibility artifacts.
- `compile_logs/`: compiler errors for rejected transformations.
- `fault_logs/`: output and diagnostics for crashes, timeouts, or missing
  injections.

`results.csv` includes external wall-clock runtime and runtime measured inside
the transformed `pain()` function. Position estimates include:

- `source_fraction`: source line divided by total source lines.
- `source_fraction_from_end`: remaining source fraction after that line.
- `first_event_fraction`: the target's first golden event divided by all golden
  instrumented events.
- `event_fraction`: the actual injected event divided by all golden events.
- `event_fraction_from_end`: remaining dynamic-event fraction after injection.
- `time_fraction`: time to injection divided by golden `pain()` runtime.
- `time_fraction_from_end`: remaining runtime fraction after injection.

For compatibility with earlier reports, every row also contains
`crash_count`, `timeout_count`, `matched_count`, and `notmatched_count`. Because
the new design runs each location/mode once, these fields are either 0 or 1.

Timing is measured once, so very short programs are noisy and `time_fraction`
can occasionally exceed 1. The event fraction is generally the more stable
dynamic beginning/end estimate.

The float campaign intentionally converts both `float` and `double` to the
current 32-bit ADAF wrapper. Programs that cannot tolerate the source-level
transformation are recorded and skipped rather than stopping the campaign.
