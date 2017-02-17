/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, xxx.
 *       Filename:  utils.h
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  01/04/2017 11:56:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), 544088192@qq.com
 *   Organization:  xxx
 *
 * =====================================================================================
 */

#ifndef __APP_UTILS_H__
#define __APP_UTILS_H__


/************************* Macro definition ****************************/
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifndef DIV_ROUND
#define DIV_ROUND(divident, divider)    ( ( (divident)+((divider)>>1)) / (divider) )
#endif

#ifndef ROUND_UP
#define ROUND_UP(x, n)	( ((x)+(n)-1u) & ~((n)-1u) )
#endif

#ifndef ROUND_DOWN
#define ROUND_DOWN(x, n)	((x) & ~((n)-1u))
#endif

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define NO_ARG		0
#define HAS_ARG		1

#define LORAWAN_WATCHDOG_PROC   "watchdog"
#define LORAWAN_GATEWAY_PROC    "lora_pkt_fwd"
#define LORAWAN_SDK_PROC        "senzflow-lorabridge"


typedef struct hint_s {
	const char *arg;
	const char *str;
} hint_t;

static int create_pid_file(char *proc)
{
	#define STRING_LENGTH	(32)
	char buf[STRING_LENGTH], pid_file[STRING_LENGTH];
	int retv, fd;
	unsigned int old_pid, new_pid, remove_old_proc;

	new_pid = getpid();
	old_pid = remove_old_proc = 0;
	sprintf(pid_file, "/tmp/%s.pid", proc);
	if ((fd = open(pid_file, O_RDONLY, 0777)) >= 0) {
		read(fd, (void*)buf, STRING_LENGTH);
		if (strlen(buf) > 0) {
			old_pid = atoi(buf);
			if ((old_pid != 0) && (old_pid != new_pid)) {
				remove_old_proc = 1;
			}
		}
		if (remove_old_proc) {
			sprintf(buf, "ps x | grep %d | grep -v grep", old_pid);
			retv = system(buf);
			if (retv == 0) {
				sprintf(buf, "kill -9 %d", old_pid);
				system(buf);
			}
		}
		close(fd);
		printf("[%s] is already running ! Re-start it again !\n", proc);
	}

	if ((fd = open(pid_file, O_CREAT | O_RDWR, 0644)) < 0) {
		printf("CANNOT create [%s] pid file !\n", proc);
		return -1;
	}
	sprintf(buf, "%d\n", new_pid);
	if ((retv = write(fd, (void *)buf, strlen(buf))) < 0) {
		printf("write length %d.\n", retv);
		return -1;
	}
	close(fd);

	return 0;
}

static int delete_pid_file(char *proc)
{
	char buf[32];
	int fd;

	sprintf(buf, "/tmp/%s.pid", proc);
	if ((fd = open(buf, O_RDONLY, 0777)) < 0) {
		return -1;
	}
	close(fd);
	sprintf(buf, "rm -rf /tmp/%s.pid", proc);
	system(buf);

	return 0;
}


#endif	// __APP_UTILS_H__

