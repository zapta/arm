#!/bin/bash
#
# A shell script to copy the generated binary image to the ISP virtual USB disk.
# Can be added to lpcxpresso as an external tool.

cp ../Debug/hello_world.bin /Volumes/CRP\ DISABLD/firmware.bin 
ls -al /Volumes/CRP\ DISABLD
