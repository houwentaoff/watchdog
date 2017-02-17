/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, xxx.
 *       Filename:  watchdog.c
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  01/04/2017 11:53:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), 544088192@qq.com
 *   Organization:  xxx
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "debug.h"
#include "auto_conn.h"

#define  EnableCoreDumps()\
{\
        struct rlimit   limit;\
        limit.rlim_cur = RLIM_INFINITY;\
        limit.rlim_max = RLIM_INFINITY;\
        setrlimit(RLIMIT_CORE, &limit);\
}

#define BUFSIZE	(128)
#define PARAM_NR		(10)
#define PARAM_LENGTH	(256)
#define DEFAULT_PATH	"/mnt/data"

typedef struct proc_s {
	int	enable;
	int	pid;
	const char *name;
	const char *path;
} proc_t;

typedef enum procst
{
    RUNNING,
    ZOMBIE,    
    EXITED,
    UNDEFINED,
}procst_e;

enum proc_pid 
{
	LORAWAN_SDK = 0,
	LORAWAN_GATEWAY,
	TOTAL_PROC_NR,
};

typedef struct proc_param_s 
{
	char param[PARAM_LENGTH];
} proc_param_t;

static proc_t child_proc[] = 
{
	{0, -1, LORAWAN_GATEWAY_PROC, NULL},
	{0, -1, LORAWAN_SDK_PROC, NULL},
};
static proc_param_t proc_params[TOTAL_PROC_NR];
static int my_exec(proc_t *proc, char *param_list)
{
	if (proc == NULL)
		return -1;

	char filename[BUFSIZE];
	char * param[PARAM_NR];
	char * separator, *substr;
	int fd, i = 0, retv = 0;;

	sprintf(filename, "./%s", proc->name);
	if ((fd = open(filename, O_RDONLY, 0)) < 0) {
		sprintf(filename, "%s/%s", DEFAULT_PATH, proc->name);
		if ((fd = open(filename, O_RDONLY, 0)) < 0) {
			sprintf(filename, "%s/%s", proc->path, proc->name);
			if ((fd = open(filename, O_RDONLY, 0)) < 0) {
				wt_err("Can't open executable file [%s]\n", proc->name);
				return -1;
			}
		}
	}
	close(fd);

	memset(param, 0, sizeof(param));
	separator = strchr(param_list, ' ');
	substr = param_list;
	param[i] = (char *)malloc(strlen(filename) + 1);
	strncpy(param[i], filename, strlen(filename));
	param[i++][strlen(filename) + 1] = 0;
	while (separator != NULL) {
		param[i] = (char *)malloc(separator - substr + 1);
		strncpy(param[i], substr, separator - substr);
		param[i][separator - substr] = 0;
		substr = separator + 1;
		separator = strchr(substr, ' ');
		++i;
	}
	retv = execv(filename, param);

	for (; i > 0; --i) {
		wt_dbg("\n!!!!!! Free string : %s\n", param[i]);
		free(param[i]);
	}

	return retv;
}

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
static int start_app(proc_t *child_proc)
{
	int pgid;

		if (child_proc->enable) 
        {
			child_proc->pid = fork();
			if (child_proc->pid < 0) 
            {
				perror("fork");
				return -1;
			} 
            else
            {
				// set all processes into one process group
				pgid = getpgrp();
				setpgid(child_proc->pid, pgid);

				if (child_proc->pid == 0) 
                {
					if (my_exec(child_proc, proc_params[0].param) < 0) 
                    {
						wt_err("Can't start child process [%s]\n", child_proc->name);
					}
                    else
                    {
						wt_err("==== EXIT ==== my process [%s]\n", child_proc->name);
					}
					return 0;
				} 
                else
                {
					wt_err("==== Create child process [%s], pid [%d] ====\n",
						child_proc->name, child_proc->pid);
					sleep(1);
				}
			}
		}

	return 0;
}

static void signal_stop(int signo)
{
	// send the "kill" signal to all processes in the same process group
//	kill(0, signo);
	delete_pid_file(LORAWAN_WATCHDOG_PROC);
	wt_dbg("Exit watchdog and stop its children processes!\n");
	exit(1);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    int i=0;
    pthread_t net_tid;

    EnableCoreDumps();

	mkdir("../cores",0775);
	system("busybox sysctl -w kernel.core_pattern=./cores/core.%e-%p-%t");

	signal(SIGINT, signal_stop);
	signal(SIGQUIT, signal_stop);
	signal(SIGTERM, signal_stop);
    if (create_pid_file(LORAWAN_WATCHDOG_PROC) < 0) 
    {
        wt_err("create_pid_file failed!\n");
        return -1;
    }
    pthread_create (&net_tid, NULL, thread_auto_connet, NULL);
    while (1)
    {
        /*-----------------------------------------------------------------------------
         *  轮训需要的program list
         *-----------------------------------------------------------------------------*/
        for (i=0; i<((sizeof child_proc)/(sizeof child_proc[0])); i++)
        {
            if (RUNNING != get_procstatus(child_proc[i].name))
            {
                if (start_app(&child_proc[i]) < 0)
                {
                    wt_err("start %s in %s err!\n", child_proc[i].name, child_proc[i].path);
                }
            }
        }
        sleep(5);
    }
    return EXIT_SUCCESS;
}
