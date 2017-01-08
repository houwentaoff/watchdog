/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  auto_conn.c
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  01/06/2017 05:14:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), houwentaoff@gmail.com
 *   Organization:  Jz
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "include.h"

typedef enum nettype
{
    ETH = 0,
    WIFI,
    GPRS,
}nettype_e;

typedef struct netcell
{
    nettype_e type;
    int priority;
    char *device_name;
    int (*start)(struct netcell *this);
    void (*stop)();
}netcell_t;
typedef enum conn_state
{
    CONNECTED = 0,
    DISCONNECT,
}connstate_e;
typedef struct netmanage
{
    netcell_t *activate;
    connstate_e state;
    
}netmanage_t;
static netcell_t netcells[] = 
{
 {.type = ETH,  .priority = 0},
 {.type = WIFI, .priority = 1},
 {.type = GPRS, .priority = 2},
};
static netmanage_t net_manage;

void update_netmanage(netcell_t *cell)
{
    net_manage.activate = cell;
}
bool is_overnet(netcell_t *cell)
{
    char cmd[512]={0};
    int ret = 0;
    /*-----------------------------------------------------------------------------
     *  ping -I cell->device_name
     *-----------------------------------------------------------------------------*/
    if (!cell || !cell->device_name)
    {
        return -1;
    }
    sprintf(cmd, "ping -I %s 8.8.8.8 -c 1 |grep '1 received'", cell->device_name);
    ret = system(cmd);
    if (ret != 0)
    {
        return false;
    }
    return true;
}
char *get_nicname(nettype_e type)
{
    char *name = NULL;
    switch(type)
    {
        case ETH:
            name = "eth0";
            break;
        case WIFI:
            name = "wlan0";
            break;
        case GPRS:
            name = "ppp0";
            break;
        default:
            name = "eth0";
            break;
    }
    return name;
}
static int eth_start(netcell_t *this)
{
    char cmd[512]={0};
    int ret = 0;
    //start success
    if (!this)
    {
        return -1;
    }
    sprintf(cmd, "service networking %s start", this->device_name);
    ret = system(cmd);
    return (ret!=0 ? -1:0);
}
static int gprs_start(netcell_t *this)
{
    char cmd[512]={0};
    int ret = 0;
    //start success
    if (!this)
    {
        return -1;
    }
    sprintf(cmd, "service networking %s start", this->device_name);
    ret = system(cmd);
    return (ret!=0 ? -1:0);
}
static int wifi_start(netcell_t *this)
{
    char cmd[512]={0};
    int ret = 0;
    //start success
    if (!this)
    {
        return -1;
    }
    sprintf(cmd, "service networking %s start", this->device_name);
    ret = system(cmd);
    return (ret!=0 ? -1:0);
}

static void netbase_stop(netcell_t *this)
{
    char cmd[512]={0};
    //start success
    sprintf(cmd, "service networking %s stop", this->device_name);
    system(cmd);
}

static void eth_stop()
{
    
}
static void gprs_stop()
{
    
}
static void wifi_stop()
{
    
}

static int init_netcells(netcell_t *cells)
{
    int ret = 0;
    int i =0;
    char *name = NULL;

    if (!cells)
    {
        ret = -1;
        return ret;
    }
    for (i=0; i<3; i++)
    {
        name = get_nicname(cells[i].type);
        cells[i].device_name = name;
        switch(cells[i].type)
        {
            case ETH:
                cells[i].start = eth_start;
                cells[i].stop = netbase_stop;
                break;
            case WIFI:
                cells[i].start = wifi_start;
                cells[i].stop = netbase_stop;
                break;
            case GPRS:
                cells[i].start = gprs_start;
                cells[i].stop = netbase_stop;
                break;                    
        }
    }
    return ret;
}
void *thread_auto_connet(void *param)
{
    int i =0;
    (void) param;
    int count = 0;

    init_netcells(netcells);
    //start net
    for (i=0; i<sizeof netcells/ sizeof netcells[0]; i++)
    {
        if (net_manage.activate)
        {
            if (netcells[i].priority > net_manage.activate->priority)
            {
                continue;
            }
        }
        if (netcells[i].start(&netcells[i]) < 0)
        {
            continue;
        }
        if (is_overnet(&netcells[i]))
        {
            if (net_manage.activate)
            {
                net_manage.activate->stop();
            }
            update_netmanage(&netcells[i]);
        }
    }
    while (1)
    {
        
        /*-----------------------------------------------------------------------------
         *  娴嬭瘯褰撳墠缃戠粶鏄惁鍙互涓婄綉
         *-----------------------------------------------------------------------------*/
        if (!is_overnet(net_manage.activate))
        {
            count++;
        }
        if (count > 3)
        {
            for (i=0; i<3; i++)
            {
                if (netcells[i].start(&netcells[i]) < 0)
                {
                    continue;
                }
                if (is_overnet(&netcells[i]))
                {
                    if (net_manage.activate)
                    {
                        net_manage.activate->stop();
                    }
                    update_netmanage(&netcells[i]);
                }
            }
            if (net_manage.state != CONNECTED)
            {
                printf("can not on the internet!!! please check!!!!!!!\n");
            }
            count = 0;
        }
        sleep(1);
    }

    return NULL;
}


