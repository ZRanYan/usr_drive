
#ifndef _NV_GMSL_SERIAL_H_
#define _NV_GMSL_SERIAL_H_
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <media/tegracam_core.h>
#include "max96724.h"

struct z_ov5640_9295 {
	struct i2c_client	*i2c_client;
	const struct i2c_device_id *id;
	struct v4l2_subdev	*subdev;
	struct device		*dser_dev;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
	struct gmsl_link_ctx    g_ctx;
	// struct gpio_desc 			*mclk_gpio;	// 外部硬件上电管脚
	u32 def_addr;
	u32 act_addr;
	u32 des_link;
	u8 eeprom[128];
	u32 awb[3];
};

typedef struct max96712_sensor {
	struct i2c_client	*i2c_client;
	const struct i2c_device_id *id;
	struct v4l2_subdev	*subdev;
	struct device		*dser_dev;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
	struct gmsl_link_ctx    g_ctx;
	u32 def_addr;
	u32 act_addr;
	u32 des_link;
	u8 eeprom[128];
}MAX96712_SENSOR;


#endif
