#!/usr/bin/env bash
set -euo pipefail

########################################
# Arguments (with defaults)
########################################

LINEINFODIR="${1:-/localdata/Mary/Workspace_2/SDC/Scripts/line_info.sh}"
OUTPUT="${2:-/localdata/Mary/Workspace_2/SDC/Scripts/report_FaultList.csv}"
ADADDIR="${3:-/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/}"
SuccessDIR="${4:-/localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/}"
CompiledDIR="${5:-/localdata/Mary/Workspace_2/SDC/Output/Compiled_Out/}"
FaultReportDIR="${6:-/localdata/Mary/Workspace_2/SDC/Output/Fault_Reports/}"

TIMEOUT=5
CORES=$(nproc)

mkdir -p "$CompiledDIR" "$FaultReportDIR"

rm -f "$CompiledDIR"/*
rm -f "$FaultReportDIR"/*

if [ -f "$OUTPUT" ]; then
    mv "$OUTPUT" "$OUTPUT.old"
fi

TMP_CSV_DIR=$(mktemp -d)

########################################
# Function to process ONE file
########################################

process_file() {

    FILE="$1"
    FULLPATH="$SuccessDIR/$FILE"
    BASENAME="${FILE%.cpp}"

    file_size=$(wc -c < "$FULLPATH")

    TMP_EXE=$(mktemp /tmp/faultXXXX.out)
    TMP_CSV=$(mktemp "$TMP_CSV_DIR/resultXXXX.csv")

    ########################################
    # Compile (NO MORE a.out collisions)
    ########################################

    set +e
    g++ -g -O0 -pedantic -Wall \
        -I"$ADADDIR" \
        "$FULLPATH" \
        "$ADADDIR/main.cpp" \
        "$ADADDIR/adad.cpp" \
        "$ADADDIR/fault.cpp" \
        -lm -o "$TMP_EXE"
    gcc_status=$?
    set -e

    compiled="No"
    status="N/A"
    runtime=0
    out_lines=0

    ########################################
    # Run if compiled
    ########################################

    if [ "$gcc_status" -eq 0 ]; then
        compiled="Yes"

        FINAL_EXE="$CompiledDIR/${BASENAME}.out"
        mv "$TMP_EXE" "$FINAL_EXE"

        start=$(date +%s.%N)
        set +e
        timeout --preserve-status ${TIMEOUT}s \
            bash "$LINEINFODIR" "$FINAL_EXE" "$FaultReportDIR"
        status=$?
        set -e
        end=$(date +%s.%N)

        runtime=$(echo "$end - $start" | bc)

        if [ -f "${FINAL_EXE}.list" ]; then
            out_lines=$(wc -l < "${FINAL_EXE}.list")
        fi
    fi

    ########################################
    # Safe CSV write (per-process file)
    ########################################

    echo "\"$compiled\",\"$gcc_status\",\"$status\",\"$runtime\",\"$FILE\",\"$SuccessDIR\",\"$file_size\",\"$out_lines\"" > "$TMP_CSV"
}

export -f process_file
export ADADDIR SuccessDIR CompiledDIR FaultReportDIR LINEINFODIR TIMEOUT TMP_CSV_DIR

########################################
# Run in parallel
########################################

echo "Running on $CORES cores..."

cd "$SuccessDIR"

ls *.cpp 2>/dev/null | \
xargs -P "$CORES" -I{} bash -c 'process_file "$@"' _ {}

########################################
# Merge CSV safely
########################################

echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,FileSize,OutLines" > "$OUTPUT"
cat "$TMP_CSV_DIR"/*.csv >> "$OUTPUT"

rm -rf "$TMP_CSV_DIR"

echo "Done."