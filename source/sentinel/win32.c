/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "version.h"
#ifdef WIN32
#include <sys/timeb.h>
#define MYWM_NOTIFYICON (WM_APP+100)
static HWND hDLG;
static int iconstatus=0;

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
	snprintf(tnd.szTip, sizeof(tnd.szTip)-1, "Sentinel Groupware Server");
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
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+0, "Start Client");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+1, "Shutdown Server");
	if (!bRet) {
		DestroyMenu(hMenu);
		return(FALSE);
	}
	SetForegroundWindow(hDLG);
	TrackPopupMenu(hMenu, TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, 0, hDLG, NULL);
	PostMessage(hDLG, WM_USER, 0, 0);
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
			snprintf(commandline, sizeof(commandline)-1, "http://%s:%d/", config.server_hostname, config.server_port);
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case 1:
			PostMessage(hDLG, WM_CLOSE, 0, 0);
			break;
		}
	case MYWM_NOTIFYICON:
		switch (lParam) {
		case WM_LBUTTONDOWN:
			snprintf(commandline, sizeof(commandline)-1, "http://%s:%d/", config.server_hostname, config.server_port);
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
		logaccess(0, "Stopping %s (Build %d)", SERVER_NAME, BUILD);
		sqlDisconnect();
		TrayMessage(NIM_DELETE);
		EndDialog(hDLG, TRUE);
		exit(0);
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
	dp.d_namlen = dp.d_reclen = strlen (strcpy (dp.d_name, dirp->dd_cp->_d_entry));
#if 0 /* JB */
	strlwr (dp.d_name);		/* JF */
#endif
	dp.d_ino = 0;
	dirp->dd_cp = dirp->dd_cp->_d_next;
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
	TIME_ZONE_INFORMATION TimeZoneInformation;
	struct timeb timebuffer;
   
	if ((tv==NULL)||(tz==NULL)) return -1;
	if (GetTimeZoneInformation(&TimeZoneInformation)==TIME_ZONE_ID_UNKNOWN) return -1;
//	tz->tz_minuteswest=TimeZoneInformation.Bias+TimeZoneInformation.DaylightBias;
	tz->tz_minuteswest=TimeZoneInformation.Bias;
	ftime(&timebuffer);
	tv->tv_sec=timebuffer.time;
	tv->tv_usec=0;
	return 0;
}

/*****************************************************************************
** The following is a simple implementation of posix conditions
*****************************************************************************/
/*
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	cond->waiting=0;
	cond->semaphore=CreateSemaphore(NULL,0,0x7FFFFFFF,(char*) 0);
	if (!cond->semaphore)
		return ENOMEM;
	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	return CloseHandle(cond->semaphore) ? 0 : EINVAL;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	InterlockedIncrement(&cond->waiting);
	LeaveCriticalSection(mutex);
	WaitForSingleObject(cond->semaphore,INFINITE);
	InterlockedDecrement(&cond->waiting);
	EnterCriticalSection(mutex);
	return 0 ;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	long prev_count;

	if (cond->waiting)
		ReleaseSemaphore(cond->semaphore,1, &prev_count);
	return 0;
}
*/
int pthread_attr_init(pthread_attr_t *connect_att)
{
	connect_att->dwStackSize	= 0;
	connect_att->dwCreatingFlag	= 0;
	connect_att->priority		= 0;
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

//int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, pthread_handler func, void *param)
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param)
{
	HANDLE hThread;
	unsigned long int id;

//	hThread=(HANDLE)_beginthread(func, attr->dwStackSize?attr->dwStackSize:65535, param);
//	hThread=(HANDLE)_beginthreadex(0, 65536L, func, param, CREATE_SUSPENDED, &id);
	hThread=(HANDLE)_beginthreadex(NULL, attr->dwStackSize?attr->dwStackSize:65535, func, param, 0, &id);
	if ((long)hThread==-1L) {
		return (errno?errno:-1);
	}
	*thread_id=hThread;
//	return (0);
	return id;
}

void pthread_exit(unsigned A)
{
	_endthread();
}
#endif
