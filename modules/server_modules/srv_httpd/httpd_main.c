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
#include "httpd_main.h"
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
	struct sockaddr_in peer;
	unsigned int fromlen=sizeof(struct sockaddr_in);
	int sid=(int)x;
	/* int lowat=1; */

	DEBUG_IN(NULL, "htloop()");
	log_error("core", __FILE__, __LINE__, 2, "Starting htloop() thread");
	http_proc.conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(http_proc.conn[sid].id);
#endif
	/* setsockopt(http_proc.conn[sid].socket, SOL_SOCKET, SO_RCVLOWAT, (void *)&lowat, sizeof(lowat)); */
	log_error("httpd", __FILE__, __LINE__, 4, "Opening connection thread [%u]", http_proc.conn[sid].socket.socket);
	proc->stats.http_conns++;
	for (;;) {
		if (http_proc.conn[sid].PostData!=NULL) { free(http_proc.conn[sid].PostData); http_proc.conn[sid].PostData=NULL; }
		if (http_proc.conn[sid].dat!=NULL) free(http_proc.conn[sid].dat);
		http_proc.conn[sid].dat=calloc(1, sizeof(CONNDATA));
		http_proc.conn[sid].dat->out_ContentLength=-1;
		http_proc.conn[sid].socket.atime=time(NULL);
		http_proc.conn[sid].socket.ctime=time(NULL);
		getpeername(http_proc.conn[sid].socket.socket, (struct sockaddr *)&peer, &fromlen);
		strncpy(http_proc.conn[sid].dat->in_RemoteAddr, inet_ntoa(peer.sin_addr), sizeof(http_proc.conn[sid].dat->in_RemoteAddr)-1);
		http_proc.conn[sid].dat->in_RemotePort=ntohs(http_proc.conn[sid].socket.ClientAddr.sin_port);
		if (http_proc.conn[sid].socket.ssl!=NULL) {
			http_proc.conn[sid].dat->in_ServerPort=http_proc.config.http_sslport;
		} else {
			http_proc.conn[sid].dat->in_ServerPort=http_proc.config.http_port;
		}

		http_dorequest(&http_proc.conn[sid]);
		http_proc.conn[sid].dat->out_bodydone=1;
		flushbuffer(&http_proc.conn[sid]);
		log_htaccess(&http_proc.conn[sid]);
		http_proc.conn[sid].state=0;
		if (http_proc.conn[sid].dat->wm!=NULL) {
			tcp_close(&http_proc.conn[sid].dat->wm->socket, 1);
			free(http_proc.conn[sid].dat->wm);
			http_proc.conn[sid].dat->wm=NULL;
		}
		/* memset(http_proc.conn[sid].dat, 0, sizeof(CONNDATA)); */
		if (p_strcasestr(http_proc.conn[sid].dat->out_Connection, "Keep-Alive")==NULL) {
			break;
		}
	}
	log_error("httpd", __FILE__, __LINE__, 4, "Closing connection thread [%u]", http_proc.conn[sid].socket.socket);
	/* closeconnect() cleans up our mess for us */
	closeconnect(&http_proc.conn[sid], 2);
	http_proc.conn[sid].socket.socket=-1;
	DEBUG_OUT(NULL, "htloop()");
	pthread_exit(0);
	return 0;
}

static int get_conn()
{
	int i;

	pthread_mutex_lock(&ListenerMutex);
	for (i=0;;i++) {
		if (i>=http_proc.config.http_maxconn) {
			msleep(10);
			i=0;
			continue;
		}
		if (http_proc.conn[i].id==0) {
			break;
		}
	}
	if (http_proc.conn[i].PostData!=NULL) { free(http_proc.conn[i].PostData); http_proc.conn[i].PostData=NULL; }
	if (http_proc.conn[i].dat!=NULL) { free(http_proc.conn[i].dat); http_proc.conn[i].dat=NULL; }
	memset((char *)&http_proc.conn[i], 0, sizeof(http_proc.conn[i]));
#ifdef WIN32
	http_proc.conn[i].id=(unsigned int)1;
#else
	http_proc.conn[i].id=(pthread_t)1;
#endif
	pthread_mutex_unlock(&ListenerMutex);
	return i;
}

#define CONN_STD 1
#define CONN_SSL 2

#ifdef WIN32
unsigned _stdcall httpd_accept_loop(void *x)
#else
void *httpd_accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int conntype=(int)x;
	int sid;
	int ListenSocket;
	short int socket;

	DEBUG_IN(NULL, "httpd_accept_loop()");
	log_error("core", __FILE__, __LINE__, 1, "Starting httpd_accept_loop() thread (%s)", (conntype==CONN_SSL?"ssl":"std"));
#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error("httpd", __FILE__, __LINE__, 0, "pthread_attr_init()");
		DEBUG_OUT(NULL, "httpd_accept_loop()");
		exit(-1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(-1);
#endif
	for (;;) {
		if ((sid=get_conn())<0) continue;
		if (conntype==CONN_STD) {
			ListenSocket=http_proc.ListenSocketSTD;
		} else if (conntype==CONN_SSL) {
			ListenSocket=http_proc.ListenSocketSSL;
		} else {
			log_error("httpd", __FILE__, __LINE__, 0, "unknown connection type %d", conntype);
			exit(-1);
		}
		socket=tcp_accept(ListenSocket, (struct sockaddr_in *)&http_proc.conn[sid].socket.ClientAddr);
		/*
		 * If ListenSocket==-1 then someone either has, or soon will kill the listener,
		 * probably from server_shutdown().
		 */
		if (ListenSocket==-1) return 0;
		http_proc.conn[sid].socket.socket=socket;
#ifdef WIN32
		if (http_proc.conn[sid].socket.socket==INVALID_SOCKET) {
			log_error("httpd", __FILE__, __LINE__, 2, "httpd_accept_loop() shutting down...");
			DEBUG_OUT(NULL, "httpd_accept_loop()");
			return 0;
#else
		if (http_proc.conn[sid].socket.socket<0) {
			continue;
#endif
		} else {
			if (conntype==CONN_SSL) ssl_accept(&http_proc.conn[sid].socket);
			if (pthread_create(&http_proc.conn[sid].handle, &thr_attr, htloop, (void *)sid)==-1) {
				log_error("httpd", __FILE__, __LINE__, 0, "htloop() failed to start...");
				DEBUG_OUT(NULL, "httpd_accept_loop()");
				exit(-1);
			}
		}
	}
	DEBUG_OUT(NULL, "httpd_accept_loop()");
	return 0;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int loaded=0;
	int i;

	http_proc.ListenSocketSTD=0;
	http_proc.ListenSocketSSL=0;
	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	log_error("core", __FILE__, __LINE__, 1, "Starting %s httpd %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (http_proc.config.http_port) {
		if ((http_proc.ListenSocketSTD=tcp_bind(http_proc.config.http_interface, http_proc.config.http_port))!=-1) loaded=1;
	}
	if ((proc->ssl_is_loaded)&&(http_proc.config.http_sslport)) {
		if ((http_proc.ListenSocketSSL=tcp_bind(http_proc.config.http_interface, http_proc.config.http_sslport))!=-1) loaded=1;
	}
	if (loaded) {
		if ((http_proc.conn=calloc(http_proc.config.http_maxconn, sizeof(CONN)))==NULL) {
			printf("\r\nconn calloc(%d, %d) failed\r\n", http_proc.config.http_maxconn, sizeof(CONN));
			return -1;
		}
		for (i=0;i<http_proc.config.http_maxconn;i++) http_proc.conn[i].socket.socket=-1;
		if (modules_init()!=0) exit(-2);
		return 0;
	}
	return -1;
}

DllExport int mod_exec()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if ((http_proc.config.http_port)&&(http_proc.ListenSocketSTD!=-1)) {
		if (pthread_create(&http_proc.ListenThreadSTD, &thr_attr, httpd_accept_loop, (void *)CONN_STD)==-1) {
			log_error(NULL, __FILE__, __LINE__, 0, "httpd_accept_loop() failed...");
			return -2;
		}
	}
	if ((http_proc.config.http_sslport)&&(http_proc.ListenSocketSSL!=-1)&&(proc->ssl_is_loaded)) {
		if (pthread_create(&http_proc.ListenThreadSSL, &thr_attr, httpd_accept_loop, (void *)CONN_SSL)==-1) {
			log_error(NULL, __FILE__, __LINE__, 0, "httpd_accept_loop_ssl() failed...");
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

	for (i=0;i<http_proc.config.http_maxconn;i++) {
		if ((http_proc.conn[i].id==0)||(http_proc.conn[i].socket.atime==0)) continue;
		connections++;
		if (http_proc.conn[i].state==0) {
			if (ctime-http_proc.conn[i].socket.atime<http_proc.config.http_maxkeepalive) continue;
		} else {
			if (ctime-http_proc.conn[i].socket.atime<http_proc.config.http_maxidle) continue;
		}
		log_error("httpd", __FILE__, __LINE__, 4, "Reaping idle thread 0x%08X (idle %d seconds)", http_proc.conn[i].id, ctime-http_proc.conn[i].socket.atime);
		/* closeconnect is _not_ ssl-friendly */
/*
		closeconnect(&http_proc.conn[i], 0);
		if (&http_proc.conn[i].dat!=NULL) {
			if (&http_proc.conn[i].dat->wm!=NULL) {
				tcp_close(&http_proc.conn[i].dat->wm->socket, 0);
			}
		}
*/
		tcp_close(&http_proc.conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error("core", __FILE__, __LINE__, 1, "Stopping %s httpd %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (http_proc.ListenSocketSTD>0) {
/*		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		http_proc.ListenSocketSTD=0;
	}
	if (http_proc.ListenSocketSSL>0) {
/*
		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		http_proc.ListenSocketSSL=0;
	}
	return 0;
}
