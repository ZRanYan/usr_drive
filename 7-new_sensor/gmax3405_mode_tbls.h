
#ifndef _GMAX3405_MODE_TBLS_H_
#define _GMAX3405_MODE_TBLS_H_

#include <media/camera_common.h>
#include <linux/miscdevice.h>
#include "nv_sensor_common.h"


#define GMAX3405_D_TEMP_0 0x3257
#define GMAX3405_D_TEMP_1 0x3258

#define GMAX3405_PGA_GAIN 0x2EAE

#define GMAX3405_REG_HOLD 0x2E01


extern const struct nv_sensor_model_info gmax3405_i2c_info;

#endif




