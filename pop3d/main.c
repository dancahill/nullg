/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#define MAIN_GLOBALS 1
#include "pop3_main.h"

#ifdef WIN32

#define OS_WIN9X 1
#define OS_WINNT 2
#define OS_WIN2K 3

short isServiceInstalled(void);
short installService(void);
short uninstallService(void);

static SERVICE_STATUS_HANDLE ghCtrlStatus;
static SERVICE_STATUS        gStatus;
static HANDLE                ghevDoForever=NULL;
static DWORD                 g_dwOSVersion;

BOOL os_version(LPDWORD dwVersion)
{
	OSVERSIONINFO osvi;

	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osvi)) return FALSE;
	switch (osvi.dwPlatformId) {
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion<=4) {
			*dwVersion=OS_WINNT;
		} else if (osvi.dwMajorVersion>=5) {
			*dwVersion=OS_WIN2K;
		} else {
			return FALSE;
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		*dwVersion=OS_WIN9X;
		break;
	case VER_PLATFORM_WIN32s:
	default:
		*dwVersion=0;
		return FALSE;
	}
	return TRUE; 
}

void WINAPI XControlHandler(DWORD dwControl)
{
	DWORD rc;

	switch (dwControl) {
	case SERVICE_CONTROL_PAUSE:
	case SERVICE_CONTROL_CONTINUE:
	case SERVICE_CONTROL_INTERROGATE:
		gStatus.dwWaitHint=0;
		break;
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		gStatus.dwCurrentState=SERVICE_STOP_PENDING;
		gStatus.dwWaitHint=5000;
		server_shutdown();
		break;
	}
	rc=SetServiceStatus(ghCtrlStatus, &gStatus);
	if (!rc) {
		printf("Error in XControlHandler-SetServiceStatus!\r\n");
		DebugBreak();
	}
	if ((dwControl==SERVICE_CONTROL_STOP)||(dwControl==SERVICE_CONTROL_SHUTDOWN)) {
		SetEvent(ghevDoForever);
	}
}

void WINAPI ServiceMain(DWORD dwNumServiceArgs, LPTSTR *lpServiceArgs)
{
	pthread_attr_t thr_attr;
	pthread_t trc;
	DWORD rc;

	ghCtrlStatus=RegisterServiceCtrlHandler("nullgw-pop3d", (LPHANDLER_FUNCTION)XControlHandler);
	if (ghCtrlStatus==(SERVICE_STATUS_HANDLE)NULL) {
		printf("Error in nullgw-pop3d_Main-RegisterServiceCtrlHandler!\r\n");
		DebugBreak();
	}
	ghevDoForever=CreateEvent(NULL, FALSE, FALSE, "Nullnullgw-pop3dRunEvt");
	gStatus.dwServiceType              = SERVICE_WIN32;
	gStatus.dwCurrentState             = SERVICE_RUNNING;
	gStatus.dwControlsAccepted         = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	gStatus.dwWin32ExitCode            = NO_ERROR;
	gStatus.dwServiceSpecificExitCode  = NO_ERROR;
	gStatus.dwCheckPoint               = 0;
	gStatus.dwWaitHint                 = 0;
	rc=SetServiceStatus(ghCtrlStatus, &gStatus);
	if (!rc) {
		printf("Error in nullgw-pop3d_Main-SetServiceStatus!\r\n");
		DebugBreak();
	}
	/* LET THE GROUPWARE BEGIN */
	init();
	if (pthread_attr_init(&thr_attr)) exit(1);
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
	if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
		logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
		exit(0);
	}
	if (pthread_attr_init(&thr_attr)) exit(1);
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
	if (pthread_create(&trc, &thr_attr, conn_reaper, NULL)==-1) {
		logerror(NULL, __FILE__, __LINE__, "conn_reaper() thread failed to start.");
		exit(0);
	}
	proc.DaemonThread=trc;
	/* NOW BACK TO THE SERVICE CRAP */
	WaitForSingleObject(ghevDoForever, INFINITE);
	gStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(ghCtrlStatus, &gStatus);
	return;
}

int main(int argc, char *argv[])
{
	pthread_attr_t thr_attr;
	SERVICE_TABLE_ENTRY SrvTable[2];
	char *ptemp;

	setvbuf(stdout, NULL, _IONBF, 0);
	proc.RunAsCGI=0;
	conn=NULL;
	memset((char *)&proc, 0, sizeof(proc));
	proc.stats.starttime=time(NULL);
	snprintf(proc.program_name, sizeof(proc.program_name)-1, "%s", GetCommandLine());
	ptemp=proc.program_name;
	if (p_strcasestr(ptemp, ".exe")!=NULL) {
		ptemp=p_strcasestr(ptemp, ".exe");
	}
	while ((ptemp)&&(*ptemp!=' ')) ptemp++;
	while (*ptemp==' ') ptemp++;
	if (!os_version(&g_dwOSVersion)) return 0;
	if ((g_dwOSVersion!=OS_WINNT)&&(g_dwOSVersion!=OS_WIN2K)) {
		if (!strcmp(ptemp, "install")) {
			printf("Services not supported on Win9x.\r\n");
			exit(0);
		} else if (!strcmp(ptemp, "remove")) {
			printf("Services not supported on Win9x.\r\n");
			exit(0);
		}
		init();
		if (pthread_attr_init(&thr_attr)) exit(1);
		if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
		if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
			logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
			exit(0);
		}
		proc.DaemonThread=(pthread_t)pthread_self();
		conn_reaper(NULL);
		logerror(NULL, __FILE__, __LINE__, "conn_reaper() loop failed to start.");
		exit(0);
	}
	memset(&gStatus, 0x00, sizeof(gStatus));
	ghevDoForever=NULL;
	if ((ghevDoForever=OpenEvent(SYNCHRONIZE, FALSE, "Nullnullgw-pop3dRunEvt"))!=0) {
		printf("nullgw-pop3d is already running.\r\n");
		CloseHandle(ghevDoForever);
		exit(-1);
	}
	if (isServiceInstalled()==1) {
		if (!strcmp(ptemp, "install")) {
			printf("nullgw-pop3d is already installed.\r\n");
			exit(0);
		} else if (!strcmp(ptemp, "remove")) {
			if (uninstallService()) {
				printf("nullgw-pop3d could not be uninstalled.\r\n");
				exit(-1);
			}
			exit(0);
		}
	} else {
		if (!strcmp(ptemp, "install")) {
			if (installService()==0) {
				printf("nullgw-pop3d was installed successfully.\r\n");
				exit(0);
			} else {
				printf("nullgw-pop3d could not be installed.\r\n");
				exit(-1);
			}
			exit(0);
		} else if (!strcmp(ptemp, "remove")) {
			printf("nullgw-pop3d is not installed.\r\n");
			exit(-1);
		}
	}
	SrvTable[0].lpServiceName = "nullgw-pop3d";
	SrvTable[0].lpServiceProc = ServiceMain;
	SrvTable[1].lpServiceName = NULL;
	SrvTable[1].lpServiceProc = NULL;
	if (!StartServiceCtrlDispatcher(SrvTable)) {
		printf("Service Start Error!\r\n");
		exit(-1);
	}
	return 0;
}
#else
/****************************************************************************
 *	main()
 *
 *	Purpose	: Program entry point (UNIX) and call accept loop
 *	Args	: Command line parameters (if any)
 *	Returns	: Exit status of program
 *	Notes	: None
 ***************************************************************************/
int main(int argc, char *argv[])
{
	pthread_attr_t thr_attr;

	setvbuf(stdout, NULL, _IONBF, 0);
	conn=NULL;
	memset((char *)&proc, 0, sizeof(proc));
	proc.stats.starttime=time(NULL);
	snprintf(proc.program_name, sizeof(proc.program_name)-1, "%s", argv[0]);
	umask(077);
	init();
	if (pthread_attr_init(&thr_attr)) exit(1);
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
		logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
		exit(0);
	}
	proc.DaemonThread=pthread_self();
	conn_reaper(NULL);
	return 0;
}
#endif
