#!/bin/bash

sudo cp -f 99-camera.rules /etc/udev/rules.d/99-camera.rules
sudo udevadm control --reload-rules
sudo udevadm trigger

