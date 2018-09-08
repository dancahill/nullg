/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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
#include "nsp/nsplib.h"

// libbase->base.c
//int nspbase_register_all(nsp_state *N);
/*
#include "libbase.h"
#include "libcrypt.h"
#include "libdata.h"
#include "libnet.h"
#include "libodbc.h"
#include "libregex.h"
#include "libzip.h"
*/
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <windowsx.h>
#include <sys/stat.h>
#include "resource.h"
/*
#ifdef HAVE_DNS
#pragma comment(lib, "dnsapi.lib")
#endif
#ifdef HAVE_MBEDTLS
#pragma comment(lib, "mbedtls.lib")
#endif
*/

#define snprintf _snprintf
#define vsnprintf _vsnprintf

#pragma comment(lib, "winmm.lib")
#define CONFFILE "ntray.conf"

#define TITLE_PREFIX "NTray: "

typedef struct GLOBALS {
	HWND wnd;
	HINSTANCE instance;
	CRITICAL_SECTION GlobalLock;
	time_t lastpoll;
	time_t lastfile;
	int iconstatus;
	int index;
	char noticetext[240];
	UINT MenuHotKey;
} GLOBALS;

typedef struct TRAYNOTICESTRUCT {
	int x;
	int y;
	int w;
	int h;
	char *text;
	short int t;
} TRAYNOTICESTRUCT;

GLOBALS G;
nsp_state *N;

NSP_FUNCTION(nsp_textinput);
NSP_FUNCTION(nsp_passinput);
NSP_FUNCTION(nsp_traynotice);
NSP_FUNCTION(nsp_traynotice2);

void preppath(nsp_state *N, char *name)
{
	char buf[1024];
	char *p;
	unsigned int j;

	p = name;
	if ((name[0] == '/') || (name[0] == '\\') || (name[1] == ':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf));
	}
	else if (name[0] == '.') {
		/* looks relative... */
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	else {
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	for (j = 0;j < strlen(buf);j++) {
		if (buf[j] == '\\') buf[j] = '/';
	}
	for (j = strlen(buf) - 1;j > 0;j--) {
		if (buf[j] == '/') { buf[j] = '\0'; p = buf + j + 1; break; }
	}
	nsp_setstr(N, &N->g, "_filename", p, -1);
	nsp_setstr(N, &N->g, "_filepath", buf, -1);
	return;
}

static void log_error(nsp_state *N, const char *format, ...)
{
	char *file = "ntray.log";
	char logbuf[2048];
	char timebuf[100];
	va_list ap;
	int fd;
	struct timeval ttime;
	struct timezone tzone;
	time_t t;

	va_start(ap, format);
	vsnprintf(logbuf, sizeof(logbuf) - 1, format, ap);
	va_end(ap);
	nc_gettimeofday(&ttime, &tzone);
	t = ttime.tv_sec;
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S - ", localtime(&t));
	fd = open(file, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
	if (fd != -1) {
		write(fd, timebuf, strlen(timebuf));
		write(fd, logbuf, strlen(logbuf));
		write(fd, "\r\n", 2);
		close(fd);
	}
	return;
}

static int show_error(nsp_state *N)
{
	int errcode = N->err;

	if (errcode) {
		log_error(N, "show_error() [%s]", N->errbuf);
		N->errbuf[0] = '\0';
		N->err = 0;
	}
	return errcode;
}

void new_menu(nsp_state *N)
{
	char *newmenutext =
		"global MENUITEMS = {\r\n"\
		"\t{ name=\"NullLogic &Nesla\", type=\"ShellExecute\",  command=\"http://nulllogic.ca/nsp/\"                  };\r\n"\
		"\t{ name=\"separator\",        type=\"separator\"                                                          };\r\n"\
		"\t{ name=\"&Configuration\",   type=\"CreateProcess\", command=\"rundll32 shell32,OpenAs_RunDLL ntray.conf\" };\r\n"\
		"\t{ name=\"E&xit\",            type=\"Exit\",          command=\"Exit\"                                      };\r\n"\
		"};\r\n\r\n"\
		"class NTrayClass {\r\n"\
		"\tfunction onload() {\r\n\t\treturn;\r\n\t};\r\n"\
		"\tfunction onreload() {\r\n\t\treturn;\r\n\t};\r\n"\
		"\tfunction onclick() {\r\n\t\treturn;\r\n\t};\r\n"\
		"\tfunction onexit() {\r\n\t\treturn;\r\n\t};\r\n"\
		"\tfunction ontimer() {\r\n\t\treturn;\r\n\t};\r\n"\
		"}\r\n"
		;
	int textlen = strlen(newmenutext);
	int fd;
	struct stat sb;

	nsp_exec(N, newmenutext);
	if (stat(CONFFILE, &sb) == 0) return;
	if ((fd = open(CONFFILE, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1) return;
	write(fd, newmenutext, textlen);
	close(fd);
}

void do_preload(nsp_state *N)
{
	char sbuf[80];

	GetWindowsDirectory(sbuf, sizeof(sbuf));
	_snprintf(sbuf + strlen(sbuf), sizeof(sbuf) - strlen(sbuf) - 1, "\\NSP\\preload.ns");
	nsp_execfile(N, sbuf);
	return;
}

NSP_FUNCTION(nsp_flush)
{
#define __FN__ __FILE__ ":nsp_flush()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");

	if (N == NULL || N->outbuflen == 0) return 0;
	N->outbuffer[N->outbuflen] = '\0';
	log_error(N, "io.flush() outbuf=[%s]", N->outbuffer);
	N->outbuflen = 0;
	return 0;
#undef __FN__
}

NSP_FUNCTION(nsp_print)
{
	log_error(N, "print() [%s]", N->outbuffer);
	return 0;
}

void init_stuff(nsp_state *N)
{
	char tmpbuf[MAX_OBJNAMELEN + 1];
	obj_t *mobj;
	obj_t *tobj;
	int i;
	char *p;
	struct stat sb;

	EnterCriticalSection(&G.GlobalLock);
	N->debug = 0;
	//nspbase_register_all(N);
	/*
		nspcrypto_register_all(N);
		nspdata_register_all(N);
		nspnet_register_all(N);
		nspodbc_register_all(N);
		nspregex_register_all(N);
		nspzip_register_all(N);
	*/
	/* add env */
	tobj = nsp_settable(N, &N->g, "_ENV");
	for (i = 0;environ[i] != NULL;i++) {
		strncpy(tmpbuf, environ[i], MAX_OBJNAMELEN);
		p = strchr(tmpbuf, '=');
		if (!p) continue;
		*p = '\0';

		/* env vars should ignore case, so force uniformity */
		p = tmpbuf; while (*p) *p++ = toupper(*p);

		p = strchr(environ[i], '=') + 1;
		nsp_setstr(N, tobj, tmpbuf, p, -1);
	}
	preppath(N, CONFFILE);

	nsp_setcfunc(N, &N->g, "print", nsp_print);

	tobj = nsp_settable(N, &N->g, "io");
	nsp_setcfunc(N, tobj, "flush", (NSP_CFUNC)nsp_flush);

	nsp_setcfunc(N, &N->g, "TextInput", nsp_textinput);
	nsp_setcfunc(N, &N->g, "PassInput", nsp_passinput);
	nsp_setcfunc(N, &N->g, "TrayNotice", nsp_traynotice);
	nsp_setcfunc(N, &N->g, "TrayNotice2", nsp_traynotice2);
	if (stat(CONFFILE, &sb) == 0) {
		G.lastfile = sb.st_mtime;
		do_preload(N);
		nsp_execfile(N, CONFFILE);
		nsp_exec(N, "global NTray = new NTrayClass();");
		if (!show_error(N)) {
			mobj = nsp_getobj(N, &N->g, "MENUITEMS");
			if (!nsp_istable(mobj)) new_menu(N);
		}
	}
	else {
		G.lastfile = 0;
	}
	G.lastpoll = time(NULL);
	LeaveCriticalSection(&G.GlobalLock);
	return;
}

int winsystem(WORD show_hide, const char *format, ...)
{
	//	DWORD exitcode=0;
	//	HANDLE hMyProcess=GetCurrentProcess();
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char command[512];
	va_list ap;
	//	int pid;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&command, sizeof(command));
	va_start(ap, format);
	_vsnprintf(command, sizeof(command) - 1, format, ap);
	va_end(ap);
	si.cb = sizeof(si);
	//	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = show_hide;
	//	si.hStdInput=NULL;
	//	si.hStdOutput=NULL;
	//	si.hStdError=NULL;
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		MessageBox(NULL, command, TITLE_PREFIX "CreateProcess error", MB_ICONSTOP);
		return -1;
	}
	//	pid=pi.dwProcessId;
	//	CloseHandle(si.hStdInput);
	//	CloseHandle(si.hStdOutput);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}

obj_t *getindex(obj_t *tobj, int i)
{
	obj_t *cobj;

	if (!nsp_istable(tobj)) return NULL;
	for (tobj = tobj->val->d.table.f;tobj;tobj = tobj->next) {
		if (!nsp_istable(tobj)) continue;
		cobj = nsp_getobj(N, tobj, "index");
		if (!nsp_isnum(cobj)) {
			cobj = nsp_getobj(N, tobj, "table");
			if (!nsp_istable(cobj)) continue;
			if ((cobj = getindex(cobj, i)) == NULL) continue;
			return cobj;
		}
		if (nsp_tonum(N, cobj) == i) return tobj;
	}
	return NULL;
}

BOOL IconNotify(DWORD dwMessage, char *info, char *infotitle)
{
	obj_t *mobj = nsp_getobj(N, &N->g, "PROGNAME");
	BOOL res;
	HICON hIcon;
	NOTIFYICONDATA tnd;

	ZeroMemory(&tnd, sizeof(tnd));

	if (G.iconstatus == 1) {
		hIcon = LoadImage(G.instance, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
	}
	else if (G.iconstatus == 2) {
		hIcon = LoadImage(G.instance, MAKEINTRESOURCE(IDI_ICON3), IMAGE_ICON, 16, 16, 0);
	}
	else {
		hIcon = LoadImage(G.instance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	}
	tnd.cbSize = sizeof(tnd);
	tnd.hWnd = G.wnd;
	tnd.uID = 0;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = IDM_STATE;
	tnd.hIcon = hIcon;
	_snprintf(tnd.szTip, sizeof(tnd.szTip) - 1, "%s", nsp_isstr(mobj) ? nsp_tostr(N, mobj) : "Nesla SysTray Host");

	if (infotitle != NULL || info != NULL) tnd.uFlags |= NIF_INFO;
	if (infotitle != NULL) _snprintf(tnd.szInfoTitle, sizeof(tnd.szInfoTitle) - 1, "%s", infotitle);
	if (info != NULL) _snprintf(tnd.szInfo, sizeof(tnd.szInfo) - 1, "%s", info);

	res = Shell_NotifyIcon(dwMessage, &tnd);
	if (hIcon) DestroyIcon(hIcon);
	return res;
}

void IconStatus(int newstatus)
{
	if (G.iconstatus != newstatus) {
		G.iconstatus = newstatus;
		IconNotify(NIM_MODIFY, NULL, NULL);
	}
	return;
}

//unsigned _stdcall TrayTimer_NewConfig(void *xx)
void TrayTimer_NewConfig(void *xx)
{
	obj_t *cobj;
	struct stat sb;
	//	time_t t;

	EnterCriticalSection(&G.GlobalLock);
	if (stat(CONFFILE, &sb) == 0 && sb.st_mtime > G.lastfile) {
		G.lastfile = sb.st_mtime;
		/* sometimes it's easier to destroy the world and start over... */
		nsp_endstate(N);
		if ((N = nsp_newstate()) == NULL) {
			LeaveCriticalSection(&G.GlobalLock);
			_endthread();
			return;
		}
		init_stuff(N);
		show_error(N);
		cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "NTray"), "onreload");
		IconStatus(1);
		if (nsp_typeof(cobj) == NT_NFUNC) {
			nsp_exec(N, "NTray.onreload();");
			show_error(N);
		}
		IconStatus(0);
		//	} else {
		/*
				/ * no config? * /
				cobj=nsp_getobj(N, &N->g, "MENUITEMS");
				if (!nsp_istable(cobj)) {
					new_menu(N);
					MessageBox(NULL, "missing config", TITLE_PREFIX "Script Error", MB_ICONSTOP);
				}
		*/
	}
	LeaveCriticalSection(&G.GlobalLock);

	//	CloseHandle(conn->handle);

	//	{
	//		GetCurrentThreadId()
	//		CloseHandle(GetCurrentThreadId());
	_endthread();
	//	}
	return;
}

//unsigned _stdcall TrayTimer_OnTimer(void *xx)
void TrayTimer_OnTimer(void *xx)
{
	obj_t *cobj;
	//	struct stat sb;
	//	time_t t;

	EnterCriticalSection(&G.GlobalLock);
	//	t=time(NULL);
	//	if ((t%60)<(G.lastpoll%60)) {
	//		G.lastpoll=t;
	IconStatus(1);
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "NTray"), "ontimer");
	if (nsp_typeof(cobj) == NT_NFUNC) {
		N->warnings = 0;
		nsp_exec(N, "NTray.ontimer();");
		show_error(N);
	}
	IconStatus(0);
	//	} else {
	//		G.lastpoll=t;
	//	}
	LeaveCriticalSection(&G.GlobalLock);
	//	CloseHandle(GetCurrentThreadId());
	_endthread();
	return;
}

//unsigned _stdcall TrayClick(void *xx)
void TrayClick(void *xx)
{
	obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "NTray"), "onclick");

	EnterCriticalSection(&G.GlobalLock);
	IconStatus(1);
	if (nsp_typeof(cobj) == NT_NFUNC) {
		nsp_exec(N, "NTray.onclick();");
		show_error(N);
	}
	IconStatus(0);
	LeaveCriticalSection(&G.GlobalLock);
	_endthread();
	return;
}

void PopupMenuDrawSub(HMENU hMenu, obj_t *tobj)
{
	obj_t *cobj;
	char *p;
	HMENU hMenuSub;
	BOOL ret;

	if (!nsp_istable(tobj)) return;
	for (tobj = tobj->val->d.table.f;tobj;tobj = tobj->next) {
		if (!nsp_istable(tobj)) continue;
		cobj = nsp_getobj(N, tobj, "type");
		if (nsp_isnull(cobj)) continue;
		p = nsp_tostr(N, cobj);
		if (stricmp(p, "menu") == 0) {
			if ((hMenuSub = CreateMenu()) == NULL) continue;
			PopupMenuDrawSub(hMenuSub, nsp_getobj(N, tobj, "table"));
			ret = AppendMenu(hMenu, MF_POPUP | MF_BYPOSITION, (DWORD)hMenuSub, nsp_getstr(N, tobj, "name"));
		}
		else if (stricmp(p, "separator") == 0) {
			ret = AppendMenu(hMenu, MF_SEPARATOR, 0, "");
		}
		else {
			p = nsp_getstr(N, tobj, "name");
			ret = AppendMenu(hMenu, MF_STRING, IDM_STATE + G.index, p);
			nsp_setnum(N, tobj, "index", G.index++);
		}
	}
	if (!ret) DestroyMenu(hMenu);
	return;
}

void PopupMenuDraw(void)
{
	HMENU hMenu;
	obj_t *mobj;
	POINT point;

	EnterCriticalSection(&G.GlobalLock);
	IconStatus(2);
	mobj = nsp_getobj(N, &N->g, "MENUITEMS");
	if (!nsp_istable(mobj)) {
		new_menu(N);
		MessageBox(NULL, "missing MENUITEMS in config", TITLE_PREFIX "Script Error", MB_ICONSTOP);
	}
	hMenu = CreatePopupMenu();
	if (hMenu) {
		G.index = 0;
		PopupMenuDrawSub(hMenu, mobj);
		SetForegroundWindow(G.wnd);
		GetCursorPos(&point);
		TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, 0, G.wnd, NULL);
		PostMessage(G.wnd, WM_USER, 0, 0);
	}
	IconStatus(0);
	LeaveCriticalSection(&G.GlobalLock);
	return;
}

void PopupMenuExecSelected(HWND wnd, UINT nNewMode)
{
	obj_t *cobj, *cobj2, *mobj, *tobj;
	char *p, *t;

	EnterCriticalSection(&G.GlobalLock);
	mobj = nsp_getobj(N, &N->g, "MENUITEMS");
	if (!nsp_istable(mobj)) {
		new_menu(N);
		MessageBox(NULL, "missing MENUITEMS in config", TITLE_PREFIX "Script Error", MB_ICONSTOP);
	}
	tobj = getindex(mobj, nNewMode);
	if (!nsp_istable(tobj)) {
		goto done;
	}
	cobj = nsp_getobj(N, tobj, "type");
	if (!nsp_isstr(cobj) || cobj->val->size < 1) {
		goto done;
	}
	t = nsp_tostr(N, cobj);
	if (stricmp(t, "exit") == 0) {
		PostMessage(wnd, WM_CLOSE, 0, 0);
		goto done;
	}
	IconStatus(1);
	if (stricmp(t, "message") == 0) {
		cobj = nsp_getobj(N, tobj, "text");
		if (nsp_isstr(cobj) && cobj->val->size > 0) {
			cobj2 = nsp_getobj(N, tobj, "title");
			if ((cobj2->val->type == NT_STRING) && (cobj2->val->d.str != NULL)) p = cobj2->val->d.str; else p = "";
			MessageBox(NULL, nsp_tostr(N, cobj), p, MB_OK);
		}
	}
	else if (stricmp(t, "script") == 0) {
		cobj = nsp_getobj(N, tobj, "command");
		if (nsp_isstr(cobj) && cobj->val->size > 0) {
			nsp_exec(N, nsp_tostr(N, cobj));
			show_error(N);
		}
	}
	else if (stricmp(t, "CreateProcess") == 0) {
		cobj = nsp_getobj(N, tobj, "command");
		if (nsp_isstr(cobj) && cobj->val->size > 0) {
			winsystem(SW_SHOW, nsp_tostr(N, cobj));
		}
	}
	else if (stricmp(t, "ShellExecute") == 0) {
		cobj = nsp_getobj(N, tobj, "command");
		if (nsp_isstr(cobj) && cobj->val->size > 0) {
			ShellExecute(NULL, "open", nsp_tostr(N, cobj), NULL, NULL, SW_SHOWMAXIMIZED);
		}
	}
	IconStatus(0);
done:
	LeaveCriticalSection(&G.GlobalLock);
	return;
}

LRESULT CALLBACK TrayMenuExec(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//	HANDLE hThread;
	//	unsigned long int threadid;
	G.wnd = wnd;

	switch (uMsg) {
	case WM_CREATE:
		G.MenuHotKey = GlobalAddAtom("NSPTrayMenuHotKey");
		RegisterHotKey(wnd, G.MenuHotKey, MOD_WIN, 'N');
		break;
	case WM_HOTKEY:
		if (wParam == G.MenuHotKey) {
			PopupMenuDraw();
		}
		break;
	case WM_COMMAND:
		PopupMenuExecSelected(wnd, GET_WM_COMMAND_ID(wParam, lParam) - (IDM_STATE));
		break;
	case IDM_STATE: {
		switch (lParam) {
		case WM_LBUTTONDBLCLK:
			//	hThread=(HANDLE)_beginthreadex(NULL, 0, TrayClick, NULL, 0, &threadid);
			//	WaitForSingleObject(hThread, INFINITE);
			//	CloseHandle(hThread);
			_beginthread(TrayClick, 0, NULL);
			break;
		case WM_LBUTTONDOWN:
			break;
		case WM_RBUTTONDOWN:
			PopupMenuDraw();
			break;
		}
		break;
	}
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		UnregisterHotKey(wnd, G.MenuHotKey);
		EndDialog(wnd, TRUE);
	}
	return DefWindowProc(wnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK DlgTextInput(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG: {
		obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
		obj_t *cobj2 = nsp_getobj(N, &N->l, "2");

		SetWindowText(wnd, nsp_tostr(N, cobj2));
		SetDlgItemText(wnd, IDC_EDIT1, nsp_tostr(N, cobj1));
		break;
	}
	case WM_COMMAND: {
		char buffer[512];

		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(wnd, IDC_EDIT1, buffer, sizeof(buffer) - 1);
			EndDialog(wnd, 0);
			nsp_setstr(N, &N->r, "", buffer, -1);
			return DefWindowProc(wnd, uMsg, wParam, lParam);
		case IDCANCEL:
			EndDialog(wnd, 0);
			nsp_setnum(N, &N->r, "", -1);
			return DefWindowProc(wnd, uMsg, wParam, lParam);
		}
		break;
	}
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(wnd, 0);
		break;
	default:
		return DefWindowProc(wnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(wnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK DlgTrayNotice(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		/*	case WM_PAINT: {
				HDC dc;
				HFONT font;
				PAINTSTRUCT ps;
				RECT rect;

				GetClientRect(hwnd, &rect);
				rect.top+=5; rect.left+=5;
				rect.bottom-=5; rect.right-=5;
				dc=BeginPaint(hwnd, &ps);
				font=CreateFont(14,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"arial");
				SelectObject(dc, font);
				SetBkMode(dc, TRANSPARENT);
				SetTextColor(dc,0xFF0000);
				DrawText(dc, G.noticetext, -1, &rect, DT_TOP|DT_LEFT|DT_WORDBREAK|DT_EXPANDTABS);
				DeleteObject(font);
				EndPaint(hwnd, &ps);
				break;
			}
		*/	default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

NSP_FUNCTION(nsp_textinput)
{
	//	IconStatus(1);
	DialogBox(G.instance, MAKEINTRESOURCE(IDD_TEXTINPUT1), NULL, DlgTextInput);
	//	IconStatus(0);
	return 0;
}

NSP_FUNCTION(nsp_passinput)
{
	//	IconStatus(1);
	DialogBox(G.instance, MAKEINTRESOURCE(IDD_TEXTINPUT2), NULL, DlgTextInput);
	//	IconStatus(0);
	return 0;
}

TRAYNOTICESTRUCT TNS;

//unsigned _stdcall traynoticethread(void *x)
void traynoticethread(void *x)
{
	WNDCLASSEX WndClsEx;
	HWND hwnd;
	static int init = 0;
	int te;
	MSG msg;

	if (!init) {
		memset(&WndClsEx, 0, sizeof(WndClsEx));
		WndClsEx.cbSize = sizeof(WNDCLASSEX);
		WndClsEx.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		WndClsEx.lpfnWndProc = DlgTrayNotice;
		WndClsEx.hInstance = G.instance;
		WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		//	WndClsEx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		WndClsEx.hbrBackground = CreateSolidBrush((COLORREF)0x00D8D8D8);
		WndClsEx.lpszClassName = "TrayNotice";
		if (!RegisterClassEx(&WndClsEx)) {
			_endthread();
			return;
		}
		init = 1;
	}
	hwnd = CreateWindowEx(WS_EX_PALETTEWINDOW, "TrayNotice", "Tray Notification", WS_POPUPWINDOW, TNS.x, TNS.y, TNS.w, TNS.h, NULL, NULL, G.instance, NULL);
	_snprintf(G.noticetext, sizeof(G.noticetext) - 1, "%s", TNS.text);
	SetWindowText(hwnd, "...");
	ShowWindow(hwnd, SW_SHOW);
	te = (int)time(NULL) + TNS.t;


	{
		HDC dc;
		HFONT font;
		PAINTSTRUCT ps;
		RECT rect;

		GetClientRect(hwnd, &rect);
		rect.top += 5; rect.left += 5;
		rect.bottom -= 5; rect.right -= 5;
		dc = BeginPaint(hwnd, &ps);
		font = CreateFont(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "arial");
		SelectObject(dc, font);
		SetBkMode(dc, TRANSPARENT);
		SetTextColor(dc, 0xFF0000);
		DrawText(dc, G.noticetext, -1, &rect, DT_TOP | DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS);
		DeleteObject(font);
		EndPaint(hwnd, &ps);
	}
	free(TNS.text);
	TNS.text = NULL;


	for (;time(NULL) < te;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Sleep(1);
		}
	}
	ShowWindow(hwnd, SW_HIDE);
	DestroyWindow(hwnd);
	_endthread();
	return;
}

NSP_FUNCTION(nsp_traynotice)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");

	IconNotify(NIM_MODIFY, nsp_isstr(cobj1) ? cobj1->val->d.str : NULL, nsp_isstr(cobj2) ? cobj2->val->d.str : NULL);
	return 0;
}

NSP_FUNCTION(nsp_traynotice2)
{
	//	HANDLE hThread;
	obj_t *cobj1, *cobj2, *cobj3, *cobj4;
	obj_t *cobj;
	RECT rcWork;
	//	unsigned long int threadid;

	cobj1 = nsp_getobj(N, &N->l, "1");
	cobj2 = nsp_getobj(N, &N->l, "2");
	cobj3 = nsp_getobj(N, &N->l, "3");
	cobj4 = nsp_getobj(N, &N->l, "4");
	TNS.t = nsp_isnum(cobj3) ? (int)nsp_tonum(N, cobj3) : 5;
	TNS.x = 0;
	TNS.y = 0;
	TNS.w = 220;
	TNS.h = 110;
	if (!nsp_isstr(cobj1) || cobj1->val->size < 1) {
		return 0;
	}
	if (TNS.t < 1) {
		return 0;
	}
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
	if (nsp_istable(cobj4)) {
		if (nsp_isnum((cobj = nsp_getobj(N, cobj4, "w")))) {
			TNS.w = (unsigned short)nsp_tonum(N, cobj);
		}
		if (nsp_isnum((cobj = nsp_getobj(N, cobj4, "h")))) {
			TNS.h = (unsigned short)nsp_tonum(N, cobj);
		}
	}
	if (nsp_istable(cobj4)) {
		if (nsp_isnum((cobj = nsp_getobj(N, cobj4, "x")))) {
			TNS.x = (int)nsp_tonum(N, cobj);
			if (TNS.x < 0) TNS.x = rcWork.right + TNS.x;
		}
		else {
			TNS.x = rcWork.right - TNS.w;
		}
		if (nsp_isnum((cobj = nsp_getobj(N, cobj4, "y")))) {
			TNS.y = (int)nsp_tonum(N, cobj);
			if (TNS.y < 0) TNS.y = rcWork.bottom + TNS.y;
		}
		else {
			TNS.y = rcWork.bottom - TNS.h;
		}
	}
	else {
		TNS.x = rcWork.right - TNS.w;
		TNS.y = rcWork.bottom - TNS.h;
	}
	TNS.text = calloc(1, cobj1->val->size + 1);
	_snprintf(TNS.text, cobj1->val->size, "%s", cobj1->val->d.str);

	//	_beginthreadex(NULL, 0, traynoticethread, NULL, 0, &threadid);
	//	hThread=(HANDLE)_beginthreadex(NULL, 0, traynoticethread, NULL, 0, &threadid);
	//	WaitForSingleObject(hThread, INFINITE);
	//	CloseHandle(hThread);
	_beginthread(traynoticethread, 0, NULL);

	while (TNS.text != NULL) Sleep(1);
	return 0;
}

DWORD do_filter(EXCEPTION_POINTERS *eps)
{
	EXCEPTION_RECORD er = *eps->ExceptionRecord;
	char errbuf[255];

	memset(errbuf, 0, sizeof(errbuf));
	switch (er.ExceptionCode) {
	case 0xE06D7363: // C++ exception
		_snprintf(errbuf, sizeof(errbuf) - 1, "Unknown C++ exception thrown. 0x%08X", er.ExceptionCode);
		break;
	case EXCEPTION_ACCESS_VIOLATION:
		_snprintf(errbuf, sizeof(errbuf) - 1, "EXCEPTION_ACCESS_VIOLATION (0x%08X): ExceptionAddress=0x%08X", er.ExceptionCode, (unsigned int)er.ExceptionAddress);
		break;
	case EXCEPTION_STACK_OVERFLOW:
		_snprintf(errbuf, sizeof(errbuf) - 1, "EXCEPTION_STACK_OVERFLOW (0x%08X): ExceptionAddress=0x%08X", er.ExceptionCode, (unsigned int)er.ExceptionAddress);
		break;
	default:
		_snprintf(errbuf, sizeof(errbuf) - 1, "SEH Exception (0x%08X): ExceptionAddress=0x%08X", er.ExceptionCode, (unsigned int)er.ExceptionAddress);
		break;
	}
	log_error(N, "SEH Exception [%s]", errbuf);
	MessageBox(NULL, errbuf, TITLE_PREFIX "SEH Exception", MB_OK);
	return EXCEPTION_EXECUTE_HANDLER;
}


void StartTrayIconLoop()
{
	//	HANDLE hThread;
	//	unsigned long int threadid;
	WNDCLASSEX  wndclass;
	HWND        MainWnd;
	MSG         msg;

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = 0;
	wndclass.lpfnWndProc = TrayMenuExec;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = G.instance;
	wndclass.hIcon = LoadIcon(G.instance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "NTrayClass";
	RegisterClassEx(&wndclass);
	MainWnd = CreateWindow("NTrayClass", "NTray", 0, 0, 0, 1, 1, NULL, NULL, G.instance, NULL);
	IconNotify(NIM_ADD, NULL, NULL);
	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_CLOSE) break;
		}
		else {
			Sleep(1);
		}
		if (time(NULL) > G.lastpoll) {
			struct stat sb;
			time_t t;

			//			Beep(880, 25);
			if (stat(CONFFILE, &sb) == 0 && sb.st_mtime > G.lastfile) {
				//	_beginthreadex(NULL, 0, TrayTimer_NewConfig, NULL, 0, &threadid);
				//	hThread=(HANDLE)_beginthreadex(NULL, 0, TrayTimer_NewConfig, NULL, 0, &threadid);
				//	WaitForSingleObject(hThread, INFINITE);
				//	CloseHandle(hThread);
				_beginthread(TrayTimer_NewConfig, 0, NULL);
				//	Sleep(1000);
			}
			else {
				t = time(NULL);
				if ((t % 60) < (G.lastpoll % 60)) {
					G.lastpoll = t;
					//	_beginthreadex(NULL, 0, TrayTimer_OnTimer, NULL, 0, &threadid);
					//	hThread=(HANDLE)_beginthreadex(NULL, 0, TrayTimer_OnTimer, NULL, 0, &threadid);
					//	WaitForSingleObject(hThread, INFINITE);
					//	CloseHandle(hThread);
					_beginthread(TrayTimer_OnTimer, 0, NULL);
				}
				else {
					G.lastpoll = t;
				}
				//	Sleep(1000);
			}
		}
	}
	IconNotify(NIM_DELETE, NULL, NULL);
	UnregisterClass("NTrayClass", G.instance);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HANDLE mutex;
	obj_t *cobj;

	__try {
		memset(&G, 0, sizeof(G));
		G.instance = hInstance;
		InitializeCriticalSection(&G.GlobalLock);
		if ((N = nsp_newstate()) == NULL) return -1;
		init_stuff(N);
		cobj = nsp_getobj(N, &N->g, "MUTEXNAME");
		if (nsp_isstr(cobj) && cobj->val->size > 0) {
			mutex = CreateMutex(NULL, FALSE, nsp_tostr(N, cobj));
		}
		else {
			mutex = CreateMutex(NULL, FALSE, "NTRAY_MUTEX");
		}
		if ((mutex == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
			if (mutex) CloseHandle(mutex);
			nsp_endstate(N);
			return 0;
		}
		cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "NTray"), "onload");
		if (nsp_typeof(cobj) == NT_NFUNC) {
			nsp_exec(N, "NTray.onload();");
			show_error(N);
		}
		StartTrayIconLoop();
		cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "NTray"), "onexit");
		if (nsp_typeof(cobj) == NT_NFUNC) {
			nsp_exec(N, "NTray.onexit();");
			show_error(N);
		}
		CloseHandle(mutex);
		nsp_endstate(N);
		DeleteCriticalSection(&G.GlobalLock);
	}
	__except (do_filter(GetExceptionInformation())) {
	}
	return 0;
}
#endif //WIN32
