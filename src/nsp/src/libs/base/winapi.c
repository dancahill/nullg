/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2019 Dan Cahill

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
#include "nsp/nsplib.h"
#include "base.h"
#ifdef WIN32
#include <mmsystem.h>
#include <shellapi.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp

static int winsystem(WORD show_hide, const char *format, ...)
{
	DWORD exitcode = 0;
	HANDLE hMyProcess = GetCurrentProcess();
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char command[256];
	va_list ap;
	int pid;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	memset(command, 0, sizeof(command));
	va_start(ap, format);
	vsnprintf(command, sizeof(command) - 1, format, ap);
	va_end(ap);
	si.cb = sizeof(si);
	//	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = show_hide;
	//	si.hStdInput=NULL;
	//	si.hStdOutput=NULL;
	//	si.hStdError=NULL;
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		MessageBox(NULL, command, "CreateProcess error", MB_ICONSTOP);
		return -1;
	}
	pid = pi.dwProcessId;
	//	CloseHandle(si.hStdInput);
	//	CloseHandle(si.hStdOutput);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}

NSP_FUNCTION(libnsp_winapi_beep)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");

	Beep((unsigned long)(nsp_isnum(cobj1) ? cobj1->val->d.num : 0), (unsigned long)(nsp_isnum(cobj1) ? cobj2->val->d.num : 0));
	return 0;
}

NSP_FUNCTION(libnsp_winapi_createprocess)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	WORD show_hide = SW_SHOW;

	if (cobj2 != NULL&&cobj2->val != NULL) {
		if ((cobj2->val->type == NT_BOOLEAN) || (cobj2->val->type == NT_NUMBER)) {
			if (cobj2->val->d.num == 0) show_hide = SW_HIDE;
		}
	}
	if ((cobj1->val->type == NT_STRING) && (cobj1->val->d.str != NULL)) {
		winsystem(show_hide, cobj1->val->d.str);
	}
	nsp_setnum(N, &N->r, "", 0);
	return 0;
}

NSP_FUNCTION(libnsp_winapi_messagebox)
{
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	UINT uType = 0;
	int rc;

	if (cobj3->val->type == NT_NUMBER) uType = (int)cobj3->val->d.num;
	rc = MessageBox(NULL, nsp_getstr(N, &N->context->l, "1"), nsp_getstr(N, &N->context->l, "2"), uType);
	nsp_setnum(N, &N->r, "", rc);
	return 0;
}

NSP_FUNCTION(libnsp_winapi_playsound)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	int opt = SND_ASYNC | SND_NODEFAULT | SND_FILENAME;
	int rc = 0;

	if ((cobj1->val->type != NT_STRING) || (cobj1->val->size < 1)) return 0;
	if (cobj2->val->type == NT_NUMBER) opt = (int)cobj2->val->d.num;
	rc = sndPlaySound(cobj1->val->d.str, opt);
	nsp_setnum(N, &N->r, "", rc);
	return 0;
}

NSP_FUNCTION(libnsp_winapi_shellexecute)
{
	ShellExecute(NULL, "open", nsp_getstr(N, &N->context->l, "1"), NULL, NULL, SW_SHOWMAXIMIZED);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
}
#endif
