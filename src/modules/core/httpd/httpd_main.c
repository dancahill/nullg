/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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

HTTP_PROC htproc;
static pthread_mutex_t ListenerMutex;

#ifdef WIN32
unsigned _stdcall htloop(void *x)
#else
void *htloop(void *x)
#endif
{
	CONN *sid=x;
	obj_t *cobj;
	short k;

	log_error(proc->N, "core", __FILE__, __LINE__, 2, "Starting htloop() thread");
	if (sid==NULL) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "missing sid");
		return 0;
	}
	sid->id=pthread_self();
#ifndef WIN32
	pthread_detach(sid->id);
#endif
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Opening connection [%s:%d]", sid->socket.RemoteAddr, sid->socket.RemotePort);
	proc->stats.http_conns++;
	for (;;) {
		if (sid->dat!=NULL) { free(sid->dat); sid->dat=NULL; }
		sid->dat=calloc(1, sizeof(CONNDATA));
		sid->dat->out_ContentLength=-1;
		sid->socket.atime=time(NULL);
		sid->socket.ctime=time(NULL);
		http_dorequest(sid);
		k=0;
		cobj=nes_getobj(sid->N, &sid->N->g, "_HEADER");
		if (cobj->val->type==NT_TABLE) {
			if (strcasecmp("Keep-Alive", nes_getstr(sid->N, cobj, "CONNECTION"))==0) k=1;
		}
		sid->N=nes_endstate(sid->N);
		sid->state=0;
		/* memset(sid->dat, 0, sizeof(CONNDATA)); */
		if (!k) break;
	}
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", sid->socket.RemoteAddr, sid->socket.RemotePort);
	/* closeconnect() cleans up our mess for us */
	closeconnect(sid, 2);
	sid->socket.socket=-1;
	pthread_exit(0);
	return 0;
}

void *get_conn()
{
	obj_t *hobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd");
	int i;
	int maxconn;

	pthread_mutex_lock(&ListenerMutex);
	maxconn=(int)nes_getnum(proc->N, hobj, "max_connections");
	for (i=0;;i++) {
		if (i>maxconn-1) { msleep(5); i=0; }
		if (htproc.conn[i].id==0) break;
	}
	if (htproc.conn[i].dat!=NULL) { free(htproc.conn[i].dat); htproc.conn[i].dat=NULL; }
	memset((char *)&htproc.conn[i], 0, sizeof(htproc.conn[i]));
#ifdef WIN32
	htproc.conn[i].id=(unsigned int)1;
#else
	htproc.conn[i].id=(pthread_t)1;
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
	int loaded=0;

	memset((char *)&htproc, 0, sizeof(htproc));
	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
	if (mod_import()!=0) return -1;
	htproc.N=nes_newstate();
	hobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd");
	cobj=nes_getobj(proc->N, hobj, "interface");
	if (cobj->val->type!=NT_STRING) cobj=nes_setstr(proc->N, hobj, "interface", "INADDR_ANY", strlen("INADDR_ANY"));
	cobj=nes_getobj(proc->N, hobj, "port");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "port", 4110);
	cobj=nes_getobj(proc->N, hobj, "ssl_port");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "ssl_port", 4112);
	cobj=nes_getobj(proc->N, hobj, "max_connections");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "max_connections", 50);
	maxconn=(int)nes_getnum(proc->N, hobj, "max_connections");
	cobj=nes_getobj(proc->N, hobj, "max_keepalive");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "max_keepalive", 15);
	cobj=nes_getobj(proc->N, hobj, "max_idle");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "max_idle", 120);
	cobj=nes_getobj(proc->N, hobj, "max_post_size");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "max_post_size", 32*1024*1024);
	cobj=nes_getobj(proc->N, hobj, "session_limit");
	if (cobj->val->type!=NT_NUMBER) cobj=nes_setnum(proc->N, hobj, "session_limit", 1);
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Starting %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	port=(int)nes_getnum(proc->N, hobj, "port");
	if (port) {
		if ((htproc.ListenSocketSTD.socket=tcp_bind(nes_getstr(proc->N, hobj, "interface"), port))!=-1) loaded=1;
	} else {
		htproc.ListenSocketSTD.socket=-1;
	}
	port=(int)nes_getnum(proc->N, hobj, "ssl_port");
	if ((proc->ssl_is_loaded)&&(port)) {
		if ((htproc.ListenSocketSSL.socket=tcp_bind(nes_getstr(proc->N, hobj, "interface"), port))!=-1) loaded=1;
	} else {
		htproc.ListenSocketSSL.socket=-1;
	}
	if (loaded) {
		if ((htproc.conn=calloc(maxconn, sizeof(CONN)))==NULL) {
			printf("\r\nconn calloc(%d, %d) failed\r\n", maxconn, sizeof(CONN));
			return -1;
		}
		for (i=0;i<maxconn;i++) htproc.conn[i].socket.socket=-1;
		if (modules_init()!=0) exit(-2);
		return 0;
	}
	return -1;
}

DllExport int mod_exec()
{
	obj_t *hobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd");

	if (nes_getnum(proc->N, hobj, "port")&&(htproc.ListenSocketSTD.socket!=-1)) {
		addlistener(MODSHORTNAME, &htproc.ListenSocketSTD, (void *)get_conn, (void *)htloop, 0);
	}
	if (nes_getnum(proc->N, hobj, "ssl_port")&&(htproc.ListenSocketSSL.socket!=-1)&&(proc->ssl_is_loaded)) {
		addlistener(MODSHORTNAME, &htproc.ListenSocketSSL, (void *)get_conn, (void *)htloop, 1);
	}
	return 0;
}

DllExport int mod_cron()
{
	obj_t *hobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd");

	int maxconn=(int)nes_getnum(proc->N, hobj, "max_connections");
	int maxkeepalive=(int)nes_getnum(proc->N, hobj, "max_keepalive");
	int maxidle=(int)nes_getnum(proc->N, hobj, "max_idle");
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	if ((htproc.ListenSocketSTD.socket==-1)&&(htproc.ListenSocketSSL.socket==-1)) return 0;
	for (i=0;i<maxconn;i++) {
		if ((htproc.conn[i].id==0)||(htproc.conn[i].socket.atime==0)) continue;
		connections++;
		if (htproc.conn[i].state==0) {
			if (ctime-htproc.conn[i].socket.atime<maxkeepalive) continue;
		} else {
			if (ctime-htproc.conn[i].socket.atime<maxidle) continue;
		}
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Reaping idle socket [%s:%d] (idle %d seconds)", htproc.conn[i].socket.RemoteAddr, htproc.conn[i].socket.RemotePort, ctime-htproc.conn[i].socket.atime);
		/* closeconnect is _not_ ssl-friendly */
/*
		closeconnect(&htproc.conn[i], 0);
		if (&htproc.conn[i].dat!=NULL) {
			if (&htproc.conn[i].dat->wm!=NULL) {
				tcp_close(&htproc.conn[i].dat->wm->socket, 0);
			}
		}
*/
		tcp_close(&htproc.conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Stopping %s " MODSHORTNAME " %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	htproc.N=nes_endstate(htproc.N);
	if (htproc.ListenSocketSTD.socket>0) {
/*		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		htproc.ListenSocketSTD.socket=0;
	}
	if (htproc.ListenSocketSSL.socket>0) {
/*
		shutdown(ListenSocket, 2);
		closesocket(ListenSocket);
*/
		htproc.ListenSocketSSL.socket=0;
	}
	return 0;
}
