
#include <nvidia/conftest.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/version.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/of_graph.h>
#include "imx566_mode_tbls.h"
#include "imx565_mode_tbls.h"
#include "sc535hgs_mode_tbls.h"
#include "gmax3405_mode_tbls.h"
#include "og02c1b_mode_tbls.h"
#include "ov5640_mode_tbls.h"
#include "og05b2b_mode_tbls.h"
#include "gmax3412_mode_tbls.h"
#include <linux/pwm.h>

#include "imx_sensor_common.h"

static int modeType = 0; //默认序列抓怕模式
module_param(modeType, int, 0644);
MODULE_PARM_DESC(modeType, "mode of sensor, 0-SEQUENTIAL_TRIGGER_MODE 1-NORMAL_MODE");

static const struct of_device_id nv_sensor_of_match[] = {
	{.compatible = "sony,imx566", .data = &imx566_i2c_info},
	{.compatible = "sony,imx565", .data = &imx565_i2c_info},
	{.compatible = "sony,sc535", .data = &sc535_i2c_info},
	{.compatible = "sony,gmax3405", .data = &gmax3405_i2c_info},
	{.compatible = "omnivision,max96717_og02c1b", .data = &og02c1b_i2c_info},
	{.compatible = "zuojisi,z_ov5640_9295", .data = &ov5640_i2c_info},
	{.compatible = "omnivision,max96717_og05b2b", .data = &og05b2b_i2c_info},
	{.compatible = "omnivision,max96717_gmax3412", .data = &gmax3412_i2c_info},
	{},
};

MODULE_DEVICE_TABLE(of, nv_sensor_of_match);
static int nv_sensor_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_stream_set)
		m_Mode_Info->sensor_stream_set(priv, enable);
	return 0;
}
static int m_sensor_set_fmt(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_fmt_set)
		return m_Mode_Info->sensor_fmt_set(priv, sd, format);
	return 0;
}
static int m_sensor_get_fmt(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_fmt_get)
		return m_Mode_Info->sensor_fmt_get(priv, sd, format);
	return 0;
}
static int nv_sensor_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	return 0;
}
static int sensor_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct camera_common_power_rail *pw;
	if (!s_data)
		return -EINVAL;
	pw = s_data->power;
	*status = pw->state == SWITCH_ON;
	return 0;
}
long sensor_camera_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	int ret = 0;
	if(NULL != m_Mode_Info->sensor_ioctl_set)
	{
		ret = m_Mode_Info->sensor_ioctl_set(priv, cmd, arg);
	}
	return ret;
}
static int camera_set_selection(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *state,
				   struct v4l2_subdev_selection *sel)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_set_selection)
		return m_Mode_Info->sensor_set_selection(priv, state, sel);
	return 0;
}
static int camera_get_selection(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_selection *sel)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_get_selection)
		return m_Mode_Info->sensor_get_selection(priv, sd_state, sel);
	return 0;
}
static const struct v4l2_subdev_internal_ops nv_sensor_subdev_internal_ops = {
	.open = nv_sensor_open,
};
static struct v4l2_subdev_core_ops v4l2sd_core_ops = {
	.s_power = camera_common_s_power,
	.ioctl = sensor_camera_ioctl,
};
static const struct v4l2_subdev_video_ops nv_sensor_video_ops = {
	.s_stream = nv_sensor_set_stream,
	.g_input_status = sensor_g_input_status,
};
static const struct v4l2_subdev_pad_ops nv_sensor_pad_ops = {
	.set_fmt = m_sensor_set_fmt,
	.get_fmt = m_sensor_get_fmt,
	.enum_mbus_code = camera_common_enum_mbus_code,
	.enum_frame_size = camera_common_enum_framesizes,
	.enum_frame_interval = camera_common_enum_frameintervals,
	.get_mbus_config = camera_common_get_mbus_config,
	.get_selection = camera_get_selection,
	.set_selection = camera_set_selection,
};
static const struct v4l2_subdev_ops nv_sensor_subdev_ops = {
	.core	= &v4l2sd_core_ops,
	.video = &nv_sensor_video_ops,
	.pad = &nv_sensor_pad_ops,
};

static ssize_t debugfs_get_nv_name(struct file *file,
                           char __user *buf,
                           size_t count, loff_t *ppos)
{
	char tmp[32] = {0};
	struct nv_sony_sensor *priv = file->private_data;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	int len;
	len = snprintf(tmp, sizeof(tmp),
		       "%s\n", m_Mode_Info->name);
	return simple_read_from_buffer(buf, count, ppos, tmp, len);;
}
static ssize_t debugfs_get_nv_id(struct file *file,
                           char __user *buf,
                           size_t count, loff_t *ppos)
{
	char tmp[32] = {0};
	struct nv_sony_sensor *priv = file->private_data;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	int len;
	len = snprintf(tmp, sizeof(tmp),
		       "%d\n", m_Mode_Info->usr_id);
	return simple_read_from_buffer(buf, count, ppos, tmp, len);;
}
int debugfs_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}
static const struct file_operations nv_sensor_get_type_name = {
    .owner = THIS_MODULE,
	.open  = debugfs_open,
    .read  = debugfs_get_nv_name,
};
static const struct file_operations nv_sensor_get_type_id = {
    .owner = THIS_MODULE,
	.open  = debugfs_open,
    .read  = debugfs_get_nv_id,
};
static struct debugfs_node{
    char *name;
    const struct file_operations *fops;
    umode_t perm;
}sensor_fls[] = {
    {.name = "sensor_type", .fops = &nv_sensor_get_type_name, .perm = S_IRUSR}, \
	{.name = "sensor_id", .fops = &nv_sensor_get_type_id, .perm = S_IRUSR}, \
};
static int nv_sensor_debugfs_init(struct nv_sony_sensor *sensor)
{
	u8 i;
	struct dentry *dir;
	dir = debugfs_lookup("nv_imx", NULL);
	if (dir) {
		pr_info("debugfs nv_imx already exists, skip create\n");
		sensor->debugfs_dir = dir;
		return 0;
	}
	sensor->debugfs_dir = debugfs_create_dir("nv_imx", NULL);
	if (IS_ERR_OR_NULL(sensor->debugfs_dir))
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(sensor_fls); i++)
		debugfs_create_file(sensor_fls[i].name, sensor_fls[i].perm, sensor->debugfs_dir,
				    sensor, sensor_fls[i].fops);
    return 0;
}
static void nv_sensor_debugfs_remove(struct nv_sony_sensor *priv)
{
	debugfs_remove_recursive(priv->debugfs_dir);
	priv->debugfs_dir = NULL;
}

static int nv_sensor_proc_init(struct nv_sony_sensor *sensor)
{
	return 0;
}

static const u32 ctrl_cid_list[] = {  //支持原生的tegracam的配置参数
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_FRAME_RATE,
	TEGRA_CAMERA_CID_SENSOR_MODE_ID,
};

static int imx566_set_gain(struct tegracam_device *tc_dev, s64 val)
{
#if 0
	int err = 0;
	struct nv_sony_sensor *priv = (struct nv_sony_sensor *)tegracam_get_privdata(tc_dev);
	struct camera_common_data *s_data = priv->s_data;
	printk("imx566_set_gain set rate val:%lld \r\n", val);
	err = imx_write_reg(s_data, SENSOR_IMX_GAIN_ADDR_L, val&0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_GAIN_ADDR_H, (val>>8)&0x01);
#endif
	return 0;
}
static int imx219_set_exposure(struct tegracam_device *tc_dev, s64 val)//单位微秒
{
#if 0
	struct nv_sony_sensor *priv = (struct nv_sony_sensor *)tegracam_get_privdata(tc_dev);
	struct camera_common_data *s_data = priv->s_data;
	int err = 0;
	u32 shs = 0;
	if(10 > val)
	{
		priv->expoValue = 10;
	}
	else if(2000000>val)
	{
		priv->expoValue = val;
	}
	else
	{
		priv->expoValue = 2000000;
	}
	shs = 1000/priv->expoValue - 3;
	shs = priv->period/shs;
	if(priv->vmax > shs)
	{
		shs = priv->vmax - shs;
	}
	else
	{
		shs = priv->vmax -10;
	}
	err = imx_write_reg(s_data, SENSOR_IMX_SHS_0, shs & 0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_SHS_1,shs >> 8 & 0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_SHS_2,shs >> 16 & 0xff);
	printk("imx219_set_exposure set val:%d \r\n", priv->expoValue);
#endif
	return 0;
}
static int imx219_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
#if 0
	int err = 0;
	u32 shs = 0;
	struct nv_sony_sensor *priv = (struct nv_sony_sensor *)tegracam_get_privdata(tc_dev);
	struct camera_common_data *s_data = priv->s_data;
	printk("imx219_set_frame_rate set rate val:%lld \r\n", val);
	if(val < 10)
	{
		priv->vmax = 1000000000/(priv->period*10) - 1;
	}
	else if(val < 40)
	{
		priv->vmax = 1000000000/(priv->period*val) - 1;
	}
	else
	{
		priv->vmax = 1000000000/(priv->period*5) - 1;
	}
	shs = 1000/priv->expoValue - 3;
	shs = priv->period/shs;
	if(priv->vmax > shs)
	{
		shs = priv->vmax - shs;
	}
	else
	{
		shs = priv->vmax -10;
	}
	err = imx_write_reg(s_data, SENSOR_IMX_VMAX_0, priv->vmax&0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_VMAX_1, (priv->vmax>>8)&0x01);
	err |= imx_write_reg(s_data, SENSOR_IMX_VMAX_2, (priv->vmax>>8)&0x01);
	err |= imx_write_reg(s_data, SENSOR_IMX_SHS_0, shs & 0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_SHS_1,shs >> 8 & 0xff);
	err |= imx_write_reg(s_data, SENSOR_IMX_SHS_2,shs >> 16 & 0xff);
#endif
	return 0;
}

static struct tegracam_ctrl_ops imx566_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = imx566_set_gain,
	.set_exposure = imx219_set_exposure,
	.set_frame_rate = imx219_set_frame_rate,
};

static int nv_sensor_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	static u8 i = 0;
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	struct nv_sony_sensor *priv;
	const struct nv_sensor_model_info *m_Mode_Info = NULL;
	int err = 0;
	if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
		return -EINVAL;
	dev_info(dev, "branch:%s %s time:%s\n", GIT_BRANCH, GIT_COMMIT, PROGRAM_DATE);
	priv = devm_kzalloc(dev,sizeof(struct nv_sony_sensor), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->model_info = of_device_get_match_data(dev);
	if (!priv->model_info)
		return -EINVAL;
	m_Mode_Info = priv->model_info;
	vc_info(dev, "model_info->usr_id:%d modeType:%d\r\n", priv->model_info->usr_id, modeType);
	if(1 == modeType || 0 == modeType)
	{
		priv->modeType = (MODE_TYPE)modeType;
	}
	tc_dev = devm_kzalloc(dev,sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;
	priv->i2c_client = tc_dev->client = client;
	tc_dev->dev = dev;
	snprintf(tc_dev->name, sizeof(tc_dev->name), "%s_%d",
         priv->model_info->name, i++);
	vc_info(dev, "tc_dev->name:%s \r\n", tc_dev->name);
	
	tc_dev->dev_regmap_config = m_Mode_Info->map_config;
	tc_dev->sensor_ops = m_Mode_Info->cam_com_ops;
	tc_dev->v4l2sd_internal_ops = &nv_sensor_subdev_internal_ops;
	imx566_ctrl_ops.set_group_hold = m_Mode_Info->sensor_set_group_hold;
	tc_dev->tcctrl_ops = &imx566_ctrl_ops;
	tc_dev->v4l2sd_ops = &nv_sensor_subdev_ops;
	if(NULL != m_Mode_Info->dtb_init)
	{
		err = m_Mode_Info->dtb_init(priv);
		if(err)
		{
			vc_err(dev, "dtb_init return:%d \r\n", err);
			return err;
		}
	}
	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}
	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;
	tegracam_set_privdata(tc_dev, (void *)priv);
	priv->input_freq = m_Mode_Info->input_freq;
	priv->bit = m_Mode_Info->pixel_bit; //配置默认bit参数
	priv->sensorMode = m_Mode_Info->sensorMode;
	priv->height = m_Mode_Info->pixel_height;
	priv->width = m_Mode_Info->pixel_width;
	priv->x = 0;
	priv->y = 0;
	priv->vflip_status = 0;
	priv->binning = 0;
	if (NULL != m_Mode_Info->board_init)
	{
		err = m_Mode_Info->board_init(priv, priv->modeType);
		if (err)
		{
			tegracam_device_unregister(tc_dev);
			dev_err(dev, "board setup failed\n");
			return err;
		}
	}

	if(NULL != m_Mode_Info->sensor_init_param)
		m_Mode_Info->sensor_init_param(priv, modeType);

	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "sensor v4l2 subdev registration failed\n");
		tegracam_v4l2subdev_unregister(tc_dev);
		tegracam_device_unregister(tc_dev);
		return err;
	}
	if(NULL != m_Mode_Info->sensor_usr_set)
		m_Mode_Info->sensor_usr_set(priv);
	
	err = nv_sensor_debugfs_init(priv);
	if (err)
		vc_warn(&client->dev, "debugfs create failed\n");

	err = nv_sensor_proc_init(priv);
	if (err)
		vc_warn(&client->dev, "proc create failed\n");

	return 0;
}
#if defined(NV_I2C_DRIVER_STRUCT_REMOVE_RETURN_TYPE_INT) /* Linux 6.1 */
static int
nv_sensor_remove(struct i2c_client *client)
#else
static void
nv_sensor_remove(struct i2c_client *client)
#endif
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_sensor *priv;
	const struct nv_sensor_model_info *m_Mode_Info = NULL;
	if (!s_data)
#if defined(NV_I2C_DRIVER_STRUCT_REMOVE_RETURN_TYPE_INT) /* Linux 6.1 */
		return -EINVAL;
#else
		return;
#endif
	priv = (struct nv_sony_sensor *)s_data->priv;
	m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->board_off_power)
	{
		m_Mode_Info->board_off_power(priv);
	}
	nv_sensor_debugfs_remove(priv);
	tegracam_v4l2subdev_unregister(priv->tc_dev);
	tegracam_device_unregister(priv->tc_dev);
#if defined(NV_I2C_DRIVER_STRUCT_REMOVE_RETURN_TYPE_INT) /* Linux 6.1 */
	return 0;
#endif
}
static const struct i2c_device_id nv_sensor_id[] = {
	{ "nv_sensor", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nv_sensor_id);
static struct i2c_driver nv_sensor_i2c_driver = {
	.driver = {
		.name = "nv_sensor",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nv_sensor_of_match),
	},
	.probe = nv_sensor_probe,
	.remove = nv_sensor_remove,
	.id_table = nv_sensor_id,
};
module_i2c_driver(nv_sensor_i2c_driver);
MODULE_DESCRIPTION("Media Controller driver for Sony IMX566");
MODULE_VERSION(BL_NV_SENSOR_FULL_VERSION);
MODULE_AUTHOR("yz");
MODULE_LICENSE("GPL v2");