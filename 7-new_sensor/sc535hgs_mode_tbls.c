
#include "sc535hgs_mode_tbls.h"
#include "imx_sensor_common.h"

#define SC535_WIDTH     2448
#define SC535_HEIGHT    2048

#define BL_NV_SC535HGS_FULL_VERSION_EX \
     "sc535hgs " BL_NV_SENSOR_FULL_VERSION

static const struct regmap_config sc535_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.use_single_read = true,
	.use_single_write = true,
};

enum {
	SC535_MODE_10BIT,
	SC535_MODE_8BIT,
	SC535_MODE_12BIT,
	SC535_MODE_ROI, 
	SC535_MODE_BIN_1424X1424_8BIT,
	SC535_MODE_STOP_STREAM,
	SC535_MODE_TEST_PATTERN
};

static const int sc535_80_6fps[] = {
	80,
};

static struct camera_common_frmfmt sc535_frmfmt[] = {
	{{SC535_WIDTH, SC535_HEIGHT}, sc535_80_6fps, 0, 0,SC535_MODE_10BIT},
};

static struct camera_common_sensor_ops sc535_common_ops = {
	.numfrmfmts = ARRAY_SIZE(sc535_frmfmt),
	.frmfmt_table = sc535_frmfmt,		
	.power_on = sensor_power_on,
	.power_off = sensor_power_off,		
	.write_reg = imx_write_reg,		
	.read_reg = imx_read_reg,		
	.parse_dt = sensor_parse_dt,		
	.power_get = sensor_power_get,	
	.power_put = sensor_power_put,		
	.set_mode = sensor_set_mode,		
};
/**
 * @brief 解析设备树配置的参数
 * 
 * @param priv 
 */
static void sc535_dtb_init(struct nv_sony_senor *priv)
{
    struct device *dev = &priv->i2c_client->dev;
    priv->pwdn_gpio =  devm_gpiod_get(dev, "pwren", GPIOD_OUT_LOW);
	if (IS_ERR(priv->pwdn_gpio)) {
        	dev_err(dev, "Failed to get pwdn_gpio\n");
        	goto error;
    }
    priv->reset_gpio =  devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpio)) {
        	dev_err(dev, "Failed to get reset_gpio\n");
        	goto error;
    }
    priv->pwdnb_gpio =  devm_gpiod_get(dev, "pwdn", GPIOD_OUT_LOW);
	if (IS_ERR(priv->pwdnb_gpio)) {
        	dev_err(dev, "Failed to get pwdnb_gpio\n");
        	goto error;
    }
	// priv->fsync_gpio = devm_gpiod_get(dev, "fsync", GPIOD_OUT_LOW);
	// if (IS_ERR(priv->fsync_gpio)) {
    //     	dev_err(dev, "Failed to get fsync_gpio\n");
    //     	goto error;
    // }
error:
	return;
}

static int sensor_power_on_set(struct camera_common_data *s_data)
{
	struct nv_sony_senor *priv = (struct nv_sony_senor *)s_data->priv;
	struct camera_common_power_rail *pw = s_data->power;
	// gpiod_set_value(priv->fsync_gpio, 0); //触发信号是上升沿触发
    gpiod_set_value(priv->pwdn_gpio, 0);
    usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
	usleep_range(1000, 1010);
    gpiod_set_value(priv->pwdn_gpio, 1);
	usleep_range(500, 510);
	gpiod_set_value(priv->reset_gpio, 1);
	usleep_range(500, 510);
	gpiod_set_value(priv->pwdnb_gpio, 1);
	pw->state = SWITCH_ON;
	return 0;
}
static int sensor_power_off_set(struct camera_common_data *s_data)
{
	struct nv_sony_senor *priv = (struct nv_sony_senor *)s_data->priv;
	struct camera_common_power_rail *pw = s_data->power;
	gpiod_set_value(priv->pwdnb_gpio, 0);
	usleep_range(500, 510);
    gpiod_set_value(priv->reset_gpio, 0);
	usleep_range(500, 510);
    gpiod_set_value(priv->pwdn_gpio, 0);
	pw->state = SWITCH_OFF;
	return 0;
}
static int sc535_board_setup(struct nv_sony_senor *priv)
{
	struct camera_common_data *s_data = priv->s_data;
    struct device *dev = s_data->dev;
	u8 reg_val = 1;
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
    err = imx_read_reg(s_data, 0x2100, &reg_val);
    vc_info(dev, "read 0x2100 value:%d \r\n", reg_val);
    if(0x00 != reg_val)
	{
		goto err_reg;
	}
	return err;

err_reg:
	sensor_power_off_set(s_data);
	camera_common_mclk_disable(s_data);
	return err;
}
static SENSOR_REG_STRUCT sc535_12bit_4line_master_init[] = {
    {0x2103, 0x01},
	{SENSOR_TABLE_WAIT_MS, 5},
	{0x2100, 0x00},
	{0x36e9, 0x80},
	{0x37f9, 0x80},
	{0x3018, 0x7b},
	{0x3019, 0xf0},
	{0x301f, 0x5e},
	{0x3031, 0x0a},
	{0x3037, 0x00},
	{0x3058, 0x21},
	{0x3059, 0x43},
	{0x305a, 0x65},
	{0x3062, 0x00},
	{0x309f, 0xb0},
	{0x30b8, 0x44},
	{0x3200, 0x00},
	{0x3201, 0x00},
	{0x3202, 0x00},
	{0x3203, 0x06},
	{0x3204, 0x09},
	{0x3205, 0x9f},
	{0x3206, 0x08},
	{0x3207, 0x09},
	{0x3208, 0x09},
	{0x3209, 0x90},
	{0x320a, 0x08},
	{0x320b, 0x00},
	{0x320c, 0x02},
	{0x320d, 0xa3},
	{0x320e, 0x08},
	{0x320f, 0x22},
	{0x3210, 0x00},
	{0x3211, 0x08},
	{0x3212, 0x00},
	{0x3213, 0x02},
	{0x321f, 0x0b},
	// {0x3224, 0x93},
	{0x3227, 0x00},
	{0x322f, 0x00},
	{0x3231, 0x00},
	{0x3241, 0x00},
	{0x3243, 0x03},
	{0x3248, 0x04},
	{0x3249, 0x0f},
	{0x3250, 0x03},
	{0x3271, 0x10},
	{0x3273, 0x13},
	{0x32c0, 0x07},
	{0x3300, 0x04},
	{0x3301, 0x10},
	{0x3303, 0x24},
	{0x3304, 0x40},
	{0x3306, 0x58},
	{0x3309, 0x58},
	{0x330b, 0xb8},
	{0x330e, 0x02},
	{0x330f, 0x04},
	{0x3310, 0x80},
	{0x3311, 0x04},
	{0x3312, 0x54},
	{0x3313, 0x11},
	{0x3314, 0x1c},
	{0x3316, 0x11},
	{0x3317, 0x5e},
	{0x331b, 0x08},
	{0x331c, 0xd1},
	{0x331d, 0x3e},
	{0x331f, 0x02},
	{0x3320, 0xf1},
	{0x333b, 0x3c},
	{0x3352, 0x1e},
	{0x3356, 0x18},
	{0x3357, 0x1e},
	{0x3360, 0x80},
	{0x3361, 0x20},
	{0x3363, 0x9f},
	{0x3385, 0x21},
	{0x3387, 0x39},
	{0x33ad, 0x38},
	{0x33af, 0x40},
	{0x33b0, 0x00},
	{0x33b5, 0x08},
	{0x33b6, 0x18},
	{0x33b8, 0x30},
	{0x33ba, 0x40},
	{0x33ef, 0x04},
	{0x33f0, 0x00},
	{0x33f8, 0x01},
	{0x33f9, 0x00},
	{0x33fa, 0x01},
	{0x3406, 0x1e},
	{0x3407, 0x2f},
	{0x341e, 0x16},
	{0x3420, 0x88},
	{0x3421, 0x98},
	{0x3422, 0xb8},
	{0x3424, 0x30},
	{0x3426, 0x38},
	{0x3428, 0x40},
	{0x342a, 0x40},
	{0x3435, 0x07},
	{0x3436, 0x01},
	{0x3437, 0x21},
	{0x3438, 0x01},
	{0x3439, 0x21},
	{0x343a, 0x01},
	{0x343b, 0x21},
	{0x343c, 0x01},
	{0x343d, 0x1c},
	{0x34af, 0x02},
	{0x34f2, 0x00},
	{0x34f3, 0x08},
	{0x34f4, 0x18},
	{0x34f5, 0x80},
	{0x361a, 0x40},
	{0x361b, 0x98},
	{0x361f, 0x00},
	{0x3628, 0xa0},
	{0x3629, 0x82},
	{0x362a, 0x00},
	{0x3630, 0x80},
	{0x3633, 0x44},
	{0x3634, 0x04},
	{0x3636, 0x53},
	{0x3639, 0x80},
	{0x363a, 0x06},
	{0x363b, 0x0e},
	{0x363c, 0x0e},
	{0x363d, 0x08},
	{0x363e, 0x4e},
	{0x363f, 0x22},
	{0x3654, 0x40},
	{0x365c, 0x40},
	{0x3665, 0x08},
	{0x3666, 0x80},
	{0x3667, 0x98},
	{0x3668, 0xb8},
	{0x3670, 0xc0},
	{0x3671, 0xa8},
	{0x3672, 0xaa},
	{0x3676, 0x82},
	{0x3677, 0x82},
	{0x3678, 0x82},
	{0x3679, 0x81},
	{0x367a, 0x81},
	{0x367c, 0x44},
	{0x367d, 0x44},
	{0x367e, 0x55},
	{0x3680, 0x04},
	{0x3681, 0x04},
	{0x3682, 0x04},
	{0x3684, 0x53},
	{0x3685, 0x53},
	{0x3686, 0x53},
	{0x3699, 0x53},
	{0x369a, 0x53},
	{0x369b, 0x53},
	{0x36c0, 0x88},
	{0x36c1, 0xb8},
	{0x36c2, 0x18},
	{0x36c3, 0x98},
	{0x36c8, 0x08},
	{0x36c9, 0x38},
	{0x36ca, 0x00},
	{0x36cb, 0x80},
	{0x36d2, 0x00},
	{0x36d3, 0x00},
	{0x36d4, 0x00},
	{0x36ea, 0xd5},
	{0x36eb, 0x14},
	{0x36ec, 0x42},
	{0x36ed, 0x84},
	{0x3718, 0x05},
	{0x3720, 0x04},
	{0x3722, 0xc8},
	{0x3724, 0xa1},
	{0x3770, 0x14},
	{0x3771, 0x14},
	{0x3772, 0x15},
	{0x3778, 0xd8},
	{0x3779, 0xd8},
	{0x377a, 0xd8},
	{0x37c0, 0x08},
	{0x37c1, 0x98},
	{0x37c4, 0x08},
	{0x37c5, 0x38},
	{0x37fa, 0x0f},
	{0x37fb, 0x45},
	{0x37fc, 0x20},
	{0x37fd, 0x24},
	{0x3900, 0x1d},
	{0x3901, 0x02},
	{0x3903, 0x40},
	{0x3904, 0x08},
	{0x3905, 0x4d},
	{0x3907, 0x00},
	{0x3908, 0x40},
	{0x391f, 0x44},
	{0x3933, 0x80},
	{0x3934, 0x00},
	{0x3937, 0x74},
	{0x393a, 0x00},
	{0x3e00, 0x00},
	{0x3e01, 0x81},
	{0x3e02, 0x80},
	{0x3e03, 0x0b},
	{0x3e08, 0x00},
	{0x3e09, 0x20},
	{0x3e15, 0x00},
	{0x3e16, 0x01},
	{0x3e17, 0x8c},
	{0x3e18, 0x01},
	{0x3e19, 0x8c},
	{0x4310, 0x00},
	{0x4311, 0x00},
	{0x4312, 0x00},
	{0x4313, 0x08},
	{0x4314, 0x01},
	{0x4315, 0x80},
	{0x4330, 0x50},
	{0x4331, 0x20},
	{0x4338, 0xae},
	{0x4350, 0x20},
	{0x4360, 0x0f},
	{0x4362, 0xb8},
	{0x4364, 0xc8},
	{0x4365, 0x08},
	{0x4366, 0x18},
	{0x4368, 0x58},
	{0x436a, 0x68},
	{0x436b, 0x08},
	{0x436c, 0x18},
	{0x4370, 0x20},
	{0x4371, 0x20},
	{0x4372, 0x08},
	{0x4373, 0x18},
	{0x437a, 0x10},
	{0x437b, 0x38},
	{0x4380, 0x20},
	{0x4381, 0x20},
	{0x4382, 0x10},
	{0x4383, 0x08},
	{0x4384, 0x18},
	{0x4385, 0x38},
	{0x4503, 0x20},
	{0x4509, 0x40},
	{0x450a, 0x00},
	{0x450d, 0x14},
	// {0x451e, 0x01}, //sensor出测试图
	{0x4837, 0x0e},
	{0x4b01, 0x11},
	{0x4b0b, 0x00},
	{0x5000, 0x38},
	{0x5002, 0x00},
	{0x502e, 0xa1},
	{0x5030, 0x09},
	{0x5031, 0xa0},
	{0x5032, 0x08},
	{0x5033, 0x10},
	{0x5034, 0x03},
	{0x5104, 0x14},
	{0x5105, 0x10},
	{0x5106, 0x03},
	{0x5107, 0xb9},
	{0x5780, 0x76},
	{0x5784, 0x10},
	{0x5785, 0x08},
	{0x5787, 0x0a},
	{0x5788, 0x0a},
	{0x5789, 0x08},
	{0x578a, 0x0a},
	{0x578b, 0x0a},
	{0x578c, 0x08},
	{0x578d, 0x40},
	{0x5790, 0x08},
	{0x5791, 0x04},
	{0x5792, 0x04},
	{0x5793, 0x08},
	{0x5794, 0x04},
	{0x5795, 0x04},
	{0x57a8, 0xd2},
	{0x57aa, 0x2a},
	{0x57ab, 0x7f},
	{0x57ac, 0x00},
	{0x57ad, 0x00},
	{0x6ec0, 0x09},
	{0x6ec1, 0x90},
	{0x6ec2, 0x08},
	{0x6ec3, 0x00},
	{0x3222, 0x01},
	{0x3282, 0x09},
	{0x3224, 0x82},
	{0x3250, 0xff},
	{0x3253, 0x04},
	{0x4347, 0x02},
	{0x34af, 0x00},
	{0x33b1, 0x05},
	{0x340f, 0x01},
	{0x34de, 0x00},
	{0x3234, 0x06},
	{0x349b, 0x05},
	{0x3231, 0x0a},
	{0x322f, 0x08},
	{0x36e9, 0x53},
	{0x37f9, 0x53},
	{0x550e, 0x02},
	{0x550f, 0x88},
	{0x4412, 0x01},
	{0x4424, 0x01},
	{0x2100, 0x01},
    {SENSOR_TABLE_WAIT_MS, 10},
	{0x363a, 0x26},
	{0x363a, 0x06},
	{SENSOR_TABLE_END, 0x00}
};

static SENSOR_REG_STRUCT *mode_table[] = {
	[SC535_MODE_10BIT] = sc535_12bit_4line_master_init,
	[SC535_MODE_8BIT] = NULL,
	[SC535_MODE_12BIT] = NULL,
};

static void sc535_init_param(struct nv_sony_senor *priv, int workMode)
{
	struct camera_common_data *s_data = priv->s_data;
	sensor_write_table(s_data, mode_table[workMode]);
}
void sc535_read_id_type(struct nv_sony_senor *priv)
{
    int err = 0;
	u8 reg_16 = 0;
	u8 reg_17 = 0;
	struct camera_common_data *s_data = priv->s_data;
	struct device *dev = &(priv->i2c_client->dev);
	err = imx_read_reg(s_data, SC_SC535_ID_H, &reg_16);
	vc_info(dev, "read reg 0x%x: 0x%x ret:%d\n", SC_SC535_ID_H, reg_16, err);
	err = imx_read_reg(s_data, SC_SC535_ID_L, &reg_17);
	vc_info(dev, "read reg 0x%x: 0x%x ret:%d\n", SC_SC535_ID_L, reg_17, err);

    if(0xbe == reg_16 && 0x77 == reg_17)
    {
        dev_info(dev, "sc535hgs sensor read id ok!\r\n");
    }
	return;
}
static int sc535hgs_get_temp(struct nv_sony_senor *priv)
{
	int err = 0;
	u8 reg_val = 0;
	int temp_int = 0;
	int temp_dec = 0;
	struct camera_common_data *s_data = priv->s_data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = &(priv->i2c_client->dev);
#endif
	err = imx_read_reg(s_data, 0x4c10, &reg_val);
	temp_int = reg_val;
	vc_info(dev, "0x4c10 0x%x ret:%d\n", reg_val, err);
	err = imx_read_reg(s_data, 0x4c11, &reg_val);
	vc_info(dev, "0x4c11 0x%x ret:%d\n", reg_val, err);
	temp_int = temp_int<<1;
	temp_int |= ((reg_val&0x04)>>2);
	temp_int *= 100;
	temp_dec = (reg_val&0x03);
	temp_dec *= 25;
	temp_int += temp_dec;
	temp_int -= 27315;
	return temp_int;
}

static void sc535hgs_set_expo_period(struct camera_common_data *s_data, SENSOR_EXPO_PERIOD_PARAM *value)
{
	u16 param =  value->period/6; //跟行单位时间有关系
	u8 reg;
	if(1 == value->type)
	{
		// imx_write_reg(s_data, SENSOR_SC_SLEEP_MODE, 0);
		msleep(5);
		printk("SENSOR_SC_BLANK_ROWS:0x%X \r\n", param);
		imx_write_reg(s_data, SENSOR_SC_BLANK_ROWS_0, param & 0xff);
		imx_write_reg(s_data, SENSOR_SC_BLANK_ROWS_1, (param>>8) & 0xff);
		param = value->expo/6;
		printk("SENSOR_SC_RB_ROWS:0x%X \r\n", param);
		imx_write_reg(s_data, SENSOR_SC_RB_ROWS_0, param & 0xff);
		imx_write_reg(s_data, SENSOR_SC_RB_ROWS_1, (param>>8) & 0xff);
		// imx_write_reg(s_data, SENSOR_SC_SLEEP_MODE, 1);
	}

	value->expo = 0;
	imx_read_reg(s_data, SENSOR_SC_RB_ROWS_0, &reg);
	value->expo = reg;
	imx_read_reg(s_data, SENSOR_SC_RB_ROWS_1, &reg);
	value->expo |= (reg<<8);
	value->expo *= 6;
	value->period = 0;
	imx_read_reg(s_data, SENSOR_SC_BLANK_ROWS_0, &reg);
	value->period = reg;
	imx_read_reg(s_data, SENSOR_SC_BLANK_ROWS_0, &reg);
	value->period |= (reg<<8);
	value->period *= 6;
	value->minPeriod = value->expo + 2064*6 + 36*6;
	printk("minPeriod:%u \r\n", value->minPeriod);
	return;
}

int sc535_ioctl_set(struct nv_sony_senor *priv, unsigned int cmd, void *arg)
{
    union sensor_ioctl_data data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
    int ret = 0;
	const char *ver;
	int tmpValue = 0;
	SENSOR_EXPO_PERIOD_PARAM mExpoPeriod;
	memset(&data, 0, sizeof(data));
    switch (cmd)
	{
		case V4L2_CID_GET_VERSION:
			vc_info(dev, "get ver:%s \r\n", BL_NV_SC535HGS_FULL_VERSION_EX);
			ver = BL_NV_SC535HGS_FULL_VERSION_EX;
			data.ver.len = strnlen(ver, SENSOR_VER_MAX_LEN - 1);
			memcpy(data.ver.ver, ver, data.ver.len);
			data.ver.ver[data.ver.len] = '\0';
			if (copy_to_user(arg, &data.ver, sizeof(data.ver))) {
            	ret = -EFAULT;
        	}
			break;
        case V4L2_CID_REGIST_IOCTL:
			ret = copy_from_user(&data.reg, (SENSOR_DEBUG_REG_PARAMS __user *)arg, sizeof(data.reg));
			vc_info(dev, "Received custom data.reg opt:%d, addr:%d 0x%x, value:%d\n", \
										data.reg.opt, data.reg.addr, data.reg.addr, data.reg.value);
			sony_sensor_reg_debug_set(priv->s_data, &data.reg);
			vc_info(dev, "Received custom data.reg opt:%d, addr:%d 0x%x, value:%d\n", \
										data.reg.opt, data.reg.addr, data.reg.addr, data.reg.value);
			ret = copy_to_user((SENSOR_DEBUG_REG_PARAMS __user *)arg, &data.reg, sizeof(data.reg));
			break;
		case CAM_GET_SENSOR_TYPE:
			ret = (int)SC535HGS;
			break;
		case CAM_SET_EXPO_PERIOD_PARAM:
			ret = copy_from_user(&mExpoPeriod, (SENSOR_DEBUG_REG_PARAMS __user *)arg, sizeof(mExpoPeriod));
			vc_info(dev, "type:%d expo:%u period:%u \r\n", mExpoPeriod.type, mExpoPeriod.expo, mExpoPeriod.period);
			sc535hgs_set_expo_period(priv->s_data, &mExpoPeriod);
			ret = copy_to_user((SENSOR_DEBUG_REG_PARAMS __user *)arg, &mExpoPeriod, sizeof(mExpoPeriod));
			break;
        case CAM_SET_ROI_FORMAT:
            break;
		case V4L2_CID_GET_TEMPERATURE_VAL:
			{
				tmpValue = sc535hgs_get_temp(priv); //返回的温度值精确到两位小数点
				vc_info(dev, "get tempValue:%d \r\n", tmpValue);
				ret = copy_to_user((int __user *)arg, &tmpValue, sizeof(tmpValue));
			}
			break;
    	default:
			break;
	}
	return ret;
}
static int sc535_s_ctrl(struct v4l2_ctrl *ctrl)
{
// #ifdef USR_DEBUG_ENABLE
// 	struct nv_sony_senor *priv = container_of(ctrl->handler, struct nv_sony_senor, ctrl_handler);
// 	struct camera_common_data	*s_data = priv->s_data;
// #endif
	int err = 0;
	switch (ctrl->id) {
		case V4L2_CID_CONTRAST:
        case V4L2_CID_TEST_PATTERN:
            break;
        default:
			break;
    }
	return err;
}

static const struct v4l2_ctrl_ops sc535_ctrl_ops = {
	.s_ctrl = sc535_s_ctrl,
};

static int sc535_ctrls_init(struct nv_sony_senor *priv)
{
    int err = 0;
	struct device *dev = priv->s_data->dev;
	v4l2_ctrl_handler_init(&priv->ctrl_handler, 4);
    priv->black = v4l2_ctrl_new_std(&priv->ctrl_handler, &sc535_ctrl_ops, V4L2_CID_BLACK_LEVEL, 0, 2048, 1, SENSOR_DEFAULT_BLACK_VALUE);
	priv->gain = v4l2_ctrl_new_std(&priv->ctrl_handler, &sc535_ctrl_ops, V4L2_CID_GAIN, 0, 48, 1, 0);//默认值为0
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

const struct nv_sensor_model_info sc535_i2c_info = {
	.usr_id = SC535HGS,
	.i2c_address = 0x30,
	.name = "sc535",
	.input_freq = 20000,
	.pixel_width = SC535_WIDTH,
	.pixel_height = SC535_HEIGHT,
	.pixel_bit = SENSOR_10_BIT,
	.sensorMode = SEQUENTIAL_TRIGGER_MODE,
	.map_config = &sc535_regmap_config,
	.cam_com_ops = &sc535_common_ops,
	.dtb_init = &sc535_dtb_init,
	.board_init = &sc535_board_setup,
	.sensor_init_param = &sc535_init_param,
	.sensor_usr_set = &sc535_read_id_type,
	.sensor_ioctl_set = &sc535_ioctl_set,
	.sensor_stream_set = NULL,
	.sensor_fmt_set = &sensor_set_fmt,
	.sensor_fmt_get = &sensor_get_fmt,
	.sensor_set_selection = sensor_set_selection,
	.sensor_get_selection = sensor_get_selection,
	.sensor_ctrls_init    = sc535_ctrls_init,
};