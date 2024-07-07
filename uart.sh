#!/bin/bash
while true; do inotifywait /dev/serial/by-id; screen /dev/serial/by-id/usb-Hrvoje_Cavrak_DeskHop_Switch_0-if02 921600; done
