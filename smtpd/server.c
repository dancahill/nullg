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
#include "smtp_main.h"
#ifndef WIN32
#include <pwd.h>
#endif

static int get_sid()
{
	int sid;

	if (proc.RunAsCGI) return 0;
	if (conn==NULL) return -1;
	for (sid=0;sid<proc.config.smtp_maxconn;sid++) {
		if (conn[sid].id==pthread_self()) break;
	}
	if ((sid<0)||(sid>=proc.config.smtp_maxconn)) return -1;
	return sid;
}

int closeconnect(CONN *sid, int exitflag)
{
	DEBUG_IN(sid, "closeconnect()");
	if (exitflag<2) {
//		flushbuffer(sid);
	}
	if (proc.RunAsCGI) {
		sql_disconnect(sid);
		fflush(stdout);
		DEBUG_OUT(sid, "closeconnect()");
		exit(0);
	} else if (sid!=NULL) {
		/* shutdown(x,0=recv, 1=send, 2=both) */
		if (sid->socket!=-1) {
			shutdown(sid->socket, 2);
			closesocket(sid->socket);
			sid->socket=-1;
		}
	}
	if ((exitflag)&&(sid!=NULL)) {
		logaccess(sid, 4, "Closing connection [%u]", sid->socket);
		if (sid->dat!=NULL) {
			free(sid->dat);
			sid->dat=NULL;
		}
#ifdef WIN32
		CloseHandle(sid->handle);
#endif
		DEBUG_OUT(sid, "closeconnect()");
		memset(sid, 0, sizeof(conn[0]));
		sid->socket=-1;
		pthread_exit(0);
	}
	DEBUG_OUT(sid, "closeconnect()");
	return 0;
}

#ifndef WIN32
int daemon(int nochdir, int noclose)
{
	int fd;

	switch (fork()) {
		case -1: return -1;
		case 0:  break;
		default: _exit(0);
	}
	if (setsid()==-1) return -1;
	if (noclose) return 0;
	fd=open(_PATH_DEVNULL, O_RDWR, 0);
	if (fd!=-1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd>2) close(fd);
	}
	return 0;
}
#endif

void server_shutdown()
{
	int i;
	int opensockets;

	if (proc.RunAsCGI) return;
#ifndef WIN32
	if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
#endif
	logaccess(NULL, 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
//	pthread_kill(proc.ListenThread, 14);
	shutdown(proc.ListenSocket, 2);
	closesocket(proc.ListenSocket);
	proc.ListenSocket=-1;
	opensockets=0;
	for (i=0;i<proc.config.smtp_maxconn;i++) {
		if (conn[i].socket!=-1) opensockets++;
	}
	if (!opensockets) goto fini;
	sleep(2);
	for (i=0;i<proc.config.smtp_maxconn;i++) {
		if (conn[i].id==0) continue;
//		pthread_kill(conn[i].handle, 14);
		if (conn[i].socket!=-1) {
			shutdown(conn[i].socket, 2);
			closesocket(conn[i].socket);
			conn[i].socket=-1;
		}
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
	}
fini:
	free(conn);
	sql_unsafedisconnect(NULL);
	for (i=0;i<proc.config.sql_maxconn;i++) {
		sql_freeresult(i);
	}
	free(sqlreply);
}

void server_restart()
{
	pthread_attr_t thr_attr;
	int i;

	if (proc.RunAsCGI) return;
#ifndef WIN32
	if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
#endif
	logaccess(NULL, 1, "Restarting %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
//	pthread_kill(proc.ListenThread, 14);
	shutdown(proc.ListenSocket, 2);
	closesocket(proc.ListenSocket);
	proc.ListenSocket=-1;
	sleep(1);
	for (i=0;i<proc.config.smtp_maxconn;i++) {
		if (conn[i].id==0) continue;
//		pthread_kill(conn[i].handle, 14);
		shutdown(conn[i].socket, 2);
		closesocket(conn[i].socket);
		conn[i].socket=-1;
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
	}
	sleep(1);
	free(conn);
	sql_unsafedisconnect(NULL);
	for (i=0;i<proc.config.sql_maxconn;i++) {
		sql_freeresult(i);
	}
	free(sqlreply);
	init();
	if (pthread_attr_init(&thr_attr)) exit(1);
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
		logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
		exit(0);
	}
	return;
}

#ifdef SIGCONTEXT
void sig_catchint(int sig, struct sigcontext info)
#else
void sig_catchint(int sig)
#endif
{
	CONN *sid=NULL;
	int _sid=get_sid();

	if (_sid>=0) sid=&conn[_sid];
	switch (sig) {
	case 1:
		if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
		server_restart();
		return;
	case 2:
		server_shutdown();
		exit(0);
	case 4:
		logerror(sid, __FILE__, __LINE__, "SIGILL [%d] Illegal Instruction", sig);
		break;
	case 8:
		logerror(sid, __FILE__, __LINE__, "SIGFPE [%d] Floating Point Exception", sig);
		break;
	case 11:
		logerror(sid, __FILE__, __LINE__, "SIGSEGV [%d] Segmentation Violation", sig);
#ifdef SIGCONTEXT
		logerror(sid, __FILE__, __LINE__, "SIGSEGV EIP=0x%08X, PID=%d", info.eip, getpid());
#endif
		exit(0);
	case 13: // SIGPIPE
		return;
	case 15:
		server_shutdown();
		exit(0);
	case 22:
		logerror(sid, __FILE__, __LINE__, "SIGABRT [%d] Abnormal Termination", sig);
		break;
	default:
		logerror(sid, __FILE__, __LINE__, "Unexpected signal [%d] received", sig);
	}
	if (sid) {
		logerror(sid, __FILE__, __LINE__, "%s - SMTP Request: ", inet_ntoa(sid->ClientAddr.sin_addr));
	}
//	closeconnect(sid, 2);
	pthread_exit(0);
}

void setsigs()
{
	short int i;
#ifdef _NSIG
	short int numsigs=_NSIG;
#else
	short int numsigs=NSIG;
#endif

#ifdef WIN32
#ifndef DEBUG
//	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
	for(i=1;i<numsigs;i++) {
		signal(i, sig_catchint);
	}
#else
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler=sig_catchint;
	sigemptyset(&sa.sa_mask);
	for(i=1;i<numsigs;i++) {
		// Trapping these sigs could be bad for our health
		switch (i) {
		case SIGKILL: // 9
		case SIGUSR1: // 10
		case SIGUSR2: // 12
		case SIGCHLD: // 17
		case SIGCONT: // 18
		case SIGSTOP: // 19
			continue;
		}
		sigaction(i, &sa, NULL);
	}
#endif
}

#ifdef WIN32
unsigned _stdcall conn_reaper(void *x)
#else
void *conn_reaper(void *x)
#endif
{
	short int connections;
	short int i;
	time_t ctime;

	for (;;) {
		sleep(1);
		connections=0;
		ctime=time(NULL);
		for (i=0;i<proc.config.smtp_maxconn;i++) {
 			if ((conn[i].id==0)||(conn[i].atime==0)) continue;
			connections++;
			if (conn[i].state==0) {
				if (ctime-conn[i].atime<15) continue;
				logaccess(NULL, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].atime);
			} else {
				if (ctime-conn[i].atime<proc.config.smtp_maxidle) continue;
				logaccess(NULL, 4, "Reaping idle thread 0x%08X (idle %d seconds)", conn[i].id, ctime-conn[i].atime);
			}
			closeconnect(&conn[i], 0);
		}
	}
	return 0;
}

/****************************************************************************
 *	init()
 *
 *	Purpose	: Test external dependencies and initialize socket interface
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void init()
{
	struct hostent *hp;
	struct sockaddr_in sin;
	int i;
	int option;
#ifndef WIN32
	struct passwd *pw;
#endif

#ifdef WIN32
	if (WSAStartup(0x202, &proc.wsaData)) {
		printf("\r\nWinsock 2 initialization failed.\r\n");
		WSACleanup();
		exit(0);
	}
#endif
	memset((char *)&proc.config, 0, sizeof(CONFIG));
	if (config_read(&proc.config)!=0) {
		printf("\r\nError reading configuration file\r\n");
		exit(0);
	}
#ifdef FREEBSD  // calling daemon() here is necessary for freebsd
	daemon(0, 0);
#endif
	proc.ListenSocket=socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family=AF_INET;
	if ((strcasecmp("ANY", proc.config.smtp_hostname)==0)||(strcasecmp("INADDR_ANY", proc.config.smtp_hostname)==0)) {
#ifndef WIN32
		logaccess(NULL, 2, "Binding to 'INADDR_ANY:%d'", proc.config.smtp_port);
#endif
		sin.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
#ifndef WIN32
		logaccess(NULL, 2, "Binding to '%s:%d'", proc.config.smtp_hostname, proc.config.smtp_port);
#endif
		hp=gethostbyname(proc.config.smtp_hostname);
		memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	sin.sin_port=htons((unsigned short)proc.config.smtp_port);
	option=1;
	setsockopt(proc.ListenSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));
	i=0;
	while (bind(proc.ListenSocket, (struct sockaddr *)&sin, sizeof(sin))<0) {
		sleep(5);
		i++;
		if (i>6) {
			logerror(NULL, __FILE__, __LINE__, "bind() error [%s:%d]", proc.config.smtp_hostname, proc.config.smtp_port);
			perror("\r\nBind error");
			exit(0);
		}
	}
	if (listen(proc.ListenSocket, 50)<0) {
		logerror(NULL, __FILE__, __LINE__, "listen() error");
		closesocket(proc.ListenSocket);
		exit(0);
	}
#ifndef WIN32
	if (getuid()==0) {
		if (!(pw=getpwnam(proc.config.server_username))) {
			printf("\r\nCannot find user '%s'.  Exiting.\r\n", proc.config.server_username);
			exit(0);
		}
		memset(pw->pw_passwd, 0, strlen(pw->pw_passwd));
		endpwent();
		if (setgid(pw->pw_gid)) exit(0);
		if (setuid(pw->pw_uid)) exit(0);
	}
	setsigs();
#endif
	logaccess(NULL, 1, "Starting %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	pthread_mutex_init(&Lock.SQL, NULL);
	conn=calloc(proc.config.smtp_maxconn, sizeof(CONN));
	if (conn==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", proc.config.smtp_maxconn, sizeof(CONN));
		exit(0);
	}
	for (i=0;i<proc.config.pop3_maxconn;i++) {
		conn[i].socket=-1;
	}
	sqlreply=calloc(proc.config.sql_maxconn, sizeof(SQLRES));
	if (sqlreply==NULL) {
		printf("\r\nsqlreply calloc(%d, %d) failed\r\n", proc.config.sql_maxconn, sizeof(SQLRES));
		exit(0);
	}
	if (sanity_dbcheck()==-1) {
		logerror(NULL, __FILE__, __LINE__, "SQL subsystem failed sanity check");
		printf("\r\nSQL subsystem failed sanity check.\r\n");
		exit(0);
	}
#ifndef WIN32
#ifndef FREEBSD
#ifndef VALGRIND
	daemon(0, 0);
#endif
#endif
#endif
	return;
}

/****************************************************************************
 *	dorequest()
 *
 *	Purpose	: Authenticate and direct the request to the correct function
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void dorequest(CONN *sid)
{
	DEBUG_IN(sid, "dorequest()");
	sid->state=1;
	smtp_main(sid);
	DEBUG_OUT(sid, "dorequest()");
	return;
}

#ifdef WIN32
unsigned _stdcall htloop(void *x)
#else
void *htloop(void *x)
#endif
{
	int sid=(int)x;

	DEBUG_IN(NULL, "htloop()");
	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	logaccess(&conn[sid], 4, "Opening connection thread [%u]", conn[sid].socket);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	proc.stats.http_conns++;
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	conn[sid].atime=time(NULL);
	conn[sid].ctime=time(NULL);
	dorequest(&conn[sid]);
	conn[sid].state=0;
	logaccess(&conn[sid], 4, "Closing connection thread [%u]", conn[sid].socket);
	closeconnect(&conn[sid], 2);
	conn[sid].socket=-1;
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
unsigned _stdcall accept_loop(void *x)
#else
void *accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	int fromlen;
	int i;

	DEBUG_IN(NULL, "accept_loop()");
#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		logerror(NULL, __FILE__, __LINE__, "pthread_attr_init()");
		DEBUG_OUT(NULL, "accept_loop()");
		exit(1);
	}
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
#endif
	for (;;) {
		for (i=0;;i++) {
			if (i>=proc.config.smtp_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].id==0) break;
		}
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].dat=NULL; }
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		fromlen=sizeof(conn[i].ClientAddr);
		conn[i].socket=accept(proc.ListenSocket, (struct sockaddr *)&conn[i].ClientAddr, &fromlen);
#ifdef WIN32
		if (conn[i].socket==INVALID_SOCKET) {
			DEBUG_OUT(NULL, "accept_loop()");
			return 0;
//			logerror(NULL, __FILE__, __LINE__, "accept() failed...");
//			closesocket(proc.ListenSocket);
//			WSACleanup();
//			DEBUG_OUT(NULL, "accept_loop()");
//			exit(0);
#else
		if (conn[i].socket<0) {
			continue;
#endif
		} else {
#ifdef WIN32
			conn[i].id=(unsigned int)1;
#else
			conn[i].id=(pthread_t)1;
#endif
			if (pthread_create(&conn[i].handle, &thr_attr, htloop, (void *)i)==-1) {
				logerror(NULL, __FILE__, __LINE__, "htloop() failed...");
				DEBUG_OUT(NULL, "accept_loop()");
				exit(0);
			}
		}
	}
	DEBUG_OUT(NULL, "accept_loop()");
	return 0;
}
