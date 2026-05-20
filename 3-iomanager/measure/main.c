#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include "../usr_io.h"
#include <errno.h>
#include "../../2-DLP3010/dlp3010.h"
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int g_fd;
IO_PLUSE_SET g_set;
int getPin;
void signalHandlerTrig(int sig)
{
    getPin = ioctl(g_fd, IO_GET_TRIGGER_PIN, &getPin);
    // printf("sig:%d signalHandlerTrig:%d pin:%d\r\n", sig, SIGUSR1, getPin);
    switch(getPin)
    {
        case IO_INPUT_1:
            printf("IO_INPUT_1 IRQF_TRIGGER_RISING\n");
            break;
        case IO_INPUT_2:
            printf("IO_INPUT_2 IRQF_TRIGGER_RISING\n");
            break;
        case IO_DLP_1:
            printf("IO_DLP_1 IRQF_TRIGGER_RISING\n");
            break;
        case IO_DLP_2:
            printf("IO_DLP_2 IRQF_TRIGGER_RISING\n");
            break;
        case IO_DLP_3:
            printf("IO_DLP_3 IRQF_TRIGGER_RISING\n");
            break;
        case IO_DLP_4:
            // g_set.defaultState = 0;
            // g_set.holdTime = 0;
            g_set.outState = 1;
            // g_set.index =  IO_SET_OUT_DLP1;
            ioctl(g_fd, IO_SET_OUT, &g_set); //拉低电平
            // printf("IO_DLP_4 IRQF_TRIGGER_RISING\n");
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    IO_PLUSE_SET m_set;
    IO_FLASH_SET flashSet;
    pthread_t threads;
    int thread_ids;
    DLP_CONTROL control;

    printf("---------main---------\n");
    memset(&m_set, 0, sizeof(m_set));
    g_fd = open("/dev/iomanager", O_RDWR);
    if (g_fd < 0)
    {
        printf("Error: open failed %d\n", g_fd);
        return -1;
    }
    int dlp_fd = open("/dev/dlp3010_3", O_RDWR);

    int dlp,dlp_id,dlp_status;

    switch (argc)
    {
    case 2:
        dlp = atoi(argv[1]);
        break;
    case 3:
        dlp = atoi(argv[1]);
        dlp_id = atoi(argv[2]);
        break;
    case 4:
        dlp = atoi(argv[1]);
        dlp_id = atoi(argv[2]);
        dlp_status = atoi(argv[3]);
        break;
    default:
        break;
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGIO);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL); //
    signal(SIGIO, signalHandlerTrig);
    fcntl(g_fd, F_SETOWN, getpid());
    int flag = fcntl(g_fd, F_GETFL);
    fcntl(g_fd, F_SETFL, flag | FASYNC);

    control.length = 3;
    control.data[0] = 0x9E;
    control.data[1] = 0;
    control.data[2] = 0;

    g_set.defaultState = 0;
    g_set.holdTime = 0;
    g_set.outState = 0;

    ret = ioctl(dlp_fd, DLP_IOCTL_SET_DATA, &control);
    g_set.index =  IO_SET_OUT_DLP1;
    ioctl(g_fd, IO_SET_OUT, &g_set); //拉高电平

    printf("ret:%d \n", ret);
    
    while (1)
    {
        pause(); // 等待信号
    }
    close(g_fd);
    return 0;
}

