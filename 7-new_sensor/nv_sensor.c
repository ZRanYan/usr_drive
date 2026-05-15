
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
#include <linux/pwm.h>

static int workMode = 0;
module_param(workMode, int, 0644);
MODULE_PARM_DESC(workMode, "work mode of sensor");
static const struct of_device_id nv_sensor_of_match[] = {
	{ .compatible = "sony,imx566", .data = &imx566_i2c_info  },
	{ .compatible = "sony,imx565", .data = &imx565_i2c_info  },
	{ },
};
MODULE_DEVICE_TABLE(of, nv_sensor_of_match);
static int nv_sensor_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_senor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_stream_set)
		m_Mode_Info->sensor_stream_set(priv, enable);
	return 0;
}
static int sensor_set_fmt(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_senor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_fmt_set)
		return m_Mode_Info->sensor_fmt_set(priv, sd, format);
	return 0;
}
static int sensor_get_fmt(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_senor *priv = s_data->priv;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if(NULL != m_Mode_Info->sensor_fmt_set)
		return m_Mode_Info->sensor_fmt_set(priv, sd, format);
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
static long sensor_camera_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct nv_sony_senor *priv = s_data->priv;
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
	struct nv_sony_senor *priv = s_data->priv;
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
	struct nv_sony_senor *priv = s_data->priv;
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
	.set_fmt = sensor_set_fmt,
	.get_fmt = sensor_get_fmt,
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
static const struct media_entity_operations sensor_media_ops = {
	.link_validate = v4l2_subdev_link_validate,
};
static int sensor_v4l2subdev_register(struct nv_sony_senor *priv)
{
	int err;
	struct camera_common_data *s_data = priv->s_data;
	struct v4l2_subdev *sd = &s_data->subdev;
	struct device *dev = &priv->i2c_client->dev;
	struct tegracam_device *tc_dev = priv->tc_dev;
	const struct nv_sensor_model_info *m_Mode_Info = priv->model_info;
	if (!s_data)
		return -EINVAL;
	v4l2_i2c_subdev_init(sd, priv->i2c_client, &nv_sensor_subdev_ops);
	priv->s_data->tegracam_ctrl_hdl->ctrl_ops = tc_dev->tcctrl_ops;
	if(NULL != m_Mode_Info->sensor_ctrls_init)
		m_Mode_Info->sensor_ctrls_init(priv);
	tc_dev->numctrls = 0;
	s_data->numctrls = priv->numctrls;//总共支持多少配置命令
	sd->ctrl_handler = &priv->ctrl_handler;
	sd->internal_ops = &nv_sensor_subdev_internal_ops;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE |
		     V4L2_SUBDEV_FL_HAS_EVENTS;
	s_data->owner = sd->owner;
	sd->owner = NULL;
#if defined(CONFIG_MEDIA_CONTROLLER)
	tc_dev->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.ops = &sensor_media_ops;
	err = tegra_media_entity_init(&sd->entity,
			1, &priv->pad, true, true);
	if (err < 0) {
		dev_err(dev, "unable to init media entity\n");
		return err;
	}
#endif
	err = v4l2_async_register_subdev(sd);
	if (err)
	{
		dev_err(dev, "%s:%d: error %d\n", __func__, __LINE__, err);
		return err;
	}
	return err;
}
static int nv_sensor_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	static u8 i = 0;
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	struct nv_sony_senor *priv;
	const struct nv_sensor_model_info *m_Mode_Info = NULL;
	int err = 0;

	if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
		return -EINVAL;
	dev_info(dev, "branch:%s %s time:%s\n", GIT_BRANCH, GIT_COMMIT, PROGRAM_DATE);
	priv = devm_kzalloc(dev,sizeof(struct nv_sony_senor), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->model_info = of_device_get_match_data(dev);
	if (!priv->model_info)
		return -EINVAL;
	m_Mode_Info = priv->model_info;
	dev_info(dev, "model_info->usr_id:%d\r\n", priv->model_info->usr_id);
	tc_dev = devm_kzalloc(dev,sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;
	priv->i2c_client = tc_dev->client = client;
	tc_dev->dev = dev;
	snprintf(tc_dev->name, sizeof(tc_dev->name), "%s_%d",
         priv->model_info->name, i++);
	vc_info(dev, "tc_dev->name:%s \r\n", tc_dev->name);
	if(NULL != m_Mode_Info->dtb_init)
		m_Mode_Info->dtb_init(priv);
	tc_dev->dev_regmap_config = m_Mode_Info->map_config;
	tc_dev->sensor_ops = m_Mode_Info->cam_com_ops;
	tc_dev->v4l2sd_internal_ops = &nv_sensor_subdev_internal_ops; 
	tc_dev->tcctrl_ops = NULL; 
	tc_dev->v4l2sd_ops = &nv_sensor_subdev_ops;
	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}
	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;
	tegracam_set_privdata(tc_dev, (void *)priv);
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
		err = m_Mode_Info->board_init(priv);
		if (err)
		{
			tegracam_device_unregister(tc_dev);
			dev_err(dev, "board setup failed\n");
			return err;
		}
	}
	if(NULL != m_Mode_Info->sensor_init_param)
		m_Mode_Info->sensor_init_param(priv, workMode);
	err = sensor_v4l2subdev_register(priv);
	if (err) {
		dev_err(dev, "sensor v4l2 subdev registration failed\n");
		return err;
	}
	if(NULL != m_Mode_Info->sensor_usr_set)
		m_Mode_Info->sensor_usr_set(priv);
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
	struct nv_sony_senor *priv;
	if (!s_data)
#if defined(NV_I2C_DRIVER_STRUCT_REMOVE_RETURN_TYPE_INT) /* Linux 6.1 */
		return -EINVAL;
#else
		return;
#endif
	priv = (struct nv_sony_senor *)s_data->priv;
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
MODULE_AUTHOR("bopixel");
MODULE_LICENSE("GPL v2");