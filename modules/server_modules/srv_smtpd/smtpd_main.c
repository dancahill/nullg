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
#include "smtpd_main.h"
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

int closeconnect(CONN *sid)
{
	DEBUG_IN(sid, "closeconnect()");
	if (sid!=NULL) {
		tcp_close(&sid->socket, 1);
		log_error("smtpd", __FILE__, __LINE__, 4, "Closing connection [%u]", sid->socket.socket);
		if (sid->dat!=NULL) {
			free(sid->dat);
			sid->dat=NULL;
		}
#ifdef WIN32
		CloseHandle(sid->handle);
#endif
		DEBUG_OUT(sid, "closeconnect()");
		memset(sid, 0, sizeof(conn[0]));
		sid->socket.socket=-1;
#ifdef WIN32
		_endthread();
#else
		pthread_exit(0);
#endif
	}
	DEBUG_OUT(sid, "closeconnect()");
	return 0;
}

#ifdef WIN32
unsigned _stdcall smtploop(void *x)
#else
void *smtploop(void *x)
#endif
{
	struct sockaddr_in peer;
	unsigned int fromlen=sizeof(struct sockaddr_in);
	int sid=(int)x;

	DEBUG_IN(NULL, "smtploop()");
	log_error("core", __FILE__, __LINE__, 2, "Starting smtploop() thread");
	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	log_error("smtpd", __FILE__, __LINE__, 4, "Opening connection thread [%u]", conn[sid].socket);
	proc->stats.smtp_conns++;
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	conn[sid].socket.atime=time(NULL);
	conn[sid].socket.ctime=time(NULL);
	getpeername(conn[sid].socket.socket, (struct sockaddr *)&peer, &fromlen);
	strncpy(conn[sid].dat->user_RemoteAddr, inet_ntoa(peer.sin_addr), sizeof(conn[sid].dat->user_RemoteAddr)-1);
	smtp_dorequest(&conn[sid]);
	conn[sid].state=0;
	log_error("smtpd", __FILE__, __LINE__, 4, "Closing connection thread [%u]", conn[sid].socket);
	/* closeconnect() cleans up our mess for us */
	closeconnect(&conn[sid]);
	conn[sid].socket.socket=-1;
	DEBUG_OUT(NULL, "smtploop()");
	pthread_exit(0);
	return 0;
}

static int get_conn()
{
	int i;

	pthread_mutex_lock(&ListenerMutex);
	for (i=0;;i++) {
		if (i>=mod_config.smtp_maxconn) {
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

#define CONN_STD 1
#define CONN_SSL 2

#ifdef WIN32
unsigned _stdcall smtpd_accept_loop(void *x)
#else
void *smtpd_accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int conntype=(int)x;
	int sid;
	int ListenSocket;
	short int socket;

	DEBUG_IN(NULL, "smtpd_accept_loop()");
	log_error("core", __FILE__, __LINE__, 1, "Starting smtpd_accept_loop() thread (%s)", (conntype==CONN_SSL?"ssl":"std"));
#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("smtpd", __FILE__, __LINE__, 0, "pthread_attr_init()");
		DEBUG_OUT(NULL, "smtpd_accept_loop()");
		exit(-1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(-1);
#endif
	for (;;) {
		if ((sid=get_conn())<0) continue;
		if (conntype==CONN_STD) {
			ListenSocket=ListenSocketSTD;
		} else if (conntype==CONN_SSL) {
			ListenSocket=ListenSocketSSL;
		} else {
			log_error("smtpd", __FILE__, __LINE__, 0, "unknown connection type %d", conntype);
			exit(-1);
		}
		socket=tcp_accept(ListenSocket, (struct sockaddr_in *)&conn[sid].socket.ClientAddr);
		/*
		 * If ListenSocket==-1 then someone either has, or soon will kill the listener,
  		 * probably from server_shutdown().
  		 */
		if (ListenSocket==-1) return 0;
		conn[sid].socket.socket=socket;
#ifdef WIN32
		if (conn[sid].socket.socket==INVALID_SOCKET) {
			log_error("smtpd", __FILE__, __LINE__, 2, "smtpd_accept_loop() shutting down...");
			DEBUG_OUT(NULL, "smtpd_accept_loop()");
			return 0;
#else
		if (conn[sid].socket.socket<0) {
			continue;
#endif
		} else {
			if (conntype==CONN_SSL) ssl_accept(&conn[sid].socket);
			if (pthread_create(&conn[sid].handle, &thr_attr, smtploop, (void *)sid)==-1) {
				log_error("smtpd", __FILE__, __LINE__, 0, "smtploop() failed...");
				DEBUG_OUT(NULL, "smtpd_accept_loop()");
				exit(-1);
			}
		}
	}
	DEBUG_OUT(NULL, "smtpd_accept_loop()");
	return 0;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int i;

	ListenSocketSTD=0;
	ListenSocketSSL=0;
	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	log_error("core", __FILE__, __LINE__, 1, "Starting %s smtpd %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (mod_config.smtp_port) {
		if ((ListenSocketSTD=tcp_bind(mod_config.smtp_interface, mod_config.smtp_port))<0) return -1;
	}
	if ((proc->ssl_is_loaded)&&(mod_config.smtp_sslport)) {
		if ((ListenSocketSSL=tcp_bind(mod_config.smtp_interface, mod_config.smtp_sslport))<0) return -1;
	}
	if ((conn=calloc(mod_config.smtp_maxconn, sizeof(CONN)))==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", mod_config.smtp_maxconn, sizeof(CONN));
		return -1;
	}
	for (i=0;i<mod_config.smtp_maxconn;i++) conn[i].socket.socket=-1;
	return 0;
}

DllExport int mod_exec()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if (mod_config.smtp_port) {
		if (pthread_create(&ListenThreadSTD, &thr_attr, smtpd_accept_loop, (void *)CONN_STD)==-1) {
			log_error("smtpd", __FILE__, __LINE__, 0, "smtp_accept_loop() failed...");
			return -2;
		}
	}
	if ((proc->ssl_is_loaded)&&(mod_config.smtp_sslport)) {
		if (pthread_create(&ListenThreadSSL, &thr_attr, smtpd_accept_loop, (void *)CONN_SSL)==-1) {
			log_error("smtpd", __FILE__, __LINE__, 0, "smtp_accept_loop_ssl() failed...");
			return -2;
		}
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	for (i=0;i<mod_config.smtp_maxconn;i++) {
		if ((conn[i].id==0)||(conn[i].socket.atime==0)) continue;
		connections++;
		if (conn[i].state==0) {
			if (ctime-conn[i].socket.atime<15) continue;
		} else {
			if (ctime-conn[i].socket.atime<mod_config.smtp_maxidle) continue;
		}
		log_error("smtpd", __FILE__, __LINE__, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].socket.atime);
		tcp_close(&conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error("core", __FILE__, __LINE__, 1, "Stopping %s smtpd %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (ListenSocketSTD>0) {
/*
		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		ListenSocketSTD=0;
	}
	if (ListenSocketSSL>0) {
/*
		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		ListenSocketSSL=0;
	}
	return 0;
}
