#ifndef _GMSL_STATUS_H_
#define _GMSL_STATUS_H_

#include "nv_sensor_common.h"


#define MAX96717_GMAX3412_ALTER_ADDR_BASE 0x42
#define MAX96717_OG05_ALTER_ADDR_BASE 0x42
#define MAX96717_OG02_ALTER_ADDR_BASE 0x42


#define MAX96717_DEV_ID  0x0d

#define MAX96717_PCLK_ADDR_BASE         0x0003
#define MAX96717_REF_VTG0_ADDR_BASE     0x03F0
#define MAX96717_REG6_ADDR_BASE         0x0006
#define MAX96717_PIO_SLEW_1_ADDR_BASE   0x0570

#define MAX96717_GPIO0_A_ADDR_BASE       0x2BE
#define MAX96717_GPIO5_A_ADDR_BASE       0x2CD

#define MAX96717_GPIO7_A_ADDR_BASE       0x02D3
#define MAX96717_GPIO7_B_ADDR_BASE       0x02D4
#define MAX96717_GPIO7_C_ADDR_BASE       0x02D5

struct reg_cfg_cmd {
    uint8_t  op;      // 0x80: 直接写, 0x81: 带掩码写
    uint16_t reg;     // 寄存器地址
    uint8_t  data;    // 写入数据
    uint8_t  mask;    // 掩码（若 op=0x80 则忽略）
};

typedef enum
{
    MAX96717 = 0,
    MAX96724 = 1,
    MAX9295  = 2
}MAX_TYPE;

int gmsl_iic_write(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 data);
int gmsl_iic_read(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 *data);
int device_reg_update_bits(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 mask, u8 val);
void gmsl_status_reg_print(struct i2c_client* client, u8 slaveaddr, MAX_TYPE type);
int gmsl_dtb_file_init(struct nv_sony_sensor *priv);

#endif
