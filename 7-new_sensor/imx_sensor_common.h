
#ifndef _IMX_SENSOR_COMMON_H_
#define _IMX_SENSOR_COMMON_H_

#include "nv_sensor_common.h"

__u8 usr_set_bit(__u8 num, __u8 bit, __u8 value);

int imx_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val);
int imx_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val);
                
void sony_sensor_read_id_type(struct nv_sony_senor *priv);
/**
 * @brief 对齐soc底层出来的pwm波形,pwm最高时408MHz进行256分频最小周期值是628uS
 * 
 * @param hmax ：sensor寄存器的值
 * @param input_freq ：输入sensor的时钟值
 * @return __u32 : 返回和底层pwm的值对齐的参数
 */
__u32 orin_calculate_pwm_period(__u32 *hmax, __u32 input_freq);

int sony_sensor_imx56x_get_temp(struct nv_sony_senor *priv);

int sony_sensor_reg_debug_set(struct camera_common_data *s_data, SENSOR_DEBUG_REG_PARAMS *data);


#endif
