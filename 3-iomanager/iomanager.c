#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/unistd.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/pwm.h>
#include "version.h"
#include "usr_debug.h"
#include "iomanager.h"
#include "usr_io.h"

#define DRIVER_DESC				"iomanager"
#define DEAULT_OUTPUT_VALUE     0
#define TRIG_OUT_NUMBER			1
#define DEVICE_NAME				"iomanager"
#define CLASS_NAME				DEVICE_NAME

#define IRQ_NAME                "usr_gpio_in_irq"
#define IRQ_DLP_NAME            "dlp_trig_out_irq"

static struct gpio_inout_dev *gdev;
static struct fasync_struct  *share_async;

static struct dentry *iomanager_debugfs;

static IO_INPUT_FLAG triggerPin = 0; //触发管脚

u64 g_triggerPin_input_count[2] = {0}; //自统计外部触发io的次数

static volatile int g_pwm_count = 0;

static ktime_t g_int_ktime_on, g_int_ktime_off;
static unsigned int g_inter_pwm_count = 0;
static unsigned int g_inter_pwm_target_count = 1;

static u16 g_led_flash_count[3] = {0}; //3个led灯的定时器计数值

//单独rgbw闪烁的配置参数
static u8 g_pwm_target_count = 1; // 任意个方波
static volatile ktime_t g_interval;
static ktime_t g_stage_1[4]; //开启led灯，和开启sensor曝光
static ktime_t g_stage_2[4]; //关闭led灯
static ktime_t g_stage_3[4]; //关闭sensor曝光
static u8 g_stage_index[4]; //根据配置任意一个灯闪烁

static volatile uint8_t g_state = 0;

static int led_blink_ms = 100;
module_param(led_blink_ms, int, 0644);
MODULE_PARM_DESC(led_blink_ms, "led or beep duration in ms:100~500");

static int trig_pulse_us= 150;		
module_param(trig_pulse_us, int, 0644);
MODULE_PARM_DESC(trig_pulse_us, "trig_pulse_us:");

static int flash_allowed=1;		
module_param(flash_allowed, int, 0644);
MODULE_PARM_DESC(flash_allowed, "flash_allowed:0=disabled,1=enabled");

enum hrtimer_restart rgb_single_set_callback(struct hrtimer *timer)
{
    uint8_t mIndex = g_stage_index[g_pwm_count/3];
    if(0 == g_state%3)
    {
        g_interval = g_stage_1[mIndex];
        gpiod_set_value(gdev->output_gpios->desc[IO_RES_SET_TRIG_SENSOR], 1);
        gpiod_set_value(gdev->dlpRgbSet.rgb_gpios->desc[mIndex], 1);
    }
    else if(1 == g_state%3)
    {
        g_interval = g_stage_2[mIndex];
        gpiod_set_value(gdev->dlpRgbSet.rgb_gpios->desc[mIndex], 0);
        // printk("273 line g_state:%d g_pwm_count:%d mIndex:%d\r\n", g_state ,g_pwm_count, mIndex);
        if (0 == ktime_to_ns(g_interval))
        {
            g_interval = g_stage_3[mIndex];
            gpiod_set_value(gdev->output_gpios->desc[IO_RES_SET_TRIG_SENSOR], 0);
            g_state++;
            g_pwm_count++;
            // printk("280 line g_state:%d g_pwm_count:%d mIndex:%d\r\n", g_state ,g_pwm_count, mIndex);
        }
    }
    else if(2 == g_state%3)
    {
        g_interval = g_stage_3[mIndex];
        gpiod_set_value(gdev->output_gpios->desc[IO_RES_SET_TRIG_SENSOR], 0);
        // printk("287 line g_state:%d g_pwm_count:%d mIndex:%d\r\n", g_state ,g_pwm_count, mIndex);
    }
    g_state++;
    g_pwm_count++;
    if (g_pwm_count >= g_pwm_target_count)
    {
        g_pwm_count = 0;
        g_state = 0;
        return HRTIMER_NORESTART;
    }
    hrtimer_forward_now(timer, g_interval);
    return HRTIMER_RESTART;
}
static void start_timer_on_cpu(void *data)
{
    u8 *count = data;
    static struct hrtimer pwm_timer;
    if(0 != g_pwm_count)
    {
        return;
    }
    memset(&pwm_timer, 0, sizeof(pwm_timer));
    g_pwm_target_count = (*count) * 3;
    g_pwm_count = 0;
    g_state = 0;
    hrtimer_init(&pwm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED_HARD);
    pwm_timer.function = rgb_single_set_callback;
    hrtimer_start(&pwm_timer, ktime_set(0, 0), HRTIMER_MODE_REL_PINNED_HARD); // 从高电平开始
}
static int rgb_flash_fn(void *data)
{
    WORK_STRUCT *ws = (WORK_STRUCT *)data;
    while (!kthread_should_stop())
    {
        wait_event_interruptible(ws->waitQueue, ws->thread_flag || kthread_should_stop());
        if (kthread_should_stop())
            break;
        start_timer_on_cpu(&ws->rgb_param.num);
        ws->thread_flag = 0; // 清除标志位
    }
    return 0;
}


static const struct of_device_id of_gpio_manager_match[] = {
	{ .compatible = "gpio-iomanager", },
	{}
};
MODULE_DEVICE_TABLE(of, of_gpio_manager_match);

static void m_disable_irq(IRQ_CTRL *c)
{
    if (!c->disabled)
    {
        disable_irq(c->irqNum);
        c->disabled = true;
    }
}
static void m_enable_irq(IRQ_CTRL *c)
{
    if (c->disabled)
    {
        enable_irq(c->irqNum);
        c->disabled = false;
    }
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    struct gpio_inout_dev *d = dev_id;
    int val = 0;
    if(irq == d->irqIo[2].irqNum) //sensor的xtrig中断管脚
    {
        val = atomic_inc_return(&d->dlpRgbSet.xtrig_num);
        if(val>0xff)
        {
            return IRQ_NONE;
        }
        if (unlikely(val > atomic_read(&d->dlpRgbSet.rgbTrigIndex))) {
            d->dlpRgbSet.rgb_flag = 1;
            wake_up_interruptible(&d->dlpRgbSet.wq_rgb);
        }
    }
    else if(irq == d->irqIo[3].irqNum) //sensor的tout0中断管脚
    {
        //triggerPin = CAM_TOUT_0;
        atomic_inc_return(&d->dlpRgbSet.tou_num);
        //kill_fasync(&share_async, SIGIO, POLLIN);
    }
    else if(irq == d->irqIo[4].irqNum) //sensor的tout0中断管脚
    {
        //triggerPin = CAM_TOUT_0_EXT;
        atomic_inc_return(&d->dlpRgbSet.tou_num_ext);
        //kill_fasync(&share_async, SIGIO, POLLIN);
    }
    else if(irq == d->irqIo[0].irqNum)
    {
        triggerPin = IO_INPUT_1;//外部2D输入
        g_triggerPin_input_count[0]++;
        if(1 == d->wq_2D->light_param.mode)
        {
            d->wq_2D->thread_flag = 1;
            wake_up_interruptible(&d->wq_2D->waitQueue);//唤醒触发sensor的管脚和IO_SET_OUT_SYNC的管脚电平触发
        }
        kill_fasync(&share_async, SIGIO, POLLIN);
    }
    else if(irq == d->irqIo[1].irqNum)
    {
        triggerPin = IO_INPUT_2;
        g_triggerPin_input_count[1]++;
        kill_fasync(&share_async, SIGIO, POLLIN);
    }
    else
    {
        return IRQ_NONE;
    }
    return IRQ_HANDLED;
}
static int iomanager_fasync(int fd,struct file *pfile,int on)
{
	return fasync_helper(fd,pfile,on, &share_async);
}
static int iomanager_open(struct inode *inode,struct file *file)
{
    return 0;
}
static ssize_t iomanager_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	return 0;
}
static ssize_t iomanager_write(struct file *file, const char __user *buf,size_t count, loff_t *offset)
{
    return 0;
}
static int iomanager_release(struct inode *inode,struct file *file)
{
	return 0;
}

static int pwm_set(struct pwm_device *pwmStruct, struct pwm_state *state)
{
    return pwm_apply_state(pwmStruct, state);;
}
static int pwm_set_param(u32 rate, u8 duty, bool isOpen)
{
    struct pwm_state state;
    memset(&state, 0, sizeof(state));
    state.enabled = isOpen;
    if(false == isOpen)
    {
        return pwm_set(gdev->rgbPwm, &state);
    }
    state.duty_cycle = duty * 1000;
    state.period = 100000;
    return pwm_set(gdev->rgbPwm, &state);
}


int set_io_led_flash(void *mData) //
{
    unsigned int    us;
    u8              times;
    IO_FLASH_SET *data = (IO_FLASH_SET *)mData;
    u8              loop_index;
    vc_info(gdev->dev, "index:%d times:%d period:%d \n", data->index, data->times, data->period);
    us = data->period*1000;
    times = data->times;
    times = times>10 ? 10 : times;
    if(0 == us)
    {
        gpiod_set_value(gdev->flash_gpios->desc[data->index], 1);
        return 0;
    }
    for(loop_index=0;loop_index<times;loop_index++)
	{
        gpiod_set_value(gdev->flash_gpios->desc[data->index], 1);
        usleep_range(us, us + TRIG_OUT_PULSE_US_JIT);
        gpiod_set_value(gdev->flash_gpios->desc[data->index], 0);
        usleep_range(us, us + TRIG_OUT_PULSE_US_JIT);
	}
    return 0;
}

static enum hrtimer_restart thread_flash_led_func(struct hrtimer *t)
{
    WORK_STRUCT *d = container_of(t, WORK_STRUCT, hrtTimer);
    IO_FLASH_SET *mset = &d->flash_param;
    if(0x00 == (g_led_flash_count[mset->index] & 0x01))//开灯
    {
        gpiod_set_value(gdev->flash_gpios->desc[mset->index], 1);
    }
    else//关灯
    {
        gpiod_set_value(gdev->flash_gpios->desc[mset->index], 0);
    }
    if((2 * mset->times) >= g_led_flash_count[mset->index])//判断循环有没有结束
    {
        g_led_flash_count[mset->index]++;
        hrtimer_forward_now(t, d->kt);
        return HRTIMER_RESTART;
    }

    return HRTIMER_NORESTART;
}

static int my_led_flash_fn(void *data)
{
    WORK_STRUCT *ws = (WORK_STRUCT *)data;
    IO_FLASH_SET *mset = &ws->flash_param;
    hrtimer_init(&ws->hrtTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED_HARD);
    ws->hrtTimer.function = thread_flash_led_func;
    while (!kthread_should_stop())
    {
        wait_event_interruptible(ws->waitQueue, ws->thread_flag || kthread_should_stop());
        if (kthread_should_stop())
            break;
        if(0 == mset->times)
        {
            gpiod_set_value(gdev->flash_gpios->desc[mset->index], 1);
        }
        else
        {
            ws->kt = ktime_set(0, mset->period * 1000 * 1000);
            if (hrtimer_active(&ws->hrtTimer))
            {
                hrtimer_cancel(&ws->hrtTimer);
            }
            g_led_flash_count[mset->index] = 0;
            hrtimer_start(&ws->hrtTimer, ktime_set(0, 0), HRTIMER_MODE_REL_PINNED_HARD);
        }
        ws->thread_flag = 0; // 清除标志位
    }
    return 0;
}

enum hrtimer_restart expo_sync_timer_callback(struct hrtimer *timer)
{
    static bool state = false;
    if(false == state)//上升沿
    {
        gpiod_set_value(gdev->output_gpios->desc[IO_SET_OUT_SYNC], 1);
        gpiod_set_value(gdev->output_gpios->desc[RGBW_LED_TRIG], 1);
    }
    else if(true == state)//下降沿
    {
        gpiod_set_value(gdev->output_gpios->desc[IO_SET_OUT_SYNC], 0);
        gpiod_set_value(gdev->output_gpios->desc[RGBW_LED_TRIG], 0);
    }
    state = !state;
    if (++g_inter_pwm_count >= g_inter_pwm_target_count * 2)
        return HRTIMER_NORESTART;
    hrtimer_forward_now(timer, state ? g_int_ktime_on : g_int_ktime_off);
    return HRTIMER_RESTART;
}
void setUsrPwmFunc(USR_PWM_TIMER *param)
{
    static struct hrtimer pwm_timer;
    g_int_ktime_on = ktime_set(0, (param->pulseTime) * 1000);
    g_int_ktime_off = ktime_set(0, (param->periodTime - param->pulseTime) * 1000);
    g_inter_pwm_target_count = param->count;
    g_inter_pwm_count = 0;
    hrtimer_init(&pwm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED_HARD);
    pwm_timer.function = param->callBack;
    hrtimer_start(&pwm_timer, ktime_set(0, 0), HRTIMER_MODE_REL_PINNED_HARD); // 从高电平开始
}
static enum hrtimer_restart ext_sensor_xtrig_timer_callback(struct hrtimer *timer)
{
    gpiod_set_value(gdev->output_gpios->desc[RGBW_LED_TRIG], 0);
    return HRTIMER_NORESTART;  // 只执行一次
}

static enum hrtimer_restart ext_out_sync_timer_callback(struct hrtimer *timer)
{
    gpiod_set_value(gdev->output_gpios->desc[IO_SET_OUT_SYNC], 0);
    return HRTIMER_NORESTART;  // 只执行一次
}
static int ext_2d_exposure_fn(void *data)
{
    WORK_STRUCT *ws = (WORK_STRUCT *)data;
    INTERNAL_CONFIG *interConf;
    EXTERNAL_CONFIG *exterConf;
    int val = 0;
    USR_PWM_TIMER usrTime;
    struct hrtimer sensor_xtrig;
    struct hrtimer out_sync;
    //初始化外部的定时器函数
    hrtimer_init(&sensor_xtrig, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hrtimer_init(&out_sync, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sensor_xtrig.function = ext_sensor_xtrig_timer_callback;
    out_sync.function = ext_out_sync_timer_callback;
    while (!kthread_should_stop()) {
        wait_event_interruptible(ws->waitQueue, ws->thread_flag || kthread_should_stop());
        if (kthread_should_stop())
            break;
        if(0 == ws->light_param.mode)
        {
            interConf = &ws->light_param.modeParam.interConfig;
            usrTime.pulseTime = interConf->sync_expo;
            usrTime.periodTime = interConf->sync_period;
            usrTime.count = interConf->sync_num;
            usrTime.callBack = expo_sync_timer_callback;
            setUsrPwmFunc(&usrTime);
        }
        else if(1 == ws->light_param.mode) //等待外部2D管脚输入唤醒
        {
            exterConf = &ws->light_param.modeParam.exterConfig;
            gpiod_set_value(gdev->output_gpios->desc[RGBW_LED_TRIG], 1);
            gpiod_set_value(gdev->output_gpios->desc[IO_SET_OUT_SYNC], 1);
            hrtimer_start(&sensor_xtrig, ktime_set(0, exterConf->ext_xtrig_expo * 1000), HRTIMER_MODE_REL);
            hrtimer_start(&out_sync, ktime_set(0, exterConf->ext_sync_expo * 1000), HRTIMER_MODE_REL);
            val = atomic_inc_return(&ws->countNum);
            if(val>=exterConf->camNum)
            {
                m_disable_irq(&gdev->irqIo[IO_INPUT_1]);
            }
        }
        ws->thread_flag = 0;  // 清除标志位
    }
    return 0;
}

static enum hrtimer_restart my_thread_hrtTimer_func(struct hrtimer *t)
{
    WORK_STRUCT *d = container_of(t, WORK_STRUCT, hrtTimer);
    gpiod_set_value(gdev->output_gpios->desc[d->param.index], d->param.defaultState);
    vc_info(gdev->dev, "defaultState:%d index:%d\r\n", d->param.defaultState, d->param.index);
    return HRTIMER_NORESTART;
}
static int out_pulse_thread_fn(void *data) // 脉冲输出的线程
{
    WORK_STRUCT *ws = (WORK_STRUCT *)data;
    IO_PLUSE_SET *mSet = &ws->param;
    hrtimer_init(&ws->hrtTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED_HARD);
    ws->hrtTimer.function = my_thread_hrtTimer_func;
    while (!kthread_should_stop())
    {
        wait_event_interruptible(ws->waitQueue, ws->thread_flag || kthread_should_stop());
        if (kthread_should_stop())
            break;
        vc_info(gdev->dev, "mSet->index:%d mSet->holdTime:%d mSet->outState:%d\r\n", mSet->index, mSet->holdTime, mSet->outState);
        if (0 == mSet->holdTime)
        {
            gpiod_set_value(gdev->output_gpios->desc[mSet->index], mSet->outState);
        }
        else
        {
            gpiod_set_value(gdev->output_gpios->desc[mSet->index], mSet->outState);
            ws->kt = ktime_set(0, mSet->holdTime * 1000);
            hrtimer_start(&ws->hrtTimer, ws->kt, HRTIMER_MODE_REL_PINNED_HARD);
        }
        ws->thread_flag = 0; // 清除标志位
    }
    return 0;
}
static void rgb_param_legal_check(IO_RGB_SET *set, IO_RGB_DRIVE_SET *out)
{
    uint8_t i = 0;
    out->num = set->num;
    out->pwmValue = set->pwmValue;
    out->expoSensor = 0;
    for (i = 0; i < out->num; i++)
    {
        out->exposureTime[i] = set->exposureTime[i];
        if (out->exposureTime[i] > out->expoSensor)
        {
            out->expoSensor = out->exposureTime[i];
        }

        out->periodTime[i] = out->expoSensor + MIN_FALL_TIME;
        if (out->periodTime[i] < MIN_RISE_TIME)
        {
            out->periodTime[i] = MIN_RISE_TIME;
        }
        if (set->periodTime[i] > out->periodTime[i])
        {
            out->periodTime[i] = set->periodTime[i];
        }
    }
    for (i = 0; i < out->num; i++)
    {
        g_stage_1[i] = ktime_set(0, (out->exposureTime[i]) * 1000);
        g_stage_2[i] = ktime_set(0, (out->expoSensor - out->exposureTime[i]) * 1000);
        g_stage_3[i] = ktime_set(0, (out->periodTime[i] - out->expoSensor) * 1000);
        //printk("i:%d - %u %u %u \r\n", i, out->exposureTime[i], (out->expoSensor - out->exposureTime[i]), (out->periodTime[i] - out->expoSensor));
        // g_stage[0][i] = ktime_set(0, (out->exposureTime[i]) * 1000);
        // g_stage[1][i] = ktime_set(0, (out->expoSensor - out->exposureTime[i]) * 1000);
        // g_stage[2][i] = ktime_set(0, (out->periodTime[i] - out->expoSensor) * 1000);
    }
    return;
}
static long iomanager_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    u8 index = 0;
    u8 index_tou = 0;
    IO_FLASH_SET mIoFlashSet;
    IO_PLUSE_SET mIoPluseSet;
    SET_IO_TRIGGER_TYPE ioTrigType;
    IO_2D_SUPPORT_LIGHT_PARAM *lightParam = &gdev->wq_2D->light_param;
    IO_RGB_SYNC_SIGNAL mRgbSyncSet;
    IO_RGB_SET mRgbSet;
    IO_RGB_DRIVE_SET driveRgbSet;
    DEV_IO_RGB_TRIG_SET rgbTrigSet;
    const char *ver;
    IO_VERSION ioVer;
    u8 dlpStatus = 0;
    int ret = 0;
    switch (cmd)
    {
    case IO_GET_VERSION:
        vc_info(gdev->dev, "get ver:%s \r\n", BLIOMANAGER_FULL_VERSION);
        ver = BLIOMANAGER_FULL_VERSION;
        ioVer.len = strnlen(ver, DLP_VER_MAX_LEN - 1);
        memcpy(ioVer.ver, ver, ioVer.len);
        ioVer.ver[ioVer.len] = '\0';
        if (copy_to_user((IO_VERSION *)arg, &ioVer, sizeof(ioVer)))
        {
            return -EINVAL;
        }
        break;
    case IO_GET_TRIGGER_PIN:
            return triggerPin;
        break;
    case IO_GET_PIN_STATUS:
        if (copy_from_user(&index, (u8 *)arg, sizeof(u8)))
            return -EINVAL;
        vc_info(gdev->dev, "index:%d \n", index);
        if (index <= gdev->num_usb)
            return gpiod_get_value(gdev->usb_gpios->desc[index]);
        return -1;
        break;
    case IO_TEST:
        // vc_info(gdev->dev, "test\n");
        break;
    case IO_SET_OUT:
        if (copy_from_user(&mIoPluseSet, (struct IO_PLUSE_SET *)arg, sizeof(IO_PLUSE_SET)))
            return -EINVAL;
        index = mIoPluseSet.index;
        if (index >= gdev->num_outputs)
        {
            vc_err(gdev->dev, "set IO_SET_OUT index:%d cross %d\n", index, gdev->num_outputs);
            return -EINVAL;
        }
        memcpy(&gdev->wq_outpin[index].param, &mIoPluseSet, sizeof(IO_PLUSE_SET));
        gdev->wq_outpin[index].thread_flag = 1;
        wake_up(&gdev->wq_outpin[index].waitQueue);
        break;
    case IO_SET_FLASH_LED:
        if (copy_from_user(&mIoFlashSet, (struct IO_FLASH_SET *)arg, sizeof(IO_FLASH_SET)))
            return -EINVAL;
        index = mIoFlashSet.index;
        if (index > gdev->num_flash) // 避免越界访问
            return -EINVAL;
        memcpy(&gdev->wq_flash[index].flash_param, &mIoFlashSet, sizeof(mIoFlashSet));
        gdev->wq_flash[index].thread_flag = 1;
        wake_up(&gdev->wq_flash[index].waitQueue);
        break;
    case IO_SET_DLP_STATUS:
    {
        if (copy_from_user(&dlpStatus, (u8 *)arg, sizeof(u8)))
            return -EINVAL;
        vc_info(gdev->dev, "dlpStatus:0x%x \n", dlpStatus);
        for (index = 0; index < 4; index++)
        {
            if (dlpStatus & (1 << index))
            {
                gpiod_set_value(gdev->dlp_gpios->desc[index], 1);
                vc_info(gdev->dev, "set index:%d open\r\n", index);
            }
            else
            {
                gpiod_set_value(gdev->dlp_gpios->desc[index], 0);
                vc_info(gdev->dev, "set index:%d close\r\n", index);
            }
        }
    }
        break;
    case IO_SET_RGB_TRIG_CAM:
        {
            // if (copy_from_user(&cam_expose, (u32 *)arg, sizeof(u32)))
            //     return -EINVAL;
        }
        break;
    case IO_SET_USB_ENABLE_DLP_STATUS:
        if (copy_from_user(&dlpStatus, (u8 *)arg, sizeof(u8)))
            return -EINVAL;
        vc_info(gdev->dev, "usb enable dlp Status:%d \n", dlpStatus);
        if (1 == dlpStatus)
        {
            gpiod_set_value(gdev->usb_gpios->desc[0], 1);
        }
        else
        {
            gpiod_set_value(gdev->usb_gpios->desc[0], 0);
        }
        break;
    case IO_SET_RGB_PARAM:
        if(NULL == gdev->wq_rgb)
        {
            vc_err(gdev->dev, "not support rgb set\r\n");
            return -EINVAL;
        }
        memset(&driveRgbSet, 0, sizeof(driveRgbSet));
        if (copy_from_user(&mRgbSet, (struct IO_RGB_SET *)arg, sizeof(mRgbSet)))
            return -EINVAL;
        rgb_param_legal_check(&mRgbSet, &driveRgbSet);
        vc_info(gdev->dev, "mRgbSet.num:%u pwmValue:%u \n", mRgbSet.num, mRgbSet.pwmValue);
        vc_info(gdev->dev, "exposureTime r:%u g:%u b:%u w:%u", driveRgbSet.exposureTime[0], driveRgbSet.exposureTime[1],
                driveRgbSet.exposureTime[2], driveRgbSet.exposureTime[3]);
        vc_info(gdev->dev, "expoSensor:%u  periodTime:%u %u %u %u\r\n", driveRgbSet.expoSensor, driveRgbSet.periodTime[0],
                driveRgbSet.periodTime[1], driveRgbSet.periodTime[2], driveRgbSet.periodTime[3]);
        memcpy(&gdev->wq_rgb->rgb_param, &driveRgbSet, sizeof(driveRgbSet));
        pwm_set_param(10000, driveRgbSet.pwmValue, true);
        gdev->dlpRgbSet.rgb_mode = 0;
        m_disable_irq(&gdev->irqIo[CAM_XTRIG_1]);
        m_disable_irq(&gdev->irqIo[CAM_TOUT_0]);
        break;
    case IO_SET_IO_TRIGGER_PARAM:
        if (copy_from_user(&ioTrigType, (struct SET_IO_TRIGGER_TYPE *)arg, sizeof(SET_IO_TRIGGER_TYPE)))
            return -EINVAL;
        vc_info(gdev->dev, "ioTrigType.num:%d  ioTrigType:%d debounce:%d\n", ioTrigType.num, ioTrigType.trigType, ioTrigType.debounce);
        // if (IO_INPUT_2 < ioTrigType.num)--为了测试暂时关闭
        // {
        //     return -1;
        // }
        vc_info(gdev->dev, "ioTrigType.num:%d  ioTrigType:%d debounce:%d\n", ioTrigType.num, ioTrigType.trigType, ioTrigType.debounce);
        m_disable_irq(&gdev->irqIo[ioTrigType.num]);
        ret = irq_set_irq_type(gdev->irqIo[ioTrigType.num].irqNum, (0 == ioTrigType.trigType) ? IRQ_TYPE_EDGE_RISING : IRQ_TYPE_EDGE_FALLING); // 动态切换为双沿
        ret |= gpiod_set_debounce(gdev->input_gpios->desc[ioTrigType.num], ioTrigType.debounce);
        m_enable_irq(&gdev->irqIo[ioTrigType.num]);
        if (0 != ret)
        {
            vc_err(gdev->dev, "irq_set_irq_type set ret:%d \n", ret);
            return -2;
        }
        break;
    case IO_2D_SET_MODE_PARAM:
        if (copy_from_user(lightParam, (struct IO_2D_SUPPORT_LIGHT_PARAM *)arg, sizeof(IO_2D_SUPPORT_LIGHT_PARAM)))
            return -EINVAL;
        if(0 == lightParam->mode) //debug调试
        {
            vc_info(gdev->dev, "sync_num:%d \n", lightParam->modeParam.interConfig.sync_num);
            vc_info(gdev->dev, "sync_expo:%d \n", lightParam->modeParam.interConfig.sync_expo);
            vc_info(gdev->dev, "sync_period:%d \n", lightParam->modeParam.interConfig.sync_period);
        }
        else
        {
            vc_info(gdev->dev, "camNum:%d \n", lightParam->modeParam.exterConfig.camNum);
            vc_info(gdev->dev, "ext_sync_expo:%d \n", lightParam->modeParam.exterConfig.ext_sync_expo);
            vc_info(gdev->dev, "ext_xtrig_expo:%d \n", lightParam->modeParam.exterConfig.ext_xtrig_expo);
        }
        //配置sensor曝光需要的参数
        if(0 == lightParam->mode)
        {
            gdev->wq_2D->thread_flag = 1;
            wake_up(&gdev->wq_2D->waitQueue);
            m_disable_irq(&gdev->irqIo[IO_INPUT_1]); //禁用2D输入响应
        }
        else
        {
            atomic_set(&gdev->wq_2D->countNum, 0);
            m_enable_irq(&gdev->irqIo[IO_INPUT_1]); //开启2D输入响应
        }
        break;
    case IO_SET_GET_RGB_SYNC_SIGNAL:
        if (copy_from_user(&mRgbSyncSet, (IO_RGB_SYNC_SIGNAL *)arg, sizeof(IO_RGB_SYNC_SIGNAL)))
            return -EINVAL;
        vc_info(gdev->dev, "mode:%d \r\n", mRgbSyncSet.mode);
        vc_info(gdev->dev, "syncDlpNum:%d \r\n", mRgbSyncSet.syncDlpNum);
        vc_info(gdev->dev, "syncRgbNum:%d \r\n", mRgbSyncSet.syncRgbNum);
        vc_info(gdev->dev, "exposureTime:%u %u %u %u\r\n", mRgbSyncSet.exposureTime[0], \
                                mRgbSyncSet.exposureTime[1], mRgbSyncSet.exposureTime[2], mRgbSyncSet.exposureTime[3]);
        pwm_set_param(10000, mRgbSyncSet.pwmValue, true);
        gdev->dlpRgbSet.rgb_time[0] = ktime_set(0, mRgbSyncSet.exposureTime[0] * 1000);
        gdev->dlpRgbSet.rgb_time[1] = ktime_set(0, mRgbSyncSet.exposureTime[1] * 1000);
        gdev->dlpRgbSet.rgb_time[2] = ktime_set(0, mRgbSyncSet.exposureTime[2] * 1000);
        gdev->dlpRgbSet.rgb_time[3] = ktime_set(0, mRgbSyncSet.exposureTime[3] * 1000);
        //清零状态值
        atomic_set(&gdev->dlpRgbSet.xtrig_num, 0);
        atomic_set(&gdev->dlpRgbSet.tou_num, 0);
        atomic_set(&gdev->dlpRgbSet.tou_num_ext, 0);
        //赋值dlp和rgb灯亮灯的边界值
        atomic_set(&gdev->dlpRgbSet.totalTrigIndex, mRgbSyncSet.syncDlpNum);
        atomic_set(&gdev->dlpRgbSet.rgbTrigIndex, mRgbSyncSet.syncRgbNum);
        gdev->dlpRgbSet.rgb_enable = 1; //开启使能开关
        gdev->dlpRgbSet.rgb_mode = mRgbSyncSet.mode; //配置rgb亮灯的模式
        if(0 == gdev->dlpRgbSet.rgb_mode) //根据模式使能输入中断信号
        {
            m_disable_irq(&gdev->irqIo[CAM_XTRIG_1]);
            m_disable_irq(&gdev->irqIo[CAM_TOUT_0]);
        }
        else
        {
            m_enable_irq(&gdev->irqIo[CAM_XTRIG_1]);
            m_enable_irq(&gdev->irqIo[CAM_TOUT_0]);
        }
        break;
    case IO_SET_CLEAR_TRIG_COUNT:
        if(0 == gdev->dlpRgbSet.rgb_mode) //不需要统计参数了
        {
            return 0;
        }
        index     = atomic_read(&gdev->dlpRgbSet.xtrig_num);
        index_tou = atomic_read(&gdev->dlpRgbSet.tou_num);
        //清零状态值
        atomic_set(&gdev->dlpRgbSet.xtrig_num, 0);
        atomic_set(&gdev->dlpRgbSet.tou_num, 0);
        atomic_set(&gdev->dlpRgbSet.tou_num_ext, 0);
        if(index == index_tou)
        {
            if(0 == index)
            {
                return 1;
            }
            else if(index == atomic_read(&gdev->dlpRgbSet.totalTrigIndex))
            {
                return 0;
            }
            else
            {
                return 2;
            }
        }
        else
        {
            return 3;
        }
        break;
    case IO_SET_SINGLE_RGB_TRIG:
        if (NULL == gdev->rgbPwm)
        {
            vc_err(gdev->dev, "not support rgb set\r\n");
            return -EINVAL;
        }
        if (0 == gdev->dlpRgbSet.rgb_mode)
        {
            if (copy_from_user(&rgbTrigSet, (struct IO_RGB_DRIVE_SET *)arg, sizeof(rgbTrigSet)))
                return -EINVAL;
            index_tou = 0;
            for(index=0;index<4;index++)
            {
                if(rgbTrigSet.rgbw_on[index])
                {
                    g_stage_index[index_tou++] = index;
                }
            }
            gdev->wq_rgb->rgb_param.num = index_tou; //配置总的亮灯数量
            gdev->wq_rgb->thread_flag = 1;
            vc_info(gdev->dev, "IO_SET_TRIG_START \n");
            wake_up(&gdev->wq_rgb->waitQueue);
        }
        break;
    default:
        break;
    }
    return 0;
}

static const struct file_operations iomanager_fops = {
	.owner	=	THIS_MODULE,
	.open	=	iomanager_open,
	.read	=	iomanager_read,
	.write	=	iomanager_write,
	.fasync	=	iomanager_fasync,
	.release=	iomanager_release,
	.unlocked_ioctl	=	iomanager_ioctl,
};
static enum hrtimer_restart dlp_trig_rgb_func(struct hrtimer *t)
{
    gpiod_set_value(gdev->dlpRgbSet.rgb_gpios->desc[gdev->dlpRgbSet.last_irq_rgb], 0);
    return HRTIMER_NORESTART;
}
static int irq_dlp_rgb_thread_fn(void *data)
{
    DLP_RGB_SET_PARAM *d = data;
    u8 mRgbIndex = 0;
    u8 mXtrigIndex = 0;
    hrtimer_init(&d->rgbTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED_HARD);
    d->rgbTimer.function = dlp_trig_rgb_func;
    while (!kthread_should_stop())
    {
        wait_event_interruptible(d->wq_rgb, d->rgb_flag ||kthread_should_stop());
        if (kthread_should_stop())
            break;
        d->rgb_flag = 0;
        mRgbIndex = atomic_read(&d->rgbTrigIndex);
        mXtrigIndex = atomic_read(&d->xtrig_num);
        mRgbIndex = mXtrigIndex - mRgbIndex - 1;
        if(mRgbIndex < 4)
        {
            if (hrtimer_active(&d->rgbTimer))
            {
                hrtimer_cancel(&d->rgbTimer);
                return 0;
            }
            d->last_irq_rgb = mRgbIndex;
            if(1 == d->rgb_enable)
            {
                gpiod_set_value(d->rgb_gpios->desc[d->last_irq_rgb], 1);
                hrtimer_start(&d->rgbTimer, d->rgb_time[d->last_irq_rgb], HRTIMER_MODE_REL_PINNED_HARD);
            }
        }
    }
    return 0;
}  
static ssize_t irq_status_read(struct file *file,
                           char __user *buf,
                           size_t count, loff_t *ppos)
{
    char tmp[128];
    int len;
    /* 你自己的驱动数据 */
    len = snprintf(tmp, sizeof(tmp),
               "rgb_mode:%d\n"
               "xtrig_num:%d\n"
               "tou_num:%d\n"
               "tou_num_ext:%d\n"
               "rgbTrigIndex:%d\n"
               "totalTrigIndex:%d\n",
               gdev->dlpRgbSet.rgb_mode,
               atomic_read(&gdev->dlpRgbSet.xtrig_num),
               atomic_read(&gdev->dlpRgbSet.tou_num),
               atomic_read(&gdev->dlpRgbSet.tou_num_ext),
               atomic_read(&gdev->dlpRgbSet.rgbTrigIndex),
               atomic_read(&gdev->dlpRgbSet.totalTrigIndex));
    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static ssize_t io_status_read(struct file *file,
                           char __user *buf,
                           size_t count, loff_t *ppos)
{
    char tmp[128];
    int len;
    /* 你自己的驱动数据 */
    len = snprintf(tmp, sizeof(tmp),
               "rgbPwm:%s\n"
               "num_dlp:%u\n"
               "num_usb:%u\n"
               "num_inputs:%u\n"
               "num_outputs:%u\n"
               "num_flash:%u\n",
               gdev->rgbPwm ? "ok" : "NULL",
               gdev->num_dlp,
               gdev->num_usb,
               gdev->num_inputs,
               gdev->num_outputs,
               gdev->num_flash);
    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static ssize_t irq_set_read(struct file *file,
                            char __user *buf,
                            size_t count, loff_t *ppos)
{
    char tmp[256];
    int len = 0;
    u8 i = 0;
    for (i = 0; i < gdev->num_inputs && i < 5; i++)
    {
        const char *name;
        switch (i)
        {
        case 0:
            name = "2D";
            break;
        case 1:
            name = "3D";
            break;
        case 2:
            name = "cam3_xtrig";
            break;
        case 3:
            name = "cam3_tout0";
            break;
        case 4:
            name = "cam1_tout0";
            break;
        default:
            name = "unknown";
            break;
        }
        len += snprintf(tmp + len, sizeof(tmp) - len,
                        "%s irq num:%d isEnable:%d\n",
                        name,
                        gdev->irqIo[i].irqNum,
                        !gdev->irqIo[i].disabled);
        /* 防止溢出 */
        if (len >= sizeof(tmp))
            break;
    }
    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static ssize_t count_2d_read(struct file *file,
                        char __user *buf,
                        size_t count, loff_t *ppos)
{
    char tmp[64];
    int len;
    len = snprintf(tmp, sizeof(tmp),
                   "2d input count:%llu\n",
                   (unsigned long long)g_triggerPin_input_count[0]);
    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}
static ssize_t count_3d_read(struct file *file,
                        char __user *buf,
                        size_t count, loff_t *ppos)
{
    char tmp[64];
    int len;
    len = snprintf(tmp, sizeof(tmp),
                   "3d input count:%llu\n",
                   (unsigned long long)g_triggerPin_input_count[1]);
    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}
static ssize_t count_2d_write(struct file *file,
                        const char __user *buf,
                        size_t count, loff_t *ppos)
{
    char kbuf[16];
    if (count >= sizeof(kbuf))
        return -EINVAL;
    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    g_triggerPin_input_count[0] = 0;
    return count;
}
static ssize_t count_3d_write(struct file *file,
                        const char __user *buf,
                        size_t count, loff_t *ppos)
{
    char kbuf[16];
    if (count >= sizeof(kbuf))
        return -EINVAL;
    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    g_triggerPin_input_count[1] = 0;
    return count;
}
static const struct file_operations debugfs_irq_fops = {
    .owner = THIS_MODULE,
    .read  = irq_status_read,
};
static const struct file_operations debugfs_display_index = {
    .owner = THIS_MODULE,
    .read  = io_status_read,
};
static const struct file_operations debugfs_2d_count = {
    .owner = THIS_MODULE,
    .read  = count_2d_read,
    .write  = count_2d_write,
};
static const struct file_operations debugfs_3d_count = {
    .owner = THIS_MODULE,
    .read  = count_3d_read,
    .write  = count_3d_write,
};
static const struct file_operations debugfs_irq_set = {
    .owner = THIS_MODULE,
    .read  = irq_set_read,
};
static struct debugfs_node{
    char *name;
    const struct file_operations *fops;
    umode_t perm;
}io_fls[] = {
    {.name = "irq_count", .fops = &debugfs_irq_fops, .perm = S_IRUSR},\
    {.name = "io_index", .fops = &debugfs_display_index, .perm = S_IRUSR},\
    {.name = "2D_irq_count", .fops = &debugfs_2d_count, .perm = S_IRUSR | S_IWUSR},\
    {.name = "3D_irq_count", .fops = &debugfs_3d_count, .perm = S_IRUSR | S_IWUSR},\
    {.name = "irq_set", .fops = &debugfs_irq_set, .perm = S_IRUSR},\
};

static int debugfs_init(struct gpio_inout_dev *gpioDev)
{
    u8 i;
    iomanager_debugfs = debugfs_create_dir("iomanager_driver", NULL);
    if (!iomanager_debugfs)
        return -ENOMEM;
    for (i = 0; i < ARRAY_SIZE(io_fls); i++)
		debugfs_create_file(io_fls[i].name, io_fls[i].perm, iomanager_debugfs,
				    gpioDev, io_fls[i].fops);
    return 0;
}

static int iomanager_probe(struct platform_device *pdev)
{
    int i = 0;
    int ret = 0;
    char thread_name[10]={0};
    struct device *dev = &pdev->dev;
    u8 index = 0;
    cpumask_t mask;
    dev_info(dev, "branch:%s %s time:%s\n", GIT_BRANCH, GIT_COMMIT, PROGRAM_DATE);
    gdev = devm_kzalloc(dev, sizeof(struct gpio_inout_dev), GFP_KERNEL);
    if (!gdev)
        return -ENOMEM;
    gdev->dev = dev;
    //分配pwd资源
    gdev->rgbPwm = devm_pwm_get(dev, "pwm-rgb");
    if (IS_ERR(gdev->rgbPwm)) {
        vc_err(dev, "pwm-rgb get NULL!\n");
        gdev->rgbPwm = NULL;  // 标记为无效
    }
    else
    {
        pwm_set_param(10000, 10, 1);//测试pwm测试
    }
    gdev->dlp_gpios = devm_gpiod_get_array(dev, "dlp", GPIOD_OUT_LOW);
    if (IS_ERR(gdev->dlp_gpios))
        return PTR_ERR(gdev->dlp_gpios);
    gdev->num_dlp = gdev->dlp_gpios->ndescs;
    vc_info(dev, "gdev->num_dlp:%d \r\n", gdev->num_dlp);
    for(index=0;index<gdev->num_dlp;index++)
    {
        gpiod_set_value(gdev->dlp_gpios->desc[index], 1);
        msleep(300);
    }

    gdev->input_gpios = devm_gpiod_get_array(dev, "input", GPIOD_IN);
    if (IS_ERR(gdev->input_gpios))
        return PTR_ERR(gdev->input_gpios);
    gdev->num_inputs = gdev->input_gpios->ndescs;
    vc_info(dev, "gdev->num_inputs:%d \r\n", gdev->num_inputs);

    gdev->usb_gpios = devm_gpiod_get_array(dev, "usb", GPIOD_OUT_LOW);
    if (IS_ERR(gdev->usb_gpios))
        return PTR_ERR(gdev->usb_gpios);
    gdev->num_usb = gdev->usb_gpios->ndescs;
    vc_info(dev, "gdev->num_usb:%d \r\n", gdev->num_usb);
    msleep(100);
    gpiod_set_value(gdev->usb_gpios->desc[0], 1);

    gdev->output_gpios = devm_gpiod_get_array(dev, "output", GPIOD_OUT_LOW);
    if (IS_ERR(gdev->output_gpios))
        return PTR_ERR(gdev->output_gpios);
    gdev->num_outputs = gdev->output_gpios->ndescs;
    gpiod_set_value(gdev->output_gpios->desc[IO_RES_SET_TRIG_SENSOR], 0);
    vc_info(dev, "gdev->num_outputs:%d \r\n", gdev->num_outputs);

    gdev->flash_gpios = devm_gpiod_get_array(dev, "flash", GPIOD_OUT_LOW);
    if (IS_ERR(gdev->flash_gpios))
    {
        vc_err(dev, "gdev->flash_gpios get error \r\n");
        return PTR_ERR(gdev->flash_gpios);
    }
    gdev->num_flash = gdev->flash_gpios->ndescs;
    vc_info(dev, "gdev->num_flash:%d \r\n", gdev->num_flash);

    // 获取rgb的管脚信息
    gdev->dlpRgbSet.rgb_gpios = devm_gpiod_get_array(dev, "rgb", GPIOD_OUT_LOW);
    if (IS_ERR(gdev->dlpRgbSet.rgb_gpios))
    {
        vc_err(dev, "gdev->rgb_gpios get NULL \r\n");
        gdev->dlpRgbSet.rgb_gpios = NULL;
        gdev->dlpRgbSet.num_rgb = 0;
    }
    else
    {
        //初始化dlp+rgbw混合闪烁配置
        atomic_set(&gdev->dlpRgbSet.xtrig_num, 0x0100);
        atomic_set(&gdev->dlpRgbSet.tou_num, 0xfff);
        atomic_set(&gdev->dlpRgbSet.tou_num_ext, 0);
        gdev->dlpRgbSet.rgb_enable = 0;
        gdev->dlpRgbSet.rgb_mode = 0;
        gdev->dlpRgbSet.num_rgb = gdev->dlpRgbSet.rgb_gpios->ndescs;
        init_waitqueue_head(&gdev->dlpRgbSet.wq_rgb);
        gdev->dlpRgbSet.thread = kthread_run(irq_dlp_rgb_thread_fn, &gdev->dlpRgbSet, "irq_rgb");
        if (IS_ERR(gdev->dlpRgbSet.thread))
        {
            vc_err(dev, "thread create failed\n");
            return PTR_ERR(gdev->dlpRgbSet.thread);
        }
        //初始化rgb单独闪烁配置
        gdev->wq_rgb = devm_kzalloc(dev, sizeof(WORK_STRUCT), GFP_KERNEL);
        if (!gdev->wq_rgb)
            return -ENOMEM;
        init_waitqueue_head(&gdev->wq_rgb->waitQueue);
        gdev->wq_rgb->thread = kthread_run(rgb_flash_fn, gdev->wq_rgb, "rgb_thread");
        if (IS_ERR(gdev->wq_rgb->thread))
        {
            vc_err(dev, "thread create failed\n");
            return PTR_ERR(gdev->wq_rgb->thread);
        }
        cpumask_clear(&mask);
        cpumask_set_cpu(CPU_RT_TIME_ID, &mask);
        set_cpus_allowed_ptr(gdev->dlpRgbSet.thread, &mask);
        set_cpus_allowed_ptr(gdev->wq_rgb->thread, &mask);
        vc_info(dev, "gdev->dlpRgbSet.num_rgb:%d \r\n", gdev->dlpRgbSet.num_rgb);
    }

    //申请一个2D曝光模式的消息队列
    gdev->wq_2D = devm_kzalloc(dev, sizeof(WORK_STRUCT), GFP_KERNEL);
    if (!gdev->wq_2D)
        return -ENOMEM;
    init_waitqueue_head(&gdev->wq_2D->waitQueue);
    snprintf(thread_name, sizeof(thread_name), "2D_expo");
    atomic_set(&gdev->wq_2D->countNum, 0);
    gdev->wq_2D->thread = kthread_run(ext_2d_exposure_fn, gdev->wq_2D, thread_name);
    //申请led闪烁消息队列
    gdev->wq_flash = devm_kzalloc(dev, sizeof(WORK_STRUCT) * gdev->num_flash, GFP_KERNEL);
    if (!gdev->wq_flash)
        return -ENOMEM;
    for (i = 0; i < gdev->num_flash; i++) { //对常驻内核线程的声明
        init_waitqueue_head(&gdev->wq_flash[i].waitQueue);
        snprintf(thread_name, sizeof(thread_name), "flash_%d", i);
        atomic_set(&gdev->wq_flash[i].countNum, 0);
        gdev->wq_flash[i].thread = kthread_run(my_led_flash_fn, &gdev->wq_flash[i], thread_name);
    }
    //申请输出配置消息队列
    gdev->wq_outpin = devm_kzalloc(dev, sizeof(WORK_STRUCT) * gdev->num_outputs, GFP_KERNEL);
    if (!gdev->wq_outpin)
        return -ENOMEM;
    for (i = 0; i < gdev->num_outputs; i++) { //对常驻内核线程的声明
        init_waitqueue_head(&gdev->wq_outpin[i].waitQueue);
        snprintf(thread_name, sizeof(thread_name), "td_%d", i);
        atomic_set(&gdev->wq_outpin[i].countNum, 0);
        gdev->wq_outpin[i].thread = kthread_run(out_pulse_thread_fn, &gdev->wq_outpin[i], thread_name);
    }
    // 申请中断
    gdev->irqIo = devm_kzalloc(dev, sizeof(IRQ_CTRL) *  gdev->num_inputs, GFP_KERNEL);
    if (!gdev->irqIo)
        return -ENOMEM;
    for (i = 0; i <  gdev->num_inputs; i++)
    {
        gdev->irqIo[i].irqNum = gpiod_to_irq(gdev->input_gpios->desc[i]);
        vc_info(dev, "gdev->irqIo[%d]:%d\n", i, gdev->irqIo[i].irqNum);
        if (gdev->irqIo[i].irqNum < 0)
        {
            dev_err(dev, "Failed to get IRQ for gdev->irqIo[%d]:%d \n", i, gdev->irqIo[i].irqNum);
            return gdev->irqIo[i].irqNum;
        }
        snprintf(thread_name, sizeof(thread_name), "io_%d", i);
        if(i<(gdev->num_inputs - 2))
        {
            ret = devm_request_irq(dev, gdev->irqIo[i].irqNum, gpio_irq_handler, IRQF_TRIGGER_RISING, thread_name, gdev);//使用硬中断
        }
        else if(i < (gdev->num_inputs -1))
        {
            ret = devm_request_irq(dev, gdev->irqIo[i].irqNum, gpio_irq_handler,  IRQF_TRIGGER_FALLING, thread_name, gdev);//使用硬中断,IRQF_TRIGGER_FALLING
        }
        else
        {
            ret = devm_request_irq(dev, gdev->irqIo[i].irqNum, gpio_irq_handler, IRQF_TRIGGER_RISING, thread_name, gdev);//使用硬中断
        }
        if (ret)
        {
            dev_err(dev, "Failed to request IRQ for input[%d]\n", i);
            return ret;
        }
        m_disable_irq(&gdev->irqIo[i]);//默认都是不开启中断的
    }
    gdev->major = register_chrdev(0,DEVICE_NAME,&iomanager_fops);
    gdev->gpio_manager_class   = class_create(THIS_MODULE, DEVICE_NAME);
    gdev->gpio_manager_dev   = device_create(gdev->gpio_manager_class, NULL, MKDEV(gdev->major, 0), NULL, DEVICE_NAME);
    platform_set_drvdata(pdev, gdev);
    debugfs_init(gdev);
    vc_info(dev, "probe success\n");
    return 0;
}

static int iomanager_remove(struct platform_device *pdev)
{
    int i = 0;
    struct gpio_inout_dev *mdev = platform_get_drvdata(pdev);
    debugfs_remove_recursive(iomanager_debugfs);
    for (i = 0; i < mdev->num_inputs; i++)
    {
        m_disable_irq(&mdev->irqIo[i]);
    }
    if(NULL != mdev->rgbPwm)
        pwm_disable(mdev->rgbPwm);
    unregister_chrdev(mdev->major,DEVICE_NAME);
	device_unregister(mdev->gpio_manager_dev);
    class_destroy(mdev->gpio_manager_class);
    return 0;
}
static struct platform_driver of_gpio_manager_driver = {
	.probe		= iomanager_probe,
	.remove		= iomanager_remove,
	.driver		= {
		.name	= DEVICE_NAME,
		.of_match_table = of_gpio_manager_match,
	},
};
module_platform_driver(of_gpio_manager_driver);

MODULE_VERSION(BLIOMANAGER_FULL_VERSION);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bopixel,yz");
MODULE_DESCRIPTION(DRIVER_DESC);