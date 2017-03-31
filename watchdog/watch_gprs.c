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
#include "gpio.h"
#include "include.h"
#include "debug.h"

#define LED1   GPIO_TO_PIN(1,4)

typedef enum procst
{
    RUNNING,
    ZOMBIE,    
    EXITED,
    UNDEFINED,
}procst_e;

procst_e get_procstatus(const char *name)
{
    char cmd_buf[512]={0};
    char buf[512]={0};
    FILE* fp =NULL;

    sprintf(cmd_buf, "ps -ef | grep \"%s\" | grep -v grep", name);

    if(NULL != (fp =popen(cmd_buf, "r")))
    {
        while(NULL != fgets(buf, 512, fp) ) 
        {
        }
        if(strcmp(buf, "") == 0)
        {
            pclose(fp);
            return EXITED;
        }
    }
    else
    {
        wt_err("popen error\n");
        return UNDEFINED;
    }

    pclose(fp);
    return RUNNING;
}
bool is_overnet()
{
    char cmd[512]={0};
    int ret = 0;
    /* -----------------------------------------------------------------------------
     *  ping -I cell->device_name
     *-----------------------------------------------------------------------------*/
    sprintf(cmd, "ping -I %s 8.8.8.8 -c 1 |grep '1 packets received'", "ppp0");
    ret = system(cmd);
    if (ret != 0)
    {
        return false;
    }
    return true;
}
static int gprs_run_hook(char* name)
{
    char   buf[40]={0};
// sprintf(buf, "/etc/init.d/S70pppd %s", name);
    sprintf(buf, "/mnt/data/S60powergps %s", name);
    system(buf);
    sleep(3);
    sprintf(buf, "/mnt/data/S70pppd %s", name);
    return !system(buf);
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

    while (1)
    {
        if (!is_overnet())
        {
            online = false;
            if (get_procstatus("pppd") == EXITED)
            {
                if (gprs_run_hook("restart"))
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
