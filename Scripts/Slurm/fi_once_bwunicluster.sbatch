#!/usr/bin/env bash
#SBATCH --job-name=fi-once
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --threads-per-core=1
#SBATCH --signal=B:USR1@300
#SBATCH --requeue

set -euo pipefail

: "${FI_ARCHIVE:?FI_ARCHIVE must point to the prepared input archive}"
: "${FI_RESULT_ROOT:?FI_RESULT_ROOT must be a durable workspace directory}"
: "${FI_RUN_ID:?FI_RUN_ID is required}"
: "${FI_SHARD_COUNT:?FI_SHARD_COUNT is required}"
: "${FI_SEED_START:?FI_SEED_START is required}"
: "${FI_CAMPAIGN:=both}"
: "${TMPDIR:?bwUniCluster must provide node-local TMPDIR}"
: "${SLURM_ARRAY_TASK_ID:?Submit this worker as a Slurm array}"

SEED_SLOT=$((SLURM_ARRAY_TASK_ID / FI_SHARD_COUNT))
SHARD_INDEX=$((SLURM_ARRAY_TASK_ID % FI_SHARD_COUNT))
SEED=$((FI_SEED_START + SEED_SLOT))

LOCAL_ROOT="$TMPDIR/fi_once_${SLURM_JOB_ID}_${SLURM_ARRAY_TASK_ID}"
LOCAL_REPO="$LOCAL_ROOT/repo"
LOCAL_OUTPUT="$LOCAL_ROOT/output"
DURABLE_DIR="$FI_RESULT_ROOT/$FI_RUN_ID/shards/seed_${SEED}/shard_${SHARD_INDEX}"

mkdir -p "$LOCAL_REPO" "$LOCAL_OUTPUT" "$DURABLE_DIR"

snapshot_csvs() {
    local source_file base temporary
    for base in results.csv program_status.csv; do
        source_file="$LOCAL_OUTPUT/$base"
        if [[ -s "$source_file" ]]; then
            temporary="$DURABLE_DIR/.${base}.${SLURM_JOB_ID}.tmp"
            cp -f "$source_file" "$temporary"
            mv -f "$temporary" "$DURABLE_DIR/$base"
        fi
    done
}

save_artifacts() {
    local items=() archive_tmp
    for item in FI_Reports locations compile_logs fault_logs golden; do
        [[ -e "$LOCAL_OUTPUT/$item" ]] && items+=("$item")
    done
    if ((${#items[@]})); then
        archive_tmp="$DURABLE_DIR/.artifacts.tar.gz.${SLURM_JOB_ID}.tmp"
        tar -C "$LOCAL_OUTPUT" -czf "$archive_tmp" "${items[@]}"
        mv -f "$archive_tmp" "$DURABLE_DIR/artifacts.tar.gz"
    fi
}

on_warning() {
    echo "Received pre-timeout signal; snapshotting CSVs" >&2
    snapshot_csvs
}
trap on_warning USR1
trap 'snapshot_csvs; exit 143' TERM

finish() {
    local status=$?
    trap - EXIT
    snapshot_csvs
    save_artifacts
    {
        echo "job_id=$SLURM_JOB_ID"
        echo "array_task_id=$SLURM_ARRAY_TASK_ID"
        echo "node=${SLURMD_NODENAME:-unknown}"
        echo "partition=${SLURM_JOB_PARTITION:-unknown}"
        echo "seed=$SEED"
        echo "shard_index=$SHARD_INDEX"
        echo "shard_count=$FI_SHARD_COUNT"
        echo "campaign=$FI_CAMPAIGN"
        echo "exit_status=$status"
        date --iso-8601=seconds
    } > "$DURABLE_DIR/job_manifest.txt"
    exit "$status"
}
trap finish EXIT

# Restore the latest durable CSV snapshot when a shard is resubmitted/requeued.
for base in results.csv program_status.csv; do
    [[ -s "$DURABLE_DIR/$base" ]] && cp -f "$DURABLE_DIR/$base" "$LOCAL_OUTPUT/$base"
done

cp "$FI_ARCHIVE" "$LOCAL_ROOT/input.tar.gz"
if [[ -s "${FI_ARCHIVE}.sha256" ]]; then
    expected=$(awk '{print $1}' "${FI_ARCHIVE}.sha256")
    actual=$(sha256sum "$LOCAL_ROOT/input.tar.gz" | awk '{print $1}')
    [[ "$expected" == "$actual" ]] || { echo "Input archive checksum mismatch" >&2; exit 2; }
fi
tar -C "$LOCAL_REPO" -xzf "$LOCAL_ROOT/input.tar.gz"

echo "Starting seed=$SEED shard=$SHARD_INDEX/$FI_SHARD_COUNT on ${SLURMD_NODENAME:-unknown}"
"$LOCAL_REPO/Scripts/Run_FI_once.sh" \
    --campaign "$FI_CAMPAIGN" \
    --occurrence random \
    --seed "$SEED" \
    --source-dir "$LOCAL_REPO/Code_source/generated_codes" \
    --output-dir "$LOCAL_OUTPUT" \
    --shard-count "$FI_SHARD_COUNT" \
    --shard-index "$SHARD_INDEX" \
    --resume &
CAMPAIGN_PID=$!
CAMPAIGN_STATUS=0

# A trapped USR1 can interrupt wait while the campaign is still alive. Resume
# waiting after the CSV snapshot instead of mistaking the warning for failure.
while kill -0 "$CAMPAIGN_PID" 2>/dev/null; do
    if wait "$CAMPAIGN_PID"; then
        CAMPAIGN_STATUS=0
        break
    else
        CAMPAIGN_STATUS=$?
        kill -0 "$CAMPAIGN_PID" 2>/dev/null || break
    fi
done
exit "$CAMPAIGN_STATUS"
