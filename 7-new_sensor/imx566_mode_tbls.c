#include "imx566_mode_tbls.h"
#include "imx_sensor_common.h"

static const SETTING_PARAM imx566SetParm[];
static IMX_SENSOR_BIT_REG  sensor_imx566_bit[];
static const unsigned int sensor_imx566_bit_num;
static const char * const scene_mode_menu[];
static int imx566_s_ctrl(struct v4l2_ctrl *ctrl);
static int imx566_sensor_set_black_level(struct camera_common_data *s_data, s64 val);
static int imx566_sensor_set_gain(struct camera_common_data *s_data, s64 val);
static int imx566_sensor_set_filp(struct camera_common_data *s_data, FILP_TYPE type, s64 val);

static const struct regmap_config imx566_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.use_single_read = true,
	.use_single_write = true,
};

static void imx566_dtb_init(struct nv_sony_senor *priv)
{
	struct device *dev = &priv->i2c_client->dev;
	priv->pwdn_gpio =  devm_gpiod_get(dev, "pwdn", GPIOD_OUT_LOW);
	if (IS_ERR(priv->pwdn_gpio)) {
        	dev_err(dev, "Failed to get pwdn_gpio\n");
        	goto error;
    }	
	priv->xclr_gpio =  devm_gpiod_get(dev, "xclr", GPIOD_OUT_LOW);
	if (IS_ERR(priv->xclr_gpio)) {
        	dev_err(dev, "Failed to get xclr_gpio\n");
        	goto error;
    	}
	priv->pwgd_gpio =  devm_gpiod_get(dev, "pg", GPIOD_IN);
	if (IS_ERR(priv->pwgd_gpio)) {
        	dev_err(dev, "Failed to get pwgd_gpio\n");
        	goto error;
    	}
	priv->inck_gpio =  devm_gpiod_get(dev, "inck", GPIOD_OUT_LOW);
	if (IS_ERR(priv->inck_gpio)) {
        	dev_err(dev, "Failed to get inck_gpio\n");
        	goto error;
    	}
	priv->xmaster_gpio = devm_gpiod_get(dev, "xmaster", GPIOD_OUT_LOW);
	if (IS_ERR(priv->xmaster_gpio)) {
        	dev_err(dev, "Failed to get xmaster_gpio\n");
        	goto error;
    }
	priv->pwm_xhs = devm_pwm_get(dev, "pwm-xhs");
	if (IS_ERR(priv->pwm_xhs)) {
		dev_err(dev, "Failed to get pwm_xhs\n");
		goto error;
	}
	return;
error:
	return;
}

enum {
	IMX566_MODE_2856X2848_8BIT,
	IMX566_MODE_2856X2848_10BIT,
	IMX566_MODE_2856X2848_12BIT,
	IMX566_MODE_ROI, 
	IMX566_MODE_BIN_1424X1424_8BIT,
	IMX566_MODE_STOP_STREAM,
	IMX566_MODE_TEST_PATTERN
};

static SENSOR_REG_STRUCT imx566_8bit_4line_master_init[] = {
	{0x3004, 0xA8}, //
	{0x3005, 0x02}, //
	{0x303C, 0x02}, // HVMODE
	{0x30D0, 0x28}, // VOPB_VBLK_HWIDTH
	{0x30D1, 0x0B}, //
	{0x30D2, 0x20}, // FINFO_HWIDTH
	{0x30D3, 0x0B}, //
	// {0x30D4, 0xD0}, // VMAX
	// {0x30D5, 0x0B}, //
	{0x30D8, 0x8C}, // HMAX
	{0x30D9, 0x01}, //
	{0x30E2, 0x06}, // GMRWT
	{0x30E3, 0x24}, // GMTWT
	{0x30E5, 0x02}, // GAINDLY
	{0x30E6, 0x10}, // GSDLY
//配置roi的参数
	{0x3104, 0x03},
	{0x3124, 0x28}, //配置ROI的宽高的参数
	{0x3125, 0x0B},
	{0x3126, 0x20},
	{0x3127, 0x0B},
//-------------------------------------------------
	{0x3200, 0x25}, // ADBIT
	{0x3224, 0x80}, // INCKSEL_D0
	{0x3226, 0x80}, // INCKSEL_D2
	{0x3227, 0x80}, // INCKSEL_D3
	{0x322B, 0x06}, //
	{0x3233, 0x10}, //
	{0x323C, 0x19}, // LLBLANK
	{0x323E, 0x33}, // VINT_EN  VINT_EN_NOR
	// {0x3240, 0x7E}, // SHS
	// {0x3241, 0x04}, //
	{0x3400, 0x09},
	{0x3430, 0x02}, // ODBIT
	{0x3480, 0x20}, // PULSE2_EN_NOR  PULSE2_EN_TRIG  PULSE2_POL
	{0x3502, 0x09}, // GAIN_RTS
	{0x3514, 0x00}, // GAIN
	{0x3521, 0x7D}, //
	{0x3535, 0x00}, //
	{0x3542, 0x27}, //
	{0x3546, 0x1F}, //
	{0x354A, 0x20}, //
	{0x359C, 0x0F}, //
	{0x359D, 0x01}, //
	{0x35A4, 0x08}, //
	{0x35A5, 0x12}, //
	{0x35A8, 0x08}, //
	{0x35A9, 0x52}, //
	{0x35B4, 0x0F}, // BLKLEVEL
	{0x35B6, 0x02}, //
	{0x35CE, 0x0E}, //
	{0x35EC, 0x08}, //
	{0x35ED, 0x12}, //
	{0x35F0, 0xFB}, //
	{0x35F1, 0x0B}, //
	{0x35F2, 0xFB}, //
	{0x35F3, 0x0B}, //
	{0x362E, 0x24}, //
	{0x3642, 0x10}, //
	{0x3656, 0x44}, //
	{0x366A, 0x2E}, //
	{0x3670, 0xC3}, //
	{0x3672, 0x05}, //
	{0x3674, 0xB6}, //
	{0x3675, 0x01}, //
	{0x3676, 0x05}, //
	{0x367E, 0x24}, //
	{0x3692, 0x10}, //
	{0x36E8, 0x11}, //
	{0x36F5, 0x0F}, //
	{0x3797, 0x20}, //
	{0x3904, 0x02}, // LANESEL
	{0x3942, 0x00},
	{0x3E2E, 0x07}, //
	{0x3E30, 0x4E}, //
	{0x3E6E, 0x07}, //
	{0x3E70, 0x35}, //
	{0x3E96, 0x01}, //
	{0x3E9E, 0x38}, //
	{0x3EA0, 0x4C}, //
	{0x3F3A, 0x04}, //
	{0x4056, 0x23}, //
	{0x4096, 0x23}, //
	{0x4182, 0x00}, //
	{0x41A2, 0x03}, //
	{0x4232, 0x3C}, //
	{0x4235, 0x22}, //
	{0x4306, 0x00}, //
	{0x4307, 0x00}, //
	{0x4308, 0x00}, //
	{0x4309, 0x00}, //
	{0x4310, 0x04}, //
	{0x4311, 0x04}, //
	{0x4312, 0x04}, //
	{0x4313, 0x04}, //
	{0x431E, 0x16}, //
	{0x431F, 0x16}, //
	{0x433C, 0x8A}, //
	{0x433D, 0x02}, //
	{0x433E, 0xE8}, //
	{0x433F, 0x05}, //
	{0x4340, 0x9E}, //
	{0x4341, 0x0C}, //
	{0x4460, 0x6C}, //
	{0x446A, 0x4C}, //
	{0x446E, 0x51}, //
	{0x4472, 0x57}, //
	{0x4476, 0x79}, //
	{0x448A, 0x4C}, //
	{0x448E, 0x51}, //
	{0x4492, 0x57}, //
	{0x4496, 0x79}, //
	{0x44EC, 0x3F}, //
	{0x44F0, 0x44}, //
	{0x44F4, 0x4A}, //
	{0x4510, 0x3F}, //
	{0x4514, 0x44}, //
	{0x4518, 0x4A}, //
	{0x4576, 0xBE}, //
	{0x457A, 0xB1}, //
	{0x4580, 0xBC}, //
	{0x4584, 0xAF}, //
	{0x472E, 0x06}, //
	{0x472F, 0x06}, //
	{0x4730, 0x06}, //
	{0x4731, 0x06}, //
	{0x473C, 0x06}, //
	{0x473D, 0x06}, //
	{0x473E, 0x06}, //
	{0x473F, 0x06}, //
	{0x4749, 0x9F}, //
	{0x474A, 0x99}, //
	{0x474B, 0x09}, //
	{0x4753, 0x90}, //
	{0x4754, 0x99}, //
	{0x4755, 0x09}, //
	{0x4788, 0x04}, //
	{0x4864, 0xDC}, //
	{0x4868, 0xDC}, //
	{0x486C, 0xDC}, //
	{0x4874, 0xDC}, //
	{0x4878, 0xDC}, //
	{0x487C, 0xDC}, //
	{0x48A4, 0xF4}, //
	{0x48A8, 0xF4}, //
	{0x48AC, 0xF4}, //
	{0x48B4, 0xF4}, //
	{0x48B8, 0xF4}, //
	{0x48BC, 0xF4}, //
	{0x4900, 0x44}, //
	{0x4901, 0x0A}, //
	{0x4902, 0x01}, //
	{0x4908, 0x6E}, //
	{0x4916, 0x00}, //
	{0x4917, 0x00}, //
	{0x4918, 0xFF}, //
	{0x4919, 0x0F}, //
	{0x491E, 0xFF}, //
	{0x491F, 0x0F}, //
	{0x4920, 0x00}, //
	{0x4921, 0x00}, //
	{0x4926, 0xFF}, //
	{0x4927, 0x0F}, //
	{0x4928, 0x00}, //
	{0x4929, 0x00}, //
	{0x4A34, 0x0A}, //
	{SENSOR_TABLE_END, 0x00}
};

static SENSOR_REG_STRUCT *mode_table[] = {
	[IMX566_MODE_2856X2848_8BIT] = imx566_8bit_4line_master_init,
	[IMX566_MODE_2856X2848_10BIT] = NULL,
	[IMX566_MODE_2856X2848_12BIT] = NULL,
	[IMX566_MODE_ROI] = NULL,
	[IMX566_MODE_BIN_1424X1424_8BIT] = NULL,
	[IMX566_MODE_STOP_STREAM] = NULL,
};

static const int imx566_62_6fps[] = {
	70,
};

static struct camera_common_frmfmt imx566_frmfmt[] = {
	{{2856, 2848}, imx566_62_6fps, 0, 0,IMX566_MODE_2856X2848_8BIT},
};

static int imx566_set_mode(struct tegracam_device *tc_dev)
{	
	return 0;
}
static int imx566_power_on(struct camera_common_data *s_data)
{
	return 0;
}
static int imx566_power_off(struct camera_common_data *s_data)
{
	return 0;
}

static struct camera_common_pdata *imx566_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct camera_common_pdata *board_priv_pdata;
	board_priv_pdata = devm_kzalloc(dev,sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;
	return board_priv_pdata;	
}
static int imx566_power_get(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	pw->state = SWITCH_OFF;
	return 0;
}
static int imx566_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	if (unlikely(!pw))
		return -EFAULT;
	return 0;
}

static struct camera_common_sensor_ops imx566_common_ops = {
	.numfrmfmts = ARRAY_SIZE(imx566_frmfmt),
	.frmfmt_table = imx566_frmfmt,		
	.power_on = imx566_power_on,		
	.power_off = imx566_power_off,		
	.write_reg = imx_write_reg,		
	.read_reg = imx_read_reg,		
	.parse_dt = imx566_parse_dt,		
	.power_get = imx566_power_get,		
	.power_put = imx566_power_put,		
	.set_mode = imx566_set_mode,		
};

static int sensor_power_on_set(struct camera_common_data *s_data)
{
	int err = 0;
	struct nv_sony_senor *priv = (struct nv_sony_senor *)s_data->priv;
	struct camera_common_power_rail *pw = s_data->power;
	struct device *dev = s_data->dev;
	char mNum = 50;
	u8 value = 0;
	//1.power off the sensor
	gpiod_set_value(priv->inck_gpio, 0);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->xclr_gpio, 0);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->pwdn_gpio, 0);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	if(priv->sensorMode == SEQUENTIAL_TRIGGER_MODE)
	{
		gpiod_set_value(priv->xmaster_gpio, 1); //配置slave模式
	}
	else
	{
		gpiod_set_value(priv->xmaster_gpio, 0); //配置master模式
	}
	usleep_range(1000, 1010);
	gpiod_set_value(priv->pwdn_gpio, 1);
	do
	{
		usleep_range(1000, 1010);
		mNum--;
	} while ((0 < mNum));
	gpiod_set_value(priv->xclr_gpio, 1);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->inck_gpio, 1);
	usleep_range(1000, 1010);
	usleep_range(100000, 100010); 
	pw->state = SWITCH_ON;
	err = imx_read_reg(s_data,0x3000,&value);
	if (err){
		dev_err(dev, "%s: Failed to read standby\n",__func__);
		return err;
	}
	return 0;
}
static int sensor_power_off_set(struct camera_common_data *s_data)
{
	struct nv_sony_senor *priv = (struct nv_sony_senor *)s_data->priv;
	struct camera_common_power_rail *pw = s_data->power;
	gpiod_set_value(priv->inck_gpio, 0);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->xclr_gpio, 0);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->pwdn_gpio, 0);
	usleep_range(1000, 1010);
	gpiod_set_value(priv->xmaster_gpio, 0);
	dev_info(s_data->dev,"get pgen_gpio %d status\n", gpiod_get_value(priv->pwgd_gpio));
	pw->state = SWITCH_OFF;
	return 0;
}
static int imx566_board_setup(struct nv_sony_senor *priv)
{
	struct camera_common_data *s_data = priv->s_data;
	struct device *dev = s_data->dev;
	u8 reg_val = 0;
	int err = 0;
	err = camera_common_mclk_enable(s_data);
	if (err) {
		dev_err(dev, "Error %d turning on mclk\n", err);
		return err;
	}
	err = sensor_power_on_set(s_data);
	if (err) {
		dev_err(dev, "Error %d during power on sensor\n", err);
		goto err_reg;
	}
	usleep_range(10000,10010);
	usleep_range(10000,10010);
	usleep_range(10000,10010);
	usleep_range(10000,10010);
	err = imx_read_reg(s_data, SENSOR_IMX_XMSTA, &reg_val);
	err = imx_read_reg(s_data, SENSOR_IMX_STANDBY, &reg_val);
	if(0x01 != reg_val)
	{
		goto err_reg;
	}
	return err;
err_reg:
	sensor_power_off_set(s_data);
	camera_common_mclk_disable(s_data);
	return err;
}
static int imx566_write_table(struct camera_common_data *s_data,
				const SENSOR_REG_STRUCT table[])
{
	const struct reg_8 *next;
	int ret = 0;
	for (next = table;; next++) {
		if(SENSOR_TABLE_END == next->addr)
		{
			break;
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

static void imx566_init_param(struct nv_sony_senor *priv, int workMode)
{
	SENSOR_REG_STRUCT *modeTable = mode_table[workMode];
	struct camera_common_data *s_data = priv->s_data;
	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 1);
	usleep_range(150, 160); // 延时150微秒
	usleep_range(150, 160); // 延时150微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 1);

	imx566_write_table(s_data, modeTable);

	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 0);
	usleep_range(1138, 1148); // 延时1138微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 0);
	imx_write_reg(s_data, SENSOR_IMX_TOUT0SEL, SENSOR_TOUT0_ENABLE); //配置out0输出
}

void imx566_read_id_type(struct nv_sony_senor *priv)
{
	int err = 0;
	u8 reg_val = 0;
	struct camera_common_data *s_data = priv->s_data;
	struct device *dev = &(priv->i2c_client->dev);
	err = imx_read_reg(s_data, SENSOR_MODEL_ID_ADDR_MSB, &reg_val);
	dev_info(dev, "read sensor 0x%x : 0x%x ret:%d\r\n", SENSOR_MODEL_ID_ADDR_MSB, reg_val, err);
	err = imx_read_reg(s_data, SENSOR_MODEL_ID_ADDR_LSB, &reg_val);
	dev_info(dev, "read sensor 0x%x : 0x%x ret:%d\r\n", SENSOR_MODEL_ID_ADDR_LSB, reg_val, err);
	return;
}
void imx533_update_vmax_fps(struct nv_sony_senor *sensor, __u32 h)
{
#ifdef USR_DEBUG_ENABLE
	struct camera_common_data *s_data = sensor->s_data;
#endif
	if (8 > h || imx566_i2c_info.pixel_height < h)
	{
		vc_err(s_data->dev, "set height:%d illegal!\r\n", h);
		return;
	}
	sensor->height = h;
	sensor->vmax = sensor->height + sensor->gmrwt + sensor->gmtwt + sensor->gsdly + 86 + 1;
	sensor->fps = 10000000000 / (sensor->period * sensor->vmax);//计算帧率精确到小数点后一位
	vc_info(s_data->dev, "update:vmax:%d period:%d fps:%d.%d \r\n", sensor->vmax, sensor->period, sensor->fps/10, sensor->fps%10);
	return;
}
int imx566_sensor_update_xhs_pwm(struct nv_sony_senor *priv)
{
	int ret = 0;
	u64 mPeriod = 0; 
	struct pwm_state pwmStatus;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
	vc_info(dev, "enter %s: set period=%d \r\n", __func__, priv->period);
#endif
	// if(mPeriod != priv->period)
	// {
		mPeriod = priv->period;
	// }
	// else
	// {
	// 	vc_warn(dev, "set pwm period %d is no need !\r\n", priv->period);
	// 	return ret;
	// }
	memset(&pwmStatus, 0, sizeof(pwmStatus));
	pwm_init_state(priv->pwm_xhs, &pwmStatus);
	pwmStatus.enabled = false;
	ret = pwm_apply_state(priv->pwm_xhs, &pwmStatus); //先关闭
	if(0 != ret)
	{
		vc_err(dev,"set period:%d error ret:%d \r\n", priv->period, ret);
		mPeriod = 0;
		return -1;
	}
#if 1
	pwmStatus.period = mPeriod * 2;
	pwmStatus.duty_cycle = (mPeriod * 104) >> 8;//ns
#else
	pwmStatus.period = mPeriod * 2;
	pwmStatus.duty_cycle = (mPeriod * 2 - 1000)>0?priv->period - 1000:400;//ns
#endif
	pwmStatus.enabled = true;
	pwmStatus.usage_power = false;
	vc_info(dev,"period=%llu duty=%llu enabled=%d polarity=%d\n",
        pwmStatus.period, pwmStatus.duty_cycle,
        pwmStatus.enabled, pwmStatus.polarity);
	ret = pwm_apply_state(priv->pwm_xhs, &pwmStatus);
	if(0 != ret)
	{
		vc_err(dev,"set period:%d error ret:%d \r\n", priv->period, ret);
		mPeriod = 0;
	}
	return ret;
}
void imx566_calculate_minimum_interval_time(struct nv_sony_senor *sen, SENSOR_ATTRIBUTE *param)
{
	//计算All-pixel模式的Sequential Trigger Mode
	param->exposureUnit = sen->period/1000;
	param->min_trigger_fall = ((5 + sen->gmrwt + sen->gmtwt + 1)*sen->period)/1000;
	param->min_trigger_rise = ((sen->vmax + 1)*(sen->period))/1000;
	param->max_frame = sen->fps;
}
static int imx566_set_roi_format(struct nv_sony_senor *priv, SENSOR_FORMAT_ROI_PARAM *roi_param)
{
	uint8_t i = 0;
	int err = 0;
	u32 shs = 0; //根据快门时间计算
	struct camera_common_data *s_data = priv->s_data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = s_data->dev;
#endif
	priv->x = CALCULATE_ROUND(roi_param->start_x, 8, V4L2_SEL_FLAG_LE);
	priv->y = CALCULATE_ROUND(roi_param->start_y, 8, V4L2_SEL_FLAG_LE);
	priv->width = CALCULATE_ROUND(roi_param->width, 8, V4L2_SEL_FLAG_LE);
	priv->height = CALCULATE_ROUND(roi_param->height, 8, V4L2_SEL_FLAG_LE);
	priv->binning = roi_param->binningMode; //更新binning参数
	priv->bit = roi_param->bitMode;
	imx566_frmfmt[0].size.width = ((0 == priv->binning) ? imx566_i2c_info.pixel_width : imx566_i2c_info.pixel_width / 2);
	imx566_frmfmt[0].size.height = priv->height;
	if (SENSOR_BINNING_2X_DISABLE == priv->binning)
	{
		priv->gmrwt = imx566SetParm[priv->bit].gmrwt;
		priv->gmtwt = imx566SetParm[priv->bit].gmtwt;
		priv->gsdly = imx566SetParm[priv->bit].gsdly;
		priv->hmax  = imx566SetParm[priv->bit].hmax;
	}
	else
	{
		priv->gmrwt = imx566SetParm[priv->bit + 3].gmrwt;
		priv->gmtwt = imx566SetParm[priv->bit + 3].gmtwt;
		priv->gsdly = imx566SetParm[priv->bit + 3].gsdly;
		priv->hmax  = imx566SetParm[priv->bit + 3].hmax;
	}
#if 1
	priv->period = orin_calculate_pwm_period(&priv->hmax, imx566_i2c_info.input_freq);
#else
	priv->period = ((priv->hmax * 1000000)/imx566_i2c_info.input_freq + 1);//更新行周期时间，xhs时间，跟bit切换有关
#endif
	imx533_update_vmax_fps(priv, priv->height);
	if(SEQUENTIAL_TRIGGER_MODE == priv->sensorMode)
	{
		vc_info(dev, "update pwm xhs \r\n");
		imx566_sensor_update_xhs_pwm(priv);
	}
	else
	{
		vc_info(dev, "not need update pwm xhs \r\n");
		//更新帧率计算公式
		priv->fps = 1;
		priv->vmax = 1000000000 / (priv->period * priv->fps) - 1;
		shs = (priv->vmax - ((( 12*1000 - 3)*1000)/priv->period)); //根据计算公式计算曝光时间
		vc_info(dev, "priv->vmax:%d fps:%d shs:%u\r\n",priv->vmax, priv->fps, shs);
		sensor_imx566_bit[26].val_1[priv->bit] = shs & 0xff;
		sensor_imx566_bit[27].val_1[priv->bit] = shs >> 8 & 0xff;
		sensor_imx566_bit[28].val_1[priv->bit] = shs >> 16 & 0xff;
	}

	vc_info(dev, "priv->x:%d \r\n", priv->x);
	vc_info(dev, "priv->y:%d \r\n", priv->y);
	vc_info(dev, "priv->width:%d \r\n", priv->width);
	vc_info(dev, "priv->height:%d \r\n", priv->height);
	vc_info(dev, "priv->vmax:%d \r\n", priv->vmax);
	vc_info(dev, "priv->hmax:%d \r\n", priv->hmax);
	vc_info(dev, "priv->binning:%d \r\n", priv->binning);

	sensor_imx566_bit[1].val_1[priv->bit] = priv->width & 0xff;
	sensor_imx566_bit[2].val_1[priv->bit] = priv->width >> 8 & 0xff;
	sensor_imx566_bit[3].val_1[priv->bit] = priv->width & 0xff;
	sensor_imx566_bit[4].val_1[priv->bit] = priv->width >> 8 & 0xff;

	sensor_imx566_bit[5].val_1[priv->bit] = priv->vmax & 0xff;
	sensor_imx566_bit[6].val_1[priv->bit] = priv->vmax >> 8 & 0xff;
	sensor_imx566_bit[7].val_1[priv->bit] = priv->vmax >> 16 & 0xff;
//---更新hmax的值
	sensor_imx566_bit[8].val_1[priv->bit] = priv->hmax & 0xff;
	sensor_imx566_bit[9].val_1[priv->bit] = priv->hmax >> 8 & 0xff;

	sensor_imx566_bit[15].val_1[priv->bit] = priv->x & 0xff;
	sensor_imx566_bit[16].val_1[priv->bit] = priv->x >> 8 & 0xff;
	sensor_imx566_bit[17].val_1[priv->bit] = priv->y & 0xff;
	sensor_imx566_bit[18].val_1[priv->bit] = priv->y >> 8 & 0xff;
	sensor_imx566_bit[19].val_1[priv->bit] = priv->width & 0xff;
	sensor_imx566_bit[20].val_1[priv->bit] = priv->width >> 8 & 0xff;
	sensor_imx566_bit[21].val_1[priv->bit] = priv->height & 0xff;
	sensor_imx566_bit[22].val_1[priv->bit] = priv->height >> 8 & 0xff;

	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 1);
	usleep_range(150, 160);//延时150微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 1);
	for(i = 0; i < sensor_imx566_bit_num; i++)
	{
		if(1 == sensor_imx566_bit[i].isReuseBinning)
		{
			imx_write_reg(s_data, sensor_imx566_bit[i].addr,
				sensor_imx566_bit[i].val_1[priv->binning]);
		}
		else if(0 == sensor_imx566_bit[i].isReuseBinning || 3 == sensor_imx566_bit[i].isReuseBinning)
		{
			imx_write_reg(s_data, sensor_imx566_bit[i].addr,
				sensor_imx566_bit[i].val_1[priv->bit]);
		}
		else if(2 == sensor_imx566_bit[i].isReuseBinning)
		{
			if(SENSOR_BINNING_2X_DISABLE != priv->binning)
			{
				imx_write_reg(s_data, sensor_imx566_bit[i].addr,sensor_imx566_bit[i].val_2[priv->bit]);
			}
			else
			{
				imx_write_reg(s_data, sensor_imx566_bit[i].addr,
					sensor_imx566_bit[i].val_1[priv->bit]);
			}
		}
	}
	// if(SENSOR_8_BIT == priv->bit)
	// {
	// 	imx566_sensor_set_filp(s_data, VERTICAL, priv->vflip_status);
	// }
	// else
	// {
	// 	imx566_sensor_set_filp(s_data, VERTICAL, ((0 == priv->vflip_status)?1:0));
	// }
	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 0);
	usleep_range(138, 148);//延时1138微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 0);
	return err;
}
int imx566_ioctl_set(struct nv_sony_senor *priv, unsigned int cmd, void *arg)
{
	union sensor_ioctl_data data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
	int ret = 0;
	int tmpValue = 0;
	const char *ver;
	s64 val;
	memset(&data, 0, sizeof(data));
	switch (cmd)
	{
		case V4L2_CID_GET_VERSION:
			vc_info(dev, "get ver:%s \r\n", BL_NV_SENSOR_FULL_VERSION);
			ver = BL_NV_SENSOR_FULL_VERSION;
			data.ver.len = strnlen(ver, SENSOR_VER_MAX_LEN - 1);
			memcpy(data.ver.ver, ver, data.ver.len);
			data.ver.ver[data.ver.len] = '\0';
			if (copy_to_user(arg, &data.ver, sizeof(data.ver))) {
            	ret = -EFAULT;
        	}
			break;
		case V4L2_CID_REGIST_IOCTL:
			ret = copy_from_user(&data.reg, (struct custon_params __user *)arg, sizeof(data.reg));
			vc_info(dev, "Received custom data.reg: %d, %d, %d\n", data.reg.opt, data.reg.addr, data.reg.value);
			sony_sensor_reg_debug_set(priv->s_data, &data.reg);
			ret = copy_to_user((SENSOR_DEBUG_REG_PARAMS __user *)arg, &data.reg, sizeof(data.reg));
			break;
		case V4L2_CID_GET_TEMPERATURE_VAL:
			{
				tmpValue = sony_sensor_imx56x_get_temp(priv);
				ret = copy_to_user((int __user *)arg, &tmpValue, sizeof(tmpValue));
				vc_info(dev, "get tempValue:%d \r\n", tmpValue);
			}
			break;
		case CAM_GET_MIN_EXPOSE:
			imx566_calculate_minimum_interval_time(priv, &data.min_interval);
			vc_info(dev, "max_frame:%d exposureUnit:%d min_trigger_fall:%d min_trigger_rise:%d\n", \
					data.min_interval.max_frame, data.min_interval.exposureUnit, data.min_interval.min_trigger_fall, data.min_interval.min_trigger_rise);
			ret = copy_to_user((SENSOR_ATTRIBUTE __user *)arg, &data.min_interval, sizeof(data.min_interval));
			break;
		case CAM_SET_ROI_FORMAT:
			{	
				ret = copy_from_user(&data.roi, (struct custon_params __user *)arg, sizeof(data.roi));
				vc_info(dev, "isEnable:%d roi_params:%d %d %d %d \n",data.roi.isEnable, data.roi.start_x, data.roi.start_y, data.roi.width, data.roi.height);
				ret = imx566_set_roi_format(priv, &data.roi);
			}
			break;
		case CAM_SET_GAIN:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_GAIN  %lld \n", val);
				ret = imx566_sensor_set_gain(priv->s_data, val);
			}
			break;
		case CAM_SET_BLACK_LEVEL:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_BLACK_LEVEL  %lld \n", val);
				ret = imx566_sensor_set_black_level(priv->s_data, val);
			}
			break;
		case CAM_SET_HFLIP:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_HFLIP  %lld \n", val);
				ret = imx566_sensor_set_filp(priv->s_data, HORIZONTAL, val);
			}
			break;
		case CAM_SET_VFLIP:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_VFLIP  %lld \n", val);
				ret = imx566_sensor_set_filp(priv->s_data, VERTICAL, val);
			}
			break;
		default:
			break;
	}
	return ret;
}

int imx566_set_fmt(struct nv_sony_senor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format)
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
		ret = camera_common_s_fmt(sd, &format->format);
	}
	return ret;
}
int imx566_get_fmt(struct nv_sony_senor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format)
{
	return camera_common_g_fmt(sd, &format->format);
}
int imx566_set_selection(struct nv_sony_senor *priv, struct v4l2_subdev_state *state, struct v4l2_subdev_selection *sel)
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
int imx566_get_selection(struct nv_sony_senor *priv, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_selection *sel)
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

static const struct v4l2_ctrl_ops imx566_ctrl_ops = {
	.s_ctrl = imx566_s_ctrl,
};

static int sensor_debug_pic_set(struct camera_common_data *s_data, uint8_t mode)
{
	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 0x01);
	usleep_range(150, 160); // 延时150微秒
	usleep_range(150, 160); // 延时150微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 1);
	imx_write_reg(s_data, 0x3520, 0x1F);
	imx_write_reg(s_data, 0x3540, 0x00);
	if (0 == mode)
	{
		imx_write_reg(s_data, 0x3550, 0x02);
	}
	else if (1 == mode)
	{
		imx_write_reg(s_data, 0x3550, 0x05);
		imx_write_reg(s_data, 0x3551, 0x07); // mode
	}
	else if (2 == mode)
	{
		imx_write_reg(s_data, 0x3550, 0x05);
		imx_write_reg(s_data, 0x3551, 0x08); // mode
	}
	else if(3 == mode)
	{
		imx_write_reg(s_data, 0x3550, 0x05);
		imx_write_reg(s_data, 0x3551, 0x03); // mode
		imx_write_reg(s_data, 0x3559, 0x01); // PGVPSTEP
	}
	imx_write_reg(s_data, 0x355C, 0x00);		   // PGDATA1
	imx_write_reg(s_data, 0x355D, 0x00 & 0x0f); // PGDATA1
	imx_write_reg(s_data, 0x355E, 0xff);		   // PGDATA1
	imx_write_reg(s_data, 0x355F, 0x00 & 0x0f); // PGDATA1
	imx_write_reg(s_data, 0x3560, 0x0B);
	imx_write_reg(s_data, 0x3562, 0x00);
	imx_write_reg(s_data, 0x35B4, 0x00);
	imx_write_reg(s_data, 0x35B5, 0x00);
	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 0);
	usleep_range(1138, 1148); // 延时1138微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 0);
	return 0;
}
static int imx566_sensor_set_readout_mode(struct camera_common_data *s_data, __u8 val)
{
	struct nv_sony_senor *priv = (struct nv_sony_senor *)s_data->priv;
	struct device *dev = s_data->dev;
	__u8 vint = 0;
	u32 shs = 0; //根据快门时间计算
	__u8 mode = 0;
	int err = 0;
	
	priv->sensorMode = val; //更新状态参数
	imx_write_reg(s_data, SENSOR_IMX_STANDBY, 1);
	usleep_range(150, 160);//延时150微秒
	usleep_range(150, 160);//延时150微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 1);
	
	if(NORMAL_MODE == val) //配置normal模式
	{
		mode = 0x00; 
		imx_write_reg(s_data, SENSOR_IMX_TRIG, mode);
		//配置vmax参数
		imx_write_reg(s_data, SENSOR_IMX_VMAX_0, priv->vmax & 0xff);
		imx_write_reg(s_data, SENSOR_IMX_VMAX_1, (priv->vmax>>8) & 0xff);
		imx_write_reg(s_data, SENSOR_IMX_VMAX_2, (priv->vmax>>16) & 0xff);

		imx_write_reg(s_data, SENSOR_IMX_GMRWT, priv->gmrwt);
		imx_write_reg(s_data, SENSOR_IMX_GMTWT, priv->gmtwt);
		imx_write_reg(s_data, SENSOR_IMX_GSDLY, priv->gsdly);

		imx_read_reg(s_data, SENSOR_IMX_VINT, &vint);
		vint = usr_set_bit(vint, 1, 1); //配置第1位为1
		imx_write_reg(s_data, SENSOR_IMX_VINT, vint);

		//需要计算
		shs = ((((100000 - 3)*1000)/priv->period) - priv->vmax);
		imx_write_reg(s_data, SENSOR_IMX_SHS_0, shs & 0xff);
		imx_write_reg(s_data, SENSOR_IMX_SHS_1, (shs>>8) & 0xff);
		imx_write_reg(s_data, SENSOR_IMX_SHS_2, (shs>>16) & 0xff);
	}
	else if (FAST_TRIGGER_MODE == val) // 配置fast模式
	{
		mode = 0x02;
		mode = (mode << 3) | (0x01);
		imx_write_reg(s_data, SENSOR_IMX_TRIG, mode);
	}
	else if(SEQUENTIAL_TRIGGER_MODE == val) //配置Sequential Trigger Mode模式
	{
		err = imx_write_reg(s_data, SENSOR_IMX_VINT, 0x5B);//V interrupt is disable
		if (err) {
			dev_err(dev, "Failed to disable V interrupt\n");
			return err;
		}
		mode = 0x01;
		mode = (mode << 3) | (0x01);
		err = imx_write_reg(s_data, SENSOR_IMX_TRIG, mode);		
		//开启pwm的参数配置，计算行周期时间，配置pwm管脚输出
		if (err) {
			dev_err(dev, "Failed to enable pwm config\n");
			return err;
		}
		// err = imx566_sensor_update_xhs_pwm(priv);
	}
	err = imx_write_reg(s_data, SENSOR_IMX_STANDBY, 0);
	usleep_range(1138, 1148);//延时1138微秒
	// imx_write_reg(s_data, SENSOR_IMX_XMSTA, 0);
	imx_write_reg(s_data, SENSOR_IMX_TOUT0SEL, SENSOR_TOUT0_ENABLE); //配置out0输出
	//丢弃前8帧，目前延时等待
	if(NORMAL_MODE == val) //配置normal模式
	{
		usleep_range(1138, 1148);//延时1138微秒
		usleep_range(1138, 1148);//延时1138微秒
	}
	return err;
}
static int imx566_sensor_set_black_level(struct camera_common_data *s_data, s64 val)
{
	int err = 0;
	u16 black = 0;
	struct device *dev = s_data->dev;
	black = val;
	err = imx_write_reg(s_data, SENSOR_IMX_BLKLEVEL_L, black&0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_BLKLEVEL_H, (black>>8)&0x0f);
	if (err)
		dev_err(dev, "%s: gain control error\n", __func__);
	return err;
}
static int imx566_sensor_set_gain(struct camera_common_data *s_data, s64 val)
{
	int err = 0;
	u16 gain = 0;
	struct device *dev = s_data->dev;
	gain = val * 10;
	err = imx_write_reg(s_data, SENSOR_IMX_GAIN_ADDR_L, gain&0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_GAIN_ADDR_H, (gain>>8)&0x01);
	if (err)
		dev_err(dev, "%s: gain control error\n", __func__);
	return 0;
}
static int imx566_sensor_set_filp(struct camera_common_data *s_data, FILP_TYPE type, s64 val)
{
	u8 value = 0;
	int err = 0;
	struct device *dev = s_data->dev;
	err = imx_read_reg(s_data, SENSOR_IMX_REVERSE_ADDR, &value);
	if(VERTICAL == type)
	{
		value &= 0x02;
		value |= (val&0x01);
	}
	else if(HORIZONTAL == type)
	{
		value &= 0x01;
		value |= ((val<<1)&0x02);
	}
	err |= imx_write_reg(s_data, SENSOR_IMX_REVERSE_ADDR, value);
	if (err)
		dev_err(dev, "%s: flip control error\n", __func__);
	return err;
}
static int imx566_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct nv_sony_senor *priv = container_of(ctrl->handler, struct nv_sony_senor, ctrl_handler);
	struct camera_common_data	*s_data = priv->s_data;
	int err = 0;
	switch (ctrl->id) {
		case V4L2_CID_CONTRAST:
			err = sensor_debug_pic_set(s_data, ctrl->val);
			break;
		case V4L2_CID_TEST_PATTERN:
			vc_info(s_data->dev,"CID Scene mode set to %s (%d)\n", scene_mode_menu[ctrl->val], ctrl->val);
			err = imx566_sensor_set_readout_mode(s_data, ctrl->val); //配置输出模式
			break;
		case V4L2_CID_BLACK_LEVEL: //黑电平参数
			vc_info(s_data->dev,"CID V4L2_CID_BLACK_LEVEL  %d \n", ctrl->val);
			err = imx566_sensor_set_black_level(s_data, ctrl->val);
			break;
		case V4L2_CID_GAIN: //增益参数
			vc_info(s_data->dev,"CID V4L2_CID_GAIN %d \n", ctrl->val);
			err = imx566_sensor_set_gain(s_data, ctrl->val);
			break;
		case V4L2_CID_HFLIP: //水平翻转
			vc_info(s_data->dev,"CID V4L2_CID_HFLIP %d \n", ctrl->val);
			err = imx566_sensor_set_filp(s_data, HORIZONTAL, ctrl->val);
			break;
		case V4L2_CID_VFLIP: //垂直翻转
			vc_info(s_data->dev,"CID V4L2_CID_VFLIP %d \n", ctrl->val);
			err = imx566_sensor_set_filp(s_data, VERTICAL, ctrl->val);
			break;
		default:
			break;
	};
	return err;
}
static const char * const scene_mode_menu[] = {
	"NormalMode",
	"FastTriggerMode",
	"SequentialTriggerMode",
};
static int imx566_ctrls_init(struct nv_sony_senor *priv)
{
	int err = 0;
	struct device *dev = priv->s_data->dev;
	int real_num_ctrls = 8;
	v4l2_ctrl_handler_init(&priv->ctrl_handler, real_num_ctrls);
	priv->sensor_mode = v4l2_ctrl_new_std_menu_items(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_TEST_PATTERN,
													  ARRAY_SIZE(scene_mode_menu) - 1, 0, priv->sensorMode, scene_mode_menu); // 1: default
	priv->black = v4l2_ctrl_new_std(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_BLACK_LEVEL, 0, 2048, 1, SENSOR_DEFAULT_BLACK_VALUE);
	priv->gain = v4l2_ctrl_new_std(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_GAIN, 0, 48, 1, 0);//默认值为0
	priv->hflip = v4l2_ctrl_new_std(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	priv->vflip = v4l2_ctrl_new_std(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	priv->test  = v4l2_ctrl_new_std(&priv->ctrl_handler, &imx566_ctrl_ops, V4L2_CID_CONTRAST, 0, 3, 1, 0); //默认不开启
	priv->numctrls = 0;
	priv->s_data->numctrls = 0;
	priv->s_data->ctrls = NULL;
	err = v4l2_ctrl_handler_setup(&priv->ctrl_handler);
	if (err) {
		dev_err(dev, "Error %d in control hdl setup\n", err);
		goto error;
	}
	err = priv->ctrl_handler.error;
	if (err) {
		dev_err(dev, "Error %d adding controls\n", err);
		goto error;
	}
	return 0;
error:
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
	return err;
}

// CSI-2 ，4line模式下，INCK = 74.25MHz
static const SETTING_PARAM imx566SetParm[] =
{
	// bit,	bingOrSub, gmrwt,  gmtwt,  gsdly,	hmax,	vmax
	{0, 	    0, 		0x06,   0x24,   0x10,   0x18C,   0xBB0}, // 8bit
	{1, 	    0, 		0x04,   0x1E,   0x0E,   0x1E5,   0xBA6}, // 10bit
	{2, 	    0, 		0x04,   0x1A,   0x0C,   0x23E,   0xBA0}, // 12bi  
//sub and binning mode param
	{0, 	    1, 		0x0C,   0x44,   0x1C,   0x0DA,   0x634}, // 8bit
	{1, 	    1, 		0x08,   0x38,   0x18,   0x106,   0x620}, // 10bit
	{2, 	    1, 		0x08,   0x30,   0x14,   0x133,   0x614}, // 12bit
};
static IMX_SENSOR_BIT_REG sensor_imx566_bit[] = { //默认是Normal模式的，不包含Binning和ROI模式
	{0x303C, 1, {0x02,0x0A,0x12},0,{0,0,0}}, //HVMODE
	{0x30D0, 0, {0x28,0x94,0x94},1,{0,0,0}}, //VOPB_VBLK_HWIDTH
	{0x30D1, 0, {0x0B,0x05,0x05},2,{0,0,0}}, 
	{0x30D2, 0, {0x28,0x94,0x94},3,{0,0,0}}, //FINFO_HWIDTH
	{0x30D3, 0, {0x0B,0x05,0x05},4,{0,0,0}}, 
	{0x30D4, 2, {0xB0,0xA6,0xA0},5,{0x34,0x20,0x14}}, //VMAX
	{0x30D5, 2, {0x0B,0x0B,0x06},6,{0x06,0x06,0x06}}, 
	{0x30D6, 2, {0x00,0x00,0x00},7,{0x00,0x00,0x00}}, 
	{0x30D8, 2, {0x8C,0xE5,0x3E},8,{0xDA,0x06,0x33}}, //HMAX
	{0x30D9, 2, {0x01,0x01,0x02},9,{0x00,0x01,0x01}},
	{0x30E2, 2, {0x06,0x04,0x04},10,{0x0C,0x08,0x08}}, //GMRWT
	{0x30E3, 2, {0x24,0x1E,0x1A},11,{0x44,0x38,0x30}}, //GMTWT
	{0x30E5, 1, {0x02,0x04,0x04},12,{0,0,0}}, //GAINDLY
	{0x30E6, 2, {0x10,0x0E,0x0C},13,{0x1C,0x18,0x14}}, //GSDLY
//	Chip ID = 03h
	{0x3104, 1, {0x03,0x00,0x03},14,{0,0,0}}, //FID0_ROIH1ON
	{0x3120, 0, {0x03,0x00,0x03},15,{0,0,0}}, //FID0_ROIPH1 
	{0x3121, 0, {0x03,0x00,0x03},16,{0,0,0}}, //
	{0x3122, 0, {0x03,0x00,0x03},17,{0,0,0}}, //FID0_ROIPV1
	{0x3123, 0, {0x03,0x00,0x03},18,{0,0,0}}, //
	{0x3124, 0, {0x03,0x00,0x03},19,{0,0,0}}, //FID0_ROIWH1 
	{0x3125, 0, {0x03,0x00,0x03},20,{0,0,0}}, //
	{0x3126, 0, {0x03,0x00,0x03},21,{0,0,0}}, //FID0_ROIWV1
	{0x3127, 0, {0x03,0x00,0x03},22,{0,0,0}}, //
//	Chip ID = 04h
	{0x3200, 3, {0x25,0x05,0x15},23,{0,0,0}}, //ADBIT，bitting模式下SHS寄存器值不同
	// {0x3204, 0, {0x01,0x01,0x01},24,{0,0,0}},//VREVERSE，HREVERSE
	{0x323E, 1, {0x31,0x29,0x29},25,{0,0,0}}, //VINT_EN  VINT_EN_NOR
	{0x3240, 0, {0x7E,0xFB,0xB3},26,{0,0,0}}, //SHS,bitting模式下SHS寄存器值不同
	{0x3241, 0, {0x05,0x05,0x00},27,{0,0,0}}, //
	{0x3242, 0, {0x00,0x00,0x00},28,{0,0,0}}, //
//	Chip ID = 06h
	{0x3400, 0, {0x09,0x09,0x09},27,{0,0,0}}, //TRIGMODE  TRIGTIMING，触发模式
	{0x3430, 3, {0x02,0x00,0x01},27,{0,0,0}}, //ODBIT，bitting模式下寄存器值不同
//  Chip ID = 07h
	{0x3521, 1, {0x7D,0x41,0x41},25,{0,0,0}}, //
	{0x3546, 1, {0x1F,0x10,0x10},26,{0,0,0}}, //
	{0x35A4, 3, {0x08,0x1C,0x08},27,{0,0,0}}, //bitting模式下寄存器值不同
	{0x35A8, 3, {0x08,0x1C,0x08},29,{0,0,0}}, //bitting模式下寄存器值不同
	// {0x35B4, 0, {0x0F,0x3C,0xF0},27,{0,0,0}}, //BLKLEVEL
	// {0x35B5, 0, {0x00,0x00,0x00},28,{0,0,0}}, //
	{0x35EC, 3, {0x08,0x1C,0x08},27,{0,0,0}}, //bitting模式下寄存器值不同 
	{0x36E8, 3, {0x11, 0x11,0x13},28,{0,0,0}}, //bitting模式下寄存器值不同
//  Chip ID = 0Bh
	{0x4728, 3, {0x00,0xD4,0x00},30,{0,0,0}}, //bit，配置，8bit没有
	{0x4729, 3, {0x00,0x0E,0x00},31,{0,0,0}}, //bit，配置，8bit没有
	{0x472E, 3, {0x06,0x00,0x06},31,{0,0,0}}, //bitting模式下寄存器值不同
	{0x472F, 3, {0x06,0x04,0x06},31,{0,0,0}}, //bitting模式下寄存器值不同
	{0x4730, 3, {0x06,0x04,0x06},31,{0,0,0}}, //bitting模式下寄存器值不同
	{0x4731, 3, {0x06,0x04,0x06},31,{0,0,0}}, //bitting模式下寄存器值不同
	{0x4900, 3, {0x44,0x64,0x6C},32,{0,0,0}}, //bitting模式下寄存器值不同
	{0x4908, 3, {0x6E,0x6E,0x68},33,{0,0,0}}, //bitting模式下寄存器值不同
};
static const unsigned int sensor_imx566_bit_num = ARRAY_SIZE(sensor_imx566_bit);

const struct nv_sensor_model_info imx566_i2c_info = {
	.usr_id = 0,
	.sensor_id = 0x1A,
	.name = "imx566",
	.input_freq = 74250,
	.pixel_width = 2856,
	.pixel_height = 2848,
	.pixel_bit = SENSOR_8_BIT,
	.sensorMode = SEQUENTIAL_TRIGGER_MODE,
	.map_config = &imx566_regmap_config,
	.cam_com_ops = &imx566_common_ops,
	.dtb_init = &imx566_dtb_init,
	.board_init = &imx566_board_setup,
	.sensor_init_param = &imx566_init_param,
	.sensor_usr_set = &sony_sensor_read_id_type,
	.sensor_ioctl_set = &imx566_ioctl_set,
	.sensor_stream_set = NULL,
	.sensor_fmt_set = &imx566_set_fmt,
	.sensor_fmt_get = &imx566_get_fmt,
	.sensor_set_selection = imx566_set_selection,
	.sensor_get_selection = imx566_get_selection,
	.sensor_ctrls_init    = imx566_ctrls_init,
};