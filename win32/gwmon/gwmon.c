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
#ifdef WIN32
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp

#define OS_WIN9X 1
#define OS_WINNT 2
#define OS_WIN2K 3

#define SVC_HTTPD 1
#define SVC_POP3D 2
#define SVC_SMTPD 3

#include <sys/timeb.h>
#define MYWM_NOTIFYICON (WM_APP+100)
static HWND hDLG;
static int iconstatus=0;

static HINSTANCE hInst;

DWORD g_dwOSVersion;
UINT  g_bUiTaskbarCreated;

int winsystem(WORD show_hide, const char *format, ...)
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
	si.wShowWindow=show_hide;
	si.hStdInput=NULL;
	si.hStdOutput=NULL;
	si.hStdError=NULL;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		return -1;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}

short installService(short int service)
{
	SC_HANDLE scHndl;
	SC_HANDLE scServ;
	char cCurDir[256];
	char *svcname=NULL;
	char *svctitle=NULL;

	memset(cCurDir, 0, sizeof(cCurDir));
	GetCurrentDirectory(256, cCurDir);
	switch (service) {
	case SVC_HTTPD:
		strcat(cCurDir, cCurDir[strlen(cCurDir)-1]==92?"nullgw-httpd.exe":"\\nullgw-httpd.exe");
		svcname="nullgw-httpd";
		svctitle="NullLogic Groupware HTTPd";
		break;
	case SVC_POP3D:
		strcat(cCurDir, cCurDir[strlen(cCurDir)-1]==92?"nullgw-pop3d.exe":"\\nullgw-pop3d.exe");
		svcname="nullgw-pop3d";
		svctitle="NullLogic Groupware POP3d";
		break;
	case SVC_SMTPD:
		strcat(cCurDir, cCurDir[strlen(cCurDir)-1]==92?"nullgw-smtpd.exe":"\\nullgw-smtpd.exe");
		svcname="nullgw-smtpd";
		svctitle="NullLogic Groupware SMTPd";
		break;
	}
	if (svcname==NULL) return 1;
	scHndl=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scHndl==NULL) return 1;
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
		CloseHandle(scHndl);
		return 1;
	}
	CloseHandle(scServ);
	CloseHandle(scHndl);
	return 0;
}

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
	snprintf(tnd.szTip, sizeof(tnd.szTip)-1, "NullLogic Groupware Monitor");
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
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	SERVICE_STATUS schSStatus;

	hMenu=CreatePopupMenu(); 
	if (!hMenu) return (FALSE);
	bRet=AppendMenu(hMenu, MF_STRING|MFS_DEFAULT, MYWM_NOTIFYICON+10+2, "Start Groupware Client");
	bRet=AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
		if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
		schService=OpenService(schSCManager, "nullgw-httpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
		if (schService==NULL) {
			AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+4, "Install HTTP Server");
		} else {
			QueryServiceStatus(schService, &schSStatus);
			if (schSStatus.dwCurrentState!=SERVICE_RUNNING) {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+3, "Start HTTP Server");
			} else {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+5, "Stop HTTP Server");
			}
		}
		if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
		schService=OpenService(schSCManager, "nullgw-pop3d", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
		if (schService==NULL) {
//			AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+7, "Install POP3 Server");
		} else {
			QueryServiceStatus(schService, &schSStatus);
			if (schSStatus.dwCurrentState!=SERVICE_RUNNING) {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+6, "Start POP3 Server");
			} else {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+8, "Stop POP3 Server");
			}
		}
		if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
		schService=OpenService(schSCManager, "nullgw-smtpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
		if (schService==NULL) {
//			AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+10, "Install SMTP Server");
		} else {
			QueryServiceStatus(schService, &schSStatus);
			if (schSStatus.dwCurrentState!=SERVICE_RUNNING) {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+9, "Start SMTP Server");
			} else {
				AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+11, "Stop SMTP Server");
			}
		}
		bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+12, "Services");
	} else {
		AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+3, "Start HTTP Server");
		AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+6, "Start POP3 Server");
		AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+9, "Start SMTP Server");
	}
	bRet=AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+1, "Configuration");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+0, "Help");
	bRet=AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	bRet=AppendMenu(hMenu, MF_STRING, MYWM_NOTIFYICON+10+13, "Exit");
	if (!bRet) {
		DestroyMenu(hMenu);
		return (FALSE);
	}
	SetForegroundWindow(hDLG);
	TrackPopupMenu(hMenu, TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, 0, hDLG, NULL);
	PostMessage(hDLG, WM_USER, 0, 0);
	return (FALSE);
}

BOOL CALLBACK NullDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char commandline[128];
	UINT nNewMode;
	POINT point;
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	SERVICE_STATUS schSStatus;

	hDLG=hDlg;
	if (uMsg==g_bUiTaskbarCreated) {
		TrayMessage(NIM_ADD);
		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}
	switch (uMsg) {
	case WM_INITDIALOG:
		TrayMessage(NIM_ADD);
		break;
	case WM_COMMAND:
		nNewMode=GET_WM_COMMAND_ID(wParam, lParam)-(MYWM_NOTIFYICON+10);
		switch (nNewMode) {
		case 0:
			snprintf(commandline, sizeof(commandline)-1, "../var/htdocs/groupware/help/en/index.html");
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case 1:
			winsystem(SW_SHOW, ".\\config.exe");
			break;
		case 2:
			snprintf(commandline, sizeof(commandline)-1, "http://localhost:4110/");
			ShellExecute(NULL, "open", commandline, NULL, NULL, SW_SHOWMAXIMIZED);
			break;
		case 3:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-httpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				StartService(schService, 0, NULL);
			} else {
				winsystem(SW_HIDE, ".\\nullgw-httpd.exe");
			}
			break;
		case 4:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-httpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				installService(SVC_HTTPD);
			}
			break;
		case 5:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-httpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				ControlService(schService, SERVICE_CONTROL_STOP, &schSStatus);
			}
			break;
		case 6:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-pop3d", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				StartService(schService, 0, NULL);
			} else {
				winsystem(SW_HIDE, ".\\nullgw-pop3d.exe");
			}
			break;
		case 7:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-pop3d", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				installService(SVC_POP3D);
			}
			break;
		case 8:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-pop3d", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				ControlService(schService, SERVICE_CONTROL_STOP, &schSStatus);
			}
			break;
		case 9:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-smtpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				StartService(schService, 0, NULL);
			} else {
				winsystem(SW_HIDE, ".\\nullgw-smtpd.exe");
			}
			break;
		case 10:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-smtpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				installService(SVC_SMTPD);
			}
			break;
		case 11:
			if ((g_dwOSVersion==OS_WINNT)||(g_dwOSVersion==OS_WIN2K)) {
				if (!(schSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))) return FALSE;
				schService=OpenService(schSCManager, "nullgw-smtpd", SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|SERVICE_USER_DEFINED_CONTROL);
				ControlService(schService, SERVICE_CONTROL_STOP, &schSStatus);
			}
			break;
		case 12:
			if (g_dwOSVersion>=OS_WIN2K) {
				ShellExecute(NULL, "open", "services.msc", "/s", NULL, SW_NORMAL);
			} else {
				winsystem(SW_SHOW, "Control.exe SrvMgr.cpl Services");
			}
			break;
		case 13:
			PostMessage(hDLG, WM_CLOSE, 0, 0);
			break;
		}
	case MYWM_NOTIFYICON:
		switch (lParam) {
		case WM_LBUTTONDOWN:
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
		TrayMessage(NIM_DELETE);
		EndDialog(hDLG, TRUE);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HANDLE hMutex;

	if (!os_version(&g_dwOSVersion)) return 0;
	hMutex=CreateMutex(NULL, FALSE, "NGMON_MUTEX");
	if ((hMutex==NULL)||(GetLastError()==ERROR_ALREADY_EXISTS)) {
		if (hMutex) CloseHandle(hMutex);
		return 0;
	}
	g_bUiTaskbarCreated=RegisterWindowMessage("TaskbarCreated");
	hInst=hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_NULLDIALOG), NULL, NullDlgProc);
	CloseHandle(hMutex);
	return 0;
}
#endif
