/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#define SRVMOD_MAIN 1
#include "nullsd/httpd_mod.h"
#ifndef WIN32
#include <sys/wait.h>
#endif

#define BUFF_SIZE 8192

typedef struct {
	int in;
	int out;
} pipe_fd;

void cgi_makeargs(CONN *conn, char *args[])
{
	obj_t *confobj=nsp_settable(proc->N, &proc->N->g, "CONFIG");
//	obj_t *hrobj=nsp_settable(conn->N, &conn->N->g, "_HEADER");
	obj_t *htobj=nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj;
	char *RequestURI=nsp_getstr(conn->N, htobj, "REQUEST_URI");
	char *ptemp;
	char progname[255];

	if (strncmp(RequestURI, "/cgi-bin/", 9)!=0) return;
	args[0]=calloc(255, sizeof(char));
//	snprintf(progname, sizeof(progname)-1, "%s", conn->dat->in_CGIScriptName+9);
	cobj=nsp_getobj(conn->N, htobj, "SCRIPT_NAME");
	if (nsp_isstr(cobj)) {
		snprintf(progname, sizeof(progname)-1, "%s", nsp_tostr(conn->N, cobj)+9);
	}
	if ((ptemp=strchr(progname, '?'))!=NULL) {
		args[1]=calloc(255, sizeof(char));
		snprintf(args[1], 254, "%s", ptemp+1);
		*ptemp='\0';
	}
//	if ((ptemp=strchr(progname, '/'))!=NULL) {
//		args[2]=calloc(255, sizeof(char));
//		snprintf(args[2], 254, "%s", ptemp);
//		*ptemp='\0';
//	}
	snprintf(args[0], 254, "%s/cgi-bin/%s", nsp_getstr(proc->N, confobj, "var_path"), progname);
	fixslashes(args[0]);
	cobj=nsp_getobj(conn->N, htobj, "QUERY_STRING");
	if (nsp_isstr(cobj)) {
//	if (strlen(conn->dat->in_QueryString)) {
		args[1]=calloc(255, sizeof(char));
		snprintf(args[1], 254, "%s", nsp_tostr(conn->N, cobj));
	}
	cobj=nsp_getobj(conn->N, htobj, "PATH_INFO");
	if (nsp_isstr(cobj)) {
//	if (strlen(conn->dat->in_PathInfo)) {
		args[2]=calloc(255, sizeof(char));
		snprintf(args[2], 254, "%s", nsp_tostr(conn->N, cobj));
	}
}

void cgi_makeenv(CONN *conn, char *env[], char *args[])
{
	obj_t *confobj=nsp_settable(proc->N, &proc->N->g, "CONFIG");
	obj_t *hrobj=nsp_settable(conn->N, &conn->N->g, "_HEADER");
	obj_t *htobj=nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj;
	char *RequestURI=nsp_getstr(conn->N, htobj, "REQUEST_URI");
	char *ptemp;
	int n=0;

	if (strncmp(RequestURI, "/cgi-bin/", 9)!=0) return;
#ifdef WIN32
	if ((ptemp=getenv("COMSPEC"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], MAX_PATH-1, "COMSPEC=%s", ptemp);
	}
#endif
	if (strcasecmp(nsp_getstr(conn->N, htobj, "REQUEST_METHOD"), "POST")==0) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "CONTENT_LENGTH=%d", atoi(nsp_getstr(conn->N, htobj, "CONTENT_LENGTH")));
		env[n]=calloc(1024, sizeof(char));
		cobj=nsp_getobj(conn->N, htobj, "CONTENT_TYPE");
		if (nsp_isstr(cobj)) {
			snprintf(env[n++], 1023, "CONTENT_TYPE=%s", nsp_tostr(conn->N, cobj));
		} else {
			snprintf(env[n++], 1023, "CONTENT_TYPE=application/x-www-form-urlencoded");
		}
	}
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "DOCUMENT_ROOT=%s/htdocs", nsp_getstr(proc->N, confobj, "var_path"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "GATEWAY_INTERFACE=CGI/1.1");
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_CONNECTION=%s", nsp_getstr(conn->N, hrobj, "CONNECTION"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_COOKIE=%s", nsp_getstr(conn->N, htobj, "HTTP_COOKIE"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_HOST=%s", nsp_getstr(conn->N, htobj, "HTTP_HOST"));
	if ((ptemp=strchr(env[n-1], ':'))!=NULL) *ptemp='\0';
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_USER_AGENT=%s", nsp_getstr(conn->N, htobj, "HTTP_USER_AGENT"));
	if ((ptemp=getenv("PATH"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "PATH=%s", ptemp);
	}
	if (args[2]!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "PATH_INFO=%s", args[2]);
	}
	env[n]=calloc(1024, sizeof(char));
	if (args[1]!=NULL) {
		snprintf(env[n++], 1023, "QUERY_STRING=%s", args[1]);
	} else {
		snprintf(env[n++], 1023, "QUERY_STRING=");
	}
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REMOTE_ADDR=%s", nsp_getstr(conn->N, htobj, "REMOTE_ADDR"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REMOTE_PORT=%d", atoi(nsp_getstr(conn->N, htobj, "REMOTE_PORT")));
	env[n]=calloc(1024, sizeof(char));
//	snprintf(env[n++], 1023, "REMOTE_USER=%s", conn->dat->user_username);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REQUEST_METHOD=%s", nsp_getstr(conn->N, htobj, "REQUEST_METHOD"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REQUEST_URI=%s", nsp_getstr(conn->N, htobj, "REQUEST_URI"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SCRIPT_FILENAME=%s", args[0]);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SCRIPT_NAME=%s", nsp_getstr(conn->N, htobj, "SCRIPT_NAME"));
	if ((ptemp=strchr(env[n-1], '?'))!=NULL) *ptemp='\0';
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_NAME=%s", nsp_getstr(conn->N, htobj, "HTTP_HOST"));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_PORT=%d", atoi(nsp_getstr(conn->N, htobj, "SERVER_PORT")));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_PROTOCOL=HTTP/1.1");
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_SIGNATURE=<ADDRESS>%s %s</ADDRESS>", SERVER_NAME, PACKAGE_VERSION);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_SOFTWARE=%s %s", SERVER_NAME, PACKAGE_VERSION);
#ifdef WIN32
	if ((ptemp=getenv("SYSTEMROOT"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "SYSTEMROOT=%s", ptemp);
	}
	if ((ptemp=getenv("WINDIR"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "WINDIR=%s", ptemp);
	}
#endif
	free(args[1]);
	args[1]=NULL;
	free(args[2]);
	args[2]=NULL;
}

DllExport int mod_main(CONN *conn)
{
	obj_t *hrobj=nsp_settable(conn->N, &conn->N->g, "_HEADER");
	obj_t *htobj=nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj;
#ifdef WIN32
	char *cgi_types[3][2]={
		{ ".php", "PHP.EXE" },
		{ ".pl",  "PERL.EXE" },
		{ NULL,   NULL }
	};
	DWORD exitcode=0;
	HANDLE hMyProcess=GetCurrentProcess();
	SECURITY_ATTRIBUTES saAttr;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char Command[512];
	char Environ[8192];
	char Path[255];
#else
	char *cgi_types[3][2]={
		{ ".php", "/usr/bin/php" },
		{ ".pl",  "/usr/bin/perl" },
		{ NULL,   NULL }
	};
	int status;
	int pset1[2];
	int pset2[2];
#endif
	char *args[10];
	char *env[50];
	char cgifilename[255];
	char *extension;
	char *ptemp;
	char szBuffer[BUFF_SIZE];
	pipe_fd local;
	pipe_fd remote;
	int nOutRead;
	int pid;
	unsigned int i;
	unsigned int n;
	int bytesleft;

	memset(args, 0, sizeof(args));
	cgi_makeargs(conn, args);
	memset(env, 0, sizeof(env));
	cgi_makeenv(conn, env, args);
	snprintf(cgifilename, sizeof(cgifilename)-1, "%s", args[0]);
	for (i=0;i<10;i++) free(args[i]);
	n=0;
	if ((extension=strrchr(cgifilename, '.'))!=NULL) {
		for (i=0;cgi_types[i][0]!=NULL;i++) {
			if (strcmp(extension, cgi_types[i][0])==0) {
				args[n]=calloc(255, sizeof(char));
				snprintf(args[n], 254, "%s", cgi_types[i][1]);
				n++;
				break;
			}
		}
	}
	args[n]=calloc(255, sizeof(char));
	snprintf(args[n], 254, "%s", cgifilename);
#ifdef WIN32
	memset(Command, 0, sizeof(Command));
	memset(Environ, 0, sizeof(Environ));
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	snprintf(Path, sizeof(Path)-1, "%s", cgifilename);
	if ((extension=strrchr(Path, '\\'))!=NULL) *extension='\0';
	if (args[1]==NULL) {
		snprintf(Command, sizeof(Command)-1, "%s", cgifilename);
	} else {
		snprintf(Command, sizeof(Command)-1, "%s \"%s\"", cgi_types[i][1], cgifilename);
	}
	for (i=0, n=0;env[i]!=NULL;i++) {
		if (n+strlen(env[i])>sizeof(Environ)) break;
		n+=sprintf(&Environ[n], "%s", env[i]);
		n++;
	}
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe((HANDLE)&remote.in, (HANDLE)&local.out, &saAttr, BUFF_SIZE)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		send_error(conn, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	if (!CreatePipe((HANDLE)&local.in, (HANDLE)&remote.out, &saAttr, BUFF_SIZE)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)local.out);
		send_error(conn, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdInput=(HANDLE)remote.in;
	si.hStdOutput=(HANDLE)remote.out;
	si.hStdError=(HANDLE)remote.out;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE|CREATE_NO_WINDOW, Environ, Path, &si, &pi)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		CloseHandle((HANDLE)local.in);
		CloseHandle((HANDLE)local.out);
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)remote.out);
		log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "CGI failed. [%s]", Command);
		send_error(conn, 500, "Internal Server Error", "There was a problem running the requested CGI.");
		return 0;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
#else
	if ((pipe(pset1)==-1) || (pipe(pset2)==-1)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		close(pset1[0]);
		close(pset1[1]);
		log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "pipe() error");
		send_error(conn, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	local.in=pset1[0]; remote.out=pset1[1];
	remote.in=pset2[0]; local.out=pset2[1];
	log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 2, "Executing CGI [%s %s]", args[0], args[1]);
	pid=fork();
	if (pid<0) {
		log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "fork() error");
		return 0;
	} else if (pid==0) {
		close(local.in);
		close(local.out);
		dup2(remote.in, fileno(stdin));
		dup2(remote.out, fileno(stdout));
//		if ((dup2(remote.in, fileno(stdin))!=0)||(dup2(remote.out, fileno(stdout))!=0)) {
//			log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "dup2() error");
//			exit(0);
//		}
		execve(args[0], &args[0], &env[0]);
		log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "execve() error [%s][%s]", args[0], args[1]);
		exit(0);
	} else {
		close(remote.in);
		close(remote.out);
	}
#endif
	if (atoi(nsp_getstr(conn->N, htobj, "CONTENT_LENGTH"))>0) {
		bytesleft=atoi(nsp_getstr(conn->N, htobj, "CONTENT_LENGTH"));

		cobj=nsp_getobj(conn->N, &conn->N->g, "POSTRAWDATA");
//		ptemp=conn->PostData;
		ptemp=cobj->val->d.str;
//		log_error(proc->N, "mod_cgi", __FILE__, __LINE__, 1, "--[%d][%s]", bytesleft, ptemp);
		while (bytesleft>0) {
#ifdef WIN32
			i=WriteFile((HANDLE)local.out, ptemp, bytesleft, &nOutRead, NULL);
#else
			i=write(local.out, ptemp, bytesleft);
#endif
			if (i>0) {
				ptemp+=i;
				bytesleft-=i;
			} else if (i<1) {
				break;
			}
		}
	}
	conn->dat->out_headdone=1;
	conn->dat->out_status=200;
	if (p_strcasestr(nsp_getstr(conn->N, hrobj, "PROTOCOL"), "HTTP/1.1")!=NULL) {
//		snprintf(conn->dat->out_Protocol, sizeof(conn->dat->out_Protocol)-1, "HTTP/1.1");
//	} else {
//		snprintf(conn->dat->out_Protocol, sizeof(conn->dat->out_Protocol)-1, "HTTP/1.0");
//	}
//	if (p_strcasestr(Protocol, "HTTP/1.1")!=NULL) {
		nsp_setstr(conn->N, hrobj, "PROTOCOL", "HTTP/1.1", strlen("HTTP/1.1"));
	} else {
		nsp_setstr(conn->N, hrobj, "PROTOCOL", "HTTP/1.0", strlen("HTTP/1.0"));
	}
	nsp_setstr(conn->N, hrobj, "CONNECTION", "Close", strlen("Close"));
//	snprintf(conn->dat->out_Connection, sizeof(conn->dat->out_Connection)-1, "Close");
	prints(conn, "%s %d OK\r\n", nsp_getstr(conn->N, hrobj, "PROTOCOL"), conn->dat->out_status);
	prints(conn, "Connection: %s\r\n", nsp_getstr(conn->N, hrobj, "CONNECTION"));
	flushbuffer(conn);
	do {
		memset(szBuffer, 0, sizeof(szBuffer));
#ifdef WIN32
		ReadFile((HANDLE)local.in, szBuffer, sizeof(szBuffer)-1, &nOutRead, NULL);
#else
		nOutRead=read(local.in, szBuffer, BUFF_SIZE-1);
#endif
		if (nOutRead>0) {
			tcp_send(&conn->socket, szBuffer, nOutRead, 0);
			conn->dat->out_bytecount+=nOutRead;
		};
	} while (nOutRead>0);
	flushbuffer(conn);
	/* cleanup */
	for (i=0;i<10;i++) free(args[i]);
	for (i=0;i<50;i++) free(env[i]);
#ifdef WIN32
	GetExitCodeProcess(pi.hProcess, &exitcode);
	if (exitcode==STILL_ACTIVE) TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle((HANDLE)local.in);
	CloseHandle((HANDLE)local.out);
#else
	close(local.in);
	close(local.out);
	wait(&status);
#endif
	conn->dat->out_bodydone=1;
	flushbuffer(conn);
	closeconnect(conn, 1);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *tobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	tobj=nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "GWMODULES"), "cgi");
	nsp_exec(proc->N, "GWMODULES.cgi={fn_name='mod_main',fn_uri='/cgi-bin/'};");
	nsp_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nsp_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nsp_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
