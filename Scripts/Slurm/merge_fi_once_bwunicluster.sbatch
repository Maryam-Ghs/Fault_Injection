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
: "${FI_SEED_START:?}"
: "${FI_SEED_COUNT:?}"

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
python3 "$SCRIPT_DIR/merge_fi_once_shards.py" \
    --run-root "$FI_RESULT_ROOT/$FI_RUN_ID" \
    --shard-count "$FI_SHARD_COUNT" \
    --seed-start "$FI_SEED_START" \
    --seed-count "$FI_SEED_COUNT"
