
#ifndef _GMAX3405_MODE_TBLS_H_
#define _GMAX3405_MODE_TBLS_H_

#include <media/camera_common.h>
#include <linux/miscdevice.h>
#include "nv_sensor_common.h"


#define GMAX3405_D_TEMP_0 0x3257
#define GMAX3405_D_TEMP_1 0x3258

#define GMAX3405_PGA_GAIN 0x2EAE

#define GMAX3405_REG_HOLD 0x2E01

#define GMAX3405_OTP_INDEX  0x3401
#define GMAX3405_OTP_DATA   0x3402

#define GMAX3405_STREAM_EN  0x2E00

#define GMAX3405_DOFF_ODD_H   0x3706
#define GMAX3405_DOFF_ODD_L   0x3707

#define GMAX3405_DOFF_EVEN_H  0x3806
#define GMAX3405_DOFF_EVEN_L  0x3807

#define GMAX3405_FLIP_H       0x3500
#define GMAX3405_WIN_X_NUM_MASK  0xF0
#define GMAX3405_WIN_X_NUM_SHIFT 4

#define GMAX3405_FLIP_V       0x2E05

#define GMAX3405_WIN1_START_L       0x2E1D //roi行数的开始
#define GMAX3405_WIN1_START_H       0x2E1E

#define GMAX3405_WIN1_LENGTH_L       0x2E1F //roi行数的高度
#define GMAX3405_WIN1_LENGTH_H       0x2E20

#define GMAX3405_WIN_ALL_LEN_L      0x3521
#define GMAX3405_WIN_ALL_LEN_H      0x3522

#define GMAX3405_WIN0_X_LENGTH_L       0x3511 //roi行数的宽度
#define GMAX3405_WIN0_X_LENGTH_H       0x3512

#define GMAX3405_WIN0_X_START_L       0x3501 //roi
#define GMAX3405_WIN0_X_START_H       0x3502


extern const struct nv_sensor_model_info gmax3405_i2c_info;

#endif




