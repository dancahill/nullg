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
#include "main.h"
#ifndef WIN32
#include <pwd.h>
#endif

#ifdef WIN32
BOOL os_version(LPDWORD dwVersion);
short isServiceInstalled(short int service);
short installService(short int service);
short uninstallService(short int service);
void WINAPI ServiceMain(DWORD dwNumServiceArgs, LPTSTR *lpServiceArgs);

int main(int argc, char *argv[])
{
	SERVICE_TABLE_ENTRY SrvTable[2];
	HANDLE ghevDoForever;
	DWORD g_dwOSVersion;
	unsigned short i;
	char *ptemp;

	setvbuf(stdout, NULL, _IONBF, 0);
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
	if (getenv("REQUEST_METHOD")!=NULL) return 0;
	if ((g_dwOSVersion!=OS_WINNT)&&(g_dwOSVersion!=OS_WIN2K)) {
		if (!strcmp(ptemp, "install")) {
			printf("Services not supported on Win9x.\r\n");
			exit(-2);
		} else if (!strcmp(ptemp, "remove")) {
			printf("Services not supported on Win9x.\r\n");
			exit(-2);
		}
		init();
		memset((char *)&proc.srvmod, 0, sizeof(SRVMOD));
		if (argc<2) {
			if (modules_init()!=0) exit(-2);
		} else {
			for (i=1;i<argc;i++) {
				module_load(argv[i]);
			}
		}
		sanity_checkdirs();
		if (modules_exec()!=0) exit(-2);
		if (modules_cron()!=0) exit(-2);
//		WaitForSingleObject(ghevDoForever, INFINITE);
//		log_error("core", __FILE__, __LINE__, 0, "conn_reaper() loop failed to start.");
//		exit(-2);
	}
//	memset(&gStatus, 0x00, sizeof(gStatus));
	ghevDoForever=NULL;
	if ((ghevDoForever=OpenEvent(SYNCHRONIZE, FALSE, "nullgroupwareRunEvent"))!=0) {
		printf("nullgroupware is already running.\r\n");
		CloseHandle(ghevDoForever);
		exit(-2);
	}
	if (isServiceInstalled(SVC_HTTPD)==1) {
		if (!strcmp(ptemp, "install")) {
			printf("nullgroupware is already installed.\r\n");
			exit(-2);
		} else if (!strcmp(ptemp, "remove")) {
			if (uninstallService(SVC_HTTPD)) {
				printf("nullgroupware could not be uninstalled.\r\n");
				exit(-2);
			}
			exit(0);
		}
	} else {
		if (!strcmp(ptemp, "install")) {
			if (installService(SVC_HTTPD)==0) {
				printf("nullgroupware was installed successfully.\r\n");
				exit(0);
			} else {
				printf("nullgroupware could not be installed.\r\n");
				exit(-2);
			}
			exit(0);
		} else if (!strcmp(ptemp, "remove")) {
			printf("nullgroupware is not installed.\r\n");
			exit(-2);
		}
	}
	SrvTable[0].lpServiceName = "nullgroupware";
	SrvTable[0].lpServiceProc = ServiceMain;
	SrvTable[1].lpServiceName = NULL;
	SrvTable[1].lpServiceProc = NULL;
	if (!StartServiceCtrlDispatcher(SrvTable)) {
		printf("Service Start Error!\r\n");
		exit(-2);
	}
	return 0;
}
#else
int main(int argc, char *argv[])
{
	struct passwd *pw;
	unsigned short i;

	setvbuf(stdout, NULL, _IONBF, 0);
	memset((char *)&proc, 0, sizeof(proc));
	proc.stats.starttime=time(NULL);
	snprintf(proc.program_name, sizeof(proc.program_name)-1, "%s", argv[0]);
	if (getenv("REQUEST_METHOD")!=NULL) return 0;
	umask(077);
	init();
	memset((char *)&proc.srvmod, 0, sizeof(SRVMOD));
	if (argc<2) {
		if (modules_init()!=0) exit(-2);
	} else {
		for (i=1;i<argc;i++) {
			module_load(argv[i]);
		}
	}
	if (getuid()==0) {
		if (!(pw=getpwnam(proc.config.server_username))) {
			printf("\r\nCannot find user '%s'.  Exiting.\r\n", proc.config.server_username);
			exit(-2);
		}
		memset(pw->pw_passwd, 0, strlen(pw->pw_passwd));
		endpwent();
//		chroot("/usr/local/nullgroupware");
//		chdir("/");
		if (setgid(pw->pw_gid)) exit(-2);
		if (setuid(pw->pw_uid)) exit(-2);
	}
	sanity_checkdirs();
	if (modules_exec()!=0) exit(-2);
//	if (modules_cron()!=0) exit(-2);
	proc.DaemonThread=pthread_self();
	cronloop(NULL);
	return 0;
}
#endif
