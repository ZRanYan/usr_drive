
SDK_ROOT := ~/r36.4.4
SDK_GCC := ~/14t-bin

TARGET_IP ?= 192.168.2.188

USR := bp#自定上传到 /home/$(USR)/nfs 文件夹里面
PASSWORD := Cr123789

export TARGET_IP USR PASSWORD

# ssh-keygen -f "/home/yz/.ssh/known_hosts" -R "192.168.2.188"

CROSS_COMPILER := $(SDK_GCC)/bin/aarch64-buildroot-linux-gnu-
KERNEL_DIR := $(SDK_ROOT)/Linux_for_Tegra/source/kernel/kernel-jammy-src
EXTRA_CFLAGS += -I$(SDK_ROOT)/Linux_for_Tegra/source/nvidia-oot/include
EXTRA_CFLAGS += -I$(SDK_ROOT)/Linux_for_Tegra/source/out/nvidia-conftest
KBUILD_EXTRA_SYMBOLS := $(SDK_ROOT)/Linux_for_Tegra/source/nvidia-oot/Module.symvers
