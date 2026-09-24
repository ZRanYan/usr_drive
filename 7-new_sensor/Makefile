THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_ROOT ?= $(abspath $(THIS_DIR)/..)
# $(info PROJECT_ROOT = $(PROJECT_ROOT))
include $(PROJECT_ROOT)/config.mk

CONFIG_MODULE_SIG=n

JOBS := $(shell nproc)
# $(info BUILD JOBS = $(JOBS))

# 根据USR_DEBUG_ENABLE变量决定模块名
ifdef USR_DEBUG_ENABLE
#     $(info Building debug version with USR_DEBUG_ENABLE)
    EXTRA_CFLAGS += -DUSR_DEBUG_ENABLE
    MODULE_NAME := nv_imx_dbg
	MODULE_MAX := max96724_dbg
else
#     $(info Building release version)
    MODULE_NAME := nv_imx
	MODULE_MAX := max96724
endif

obj-m := $(MODULE_NAME).o
obj-m += $(MODULE_MAX).o

$(MODULE_MAX)-objs := max96724_drv.o

$(MODULE_NAME)-objs := imx566_mode_tbls.o imx565_mode_tbls.o gmsl_status.o\
						sc535hgs_mode_tbls.o gmax3405_mode_tbls.o gmax3412_mode_tbls.o\
						og02c1b_mode_tbls.o og05b2b_mode_tbls.o ov5640_mode_tbls.o \
						nv_sensor.o imx_sensor_common.o

PWD:= $(shell pwd)

# EXTRA_CFLAGS += -Wunused
# EXTRA_CFLAGS += -Wall
# # EXTRA_CFLAGS += -Wextra
# EXTRA_CFLAGS += -Werror=unused-variable
# EXTRA_CFLAGS += -Werror=unused-function

all : generate_header
	@$(MAKE) -s -C $(KERNEL_DIR) M=$(PWD) clean
	@$(MAKE) -s -j$(JOBS) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERNEL_DIR) M=$(PWD) modules
	@$(MAKE) upload

upload:
	$(call CHECK_AND_UPLOAD,./$(MODULE_NAME).ko);
	$(call CHECK_AND_UPLOAD,./$(MODULE_MAX).ko);

generate_header:compile_info.sh
	@rm -f version.h
	@./compile_info.sh

to:
	$(call CHECK_AND_UPLOAD,./m.sh);
	$(call CHECK_AND_UPLOAD,./mk.sh);
ru:
	$(call CHECK_AND_UPLOAD,./99-camera.rules);
	$(call CHECK_AND_UPLOAD,./rules_update.sh);

clean:
	@$(MAKE) -s -C $(KERNEL_DIR) M=$(PWD) clean
