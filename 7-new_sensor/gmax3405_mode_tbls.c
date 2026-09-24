#include "gmax3405_mode_tbls.h"
#include "imx_sensor_common.h"

#define GMAX3405_WIDTH     2448
#define GMAX3405_HEIGHT    2048

#define GAMX3405_FULL_VERSION_EX \
    "gmax3405 " BL_NV_SENSOR_FULL_VERSION

static int gmax3405_board_power_off(struct nv_sony_sensor *priv);

static const struct regmap_config gmax3405_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
    .cache_type = REGCACHE_NONE,
	.use_single_read = true,
	.use_single_write = true,
};
enum {
	GMAX3405_MODE_12BIT,
	GMAX3405_MODE_ROI, 
	GMAX3405_MODE_STOP_STREAM,
	GMAX3405_MODE_TEST_PATTERN
};
static const int gmax3405_80_6fps[] = {
	73,
};

static struct camera_common_frmfmt gmax3405_frmfmt[] = {
	{{GMAX3405_WIDTH, GMAX3405_HEIGHT}, gmax3405_80_6fps, 0, 0,GMAX3405_MODE_12BIT},
};

static struct camera_common_sensor_ops gmax3405_common_ops = {
	.numfrmfmts = ARRAY_SIZE(gmax3405_frmfmt),
	.frmfmt_table = gmax3405_frmfmt,		
	.power_on = sensor_power_on,
	.power_off = sensor_power_off,		
	.write_reg = imx_write_reg,		
	.read_reg = imx_read_reg,		
	.parse_dt = sensor_parse_dt,		
	.power_get = sensor_power_get,	
	.power_put = sensor_power_put,		
	.set_mode = sensor_set_mode,		
};

static int gmax3405_get_temp(struct camera_common_data *s_data);
/**
 * @brief 解析设备树配置的参数
 * 
 * @param priv 
 */
static int gmax3405_dtb_init(struct nv_sony_sensor *priv)
{
    struct device *dev = &priv->i2c_client->dev;
    priv->pwdn_gpio =  devm_gpiod_get(dev, "pwdn", GPIOD_OUT_LOW);
	if (IS_ERR(priv->pwdn_gpio)) {
        	dev_err(dev, "Failed to get pwdn_gpio\n");
        	goto error;
    }
    priv->pwgd_gpio =  devm_gpiod_get(dev, "pwrpg", GPIOD_IN);
    if (IS_ERR(priv->pwgd_gpio)) {
        	dev_err(dev, "Failed to get pwgd_gpio\n");
        	goto error;
    }
    priv->reset_gpio =  devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpio)) {
        	dev_err(dev, "Failed to get reset_gpio\n");
        	goto error;
    }
    priv->clk_gpio =  devm_gpiod_get(dev, "clken", GPIOD_OUT_LOW);
	if (IS_ERR(priv->clk_gpio)) {
        	dev_err(dev, "Failed to get clk_gpio\n");
        	goto error;
    }
    return 0;
error:
	return -1;
}
static int sensor_power_on_set(struct nv_sony_sensor *priv)
{
    u8 val,i;
	int err = 0;
    gpiod_set_value(priv->pwdn_gpio, 1);
	usleep_range(1000, 1010);
	for(i=0;i<200;i++)
    {
        usleep_range(1000, 1010);
        val = gpiod_get_value(priv->pwgd_gpio);
        if(1 == val)
            break;
    }
	usleep_range(100, 110);
	gpiod_set_value(priv->clk_gpio, 1);
	msleep_range(100);
	gpiod_set_value(priv->reset_gpio, 1);
	msleep_range(100);
    val = 0;
    err = imx_read_reg(priv->s_data, GMAX3405_REG_HOLD, &val);
    if(err != 0 || 0 != val)
    {
        goto err_reg;
    }
	return 0;
err_reg:
    gmax3405_board_power_off(priv);
    return -1;
}

// static void gmax3405_read_otp_info(struct camera_common_data *s_data)
// {
// 	u8 reg_val = 0;
//     int i = 0;
//     struct device *dev = s_data->dev;
//     for(i=0;i<4;i++)
//     {
//         imx_write_reg(s_data, GMAX3405_OTP_INDEX,i);
//         msleep_range(5);
//         reg_val = 0;
//         imx_read_reg(s_data, GMAX3405_OTP_DATA, &reg_val);
//         dev_info(dev, "i:%d  0x%x\r\n", i, reg_val);
//     }
// }

static int gmax3405_board_setup(struct nv_sony_sensor *priv, MODE_TYPE mode)
{
	// struct camera_common_data *s_data = priv->s_data;
    // struct device *dev = s_data->dev;
	// u8 reg_val = 1;
	int err = 0;
	err = camera_common_mclk_enable(priv->s_data);
	if (err) {
		return err;
	}
    err = sensor_power_on_set(priv); //硬件上电顺序
	msleep_range(20);
	return err;
}
static int gmax3405_board_power_off(struct nv_sony_sensor *priv)
{
    uint8_t i;
    int val;
    struct camera_common_data *s_data = priv->s_data;
    imx_write_reg(s_data, 0x3301, 0x00);
    msleep_range(1);
    imx_write_reg(s_data, 0x3302, 0x01);
    msleep_range(1);
    gpiod_set_value(priv->reset_gpio, 0);
    usleep_range(1000, 1010);
    gpiod_set_value(priv->clk_gpio, 0);
    usleep_range(1000, 1010);
    gpiod_set_value(priv->pwdn_gpio, 0);
    for(i=0;i<200;i++)
    {
        usleep_range(10000, 10010);
        val = gpiod_get_value(priv->pwgd_gpio);
        if(0 == val)
            break;
    }
    return 0;
}
/* I2C Register Initialization Table for GMX3405 (0x2E00 ~ 0x3813) */
static struct reg_8 gmax3405_12bit_4line_master_init[] = {
    {0x2E00, 0x10},
    {0x2E01, 0x0},
    {0x2E02, 0x0},
    // {0x2E03, 0x3}, //自由出流模式
    // {0x2E04, 0x0},
    {0x2E03, 0x0},//配置外部触发
    {0x2E04, 0x0},
    {0x2E05, 0x8},
    {0x2E06, 0x0},
    {0x2E07, 0x1},
    {0x2E08, 0x0},
    {0x2E09, 0x0},
    {0x2E0A, 0x0},
    {0x2E0B, 0x0},
    {0x2E0C, 0x0},
    {0x2E0D, 0x0},
    {0x2E0E, 0x0},
    {0x2E0F, 0x0},
    {0x2E10, 0xFF},
    {0x2E11, 0xFF},
    {0x2E12, 0xE},
    {0x2E13, 0x8},
    {0x2E14, 0x1},
    {0x2E15, 0x0},
    {0x2E16, 0x0},
    {0x2E17, 0x0},
    {0x2E18, 0x0},
    {0x2E19, 0xE},
    {0x2E1A, 0x8},
    {0x2E1B, 0x0},
    {0x2E1C, 0x0},
    {0x2E1D, 0x8},
    {0x2E1E, 0x0},
    {0x2E1F, 0x0},
    {0x2E20, 0x8},
    {0x2E21, 0x0},
    {0x2E22, 0x0},
    {0x2E23, 0x0},
    {0x2E24, 0x0},
    {0x2E25, 0x0},
    {0x2E26, 0x0},
    {0x2E27, 0x0},
    {0x2E28, 0x0},
    {0x2E29, 0x0},
    {0x2E2A, 0x0},
    {0x2E2B, 0x0},
    {0x2E2C, 0x0},
    {0x2E2D, 0x0},
    {0x2E2E, 0x0},
    {0x2E2F, 0x0},
    {0x2E30, 0x0},
    {0x2E31, 0x0},
    {0x2E32, 0x0},
    {0x2E33, 0x0},
    {0x2E34, 0x0},
    {0x2E35, 0x0},
    {0x2E36, 0x0},
    {0x2E37, 0x0},
    {0x2E38, 0x0},
    {0x2E39, 0x0},
    {0x2E3A, 0x0},
    {0x2E3B, 0x0},
    {0x2E3C, 0x0},
    {0x2E3D, 0x1},
    {0x2E3E, 0x6},
    {0x2E3F, 0x0},
    {0x2E40, 0x2},
    {0x2E41, 0x3},
    {0x2E42, 0x4A},
    {0x2E43, 0x1},
    {0x2E44, 0x2},
    {0x2E45, 0x2},
    {0x2E46, 0x2},
    {0x2E47, 0x1E},
    {0x2E48, 0x1E},
    {0x2E49, 0x6},
    {0x2E4A, 0x0},
    {0x2E4B, 0x7},
    {0x2E4C, 0x0},
    {0x2E4D, 0xA4},
    {0x2E4E, 0x84},
    {0x2E4F, 0x62},
    {0x2E50, 0x6},
    {0x2E51, 0x3C},
    {0x2E52, 0x8C},
    {0x2E53, 0x2},
    {0x2E54, 0x2A},
    {0x2E55, 0xFF},
    {0x2E56, 0xFF},
    {0x2E57, 0xFF},
    {0x2E58, 0xFF},
    {0x2E59, 0x46},
    {0x2E5A, 0x5A},
    {0x2E5B, 0x10},
    {0x2E5C, 0xFF},
    {0x2E5D, 0xFF},
    {0x2E5E, 0xFF},
    {0x2E5F, 0x1},
    {0x2E60, 0x9E},
    {0x2E61, 0x1},
    {0x2E62, 0x9F},
    {0x2E63, 0xFF},
    {0x2E64, 0xFF},
    {0x2E65, 0x5},
    {0x2E66, 0x25},
    {0x2E67, 0xFF},
    {0x2E68, 0xFF},
    {0x2E69, 0x5},
    {0x2E6A, 0x3D},
    {0x2E6B, 0x5A},
    {0x2E6C, 0x9B},
    {0x2E6D, 0xD},
    {0x2E6E, 0x28},
    {0x2E6F, 0x2F},
    {0x2E70, 0x9C},
    {0x2E71, 0x5},
    {0x2E72, 0xFF},
    {0x2E73, 0xFF},
    {0x2E74, 0xFF},
    {0x2E75, 0x6},
    {0x2E76, 0xFF},
    {0x2E77, 0xFF},
    {0x2E78, 0xFF},
    {0x2E79, 0xE},
    {0x2E7A, 0x28},
    {0x2E7B, 0x30},
    {0x2E7C, 0x9C},
    {0x2E7D, 0xB},
    {0x2E7E, 0xC},
    {0x2E7F, 0x2D},
    {0x2E80, 0x2E},
    {0x2E81, 0xE},
    {0x2E82, 0x28},
    {0x2E83, 0x30},
    {0x2E84, 0x9C},
    {0x2E85, 0x26},
    {0x2E86, 0x28},
    {0x2E87, 0x9A},
    {0x2E88, 0x9C},
    {0x2E89, 0x28},
    {0x2E8A, 0x30},
    {0x2E8B, 0x2C},
    {0x2E8C, 0xFF},
    {0x2E8D, 0xFF},
    {0x2E8E, 0xFF},
    {0x2E8F, 0xFF},
    {0x2E90, 0xFF},
    {0x2E91, 0x1},
    {0x2E92, 0x2},
    {0x2E93, 0x5},
    {0x2E94, 0xFF},
    {0x2E95, 0xFF},
    {0x2E96, 0xFF},
    {0x2E97, 0x6},
    {0x2E98, 0xFF},
    {0x2E99, 0xFF},
    {0x2E9A, 0xFF},
    {0x2E9B, 0x24},
    {0x2E9C, 0x1C},
    {0x2E9D, 0x1E},
    {0x2E9E, 0xC},
    {0x2E9F, 0x3},
    {0x2EA0, 0x0},
    {0x2EA1, 0x1},
    {0x2EA2, 0x0},
    {0x2EA3, 0x1},
    {0x2EA4, 0x3},
    {0x2EA5, 0x1},
    {0x2EA6, 0x1},
    {0x2EA7, 0x0},
    {0x2EA8, 0x1},
    {0x2EA9, 0x1E},
    {0x2EAA, 0xC},
    {0x2EAB, 0x0},
    {0x2EAC, 0x0},
    {0x2EAD, 0x1},
    {0x2EAE, 0x12},
    {0x2EAF, 0x1},
    {0x2EB0, 0x1},
    {0x2EB1, 0x0},
    {0x2EB2, 0x0},
    {0x2EB3, 0x0},
    {0x2EB4, 0x0},
    {0x2EB5, 0x0},
    {0x2EB6, 0x0},
    {0x2EB7, 0x0},
    {0x2EB8, 0x0},
    {0x2EB9, 0x5},
    {0x2EBA, 0x0},
    {0x2EBB, 0x0},
    {0x3000, 0x1},
    {0x3001, 0x2},
    {0x3002, 0x2},
    {0x3003, 0x0},
    {0x3004, 0x3},
    {0x3005, 0x0},
    {0x3006, 0x0},
    {0x3007, 0x0},
    {0x3008, 0x0},
    {0x3009, 0x6},
    {0x300A, 0x0},
    {0x300B, 0x1},
    {0x300C, 0x1},
    {0x300D, 0xF},
    {0x300E, 0x0},
    {0x300F, 0x11},
    {0x3010, 0x1},
    {0x3011, 0x1},
    {0x3012, 0x0},
    {0x3013, 0x0},
    {0x3014, 0x65},
    {0x3015, 0x1},
    {0x3016, 0x4},
    {0x3017, 0x0},
    {0x3018, 0x8},
    {0x3019, 0x7},
    {0x301A, 0x10},
    {0x301B, 0x7},
    {0x301C, 0x27},
    {0x301D, 0x0},
    {0x301E, 0xB},
    {0x301F, 0x9},
    {0x3020, 0x5},
    {0x3021, 0x6},
    {0x3022, 0x96},
    {0x3023, 0xF8},
    {0x3024, 0x14},
    {0x3025, 0x0},
    {0x3026, 0x0},
    {0x3027, 0x0},
    {0x3028, 0x0},
    {0x3029, 0x0},
    {0x302A, 0x4},
    {0x302B, 0x4},
    {0x302C, 0x4},
    {0x302D, 0x4},
    {0x302E, 0x0},
    {0x302F, 0x0},
    {0x3030, 0x0},
    {0x3031, 0x0},
    {0x3032, 0x0},
    {0x3033, 0x1F},
    {0x3034, 0x1F},
    {0x3035, 0x8},
    {0x3036, 0x8},
    {0x3037, 0x90},
    {0x3038, 0x9},
    {0x3200, 0x20},
    {0x3201, 0x0},
    {0x3202, 0x4},
    {0x3203, 0x3},
    {0x3204, 0x20},
    {0x3205, 0x3},
    {0x3206, 0x3},
    {0x3207, 0x3},
    {0x3208, 0x16},
    {0x3209, 0x4},
    {0x320A, 0x0},
    {0x320B, 0x16},
    {0x320C, 0x4},
    {0x320D, 0x16},
    {0x320E, 0xF},
    {0x320F, 0x0},
    {0x3210, 0x11},
    {0x3211, 0x7},
    {0x3212, 0x0},
    {0x3213, 0xE},
    {0x3214, 0x1B},
    {0x3215, 0x3},
    {0x3216, 0x21},
    {0x3217, 0x4},
    {0x3218, 0x7},
    {0x3219, 0x0},
    {0x321A, 0x29},
    {0x321B, 0x7},
    {0x321C, 0x0},
    {0x321D, 0x4},
    {0x321E, 0x29},
    {0x321F, 0x4},
    {0x3220, 0x7},
    {0x3221, 0x0},
    {0x3222, 0xF},
    {0x3223, 0x3},
    {0x3224, 0x0},
    {0x3225, 0x0},
    {0x3226, 0x29},
    {0x3227, 0x3},
    {0x3228, 0x0},
    {0x3229, 0x0},
    {0x322A, 0x11},
    {0x322B, 0x3},
    {0x322C, 0x1B},
    {0x322D, 0x0},
    {0x322E, 0x7},
    {0x322F, 0xF},
    {0x3230, 0xE},
    {0x3231, 0x61},
    {0x3232, 0x53},
    {0x3233, 0x4D},
    {0x3234, 0x46},
    {0x3235, 0x43},
    {0x3236, 0x43},
    {0x3237, 0x45},
    {0x3238, 0x4F},
    {0x3239, 0x47},
    {0x323A, 0x53},
    {0x323B, 0x53},
    {0x323C, 0x49},
    {0x323D, 0x49},
    {0x323E, 0x68},
    {0x323F, 0x64},
    {0x3240, 0x0},
    {0x3241, 0x1A},
    {0x3242, 0x23},
    {0x3243, 0x4},
    {0x3244, 0x13},
    {0x3245, 0x13},
    {0x3246, 0x34},
    {0x3247, 0xA},
    {0x3248, 0x7D},
    {0x3249, 0x21},
    {0x324A, 0xF},
    {0x324B, 0x53},
    {0x324C, 0x84},
    {0x324D, 0x1C},
    {0x324E, 0x28},
    {0x324F, 0x7},
    {0x3250, 0x4},
    {0x3251, 0x0},
    {0x3252, 0xF},
    {0x3253, 0x0},
    {0x3254, 0x0},
    {0x3255, 0x0},
    {0x3256, 0x0},
    {0x3257, 0x0},
    {0x3258, 0x0},
    {0x3300, 0x0},
    {0x3301, 0x0},
    {0x3302, 0x0},
    {0x3303, 0x0},
    {0x3304, 0x0},
    {0x3305, 0x0},
    {0x3306, 0x0},
    {0x3307, 0x1},
    {0x3308, 0x0},
    {0x3309, 0x1},
    {0x330A, 0x0},
    {0x330B, 0x1},
    {0x330C, 0x0},
    {0x330D, 0x1},
    {0x330E, 0x0},
    {0x330F, 0x1},
    {0x3310, 0x0},
    {0x3311, 0x1},
    {0x3312, 0x0},
    {0x3313, 0x2C},
    {0x3314, 0x1},
    {0x3315, 0x1},
    {0x3316, 0x0},
    {0x3317, 0xE2},
    {0x3318, 0x4},
    {0x3319, 0xE2},
    {0x331A, 0x4},
    {0x331B, 0xE2},
    {0x331C, 0x4},
    {0x331D, 0xE2},
    {0x331E, 0x4},
    {0x331F, 0xE2},
    {0x3320, 0x4},
    {0x3321, 0xE2},
    {0x3322, 0x4},
    {0x3323, 0xEE},
    {0x3324, 0x2},
    {0x3325, 0xEE},
    {0x3326, 0x2},
    {0x3327, 0x1},
    {0x3328, 0x0},
    {0x3329, 0x1},
    {0x332A, 0x0},
    {0x332B, 0x1},
    {0x332C, 0x0},
    {0x332D, 0x1},
    {0x332E, 0x0},
    {0x332F, 0x77},
    {0x3330, 0x1},
    {0x3331, 0x1},
    {0x3332, 0x0},
    {0x3333, 0xE2},
    {0x3334, 0x4},
    {0x3335, 0xE2},
    {0x3336, 0x4},
    {0x3337, 0xE2},
    {0x3338, 0x4},
    {0x3339, 0xE2},
    {0x333A, 0x4},
    {0x333B, 0xE2},
    {0x333C, 0x4},
    {0x333D, 0xE2},
    {0x333E, 0x4},
    {0x333F, 0x1},
    {0x3340, 0x0},
    {0x3341, 0x1},
    {0x3342, 0x0},
    {0x3343, 0x1},
    {0x3344, 0x0},
    {0x3345, 0x0},
    {0x3346, 0x0},
    {0x3347, 0x0},
    {0x3348, 0x0},
    {0x3349, 0x0},
    {0x334A, 0x0},
    {0x334B, 0x0},
    {0x334C, 0x0},
    {0x334D, 0x10},
    {0x334E, 0x32},
    {0x334F, 0x54},
    {0x3350, 0x86},
    {0x3351, 0xA9},
    {0x3352, 0xCB},
    {0x3353, 0xED},
    {0x3354, 0x7F},
    {0x3355, 0x10},
    {0x3356, 0x32},
    {0x3357, 0x54},
    {0x3358, 0x87},
    {0x3359, 0xA9},
    {0x335A, 0xCB},
    {0x335B, 0xED},
    {0x335C, 0x5F},
    {0x335D, 0x0},
    {0x335E, 0x0},
    {0x335F, 0x0},
    {0x3360, 0x0},
    {0x3361, 0x1},
    {0x3362, 0x0},
    {0x3363, 0x76},
    {0x3364, 0xE8},
    {0x3365, 0x3},
    {0x3366, 0xE8},
    {0x3367, 0x3},
    {0x3368, 0x88},
    {0x3369, 0x13},
    {0x336A, 0x88},
    {0x336B, 0x13},
    {0x336C, 0x0},
    {0x3400, 0x0},
    {0x3401, 0x12},
    {0x3402, 0x0},
    {0x3403, 0x0},
    {0x3404, 0x64},
    {0x3405, 0x2},
    {0x3500, 0x10},
    {0x3501, 0x86},
    {0x3502, 0x0},
    {0x3503, 0x0},
    {0x3504, 0x0},
    {0x3505, 0x0},
    {0x3506, 0x0},
    {0x3507, 0x0},
    {0x3508, 0x0},
    {0x3509, 0x0},
    {0x350A, 0x0},
    {0x350B, 0x0},
    {0x350C, 0x0},
    {0x350D, 0x0},
    {0x350E, 0x0},
    {0x350F, 0x0},
    {0x3510, 0x0},
    {0x3511, 0x90},
    {0x3512, 0x9},
    {0x3513, 0x0},
    {0x3514, 0x0},
    {0x3515, 0x0},
    {0x3516, 0x0},
    {0x3517, 0x0},
    {0x3518, 0x0},
    {0x3519, 0x0},
    {0x351A, 0x0},
    {0x351B, 0x0},
    {0x351C, 0x0},
    {0x351D, 0x0},
    {0x351E, 0x0},
    {0x351F, 0x0},
    {0x3520, 0x0},
    {0x3521, 0x90},
    {0x3522, 0x9},
    {0x3523, 0x53},
    {0x3524, 0x4},
    {0x3525, 0x0},
    {0x3526, 0x0},
    {0x3527, 0x0},
    {0x3528, 0x0},
    {0x3529, 0x0},
    {0x352A, 0x0},
    {0x3700, 0xE0},
    {0x3701, 0x8},
    {0x3702, 0x6E},
    {0x3703, 0x12},
    {0x3704, 0x0},
    {0x3705, 0x80},
    {0x3706, 0x0},
    {0x3707, 0x80},
    {0x3708, 0x1},
    {0x3709, 0x28},
    {0x370A, 0x32},
    {0x370B, 0x0},
    {0x370C, 0x2},
    {0x370D, 0x24},
    {0x370E, 0x3F},
    {0x370F, 0xF},
    {0x3710, 0x64},
    {0x3711, 0x0},
    {0x3712, 0x0},
    {0x3713, 0x3A},
    {0x3800, 0xE0},
    {0x3801, 0x8},
    {0x3802, 0x6E},
    {0x3803, 0x12},
    {0x3804, 0x0},
    {0x3805, 0x80},
    {0x3806, 0x0},
    {0x3807, 0x80},
    {0x3808, 0x1},
    {0x3809, 0x28},
    {0x380A, 0x32},
    {0x380B, 0x0},
    {0x380C, 0x2},
    {0x380D, 0x24},
    {0x380E, 0x3F},
    {0x380F, 0xF},
    {0x3810, 0x64},
    {0x3811, 0x0},
    {0x3812, 0x0},
    {0x3813, 0x3A},
    {SENSOR_TABLE_END, 0x00}
};

static SENSOR_REG_STRUCT *mode_table[] = {
	[GMAX3405_MODE_12BIT] = gmax3405_12bit_4line_master_init,
	[GMAX3405_MODE_ROI] = NULL,
	[GMAX3405_MODE_STOP_STREAM] = NULL,
};

static void gmax3405_init_param(struct nv_sony_sensor *priv, int modeType)
{
	struct camera_common_data *s_data = priv->s_data;
	sensor_write_table(s_data, mode_table[modeType]);
    msleep_range(10);
	imx_write_reg(s_data, 0x2E00, 0x11);
	imx_write_reg(s_data, 0x3301, 0x01);
	msleep_range(90);
	imx_write_reg(s_data, 0x3005, 0x01);
	imx_write_reg(s_data, 0x3024, 0x14);
	msleep_range(2);
	imx_write_reg(s_data, 0x3023, 0xF8);
	msleep_range(2);
	imx_write_reg(s_data, 0x3023, 0xF9);
	msleep_range(2);
	imx_write_reg(s_data, 0x3023, 0xFF);
	msleep_range(2);
	imx_write_reg(s_data, 0x3024, 0x34);
	msleep_range(2);
	imx_write_reg(s_data, 0x3023, 0xFB);
	msleep_range(2);
	imx_write_reg(s_data, 0x2E00, 0x13);
    msleep_range(10);
}

void gmax3405_read_id_type(struct nv_sony_sensor *priv)
{
}

static int gmax3405_get_temp(struct camera_common_data *s_data)
{
    u8 reg_val = 0;
    int32_t val = 0;
    // imx_write_reg(s_data, GMAX3405_D_TEMP_0, 1);
    imx_read_reg(s_data, GMAX3405_D_TEMP_0, &reg_val);
    val = reg_val;
    imx_read_reg(s_data, GMAX3405_D_TEMP_1, &reg_val);
    val |= (reg_val<<8);
    val *= 42;
    val -= 32982;
    return val;
}
/**
 * @brief 配置增益参数，目前仅支持模拟增益参数配置0-40的值
 * 
 * @param s_data 
 * @param val 
 * @return int 
 */
static int gmx3405_sensor_set_gain(struct camera_common_data *s_data, s64 val)
{
    u8 reg_val = 0x12;
    if(val <0 || 40 < val)
    {
        return -1;
    }
    reg_val+=val;
    imx_write_reg(s_data, GMAX3405_REG_HOLD, 0x01);
    imx_write_reg(s_data, GMAX3405_PGA_GAIN, reg_val);
    imx_write_reg(s_data, GMAX3405_REG_HOLD, 0x00);
    return 0;
}

static int gmax3405_sensor_set_black_level(struct camera_common_data *s_data, s64 val)
{
	int err = 0;
	u16 black = 0;
	struct device *dev = s_data->dev;
    black = val & 0xFFF;
    err = imx_write_reg(s_data, GMAX3405_REG_HOLD, 1);
    imx_write_reg(s_data,
                  GMAX3405_DOFF_ODD_H,
                  (black >> 8) & 0x0F);
    imx_write_reg(s_data,
                  GMAX3405_DOFF_ODD_L,
                  black & 0xFF);
    imx_write_reg(s_data,
                  GMAX3405_DOFF_EVEN_H,
                  (black >> 8) & 0x0F);
    imx_write_reg(s_data,
                  GMAX3405_DOFF_EVEN_L,
                  black & 0xFF);
    err |= imx_write_reg(s_data, GMAX3405_REG_HOLD, 0);
    if (err)
		dev_err(dev, "%s: gain control error\n", __func__);
	return err;
}
static int gmax3405_sensor_set_filp(struct camera_common_data *s_data, FILP_TYPE type, s64 val)
{
	u8 value = 0;
	int err = 0;
	struct device *dev = s_data->dev;
    err = imx_write_reg(s_data, GMAX3405_REG_HOLD, 1);
	if(VERTICAL == type)
	{
        err = imx_read_reg(s_data, GMAX3405_FLIP_V, &value);
        value &= ~(0x01);
        value |= (val&0x01);
		err = imx_write_reg(s_data, GMAX3405_FLIP_V, value);
	}
	else if(HORIZONTAL == type)
	{
        err = imx_read_reg(s_data, GMAX3405_FLIP_H, &value);
        value &= ~(0x01);
        value |= (val&0x01);
		err = imx_write_reg(s_data, GMAX3405_FLIP_H, value);
	}
    err = imx_write_reg(s_data, GMAX3405_REG_HOLD, 0);
	if (err)
		dev_err(dev, "%s: flip control error\n", __func__);
	return err;
}

static int gmax3405_sensor_set_test_img(struct camera_common_data *s_data, s64 val)
{
    u8 value = 0;
	int err = 0;
	// struct device *dev = s_data->dev;
    err = imx_read_reg(s_data, GMAX3405_FLIP_H, &value);
    value &= ~(0x07 << 1);
    if(0 == val)
    {
        value |= (0x00 << 1);
    }
    else
    {
        value |= (0x04 << 1);
    }
    err |= imx_write_reg(s_data, GMAX3405_FLIP_H, value);
    return err;
}

static int gmax3405_set_roi_format(struct nv_sony_sensor *priv, SENSOR_FORMAT_ROI_PARAM *roi_param)
{
	int err = 0;
    u16 tmpValue = 134;
    u8 val;
    struct camera_common_data *s_data = priv->s_data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = s_data->dev;
#endif
    priv->x = CALCULATE_ROUND(roi_param->start_x, 16, V4L2_SEL_FLAG_LE);
	priv->y = CALCULATE_ROUND(roi_param->start_y, 8, V4L2_SEL_FLAG_LE);
	priv->width = CALCULATE_ROUND(roi_param->width, 16, V4L2_SEL_FLAG_LE);
	priv->height = CALCULATE_ROUND(roi_param->height, 8, V4L2_SEL_FLAG_LE);
    priv->binning = roi_param->binningMode; //更新binning参数
	priv->bit = roi_param->bitMode;
    priv->vmax = 0;
    // gmax3405_frmfmt[0].size.width = ((0 == priv->binning) ? gmax3405_i2c_info.pixel_width : gmax3405_i2c_info.pixel_width / 2);
    gmax3405_frmfmt[0].size.width = priv->width;
	gmax3405_frmfmt[0].size.height = priv->height;
    // vc_info(dev, "priv->x:%d \r\n", priv->x);
	// vc_info(dev, "priv->y:%d \r\n", priv->y);
	// vc_info(dev, "priv->width:%d \r\n", priv->width);
	// vc_info(dev, "priv->height:%d \r\n", priv->height);
	// vc_info(dev, "priv->hmax:%d \r\n", priv->hmax);
	// vc_info(dev, "priv->binning:%d \r\n", priv->binning);
    vc_warn(dev, "width:%d height:%d\r\n", gmax3405_frmfmt[0].size.width, gmax3405_frmfmt[0].size.height);
    //写入WIN_X_NUM的值为1
    err = imx_write_reg(s_data, GMAX3405_REG_HOLD, 1);
    err = imx_read_reg(s_data, GMAX3405_FLIP_H, &val);
    val &= ~GMAX3405_WIN_X_NUM_MASK;
    val |= ((1 << GMAX3405_WIN_X_NUM_SHIFT) & GMAX3405_WIN_X_NUM_MASK);
    err = imx_write_reg(s_data, GMAX3405_FLIP_H, val);
    tmpValue = 8;
    tmpValue =  tmpValue + priv->y;
    vc_info(dev, "GMAX3405_WIN1_START:%d \r\n",tmpValue);
    imx_write_reg(s_data, GMAX3405_WIN1_START_L, tmpValue&0xFF);
    imx_write_reg(s_data, GMAX3405_WIN1_START_H, (tmpValue>>8)&0xFF);
    tmpValue = priv->height;
    vc_info(dev, "GMAX3405_WIN1_LENGTH:%d \r\n", tmpValue);
    imx_write_reg(s_data, GMAX3405_WIN1_LENGTH_L, tmpValue&0xFF);
    imx_write_reg(s_data, GMAX3405_WIN1_LENGTH_H, (tmpValue>>8)&0xFF);
    priv->vmax += tmpValue;//WIN1_L
    priv->vmax += 12;//计算row的值
    imx_write_reg(s_data, GMAX3405_WIN_ALL_LEN_L, priv->width&0xFF);
    imx_write_reg(s_data, GMAX3405_WIN_ALL_LEN_H, (priv->width>>8)&0x0F);
    imx_write_reg(s_data, GMAX3405_WIN0_X_LENGTH_L, priv->width&0xFF);
    imx_write_reg(s_data, GMAX3405_WIN0_X_LENGTH_H, (priv->width>>8)&0x0F);
    tmpValue = 134;
    tmpValue += priv->x;
    imx_write_reg(s_data, GMAX3405_WIN0_X_START_L, tmpValue&0xFF);
    imx_write_reg(s_data, GMAX3405_WIN0_X_START_H, (tmpValue>>8)&0x0F);
    err = imx_write_reg(s_data, GMAX3405_REG_HOLD, 0);
	return err;
}

// static int gmax3405_set_power_status(struct nv_sony_sensor *priv, bool onOff)
// {
//     if(onOff)//执行上电操作
//     {
//         sensor_power_on_set(priv);
// 	    msleep_range(20);
//         gmax3405_init_param(priv, 0);
//     }
//     else //执行下电操作
//     {
// 	    imx_write_reg(priv->s_data, 0x2E00, 0x11);
//         return gmax3405_board_power_off(priv);
//     }
//     return 0;
// }
void gmax3405_calculate_minimum_interval_time(struct nv_sony_sensor *sen, SENSOR_ATTRIBUTE *param)
{
    uint16_t Tline = 6600; //单位ns
    uint8_t Tfot = 46; //实际的值是46.2us
    uint32_t Trd  = (sen->vmax * Tline)/1000; //帧读出时间为，单位转化成us
    vc_info(&sen->i2c_client->dev, "vmax:%d Trd:%d\r\n", sen->vmax, Trd);
    Trd += Tfot;
    vc_info(&sen->i2c_client->dev, "Trd:%d  %d\r\n", Trd, 10000000/Trd);
    param->max_frame = 10000000/Trd;
}

int gmax3405_ioctl_set(struct nv_sony_sensor *priv, unsigned int cmd, void *arg)
{
    union sensor_ioctl_data data;
#ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
#endif
    int ret = 0;
	const char *ver;
	int tmpValue = 0;
    s64 val;
	memset(&data, 0, sizeof(data));
    switch (cmd)
	{
		case V4L2_CID_GET_VERSION:
			vc_info(dev, "get ver:%s \r\n", GAMX3405_FULL_VERSION_EX);
			ver = GAMX3405_FULL_VERSION_EX;
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
			ret = (int)GMAX3405;
			break;
        case CAM_SET_ROI_FORMAT:
            {
                ret = copy_from_user(&data.roi, (struct custon_params __user *)arg, sizeof(data.roi));
				vc_info(dev, "isEnable:%d roi_params:%d %d %d %d \n",data.roi.isEnable, data.roi.start_x, data.roi.start_y, data.roi.width, data.roi.height);
				ret = gmax3405_set_roi_format(priv, &data.roi);
            }
            break;
        case CAM_SET_GAIN:
            ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
			vc_info(dev,"set CAM_SET_GAIN  %lld \n", val);
			ret = gmx3405_sensor_set_gain(priv->s_data, val);
            break;
		case CAM_SET_BLACK_LEVEL:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_BLACK_LEVEL  %lld \n", val);
				ret = gmax3405_sensor_set_black_level(priv->s_data, val);
			}
			break;
		case CAM_SET_HFLIP:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_HFLIP  %lld \n", val);
				ret = gmax3405_sensor_set_filp(priv->s_data, HORIZONTAL, val);
			}
			break;
		case CAM_SET_VFLIP:
			{
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev,"set CAM_SET_VFLIP  %lld \n", val);
				ret = gmax3405_sensor_set_filp(priv->s_data, VERTICAL, val);
			}
			break;
		case V4L2_CID_GET_TEMPERATURE_VAL:
			{
				tmpValue = gmax3405_get_temp(priv->s_data); //返回的温度值精确到两位小数点
				vc_info(dev, "get tempValue:%d \r\n", tmpValue);
				ret = copy_to_user((int __user *)arg, &tmpValue, sizeof(tmpValue));
			}
			break;
        case CAM_SET_SENSOR_POWER_STATUS:
            {
				ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
				vc_info(dev, "set CAM_SET_SENSOR_POWER_STATUS:%lld \r\n", val);
                if(0 == val || 1 == val)
                {
                    // ret = gmax3405_set_power_status(priv, (bool)val);
                }
                else
                {
            	    ret = -EFAULT;
                }
            }
            break;
        case CAM_GET_MIN_EXPOSE:
            gmax3405_calculate_minimum_interval_time(priv, &data.min_interval);
			vc_info(dev, "max_frame:%d exposureUnit:%d min_trigger_fall:%d min_trigger_rise:%d\n", \
					data.min_interval.max_frame, data.min_interval.exposureUnit, data.min_interval.min_trigger_fall, data.min_interval.min_trigger_rise);
			ret = copy_to_user((SENSOR_ATTRIBUTE __user *)arg, &data.min_interval, sizeof(data.min_interval));
            break;
        case CAM_SET_CUSTOM_TEST:
            {
                ret = copy_from_user(&val, (s64 __user *)arg, sizeof(val));
                vc_info(dev,"set CAM_SET_CUSTOM_TEST  %lld \n", val);
                ret = gmax3405_sensor_set_test_img(priv->s_data, val);
            }
            break;
    	default:
			break;
	}
	return ret;
}

static int gmax3405_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	return 0;
}

// static int gmax3405_stream_set(struct nv_sony_sensor *priv, int enable)
// {
// 	struct camera_common_data *s_data = priv->s_data;
// 	// struct device *dev = s_data->dev;
//     u8 reg_val = 0;
//     imx_read_reg(s_data, GMAX3405_STREAM_EN, &reg_val);
//     if(0 == enable)
//     {
//         reg_val &= ~(1 << 1);
//     }
//     else
//     {
//         reg_val |= (1 << 1);
//     }
//     imx_write_reg(s_data, GMAX3405_STREAM_EN, reg_val);
//     return 0;
// }

const struct nv_sensor_model_info gmax3405_i2c_info = {
	.usr_id = GMAX3405,
	.i2c_address = 0x10,
	.name = "gmax3405",
	.input_freq = 40000,
	.pixel_width = GMAX3405_WIDTH,
	.pixel_height = GMAX3405_HEIGHT,
	.pixel_bit = SENSOR_12_BIT,
	.sensorMode = SEQUENTIAL_TRIGGER_MODE,
	.map_config = &gmax3405_regmap_config,
	.cam_com_ops = &gmax3405_common_ops,
	.dtb_init = &gmax3405_dtb_init,
	.board_init = &gmax3405_board_setup,
	.sensor_init_param = &gmax3405_init_param,
	.sensor_usr_set = &gmax3405_read_id_type,
	.sensor_ioctl_set = &gmax3405_ioctl_set,
	.sensor_stream_set = NULL,
	.sensor_fmt_set = &sensor_set_fmt,
	.sensor_fmt_get = &sensor_get_fmt,
	.sensor_set_selection = sensor_set_selection,
	.sensor_get_selection = sensor_get_selection,
    .sensor_set_group_hold = gmax3405_set_group_hold,
    .board_off_power = &gmax3405_board_power_off,
};