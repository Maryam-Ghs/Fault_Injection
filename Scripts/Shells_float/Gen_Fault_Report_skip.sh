#!/usr/bin/env bash

set -u  # safer: undefined vars cause error

############################################
# CONFIG
############################################

FIDIR="${1:-/localdata/Mary/Workspace_2/SDC/Scripts/FI.sh}"
TRY="${2:-10}"
FALT_PROB="${3:-0.5}"
FaultReportDIR="${4:-/localdata/Mary/Workspace_2/SDC/Output/Fault_Reports/}"
FICSV="${5:-/localdata/Mary/Workspace_2/SDC/Output/FI_Reports/}"
OUTTXT="${6:-/localdata/Mary/Workspace_2/SDC/Output/FI_TXT/}"
FALT_TYPE="${7:-0}"

TIMEOUT=1
NC=8   # parallel jobs

BASE_DIR="/localdata/Mary/Workspace_2/SDC/Output"
DONE_DIR="$BASE_DIR/done"
LOG_DIR="$BASE_DIR/logs"
FAIL_DIR="$BASE_DIR/failed"

mkdir -p "$DONE_DIR" "$LOG_DIR" "$FAIL_DIR"

GLOBAL_LOG="$LOG_DIR/global.log"

############################################
# CLEAN OUTPUT DIRECTORIES (optional)
############################################
find -L "$FICSV" -type f -delete
find -L "$OUTTXT" -type f -delete

############################################
# SIGNAL HANDLING
############################################
echo "Starting run at $(date)" | tee -a "$GLOBAL_LOG"

trap 'echo "Interrupted! Exiting..."; exit 1' SIGINT SIGTERM

############################################
# FUNCTION: RUN ONE FILE
############################################
run_file() {
    local FILE="$1"
    local BASENAME
    BASENAME=$(basename "$FILE")

    local LOG_FILE="$LOG_DIR/$BASENAME.log"

    echo "[$(date)] START $FILE" | tee -a "$GLOBAL_LOG"

    #timeout --preserve-status ${TIMEOUT}s \
        bash "$FIDIR" "$FILE" "$TRY" "$FALT_PROB" "$FICSV" "$OUTTXT" "$FALT_TYPE" \
        > "$LOG_FILE" 2>&1

    local STATUS=$?

    if [[ $STATUS -eq 0 ]]; then
        echo "[$(date)] DONE $FILE" | tee -a "$GLOBAL_LOG"
        touch "$DONE_DIR/$BASENAME.done"
    elif [[ $STATUS -eq 124 ]]; then
        echo "[$(date)] TIMEOUT $FILE" | tee -a "$GLOBAL_LOG"
        touch "$FAIL_DIR/$BASENAME.timeout"
    else
        echo "[$(date)] FAIL $FILE (status=$STATUS)" | tee -a "$GLOBAL_LOG"
        touch "$FAIL_DIR/$BASENAME.fail"
    fi
}

############################################
# MAIN LOOP
############################################

cd "$FaultReportDIR" || exit 1

N=0

for FILE in *.csv; do
    BASENAME=$(basename "$FILE")

    # Skip already completed
    if [[ -f "$DONE_DIR/$BASENAME.done" || \
        -f "$FAIL_DIR/$BASENAME.fail" || \
        -f "$FAIL_DIR/$BASENAME.timeout" ]]; then
        echo "Skipping $FILE (already processed)"
        continue
    fi

    # Launch in background
    run_file "$FILE" &

    ((N++))

    # Control parallel jobs
    if [[ $N -ge $NC ]]; then
        wait
        N=0
    fi
done

# Wait for remaining jobs
wait

echo "Finished all jobs at $(date)" | tee -a "$GLOBAL_LOG"