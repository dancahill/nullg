/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"
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

BOOL TrayMessage(DWORD dwMessage)
{
        BOOL res;
	HICON hIcon;
	NOTIFYICONDATA tnd;

	if (iconstatus) {
                hIcon=LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
	} else {
                hIcon=LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	}
	tnd.cbSize		= sizeof(NOTIFYICONDATA);
	tnd.hWnd		= hDLG;
	tnd.uID			= 0;
	tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon		= hIcon;
	snprintf(tnd.szTip, sizeof(tnd.szTip)-1, "NullLogic Groupware Server");
	res=Shell_NotifyIcon(dwMessage, &tnd);
	if (hIcon) {
		DestroyIcon(hIcon);
	}
	return res;
}

void TrayIcon(int newstatus)
{
	if (iconstatus!=newstatus) {
		iconstatus=newstatus;
		TrayMessage(NIM_MODIFY);
	}
	return;
}

BOOL APIENTRY HandlePopupMenu(POINT point)
{
	HMENU hMenu;
	BOOL bRet;
    
	hMenu=CreatePopupMenu(); 
	if (!hMenu) return (FALSE);
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+0, "Help");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+1, "Configuration");
	bRet=AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+2, "Start Client");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+3, "Restart Server");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+4, "Shutdown Server");
	if (!bRet) {
		DestroyMenu(hMenu);
		return (FALSE);
	}
	SetForegroundWindow(hDLG);
	TrackPopupMenu(hMenu, TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, 0, hDLG, NULL);
	PostMessage(hDLG, WM_USER, 0, 0);
	return (FALSE);
}

/****************************************************************************
 *	NullDlgProc()
 *
 *	Purpose	: Message loop to handle tray icon and form messages
 *	Args	: Nothing of consequence
 *	Returns	: Nothing of consequence
 *	Notes	: None
 ***************************************************************************/
BOOL CALLBACK NullDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char commandline[128];
	UINT nNewMode;
	POINT point;

	hDLG=hDlg;
	switch (uMsg) {
	case WM_INITDIALOG:
		TrayMessage(NIM_ADD);
		break;
	case WM_COMMAND:
		nNewMode=GET_WM_COMMAND_ID(wParam, lParam)-(MYWM_NOTIFYICON+10);
		switch (nNewMode) {
		case 0:
			if ((strcasecmp("ANY", config.server_hostname)==0)||(strcasecmp("INADDR_ANY", config.server_hostname)==0)) {
				snprintf(commandline, sizeof(commandline)-1, "http://localhost:%d/groupware/help/index.html", config.server_port);
			} else {
				snprintf(commandline, sizeof(commandline)-1, "http://%s:%d/groupware/help/index.html", config.server_hostname, config.server_port);
			}
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case 1:
			winsystem(".\\config.exe");
			break;
		case 2:
			if ((strcasecmp("ANY", config.server_hostname)==0)||(strcasecmp("INADDR_ANY", config.server_hostname)==0)) {
				snprintf(commandline, sizeof(commandline)-1, "http://localhost:%d/", config.server_port);
			} else {
				snprintf(commandline, sizeof(commandline)-1, "http://%s:%d/", config.server_hostname, config.server_port);
			}
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case 3:
			server_restart();
			break;
		case 4:
			PostMessage(hDLG, WM_CLOSE, 0, 0);
			break;
		}
	case MYWM_NOTIFYICON:
		switch (lParam) {
		case WM_LBUTTONDOWN:
			if ((strcasecmp("ANY", config.server_hostname)==0)||(strcasecmp("INADDR_ANY", config.server_hostname)==0)) {
				snprintf(commandline, sizeof(commandline)-1, "http://localhost:%d/", config.server_port);
			} else {
				snprintf(commandline, sizeof(commandline)-1, "http://%s:%d/", config.server_hostname, config.server_port);
			}
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case WM_RBUTTONDOWN:
			GetCursorPos(&point);
			HandlePopupMenu(point);
			break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		sqlDisconnect(NULL);
		TrayMessage(NIM_DELETE);
		EndDialog(hDLG, TRUE);
		server_shutdown();
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
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
