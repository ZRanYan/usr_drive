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

remove_module_if_exists dlp3010
remove_module_if_exists nv_imx
remove_module_if_exists iomanager
remove_module_if_exists temp

dmesg -c

insmod iomanager.ko
sleep 1

insmod dlp3010.ko
sleep 1

insmod nv_imx.ko
sleep 1
insmod temp.ko

dmesg -c



