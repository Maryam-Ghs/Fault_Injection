#!/usr/bin/env bash
#SBATCH --job-name=fi-merge
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --threads-per-core=1

set -euo pipefail
: "${FI_RESULT_ROOT:?}"
: "${FI_RUN_ID:?}"
: "${FI_SHARD_COUNT:?}"
: "${FI_SEEDS:?}"
: "${FI_SLURM_SCRIPT_DIR:?FI_SLURM_SCRIPT_DIR must be the real Scripts/Slurm directory}"

SCRIPT_DIR=$FI_SLURM_SCRIPT_DIR
[[ -f "$SCRIPT_DIR/merge_fi_once_shards.py" ]] || {
    echo "Merge helper not found: $SCRIPT_DIR/merge_fi_once_shards.py" >&2
    exit 2
}
python3 "$SCRIPT_DIR/merge_fi_once_shards.py" \
    --run-root "$FI_RESULT_ROOT/$FI_RUN_ID" \
    --shard-count "$FI_SHARD_COUNT" \
    --seed-list "$FI_SEEDS"
