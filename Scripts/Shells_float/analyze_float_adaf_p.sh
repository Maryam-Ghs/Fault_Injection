#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="${1:-"/localdata/Mary/Workspace_2/SDC/Code_source/rosetta/RosettaCodeData/Lang/C"}"
EXT="${2:-".c"}"
OUTPUT_CSV="${3:-"/localdata/Mary/Workspace_2/SDC/Scripts/report_float_adad.csv"}"
ADADDIR="${4:-"/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/"}"
PAINDIR="${5:-"/localdata/Mary/Workspace_2/SDC/Output/Pain_Funcs/"}"
SuccessDIR="${6:-"/localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/"}"
SuccessOrigDIR="${7:-"/localdata/Mary/Workspace_2/SDC/Output/Codes_Orig/"}"

#empty folders
#find -L $SDCDIR -type f -name "*.cpp" -exec rm -f {} +
find -L $PAINDIR -type f -name "*" -exec rm -f {} +
find -L $SuccessDIR -type f -name "*" -exec rm -f {} +
find -L $SuccessOrigDIR -type f -name "*" -exec rm -f {} +

TIMEOUT=1
NPROC=8   # number of parallel jobs (adjust!)

TMP_DIR=$(mktemp -d)

echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,CodeLines,OutLines" > "$OUTPUT_CSV"

process_file() {
    FILE="$1"

    DIR=$(dirname "$FILE")
    BASENAME=$(basename "$FILE")

    WORKDIR=$(mktemp -d)
    TMP_CPP="$WORKDIR/tmp.cpp"
    TMP_OUT="$WORKDIR/tmp.out"
    TMP_CSV="$WORKDIR/result.csv"

    cp "$FILE" "$TMP_CPP"
    cp "$FILE" "$SuccessOrigDIR/$BASENAME"

    sed -i -e 's/inline//' "$TMP_CPP"
    sed -i -e 's/constexpr//' "$TMP_CPP"
    sed -i -e 's/const//' "$TMP_CPP"
    sed -i -e 's/int main(/int pain(/' "$TMP_CPP"
    sed -i -e 's/(float)/(PINT)/g' "$TMP_CPP"
    sed -i -e 's/(double)/(PINT)/g' "$TMP_CPP"

    sed -i '1 i\#define PINT float' "$TMP_CPP"
    sed -i '1 i\#include "adaf.h"' "$TMP_CPP"

    sed -i -E '
      s/\bunsigned[[:space:]]+long[[:space:]]+long[[:space:]]+float\b/adaf/g;
      s/\blong[[:space:]]+long[[:space:]]+float\b/adaf/g;
      s/\bunsigned[[:space:]]+long[[:space:]]+float\b/adaf/g;
      s/\bunsigned[[:space:]]+float\b/adaf/g;
      s/\blong[[:space:]]+float\b/adaf/g;
      s/\bfloat\b/adaf/g;
      s/\bunsigned[[:space:]]+long[[:space:]]+long[[:space:]]+double\b/adaf/g;
      s/\blong[[:space:]]+long[[:space:]]+double\b/adaf/g;
      s/\bunsigned[[:space:]]+long[[:space:]]+double\b/adaf/g;
      s/\bunsigned[[:space:]]+double\b/adaf/g;
      s/\blong[[:space:]]+double\b/adaf/g;
      s/\bdouble\b/adaf/g;
    ' "$TMP_CPP"

    code_lines=$(wc -l < "$FILE")

    g++ -g -O0 -fno-inline -fno-omit-frame-pointer \
        -I${ADADDIR} "$TMP_CPP" \
        ${ADADDIR}/main.cpp ${ADADDIR}/adaf.cpp ${ADADDIR}/fault.cpp \
        -lm -o "$WORKDIR/a.out" &>/dev/null

    gcc_status=$?
    compiled="No"
    status="N/A"
    runtime=0
    out_lines=0

    if [ -f "$WORKDIR/a.out" ]; then
        start=$(date +%s.%N)
        timeout ${TIMEOUT}s "$WORKDIR/a.out" > "$TMP_OUT"
        status=$?
        end=$(date +%s.%N)

        runtime=$(echo "$end - $start" | bc)
        out_lines=$(wc -l < "$TMP_OUT")

        compiled="Yes"
        mv "$TMP_CPP" "$SuccessDIR/$BASENAME"
    else
        rm -f "$SuccessOrigDIR/$BASENAME"
    fi

    echo "\"$compiled\",\"$gcc_status\",\"$status\",\"$runtime\",\"$BASENAME\",\"$DIR\",\"$code_lines\",\"$out_lines\"" > "$TMP_CSV"

    cat "$TMP_CSV"
    rm -rf "$WORKDIR"
}

export -f process_file
export ADADDIR TIMEOUT SuccessDIR SuccessOrigDIR

find -L "$BASE_DIR" -type f -name "*$EXT" | \
    xargs -P $NPROC -I{} bash -c 'process_file "$@"' _ {} \
    >> "$OUTPUT_CSV"