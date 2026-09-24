
#include "imx_sensor_common.h"


__u8 usr_set_bit(__u8 num, __u8 bit, __u8 value)
{
	if (bit >= 8)
	{
		return num; // 超出范围，不修改
	}
	if (value)
	{
		return num | (1 << bit); // 设置 bit 为 1
	}
	else
	{
		return num & ~(1 << bit); // 清除 bit 为 0
	}
}

int imx_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val)
{
	int err = 0;
	u32 reg_val = 0;
	err = regmap_read(s_data->regmap, addr, &reg_val);
	usleep_range(100, 110);
	*val = reg_val & 0xFF;
	// printk("read addr:0x%X val:0x%02X\r\n", addr, reg_val);
	return err;
}

int imx_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val)
{
	int err = 0;
	// u32 reg_val = 0;
	struct device *dev = s_data->dev;
	err = regmap_write(s_data->regmap, addr, val);
	if (err)
		dev_err(dev, "%s: i2c write failed, 0x%x = %x err:%d\n",
			__func__, addr, val, err);
	usleep_range(100, 110);
	// err = regmap_read(s_data->regmap, addr, &reg_val);
	// printk("write addr:0x%X val:0x%02X - read_val:0x%04X\r\n", addr, val, reg_val);
	// printk("sensor.register(0x%X,  0,  8) =        0x%02X //\r\n", addr, val);
	// printk("0x%x,0x%02x,\r\n", addr, val);
	return err;
}

void sony_sensor_read_id_type(struct nv_sony_sensor *priv)
{
    int err = 0;
	u8 reg_16 = 0;
	u8 reg_17 = 0;
	u16 id = 0;
    bool is_mono;
	struct camera_common_data *s_data = priv->s_data;
	struct device *dev = &(priv->i2c_client->dev);
	/* 读 0x16 */
	err = imx_read_reg(s_data, SENSOR_MODEL_ID_ADDR_LSB, &reg_16);
	vc_info(dev, "read reg 0x16: 0x%x ret:%d\n", reg_16, err);
	/* 读 0x17 */
	err = imx_read_reg(s_data, SENSOR_MODEL_ID_ADDR_MSB, &reg_17);
	vc_info(dev, "read reg 0x17: 0x%x ret:%d\n", reg_17, err);
	/* 解析 ID (10bit) */
	id = reg_16 | reg_17<<8;
	id = (id&0x7FE0)>>5;
	is_mono = (reg_17 & 0x80) ? true : false;
	dev_info(dev, "Sony Sensor: IMX%d, Type: %s\n",
		 id,
		 is_mono ? "Monochrome" : "Color");
	
	return;
}

__u32 orin_calculate_pwm_period(__u32 *hmax, __u32 input_freq)
{
	uint32_t  mDivisor = 0;
	uint32_t mPeriod = 0;
	mDivisor = DIV_ROUND_UP_ULL(6375*(*hmax), input_freq*4);
	mPeriod = (32000*mDivisor)/51;
	*hmax = div_u64((mPeriod*input_freq), 1000000);
	// printk("mDivisor:%u mPeriod:%u mHmax:%u \r\n", mDivisor, mPeriod, *hmax);
	return mPeriod;
}

int sony_sensor_imx56x_get_temp(struct nv_sony_sensor *priv)
{
	int err = 0;
	int32_t temp_x100;
	u8 reg_val = 0;
	struct camera_common_data *s_data = priv->s_data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = &(priv->i2c_client->dev);
#endif
	err = imx_write_reg(s_data, SENSOR_IMX_TEMPERATURE_EN, 1);
	msleep(200);
	err = imx_read_reg(s_data, SENSOR_IMX_TEMPERATURE_EN, &reg_val);
	vc_info(dev, "SENSOR_IMX_TEMPERATURE_EN 0x%x ret:%d\n", reg_val, err);
	err = imx_read_reg(s_data, SENSOR_IMX_TEMPERATURE_VAL, &reg_val);
	vc_info(dev, "SENSOR_IMX_TEMPERATURE_EN 0x%x ret:%d\n", reg_val, err);
	temp_x100 = ((int32_t)reg_val * 1000 - 51784) * 1600 / 21000;
	return temp_x100;
}

int sony_sensor_reg_debug_set(struct camera_common_data *s_data, SENSOR_DEBUG_REG_PARAMS *reg)
{
	if (0 == reg->opt)
	{
		return imx_read_reg(s_data, reg->addr, &reg->value);
	}
	else if(1 == reg->opt)
	{
		return imx_write_reg(s_data, reg->addr, reg->value);
	}
	return -1;
}

const struct regmap_config *sensor_get_regmap_config(void)
{
    static const struct regmap_config sensor_regmap_config = {
        .reg_bits = 16,
        .val_bits = 8,
        .cache_type = REGCACHE_NONE,
        .use_single_read = true,
        .use_single_write = true,
    };
    return &sensor_regmap_config;
}

int sensor_power_get(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	pw->state = SWITCH_OFF;
	return 0;
}

int sensor_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	if (unlikely(!pw))
		return -EFAULT;
	return 0;
}

int sensor_set_mode(struct tegracam_device *tc_dev)
{	
	return 0;
}
int sensor_power_on(struct camera_common_data *s_data)
{
	return 0;
}
int sensor_power_off(struct camera_common_data *s_data)
{
	return 0;
}
struct camera_common_pdata *sensor_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct camera_common_pdata *board_priv_pdata;
	board_priv_pdata = devm_kzalloc(dev,sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;
	return board_priv_pdata;	
}

int sensor_write_table(struct camera_common_data *s_data,
				const SENSOR_REG_STRUCT table[])
{
	const struct reg_8 *next;
	int ret = 0;
	for (next = table;; next++) {
		if(SENSOR_TABLE_END == next->addr)
		{
			break;
		}
		else if(SENSOR_TABLE_WAIT_MS == next->addr)
		{
			msleep_range(next->val);
			continue;
		}
		else
		{
			ret = imx_write_reg(s_data, next->addr, next->val);
			if(0 != ret)
			{
				return ret;
			}
		}
	}
	return ret;
}


int sensor_set_fmt(struct nv_sony_sensor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format)
{
	int ret;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
	{
		vc_info(dev, "Try format width:%d height:%d \n", format->format.width, format->format.height);
		ret = camera_common_try_fmt(sd, &format->format);
	}
	else
	{
		vc_info(dev, "set format width:%d height:%d \n", format->format.width, format->format.height);
		if(0!=format->format.width || 0 != format->format.height)
			ret = camera_common_s_fmt(sd, &format->format);
	}
	return ret;
}
int sensor_get_fmt(struct nv_sony_sensor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format)
{
	return camera_common_g_fmt(sd, &format->format);
}
int sensor_set_selection(struct nv_sony_sensor *priv, struct v4l2_subdev_state *state, struct v4l2_subdev_selection *sel)
{
	struct v4l2_rect rect;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
	memcpy(&rect, &sel->r, sizeof(struct v4l2_rect));
	if (sel->which == V4L2_SUBDEV_FORMAT_TRY) {
		vc_info(dev, "try rect:%d %d %d %d \n", rect.left, rect.top, rect.height, rect.width);
		if (state && state->pads)
			state->pads->try_crop = rect;
		return 0;
	}
	vc_info(dev, "set active rect:%d %d %d %d \n", rect.left, rect.top, rect.height, rect.width);
	priv->m_rect = rect;
	return 0;
}
int sensor_get_selection(struct nv_sony_sensor *priv, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_selection *sel)
{
	struct v4l2_rect *rect;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
	if (sel->which == V4L2_SUBDEV_FORMAT_TRY) {
		rect = &sd_state->pads->try_crop;
	} else {
		if (priv)
			rect = &priv->m_rect;
		else
			rect = &sd_state->pads->try_crop; /* fallback */
	}
	sel->r = *rect;
	vc_info(dev, "which:0x%x rect:%d %d %d %d \n", sel->which, rect->left, rect->top, rect->height, rect->width);
	return 0;
}

void sensor_reg_info_print(struct device *dev, int set, struct reg_8 *reg)
{
#ifdef USR_DEBUG_ENABLE
	u16 reg_addr = reg->addr;
	u8 val = reg->val;
    char bin[10];
	const char *op;
    bin[0] = ((val >> 7) & 0x1) + '0';
    bin[1] = ((val >> 6) & 0x1) + '0';
    bin[2] = ((val >> 5) & 0x1) + '0';
    bin[3] = ((val >> 4) & 0x1) + '0';
    bin[4] = ' ';
    bin[5] = ((val >> 3) & 0x1) + '0';
    bin[6] = ((val >> 2) & 0x1) + '0';
    bin[7] = ((val >> 1) & 0x1) + '0';
    bin[8] = ((val >> 0) & 0x1) + '0';
    bin[9] = '\0';
	op = set ? "set" : "get";
    dev_info(dev,
        "[%s:%d %s]: %s regAddr:0x%04x val:0x%02x -> 0b%s\r\n",
        FILE_NAME(__FILE__),
        __LINE__,
        __func__,
        op,
        reg_addr,
        val,
        bin);
#endif
	return;
}

