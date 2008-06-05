/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "main.h"
#ifndef WIN32
#include <pwd.h>
#endif

#ifdef WIN32
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

short installService(void)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	char cCurDir[256];
	char *svcname=SERVICE_NAME;
	char *svctitle=SERVER_NAME;

	memset(cCurDir, 0, sizeof(cCurDir));
	GetCurrentDirectory(256, cCurDir);
	strcat(cCurDir, cCurDir[strlen(cCurDir)-1]==92?"nullgsd.exe":"\\nullgsd.exe");
	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in installService-OpenSCManager!\r\n");
		return 1;
	}
	scServ=CreateService(scHndl,
		svcname,
		svctitle,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		cCurDir,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL );
	if (scServ==NULL) {
		printf("Error in installService-CreateService !\r\n");
		CloseHandle(scHndl);
		return 1;
	}
	CloseHandle(scServ);
	CloseHandle(scHndl);
	return 0;
}

short uninstallService(void)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	DWORD _err = 0;
	char *svcname=SERVICE_NAME;

	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in uninstallService-OpenSCManager!\r\n");
		return 1;
	}
	scServ=OpenService(scHndl, svcname, SERVICE_ALL_ACCESS);
	_err=GetLastError();
	if (scServ==NULL) {
		if (_err==ERROR_SERVICE_DOES_NOT_EXIST) {
			CloseHandle(scServ);
			CloseHandle(scHndl);
		}
		return 1;
	}
	if (!DeleteService(scServ)) {
		printf("Error in uninstallService-DeleteService !\r\n");
		return 1;
	}
	CloseHandle(scServ);
	CloseHandle(scHndl);
	printf("%s was uninstalled sucessfully.\r\n", svcname);
	return 0;
}

short isServiceInstalled(void)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	DWORD _err = 0;
	char *svcname=SERVICE_NAME;

	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in isServiceInstalled-OpenSCManager!\r\n");
		exit(-2);
	}
	scServ=OpenService(scHndl, svcname, SERVICE_ALL_ACCESS);
	_err=GetLastError();
	if (scServ==NULL) {
		if (_err==ERROR_SERVICE_DOES_NOT_EXIST) {
			CloseHandle(scServ);
			CloseHandle(scHndl);
			return 0;
		} else {
			printf("Error in isServiceInstalled-OpenService !\r\n");
			exit(-2);
		}
	}
	CloseHandle(scServ);
	CloseHandle(scHndl);
	return 1;
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
	DWORD rc;

	ghCtrlStatus=RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)XControlHandler);
	if (ghCtrlStatus==(SERVICE_STATUS_HANDLE)NULL) {
		printf("Error in RegisterServiceCtrlHandler!\r\n");
		DebugBreak();
	}
	ghevDoForever=CreateEvent(NULL, FALSE, FALSE, "nullgsRunEvent");
	memset(&gStatus, 0x00, sizeof(gStatus));
	gStatus.dwServiceType              = SERVICE_WIN32;
	gStatus.dwCurrentState             = SERVICE_RUNNING;
	gStatus.dwControlsAccepted         = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	gStatus.dwWin32ExitCode            = NO_ERROR;
	gStatus.dwServiceSpecificExitCode  = NO_ERROR;
	gStatus.dwCheckPoint               = 0;
	gStatus.dwWaitHint                 = 0;
	rc=SetServiceStatus(ghCtrlStatus, &gStatus);
	if (!rc) {
		printf("Error in SetServiceStatus!\r\n");
		DebugBreak();
	}
	/* LET THE GROUPWARE BEGIN */
	init(proc.N);
	memset((char *)&proc.srvmod, 0, sizeof(SRVMOD));
	if (modules_init(proc.N)!=0) exit(-2);
	sanity_checkdirs();
	if (modules_exec()!=0) exit(-2);
	if (startlisteners()!=0) exit(-2);
	if (modules_cron()!=0) exit(-2);
	/* NOW BACK TO THE SERVICE CRAP */
	WaitForSingleObject(ghevDoForever, INFINITE);
	gStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(ghCtrlStatus, &gStatus);
	return;
}

int main(int argc, char *argv[], char *envp[])
{
	char tmpbuf[MAX_OBJNAMELEN+1];
	SERVICE_TABLE_ENTRY SrvTable[2];
	HANDLE ghevDoForever;
	DWORD g_dwOSVersion;
	unsigned short i;
	unsigned short forcerun=0;
	obj_t *cobj, *tobj;
	char *ptemp, *p;

	setvbuf(stdout, NULL, _IONBF, 0);
	memset((char *)&proc, 0, sizeof(proc));
	if (getenv("REQUEST_METHOD")!=NULL) return 0;
	if ((proc.N=nes_newstate())==NULL) {
		printf("nes_newstate() error\r\n");
		return -1;
	}
	/* add args */
	tobj=nes_settable(proc.N, &proc.N->g, "_ARGS");
	tobj->val->attr|=NST_AUTOSORT;
	for (i=0;i<argc;i++) {
		sprintf(tmpbuf, "%d", i);
		nes_setstr(proc.N, tobj, tmpbuf, argv[i], strlen(argv[i]));
	}
	/* add env */
	tobj=nes_settable(proc.N, &proc.N->g, "_ENV");
	tobj->val->attr|=NST_AUTOSORT;
	for (i=0;envp[i]!=NULL;i++) {
		strncpy(tmpbuf, envp[i], MAX_OBJNAMELEN);
		p=strchr(tmpbuf, '=');
		if (!p) continue;
		*p='\0';
		p=strchr(envp[i], '=')+1;
		nes_setstr(proc.N, tobj, tmpbuf, p, strlen(p));
	}
	proc.stats.starttime=time(NULL);
	cobj=nes_setstr(proc.N, &proc.N->g, "program_name", GetCommandLine(), strlen(GetCommandLine()));
	ptemp=cobj->val->d.str;
	if (p_strcasestr(ptemp, ".exe")!=NULL) {
		ptemp=p_strcasestr(ptemp, ".exe");
	}
	while ((ptemp)&&(*ptemp!=' ')) ptemp++;
	while (*ptemp==' ') ptemp++;
	if (!os_version(&g_dwOSVersion)) return 0;
	if ((g_dwOSVersion!=OS_WINNT)&&(g_dwOSVersion!=OS_WIN2K)) {
		if (!strcmp(ptemp, "install")) {
			printf("Services not supported on Win9x.\r\n");
			exit(-2);
		} else if (!strcmp(ptemp, "remove")) {
			printf("Services not supported on Win9x.\r\n");
			exit(-2);
		}
		forcerun=1;
	}
	if (!strcmp(ptemp, "forcerun")) {
		printf("NullLogic GroupServer is bypassing service stuff.\r\n");
		forcerun=1;
	}
	if (forcerun) {
		init(proc.N);
		memset((char *)&proc.srvmod, 0, sizeof(SRVMOD));
		if (argc<3) {
			if (modules_init(proc.N)!=0) exit(-2);
		} else {
			for (i=2;i<argc;i++) {
				module_load(argv[i]);
			}
		}
		sanity_checkdirs();
		if (modules_exec()!=0) exit(-2);
		if (startlisteners()!=0) exit(-2);
		if (modules_cron()!=0) exit(-2);
		if ((ghevDoForever=OpenEvent(SYNCHRONIZE, FALSE, "nullgsRunEvent"))!=0) {
			printf("NullLogic GroupServer is already running.\r\n");
			CloseHandle(ghevDoForever);
			exit(-2);
		}
		ghevDoForever=CreateEvent(NULL, FALSE, FALSE, "nullgsRunEvent");
		WaitForSingleObject(ghevDoForever, INFINITE);
		exit(-2);
	}
/*	memset(&gStatus, 0x00, sizeof(gStatus)); */
	ghevDoForever=NULL;
	if ((ghevDoForever=OpenEvent(SYNCHRONIZE, FALSE, "nullgsRunEvent"))!=0) {
		printf("NullLogic GroupServer is already running.\r\n");
		CloseHandle(ghevDoForever);
		exit(-2);
	}
	if (isServiceInstalled()==1) {
		if (!strcmp(ptemp, "install")) {
			printf("NullLogic GroupServer is already installed.\r\n");
			exit(-2);
		} else if (!strcmp(ptemp, "remove")) {
			if (uninstallService()) {
				printf("NullLogic GroupServer could not be uninstalled.\r\n");
				exit(-2);
			}
			exit(0);
		}
	} else {
		if (!strcmp(ptemp, "install")) {
			if (installService()==0) {
				printf("NullLogic GroupServer was installed successfully.\r\n");
				exit(0);
			} else {
				printf("NullLogic GroupServer could not be installed.\r\n");
				exit(-2);
			}
			exit(0);
		} else if (!strcmp(ptemp, "remove")) {
			printf("NullLogic GroupServer is not installed.\r\n");
			exit(-2);
		}
	}
	SrvTable[0].lpServiceName = SERVICE_NAME;
	SrvTable[0].lpServiceProc = ServiceMain;
	SrvTable[1].lpServiceName = NULL;
	SrvTable[1].lpServiceProc = NULL;
	if (!StartServiceCtrlDispatcher(SrvTable)) {
		printf("Service Start Error!\r\n");
		exit(-2);
	}
	if (proc.N->err) {
		printf("errno=%d :: \"%s\"\r\n", proc.N->err, proc.N->errbuf);
	}
	proc.N=nes_endstate(proc.N);
	return 0;
}
#else
int main(int argc, char *argv[], char *envp[])
{
	char tmpbuf[MAX_OBJNAMELEN+1];
	obj_t *tobj;
	FILE *fp=NULL;
	struct passwd *pw;
	unsigned short i;
	char *p;

	setvbuf(stdout, NULL, _IONBF, 0);
	memset((char *)&proc, 0, sizeof(proc));
	if (getenv("REQUEST_METHOD")!=NULL) return 0;
	if ((proc.N=nes_newstate())==NULL) {
		printf("nes_newstate() error\r\n");
		return -1;
	}
	/* add args */
	tobj=nes_settable(proc.N, &proc.N->g, "_ARGS");
	tobj->val->attr|=NST_AUTOSORT;
	for (i=0;i<argc;i++) {
		sprintf(tmpbuf, "%d", i);
		nes_setstr(proc.N, tobj, tmpbuf, argv[i], strlen(argv[i]));
	}
	/* add env */
	tobj=nes_settable(proc.N, &proc.N->g, "_ENV");
	tobj->val->attr|=NST_AUTOSORT;
	for (i=0;envp[i]!=NULL;i++) {
		strncpy(tmpbuf, envp[i], MAX_OBJNAMELEN);
		p=strchr(tmpbuf, '=');
		if (!p) continue;
		*p='\0';
		p=strchr(envp[i], '=')+1;
		nes_setstr(proc.N, tobj, tmpbuf, p, strlen(p));
	}
	proc.stats.starttime=time(NULL);
	nes_setstr(proc.N, &proc.N->g, "program_name", argv[0], strlen(argv[0]));
	init(proc.N);
	memset((char *)&proc.srvmod, 0, sizeof(SRVMOD));
	if (argc<2) {
		if (modules_init(proc.N)!=0) exit(-2);
	} else {
		for (i=1;i<argc;i++) {
			module_load(argv[i]);
		}
	}
	tobj=nes_getobj(proc.N, &proc.N->g, "CONFIG");
	if (getuid()==0) {
		if (!(pw=getpwnam(nes_getstr(proc.N, tobj, "uid")))) {
			printf("\r\nCannot find user '%s'.  Exiting.\r\n", nes_getstr(proc.N, tobj, "uid"));
			exit(-2);
		}
		memset(pw->pw_passwd, 0, strlen(pw->pw_passwd));
		endpwent();
		if ((fp=fopen("/var/run/nullgsd.pid", "w"))!=NULL) {
			fprintf(fp, "%d\n", getpid());
			fclose(fp);
		}
		if (setgid(pw->pw_gid)) exit(-2);
		if (setuid(pw->pw_uid)) exit(-2);
	}
	sanity_checkdirs();
	if (modules_exec()!=0) exit(-2);
	if (startlisteners()!=0) exit(-2);
/*	if (modules_cron()!=0) exit(-2); */
	proc.DaemonThread=pthread_self();
	cronloop(NULL);
	if (proc.N->err) {
		printf("errno=%d :: \"%s\"\r\n", proc.N->err, proc.N->errbuf);
	}
	proc.N=nes_endstate(proc.N);
	return 0;
}
#endif
