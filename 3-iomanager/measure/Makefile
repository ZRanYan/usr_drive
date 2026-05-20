
THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_ROOT ?= $(abspath $(THIS_DIR)../..)
$(info PROJECT_ROOT = $(PROJECT_ROOT))
include $(PROJECT_ROOT)/config.mk

CFLAG:= -pthread

TARGET := time

SRCS := main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CROSS_COMPILER)gcc $(CFLAG) $^ -o $(TARGET)
	sshpass -p '$(PASSWORD)' scp ./$(TARGET) $(USR)@$(TARGET_IP):/home/$(USR)/nfs

clean:
	rm -f $(TARGET)