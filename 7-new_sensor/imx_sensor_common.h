#ifndef _IMX_SENSOR_COMMON_H_
#define _IMX_SENSOR_COMMON_H_

#include "nv_sensor_common.h"

__u8 usr_set_bit(__u8 num, __u8 bit, __u8 value);

int imx_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val);
int imx_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val);
                
void sony_sensor_read_id_type(struct nv_sony_sensor *priv);
/**
 * @brief 对齐soc底层出来的pwm波形,pwm最高时408MHz进行256分频最小周期值是628uS
 * 
 * @param hmax ：sensor寄存器的值
 * @param input_freq ：输入sensor的时钟值
 * @return __u32 : 返回和底层pwm的值对齐的参数
 */
__u32 orin_calculate_pwm_period(__u32 *hmax, __u32 input_freq);

int sony_sensor_imx56x_get_temp(struct nv_sony_sensor *priv);

int sony_sensor_reg_debug_set(struct camera_common_data *s_data, SENSOR_DEBUG_REG_PARAMS *data);

const struct regmap_config *sensor_get_regmap_config(void);

int sensor_power_get(struct tegracam_device *tc_dev);
int sensor_power_put(struct tegracam_device *tc_dev);
int sensor_set_mode(struct tegracam_device *tc_dev);
int sensor_power_on(struct camera_common_data *s_data);
int sensor_power_off(struct camera_common_data *s_data);
struct camera_common_pdata *sensor_parse_dt(struct tegracam_device *tc_dev);
int sensor_write_table(struct camera_common_data *s_data,
				const SENSOR_REG_STRUCT table[]);
int sensor_set_fmt(struct nv_sony_sensor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format);
int sensor_get_fmt(struct nv_sony_sensor *priv, struct v4l2_subdev *sd, struct v4l2_subdev_format *format);
int sensor_set_selection(struct nv_sony_sensor *priv, struct v4l2_subdev_state *state, struct v4l2_subdev_selection *sel);
int sensor_get_selection(struct nv_sony_sensor *priv, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_selection *sel);

void sensor_reg_info_print(struct device *dev, int set, struct reg_8 *reg);

#endif
