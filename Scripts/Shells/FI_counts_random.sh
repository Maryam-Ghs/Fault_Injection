#!/usr/bin/env bash
# set -euo pipefail

# Usage: ./FI.sh <input_csv> <TRY> <FICSV> <OUTTXT>
INCSV="${1:?Usage: $0 <input_csv> <TRY> <FICSV> <OUTTXT>}"
TRY="${2:-10}"
FALT_PROB="${3:-0.5}"
FICSV="${4:-"/localdata/Mary/Workspace_2/SDC/Output/FI_Reports/"}"
OUTTXT="${5:-"/localdata/Mary/Workspace_2/SDC/Output/FI_TXT/"}"

# Extract executable path from CSV (we assume addresspath column contains path without .out)
EXEC=$(awk -F, 'NR==2 {print $3}' "$INCSV")
BASENAME=$(basename "$EXEC" .out)
TIMEOUT_DURATION=5

echo "Analysing $INCSV, fault injection on $BASENAME with executable:$EXEC, TRY=$TRY and FALT_PROB=$FALT_PROB"

# Golden run
GOLD_OUT="${OUTTXT}golden_${BASENAME}.txt"
if ! "$EXEC" >"$GOLD_OUT" 2>/dev/null; then
    echo "Golden run failed for $BASENAME" >&2
    exit 1
fi
GOLD_LINES=$(wc -l < "$GOLD_OUT")

# Prepare output CSV
OUTCSV="${FICSV}${BASENAME}_results.csv"
header=$(head -1 "$INCSV")
echo "${header},crash_count,timeout_count,matched_count,notmatched_count" > "$OUTCSV"

# Iterate over input rows (skip header)
tail -n +2 "$INCSV" | while IFS=, read -r id op op_path addr fulladdr line disc count; do
    crash=0
    timeout=0
    matched=0
    notmatched=0

    echo "id: $id"

    for ((t=1; t<=TRY; t++)); do
        OUTFILE="${OUTTXT}run_${BASENAME}_${id}_${t}.txt"
        ERRFILE="${OUTTXT}run_${BASENAME}_${id}_${t}.ERR"

        MAXSEED=2147483647
        BIGSEED=$(date +%s%N)
        SEED=$(( BIGSEED % MAXSEED ))
        echo "SEED=$SEED"
        
        cycle=$(( SEED % count + 1 ))
        # Run with timeout: "$EXEC" -p "${id},${FALT_PROB}" -s "$SEED" >"$OUTFILE" 2>"$ERRFILE"; then
        if ! timeout $TIMEOUT_DURATION "$EXEC" -p "${id},${FALT_PROB}" -s "$SEED" -c "$cycle" >"$OUTFILE" 2>"$ERRFILE"; then
            status=$?
            echo "Run failed or timed out with status $status">>"$ERRFILE"
            if [[ $status -eq 124 ]]; then
                ((timeout++))
            else
                ((crash++))
            fi
            continue
        fi

        lines=$(wc -c < "$OUTFILE")
        if [[ "$lines" -eq 0 ]]; then
        echo "Run produced empty output"
            # empty output, treat as crash-like
            ((crash++))
            continue
        fi

        if diff -q "$OUTFILE" "$GOLD_OUT" >/dev/null; then
            echo "$BASENAME, Try $t: Output matches golden run"
            ((matched++))
        else
            echo "$BASENAME, Try $t: Output does not match golden run"
            ((notmatched++))
        fi

        # Cleanup
        rm -f "$OUTFILE" "$ERRFILE"
    done
    echo "$id,$op,$op_path,$addr,$fulladdr,$line,$disc,$count,$crash,$timeout,$matched,$notmatched" >> "$OUTCSV"


    
done

echo "Results written to $OUTCSV"

