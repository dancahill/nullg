/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "httpd_main.h"

#ifdef WIN32
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
#define pthread_exit(A) _endthread()
typedef unsigned(__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned(__stdcall *func)(void *), void *param);
int pthread_kill(pthread_t handle, int sig);

int pthread_attr_init(pthread_attr_t *connect_att)
{
	connect_att->dwStackSize = 0;
	connect_att->dwCreatingFlag = 0;
	connect_att->priority = 0;
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack)
{
	connect_att->dwStackSize = stack;
	return 0;
}

int pthread_attr_setprio(pthread_attr_t *connect_att, int priority)
{
	connect_att->priority = priority;
	return 0;
}

int pthread_attr_destroy(pthread_attr_t *connect_att)
{
	return 0;
}

int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned(__stdcall *func)(void *), void *param)
{
	HANDLE hThread;
	unsigned long int id;

	hThread = (HANDLE)_beginthreadex(NULL, attr->dwStackSize ? attr->dwStackSize : PTHREAD_STACK_SIZE, func, param, 0, &id);
	if ((long)hThread == -1L) return (errno ? errno : -1);
	*thread_id = hThread;
	return id;
}

int pthread_kill(pthread_t handle, int sig)
{
	int rc;

	TerminateThread(handle, (DWORD)&rc);
	CloseHandle(handle);
	return 0;
}
#endif

HTTP_PROC htproc;
static pthread_mutex_t ListenerMutex;

#ifdef WIN32
DWORD do_filter(EXCEPTION_POINTERS *eps, CONN *conn)
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
	{
		log_error(proc->N, "core", __FILE__, __LINE__, 0, "SEH Exception (htloop): %s", errbuf);
		log_error(proc->N, "core", __FILE__, __LINE__, 0, "SEH Exception (htloop):\r\n\t"
			"RemoteAddr=%s", conn->socket.RemoteAddr);
		system("crash.ns");
	}
	//printf("SEH Exception (htloop): %s", errbuf);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef WIN32
unsigned _stdcall htloop(void *x)
#else
void *htloop(void *x)
#endif
{
	CONN *conn = x;
	obj_t *cobj;
	short k;

	log_error(proc->N, "core", __FILE__, __LINE__, 2, "Starting htloop() thread");
	if (conn == NULL) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "missing sid");
		return 0;
	}
	conn->id = pthread_self();
#ifndef WIN32
	pthread_detach(conn->id);
#endif
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Opening connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
	proc->stats.http_conns++;
#ifdef WIN32
	__try {
#endif
		for (;;) {
			if (conn->dat != NULL) { free(conn->dat); conn->dat = NULL; }
			conn->dat = calloc(1, sizeof(CONNDATA));
			conn->dat->out_ContentLength = -1;
			conn->socket.mtime = time(NULL);
			conn->socket.ctime = time(NULL);
			http_dorequest(conn);
			k = 0;
			cobj = nsp_getobj(conn->N, &conn->N->g, "_HEADER");
			if (cobj->val->type == NT_TABLE) {
				if (strcasecmp("Keep-Alive", nsp_getstr(conn->N, cobj, "CONNECTION")) == 0) k = 1;
			}
			conn->N = nsp_endstate(conn->N);
			conn->state = 0;
			/* memset(conn->dat, 0, sizeof(CONNDATA)); */
			if (!k) break;
		}
#ifdef WIN32
	}
	__except (do_filter(GetExceptionInformation(), conn)) {
	}
#endif
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
	/* closeconnect() cleans up our mess for us */
	closeconnect(conn, 2);
	conn->socket.socket = -1;
	pthread_exit(0);
	return 0;
}

void *get_conn()
{
	obj_t *hobj = nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd");
	int i;
	int maxconn;

	pthread_mutex_lock(&ListenerMutex);
	maxconn = (int)nsp_getnum(proc->N, hobj, "max_connections");
	for (i = 0;; i++) {
		if (i > maxconn - 1) { msleep(5); i = 0; }
		if (htproc.conn[i].id == 0) break;
	}
	if (htproc.conn[i].dat != NULL) { free(htproc.conn[i].dat); htproc.conn[i].dat = NULL; }
	memset((char *)&htproc.conn[i], 0, sizeof(htproc.conn[i]));
#ifdef WIN32
	htproc.conn[i].id = (unsigned int)1;
#else
	htproc.conn[i].id = (pthread_t)1;
#endif
	pthread_mutex_unlock(&ListenerMutex);
	return &htproc.conn[i];
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *hobj, *cobj;
	unsigned short port;
	int i;
	int maxconn;
	int loaded = 0;

	memset((char *)&htproc, 0, sizeof(htproc));
	pthread_mutex_init(&ListenerMutex, NULL);
	proc = _proc;
	if (mod_import() != 0) return -1;
	htproc.N = nsp_newstate();
	hobj = nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd");
	cobj = nsp_getobj(proc->N, hobj, "interface");
	if (cobj->val->type != NT_STRING) cobj = nsp_setstr(proc->N, hobj, "interface", "INADDR_ANY", strlen("INADDR_ANY"));
	cobj = nsp_getobj(proc->N, hobj, "port");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "port", 4110);
	cobj = nsp_getobj(proc->N, hobj, "tls_port");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "tls_port", 4112);
	cobj = nsp_getobj(proc->N, hobj, "max_connections");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "max_connections", 50);
	maxconn = (int)nsp_getnum(proc->N, hobj, "max_connections");
	cobj = nsp_getobj(proc->N, hobj, "max_keepalive");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "max_keepalive", 15);
	cobj = nsp_getobj(proc->N, hobj, "max_idle");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "max_idle", 120);
	cobj = nsp_getobj(proc->N, hobj, "max_post_size");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "max_post_size", 32 * 1024 * 1024);
	cobj = nsp_getobj(proc->N, hobj, "session_limit");
	if (cobj->val->type != NT_NUMBER) cobj = nsp_setnum(proc->N, hobj, "session_limit", 1);
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Starting %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	port = (int)nsp_getnum(proc->N, hobj, "port");
	if (port) {
		if ((htproc.ListenSocketSTD.socket = tcp_bind(nsp_getstr(proc->N, hobj, "interface"), port)) != -1) loaded = 1;
	}
	else {
		htproc.ListenSocketSTD.socket = -1;
	}
	port = (int)nsp_getnum(proc->N, hobj, "tls_port");
	if ((proc->ssl_is_loaded) && (port)) {
		if ((htproc.ListenSocketSSL.socket = tcp_bind(nsp_getstr(proc->N, hobj, "interface"), port)) != -1) loaded = 1;
	}
	else {
		htproc.ListenSocketSSL.socket = -1;
	}
	if (loaded) {
		if ((htproc.conn = calloc(maxconn, sizeof(CONN))) == NULL) {
			printf("\r\nconn calloc(%d, %d) failed\r\n", maxconn, (int)sizeof(CONN));
			return -1;
		}
		for (i = 0; i < maxconn; i++) htproc.conn[i].socket.socket = -1;
		if (modules_init() != 0) exit(-2);
		return 0;
	}
	return -1;
}

DllExport int mod_exec()
{
	obj_t *hobj = nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd");

	if (nsp_getnum(proc->N, hobj, "port") && (htproc.ListenSocketSTD.socket != -1)) {
		addlistener(MODSHORTNAME, &htproc.ListenSocketSTD, (void *)get_conn, (void *)htloop, 0);
	}
	if (nsp_getnum(proc->N, hobj, "tls_port") && (htproc.ListenSocketSSL.socket != -1) && (proc->ssl_is_loaded)) {
		addlistener(MODSHORTNAME, &htproc.ListenSocketSSL, (void *)get_conn, (void *)htloop, 1);
	}
	return 0;
}

DllExport int mod_cron()
{
	obj_t *hobj = nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd");

	int maxconn = (int)nsp_getnum(proc->N, hobj, "max_connections");
	int maxkeepalive = (int)nsp_getnum(proc->N, hobj, "max_keepalive");
	int maxidle = (int)nsp_getnum(proc->N, hobj, "max_idle");
	time_t ctime = time(NULL);
	short int connections = 0;
	short int i;

	if ((htproc.ListenSocketSTD.socket == -1) && (htproc.ListenSocketSSL.socket == -1)) return 0;
	for (i = 0; i < maxconn; i++) {
		if ((htproc.conn[i].id == 0) || (htproc.conn[i].socket.mtime == 0)) continue;
		connections++;
		if (htproc.conn[i].state == 0) {
			if (ctime - htproc.conn[i].socket.mtime < maxkeepalive) continue;
		}
		else {
			if (ctime - htproc.conn[i].socket.mtime < maxidle) continue;
		}

		if (ctime - htproc.conn[i].socket.mtime > 59) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Reaping idle socket [%s:%d] (idle %d seconds)", htproc.conn[i].socket.RemoteAddr, htproc.conn[i].socket.RemotePort, ctime - htproc.conn[i].socket.mtime);
		}
		/* closeconnect is _not_ ssl-friendly */
/*
		closeconnect(&htproc.conn[i], 0);
		if (&htproc.conn[i].dat!=NULL) {
			if (&htproc.conn[i].dat->wm!=NULL) {
				tcp_close(&htproc.conn[i].dat->wm->socket, 0);
			}
		}
*/
		/* TODO: this doesn't seem to do anything - check later, comment out log_error for now */
		tcp_close(&htproc.conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Stopping %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	htproc.N = nsp_endstate(htproc.N);
	if (htproc.ListenSocketSTD.socket > 0) {
		/*		shutdown(ListenSocket, 2);
				closesocket(ListenSocket);
		*/
		htproc.ListenSocketSTD.socket = 0;
	}
	if (htproc.ListenSocketSSL.socket > 0) {
		/*
				shutdown(ListenSocket, 2);
				closesocket(ListenSocket);
		*/
		htproc.ListenSocketSSL.socket = 0;
	}
	return 0;
}
