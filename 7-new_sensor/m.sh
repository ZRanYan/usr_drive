#!/bin/bash

set -e

remove_module_if_exists() {
    local mod="$1"
    if lsmod | grep -w "^${mod}" > /dev/null 2>&1; then
        echo "Removing module: $mod"
        rmmod "$mod"
    else
        echo "Module $mod not loaded, skip"
    fi
}
remove_module_if_exists nv_imx_dbg
remove_module_if_exists max96724_dbg
dmesg -c
insmod max96724_dbg.ko
sleep 1
insmod nv_imx_dbg.ko
dmesg -c




