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
#include "pop3d_main.h"

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

int closeconnect(CONN *conn)
{
	if (conn != NULL) {
		tcp_close(&conn->socket, 1);
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
		if (conn->dat != NULL) {
			free(conn->dat);
			conn->dat = NULL;
		}
#ifdef WIN32
		CloseHandle(conn->handle);
#endif
		memset(conn, 0, sizeof(conn[0]));
		conn->socket.socket = -1;
#ifdef WIN32
		_endthread();
#else
		pthread_exit(0);
#endif
	}
	return 0;
}

#ifdef WIN32
unsigned _stdcall poploop(void *x)
#else
void *poploop(void *x)
#endif
{
	CONN *conn = x;

	log_error(proc->N, "core", __FILE__, __LINE__, 2, "Starting poploop() thread");
	conn->id = pthread_self();
#ifndef WIN32
	pthread_detach(conn->id);
#endif
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Opening connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
	proc->stats.pop3_conns++;
	if (conn->dat != NULL) free(conn->dat);
	conn->dat = calloc(1, sizeof(CONNDATA));
	conn->socket.mtime = time(NULL);
	conn->socket.ctime = time(NULL);
	strncpy(conn->dat->RemoteAddr, conn->socket.RemoteAddr, sizeof(conn->dat->RemoteAddr) - 1);
	conn->dat->RemotePort = conn->socket.RemotePort;
	pop3_dorequest(conn);
	conn->state = 0;
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
	/* closeconnect() cleans up our mess for us */
	closeconnect(conn);
	conn->socket.socket = -1;
	pthread_exit(0);
	return 0;
}

void *get_conn()
{
	int i;

	pthread_mutex_lock(&ListenerMutex);

	for (i = 0;;i++) {
		if (i > mod_config.pop3_maxconn - 1) { msleep(5); i = 0; }
		if (conn[i].id == 0) break;
	}
	if (conn[i].dat != NULL) { free(conn[i].dat); conn[i].dat = NULL; }
	memset((char *)&conn[i], 0, sizeof(conn[i]));
#ifdef WIN32
	conn[i].id = (unsigned int)1;
#else
	conn[i].id = (pthread_t)1;
#endif
	pthread_mutex_unlock(&ListenerMutex);
	return &conn[i];
}

DllExport int mod_init(_PROC *_proc)
{
	int loaded = 0;
	int i;

	memset((char *)&ListenSocketSTD, 0, sizeof(ListenSocketSTD));
	memset((char *)&ListenSocketSSL, 0, sizeof(ListenSocketSSL));
	pthread_mutex_init(&ListenerMutex, NULL);
	proc = _proc;
	//	config=&proc->config;
	//	functions=_functions;
	if (mod_import() != 0) return -1;
	conf_read();
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Starting %s pop3d %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (mod_config.pop3_port) {
		if ((ListenSocketSTD.socket = tcp_bind(mod_config.pop3_interface, mod_config.pop3_port)) != -1) loaded = 1;
	}
	else {
		ListenSocketSTD.socket = -1;
	}
	if ((proc->ssl_is_loaded) && (mod_config.pop3_sslport)) {
		if ((ListenSocketSSL.socket = tcp_bind(mod_config.pop3_interface, mod_config.pop3_sslport)) != -1) loaded = 1;
	}
	else {
		ListenSocketSSL.socket = -1;
	}
	if (loaded) {
		if ((conn = calloc(mod_config.pop3_maxconn, sizeof(CONN))) == NULL) {
			printf("\r\nconn calloc(%d, %d) failed\r\n", mod_config.pop3_maxconn, (int)sizeof(CONN));
			return -1;
		}
		for (i = 0;i < mod_config.pop3_maxconn;i++) conn[i].socket.socket = -1;
		return 0;
	}
	return -1;
}

DllExport int mod_exec()
{
	if ((mod_config.pop3_port) && (ListenSocketSTD.socket != -1)) {
		addlistener("pop3d", &ListenSocketSTD, get_conn, poploop, 0);
	}
	if ((mod_config.pop3_sslport) && (ListenSocketSSL.socket != -1) && (proc->ssl_is_loaded)) {
		addlistener("pop3d", &ListenSocketSSL, get_conn, poploop, 1);
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime = time(NULL);
	short int connections = 0;
	short int i;

	if ((ListenSocketSTD.socket == -1) && (ListenSocketSSL.socket == -1)) return 0;
	for (i = 0;i < mod_config.pop3_maxconn;i++) {
		if ((conn[i].id == 0) || (conn[i].socket.mtime == 0)) continue;
		connections++;
		if (conn[i].state == 0) {
			if (ctime - conn[i].socket.mtime < 15) continue;
		}
		else {
			if (ctime - conn[i].socket.mtime < mod_config.pop3_maxidle) continue;
		}
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Reaping idle socket [%s:%d] (idle %d seconds)", conn[i].socket.RemoteAddr, conn[i].socket.RemotePort, ctime - conn[i].socket.mtime);
		tcp_close(&conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Stopping %s pop3d %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (ListenSocketSTD.socket > 0) {
		/*		shutdown(ListenSocket, 2);
				closesocket(ListenSocket);
		*/
		ListenSocketSTD.socket = 0;
	}
	if (ListenSocketSSL.socket > 0) {
		/*
				shutdown(ListenSocket, 2);
				closesocket(ListenSocket);
		*/
		ListenSocketSSL.socket = 0;
	}
	return 0;
}
