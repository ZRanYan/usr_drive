#!/bin/bash

MOD_PATH=$1
# MODETYPE=$2
MODETYPE=0


echo "module=$MOD_PATH modeType=$MODETYPE"

sudo rmmod $MOD_PATH 2>/dev/null

sudo dmesg -c > /dev/null

echo "insert module..."

sudo insmod $MOD_PATH modeType=$MODETYPE

# echo "check param:"
# cat /sys/module/nv_imx/parameters/modeType 2>/dev/null

dmesg -c