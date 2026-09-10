#!/data/data/com.termux/files/usr/bin/bash
set -e
OUT="/sdcard/BTP-log.txt"
echo "BTP debug logger"
echo "Saving to: $OUT"
echo "Press Ctrl+C to stop."
logcat -c || true
logcat -v time -s BTP:* | tee "$OUT"
