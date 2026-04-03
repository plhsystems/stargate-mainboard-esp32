#!/bin/bash
# Send OTA firmware update to pinky-board at http://192.168.5.98/
# Usage: ./send-ota.sh [path/to/firmware.bin]
#   If no argument is given, defaults to pinky-board/build/stargate-fw.bin

set -e

BOARD_URL="http://192.168.5.98/ota/upload"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_BIN="$SCRIPT_DIR/pinky-board/build/stargate-fw.bin"
BIN="${1:-$DEFAULT_BIN}"

if [ ! -f "$BIN" ]; then
    echo "Error: firmware binary not found: $BIN"
    echo "Build first with: cd pinky-board && idf.py build"
    exit 1
fi

echo "Sending OTA update..."
echo "  Target : $BOARD_URL"
echo "  Binary : $BIN ($(du -h "$BIN" | cut -f1))"

curl --silent --show-error --fail \
    --request POST \
    --header "Content-Type: application/octet-stream" \
    --data-binary "@$BIN" \
    "$BOARD_URL" && echo "OTA upload complete. Device will reboot."
