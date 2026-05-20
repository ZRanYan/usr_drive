#ifndef _USR_IO_H_
#define _USR_IO_H_
#define DLP_VER_MAX_LEN 128

typedef unsigned char  u8;
typedef unsigned int u32;
typedef unsigned short u16;

typedef enum 
{
    IO_SET_OUT_SYNC = 0,
    IO_SET_OUT_AC = 1,
    IO_SET_OUT_CC = 2,
    RGBW_LED_TRIG = 3,
}IO_OUT_INDEX;

typedef enum
{
    IO_RES_SET_OUT_SYNC = 0,
    IO_RES_SET_OUT_AC = 1,
    IO_RES_SET_OUT_CC = 2,
    IO_RES_SET_TRIG_SENSOR = 3,
}IO_OUT_TWO_SENSOR;   //用于双目触发sensor的配置

typedef enum{
    IO_INPUT_1 = 0, //2D
    IO_INPUT_2 = 1, //3D
    CAM_XTRIG_1 = 2,
    CAM_TOUT_0 = 3,
    CAM_TOUT_0_EXT = 4,
}IO_INPUT_FLAG;

typedef enum{
    LED_1 = 0,
    LED_2 = 1,
    LED_3 = 2,
}LED_FLAG;

typedef struct
{
    u8  outState;//输出状态
    u8  defaultState; //默认状态，既维持完上面的时间配置的状态
    u8  index; //输出io下标，IO_OUT_INDEX 枚举成员
    u8  res[1];
    u32   holdTime;//保持时间,单位微秒，配置0表示一直保持
}IO_PLUSE_SET;

typedef struct
{
    u8 index; //代表LED_FLAG成员
    u8 times; //闪烁次数
    unsigned short int period; //单位毫秒,配置0 表示一直保持
}IO_FLASH_SET;

typedef struct 
{
    u8 num; //亮灯数量,跟下面的参数保持一致,最多四个参数
    u8 pwmValue;//pwm的值
    u8 res[2]; //预留填充
    u8 states[4]; //配置管脚状态,低四位代表管脚坐标，高四位代表输出状态，每个灯的输出时间和下面保持一致
    u32 exposureTime[4]; //sensor的曝光时间，单位(微秒)/1-r,2-g,3-b,4-w
    u32 periodTime[4]; //周期时间，单位(微秒)
}IO_RGB_SET; //配置一次参数，代表进行num次循环

typedef struct 
{
    u8 num; //亮灯数量,跟下面的参数保持一致,最多四个参数
    u8 pwmValue;//pwm的值
    u8 res[2]; //预留填充
    u8 states[4]; //配置管脚状态,低四位代表管脚坐标，高四位代表输出状态，每个灯的输出时间和下面保持一致
    u32 expoSensor; //根据下面的值计算最高的sensor曝光时间(微秒)，驱动程序最终计算值，不需要配置传参
    u32 periodTime[4]; //周期时间，驱动程序最终计算值，不需要配置传参,单位(微秒)
    u32 exposureTime[4]; //led的曝光时间，单位(微秒)/1-r,2-g,3-b,4-w,需要应用程序传参
}IO_RGB_DRIVE_SET; //配置一次参数，代表进行num次循环

typedef struct{
    bool rgbw_on[4];
}DEV_IO_RGB_TRIG_SET; //触发的时候选择亮那个灯，比如我只需要触发里面的任意两个按需要进行配置

typedef struct
{
    u8 num; //代表input下标，范围：0，1
    u8 trigType; //0-代表上升沿，1-代表下降沿
    u16 debounce; //防抖时间，单位(毫秒)
}SET_IO_TRIGGER_TYPE;

typedef struct
{
    u8 sync_num; // 输出方波数量
    u8 res[3];
    u32 sync_expo;   // 输出高电平时间，单位微秒
    u32 sync_period; // 输出周期时间，单位微秒
} INTERNAL_CONFIG;

typedef struct
{
    u8 camNum;          // 代表要出图的数量
    u8 res[3];
    u32 ext_sync_expo;  // 仅输出一次外部脉冲信号的持续时间，单位微秒
    u32 ext_xtrig_expo; // 触发相机曝光时间，单位微秒PORT
} EXTERNAL_CONFIG;

typedef union
{
    INTERNAL_CONFIG interConfig; // 内部控制
    EXTERNAL_CONFIG exterConfig; // 外部控制
} MODE_PARAM;

typedef struct
{
    u8 mode; // 模式参数,0-完全控制模式，1-外部配合控制模式
    u8 res[3];
    MODE_PARAM modeParam;
} IO_2D_SUPPORT_LIGHT_PARAM;

typedef struct {
    __u32 len;                 /* 实际字符串长度 */
    char  ver[DLP_VER_MAX_LEN];
}IO_VERSION;

/**
 * @brief 配置1个dlp触发dlp+rgbw或者是不联动参数配置
 * 
 */
typedef struct
{
    u8 mode; //0-代表仅dlp闪烁,不联动rgb灯，后面的参数不生效，1-代表dlp+rgb联动方式，
    u8 syncDlpNum; //dlp触发的xtrig的总次数
    u8 syncRgbNum; //需要在光机触发的xtrig后的第几次执行rgb闪烁
    u8 pwmValue;//pwm的值
    u32 exposureTime[4]; //led的曝光时间，单位(微秒)/1-r,2-g,3-b,4-w,需要应用程序传参
}IO_RGB_SYNC_SIGNAL;

#define IO_GET_VERSION                  _IOW('k', 0, IO_VERSION) // 获取驱动版本
#define IO_TEST                         _IOW('k', 1, IO_PLUSE_SET)      // 测试使用，暂时不用
#define IO_GET_TRIGGER_PIN              _IOW('k', 2, int)    // 获取中断管脚，返回值在IO_INPUT_FLAG 枚举成员
#define IO_SET_OUT                      _IOW('k', 3, IO_PLUSE_SET)   // 配置IO输出电平，配置高，配置低
#define IO_SET_DLP_STATUS               _IOW('k', 4, u8)                           // 配置dlp总电源开关,只要低四位，1,2,4,8代表依次使能不同的dlp光机，0x0f代表全部使能
#define IO_GET_PIN_STATUS               _IOW('k', 5, u8)                           // 获取电路板id的号，暂时不用
#define IO_SET_FLASH_LED                _IOW('k', 6, IO_FLASH_SET)                  //
/**
 * @brief 配置了IO_SET_RGB_PARAM，会将IO_RGB_SYNC_SIGNAL里面的参数mode强制写成0
 * 
 */
#define IO_SET_RGB_PARAM                _IOW('k', 7, IO_RGB_SET)                    // 配置rgb灯闪烁周期和曝光时间
#define IO_SET_IO_TRIGGER_PARAM         _IOW('k', 8, SET_IO_TRIGGER_TYPE)    //
#define IO_2D_SET_MODE_PARAM            _IOW('k', 9, IO_2D_SUPPORT_LIGHT_PARAM) //
#define IO_SET_USB_ENABLE_DLP_STATUS    _IOW('k', 10, u8)               // 高电平使能usb和dlp连接，
#define IO_SET_RGB_TRIG_CAM             _IOW('k', 11, u32)                       //暂时没有用到
/**
 * @brief 清零rgb和dlp的触发计数值，不用传参,
 * 返回值1:代表没有人任何触发，0代表正常触发到配置的dlp数量，3代表xtrig和tou0的数量没有对上
 * 
 */
#define IO_SET_CLEAR_TRIG_COUNT         _IOW('k', 12, u8) // 
/**
 * @brief 配置dlp和rgb的联动参数
 * 
 */
#define IO_SET_GET_RGB_SYNC_SIGNAL      _IOW('k', 13, IO_RGB_SYNC_SIGNAL)   //配置dlp和rgb联动参数
/**
 * @brief 生效 IO_SET_RGB_PARAM 里面的rgb灯的触发参数，触发的时候会判断IO_RGB_SYNC_SIGNAL的mode参数是否为0，否则配置参数不响应
 * 
 */
#define IO_SET_SINGLE_RGB_TRIG          _IOW('k', 14, DEV_IO_RGB_TRIG_SET)   //生效 IO_SET_RGB_PARAM 里面的rgb灯的触发参数，



#endif
