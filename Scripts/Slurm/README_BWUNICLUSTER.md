# bwUniCluster once-only random fault campaign

This workflow uses independent single-core Slurm array tasks. Every task copies
one compressed input archive to its node-local `$TMPDIR`, processes a disjoint
source shard, and copies compact results back to a durable workspace. Tasks
never write to the same CSV.

The submit helper exports the real absolute `Scripts/Slurm` directory because
Slurm executes a spooled copy of each `.sbatch` file; resolving helper scripts
relative to the spooled copy would incorrectly search under `/var/spool/slurmd`.

## Why `$TMPDIR`

bwUniCluster creates a unique `$TMPDIR` on the local SSD for each job. It is
fast for the many small transformed, compiler, binary, and output files used by
this campaign. Its contents are deleted at job end, so the worker snapshots
`results.csv` and `program_status.csv` and packages useful artifacts before
exit. Do not replace `$TMPDIR` with `/tmp` or `/scratch`.

BeeOND is not recommended for this workflow: each array task is a single-node,
independent workload, whereas BeeOND is useful for a shared temporary parallel
filesystem across multiple exclusively allocated nodes. Requesting BeeOND and
`--exclusive` here would usually waste resources.

## 1. Create durable storage

On a login node, allocate/find a workspace (choose your own name and lifetime):

```bash
ws_allocate fi-once-results 60
export FI_WS=$(ws_find fi-once-results)
```

KIT users running mainly on `cpu_il` can consider the flash workspace `ffuc`,
but ordinary workspace storage is sufficient because heavy small-file I/O stays
on `$TMPDIR`:

```bash
ws_allocate -F ffuc fi-once-results 60
export FI_WS=$(ws_find -F ffuc fi-once-results)
```

## 2. Prepare one compact input archive

Run this from the repository checkout after pulling the desired commit:

```bash
Scripts/Slurm/prepare_fi_once_archive.sh "$FI_WS/fault-injection-input.tar.gz"
```

The archive contains only `Code_source`, `adad`, and `Scripts`, not `.git` or
previous outputs. Keep this archive unchanged while resuming a run, because
shard membership depends on the sorted source corpus.

## 3. Test in a development allocation

Development queues are only for workflow validation, not the production
campaign. An interactive smoke test can confirm the toolchain and `$TMPDIR`:

```bash
salloc -p dev_cpu_il -n 1 -t 00:20:00
```

Inside the allocation, extract the archive under `$TMPDIR` and run a small
campaign with `--limit` and `--max-locations`, or submit a small array with a
short wall time.

## 4. Check availability once and submit production work

```bash
sinfo_t_idle
squeue --start
```

Choose `cpu_il` or `cpu` based on access, current availability, and expected
start time. Do not place Slurm commands in a tight loop or use plain `watch`;
bwUniCluster requires at least 30 seconds between scheduler queries.

One complete random-occurrence campaign with seed 2026:

```bash
Scripts/Slurm/submit_fi_once_bwunicluster.sh \
  --archive "$FI_WS/fault-injection-input.tar.gz" \
  --result-root "$FI_WS/results" \
  --partition cpu_il \
  --time 12:00:00 \
  --shards 64 \
  --max-concurrent 16 \
  --seed-start 2026 \
  --seed-count 1 \
  --campaign both
```

The occurrence choice already varies by source, campaign, and location even
with one seed. For five complete statistical repetitions (seeds 2026–2030),
use `--seed-count 5`. This multiplies compute cost by five:

```bash
Scripts/Slurm/submit_fi_once_bwunicluster.sh \
  --archive "$FI_WS/fault-injection-input.tar.gz" \
  --result-root "$FI_WS/results" \
  --seed-start 2026 --seed-count 5
```

For one reproducible baseline plus one newly generated Monte Carlo sample, use:

```bash
Scripts/Slurm/submit_fi_once_bwunicluster.sh \
  --archive "$FI_WS/fault-injection-input.tar.gz" \
  --result-root "$FI_WS/results" \
  --partition cpu_il \
  --time 12:00:00 \
  --shards 64 \
  --max-concurrent 16 \
  --seed-start 2026 \
  --seed-count 1 \
  --random-seed-count 1 \
  --campaign both
```

Random seeds combine nanosecond submission time with operating-system entropy.
The exact values and their origins are saved before submission in
`RESULT_ROOT/RUN_ID/seed_manifest.csv`; every result row also contains its seed
in `random_seed`. Thus a random sample is still exactly reproducible. Resuming
with the same `--run-id` reuses the saved manifest instead of generating new
seeds. Increase `--random-seed-count` for more Monte Carlo samples, remembering
that every additional seed repeats the complete corpus.

The defaults request one physical core per array task. The Python campaign is
sequential, so asking for more CPUs per task would not speed it up. `64` shards
with at most `16` concurrent tasks is a conservative starting point. More
shards make individual jobs shorter and improve tail recovery; concurrency
controls simultaneous resource use and fair-share impact.

## Monitoring and recovery

The submit helper prints the worker-array and dependent merge-job IDs:

```bash
squeue -j WORKER_ID,MERGE_ID
squeue --start -j WORKER_ID
scontrol show job WORKER_ID
```

At five minutes before the time limit Slurm sends `USR1`; the worker snapshots
its CSVs. Independently, each worker validates and atomically publishes its
current `results.csv` and `program_status.csv` every five minutes (configurable
with `--checkpoint-seconds`). A successful shard writes `complete.marker`, so
the merger reports partial shards as `incomplete_shards` instead of treating a
checkpoint as finished. For compatibility, successful `exit_status=0` manifests
from runs made before completion markers were added are also accepted. If tasks
time out or fail, resubmit with the same archive, shard count,
seed settings, and printed run ID:

```bash
Scripts/Slurm/submit_fi_once_bwunicluster.sh \
  --archive "$FI_WS/fault-injection-input.tar.gz" \
  --result-root "$FI_WS/results" \
  --run-id PREVIOUS_RUN_ID \
  --shards 64
```

Each task restores its durable snapshot and resumes completed source/config
rows. The merge job runs after the array finishes even if some tasks fail. Its
`merged/merge_summary.txt` lists missing shards; a later resubmission and merge
fills them. Result shards are consolidated as a bounded-memory stream, so a
large campaign does not need to fit entirely in the merge job's RAM.

## Durable output layout

```text
RESULT_ROOT/RUN_ID/
  seed_manifest.csv
  logs/
  shards/seed_<N>/shard_<K>/
    results.csv
    program_status.csv
    artifacts.tar.gz
    job_manifest.txt
    checkpoint_status.txt
    complete.marker
  merged/
    results.csv
    program_status.csv
    merge_summary.txt
```

Large reproducible intermediates (`binaries/` and `transformed/`) stay on local
SSD and are deleted with `$TMPDIR`. The durable artifact archive contains
per-program reports, locations, golden output, and failure logs.
