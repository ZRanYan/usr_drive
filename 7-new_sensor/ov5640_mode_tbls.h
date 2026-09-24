
#ifndef _OV5640_MODE_TBLS_H_
#define _OV5640_MODE_TBLS_H_

#include <media/camera_common.h>
#include <linux/miscdevice.h>
#include "nv_sensor_common.h"

#if 1
#define MAX9295A_ALTER_ADDR_BASE 0x20
#else
#define MAX9295A_ALTER_ADDR_BASE 0x42
#endif


#define MAX9295A_PCLK_ADDR_BASE         0x0003
#define MAX9295A_REF_VTG0_ADDR_BASE     0x03F0
#define MAX9295A_REG6_ADDR_BASE         0x0006
#define MAX9295A_PIO_SLEW_1_ADDR_BASE   0x0570

#define MAX9295A_GPIO7_A_ADDR_BASE       0x02D3
#define MAX9295A_GPIO7_B_ADDR_BASE       0x02D4
#define MAX9295A_GPIO7_C_ADDR_BASE       0x02D5


#define EEPROM_BASEADDR 0x51

#define MAX9295A_DEV_ID  0x0d

extern const struct nv_sensor_model_info ov5640_i2c_info;

#endif




