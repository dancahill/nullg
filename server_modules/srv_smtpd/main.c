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
unsigned _stdcall smtploop(void *x)
#else
void *smtploop(void *x)
#endif
{
	int sid=(int)x;

	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	log_error("smtp", __FILE__, __LINE__, 4, "Opening connection thread [%u]", conn[sid].socket);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	proc->stats.smtp_conns++;
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	conn[sid].socket.atime=time(NULL);
	conn[sid].socket.ctime=time(NULL);
	smtp_dorequest(&conn[sid]);
	conn[sid].state=0;
	log_error("smtp", __FILE__, __LINE__, 4, "Closing connection thread [%u]", conn[sid].socket);
	tcp_close(&conn[sid].socket);
	conn[sid].socket.socket=-1;
	pthread_exit(0);
	return 0;
}

#ifdef WIN32
unsigned _stdcall smtp_accept_loop(void *x)
#else
void *smtp_accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int i;

#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("smtp", __FILE__, __LINE__, 0, "pthread_attr_init()");
		exit(1);
	}
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	for (;;) {
		for (i=0;;i++) {
			if (i>=config->smtp_maxconn) {
				msleep(50);
				i=0;
				continue;
			}
			if (conn[i].id==0) break;
		}
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		conn[i].socket.socket=tcp_accept(ListenSocket, (struct sockaddr *)&conn[i].socket.ClientAddr);
#ifdef WIN32
		if (conn[i].socket.socket==INVALID_SOCKET) {
			return 0;
//			log_error("smtp", __FILE__, __LINE__, 0, "accept() failed...");
//			closesocket(proc.ListenSocket);
//			WSACleanup();
//			exit(0);
#else
		if (conn[i].socket.socket<0) {
			continue;
#endif
		} else {
#ifdef WIN32
			conn[i].id=(unsigned int)1;
#else
			conn[i].id=(pthread_t)1;
#endif
			if (pthread_create(&conn[i].handle, &thr_attr, smtploop, (void *)i)==-1) {
				log_error("smtp", __FILE__, __LINE__, 0, "smtploop() failed...");
				exit(0);
			}
		}
	}
	return 0;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int i;

	ListenSocket=0;
	ListenSocketSSL=0;
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	log_error("core", __FILE__, __LINE__, 1, "Starting %s smtpd %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (config->smtp_port) {
		if ((ListenSocket=tcp_bind(config->smtp_hostname, config->smtp_port))<0) return -1;
	}
	if ((conn=calloc(config->smtp_maxconn, sizeof(CONN)))==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", config->smtp_maxconn, sizeof(CONN));
		return -1;
	}
	for (i=0;i<config->smtp_maxconn;i++) conn[i].socket.socket=-1;
	return 0;
}

DllExport int mod_exec()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if (pthread_create(&ListenThread, &thr_attr, smtp_accept_loop, NULL)==-1) {
		log_error(NULL, __FILE__, __LINE__, 0, "smtp_accept_loop() failed...");
		return -2;
	}
	if (pthread_attr_init(&thr_attr)) exit(1);
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	if (pthread_create(&SpoolThread, &thr_attr, smtp_spool, NULL)==-1) {
		log_error(NULL, __FILE__, __LINE__, 0, "smtp_spool() loop failed to start.");
		exit(0);
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	for (i=0;i<config->smtp_maxconn;i++) {
		if ((conn[i].id==0)||(conn[i].socket.atime==0)) continue;
		connections++;
		if (conn[i].state==0) {
			if (ctime-conn[i].socket.atime<15) continue;
		} else {
			if (ctime-conn[i].socket.atime<config->smtp_maxidle) continue;
		}
		log_error("smtp", __FILE__, __LINE__, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].socket.atime);
//		closeconnect(&conn[i], 0);
		tcp_close(&conn[i].socket);
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
//	log_access("smtp", 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	log_error("core", __FILE__, __LINE__, 1, "Stopping %s smtpd %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
//	pthread_kill(proc.ListenThread, 14);
//	shutdown(proc.ListenSocket, 2);
//	closesocket(proc.ListenSocket);
//	proc.ListenSocket=-1;
	opensockets=0;
	for (i=0;i<config->smtp_maxconn;i++) {
		if (conn[i].socket.socket!=-1) opensockets++;
	}
	if (!opensockets) goto fini;
	sleep(2);
	for (i=0;i<config->smtp_maxconn;i++) {
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
