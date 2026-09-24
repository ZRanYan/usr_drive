THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_ROOT ?= $(abspath $(THIS_DIR)/..)
$(info PROJECT_ROOT = $(PROJECT_ROOT))
include $(PROJECT_ROOT)/config.mk

CONFIG_MODULE_SIG=n

BASE_NAME := iomanager

# 根据USR_DEBUG_ENABLE变量决定模块名
ifdef USR_DEBUG_ENABLE
    $(info Building debug version with USR_DEBUG_ENABLE)
    EXTRA_CFLAGS += -DUSR_DEBUG_ENABLE
    MODULE_NAME := $(BASE_NAME)_dbg
	obj-m:= $(MODULE_NAME).o
	$(MODULE_NAME)-objs := $(BASE_NAME).o
else
    $(info Building release version)
    MODULE_NAME := $(BASE_NAME)
	obj-m:= $(MODULE_NAME).o
endif

CFLAGS_$(MODULE_NAME).o := -O1 -fno-inline

PWD:= $(shell pwd)

all : generate_header
	make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERNEL_DIR) M=$(PWD) modules
	$(call CHECK_AND_UPLOAD,./$(MODULE_NAME).ko);

mk:
	$(call CHECK_AND_UPLOAD,./mk.sh);

generate_header:compile_info.sh
	rm -f version.h
	./compile_info.sh

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.mod

