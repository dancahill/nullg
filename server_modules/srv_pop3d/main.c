/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "main.h"
#ifndef WIN32
#include <pwd.h>
#endif

#ifdef WIN32
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
#define pthread_exit(A) _endthread()
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
int pthread_kill(pthread_t handle, int sig);

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
#endif

#ifdef WIN32
unsigned _stdcall poploop(void *x)
#else
void *poploop(void *x)
#endif
{
	int sid=(int)x;

	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	log_error("pop3", __FILE__, __LINE__, 4, "Opening connection thread [%u]", conn[sid].socket.socket);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	proc->stats.pop3_conns++;
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	conn[sid].socket.atime=time(NULL);
	conn[sid].socket.ctime=time(NULL);
	pop3_dorequest(&conn[sid]);
	conn[sid].state=0;
	log_error("pop3", __FILE__, __LINE__, 4, "Closing connection thread [%u]", conn[sid].socket.socket);
	tcp_close(&conn[sid].socket, 1);
	conn[sid].socket.socket=-1;
	pthread_exit(0);
	return 0;
}

static int get_conn()
{
	int i;

	pthread_mutex_lock(&ListenerMutex);
	for (i=0;;i++) {
		if (i>=config->http_maxconn) {
			msleep(10);
			i=0;
			continue;
		}
		if (conn[i].id==0) {
			break;
		}
	}
	if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
	memset((char *)&conn[i], 0, sizeof(conn[i]));
#ifdef WIN32
	conn[i].id=(unsigned int)1;
#else
	conn[i].id=(pthread_t)1;
#endif
	pthread_mutex_unlock(&ListenerMutex);
	return i;
}

#ifdef WIN32
unsigned _stdcall pop3_accept_loop(void *x)
#else
void *pop3_accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int sid;
	short int socket;

#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("pop3", __FILE__, __LINE__, 0, "pthread_attr_init()");
		exit(1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	for (;;) {
		if ((sid=get_conn())<0) continue;
		socket=tcp_accept(ListenSocket, (struct sockaddr *)&conn[sid].socket.ClientAddr);
		/*
		 * If ListenSocket==-1 then someone either has, or soon will kill the listener,
 		 * probably from server_shutdown().
 		 */
 		if (ListenSocket==-1) return 0;
		conn[sid].socket.socket=socket;
#ifdef WIN32
		if (conn[sid].socket.socket==INVALID_SOCKET) {
			log_error("pop3", __FILE__, __LINE__, 2, "pop3_accept_loop() shutting down...");
			return 0;
#else
		if (conn[sid].socket.socket<0) {
			continue;
#endif
		} else {
			if (pthread_create(&conn[sid].handle, &thr_attr, poploop, (void *)sid)==-1) {
				log_error("pop3", __FILE__, __LINE__, 0, "poploop() failed...");
				exit(0);
			}
		}
	}
	return 0;
}

#ifdef HAVE_LIBSSL
#ifdef WIN32
unsigned _stdcall pop3_accept_loop_ssl(void *x)
#else
void *pop3_accept_loop_ssl(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int sid;
	short int socket;

#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("pop3", __FILE__, __LINE__, 0, "pthread_attr_init()");
		exit(1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	for (;;) {
		if ((sid=get_conn())<0) continue;
		socket=tcp_accept(ListenSocketSSL, (struct sockaddr *)&conn[sid].socket.ClientAddr);
		/*
		 * If ListenSocketSSL==-1 then someone either has, or soon will kill the listener,
 		 * probably from server_shutdown().
 		 */
 		if (ListenSocketSSL==-1) return 0;
		conn[sid].socket.socket=socket;
#ifdef WIN32
		if (conn[i].socket.socket==INVALID_SOCKET) {
			log_error("pop3", __FILE__, __LINE__, 2, "pop3_accept_loop_ssl() shutting down...");
			return 0;
#else
		if (conn[sid].socket.socket<0) {
			continue;
#endif
		} else {
			ssl_accept(&conn[sid].socket);
			if (pthread_create(&conn[sid].handle, &thr_attr, poploop, (void *)sid)==-1) {
				log_error("pop3", __FILE__, __LINE__, 0, "poploop() failed...");
				exit(0);
			}
		}
	}
	return 0;
}
#endif // HAVE_LIBSSL

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int i;

	ListenSocket=0;
	ListenSocketSSL=0;
	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	log_error("core", __FILE__, __LINE__, 1, "Starting %s pop3d %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (config->pop3_port) {
		if ((ListenSocket=tcp_bind(config->pop3_hostname, config->pop3_port))<0) return -1;
	}
#ifdef HAVE_LIBSSL
	if ((proc->ssl_is_loaded)&&(config->pop3_port_ssl)) {
		if ((ListenSocketSSL=tcp_bind(config->http_hostname, config->pop3_port_ssl))<0) return -1;
	}
#endif
	if ((conn=calloc(config->pop3_maxconn, sizeof(CONN)))==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", config->pop3_maxconn, sizeof(CONN));
		return -1;
	}
	for (i=0;i<config->pop3_maxconn;i++) conn[i].socket.socket=-1;
	return 0;
}

DllExport int mod_exec()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if (config->pop3_port) {
		if (pthread_create(&ListenThread, &thr_attr, pop3_accept_loop, NULL)==-1) {
			log_error("pop3", __FILE__, __LINE__, 0, "pop3_accept_loop() failed...");
			return -2;
		}
	}
#ifdef HAVE_LIBSSL
	if ((proc->ssl_is_loaded)&&(config->pop3_port_ssl)) {
		if (pthread_create(&ListenThreadSSL, &thr_attr, pop3_accept_loop_ssl, NULL)==-1) {
			log_error("pop3", __FILE__, __LINE__, 0, "pop3_accept_loop_ssl() failed...");
			return -2;
		}
	}
#endif
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	for (i=0;i<config->pop3_maxconn;i++) {
		if ((conn[i].id==0)||(conn[i].socket.atime==0)) continue;
		connections++;
		if (conn[i].state==0) {
			if (ctime-conn[i].socket.atime<15) continue;
		} else {
			if (ctime-conn[i].socket.atime<config->pop3_maxidle) continue;
		}
		log_error("pop3", __FILE__, __LINE__, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].socket.atime);
//		closeconnect(&conn[i], 0);
		tcp_close(&conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
/*
void server_shutdown()
{
	int i;
	int opensockets;

#ifndef WIN32
	if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
#endif
//	log_access("pop3", 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	log_error("core", __FILE__, __LINE__, 1, "Stopping %s pop3d %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
//	pthread_kill(proc.ListenThread, 14);
//	shutdown(proc.ListenSocket, 2);
//	closesocket(proc.ListenSocket);
//	proc.ListenSocket=-1;
	opensockets=0;
	for (i=0;i<config->pop3_maxconn;i++) {
		if (conn[i].socket.socket!=-1) opensockets++;
	}
	if (!opensockets) goto fini;
	sleep(2);
	for (i=0;i<config->pop3_maxconn;i++) {
		if (conn[i].id==0) continue;
//		pthread_kill(conn[i].handle, 14);
		if (conn[i].socket.socket!=-1) {
			shutdown(conn[i].socket.socket, 2);
			closesocket(conn[i].socket.socket);
			conn[i].socket.socket=-1;
		}
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
	}
fini:
	free(conn);
}
*/

	if (ListenSocket>0) {
//		shutdown(ListenSocket, 2);
//		closesocket(ListenSocket);
		ListenSocket=0;
	}
	if (ListenSocketSSL>0) {
//		shutdown(ListenSocket, 2);
//		closesocket(ListenSocket);
		ListenSocketSSL=0;
	}
	return 0;
}
