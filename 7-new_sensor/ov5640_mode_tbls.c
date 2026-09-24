#include "ov5640_mode_tbls.h"
#include "imx_sensor_common.h"
#include "max96724.h"
#include "gmsl_status.h"

#define OV5640_WIDTH   2592
#define OV5640_HEIGHT  1944

#define OV5640_FULL_VERSION_EX \
    "ov5640 " BL_NV_SENSOR_FULL_VERSION

static struct mutex serdes_lock__;

static const struct regmap_config ov5640_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
    .cache_type = REGCACHE_NONE,
	.use_single_read = true,
	.use_single_write = true,
};
enum {
    OV5640_9295_MODE_1920x1080_30FPS,
    OV5640_9295_START_STREAM,
    OV5640_9295_STOP_STREAM,
};

static struct reg_8 ov5640_9295_mode_1920x1080_30fps[] = {
    {0x3103, 0x11}, //select CLOCK from PAD
    {0x3008, 0x82}, //SW RESET
    {SENSOR_TABLE_WAIT_MS, 10},
    {0x3008, 0x42}, //SW Power Down
    {0x3103, 0x03},
    {0x3017, 0x00},
    {0x3018, 0x00},
    {0x3034, 0x18},
    {0x3035, 0x11},
    {0x3036, 0x54},
    {0x3037, 0x13},
    {0x3108, 0x01},
    {0x3630, 0x36},
    {0x3631, 0x0e},
    {0x3632, 0xe2},
    {0x3633, 0x12},
    {0x3621, 0xe0},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3905, 0x02},
    {0x3906, 0x10},
    {0x3901, 0x0a},
    {0x3731, 0x12},
    {0x3600, 0x08},
    {0x3601, 0x33},
    {0x302d, 0x60},
    {0x3620, 0x52},
    {0x371b, 0x20},
    {0x471c, 0x50},
    {0x3a13, 0x43},
    {0x3a18, 0x00},
    {0x3a19, 0xf8},
    {0x3635, 0x13},
    {0x3636, 0x03},
    {0x3634, 0x40},
    {0x3622, 0x01},
    {0x3c01, 0x34},
    {0x3c04, 0x28},
    {0x3c05, 0x98},
    {0x3c06, 0x00},
    {0x3c07, 0x07},
    {0x3c08, 0x00},
    {0x3c09, 0x1c},
    {0x3c0a, 0x9c},
    {0x3c0b, 0x40},
    {0x3820, 0x40},
    {0x3821, 0x06},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3800, 0x01},
    {0x3801, 0x50},
    {0x3802, 0x01},
    {0x3803, 0xb2},
    {0x3804, 0x08},
    {0x3805, 0xef},
    {0x3806, 0x05},
    {0x3807, 0xf1},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x380c, 0x09},
    {0x380d, 0xc4},
    {0x380e, 0x04},
    {0x380f, 0x60},
    {0x3810, 0x00},
    {0x3811, 0x10},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3618, 0x04},
    {0x3612, 0x2b},
    {0x3708, 0x64},
    {0x3709, 0x12},
    {0x370c, 0x00},
    {0x3a02, 0x04},
    {0x3a03, 0x60},
    {0x3a08, 0x01},
    {0x3a09, 0x50},
    {0x3a0a, 0x01},
    {0x3a0b, 0x18},
    {0x3a0e, 0x03},
    {0x3a0d, 0x04},
    {0x3a14, 0x04},
    {0x3a15, 0x60},
    {0x4001, 0x02},
    {0x4004, 0x06},
    {0x3000, 0x00},
    {0x3002, 0x1c},
    {0x3004, 0xff},
    {0x3006, 0xc3},
    {0x300e, 0x45},
    {0x302e, 0x08},
    {0x4300, 0x32}, //zuojisi 0x30 = YUYV, 0x32 = UYVY
    {0x501f, 0x00},
    {0x4713, 0x02},
    {0x4407, 0x04},
    {0x440e, 0x00},
    {0x460b, 0x37},
    {0x460c, 0x20},
    {0x4837, 0x0a},
    {0x3824, 0x04},
    {0x5000, 0xa7},
    {0x5001, 0x83},
    //zuojisi : AWB
    {0x5180, 0xff},
    {0x5181, 0xf2},
    {0x5182, 0x00},
    {0x5183, 0x14},
    {0x5184, 0x25},
    {0x5185, 0x24},
    {0x5186, 0x09},
    {0x5187, 0x09},
    {0x5188, 0x09},
    {0x5189, 0x75},
    {0x518a, 0x54},
    {0x518b, 0xe0},
    {0x518c, 0xb2},
    {0x518d, 0x42},
    {0x518e, 0x3d},
    {0x518f, 0x56},
    {0x5190, 0x46},
    {0x5191, 0xf8},
    {0x5192, 0x04},
    {0x5193, 0x70},
    {0x5194, 0xf0},
    {0x5195, 0xf0},
    {0x5196, 0x03},
    {0x5197, 0x01},
    {0x5198, 0x04},
    {0x5199, 0x12},
    {0x519a, 0x04},
    {0x519b, 0x00},
    {0x519c, 0x06},
    {0x519d, 0x82},
    {0x519e, 0x38},
    {0x5381, 0x1e},
    {0x5382, 0x5b},
    {0x5383, 0x08},
    {0x5384, 0x0a},
    {0x5385, 0x7e},
    {0x5386, 0x88},
    {0x5387, 0x7c},
    {0x5388, 0x6c},
    {0x5389, 0x10},
    {0x538a, 0x01},
    {0x538b, 0x98},
    {0x5300, 0x08},
    {0x5301, 0x30},
    {0x5302, 0x10},
    {0x5303, 0x00},
    {0x5304, 0x08},
    {0x5305, 0x30},
    {0x5306, 0x08},
    {0x5307, 0x16},
    {0x5309, 0x08},
    {0x530a, 0x30},
    {0x530b, 0x04},
    {0x530c, 0x06},
    {0x5480, 0x01},
//    {0x5481, 0x08},   zuojisi: default gamma setting is OK
//    {0x5482, 0x14},
//    {0x5483, 0x28},
//    {0x5484, 0x51},
//    {0x5485, 0x65},
//    {0x5486, 0x71},
//    {0x5487, 0x7d},
//    {0x5488, 0x87},
//    {0x5489, 0x91},
//    {0x548a, 0x9a},
//    {0x548b, 0xaa},
//    {0x548c, 0xb8},
//    {0x548d, 0xcd},
//    {0x548e, 0xdd},
//    {0x548f, 0xea},
//    {0x5490, 0x1d},
    {0x5580, 0x02},
    {0x5583, 0x40},
    {0x5584, 0x10},
    {0x5589, 0x10},
    {0x558a, 0x00},
    {0x558b, 0xf8},
    {0x5800, 0x23},
    {0x5801, 0x14},
    {0x5802, 0x0f},
    {0x5803, 0x0f},
    {0x5804, 0x12},
    {0x5805, 0x26},
    {0x5806, 0x0c},
    {0x5807, 0x08},
    {0x5808, 0x05},
    {0x5809, 0x05},
    {0x580a, 0x08},
    {0x580b, 0x0d},
    {0x580c, 0x08},
    {0x580d, 0x03},
    {0x580e, 0x00},
    {0x580f, 0x00},
    {0x5810, 0x03},
    {0x5811, 0x09},
    {0x5812, 0x07},
    {0x5813, 0x03},
    {0x5814, 0x00},
    {0x5815, 0x01},
    {0x5816, 0x03},
    {0x5817, 0x08},
    {0x5818, 0x0d},
    {0x5819, 0x08},
    {0x581a, 0x05},
    {0x581b, 0x06},
    {0x581c, 0x08},
    {0x581d, 0x0e},
    {0x581e, 0x29},
    {0x581f, 0x17},
    {0x5820, 0x11},
    {0x5821, 0x11},
    {0x5822, 0x15},
    {0x5823, 0x28},
    {0x5824, 0x46},
    {0x5825, 0x26},
    {0x5826, 0x08},
    {0x5827, 0x26},
    {0x5828, 0x64},
    {0x5829, 0x26},
    {0x582a, 0x24},
    {0x582b, 0x22},
    {0x582c, 0x24},
    {0x582d, 0x24},
    {0x582e, 0x06},
    {0x582f, 0x22},
    {0x5830, 0x40},
    {0x5831, 0x42},
    {0x5832, 0x24},
    {0x5833, 0x26},
    {0x5834, 0x24},
    {0x5835, 0x22},
    {0x5836, 0x22},
    {0x5837, 0x26},
    {0x5838, 0x44},
    {0x5839, 0x24},
    {0x583a, 0x26},
    {0x583b, 0x28},
    {0x583c, 0x42},
    {0x583d, 0xce},
    {0x5025, 0x00},
    {0x3a0f, 0x30},
    {0x3a10, 0x28},
    {0x3a1b, 0x30},
    {0x3a1e, 0x26},
    {0x3a11, 0x60},
    {0x3a1f, 0x14},
    {SENSOR_TABLE_WAIT_MS, 3},
    {SENSOR_TABLE_END, 0x00}
};

static struct reg_8 ov5640_9295_start_stream[] = {
    {0x3008, 0x02}, //WAKE UP
    {SENSOR_TABLE_WAIT_MS, 5},
    {SENSOR_TABLE_END, 0x00}
};
 
static struct reg_8 ov5640_9295_stop_stream[] = {
    {0x3008, 0x42}, //SW Power Down
    {SENSOR_TABLE_END, 0x00}
};

static SENSOR_REG_STRUCT *mode_table[] = {
    [OV5640_9295_MODE_1920x1080_30FPS] = ov5640_9295_mode_1920x1080_30fps,
    [OV5640_9295_START_STREAM]  = ov5640_9295_start_stream,
    [OV5640_9295_STOP_STREAM]  = ov5640_9295_stop_stream,
};

static const int ov5640_9295_30fps[] = {
    30,
};
static const struct camera_common_frmfmt ov5640_9295_frmfmt[] = {
    {{1920, 1080}, ov5640_9295_30fps, 1, 0, OV5640_9295_MODE_1920x1080_30FPS},
};




static int ov5640_9295_start_streaming(struct tegracam_device *tc_dev)
{
    printk("ov5640_9295_start_streaming.............\r\n");
    return 0;
}

static int ov5640_9295_stop_streaming(struct tegracam_device *tc_dev)
{
    printk("ov5640_9295_stop_streaming.............\r\n");
    return 0;
}

static struct camera_common_sensor_ops ov5640_common_ops = {
	.numfrmfmts = ARRAY_SIZE(ov5640_9295_frmfmt),
	.frmfmt_table = ov5640_9295_frmfmt,		
	.power_on = sensor_power_on,
	.power_off = sensor_power_off,		
	.write_reg = imx_write_reg,		
	.read_reg = imx_read_reg,
	.parse_dt = sensor_parse_dt,
	.power_get = sensor_power_get,	
	.power_put = sensor_power_put,		
	.set_mode = sensor_set_mode,
    .start_streaming = ov5640_9295_start_streaming,
	.stop_streaming = ov5640_9295_stop_streaming,		
};
static int ov5640_board_setup(struct nv_sony_sensor *priv, MODE_TYPE mode)
{
    int err = 0;
	u8 buf[2];
	u8 val;
    struct i2c_client *client = priv->i2c_client;
    z_max96724_lock_link(priv->dser_dev);
    vc_info(&client->dev, "priv->des_link:%d \r\n", priv->des_link);
    err = z_max96724_check_link_status(priv->dser_dev, priv->des_link);
    if (err)
    {
        z_max96724_unlock_link(priv->dser_dev);
        vc_err(&client->dev, " link_%c is occupied err:%d\n", 'A' + priv->des_link, err);
        err = -EINVAL;
    }
    z_max96724_monopolize_link(priv->dser_dev, priv->des_link);
    mdelay(50);
    gmsl_iic_write(client, 0x40, 0x0000, (MAX9295A_ALTER_ADDR_BASE + priv->des_link) << 1);
    mdelay(50);
    err = gmsl_iic_read(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link, 0x000d, &val);
	if (err || (val != 0x91 && val != 0xbf) ) {
		vc_err(&client->dev, "zuojisi access 's 9295 failed\n");
		z_max96724_restore_link(priv->dser_dev);
		z_max96724_unlock_link(priv->dser_dev);
		return -EINVAL;
	}
    err = gmsl_iic_read(client, priv->def_addr, 0x300a, &buf[0]);
	err |= gmsl_iic_read(client, priv->def_addr, 0x300b, &buf[1]);
	if (err || buf[0] != 0x56 || buf[1] != 0x40) {
		vc_err(&client->dev, "zuojisi access 's sensor id failed\n");
		z_max96724_restore_link(priv->dser_dev);
		z_max96724_unlock_link(priv->dser_dev);
		return -EINVAL;
	}
    gmsl_iic_write(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link, 0x0044, priv->act_addr<<1); //dst addr
	gmsl_iic_write(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link, 0x0045, priv->def_addr<<1); //src addr, original sensor addr

    //2lane 修改为4lane需要修改的地方
	gmsl_iic_write(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link,0x0331, (((priv->g_ctx.num_ser_csi_lanes-1)<<4))); 
	gmsl_iic_write(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link,0x0334, 0x70);//phy1 for xcSer inverse polarity
	gmsl_iic_write(client, MAX9295A_ALTER_ADDR_BASE+priv->des_link,0x0335, 0x07);//phy2 for xcSer inverse polarity

    z_max96724_enable_link(priv->dser_dev, priv->des_link);
	z_max96724_restore_link(priv->dser_dev);
	z_max96724_unlock_link(priv->dser_dev);
    return err;
}
static void ov5640_init_param(struct nv_sony_sensor *priv, int modeType)
{
    struct camera_common_data *s_data = priv->s_data;
	sensor_write_table(s_data, mode_table[modeType]);
    msleep_range(10);
}
void ov5640_read_id_type(struct nv_sony_sensor *priv)
{

}
int ov5640_ioctl_set(struct nv_sony_sensor *priv, unsigned int cmd, void *arg)
{
    union sensor_ioctl_data data;
// #ifdef USR_DEBUG_ENABLE
	struct device *dev = priv->s_data->dev;
// #endif
    int ret = 0;
	const char *ver;
	memset(&data, 0, sizeof(data));
    switch (cmd)
	{
		case V4L2_CID_GET_VERSION:
			vc_info(dev, "get ver:%s \r\n", OV5640_FULL_VERSION_EX);
			ver = OV5640_FULL_VERSION_EX;
			data.ver.len = strnlen(ver, SENSOR_VER_MAX_LEN - 1);
			memcpy(data.ver.ver, ver, data.ver.len);
			data.ver.ver[data.ver.len] = '\0';
			if (copy_to_user(arg, &data.ver, sizeof(data.ver))) {
            	ret = -EFAULT;
        	}
			break;
        case CAM_SET_CUSTOM_TEST:
            {
                z_max96724_print_reg_status(priv->dser_dev);
                vc_info(dev, "===========================max9295A===========================\r\n");
                gmsl_status_reg_print(priv->i2c_client, MAX9295A_ALTER_ADDR_BASE + priv->des_link, MAX9295);
            }
            break;
    	default:
            break;
    }
    return ret;
}
int ov5640_stream_set(struct nv_sony_sensor *priv, int enable)
{
    int err = 0;
    struct device *dev = &priv->i2c_client->dev;
    struct camera_common_data *s_data = priv->s_data;
    vc_info(dev, "stream_set enable:%d \r\n", enable);
    if (1 == enable)
    {
        mutex_lock(&serdes_lock__);
        gmsl_iic_write(priv->i2c_client, MAX9295A_ALTER_ADDR_BASE + priv->des_link, 0x0308, 0x64); // enable CSI-B
        gmsl_iic_write(priv->i2c_client, MAX9295A_ALTER_ADDR_BASE + priv->des_link, 0x0311, 0x40); // start z from CSI-B
        gmsl_iic_write(priv->i2c_client, MAX9295A_ALTER_ADDR_BASE + priv->des_link, 0x0002, 0x43); // enable z

        err = z_max96724_start_streaming(priv->dser_dev, &priv->g_ctx);
        if (err)
        {
            mutex_unlock(&serdes_lock__);
            return err;
        }
        err = sensor_write_table(s_data, mode_table[OV5640_9295_START_STREAM]);
        if (err)
        {
            mutex_unlock(&serdes_lock__);
            return err;
        }
        mdelay(200);
        dev_info(dev,"zuojisi:  start streaming - exit\n");
        mutex_unlock(&serdes_lock__);
    }
    else
    {
        z_max96724_stop_streaming(priv->dser_dev, &priv->g_ctx);
        sensor_write_table(s_data, mode_table[OV5640_9295_STOP_STREAM]);
    }
    return err;
}
static int ov5640_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
    return 0;
}
const struct nv_sensor_model_info ov5640_i2c_info = {
	.usr_id = OV5640,
	.i2c_address = 0x14,
	.name = "ov5640",
	.input_freq = 40000,
	.pixel_width = OV5640_WIDTH,
	.pixel_height = OV5640_HEIGHT,
	.pixel_bit = SENSOR_10_BIT,
	.sensorMode = NORMAL_MODE,
	.map_config = &ov5640_regmap_config,
	.cam_com_ops = &ov5640_common_ops,
	.dtb_init = &gmsl_dtb_file_init,
	.board_init = &ov5640_board_setup,
	.sensor_init_param = &ov5640_init_param,
	.sensor_usr_set = &ov5640_read_id_type,
	.sensor_ioctl_set = &ov5640_ioctl_set,
	.sensor_stream_set = &ov5640_stream_set,
	.sensor_fmt_set = &sensor_set_fmt,
	.sensor_fmt_get = &sensor_get_fmt,
	.sensor_set_selection = sensor_set_selection,
	.sensor_get_selection = sensor_get_selection,
    .sensor_set_group_hold = ov5640_set_group_hold,
    .board_off_power = NULL,
};