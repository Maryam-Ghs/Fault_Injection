#!/usr/bin/env bash
set -x

# Base directory
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

if [ -e $OUTPUT_CSV ]; then
    mv $OUTPUT_CSV $OUTPUT_CSV.old
fi

# how long to give each program
TIMEOUT=1

# Write CSV header
echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,CodeLines,OutLines" > "$OUTPUT_CSV"

# Find all source files recursively
# find -L "$BASE_DIR" -type f -name "*$EXT" | while read -r FILE; do
for FILE in $(find -L "$BASE_DIR" -type f -name "*$EXT"); do
    DIR=$(dirname "$FILE")
    BASENAME=$(basename "$FILE")

    # If the file is named 'main.c' or 'main.cpp', use its directory name to make it unique
    if [[ "$BASENAME" == "main.c" || "$BASENAME" == "main.cpp" || "$BASENAME" == "Prog.cpp" ]]; then
        # Use directory path (relative to BASE_DIR) as prefix
        RELDIR=${DIR#"$BASE_DIR"/}
        # Split RELDIR by '/' and take only the last 2 directories (adjust number if you want)
        SAFE_NAME=$(echo "$RELDIR" | awk -F'/' '{ if (NF>2) printf "%s_%s", $(NF-1), $NF; else print $NF }')

        # Replace possible slashes (shouldn’t exist now, but just to be safe)
        SAFE_NAME=$(echo "$SAFE_NAME" | tr '/' '_')
        BASENAME="${SAFE_NAME}.cpp"
    fi
    code_lines=$(wc -l < "$FILE")

    echo "Processing: $FILE ($code_lines lines)"
    
    # copy the file to a temporary name
    cp "$FILE" $$.cpp
    cp "$FILE" $SuccessOrigDIR/$BASENAME
    sed -i -e 's/inline//' $$.cpp
    sed -i -e 's/constexpr//' $$.cpp
    sed -i -e 's/const//' $$.cpp
    sed -i -e 's/int main(/int pain(/' $$.cpp
    sed -i -e 's/(float)/(PINT)/g' $$.cpp
    sed -i -e 's/(double)/(PINT)/g' $$.cpp
    # === Replace all int-based types with 'adad' (new improved logic) ===
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
    ' $$.cpp
    # sed -i -E 's/\badaf[[:space:]]+double\b/long double/g' $$.cpp

    # add headers and macro
    sed -i '1 i\#define PINT float' $$.cpp
    sed -i '1 i\#include "adaf.h"' $$.cpp

    # detect differences
    N=$(diff --suppress-common-lines -y "$FILE" $$.cpp | wc -l)
    if [ $N -le 3 ]; then
        rm $$.cpp
        continue
    fi

    # move to Pain_Funcs
    mv $$.cpp "${PAINDIR}/${BASENAME}"
    cd "$PAINDIR"

    set +e
    g++ -g -O0 -fno-inline -fno-omit-frame-pointer -I${ADADDIR} "$PAINDIR/$BASENAME" ${ADADDIR}/main.cpp ${ADADDIR}/adaf.cpp ${ADADDIR}/fault.cpp -lm > /dev/null 2> >(tee -a compile_errors.log >&2)
    gcc_status=$?
    set -e

    compiled="No"
    status="N/A"
    out_lines=0
    runtime=0

    if [ -e a.out ]; then
        set +e
        start=$(date +%s.%N)
        timeout --preserve-status ${TIMEOUT}s ./a.out > $$.out
        status=$?
        end=$(date +%s.%N)
        runtime=$(echo "$end - $start" | bc)
        out_lines=$(cat $$.out | wc -l)
        set -e
        compiled="Yes"
        mv "$PAINDIR/$BASENAME" "$SuccessDIR/$BASENAME"
    else
        rm -f $SuccessOrigDIR/$BASENAME
    fi

    echo "\"$compiled\",\"$gcc_status\",\"$status\",\"$runtime\",\"$BASENAME\",\"$DIR\",\"$code_lines\",\"$out_lines\"" >> "$OUTPUT_CSV"
    rm -f a.out
    cd - >/dev/null
done

