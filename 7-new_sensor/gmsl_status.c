#include "gmsl_status.h"
#include "imx_sensor_common.h"


static ERROR_REG_SET g_max96717_err_reg[] =
{
    {"REG1", 0x0001, "[7][6][5]"},
    {"CTRL3", 0x0013, "[3][1]"},
    /* ================= 视频出流 (VID_TX) 错误 ================= */
    // 注：MAX96717 仅有 Pipe Z (0x0112)
    {"vid_tx2_pipe_z", 0x0112, "[4],[5],[6],[7]"}, // Pipe Z 出流状态 (FIFO_WARN, OVERFLOW, DRIFT_ERR, PCLKDET)
    /* =============== Tunneling 模式 FIFO 错误 =============== */
    {"tun_fifo_ovfl", 0x0380, "[1]"}, // EXT8 寄存器: 隧道模式 FIFO 溢出 (tun_fifo_overflow)
    {"tun mode", 0x383, "[7]"},
    /* ================ MIPI D-PHY 层 (LP) 错误 ================ */
    // 包含 Escape 序列错误、无效行状态等
    {"phy1_lp_err", 0x033B, "[0]~[4]"}, // MIPI PHY1 LP 层错误
    {"phy2_lp_err", 0x033D, "[0]~[4]"}, // MIPI PHY2 LP 层错误
    /* ================ MIPI D-PHY 层 (HS) 错误 ================ */
    // 包含 Sync pattern 位错 (bit0~3) 和 Skew 校准错误 (bit4~5)
    {"phy1_hs_err", 0x033C, "[0]~[5]"},  // MIPI PHY1 HS 层错误
    {"phy2_hs_err", 0x033E, "[0]~[5]"},  // MIPI PHY2 HS 层错误
    {"phy1_pkt_cnt", 0x038D, "[0]~[7]"}, // MIPI PHY1 pkt cnt
    {"csi1_pkt_cnt", 0x038E, "[7~0]"},   // Packet count of CSI-2 Controller
    {"tun_pkt_cnt", 0x038F, "[7~0]"},    // Packet count of CSI-2 Controller
    {"phy_clk_cnt", 0x0390, "[0]~[7]"},  // MIPI Tunnel Packets Processed
    /* ============== MIPI CSI-2 控制器 (Ctrl) 错误 ============== */
    // 注：MAX96717 仅使用 Ctrl 1
    {"ctrl1_csi_err_l", 0x0343, "[0],[1],[7]"}, // CSI 控制器低位错误 (1-bit ECC, 2-bit ECC, CRC)
    {"ctrl1_csi_err_h", 0x0344, "[0],[1],[2]"}, // CSI 控制器高位错误 (包提前终止, 帧计数错误, 不支持的数据类型)
};

static ERROR_REG_SET g_max96724_err_reg[] = 
{
	{"GMSL2 link status", 0x001A, "[3][2][1][0]"},
	{"VPRBS", 0x01DC, "[7-0]"},
	{"VPRBS", 0x01FC, "[7-0]"},
	{"VPRBS", 0x021C, "[7-0]"},
	{"VPRBS", 0x023C, "[7-0]"},
	{"Error Packet", 0x0423, "[7][5:0]"}, //打包类型
	{"Error Packet", 0x0424, "[7][5:0]"}, //打包类型
	{"Error Packet", 0x0425, "[7][5:0]"}, //打包类型
	{"Error Packet", 0x0426, "[7][5:0]"}, //打包类型
	{"MIPI ", 0x0936, "[7-0]"},
	{"MIPI ", 0x0976, "[7-0]"},
	{"MIPI ", 0x09B6, "[7-0]"},
	{"MIPI ", 0x09F6, "[7-0]"},
	{"CSI-2 1-0 COUNT", 0x08D0, 	"[7-4][3-0]"},
	{"CSI-2 2-3 COUNT", 0x08D1, 	"[7-4][3-0]"},
	{"MIPI PHY1-0 COUNT", 0x08D2, 	"[7-4][3-0]"},
	{"MIPI PHY3-2 COUNT", 0x08D3, 	"[7-4][3-0]"},
};

static ERROR_REG_SET g_max9295_err_reg[] = 
{
    {"vid_tx2_pipe_x", 0x0102, "[4],[5],[6],[7]"},
    {"vid_tx2_pipe_y", 0x010A, "[4],[5],[6],[7]"},
    {"vid_tx2_pipe_z", 0x0112, "[4],[5],[6],[7]"},
    {"vid_tx2_pipe_u", 0x011A, "[4],[5],[6],[7]"},
    {"phy1_lp_err",    0x033B, "[0]~[4]"},
    {"phy2_lp_err",    0x033D, "[0]~[4]"},
    {"phy0_hs_err",     0x033A, "[0],[1],[4],[5]"},
    {"phy1_hs_err",     0x033C, "[0],[1],[4],[5]"},
    {"phy2_hs_err",     0x033E, "[0],[1],[4],[5]"},
    {"phy3_hs_err",     0x0340, "[0],[1],[4],[5]"},
    {"ctrl0_csi_err_l", 0x0341, "[0],[1],[7]"},
    {"ctrl0_csi_err_h", 0x0342, "[0]"},
    {"ctrl1_csi_err_l", 0x0343, "[0],[1],[7]"},
    {"ctrl1_csi_err_h", 0x0344, "[0]"},
};

int gmsl_iic_write(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 data)
{
	struct i2c_msg msg[1];
	u8 buf[3];
	int ret;

	msg[0].addr = slaveaddr;
	msg[0].flags = 0;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	buf[0] = regaddr >> 8;
	buf[1] = regaddr & 0xff;
	buf[2] = data;
	ret = i2c_transfer(client->adapter, msg, 1);
	usleep_range(100, 110);
	if (ret == 1) {
		return 0;
	} else {
		dev_err(&client->dev, "%s: ret:%d i2c write failed, slave=0x%02x addr(0x%04x)->0x%02x\n",
			__func__, ret, slaveaddr, regaddr, data);
		return -1;
	}
}

int gmsl_iic_read(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 *data)
{
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	msg[0].addr = slaveaddr;
	msg[0].flags = 0;
	msg[0].buf = buf;
	msg[0].len = 2;

	buf[0] = regaddr >> 8;
	buf[1] = regaddr & 0xff;

	msg[1].addr = slaveaddr;
	msg[1].buf = data;
	msg[1].len = 1;
	msg[1].flags = I2C_M_RD;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret == 2) {
		return 0;
	} else {
		dev_err(&client->dev, " %s : ret:%d i2c read failed, slave=0x%02x, addr=0x%04x\n", __func__, ret, slaveaddr, regaddr);
		return -1;
	}
}

int device_reg_update_bits(struct i2c_client* client, u8 slaveaddr, u16 regaddr, u8 mask, u8 val)
{
    u8 old_val, new_val;
    int ret = 0;
    ret = gmsl_iic_read(client, slaveaddr, regaddr, &old_val);
    if (ret < 0) return ret;
    new_val = (old_val & ~mask) | (val & mask);
    if (new_val == old_val)
        return 0;
    return gmsl_iic_write(client, slaveaddr, regaddr, new_val);
}

int gmsl_dtb_file_init(struct nv_sony_sensor *priv)
{
	int err = 0;
	int  i;
    struct device *dev = &priv->i2c_client->dev;
	struct device_node *node = dev->of_node;
	const char *str_value1[2],*str_value;
	struct device_node *gmsl;
	struct device_node *dser_node;
	struct i2c_client *dser_i2c = NULL;
	int value = 0xFFFF;

    err = of_property_read_u32(node, "reg", &priv->act_addr);
	if (err < 0) {
		dev_err(dev, " reg not found\n");
		goto ERROR;
	}
	err = of_property_read_u32(node, "def-addr",
					&priv->def_addr);
	if (err < 0) {
		vc_err(dev, " def-addr not found\n");
		goto ERROR;
	}
    priv->g_ctx.ser_dev = dev;
	dser_node = of_parse_phandle(node, "nvidia,gmsl-dser-device", 0);
	if (dser_node == NULL) {
		vc_err(dev, " missing %s handle\n",	"nvidia,gmsl-dser-device");
		err = -EINVAL;
		goto ERROR;
	}
	dser_i2c = of_find_i2c_device_by_node(dser_node);
	of_node_put(dser_node);
    if (dser_i2c == NULL) {
		vc_err(dev, " missing deserializer dev handle\n");
		err = -EINVAL;
		goto ERROR;
	}
	if (dser_i2c->dev.driver == NULL) {
		vc_err(dev, " missing deserializer driver\n");
		err = -EINVAL;
		goto ERROR;
	}
    priv->dser_dev = &dser_i2c->dev;
	priv->g_ctx.des_dev =  &dser_i2c->dev;
    err = of_property_read_string(node, "des-link", &str_value);
    if (err < 0) {
        vc_err(dev, " des-link property is not found\n");
        goto ERROR;
    }
    vc_info(dev, " link is %s\n",str_value);
    priv->des_link = str_value[0]-'A';
	priv->g_ctx.des_link = priv->des_link;
    /* populate g_ctx from DT */
	gmsl = of_get_child_by_name(node, "gmsl-link");
	if (gmsl == NULL) {
		vc_err(dev, " missing gmsl-link device node\n");
		err = -EINVAL;
		goto ERROR;
	}
	err = of_property_read_u32(gmsl, "num-ser-lanes", &value);
	if (err < 0) {
		vc_err(dev, " No num-lanes info\n");
		goto ERROR;
	}
	priv->g_ctx.num_ser_csi_lanes = value;
	priv->g_ctx.num_streams = of_property_count_strings(gmsl, "streams");
	if (priv->g_ctx.num_streams <= 0) {
		vc_err(dev, " No streams found\n");
		err = -EINVAL;
		goto ERROR;
	}
    for (i = 0; i < priv->g_ctx.num_streams; i++) {
		of_property_read_string_index(gmsl, "streams", i,
						&str_value1[i]);
		if (!str_value1[i]) {
			vc_err(dev, " invalid stream info\n");
			goto ERROR;
		}
        if (!strcmp(str_value1[i], "raw8")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_RAW_8;
		} else if (!strcmp(str_value1[i], "raw10")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_RAW_10;
		} else if (!strcmp(str_value1[i], "raw12")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_RAW_12;
		} else if (!strcmp(str_value1[i], "embed")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_EMBED;
		} else if (!strcmp(str_value1[i], "ued-u1")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_UED_U1;
		} else if (!strcmp(str_value1[i], "yuv16")) {
			priv->g_ctx.streams[i] =
							GMSL_CSI_DT_YUV16;
		}  else {
			vc_err(dev, " invalid stream data type\n");
            err = -EINVAL;
			goto ERROR;
		}
	}
    priv->g_ctx.sensor_dev = dev;
    return 0;
ERROR:
    vc_err(dev, "dtb_init failed\n");
	return err;
}


void gmsl_status_reg_print(struct i2c_client* client, u8 slaveaddr, MAX_TYPE type)
{
    struct reg_8 regValue;
    u8 i = 0;
    u8 reg_num = 0;
    ERROR_REG_SET *reg_set;
    switch (type)
    {
    case MAX96717:
        reg_num = sizeof(g_max96717_err_reg) / sizeof(g_max96717_err_reg[0]);
        reg_set = g_max96717_err_reg;
        break;
    case MAX96724:
        reg_num = sizeof(g_max96724_err_reg) / sizeof(g_max96724_err_reg[0]);
        reg_set = g_max96724_err_reg;
        break;
    case MAX9295:
        reg_num = sizeof(g_max9295_err_reg) / sizeof(g_max9295_err_reg[0]);
        reg_set = g_max9295_err_reg;
        break;
    default:
        return;
    }
    for (i = 0; i < reg_num; i++)
    {
        regValue.addr = reg_set[i].addr;
        gmsl_iic_read(client, slaveaddr, regValue.addr, &regValue.val);
        sensor_reg_info_print(&client->dev, 0, &regValue);
        msleep_range(5);
    }
}
