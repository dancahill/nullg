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
unsigned _stdcall htloop(void *x)
#else
void *htloop(void *x)
#endif
{
	int sid=(int)x;
//	int lowat=1;

	DEBUG_IN(NULL, "htloop()");
	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
//	setsockopt(conn[sid].socket, SOL_SOCKET, SO_RCVLOWAT, (void *)&lowat, sizeof(lowat));
	log_error("http", __FILE__, __LINE__, 4, "Opening connection thread [%u]", conn[sid].socket);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	proc->stats.http_conns++;
	for (;;) {
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		conn[sid].PostData=NULL;
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		conn[sid].dat=calloc(1, sizeof(CONNDATA));
		conn[sid].dat->out_ContentLength=-1;
		conn[sid].socket.atime=time(NULL);
		conn[sid].socket.ctime=time(NULL);
		http_dorequest(&conn[sid]);
		conn[sid].dat->out_bodydone=1;
		flushbuffer(&conn[sid]);
		log_access("http", "%s \"%s %s %s\" %d %d \"%s\"", conn[sid].dat->in_RemoteAddr, conn[sid].dat->in_RequestMethod, conn[sid].dat->in_RequestURI, conn[sid].dat->in_Protocol, conn[sid].dat->out_status, conn[sid].dat->out_bytecount, conn[sid].dat->in_UserAgent);
		conn[sid].state=0;
		if (conn[sid].dat->wm!=NULL) {
			tcp_close(&conn[sid].dat->wm->socket);
			free(conn[sid].dat->wm);
			conn[sid].dat->wm=NULL;
		}
//		memset(conn[sid].dat, 0, sizeof(CONNDATA));
		if (p_strcasestr(conn[sid].dat->out_Connection, "Keep-Alive")==NULL) {
			break;
		}
	}
	log_error("http", __FILE__, __LINE__, 4, "Closing connection thread [%u]", conn[sid].socket.socket);
	closeconnect(&conn[sid], 2);
	conn[sid].socket.socket=-1;
	// closeconnect() cleans up our mess for us
	DEBUG_OUT(NULL, "htloop()");
	pthread_exit(0);
	return 0;
}

/****************************************************************************
 *	accept_loop()
 *
 *	Purpose	: Function to handle incoming socket connections
 *	Args	: None
 *	Returns	: void
 *	Notes	: Created as a thread in Win32
 ***************************************************************************/
#ifdef WIN32
unsigned _stdcall http_accept_loop(void *x)
#else
void *http_accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int i;
	short int socket;
#ifdef LINUX
	char cmdline[255];
#endif

	DEBUG_IN(NULL, "http_accept_loop()");
#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("http", __FILE__, __LINE__, 0, "pthread_attr_init()");
		DEBUG_OUT(NULL, "http_accept_loop()");
		exit(-1);
	}
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(-1);
#endif
	// UNGODLY DEBUGGING FOR LINUX
#ifdef LINUX
	if (config->server_loglevel==42) {
		snprintf(cmdline, sizeof(cmdline)-1, "strace -f -ff -s 64 -p %d -o %s/trace-%d &", getpid()-1, config->server_dir_var_tmp, getpid()-1);
		system(cmdline);
	}
#endif
	for (;;) {
		for (i=0;;i++) {
			if (i>=config->http_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].id==0) break;
		}
		if (conn[i].PostData!=NULL) { free(conn[i].PostData); conn[i].PostData=NULL; }
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		socket=tcp_accept(http_proc.ListenSocket, (struct sockaddr *)&conn[i].socket.ClientAddr);
		/*
		 * If ListenSocket==-1 then someone either has, or soon will kill the listener,
		 * probably from server_shutdown().
		 */
		if (http_proc.ListenSocket==-1) return 0;
		conn[i].socket.socket=socket;
#ifdef WIN32
		if (conn[i].socket.socket==INVALID_SOCKET) {
			DEBUG_OUT(NULL, "http_accept_loop()");
			return 0;
//			log_error("http", __FILE__, __LINE__, 0, "accept() failed...");
//			closesocket(proc.ListenSocket);
//			WSACleanup();
//			DEBUG_OUT(NULL, "http_accept_loop()");
//			exit(-1);
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
			if (pthread_create(&conn[i].handle, &thr_attr, htloop, (void *)i)==-1) {
				log_error("http", __FILE__, __LINE__, 0, "htloop() failed to start...");
				DEBUG_OUT(NULL, "http_accept_loop()");
				exit(-1);
			}
		}
	}
	DEBUG_OUT(NULL, "http_accept_loop()");
	return 0;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int i;

	http_proc.ListenSocket=0;
	http_proc.ListenSocketSSL=0;
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	log_error("http", __FILE__, __LINE__, 1, "Starting %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	if (config->http_port) {
		if ((http_proc.ListenSocket=tcp_bind(config->http_hostname, config->http_port))<0) return -1;
	}
	if ((conn=calloc(config->http_maxconn, sizeof(CONN)))==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", config->http_maxconn, sizeof(CONN));
		return -1;
	}
	for (i=0;i<config->http_maxconn;i++) conn[i].socket.socket=-1;
	if (modules_init()!=0) exit(-2);
	return 0;
}

DllExport int mod_exec()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if (pthread_create(&http_proc.ListenThread, &thr_attr, http_accept_loop, NULL)==-1) {
		log_error(NULL, __FILE__, __LINE__, 0, "http_accept_loop() failed...");
		return -2;
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	for (i=0;i<config->http_maxconn;i++) {
		if ((conn[i].id==0)||(conn[i].socket.atime==0)) continue;
		connections++;
		if (conn[i].state==0) {
			if (ctime-conn[i].socket.atime<15) continue;
		} else {
			if (ctime-conn[i].socket.atime<config->http_maxidle) continue;
		}
		log_error("http", __FILE__, __LINE__, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].socket.atime);
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
//	log_access("http", 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	log_error("http", __FILE__, __LINE__, 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
//	pthread_kill(proc.ListenThread, 14);
//	shutdown(proc.ListenSocket, 2);
//	closesocket(proc.ListenSocket);
//	proc.ListenSocket=-1;
	opensockets=0;
	for (i=0;i<config->http_maxconn;i++) {
		if (conn[i].socket.socket!=-1) opensockets++;
	}
	if (!opensockets) goto fini;
	sleep(2);
	for (i=0;i<config->http_maxconn;i++) {
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

	if (http_proc.ListenSocket>0) {
//		shutdown(ListenSocket, 2);
//		closesocket(ListenSocket);
		http_proc.ListenSocket=0;
	}
	if (http_proc.ListenSocketSSL>0) {
//		shutdown(ListenSocket, 2);
//		closesocket(ListenSocket);
		http_proc.ListenSocketSSL=0;
	}
	return 0;
}
