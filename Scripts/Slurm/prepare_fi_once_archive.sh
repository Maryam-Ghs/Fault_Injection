#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd -- "$SCRIPT_DIR/../.." && pwd)
ARCHIVE=${1:?Usage: $0 /durable/workspace/fault-injection-input.tar.gz}

mkdir -p "$(dirname -- "$ARCHIVE")"
TEMP_ARCHIVE="${ARCHIVE}.tmp.$$"

tar -C "$REPO_ROOT" \
    --exclude='Scripts/__pycache__' \
    --exclude='*.pyc' \
    -czf "$TEMP_ARCHIVE" \
    Code_source adad Scripts
mv -f "$TEMP_ARCHIVE" "$ARCHIVE"
sha256sum "$ARCHIVE" > "${ARCHIVE}.sha256"

echo "Created $ARCHIVE"
ls -lh "$ARCHIVE" "${ARCHIVE}.sha256"
