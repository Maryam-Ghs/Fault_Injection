#!/usr/bin/env bash
set -euo pipefail

########################################
# Arguments (with defaults)
########################################

BASE_DIR="${1:-/localdata/Mary/Workspace_2/SDC/Code_source/rosetta/RosettaCodeData/Lang/C}"
EXT="${2:-.c}"
OUTPUT_CSV="${3:-/localdata/Mary/Workspace_2/SDC/Scripts/report_int_adad.csv}"
ADADDIR="${4:-/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/}"
PAINDIR="${5:-/localdata/Mary/Workspace_2/SDC/Output/Pain_Funcs/}"
SuccessDIR="${6:-/localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/}"
SuccessOrigDIR="${7:-/localdata/Mary/Workspace_2/SDC/Output/Codes_Orig/}"

TIMEOUT=1
#CORES=$(nproc)
CORES=8

########################################
# Prepare directories
########################################

mkdir -p "$PAINDIR" "$SuccessDIR" "$SuccessOrigDIR"

rm -f "$PAINDIR"/*
rm -f "$SuccessDIR"/*
rm -f "$SuccessOrigDIR"/*

if [ -f "$OUTPUT_CSV" ]; then
    mv "$OUTPUT_CSV" "$OUTPUT_CSV.old"
fi

TMP_CSV_DIR=$(mktemp -d)

########################################
# Function to process ONE file
########################################

process_file() {

    FILE="$1"
    DIR=$(dirname "$FILE")
    BASENAME=$(basename "$FILE")

    code_lines=$(wc -l < "$FILE")

    TMP_CPP=$(mktemp /tmp/adadXXXX.cpp)
    TMP_EXE=$(mktemp /tmp/adadXXXX.out)
    TMP_OUT=$(mktemp /tmp/adadXXXX.run)
    TMP_CSV=$(mktemp "$TMP_CSV_DIR/resultXXXX.csv")

    cp "$FILE" "$TMP_CPP"
    cp "$FILE" "$SuccessOrigDIR/$BASENAME"

    ########################################
    # Transformations
    ########################################

    sed -i -e 's/inline//' "$TMP_CPP"
    sed -i -e 's/int main(/PINT pain(/' "$TMP_CPP"
    sed -i -e 's/(int)/(PINT)/g' "$TMP_CPP"

    sed -i -E '
      s/\bunsigned[[:space:]]+long[[:space:]]+long[[:space:]]+int\b/adad/g;
      s/\bunsigned[[:space:]]+long[[:space:]]+long\b/adad/g;
      s/\blong[[:space:]]+long[[:space:]]+int\b/adad/g;
      s/\blong[[:space:]]+long\b/adad/g;
      s/\bunsigned[[:space:]]+long[[:space:]]+int\b/adad/g;
      s/\bunsigned[[:space:]]+long\b/adad/g;
      s/\bunsigned[[:space:]]+int\b/adad/g;
      s/\bunsigned\b/adad/g;
      s/\blong[[:space:]]+int\b/adad/g;
      s/\bint\b/adad/g;
    ' "$TMP_CPP"

    sed -i -E 's/\badad[[:space:]]+double\b/long double/g' "$TMP_CPP"

    sed -i '1 i\#define PINT int' "$TMP_CPP"
    sed -i '1 i\#include "adad.h"' "$TMP_CPP"

    ########################################
    # Detect differences
    ########################################

    N=$(diff --suppress-common-lines -y "$FILE" "$TMP_CPP" | wc -l)

    if [ "$N" -le 3 ]; then
        rm -f "$TMP_CPP" "$TMP_EXE" "$TMP_OUT"
        rm -f "$SuccessOrigDIR/$BASENAME"
        return
    fi

    ########################################
    # Compile
    ########################################

    set +e
    g++ -g -O0 -fno-inline -fno-omit-frame-pointer \
        -I"$ADADDIR" \
        "$TMP_CPP" \
        "$ADADDIR/main.cpp" \
        "$ADADDIR/adad.cpp" \
        "$ADADDIR/fault.cpp" \
        -lm -o "$TMP_EXE" > /dev/null 2>&1
    gcc_status=$?
    set -e

    compiled="No"
    run_status="N/A"
    runtime=0
    out_lines=0

    ########################################
    # Run if compiled
    ########################################

    if [ "$gcc_status" -eq 0 ]; then
        compiled="Yes"

        start=$(date +%s.%N)
        set +e
        timeout --preserve-status ${TIMEOUT}s "$TMP_EXE" > "$TMP_OUT"
        run_status=$?
        set -e
        end=$(date +%s.%N)

        runtime=$(echo "$end - $start" | bc)
        out_lines=$(wc -l < "$TMP_OUT")

        mv "$TMP_CPP" "$SuccessDIR/$BASENAME"
    else
        rm -f "$SuccessOrigDIR/$BASENAME"
    fi

    ########################################
    # Write result (SAFE)
    ########################################

    echo "\"$compiled\",\"$gcc_status\",\"$run_status\",\"$runtime\",\"$BASENAME\",\"$DIR\",\"$code_lines\",\"$out_lines\"" > "$TMP_CSV"

    rm -f "$TMP_EXE" "$TMP_OUT"
}

export -f process_file
export ADADDIR TIMEOUT SuccessDIR SuccessOrigDIR TMP_CSV_DIR

########################################
# Run in parallel
########################################

echo "Running with $CORES cores..."

find "$BASE_DIR" -type f -name "*$EXT" | \
xargs -P "$CORES" -I{} bash -c 'process_file "$@"' _ {}

########################################
# Merge CSV safely
########################################

echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,CodeLines,OutLines" > "$OUTPUT_CSV"
cat "$TMP_CSV_DIR"/*.csv >> "$OUTPUT_CSV"

rm -rf "$TMP_CSV_DIR"

echo "Done."