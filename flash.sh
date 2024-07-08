#!/bin/bash
# Flash via openocd
if [[ $1 == a ]]; then
    bin_file=build/board_A.bin
elif [[ $1 == b ]]; then
    bin_file=build/board_B_boot3.bin
else
    exit 1
fi

cmd="program $bin_file 0x10000000 verify"

if [[ $1 == b ]]; then
    # Verify app image for checksum
    cmd="$cmd; init; halt; flash verify_image build/board_B_app_img.bin 0x10004000; exit"
else
    cmd="$cmd; exit"
fi

echo $cmd
openocd -f openocd.cfg -c "$cmd"