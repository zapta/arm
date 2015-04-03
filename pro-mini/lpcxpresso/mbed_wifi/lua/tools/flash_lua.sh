#!/bin/bash

# Flash a new binary. 
# Requires connecting GPIO_0 to ground.

port="/dev/tty.usbserial-A602VGAX"
baud="9600"
file="nodemcu_20150213.bin"

echo ./esptool.py \
  --port ${port} \
  --baud ${baud} \
  write_flash 0x00000 ${file}
