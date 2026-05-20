#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "../usr_io.h"
#include <errno.h>
#include "define.h"
#include <linux/types.h>

// #define u8 (unsigned char)
// #define u32 (unsigned int)

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int g_fd = 0;
IO_PLUSE_SET g_set;
int getPin;

static int mI = 0;
int i = 0;
int iNum = 0;
void signalHandlerTrig_1(int sig)
{
    getPin = ioctl(g_fd, IO_GET_TRIGGER_PIN, &getPin);
    // printf("sig:%d signalHandlerTrig:%d pin:%d\r\n", sig, SIGUSR1, getPin);
    switch(getPin)
    {
        case IO_INPUT_1:
            printf("IO_INPUT_1 \n");
            break;
        case IO_INPUT_2:
            printf("IO_INPUT_2 \n");
            break;
        default:
            break;
    }
    // g_set.defaultState = 0;
    // g_set.holdTime = 300;
    // g_set.outState = 1;
    // ioctl(g_fd, IO_SET_OUT, &g_set);
}
static unsigned int num=0;
static int Pin;
static unsigned int irqNum = 0;

void test(void)
{
     irqNum++;

    if(0 == irqNum % 100)
    {
        printf("irqNum %d  \n", irqNum);
    }
    //printf("Enter trig_numbers :%d  ", num++);
    Pin = ioctl(g_fd, IO_GET_TRIGGER_PIN, &Pin);

    //printf("sig:%d signalHandlerTrig:%d pin:%d\r\n", sig, SIGUSR1, Pin);
    switch(Pin)
    {
    case IO_INPUT_1:
        printf("IO_INPUT_1 IRQF_TRIGGER_RISING\n");
        break;
    case IO_INPUT_2:
        printf("IO_INPUT_2 IRQF_TRIGGER_RISING\n");
        break;
    default:
        break;
    }
    num++;
}

void signalHandlerTrig(int sig)
{
    Pin = ioctl(g_fd, IO_GET_TRIGGER_PIN, &Pin);
    printf("sig:%d signalHandlerTrig:%d pin:%d\r\n", sig, SIGUSR1, Pin);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    IO_PLUSE_SET m_set;
    IO_FLASH_SET flashSet;
    IO_RGB_SET mRgbSet;
    pthread_t threads;
    int thread_ids;
    printf("---------main---------\n");
    memset(&m_set, 0, sizeof(m_set));
    g_fd = open("/dev/iomanager", O_RDWR);
    if (g_fd < 0)
    {
        printf("Error: open failed %d\n", g_fd);
        return -1;
    }
    printf("g_fd:%d \n", g_fd);
    // unsigned char parameter_1,parameter_2,parameter_3;

    int parameter_1 = 0,parameter_2 = 0,parameter_3 = 0;

    switch (argc)
    {
    case 2:
        parameter_1 = atoi(argv[1]);
        break;
    case 3:
        parameter_1 = atoi(argv[1]);
        parameter_2 = atoi(argv[2]);
        break;
    case 4:
        parameter_1 = atoi(argv[1]);
        parameter_2 = atoi(argv[2]);
        parameter_3 = atoi(argv[3]);
        break;
    default:
        break;
    }
    printf("parameter:%d %d %d \n",parameter_1, parameter_2, parameter_3);

#if TWO_SENSOR_TRIG
    m_set.defaultState = 0;
    m_set.holdTime = 1000 + 1000 * parameter_3;
    m_set.outState = 1;
    m_set.index =IO_RES_SET_TRIG_SENSOR;
    int i = (0 == parameter_2)?1:parameter_2;
    while(i--)
    {
        if(i<0)
            break;
        printf("=============i:%d  %d \n", i, m_set.holdTime);
        ioctl(g_fd, IO_SET_OUT, &m_set);
        usleep(parameter_1 * 100000 + 100000);
    }

#endif

#if CAMERA_TRIG
    m_set.defaultState = 0;
    m_set.holdTime = 1000 + 1000 * parameter_3;
    m_set.outState = 1;
    m_set.index =CAM_XTRIG_1;
    int i = (0 == parameter_2)?1:parameter_2;
    while(i--)
    {
        if(i<0)
            break;
        printf("=============i:%d\n", i);
        ioctl(g_fd, IO_SET_OUT, &m_set);
        usleep(parameter_1 * 100000 + 100000);
    }
    
#endif

#if RGB_PWM_TRIG
#define PERIOD_TIME 20000
    int i = 0;
    DEV_IO_RGB_TRIG_SET trg_set;
    mRgbSet.num = 4;
    mRgbSet.pwmValue = 90;
    mRgbSet.periodTime[0] = PERIOD_TIME;
#if 1
    mRgbSet.exposureTime[0] = 2215; // 4毫秒
    mRgbSet.exposureTime[1] = 3000; // 4毫秒
    mRgbSet.exposureTime[2] = 1322; // 4毫秒
    mRgbSet.exposureTime[3] = 1666; // 4毫秒
#else
    mRgbSet.exposureTime[0] = PERIOD_TIME; // 4毫秒
    mRgbSet.exposureTime[1] = PERIOD_TIME; // 4毫秒
    mRgbSet.exposureTime[2] = PERIOD_TIME; // 4毫秒
    mRgbSet.exposureTime[3] = PERIOD_TIME; // 4毫秒
#endif
    unsigned int periodTime = PERIOD_TIME*4;

    trg_set.rgbw_on[0] = true;
    trg_set.rgbw_on[1] = true;
    trg_set.rgbw_on[2] = false;
    trg_set.rgbw_on[3] = true;

    periodTime = ioctl(g_fd, IO_SET_RGB_PARAM, &mRgbSet);
    printf("periodTime:%d \n",periodTime);
    
    ioctl(g_fd, IO_SET_SINGLE_RGB_TRIG, &trg_set);

#endif

#if DLP_USB_TEST
    unsigned char bOpen = parameter_1;//1
    unsigned char index = parameter_2; //8 4 2 1
    ioctl(g_fd, IO_SET_USB_ENABLE_DLP_STATUS, &bOpen);
    ioctl(g_fd, IO_SET_DLP_STATUS, &index);
#endif

#if DLP_OPEN
    printf("parameter_1:%d \r\n", parameter_1);
    ioctl(g_fd, IO_SET_DLP_STATUS, &parameter_1); //测试dlp总电源开关
#endif 

#if DLP_OUT
    m_set.defaultState = 0;
    m_set.holdTime = parameter_1;
    m_set.outState = parameter_2;
    m_set.index = (IO_OUT_INDEX)(parameter_3+IO_SET_OUT_CC);
    ioctl(g_fd, IO_SET_OUT, &m_set);
#endif

#if IO_OUT
    // int i = 10;
    m_set.defaultState = 0;
    m_set.holdTime = parameter_1;
    m_set.outState = parameter_2;
    m_set.index =CAM_XTRIG_1; //范围0-2
    // while(i--)
    // {
    //     if(i<0)
    //         break;
        // printf("=============i:%d\n", i);
        ioctl(g_fd, IO_SET_OUT, &m_set);
        // usleep(parameter_1+000);
        // usleep(500000);
    // }

#endif

#if IO_IRQ
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGIO);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL); //
    signal(SIGIO, signalHandlerTrig);
    fcntl(g_fd, F_SETOWN, getpid());
    int flag = fcntl(g_fd, F_GETFL);
    fcntl(g_fd, F_SETFL, flag|FASYNC);
    
    SET_IO_TRIGGER_TYPE ioType = 
    {
        .num = parameter_1,
        .trigType = parameter_2,
        .debounce = parameter_3,
    };
    ioctl(g_fd, IO_SET_IO_TRIGGER_PARAM, &ioType);
#endif

#if LED_TEST
    for (int i = 0; i < 3; i++)
    {
        flashSet.index = i;
        flashSet.times = 2;
        flashSet.period = 500;
        ioctl(g_fd, IO_SET_FLASH_LED, &flashSet);
        printf("i:%d \r\n", i);
        sleep(2);
    }
#endif

#if BOARD_ID
    ret = ioctl(g_fd, IO_GET_PIN_STATUS, &parameter_1);
    printf("get led_%d value:%d \n", parameter_1, ret);
#endif

#if SENSOR_OUT
    int i = parameter_1;
    m_set.defaultState = 0;
    m_set.holdTime = 10000;
    m_set.outState = 1;
    m_set.index = CAM_XTRIG_1; 
    while(i--)
    {
        if(i<0)
            break;
        ret = ioctl(g_fd, IO_SET_OUT, &m_set);
        printf("i:%d ret:%d \n", i, ret);
        usleep(100000);
    }
#endif

#if RGB_TEST
    mRgbSet.num = parameter_1;
    mRgbSet.states[0] = 0x1F;
    mRgbSet.states[1] = 0x2F;
    mRgbSet.states[2] = 0x4F;
    mRgbSet.states[3] = 0x8F;
    mRgbSet.exposureTime = 10000; //10毫秒
    mRgbSet.periodTime = 1000000;//1秒时间
    ret = ioctl(g_fd, IO_SET_RGB_PARAM, &mRgbSet);

#endif    

#if IO_2D_MODE_TEST

    IO_2D_SUPPORT_LIGHT_PARAM interConfig = 
    {
        .mode = 0,
        .modeParam.interConfig = {
            .sync_num = 3,
            .sync_expo = 10000,
            .sync_period = 1000000
        }
    };
    IO_2D_SUPPORT_LIGHT_PARAM externConfig = 
    {
        .mode = 1,
        .modeParam.exterConfig = {
            .camNum = 1,
            .ext_sync_expo = 10000,
            .ext_xtrig_expo = 10000
        }
    };
    if (0 == parameter_1)
    {
        ret = ioctl(g_fd, IO_2D_SET_MODE_PARAM, &externConfig);
    }
    else
    {
        ret = ioctl(g_fd, IO_2D_SET_MODE_PARAM, &interConfig);
    }

#endif

#if IO_IRQ
    while (1) {
        pause();  // 等待信号
    }
#endif
    close(g_fd);
    return 0;
}

