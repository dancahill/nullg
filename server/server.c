/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

static int get_sid()
{
	int sid;

	if (proc.RunAsCGI) return 0;
	if (conn==NULL) return -1;
	for (sid=0;sid<proc.config.server_maxconn;sid++) {
		if (conn[sid].id==pthread_self()) break;
	}
	if ((sid<0)||(sid>=proc.config.server_maxconn)) return -1;
	return sid;
}

int closeconnect(CONN *sid, int exitflag)
{
	DEBUG_IN(sid, "closeconnect()");
//logdebug(sid, __FILE__, __LINE__, "A %d [%s]", exitflag, sid->dat->in_RequestURI);
	if (exitflag<2) {
		flushbuffer(sid);
	}
	if (proc.RunAsCGI) {
		sql_disconnect(sid);
		fflush(stdout);
		DEBUG_OUT(sid, "closeconnect()");
		exit(0);
	} else if (sid!=NULL) {
		/* shutdown(x,0=recv, 1=send, 2=both) */
		if (sid->dat->wm!=NULL) {
			if (sid->dat->wm->socket>0) {
				shutdown(sid->dat->wm->socket, 2);
				closesocket(sid->dat->wm->socket);
				sid->dat->wm->socket=0;
			}
		}
		shutdown(sid->socket, 2);
		closesocket(sid->socket);
	}
	if ((exitflag)&&(sid!=NULL)) {
		logaccess(sid, 4, "Closing connection [%u]", sid->socket);
		if (sid->PostData!=NULL) {
			free(sid->PostData);
			sid->PostData=NULL;
		}
		if (sid->dat!=NULL) {
			free(sid->dat);
			sid->dat=NULL;
		}
#ifdef WIN32
		CloseHandle(sid->handle);
#endif
		DEBUG_OUT(sid, "closeconnect()");
		memset(sid, 0, sizeof(conn[0]));
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

	if (proc.RunAsCGI) return;
#ifndef WIN32
	if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
#endif
	logaccess(NULL, 1, "Stopping %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	pthread_kill(proc.ListenThread, 14);
	shutdown(proc.ListenSocket, 2);
	closesocket(proc.ListenSocket);
	sleep(1);
	for (i=0;i<proc.config.server_maxconn;i++) {
		if (conn[i].id==0) continue;
		pthread_kill(conn[i].handle, 14);
		shutdown(conn[i].socket, 2);
		closesocket(conn[i].socket);
		if (conn[i].PostData!=NULL) { free(conn[i].PostData); conn[i].PostData=NULL; }
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].PostData=NULL; }
	}
	sleep(1);
	free(conn);
	sql_unsafedisconnect(NULL);
	for (i=0;i<proc.config.sql_maxconn;i++) {
		sql_freeresult(i);
	}
	free(sqlreply);
	exit(0);
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
	pthread_kill(proc.ListenThread, 14);
	shutdown(proc.ListenSocket, 2);
	closesocket(proc.ListenSocket);
	sleep(1);
	for (i=0;i<proc.config.server_maxconn;i++) {
		if (conn[i].id==0) continue;
		pthread_kill(conn[i].handle, 14);
		shutdown(conn[i].socket, 2);
		closesocket(conn[i].socket);
		if (conn[i].PostData!=NULL) { free(conn[i].PostData); conn[i].PostData=NULL; }
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].PostData=NULL; }
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
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
	if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
		logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
		exit(0);
	}
	return;
}

void sig_catchint(int sig)
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
		break;
	case 13: // SIGPIPE
		return;
	case 14:
//		logerror(sid, __FILE__, __LINE__, "SIGALRM [%d] thread timed out (non-fatal)", sig);
		closeconnect(sid, 2);
		pthread_exit(0);
		return;
	case 15:
		server_shutdown();
		exit(0);
	case 17:
	case 18:
		return;
	case 22:
		logerror(sid, __FILE__, __LINE__, "SIGABRT [%d] Abnormal Termination", sig);
		break;
	default:
		logerror(sid, __FILE__, __LINE__, "Unexpected signal [%d] received", sig);
	}
	if (sid) {
		logerror(sid, __FILE__, __LINE__, "%s - HTTP Request: %s %s", sid->dat->in_RemoteAddr, sid->dat->in_RequestMethod, sid->dat->in_RequestURI);
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
		if ((i==SIGKILL)||(i==SIGSTOP)) continue;
		sigaction(i, &sa, NULL);
	}
#endif
}

/****************************************************************************
 *	CGIkilltimer()
 *
 *	Purpose	: Function to kill retarded child processes
 *	Args	: None
 *	Returns	: void
 *	Notes	: Created as a thread in Win32
 ***************************************************************************/
#ifdef WIN32
void CGIkilltimer(void *x)
{
	int idleseconds;

	for (;;) {
		sleep(5);
		idleseconds=time(NULL)-conn[0].atime;
		if (idleseconds>proc.config.server_maxidle) break;
	}
	logaccess(NULL, 4, "CGI is idle for more than %d seconds.  Terminating.", proc.config.server_maxidle);
	exit(0);
	return;
}
#endif

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
		for (i=0;i<proc.config.server_maxconn;i++) {
 			if ((conn[i].id==0)||(conn[i].atime==0)) continue;
			connections++;
			if (conn[i].state==0) {
				if (ctime-conn[i].atime<15) continue;
				logaccess(NULL, 4, "Reaping idle keepalive 0x%08X (idle %d seconds)(%s)", conn[i].id, ctime-conn[i].atime, conn[i].dat->in_RequestURI);
			} else {
				if (ctime-conn[i].atime<proc.config.server_maxidle) continue;
				logaccess(NULL, 4, "Reaping idle thread 0x%08X (idle %d seconds)(%s)", conn[i].id, ctime-conn[i].atime, conn[i].dat->in_RequestURI);
			}
	DEBUG_IN(&conn[i], "conn_reaper() taking action");
			closeconnect(&conn[i], 0);
	DEBUG_IN(&conn[i], "conn_reaper() done taking action");
		}
#ifdef WIN32
		if (connections==0) {
			TrayIcon(0);
		} else {
			TrayIcon(1);
		}
#endif
	}
	return 0;
}

/****************************************************************************
 *	cgiinit()
 *
 *	Purpose	: Alternate init() for cgi version
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void cgiinit()
{
#ifdef WIN32
	if (WSAStartup(0x202, &proc.wsaData)) {
		MessageBox(0, "Winsock 2 initialization failed.", APPTITLE, MB_ICONERROR);
		WSACleanup();
		exit(0);
	}
#endif
	proc.RunAsCGI=1;
#ifdef WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	memset((char *)&proc.config, 0, sizeof(CONFIG));
	if (config_read(&proc.config)!=0) {
		send_error(0, 500, "Server Error", "Cannot read or create configuration file.");
		closeconnect(0, 1);
		exit(0);
	}
	proc.config.server_maxconn=1;
	proc.config.sql_maxconn=5;
	conn=calloc(proc.config.server_maxconn, sizeof(CONN));
	conn[0].dat=calloc(1, sizeof(CONNDATA));
	conn[0].dat->out_ContentLength=-1;
	conn[0].atime=time(NULL);
	conn[0].ctime=time(NULL);
	sqlreply=calloc(proc.config.sql_maxconn, sizeof(SQLRES));
	if (sanity_dbcheck()==-1) {
		logerror(NULL, __FILE__, __LINE__, "SQL subsystem failed sanity check");
		send_error(0, 500, "Server Error", "SQL subsystem failed sanity check.");
		closeconnect(0, 1);
		exit(0);
	}
	if (modules_init()!=0) exit(0);
#ifdef WIN32
	_beginthread(CGIkilltimer, 0, NULL);
#endif
	dorequest(&conn[0]);
	closeconnect(&conn[0], 1);
	exit(0);
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

	printf("Starting %s %s", SERVER_NAME, SERVER_VERSION);
#ifndef WIN32
	if (getuid()==0) {
		printf("\r\n%s cannot be run as user 'root'.  Exiting.\r\n", SERVER_NAME);
		exit(0);
	}
#endif
#ifdef WIN32
	if (WSAStartup(0x202, &proc.wsaData)) {
		MessageBox(0, "Winsock 2 initialization failed.", APPTITLE, MB_ICONERROR);
		WSACleanup();
		exit(0);
	}
#endif
	pthread_mutex_init(&Lock.Global, NULL);
	pthread_mutex_init(&Lock.DB_mheader, NULL);
	pthread_mutex_init(&Lock.FileList, NULL);
	pthread_mutex_init(&Lock.SQL, NULL);
	memset((char *)&proc.config, 0, sizeof(CONFIG));
	if (config_read(&proc.config)!=0) {
#ifdef WIN32
		MessageBox(0, "Error reading configuration file", APPTITLE, MB_ICONERROR);
#else
		printf("\r\nError reading configuration file\r\n");
#endif
		exit(0);
	}
	conn=calloc(proc.config.server_maxconn, sizeof(CONN));
	if (conn==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", proc.config.server_maxconn, sizeof(CONN));
		exit(0);
	}
	logaccess(NULL, 1, "Starting %s %s (%s)", SERVER_NAME, SERVER_VERSION, __DATE__);
	sqlreply=calloc(proc.config.sql_maxconn, sizeof(SQLRES));
	if (sqlreply==NULL) {
		printf("\r\nsqlreply calloc(%d, %d) failed\r\n", proc.config.sql_maxconn, sizeof(SQLRES));
		exit(0);
	}
	if (sanity_dbcheck()==-1) {
		logerror(NULL, __FILE__, __LINE__, "SQL subsystem failed sanity check");
#ifdef WIN32
		MessageBox(0, "SQL subsystem failed sanity check.", APPTITLE, MB_ICONERROR);
#else
		printf("\r\nSQL subsystem failed sanity check.\r\n");
#endif
		exit(0);
	}
	printf(".");
	if (modules_init()!=0) exit(0);
	printf(".");
	proc.ListenSocket=socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family=AF_INET;
	if ((strcasecmp("ANY", proc.config.server_hostname)==0)||(strcasecmp("INADDR_ANY", proc.config.server_hostname)==0)) {
#ifndef WIN32
		logaccess(NULL, 2, "Binding to 'http://INADDR_ANY:%d/'", proc.config.server_port);
		fflush(stdout);
#endif
		sin.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
#ifndef WIN32
		logaccess(NULL, 2, "Binding to 'http://%s:%d/'", proc.config.server_hostname, proc.config.server_port);
		fflush(stdout);
#endif
		hp=gethostbyname(proc.config.server_hostname);
		memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	sin.sin_port=htons((unsigned short)proc.config.server_port);
	option=1;
	setsockopt(proc.ListenSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));
	i=0;
	while (bind(proc.ListenSocket, (struct sockaddr *)&sin, sizeof(sin))<0) {
		sleep(5);
		i++;
		if (i>6) {
			logerror(NULL, __FILE__, __LINE__, "bind() error [%s:%d]", proc.config.server_hostname, proc.config.server_port);
#ifdef WIN32
			MessageBox(0, "The server could not bind itself to the specified port.", APPTITLE, MB_ICONERROR);
#else
			perror("\r\nBind error");
#endif
			exit(0);
		}
	}
	if (listen(proc.ListenSocket, 50)<0) {
		logerror(NULL, __FILE__, __LINE__, "listen() error");
		closesocket(proc.ListenSocket);
		exit(0);
	}
	printf(".");
	printf("OK.\r\n");
	setsigs();
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
	char pageuri[200];
	unsigned char file[255];
	unsigned char htdir[50];
	char *ptemp;
	int relocate=0;

	DEBUG_IN(sid, "dorequest()");
	if (proc.RunAsCGI) {
		read_cgienv(sid);
	} else {
		if (read_header(sid)<0) {
			closeconnect(sid, 1);
			DEBUG_OUT(sid, "dorequest()");
			return;
		}
	}
	proc.stats.http_pages++;
	logaccess(sid, 2, "%s - HTTP Request: %s %s", sid->dat->in_RemoteAddr, sid->dat->in_RequestMethod, sid->dat->in_RequestURI);
	snprintf(file, sizeof(file)-1, "%s%s", proc.config.server_dir_var_htdocs, sid->dat->in_RequestURI);
	memset(htdir, 0, sizeof(htdir));
	snprintf(htdir, sizeof(htdir)-1, "/%s/", SERVER_BASENAME);
	if (strncmp(sid->dat->in_RequestURI, htdir, strlen(htdir))==0) {
		if (filesend(sid, file)==0) {
			DEBUG_OUT(sid, "dorequest()");
			return;
		}
	}
	if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/", 9)==0) {
		if (module_menucall(sid)) {
			DEBUG_OUT(sid, "dorequest()");
			return;
		}
	}
	snprintf(sid->dat->out_ContentType, sizeof(sid->dat->out_ContentType)-1, "text/html");
	if (auth_getcookie(sid)!=0) {
		if (auth_setcookie(sid)==0) {
			db_log_activity(sid, 0, "login", 0, "login", "%s - Login: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
			relocate=1;
		} else if ((getgetenv(sid, "USERNAME")!=NULL)||(getpostenv(sid, "USERNAME")!=NULL)) {
			db_log_activity(sid, 0, "login", 0, "failed login", "%s - Login failed: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
			htpage_login(sid);
			DEBUG_OUT(sid, "dorequest()");
			return;
		} else {
			htpage_login(sid);
			DEBUG_OUT(sid, "dorequest()");
			return;
		}
	}
	memset(pageuri, 0, sizeof(pageuri));
	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s%s", sid->dat->in_ScriptName, sid->dat->in_RequestURI);
	}
	if ((ptemp=strcasestr(pageuri, "username"))!=NULL) { *ptemp='\0'; relocate=1; }
	if ((ptemp=strcasestr(pageuri, "password"))!=NULL) { *ptemp='\0'; relocate=1; }
	if (pageuri[strlen(pageuri)-1]=='&') pageuri[strlen(pageuri)-1]='\0';
	if (pageuri[strlen(pageuri)-1]=='?') pageuri[strlen(pageuri)-1]='\0';
	if (relocate) {
		snprintf(sid->dat->out_Location, sizeof(sid->dat->out_Location)-1, "%s", pageuri);
		send_header(sid, 0, 302, "OK", "1", "text/html", -1, -1);
		prints(sid, "<BR><CENTER>The requested resource can be found at <A HREF=%s>%s</A>.</CENTER>\n", pageuri, pageuri);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('%s');\r\n// -->\r\n</SCRIPT>\r\n", pageuri);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='%s/'\">\r\n", pageuri);
		DEBUG_OUT(sid, "dorequest()");
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/")==0) {
		if (sid->dat->user_menustyle==0) {
			htpage_motd(sid);
		} else {
			htpage_frameset(sid);
		}
		DEBUG_OUT(sid, "dorequest()");
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/logout", 7)==0) {
		auth_logout(sid);
	} else if (module_menucall(sid)) {
		DEBUG_OUT(sid, "dorequest()");
		return;
	} else if (filesend(sid, file)==0) {
		DEBUG_OUT(sid, "dorequest()");
		return;
	} else {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		prints(sid, "<BR><CENTER>The file or function '");
		printht(sid, "%s", sid->dat->in_RequestURI);
		prints(sid, "' could not be found.</CENTER>\n");
		logerror(sid, __FILE__, __LINE__, "%s - Bad request '%s' by %s", sid->dat->in_RemoteAddr, sid->dat->in_RequestURI, sid->dat->user_username);
	}
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
#ifdef WIN32
	TrayIcon(1);
#endif
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	proc.stats.http_conns++;
	for (;;) {
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		conn[sid].PostData=NULL;
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		conn[sid].dat=calloc(1, sizeof(CONNDATA));
		conn[sid].dat->out_ContentLength=-1;
		conn[sid].atime=time(NULL);
		conn[sid].ctime=time(NULL);
		dorequest(&conn[sid]);
		conn[sid].dat->out_bodydone=1;
		flushbuffer(&conn[sid]);
		conn[sid].state=0;
//		memset(conn[sid].dat, 0, sizeof(CONNDATA));
		if (strcasestr(conn[sid].dat->out_Connection, "Keep-Alive")==NULL) {
			break;
		}
	}
	logaccess(&conn[sid], 4, "Closing connection thread [%u]", conn[sid].socket);
	closeconnect(&conn[sid], 2);
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
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) {
		logerror(NULL, __FILE__, __LINE__, "pthread_attr_setstacksize()");
		DEBUG_OUT(NULL, "accept_loop()");
		exit(1);
	}
	for (;;) {
		for (i=0;;i++) {
			if (i>=proc.config.server_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].socket==0) break;
		}
		if (conn[i].PostData!=NULL) { free(conn[i].PostData); conn[i].PostData=NULL; }
		if (conn[i].dat!=NULL) { free(conn[i].dat); conn[i].PostData=NULL; }
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		fromlen=sizeof(conn[i].ClientAddr);
		conn[i].socket=accept(proc.ListenSocket, (struct sockaddr *)&conn[i].ClientAddr, &fromlen);
#ifdef WIN32
		if (conn[i].socket==INVALID_SOCKET) {
			logerror(NULL, __FILE__, __LINE__, "accept() failed...");
			closesocket(proc.ListenSocket);
			WSACleanup();
			DEBUG_OUT(NULL, "accept_loop()");
			exit(0);
#else
		if (conn[i].socket<0) {
			continue;
#endif
		} else {
			conn[i].id=1;
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
