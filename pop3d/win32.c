/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "pop3_main.h"
#ifdef WIN32
#include <sys/timeb.h>
#define MYWM_NOTIFYICON (WM_APP+100)
static HWND hDLG;
static int iconstatus=0;

int winsystem(const char *format, ...)
{
	DWORD exitcode=0;
	HANDLE hMyProcess=GetCurrentProcess();
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char Command[512];
	va_list ap;
	int pid;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	memset(Command, 0, sizeof(Command));
	va_start(ap, format);
	vsnprintf(Command, sizeof(Command)-1, format, ap);
	va_end(ap);
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_SHOW;
	si.hStdInput=NULL;
	si.hStdOutput=NULL;
	si.hStdError=NULL;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		return -1;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
//	GetExitCodeProcess(pi.hProcess, &exitcode);
//	if (exitcode==STILL_ACTIVE) TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}

short installService(void)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	char cCurDir[256];

	memset(cCurDir, 0, sizeof(cCurDir));
	GetCurrentDirectory(256, cCurDir);
	strcat(cCurDir, cCurDir[strlen(cCurDir)-1]==92?"nullgw-pop3d.exe":"\\nullgw-pop3d.exe");
	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in installService-OpenSCManager!\r\n");
		return 1;
	}
	scServ=CreateService(scHndl,
		"nullgw-pop3d",
		"NullLogic Groupware POP3d",
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

	// UnInstall the XService from Service Control Manager Database
	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in uninstallService-OpenSCManager!\r\n");
		return 1;
	}
	scServ=OpenService(scHndl, "nullgw-pop3d", SERVICE_ALL_ACCESS);
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
	printf("nullgw-pop3d was uninstalled sucessfully.\r\n");
	return 0;
}

short isServiceInstalled(void)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	DWORD _err = 0;

	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) {
		printf("Error in isServiceInstalled-OpenSCManager!\r\n");
		exit(-1);
	}
	scServ=OpenService(scHndl, "nullgw-pop3d", SERVICE_ALL_ACCESS);
	_err=GetLastError();
	if (scServ==NULL) {
		if (_err==ERROR_SERVICE_DOES_NOT_EXIST) {
			CloseHandle(scServ);
			CloseHandle(scHndl);
			return 0;
		} else {
			printf("Error in isServiceInstalled-OpenService !\r\n");
			exit(-1);
		}
	}
	CloseHandle(scServ);
	CloseHandle(scHndl);
	return 1;
}

unsigned sleep(unsigned seconds)
{
	Sleep(1000*seconds);
	return 0;
}

/* Garbage collection */
void free_dircontents(struct _dircontents *dp)
{
	struct _dircontents *odp;

	while (dp) {
		if (dp->_d_entry) free(dp->_d_entry);
		dp=(odp=dp)->_d_next;
		free(odp);
	}
}

DIR* opendir(const char *name)
{
	struct _finddata_t find_buf;
	DIR *dirp;
	struct _dircontents *dp;
	char name_buf[_MAX_PATH + 1];
	char *slash = "";
	long hFile;

	if (!name) {
		name="";
	} else if (*name) {
		const char *s;
		int l = strlen (name);

		s=name+l-1;
		if ( !(l == 2 && *s == ':') && *s != '\\' && *s != '/')
			slash = "/";	/* save to insert slash between path and "*.*" */
	}
	strcat(strcat(strcpy(name_buf, name), slash), "*.*");
	dirp=(DIR *)malloc(sizeof(DIR));
	if (dirp==(DIR *)0)
		return (DIR *)0;
	dirp->dd_loc=0;
	dirp->dd_contents=dirp->dd_cp=(struct _dircontents *)0;
	if ((hFile = _findfirst (name_buf, &find_buf)) < 0) {
		free(dirp);
		return (DIR *)0;
	}
	do {
		dp=(struct _dircontents *) malloc (sizeof (struct _dircontents));
		if (dp == (struct _dircontents *)0) {
			free_dircontents (dirp->dd_contents);
			return (DIR *)0;
		}
		dp->_d_entry = malloc (strlen (find_buf.name) + 1);
		if (dp->_d_entry == (char *)0) {
			free (dp);
			free_dircontents (dirp->dd_contents);
			return (DIR *)0;
		}
		if (dirp->dd_contents)
			dirp->dd_cp = dirp->dd_cp->_d_next = dp;
		else
			dirp->dd_contents = dirp->dd_cp = dp;
		strcpy (dp->_d_entry, find_buf.name);
		dp->_d_next = (struct _dircontents *)0;
	} while (!_findnext (hFile, &find_buf));
	dirp->dd_cp = dirp->dd_contents;
	_findclose(hFile);
	return dirp;
}

void closedir(DIR *dirp)
{
	free_dircontents (dirp->dd_contents);
	free((char *) dirp);
}

int symlink(const char *oldpath, const char *newpath)
{
	return 1;
}
struct direct *readdir(DIR *dirp)
{
	static struct direct dp;

	if (dirp->dd_cp == (struct _dircontents *)0)
		return (struct direct *)0;
	dp.d_namlen=dp.d_reclen=strlen(strcpy(dp.d_name, dirp->dd_cp->_d_entry));
#if 0 /* JB */
	strlwr (dp.d_name);		/* JF */
#endif
	dp.d_ino=0;
	dirp->dd_cp=dirp->dd_cp->_d_next;
	dirp->dd_loc++;
	return &dp;
}

void seekdir(DIR *dirp, long off)
{
	long i = off;
	struct _dircontents *dp;

	if (off<0) return;
	for (dp = dirp->dd_contents; --i >= 0 && dp; dp = dp->_d_next);
	dirp->dd_loc = off - (i + 1);
	dirp->dd_cp = dp;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct timeb tb;
	struct tm *today;
   
	if ((tv==NULL)||(tz==NULL)) return -1;
	ftime(&tb);
	today=localtime(&tb.time);
	tv->tv_sec=tb.time;
	tv->tv_usec=tb.millitm*1000;
	tz->tz_minuteswest=tb.timezone-(today->tm_isdst*60);
	return 0;
}

int pthread_attr_init(pthread_attr_t *connect_att)
{
	connect_att->dwStackSize=0;
	connect_att->dwCreatingFlag=0;
	connect_att->priority=0;
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack)
{
	connect_att->dwStackSize=stack;
	return 0;
}

int pthread_attr_setprio(pthread_attr_t *connect_att, int priority)
{
	connect_att->priority=priority;
	return 0;
}

int pthread_attr_destroy(pthread_attr_t *connect_att)
{
	return 0;
}

int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)(void *), void *param)
{
	HANDLE hThread;
	unsigned long int id;

//	hThread=(HANDLE)_beginthread(func, attr->dwStackSize?attr->dwStackSize:65535, param);
//	hThread=(HANDLE)_beginthreadex(0, 65536L, func, param, CREATE_SUSPENDED, &id);
	hThread=(HANDLE)_beginthreadex(NULL, attr->dwStackSize?attr->dwStackSize:65535, func, param, 0, &id);
	if ((long)hThread==-1L) return (errno?errno:-1);
	*thread_id=hThread;
	return id;
}

int pthread_kill(pthread_t handle, int sig)
{
	int rc;

	TerminateThread(handle, (DWORD)&rc);
	CloseHandle(handle);
	return 0;
}

void pthread_exit(unsigned A)
{
	_endthread();
}
#endif
