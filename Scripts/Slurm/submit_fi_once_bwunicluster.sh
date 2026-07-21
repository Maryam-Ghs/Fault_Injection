#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PARTITION=cpu_il
WALLTIME=12:00:00
SHARDS=64
MAX_CONCURRENT=16
SEED_START=2026
SEED_COUNT=1
CAMPAIGN=both
ACCOUNT=""
RUN_ID=""
ARCHIVE=""
RESULT_ROOT=""

usage() {
    cat <<'EOF'
Usage: submit_fi_once_bwunicluster.sh --archive FILE --result-root DIR [options]

Required:
  --archive FILE          Archive made by prepare_fi_once_archive.sh
  --result-root DIR       Durable workspace (not node-local TMPDIR)

Options:
  --partition NAME        cpu_il (default) or cpu
  --time HH:MM:SS         12:00:00 default
  --shards N              Disjoint source shards (default 64)
  --max-concurrent N      Maximum simultaneously running tasks (default 16)
  --seed-start N          First reproducible random seed (default 2026)
  --seed-count N          Complete corpus repetitions (default 1)
  --campaign MODE         integer, float, or both (default both)
  --account NAME          Optional Slurm project account
  --run-id NAME           Resume/name a run (default timestamp)

Each seed is a complete run over every source. Array size is shards*seed-count.
EOF
}

while (($#)); do
    case "$1" in
        --archive) ARCHIVE=$2; shift 2 ;;
        --result-root) RESULT_ROOT=$2; shift 2 ;;
        --partition) PARTITION=$2; shift 2 ;;
        --time) WALLTIME=$2; shift 2 ;;
        --shards) SHARDS=$2; shift 2 ;;
        --max-concurrent) MAX_CONCURRENT=$2; shift 2 ;;
        --seed-start) SEED_START=$2; shift 2 ;;
        --seed-count) SEED_COUNT=$2; shift 2 ;;
        --campaign) CAMPAIGN=$2; shift 2 ;;
        --account) ACCOUNT=$2; shift 2 ;;
        --run-id) RUN_ID=$2; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

[[ -f "$ARCHIVE" ]] || { echo "Archive not found: $ARCHIVE" >&2; exit 2; }
[[ -n "$RESULT_ROOT" ]] || { echo "--result-root is required" >&2; exit 2; }
[[ "$SHARDS" =~ ^[1-9][0-9]*$ ]] || { echo "Invalid shard count" >&2; exit 2; }
[[ "$SEED_COUNT" =~ ^[1-9][0-9]*$ ]] || { echo "Invalid seed count" >&2; exit 2; }
[[ "$MAX_CONCURRENT" =~ ^[1-9][0-9]*$ ]] || { echo "Invalid concurrency" >&2; exit 2; }
[[ "$CAMPAIGN" == integer || "$CAMPAIGN" == float || "$CAMPAIGN" == both ]] || {
    echo "Invalid campaign: $CAMPAIGN" >&2; exit 2;
}

ARCHIVE=$(readlink -f "$ARCHIVE")
mkdir -p "$RESULT_ROOT"
RESULT_ROOT=$(readlink -f "$RESULT_ROOT")
[[ -n "$RUN_ID" ]] || RUN_ID="fi_random_$(date +%Y%m%d_%H%M%S)"
RUN_ROOT="$RESULT_ROOT/$RUN_ID"
mkdir -p "$RUN_ROOT/logs"

TASK_COUNT=$((SHARDS * SEED_COUNT))
ARRAY_LAST=$((TASK_COUNT - 1))
EXPORTS="ALL,FI_ARCHIVE=$ARCHIVE,FI_RESULT_ROOT=$RESULT_ROOT,FI_RUN_ID=$RUN_ID,FI_SHARD_COUNT=$SHARDS,FI_SEED_START=$SEED_START,FI_CAMPAIGN=$CAMPAIGN"
SBATCH_ARGS=(
    --parsable
    --partition "$PARTITION"
    --time "$WALLTIME"
    --array "0-${ARRAY_LAST}%${MAX_CONCURRENT}"
    --output "$RUN_ROOT/logs/worker_%A_%a.out"
    --error "$RUN_ROOT/logs/worker_%A_%a.err"
    --export "$EXPORTS"
)
[[ -n "$ACCOUNT" ]] && SBATCH_ARGS+=(--account "$ACCOUNT")

ARRAY_JOB=$(sbatch "${SBATCH_ARGS[@]}" "$SCRIPT_DIR/fi_once_bwunicluster.sbatch")
ARRAY_JOB=${ARRAY_JOB%%;*}
echo "Submitted worker array: $ARRAY_JOB ($TASK_COUNT tasks, max $MAX_CONCURRENT concurrent)"

MERGE_EXPORTS="ALL,FI_RESULT_ROOT=$RESULT_ROOT,FI_RUN_ID=$RUN_ID,FI_SHARD_COUNT=$SHARDS,FI_SEED_START=$SEED_START,FI_SEED_COUNT=$SEED_COUNT"
MERGE_ARGS=(
    --parsable
    --partition "$PARTITION"
    --time 00:15:00
    --nodes 1
    --ntasks 1
    --dependency "afterany:$ARRAY_JOB"
    --job-name fi-merge
    --output "$RUN_ROOT/logs/merge_%j.out"
    --error "$RUN_ROOT/logs/merge_%j.err"
    --export "$MERGE_EXPORTS"
)
[[ -n "$ACCOUNT" ]] && MERGE_ARGS+=(--account "$ACCOUNT")
MERGE_JOB=$(sbatch "${MERGE_ARGS[@]}" "$SCRIPT_DIR/merge_fi_once_bwunicluster.sbatch")
MERGE_JOB=${MERGE_JOB%%;*}

cat <<EOF
Submitted merge job: $MERGE_JOB
Run ID: $RUN_ID
Durable results: $RUN_ROOT

Monitor occasionally (not more often than every 30 seconds):
  squeue -j $ARRAY_JOB,$MERGE_JOB
  squeue --start -j $ARRAY_JOB
EOF
