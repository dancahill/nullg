/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "ghttpd_main.h"

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

	hThread=(HANDLE)_beginthreadex(NULL, attr->dwStackSize?attr->dwStackSize:PTHREAD_STACK_SIZE, func, param, 0, &id);
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
	CONN *sid=x;

	DEBUG_IN(NULL, "htloop()");
	log_error("core", __FILE__, __LINE__, 2, "Starting htloop() thread");
	if (sid==NULL) {
		log_error(MODSHORTNAME, __FILE__, __LINE__, 0, "missing sid");
		return 0;
	}
	sid->id=pthread_self();
#ifndef WIN32
	pthread_detach(sid->id);
#endif
	log_error(MODSHORTNAME, __FILE__, __LINE__, 4, "Opening connection [%s:%d]", sid->socket.RemoteAddr, sid->socket.RemotePort);
	proc->stats.http_conns++;
	for (;;) {
		if (sid->PostData!=NULL) { free(sid->PostData); sid->PostData=NULL; }
		if (sid->dat!=NULL) { free(sid->dat); sid->dat=NULL; }
		sid->dat=calloc(1, sizeof(CONNDATA));
		sid->dat->out_ContentLength=-1;
		sid->socket.atime=time(NULL);
		sid->socket.ctime=time(NULL);
		strncpy(sid->dat->in_RemoteAddr, sid->socket.RemoteAddr, sizeof(sid->dat->in_RemoteAddr)-1);
		sid->dat->in_RemotePort=sid->socket.RemotePort;
		if (sid->socket.ssl!=NULL) {
			sid->dat->in_ServerPort=http_proc.config.http_sslport;
		} else {
			sid->dat->in_ServerPort=http_proc.config.http_port;
		}
		http_dorequest(sid);
		sid->dat->out_bodydone=1;
		flushbuffer(sid);
		log_htaccess(sid);
		sid->state=0;
		if (sid->dat->wm!=NULL) {
			tcp_close(&sid->dat->wm->socket, 1);
			free(sid->dat->wm);
			sid->dat->wm=NULL;
		}
		/* memset(sid->dat, 0, sizeof(CONNDATA)); */
		if (p_strcasestr(sid->dat->out_Connection, "Keep-Alive")==NULL) {
			break;
		}
	}
	log_error(MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", sid->socket.RemoteAddr, sid->socket.RemotePort);
	/* closeconnect() cleans up our mess for us */
	closeconnect(sid, 2);
	sid->socket.socket=-1;
	DEBUG_OUT(NULL, "htloop()");
	pthread_exit(0);
	return 0;
}

void *get_conn()
{
	int i;

	pthread_mutex_lock(&ListenerMutex);
	for (i=0;;i++) {
		if (i>http_proc.config.http_maxconn-1) { msleep(5); i=0; }
		if (http_proc.conn[i].id==0) break;
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
	return &http_proc.conn[i];
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	int loaded=0;
	int i;

	memset((char *)&http_proc.ListenSocketSTD, 0, sizeof(http_proc.ListenSocketSTD));
	memset((char *)&http_proc.ListenSocketSSL, 0, sizeof(http_proc.ListenSocketSSL));
	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	lang_read();
	log_error("core", __FILE__, __LINE__, 1, "Starting %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (http_proc.config.http_port) {
		if ((http_proc.ListenSocketSTD.socket=tcp_bind(http_proc.config.http_interface, http_proc.config.http_port))!=-1) loaded=1;
	} else {
		http_proc.ListenSocketSTD.socket=-1;
	}
	if ((proc->ssl_is_loaded)&&(http_proc.config.http_sslport)) {
		if ((http_proc.ListenSocketSSL.socket=tcp_bind(http_proc.config.http_interface, http_proc.config.http_sslport))!=-1) loaded=1;
	} else {
		http_proc.ListenSocketSSL.socket=-1;
	}
	if (loaded) {
		if ((http_proc.conn=calloc(http_proc.config.http_maxconn, sizeof(CONN)))==NULL) {
			printf("\r\nconn calloc(%d, %d) failed\r\n", http_proc.config.http_maxconn, (int)sizeof(CONN));
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
	if ((http_proc.config.http_port)&&(http_proc.ListenSocketSTD.socket!=-1)) {
		addlistener(MODSHORTNAME, &http_proc.ListenSocketSTD, get_conn, htloop, 0);
	}
	if ((http_proc.config.http_sslport)&&(http_proc.ListenSocketSSL.socket!=-1)&&(proc->ssl_is_loaded)) {
		addlistener(MODSHORTNAME, &http_proc.ListenSocketSSL, get_conn, htloop, 1);
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	if ((http_proc.ListenSocketSTD.socket==-1)&&(http_proc.ListenSocketSSL.socket==-1)) return 0;
	for (i=0;i<http_proc.config.http_maxconn;i++) {
		if ((http_proc.conn[i].id==0)||(http_proc.conn[i].socket.atime==0)) continue;
		connections++;
		if (http_proc.conn[i].state==0) {
			if (ctime-http_proc.conn[i].socket.atime<http_proc.config.http_maxkeepalive) continue;
		} else {
			if (ctime-http_proc.conn[i].socket.atime<http_proc.config.http_maxidle) continue;
		}
		log_error(MODSHORTNAME, __FILE__, __LINE__, 4, "[%s:%d] Reaping idle socket (idle %d seconds)", http_proc.conn[i].socket.RemoteAddr, http_proc.conn[i].socket.RemotePort, ctime-http_proc.conn[i].socket.atime);
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
	log_error("core", __FILE__, __LINE__, 1, "Stopping %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	if (http_proc.ListenSocketSTD.socket>0) {
/*		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		http_proc.ListenSocketSTD.socket=0;
	}
	if (http_proc.ListenSocketSSL.socket>0) {
/*
		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		http_proc.ListenSocketSSL.socket=0;
	}
	return 0;
}
