#!/bin/bash

retry_udisks() {
    until udisksctl mount -b /dev/sdg1
    do sleep 2;
    done
}

echo "Waiting for A"
inotifywait -e close /dev --include sdg1 && retry_udisks && cp build/board_A.uf2 /run/media/hkj/RPI-RP2
[[ -b /dev/sdg1 ]] && inotifywait -e delete /dev --include sdg1
echo "Waiting for B"
inotifywait -e close /dev --include sdg1 && retry_udisks && cp build/board_B.uf2 /run/media/hkj/RPI-RP2