/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  watch_gprs.c
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  03/01/2017 11:42:14 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), houwentaoff@gmail.com
 *   Organization:  Jz
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "gpio.h"
#include "include.h"
#include "debug.h"

#define LED1   GPIO_TO_PIN(1,4)

#define DEVICE_TTY    "/dev/ttyUSB0"
#define DEVICE_PPP0   "/sys/class/net/ppp0/address"

#define POWER_SCRIPT  "/etc/init.d/S60power3g"
#define PPPD_SCRIPT   "/etc/init.d/S70pppd"

typedef enum procst
{
    RUNNING,
    ZOMBIE,    
    NOEXIST,
    UNDEFINED,
}procst_e;

procst_e get_procstatus(const char *name)
{
    char cmd_buf[512]={0};
    char buf[512]={0};
    FILE* fp =NULL;

    sprintf(cmd_buf, "ps -ef | grep \"%s\" | grep -v grep", name);

    if(NULL != (fp = popen(cmd_buf, "r")))
    {
        if (NULL == fgets(buf, 512, fp))
        {
            pclose(fp);
            return NOEXIST;
        }
        if(strcmp(buf, "") == 0)
        {
            pclose(fp);
            return NOEXIST;
        }
    }
    else
    {
        pclose(fp);
        wt_err("popen error\n");
        return UNDEFINED;
    }

    pclose(fp);
    return RUNNING;
}
#define MAX_RETRY     5   

bool is_overnet()
{
    char cmd[512]={0};
    int ret = 0;
    int count = 0;
    
    if (access(DEVICE_PPP0, F_OK) < 0)
    {
        return false;
    }
    /* -----------------------------------------------------------------------------
     *  ping -I cell->device_name
     *-----------------------------------------------------------------------------*/
    while (count++ < 3)
    {
        sprintf(cmd, "ping -I %s 8.8.8.8 -c 1 -s 1 -W 1 |grep '1 packets received'", "eth0");
        ret = system(cmd);
        if (ret == 0)
        {
            break;
        }
        sprintf(cmd, "ping -I %s 8.8.8.8 -c 1 -s 1 -W 1 |grep '1 packets received'", "ppp0");
        ret = system(cmd);//time out ?
        if (ret == 0)
        {
            break;
        }
    }
    if (ret != 0)
    {
        return false;
    }
    return true;
}

static int gprs_reset()
{    
    int count = 0;
    char   buf[40]={0};

    sprintf(buf, POWER_SCRIPT" %s", "reset");
    system(buf);
    sleep(16);
    while (count++ < MAX_RETRY)
    {
        if (access(DEVICE_TTY, F_OK) < 0)
        {
            sleep(5);
        }
        else
        {
            break;
        }
    }
    if (count >= MAX_RETRY)
    {
        return -1;
    }
    return 0;
}
static int call()
{
    int ret = 0;
    char   buf[40]={0};
    int i =0;
    
    if (access(DEVICE_TTY, F_OK) < 0)
    {
        ret = 0;
        return ret;
    }
    sprintf(buf, PPPD_SCRIPT" %s", "start");
    for (i=0; i<MAX_RETRY; i++)
    {
        printf("attempt the %d time:", i+1);
        ret = system(buf);
        if (ret != 0)
        {
            printf("call failed\n");
            sleep(i*5);
            continue;
        }
        printf("call successed!!!\n");
        break;
    }
    return !ret;
}
static int gprs_run_hook(char* name)
{
    char   buf[40]={0};
    int i = 0;
    int ret = 0;
    static bool first = true;

    if (first == false)
    {
        printf("attempt the first reset\n");
        if (gprs_reset() < 0)
        {   
            printf("attempt the seconed reset\n");
            gprs_reset();
        }
    }
    if (first == true)
    {
        first = false;
    }
    if (access(DEVICE_TTY, F_OK) < 0)
    {
        ret = 0;
        return ret;
    }
    sprintf(buf, PPPD_SCRIPT" %s", name);
    for (i=0; i<MAX_RETRY; i++)
    {
        printf("attempt the %d time:", i+1);
        ret = system(buf);
        if (ret != 0)
        {
            printf("call failed\n");
            sleep(i*5);
            continue;
        }
        printf("call successed!!!\n");
        break;
    }
    return !ret;
}
static bool online = false;
static bool calling = false;
void *show_stat(void *param)
{
    gpio_export(LED1);
    gpio_direction_output(LED1, 0);
    while(1)
    {
        if (online)
        {
            int value = gpio_get_value(LED1);
            //联网状态正常 一闪一闪的
            usleep(850000);//0.5s
            printf("set value[%d]\n", value);
            gpio_set_value(LED1, value == GPIO_HIGH ? GPIO_LOW : GPIO_HIGH);
        }
        else
        {
            if (calling)
            {
                //正在拨号 快闪
                int value = gpio_get_value(LED1);
                gpio_set_value(LED1, value == GPIO_HIGH ? GPIO_LOW : GPIO_HIGH);
            }
            else
            {
                //网络不通灭灯
                //int value = gpio_get_value(LED1);
                gpio_set_value(LED1, GPIO_HIGH);
            }
        }
        usleep(150000);//0.5s
    }
    return NULL;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    pthread_t net_tid;
    pthread_create (&net_tid, NULL, show_stat, NULL);
    int count = 0;
    
    while (1)
    {
        if (!is_overnet())
        {
            online = false;
            if (get_procstatus("pppd") == NOEXIST)
            {
#if 1
                if (count++ < MAX_RETRY)  
                {
                    sleep(count*5);
                    printf("attemp call %d\n", count);
                    if (call())
                    {
                        calling = true;
                    }
                    else
                    {
                        calling = false;
                    }
                }
                else
                {
                    if (gprs_run_hook("restart"))
                    {
                    
                        calling = true;
                    }
                    else
                    {
                        calling = false;
                    }
                    count = 0;
                }                
#endif
            }
            else
            {
                calling = true;
            }

        }
        else
        {
            online = true;
            calling = false;
        }
        sleep(1);
    }
    return EXIT_SUCCESS;
}
