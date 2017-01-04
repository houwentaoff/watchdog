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

static bool EnableCoreDumps(void)
{
	bool bRet = true;
	struct rlimit rlim_new,rlim;
	if (getrlimit(RLIMIT_CORE, &rlim) == 0) 
	{
		rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
		if (setrlimit(RLIMIT_CORE, &rlim_new)!= 0) 
		{
			/* failed. try raising just to the old max */
			rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
			(void)setrlimit(RLIMIT_CORE, &rlim_new);
			bRet = false;
		}
	}

	return bRet;
}
PROCSTATUS GetProcessStatus(const char *name)
{
	string psCmd;
	if (m_nfiletype == 0)
	{
		psCmd = "ps -ef | grep \""+ name +"\" | grep -v grep | grep -v gdb | grep -v log";
	}
	else if (m_nfiletype == 1)
	{
		psCmd = "ps -ef | grep \""+ m_strKey +"\" | grep java | grep -v vi | grep -v grep | grep -v gdb";
	}
	else
	{
		assert(false);
	}

	char buf[512]={0};
	FILE* fp =NULL;

	if(NULL != (fp =popen(psCmd.c_str(), "r")))
	{
		while(NULL != fgets(buf, 512, fp) ) 
		{
		}
		if(string(buf) == string(""))
		{
			pclose(fp);
			//AddBootLog("fiioctlproxy is exited!\n");
			return EXITED;
		}
	}
	else
	{
		cerr<<"popen error["<<errno<<"]"<<endl;
		//ostringstream s;
		//s << "popen error["<<errno<<"]\n";
		//AddBootLog(s.str());
		return UNDEFINED;
	}

	pclose(fp);
	//AddBootLog("fiioctlproxy is running!\n");
	return RUNNING;
}

static void signal_stop(int signo)
{
	// send the "kill" signal to all processes in the same process group
//	kill(0, signo);
	delete_pid_file(MEDIA_SERVER_PROC);
	APP_PRINTF("Exit mediaserver and stop its children processes!\n");
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
  
	signal(SIGINT, signal_stop);
	signal(SIGQUIT, signal_stop);
	signal(SIGTERM, signal_stop);

	mkdir("../cores",0775);
	system("sysctl -w kernel.core_pattern=../cores/core.%e-%p-%t");

    while (1)
    {
        
        /*-----------------------------------------------------------------------------
         *  轮训需要的program list
         *-----------------------------------------------------------------------------*/
        for ()
        sleep(5);
    }
    return EXIT_SUCCESS;
}
