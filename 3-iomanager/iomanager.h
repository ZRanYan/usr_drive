#ifndef BLIO_M_H
#define BLIO_M_H
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include "usr_io.h"
#include "no_opt.h"
#define BLIOMANAGER_VER          "V0.3.1"
#define BLIOMANAGER_FULL_VERSION \
    BLIOMANAGER_VER " (" GIT_COMMIT " on " GIT_BRANCH " " PROGRAM_DATE ")"

/****************************************************************
//2024-01-04
//1.creat this driver
****************************************************************/ 

#define MIN_RISE_TIME 19000
#define MIN_FALL_TIME 500 //单位微秒

#define CPU_RT_TIME_ID 7

/*****************************************************************************/
//user reference start
/*****************************************************************************/
typedef enum BLIO_IN{
    BLIO_IO_IN1          = 0,
}BLIO_IN_t;

typedef enum BLIO_LED{
    BLIO_IO_LED1         = 0,
    BLIO_IO_LED2,
    BLIO_IO_LED3,
}BLIO_LED_t;

/*
 * @brief: bl-iomanager return sattus
 */
typedef enum DEV_RTN{
    RTN_OKAY                =   0,
    RTN_NOT_ALLOWED         =   1,
    RTN_INVALID_ARG         =   2,
    RTN_BUSY_NOW            =   3,
    RTN_NOT_SUPPORT         =   4,
    RTN_UNKNOWN_ERR         =   5,
}DEV_RTN_t;

typedef enum FLAG_TYPE{
    FLAG_TYPE_EVENT_RESET   =   0,
    FLAG_TYPE_FRAME_VALID   =   1,
    FLAG_TYPE_BUTTON_TUNE   =   2,
    FLAG_TYPE_DEV_CHANGED   =   3,
    FLAG_TYPE_TRIG_IN1      =   4,
    FLAG_TYPE_TRIG_IN2      =   5,
    FLAG_TYPE_TRIG_BTN      =   6,
    FLAG_TYPE_SETTING_DEF   =   7,
    FLAG_TYPE_REBOOT        =   8,
    FLAG_TYPE_ETH_DISCON    =   9,
    FLAG_TYPE_ETH_CON       =   10,
}FLAG_TYPE_t;

/*
 * @brief: bl-iomanager trig in mode
 */
typedef enum TRIGIN_MODE{
    TRIG_IN_MODE_NULL,
    TRIG_IN_MODE_FALL,
    TRIG_IN_MODE_RISE
}TRIGIN_MODE_t;

/*
 * @brief: bl-iomanager trig in event(yes or not return to workspace)
 */
typedef enum{
    TRIG_IN_EVENT_RTN_EVENT           =   0,
    TRIG_IN_EVENT_TRIG_SENSOR         =   1,
}TRIGIN_EVENT_t;

/*
 * @brief: bl-iomanager trig out mode(source)
 */
typedef enum TRIGOUT_MODE{
    SOURCE_NONE             =   0,
    SOURCE_CAPTURE          =   1,
    SOURCE_TRIG_IN1         =   2,
    SOURCE_TRIG_IN2         =   3,
    SOURCE_TRIG_BTN         =   4,
    SOURCE_OK               =   5,
    SOURCE_NG               =   6,
    SOURCE_TRIG_BUSY        =   7,
    SOURCE_MODE_BUSY        =   8,
    SOURCE_ETHERNET_ERROR   =   9,
    SOURCE_USER_DEF         =   10,
    SOURCE_MAX,//do not use this value              
}TRIGOUT_MODE_t;

/*
 * @brief: bl-iomanager ethernet connect-status
 */
typedef enum{
    ETH_STATE_NG,
    ETH_STATE_OK,
}ETH_STATE_t;

//desc:     trig in event config
//mode:     one of enum TRIG_IN_MODE,default value is TRIG_IN_MODE_NULL
//delay_us: trig sensor delay after trig event,invalid rangs is IN_PULSE_US_MIN~IN_PULSE_US_MAX
typedef struct TRIGIN_CFG{
    int                gpio_num;
    TRIGIN_MODE_t      mode;
    TRIGIN_EVENT_t     event;
    uint               delay_us;
}TRIGIN_CFG_t;

//desc:     trig out config
//invert:   0 means no-invert,1 means invert
//pulse_us: pulse duration in us
//source:   see SOURCE_t
typedef struct TRIGOUT_CFG{
    int         gpio_num;
    uint        invert;
    uint        pulse_us;
    TRIGOUT_MODE_t    source;
}TRIGOUT_CFG_t;

typedef struct 
{
    u8 thread_flag;
    atomic_t countNum;              //原子计数值
    struct task_struct *thread;
    wait_queue_head_t   waitQueue; //等待消息队列
    struct hrtimer      hrtTimer;//一个软件定时器
    ktime_t             kt;      //定时器循环的时间参数
    IO_PLUSE_SET        param;   //给线程传递的参数
    IO_FLASH_SET        flash_param; //led闪烁配置的参数
    IO_RGB_DRIVE_SET    rgb_param; //rgb灯参数
    IO_2D_SUPPORT_LIGHT_PARAM light_param; //2D闪烁服务
}WORK_STRUCT;

typedef struct 
{
   u8 thread_flag;
   u8 state;
   u8 index;
   struct task_struct *thread;
   wait_queue_head_t   waitQueue;
}RT_TASK_STRUCT;


typedef enum hrtimer_restart (*usr_pwm_callback_t)(struct hrtimer *);

typedef struct 
{
    u32 pulseTime;
    u32 periodTime;
    u32 count;
    usr_pwm_callback_t callBack;
}USR_PWM_TIMER;

typedef struct 
{
    u32 pulseTime[4];
    u32 periodTime[4];
    u32 count;
    usr_pwm_callback_t callBack;
}USR_RGB_PWM_TIMER;

typedef struct 
{
    u8 rgb_mode;                       //0-代表仅仅DLP亮灯，1-代表DLP+RGB联动闪烁出图
    u8 num_rgb;                       //dlp的控制数量
    struct gpio_descs *rgb_gpios;     // 输出控制rgb灯
    ktime_t rgb_time[4];                //dlp和rgb联动触发的曝光时间
    struct hrtimer rgbTimer;
    //计算管脚中断的计数值
    atomic_t xtrig_num; //统计xtrig的数量
    atomic_t tou_num; //统计tou0的数量
    atomic_t tou_num_ext; //统计tou0的数量
    //开始rgb等闪烁的开始
    atomic_t rgbTrigIndex;//开始触发rgb的开始，等xtrig管脚到达这个值的时候开始rgb灯闪烁
    atomic_t totalTrigIndex;//当达到tou0管脚达到这个数的时候，唤醒队列检查这个xtrig_num和tou_num，并更新状态值
    int last_irq_rgb; //当前想点亮的rgb灯的下坐标,根据中断任务里面的判断
    u8 rgb_flag;   //线程唤醒标志
    u8 rgb_enable; //rgbw灯的使能开关,避免开机管脚异常电平中断导致rgb灯亮
    wait_queue_head_t wq_rgb;
    struct task_struct *thread;
}DLP_RGB_SET_PARAM;

typedef struct
{
    int irqNum;
    bool disabled;
}IRQ_CTRL;


struct gpio_inout_dev
{
    struct pwm_device *rgbPwm;        //rgb灯占空比配置
    struct gpio_descs *dlp_gpios;    //DLP电路总开关
    struct gpio_descs *input_gpios;  // 输入 GPIO 数组
    struct gpio_descs *usb_gpios;     //板载id编号
    struct gpio_descs *output_gpios; // 输出 GPIO 数组
    struct gpio_descs *flash_gpios;  // 输出 GPIO 数组

    IRQ_CTRL *irqIo;                  // 中断号数组
    int *dlp_irq;                      //DLP中断数组
    u8 num_dlp;                       //dlp的控制数量
    u8 num_usb;                        //板载id编号
    u8 num_inputs;                  // 输入 GPIO 数量
    u8 num_outputs;                 // 输出 GPIO 数量
    u8 num_flash;                  //led闪烁的gpio数量
    WORK_STRUCT *wq_outpin;                // 根输出GPIO数据保持一致
    WORK_STRUCT *wq_flash;          // led闪烁的gpio数据保持一致
    WORK_STRUCT *wq_2D;             //响应2D曝光控制模式
//-----------------------------------------------------
    WORK_STRUCT *wq_rgb;            //rgb灯闪烁控制线程
//--------------------------------------------------------------------------
    DLP_RGB_SET_PARAM dlpRgbSet; //dlp和rgb参数联动
//----------------------------------end--------------------------------------------
    int major;
    struct class *gpio_manager_class;
    struct device *gpio_manager_dev;
    struct device *dev;
};

#define BEEP_MS_MIN             (1)
#define BEEP_MS_MAX             (10)
#define BEEP_TIMES_MIN          (1)
#define BEEP_TIMES_MAX          (10)

#define TRIG_OUT_PULSE_US_JIT   10

#define IN_PULSE_US_MIN	        (1000U)
#define IN_PULSE_US_MAX	        (10000000U)

#define OUT_PULSE_US_MIN	    (1000U)
#define OUT_PULSE_US_MAX	    (10000000U)

/*****************************************************************************/
//user reference end
/*****************************************************************************/
#endif