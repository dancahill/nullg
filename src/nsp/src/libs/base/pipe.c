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

#ifdef HAVE_PIPE

//void pipe_murder(nsp_state *N, obj_t *cobj);

typedef struct {
	int in;
	int out;
} pipe_fd;

typedef struct PIPE_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's pipe-specific */
	pipe_fd local;
	pipe_fd remote;
#ifdef WIN32
	PROCESS_INFORMATION pi;
#endif
} PIPE_CONN;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define BUFF_SIZE 8192

void pipe_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":pipe_murder()"
	//	PIPE_CONN *conn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "pipe-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a pipe conn");
	//	conn=(PIPE_CONN *)cobj->val->d.str;
	n_free(N, (void *)&cobj->val->d.str, sizeof(PIPE_CONN) + 1);
	return;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_pipe_open)
{
#define __FN__ __FILE__ ":libnsp_base_pipe_open()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	PIPE_CONN *conn;

#ifdef WIN32
	HANDLE hMyProcess = GetCurrentProcess();
	SECURITY_ATTRIBUTES saAttr;
	STARTUPINFO si;
	char Command[512];
	char Path[255];
	char *ptemp;
#else
	int pset1[2];
	int pset2[2];
#endif
	char *args[5];

	int pid;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	conn = calloc(1, sizeof(PIPE_CONN) + 1);
	if (conn == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(PIPE_CONN) + 1);
		return -1;
	}
	conn->obj_term = (NSP_CFREE)pipe_murder;
	strcpy(conn->obj_type, "pipe-conn");

	//	fixslashes(aspell_loc);
	//	if (strlen(searchstring)<1) return 0;
	memset(args, 0, sizeof(args));
	args[0] = cobj1->val->d.str;
	if (cobj2->val->type == NT_STRING) args[1] = cobj2->val->d.str;
	if (cobj3->val->type == NT_STRING) args[2] = cobj3->val->d.str;
#ifdef WIN32
	memset(Command, 0, sizeof(Command));
	ZeroMemory(&conn->pi, sizeof(conn->pi));
	ZeroMemory(&si, sizeof(si));
	snprintf(Path, sizeof(Path) - 1, "%s", cobj1->val->d.str);
	if ((ptemp = strrchr(Path, '\\')) != NULL) *ptemp = '\0';
	snprintf(Command, sizeof(Command) - 1, "%s", cobj1->val->d.str);
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe((HANDLE)&conn->remote.in, (HANDLE)&conn->local.out, &saAttr, BUFF_SIZE)) {
		n_warn(N, __FN__, "Unable to create pipe");
		return 0;
	}
	if (!CreatePipe((HANDLE)&conn->local.in, (HANDLE)&conn->remote.out, &saAttr, BUFF_SIZE)) {
		CloseHandle((HANDLE)conn->remote.in);
		CloseHandle((HANDLE)conn->local.out);
		n_warn(N, __FN__, "Unable to create pipe");
		return 0;
	}
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = (HANDLE)conn->remote.in;
	si.hStdOutput = (HANDLE)conn->remote.out;
	si.hStdError = (HANDLE)conn->remote.out;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_NO_WINDOW, NULL, Path, &si, &conn->pi)) {
		CloseHandle((HANDLE)conn->local.in);
		CloseHandle((HANDLE)conn->local.out);
		CloseHandle((HANDLE)conn->remote.in);
		CloseHandle((HANDLE)conn->remote.out);
		n_warn(N, __FN__, "program failed. [%s]", Command);
		return 0;
	}
	pid = conn->pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
#else
	if ((pipe(pset1) == -1) || (pipe(pset2) == -1)) {
		close(pset1[0]);
		close(pset1[1]);
		n_warn(N, __FN__, "pipe() error");
		return 0;
	}
	conn->local.in = pset1[0]; conn->remote.out = pset1[1];
	conn->remote.in = pset2[0]; conn->local.out = pset2[1];
	pid = fork();
	if (pid < 0) {
		n_warn(N, __FN__, "fork() error");
		return 0;
	}
	else if (pid == 0) {
		close(conn->local.in);
		close(conn->local.out);
		dup2(conn->remote.in, fileno(stdin));
		dup2(conn->remote.out, fileno(stdout));
		execve(args[0], &args[0], NULL);
		n_warn(N, __FN__, "execve() error [%s]", args[0]);
		//		exit(0);
	}
	else {
		close(conn->remote.in);
		close(conn->remote.out);
	}
#endif
	/* nsp_setcdata(N, &N->r, "", sock, sizeof(TCP_SOCKET)+1); */
	nsp_setcdata(N, &N->r, "", NULL, 0);
	N->r.val->d.str = (void *)conn;
	N->r.val->size = sizeof(PIPE_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_pipe_read)
{
#define __FN__ __FILE__ ":libnsp_base_pipe_read()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	PIPE_CONN *conn;
	int bytesin;
	char szBuffer[BUFF_SIZE];

	if ((cobj1->val->type != NT_CDATA) || (cobj1->val->d.str == NULL) || (strcmp(cobj1->val->d.str, "pipe-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a pipe-conn for arg1");
	conn = (PIPE_CONN *)cobj1->val->d.str;
	nsp_setstr(N, &N->r, "", NULL, 0);
	do {
		memset(szBuffer, 0, sizeof(szBuffer));
#ifdef WIN32
		ReadFile((HANDLE)conn->local.in, szBuffer, sizeof(szBuffer) - 1, &bytesin, NULL);
#else
		bytesin = read(conn->local.in, szBuffer, sizeof(szBuffer) - 1);
#endif
		// printf("[%d][%s]", bytesin, szBuffer);
		nsp_strcat(N, &N->r, szBuffer, bytesin);
		if (bytesin < 1) break;
	} while (0);
	//	} while (strchr(szBuffer, '\n')==NULL);
	//	nsp_setstr(N, &N->r, "", szBuffer, bytesin);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_pipe_write)
{
#define __FN__ __FILE__ ":libnsp_base_pipe_write()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	PIPE_CONN *conn;
	char *p;
	int pl;
	int bytesout;

	if ((cobj1->val->type != NT_CDATA) || (cobj1->val->d.str == NULL) || (strcmp(cobj1->val->d.str, "pipe-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a pipe-conn for arg1");
	conn = (PIPE_CONN *)cobj1->val->d.str;
	p = cobj2->val->d.str;
	pl = cobj2->val->size;
	do {
#ifdef WIN32
		WriteFile((HANDLE)conn->local.out, p, pl, &bytesout, NULL);
#else
		bytesout = write(conn->local.out, p, pl);
#endif
		if (bytesout > 0) {
			p += bytesout;
			pl -= bytesout;
		}
	} while (pl > 0);
	nsp_setnum(N, &N->r, "", bytesout);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_pipe_close)
{
#define __FN__ __FILE__ ":libnsp_base_pipe_close()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	PIPE_CONN *conn;
#ifdef WIN32
	DWORD exitcode = 0;
#else
	int status;
#endif

	if ((cobj1->val->type != NT_CDATA) || (cobj1->val->d.str == NULL) || (strcmp(cobj1->val->d.str, "pipe-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a pipe-conn for arg1");
	conn = (PIPE_CONN *)cobj1->val->d.str;
#ifdef WIN32
	GetExitCodeProcess(conn->pi.hProcess, &exitcode);
	if (exitcode == STILL_ACTIVE) TerminateProcess(conn->pi.hProcess, 1);
	CloseHandle(conn->pi.hThread);
	CloseHandle(conn->pi.hProcess);
	CloseHandle((HANDLE)conn->local.in);
	CloseHandle((HANDLE)conn->local.out);
#else
	close(conn->local.in);
	close(conn->local.out);
	wait(&status);
#endif
	n_free(N, (void *)&cobj1->val->d.str, sizeof(PIPE_CONN) + 1);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

#endif /* HAVE_PIPE */
