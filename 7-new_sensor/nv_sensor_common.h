#ifndef NV_SENSOR_COMMON_H
#define NV_SENSOR_COMMON_H
#include <soc/tegra/pmc.h>
#include <soc/tegra/fuse.h>
#include <media/tegra_v4l2_camera.h>
#include <media/tegracam_core.h>
#include <media/tegracam_utils.h>
#include <linux/pwm.h>
#include <linux/gpio/consumer.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include "z_gmsl-link.h"
#include "usr_debug.h"

#define BL_NV_IMX_SENSOR_VER "V0.4.1"

#define BL_NV_SENSOR_FULL_VERSION \
    BL_NV_IMX_SENSOR_VER " (" GIT_COMMIT " on " GIT_BRANCH " " PROGRAM_DATE ")"


#define IMX185_FUSE_ID_ADDR	0x3382
#define IMX185_FUSE_ID_SIZE	6
#define IMX185_FUSE_ID_STR_SIZE	(IMX185_FUSE_ID_SIZE * 2)
#define SENSOR_TABLE_END	    1	
#define SENSOR_TABLE_WAIT_MS	0	
#define SENSOR_8_BIT 	0
#define SENSOR_10_BIT 	1
#define SENSOR_12_BIT	2
#define SENSOR_TOUT0_ENABLE		1 //tou0输出配置
#define SENSOR_DEFAULT_BLACK_VALUE  15//8bit-15, 10-60, 12bit-240
#define SENSOR_VER_MAX_LEN 128

#define SENSOR_IMX_STANDBY 0x3000
#define SENSOR_IMX_XMSTA 0x3010
#define SENSOR_MODEL_ID_ADDR_MSB 0x3817
#define SENSOR_MODEL_ID_ADDR_LSB 0x3816
#define SENSOR_BINNING_2X_DISABLE 0 // 0-关闭,正常的ROI模式 1-开启2x2 binning，2-开启2x2 subsampling
#define SENSOR_SUBSAMPLING_2X 1		// 0-关闭,正常的ROI模式 1-开启2x2 binning，2-开启2x2 subsampling
#define SENSOR_BINNING_2X 2			// 0-关闭,正常的ROI模式 1-开启2x2 binning，2-开启2x2 subsampling
#define SENSOR_IMX_TRIG 0x3400
#define SENSOR_IMX_GAIN_ADDR_L 0x3514
#define SENSOR_IMX_GAIN_ADDR_H 0x3515
#define SENSOR_IMX_REVERSE_ADDR 0x3204

#define SENSOR_IMX_TEMPERATURE_VAL 	0x3594
#define SENSOR_IMX_TEMPERATURE_EN	0x3596

#define SENSOR_IMX_BLKLEVEL_L 0x35B4
#define SENSOR_IMX_BLKLEVEL_H 0x35B5

#define SENSOR_IMX_VMAX_0 0x30D4
#define SENSOR_IMX_VMAX_1 0x30D5
#define SENSOR_IMX_VMAX_2 0x30D6

#define SENSOR_IMX_HMAX_0 0x30D8
#define SENSOR_IMX_HMAX_1 0x30D9

#define SENSOR_IMX_GMRWT 0x30E2
#define SENSOR_IMX_GMTWT 0x30E3
#define SENSOR_IMX_GAINDLY 0x30E5
#define SENSOR_IMX_GSDLY 0x30E6

#define SENSOR_IMX_VINT 0x323E
#define SENSOR_IMX_SHS_0 0x3240
#define SENSOR_IMX_SHS_1 0x3241
#define SENSOR_IMX_SHS_2 0x3242
#define SENSOR_IMX_REGHOLD					0x3034
#define SENSOR_IMX_HVMODE					0x303C //0h: All-pixel,1h: 1/2 Subsampling mode,2h: FD binning
#define SENSOR_IMX_VOPB_VBLK_HWIDTH_0		0x30D0
#define SENSOR_IMX_VOPB_VBLK_HWIDTH_1		0x30D1
#define SENSOR_IMX_FINFO_HWIDTH_0			0x30D2
#define SENSOR_IMX_FINFO_HWIDTH_1			0x30D3
#define SENSOR_IMX_ROI_MODE					0x3100
#define SENSOR_IMX_FID0_ROI					0x3104
#define SENSOR_IMX_FID0_ROIPH1_0				0x3120
#define SENSOR_IMX_FID0_ROIPH1_1				0x3121
#define SENSOR_IMX_FID0_ROIPV1_0				0x3122
#define SENSOR_IMX_FID0_ROIPV1_1				0x3123
#define SENSOR_IMX_FID0_ROIWH1_0				0x3124
#define SENSOR_IMX_FID0_ROIWH1_1				0x3125
#define SENSOR_IMX_FID0_ROIWV1_0				0x3126
#define SENSOR_IMX_FID0_ROIWV1_1				0x3127
#define SENSOR_IMX_ADBIT						0x3200
#define SENSOR_IMX_TOUT0SEL					0x3436

#define SENSOR_SC_SLEEP_MODE				0x2100
#define SENSOR_SC_RB_ROWS_0					0x3231
#define SENSOR_SC_RB_ROWS_1					0x3230

#define SENSOR_SC_BLANK_ROWS_0				0x320f
#define SENSOR_SC_BLANK_ROWS_1				0x320e

#define CALCULATE_ROUND(dim, step, flags)	\
	((flags) & V4L2_SEL_FLAG_GE			\
	 ? roundup((dim), (step))			\
	 : ((flags) & V4L2_SEL_FLAG_LE			\
	    ? rounddown((dim), (step))			\
	    : rounddown((dim) + (step) / 2, (step))))

//公共的
#define V4L2_CID_GET_VERSION 				(V4L2_CID_USER_BASE + 0)
#define V4L2_CID_REGIST_IOCTL  				(V4L2_CID_USER_BASE + 1)
#define V4L2_CID_GET_TEMPERATURE_VAL 		(V4L2_CID_USER_BASE + 2)
#define V4L2_CID_SET_PATTERN_GENERATOR  	(V4L2_CID_USER_BASE + 3)
#define CAM_SET_PWM_XTRIG       			(V4L2_CID_USER_BASE + 4)
#define CAM_GET_MIN_EXPOSE      			(V4L2_CID_USER_BASE + 5)
#define CAM_SET_ROI_FORMAT      			(V4L2_CID_USER_BASE + 6)

#define CAM_SET_GAIN						(V4L2_CID_USER_BASE + 7)
#define CAM_SET_BLACK_LEVEL					(V4L2_CID_USER_BASE + 8)
#define CAM_SET_HFLIP						(V4L2_CID_USER_BASE + 9)
#define CAM_SET_VFLIP						(V4L2_CID_USER_BASE + 10)
//私有的sc535hgs的配置参数
#define CAM_SET_EXPO_PERIOD_PARAM			(V4L2_CID_USER_BASE + 11)
#define CAM_GET_SENSOR_TYPE					(V4L2_CID_USER_BASE + 12)
#define CAM_SET_CUSTOM_TEST                 (V4L2_CID_USER_BASE + 13)

#define CAM_SET_SENSOR_POWER_STATUS			(V4L2_CID_USER_BASE + 14)

struct nv_sony_sensor;

typedef struct reg_8 SENSOR_REG_STRUCT;
typedef enum
{
	SEQUENTIAL_TRIGGER_MODE = 0,
	NORMAL_MODE = 1,
	FAST_TRIGGER_MODE = 2,
} MODE_TYPE;


typedef int (*nv_sensor_dtb_init_fun)(struct nv_sony_sensor *);
typedef int (*nv_sensor_board_setup_fun)(struct nv_sony_sensor *, MODE_TYPE);
typedef void (*nv_sensor_param_set_fun)(struct nv_sony_sensor *,int);
typedef void (*nv_sensor_usr_read_id_fun)(struct nv_sony_sensor *);
typedef int (*nv_sensor_ioctl_fun)(struct nv_sony_sensor*, unsigned int, void *);
typedef int (*nv_sensor_stream_set_fun)(struct nv_sony_sensor *, int);
typedef int (*nv_sesnor_set_fmt_fun)(struct nv_sony_sensor *, struct v4l2_subdev *, struct v4l2_subdev_format *);
typedef int (*nv_sesnor_get_fmt_fun)(struct nv_sony_sensor *, struct v4l2_subdev *, struct v4l2_subdev_format *);
typedef int (*nv_sensor_set_selection)(struct nv_sony_sensor *, struct v4l2_subdev_state *, struct v4l2_subdev_selection *);
typedef int (*nv_sensor_get_selection)(struct nv_sony_sensor *, struct v4l2_subdev_state *, struct v4l2_subdev_selection *);
typedef int (*nv_sensor_set_group_hold)(struct tegracam_device *tc_dev, bool val);
typedef int (*nv_sensor_board_power_off)(struct nv_sony_sensor *);

typedef enum
{
	VERTICAL = 0,
	HORIZONTAL = 1
}FILP_TYPE;

typedef enum
{
	IMX566 = 0,
	IMX565 = 1,
	SC535HGS = 2,
	GMAX3405 = 3,
	OG02C1B = 4,
	OV5640 = 5,
	OG05B2B = 6,
	GMAX3412 = 7,
} SENSOR_TYPE_ENUM;

typedef struct{
    __u8 opt; //0-代表读，1-代表写
    __u8 value;
    __u16 addr;
} SENSOR_DEBUG_REG_PARAMS;

typedef struct{
    __u8 enable;  //PWM的开关
    __u8 frame_s; //配置每秒多少帧
    __u8 res[2];  //保持对齐
    __u32 exposure; //曝光时间,us单位
} SENSOR_XTRIG_PWM_PARAM;
typedef struct
{
	__u8 isEnable; //0-disable,1-enable
	__u8 bitMode; //0-8bit,1-10bit,2-12bit
	__u8 binningMode; //0-no binning,1-hbinning,2-vbinning,3-hvbinning
	__u8 res;	//保持对齐
	__u16 start_x;
	__u16 start_y;
	__u16 width;
	__u16 height;
}SENSOR_FORMAT_ROI_PARAM;
typedef struct{
    __u16 exposureUnit; //曝光的1H时间单位倍数，既最小的曝光时间要求(xtrig持续电平的时间)
    __u16 min_trigger_fall; //相对上一次xtrig的结束点(既上升沿)的下一次xtrig的下降沿触发的禁止触发时间，单位(微秒)
    __u16 min_trigger_rise; //相对上一次xtrig的结束点(既上升沿)的下一次xtrig的上升降沿触发的禁止触发时间，单位(微秒)
	__u16 max_frame; //最高帧率,(frame/s)
}SENSOR_ATTRIBUTE;

typedef struct sensor_ver_info {
    __u32 len;                 /* 实际字符串长度 */
    char  ver[SENSOR_VER_MAX_LEN];
}SENSOR_VERSION;

union sensor_ioctl_data {
	SENSOR_DEBUG_REG_PARAMS    reg;
	SENSOR_XTRIG_PWM_PARAM     pwm;
	SENSOR_ATTRIBUTE           min_interval;
	SENSOR_FORMAT_ROI_PARAM    roi;
	SENSOR_VERSION			   ver;
};

typedef struct 
{
	__u8 bit;
	__u8 binningOrsubSampling;
	__u8 gmrwt;
	__u8 gmtwt;
	__u8 gsdly;
	__u16 hmax;
	__u32 vmax;
}SETTING_PARAM;

typedef struct 
{
    char str[20];
    u16  addr;
    char bitfield[20]; // 补充的 BITFIELD 字符串字段
} ERROR_REG_SET;

typedef struct 
{
	u16 addr;
	u8 isReuseBinning;//0-不复用，1-复用binning模式的寄存器值,2-复用vmax，gmrwt，gsdly，hmax寄存器值,3-仅用作bit切换的寄存器
	u8 val_1[3];//bit0:8bit; bit1:10bit; bit2:12bit,如果isReuseBinning=1,则表示复用binning模式的寄存器值,Normal模式的寄存器值,bingning模式的寄存器值
	u8 index;
	u8 val_2[3]; //针对isReuseBinning=2的情况，表示复用vmax，gmrwt，gsdly，hmax寄存器值,Normal模式的寄存器值,bingning模式的寄存器值
} IMX_SENSOR_BIT_REG;

typedef struct
{
	u8 type; //1-是配置参数，0-是获取系统的参数
	u32 expo; //配置曝光时间，单位微秒，必须是6的倍数
	u32 period; //配置出图的周期时间，单位微秒，必须是6的倍数
	u32 minPeriod; //根据曝光时间和出图的行数计算最小的帧周期时间参数，单位微秒
}SENSOR_EXPO_PERIOD_PARAM;

struct nv_sensor_model_info {
	__u8 usr_id;
	__u16 i2c_address;
	char name[32];
	__u32 input_freq;
	__u16 pixel_width;
	__u16 pixel_height;
	__u8 pixel_bit;
	__u8 sensorMode; // 0-normal,1-fastTrigger, 2-sequentialTrigger
	const struct regmap_config *map_config;
	struct camera_common_sensor_ops *cam_com_ops;
	nv_sensor_dtb_init_fun dtb_init;
	nv_sensor_board_setup_fun board_init;
	nv_sensor_param_set_fun sensor_init_param;
	nv_sensor_usr_read_id_fun sensor_usr_set;
	nv_sensor_ioctl_fun	sensor_ioctl_set;
	nv_sensor_stream_set_fun sensor_stream_set;
	nv_sesnor_set_fmt_fun sensor_fmt_set;
	nv_sesnor_get_fmt_fun sensor_fmt_get;
	nv_sensor_set_selection sensor_set_selection;
	nv_sensor_get_selection sensor_get_selection;
	nv_sensor_set_group_hold sensor_set_group_hold; //必须实现
	nv_sensor_board_power_off board_off_power; //sensor板子关电
};

struct nv_sony_sensor
{
	struct i2c_client *i2c_client;
	struct v4l2_subdev *subdev;
	u8 fuse_id[IMX185_FUSE_ID_SIZE];
	u32 frame_length;
	s64 last_wdr_et_val;
	struct camera_common_data *s_data;
	struct tegracam_device *tc_dev;
	//下面自定的地方
	const struct nv_sensor_model_info *model_info; //绑定每个sensor型号的配置信息
	struct v4l2_ctrl_handler ctrl_handler;
	struct media_pad pad;
	struct v4l2_rect	m_rect;
	bool is_streaming;

	//sony的主要配置接口
	struct gpio_desc *pwdn_gpio;	// 外部硬件上电管脚
	struct gpio_desc *xclr_gpio;	// xclr
	struct gpio_desc *pwgd_gpio;	// power good,获取电源的输出状态
	struct gpio_desc *inck_gpio;	// inck enable
	struct gpio_desc *xmaster_gpio; // xmaster
	struct pwm_device *pwm_xhs;
	//思特微主要的配置接口
	struct gpio_desc *reset_gpio;	// 用于sensor上电，也复用gamx3405的SYS_RST_N
	struct gpio_desc *pwdnb_gpio; 	//用于sensor的上电
	struct gpio_desc *fsync_gpio;	//用于sensor的触发出图
	//长光辰芯主要的配置
	struct gpio_desc *clk_gpio;	//用于外部40MHz的晶振使能
	//gmsl相关的配置
	struct gmsl_link_ctx    g_ctx;
	struct device		*dser_dev;
	u32 def_addr;
	u32 act_addr;
	u32 des_link;
	u8 eeprom[128];
	u32 awb[3];

	struct pwm_state pwm_state;
	__u32 input_freq; //输入时钟参数
	__u32 x; // roi参数
	__u32 y;
	__u32 height; // 图像的高度
	__u32 width;
	__u8 sensorMode; // 0-normal,1-fastTrigger, 2-sequentialTrigger
	__u32 vmax;		 // 有效的像素行数,复用gmax3405的w_sum参数
	__u32 hmax;		 // 每行数据的时钟数，跟bit，和line的速率有关系
	__u8 gmrwt;		 // 全局内存读取等待时间
	__u8 gmtwt;		 // 全局内存等待时间
	__u8 gsdly;		 // 全局快门延迟时间
	__u32 period;	 // 行周期时间
	__u32 expoValue; //曝光时间，单位微秒
	__u8 bit;		 // 0-8bit,1-10bit,2-12bit
	__u16 fps;		 // 帧率，(frame/s)
	__u8 binning; //暂存的binning or subsampling的状态
	bool vflip_status; //暂存的上下翻转的状态
	MODE_TYPE modeType;	//sensor的模式，决定sensor的外部io触发还是自由出流

	struct dentry *debugfs_dir; //调试节点
	struct proc_dir_entry *proc_dir;//proc的调试节点
	int numctrls;
	struct v4l2_ctrl *expo;		   // 曝光时间
	struct v4l2_ctrl *pulse_set;   // 输出io口配置，TOUT0 pin，TOUT1 pin，TOUT2 pin
	struct v4l2_ctrl *roi;		   // roi参数
	struct v4l2_ctrl *gain;		   // 增益
	struct v4l2_ctrl *black;	   // 黑电平
	struct v4l2_ctrl *hflip;	   // 水平翻转
	struct v4l2_ctrl *vflip;	   // 垂直翻转
	struct v4l2_ctrl *tempSensor;  // sensor温度
	struct v4l2_ctrl *sensor_mode; // 曝光模式，normal或fast模式
	struct v4l2_ctrl *test;	   // sensor的test模式
	struct v4l2_ctrl *sensor_binning_2x_set; // sensor的binning or subsampling设置
	struct v4l2_ctrl *ctrls[];
};

#endif
