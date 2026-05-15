
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
	*val = reg_val & 0xFF;
	//printk("read addr:0x%X val:0x%02X\r\n", addr, *val);
	return err;
}

int imx_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val)
{
	int err = 0;
	struct device *dev = s_data->dev;
	err = regmap_write(s_data->regmap, addr, val);
	if (err)
		dev_err(dev, "%s: i2c write failed, 0x%x = %x\n",
			__func__, addr, val);
	//printk("write addr:0x%X val:0x%02X\r\n", addr, val);
	// printk("sensor.register(0x%X,  0,  8) =        0x%02X //\r\n", addr, val);

	return err;
}

void sony_sensor_read_id_type(struct nv_sony_senor *priv)
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

	// sony_sensor_imx56x_get_temp(priv);
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

int sony_sensor_imx56x_get_temp(struct nv_sony_senor *priv)
{
	int err = 0;
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

	return err;
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
