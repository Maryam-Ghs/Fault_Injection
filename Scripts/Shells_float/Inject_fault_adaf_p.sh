#!/usr/bin/env bash
set -euo pipefail

LINEINFODIR="${1:-"/localdata/Mary/Workspace_2/SDC/Scripts/line_info.sh"}"
OUTPUT="${2:-"/localdata/Mary/Workspace_2/SDC/Scripts/report_FaultList.csv"}"
ADADDIR="${3:-"/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/"}"
SuccessDIR="${4:-"/localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/"}"
CompiledDIR="${5:-"/localdata/Mary/Workspace_2/SDC/Output/Compiled_Out/"}"
FaultReportDIR="${6:-"/localdata/Mary/Workspace_2/SDC/Output/Fault_Reports/"}"

#empty folders
find -L $CompiledDIR -type f -name "*" -exec rm -f {} +
find -L $FaultReportDIR -type f -name "*" -exec rm -f {} +

TIMEOUT=5
NPROC=8   # adjust to your CPU

# clean output dirs
find -L "$CompiledDIR" -type f -delete
find -L "$FaultReportDIR" -type f -delete

[ -e "$OUTPUT" ] && mv "$OUTPUT" "$OUTPUT.old"

echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,FileSize,OutLines" > "$OUTPUT"

process_file() {
    FILE="$1"

    BASENAME=$(basename "$FILE")
    FILEPATH="$SuccessDIR/$BASENAME"

    WORKDIR=$(mktemp -d)
    BIN="$WORKDIR/a.out"
    TMP_OUT="$WORKDIR/run.out"
    TMP_CSV="$WORKDIR/result.csv"

    file_size=$(wc -c < "$FILEPATH")

    # compile
    g++ -g -O0 -pedantic -Wall -I${ADADDIR} \
        "$FILEPATH" \
        ${ADADDIR}/main.cpp ${ADADDIR}/adaf.cpp ${ADADDIR}/fault.cpp \
        -lm -o "$BIN" &>/dev/null

    gcc_status=$?
    compiled="No"
    status="N/A"
    runtime=0
    out_lines=0

    if [ -f "$BIN" ]; then
        compiled="Yes"

        # unique output names to avoid collision
        SAFE_NAME="${BASENAME%.cpp}_$$"

        OUTFILE="$CompiledDIR/${SAFE_NAME}.out"
        LISTFILE="$CompiledDIR/${SAFE_NAME}.list"

        mv "$BIN" "$OUTFILE"

        start=$(date +%s.%N)
        timeout ${TIMEOUT}s bash "$LINEINFODIR" "$OUTFILE" "$FaultReportDIR" > "$TMP_OUT"
        status=$?
        end=$(date +%s.%N)

        runtime=$(echo "$end - $start" | bc)

        if [ -f "$LISTFILE" ]; then
            out_lines=$(wc -l < "$LISTFILE")
        fi
    fi

    echo "\"$compiled\",\"$gcc_status\",\"$status\",\"$runtime\",\"$BASENAME\",\"$SuccessDIR\",\"$file_size\",\"$out_lines\"" > "$TMP_CSV"

    cat "$TMP_CSV"
    rm -rf "$WORKDIR"
}

export -f process_file
export ADADDIR LINEINFODIR TIMEOUT CompiledDIR FaultReportDIR SuccessDIR

find "$SuccessDIR" -maxdepth 1 -name "*.cpp" | \
    xargs -P $NPROC -I{} bash -c 'process_file "$@"' _ {} \
    >> "$OUTPUT"