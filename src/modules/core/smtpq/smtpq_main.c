/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "smtpq_main.h"

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

DllExport int mod_init(_PROC *_proc)
{
	int i;

	pthread_mutex_init(&ListenerMutex, NULL);
	proc=_proc;
//	config=&proc->config;
//	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Starting %s smtpq %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
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
	if (pthread_attr_setstacksize(&thr_attr, PTHREAD_STACK_SIZE)) return -2;
#endif
	if (pthread_create(&SpoolThread, &thr_attr, smtp_spool, NULL)==-1) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "smtp_spool() loop failed to start.");
		exit(0);
	}
	return 0;
}

DllExport int mod_cron()
{
	time_t ctime=time(NULL);
	short int connections=0;
	short int i;

	return 0;
	for (i=0;i<mod_config.smtp_maxconn;i++) {
		if ((conn[i].id==0)||(conn[i].socket.atime==0)) continue;
		connections++;
		if (conn[i].state==0) {
			if (ctime-conn[i].socket.atime<15) continue;
		} else {
			if (ctime-conn[i].socket.atime<mod_config.smtp_maxidle) continue;
		}
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Reaping idle socket [%s:%d] (idle %d seconds)", conn[i].socket.RemoteAddr, conn[i].socket.RemotePort, ctime-conn[i].socket.atime);
		tcp_close(&conn[i].socket, 0);
	}
	return 0;
}

DllExport int mod_exit()
{
	log_error(proc->N, "core", __FILE__, __LINE__, 1, "Stopping %s smtpq %s (%s)", SERVER_NAME, PACKAGE_VERSION, __DATE__);
	return 0;
}
