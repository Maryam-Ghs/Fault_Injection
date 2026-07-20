#!/usr/bin/env bash
set -euo pipefail

BIN="${1:?Usage: $0 <binary> <outdir>}"
OUTDIR="${2:?Usage: $0 <binary> <outdir>}"

filename=$(basename "$BIN")
BASENAME="${filename%.out}"
OUTCSV="$OUTDIR/${BASENAME}_fault_locations.csv"

RAW=$("$BIN" -l)

echo "id,operation,addresspath,address,fulladdress,line,discriminator,count" > "$OUTCSV"

echo "$RAW" | awk '
BEGIN { in_faults=0 }

/Fault Locations/ { in_faults=1; next }

in_faults && /^[[:space:]]*[0-9]+/ {

    id = $1
    op = $2
    count = $NF

    # Remove first 2 fields and last field to reconstruct description
    desc = $0
    sub(/^[[:space:]]*[0-9]+[[:space:]]+[^[:space:]]+[[:space:]]+/, "", desc)
    sub(/[[:space:]]+[0-9]+[[:space:]]*$/, "", desc)

    # Extract .out path
    match(desc, /(\/[^ ]+\.out)/, p)
    path = (p[1] ? p[1] : "N/A")

    # Extract relative address
    match(desc, /\(\+0x[0-9a-fA-F]+\)/, a)
    addr = (a[0] ? substr(a[0],3,length(a[0])-3) : "N/A")

    # Extract absolute address
    match(desc, /\[0x[0-9a-fA-F]+\]/, b)
    full = (b[0] ? substr(b[0],2,length(b[0])-2) : "N/A")

    print id "," op "," path "," addr "," full "," count
}
' | while IFS=, read -r id op path addr fulladdr count; do

    if [[ "$addr" != "N/A" ]]; then
        cleanaddr="${addr#+}"
        [[ "$cleanaddr" != 0x* ]] && cleanaddr="0x$cleanaddr"
        INFO=$(addr2line -C -f -p -e "$BIN" "$cleanaddr" 2>/dev/null || true)
    else
        INFO="??:0"
    fi

    line=$(echo "$INFO" | grep -oE ':[0-9]+' | tail -1 | tr -d ':' || true)
    [[ -z "$line" ]] && line="N/A"

    disc=$(echo "$INFO" | grep -oE 'discriminator [0-9]+' | awk '{print $2}' || true)
    [[ -z "$disc" ]] && disc="N/A"

    echo "$id,$op,$path,$addr,$fulladdr,$line,$disc,$count" >> "$OUTCSV"

done

echo "CSV written to $OUTCSV"


# #!/usr/bin/env bash
# set -euo pipefail

# BIN="${1:?Usage: $0 <binary> <outdir>}"
# OUTDIR="${2:?Usage: $0 <binary> <outdir>}"

# filename=$(basename "$BIN")
# BASENAME="${filename%.out}"
# OUTCSV="$OUTDIR/${BASENAME}_fault_locations.csv"

# RAW=$("$BIN" -l)

# echo "id,operation,addresspath,address,fulladdress,line,discriminator" > "$OUTCSV"

# echo "$RAW" | awk '
# BEGIN { in_faults=0 }

# /Fault Locations/ { in_faults=1; next }

# in_faults && /^[[:space:]]*[0-9]+/ {

#     id = $1

#     # Extract entire description after ID
#     desc = substr($0, index($0, $2))

#     # Operation = text before first /
#     match(desc, /^[^\/]+/, m)
#     op = (m[0] ? m[0] : "N/A")

#     # Extract .out path ONLY
#     match(desc, /(\/[^ ]+\.out)/, p)
#     path = (p[1] ? p[1] : "N/A")

#     # Extract relative address (+0x...)
#     match(desc, /\(\+0x[0-9a-f]+\)/, a)
#     addr = (a[0] ? substr(a[0],3,length(a[0])-3) : "N/A")

#     # Extract absolute address [0x...]
#     match(desc, /\[0x[0-9a-f]+\]/, b)
#     full = (b[0] ? substr(b[0],2,length(b[0])-2) : "N/A")

#     print id "," op "," path "," addr "," full
# }
# ' | while IFS=, read -r id op path addr fulladdr; do

#     # Normalize address for addr2line
#     if [[ "$addr" != "N/A" ]]; then
#         cleanaddr="${addr#+}"
#         [[ "$cleanaddr" != 0x* ]] && cleanaddr="0x$cleanaddr"
#         INFO=$(addr2line -C -f -p -e "$BIN" "$cleanaddr" 2>/dev/null || true)
#     else
#         INFO="??:0"
#     fi

#     # Extract line number
#     line=$(echo "$INFO" | grep -oE ':[0-9]+' | tail -1 | tr -d ':' || true)
#     [[ -z "$line" ]] && line="N/A"

#     # Extract discriminator
#     disc=$(echo "$INFO" | grep -oE 'discriminator [0-9]+' | awk '{print $2}' || true)
#     [[ -z "$disc" ]] && disc="N/A"

#     echo "$id,$op,$path,$addr,$fulladdr,$line,$disc" >> "$OUTCSV"

# done

# echo "CSV written to $OUTCSV"