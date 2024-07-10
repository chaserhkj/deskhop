#!/usr/bin/env python3
import hid
from connect_bootloader import flash, write, send, stop
import time

VID=0x2e8a
PID=0x107c

dev = hid.Device(VID, PID)
# Probe if forwarder is up
try:
    print("Waiting for the device...", end="", flush=True)
    while True:
        try:
            print(".", end="", flush=True)
            write(dev, b'SYNC')
            print("Device found!")
            break
        except Exception as e:
            time.sleep(1)
except KeyboardInterrupt:
    exit(1)

# flash
flash(dev)

# reboot and stop forwarder
payload = b'BOOT\x00\x00\x00\x00'
ck = write(dev, payload)
send(dev, ck)

stop(dev)