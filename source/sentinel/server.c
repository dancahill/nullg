/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "version.h"

#ifdef WIN32
static WSADATA wsaData;
#endif
static int ListenSocket;

int getsid()
{
	int sid;

	if (RunAsCGI) return 0;
	for (sid=0;sid<config.server_maxconn;sid++) {
		if (conn[sid].id==pthread_self()) break;
	}
	if ((sid<0)||(sid>=config.server_maxconn)) {
//		logerror("cannot find sid for %u", pthread_self());
		return -1;
	}
	return sid;
}

void flushheader(int sid)
{
	char line[256];

	if (conn[sid].dat->out_headdone) return;
	if (!conn[sid].dat->out_status) {
		conn[sid].dat->out_headdone=1;
		return;
	}
#ifdef DEBUG
logdata("\n[[[ FLUSHING HEADER ]]]\n");
#endif
	if ((conn[sid].dat->out_bodydone)&&(!conn[sid].dat->out_flushed)) {
		conn[sid].dat->out_ContentLength=strlen(conn[sid].dat->out_ReplyData);
	}
	if ((strcasecmp(conn[sid].dat->in_Connection, "Keep-Alive")==0)&&(conn[sid].dat->out_bodydone)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Keep-Alive");
	} else {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	// Nutscrape and Mozilla don't know what a fucking keepalive is
	if ((strcasestr(conn[sid].dat->in_UserAgent, "MSIE")==NULL)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	if (RunAsCGI) {
		if (strlen(conn[sid].dat->out_CacheControl)) {
			snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", conn[sid].dat->out_CacheControl);
			printf("%s", line);
		}
		if (conn[sid].dat->out_ContentLength>-1) {
			snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", conn[sid].dat->out_ContentLength);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_Date)) {
			snprintf(line, sizeof(line)-1, "Date: %s\r\n", conn[sid].dat->out_Date);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_Expires)) {
			snprintf(line, sizeof(line)-1, "Expires: %s\r\n", conn[sid].dat->out_Expires);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_LastModified)) {
			snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", conn[sid].dat->out_LastModified);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_Pragma)) {
			snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", conn[sid].dat->out_Pragma);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_SetCookieUser)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", conn[sid].dat->out_SetCookieUser);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_SetCookiePass)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", conn[sid].dat->out_SetCookiePass);
			printf("%s", line);
		}
		if (strlen(conn[sid].dat->out_ContentType)) {
			snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", conn[sid].dat->out_ContentType);
			printf("%s", line);
		} else {
			snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
			printf("%s", line);
		}
	} else {
		if (strcasestr(conn[sid].dat->in_Protocol, "HTTP/1.1")!=NULL) {
			snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.1");
		} else {
			snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.0");
		}
		snprintf(line, sizeof(line)-1, "%s %d OK\r\n", conn[sid].dat->out_Protocol, conn[sid].dat->out_status);
		send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		if (strlen(conn[sid].dat->out_CacheControl)) {
			snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", conn[sid].dat->out_CacheControl);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_Connection)) {
			snprintf(line, sizeof(line)-1, "Connection: %s\r\n", conn[sid].dat->out_Connection);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (conn[sid].dat->out_bodydone) {
			snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", conn[sid].dat->out_ContentLength);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_Date)) {
			snprintf(line, sizeof(line)-1, "Date: %s\r\n", conn[sid].dat->out_Date);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_Expires)) {
			snprintf(line, sizeof(line)-1, "Expires: %s\r\n", conn[sid].dat->out_Expires);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_LastModified)) {
			snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", conn[sid].dat->out_LastModified);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_Pragma)) {
			snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", conn[sid].dat->out_Pragma);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		snprintf(line, sizeof(line)-1, "Server: %s\r\n", SERVER_NAME);
		send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		if (strlen(conn[sid].dat->out_SetCookieUser)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", conn[sid].dat->out_SetCookieUser);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_SetCookiePass)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", conn[sid].dat->out_SetCookiePass);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
		if (strlen(conn[sid].dat->out_ContentType)) {
			snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", conn[sid].dat->out_ContentType);
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		} else {
			snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
			send(conn[sid].socket, line, strlen(line), 0);
#ifdef DEBUG
logdata(">> %s", line);
#endif
		}
	}
	conn[sid].dat->out_headdone=1;
	return;
}

void flushbuffer(int sid)
{
	char *pTemp=conn[sid].dat->out_ReplyData;
	unsigned int dcount;

	flushheader(sid);
	if (strlen(pTemp)==0) return;
#ifdef DEBUG
logdata("\n[[[ FLUSHING BUFFER %d ]]]\n", conn[sid].socket);
#endif
	conn[sid].dat->out_flushed=1;
	while (strlen(pTemp)) {
		dcount=512;
		if (strlen(pTemp)<dcount) dcount=strlen(pTemp);
		if (RunAsCGI) {
			fwrite(pTemp, sizeof(char), dcount, stdout);
		} else {
			send(conn[sid].socket, pTemp, dcount, 0);
		}
#ifdef DEBUG
logdata("%-.512s", pTemp);
#endif
		pTemp+=dcount;
	}
	memset(conn[sid].dat->out_ReplyData, 0, sizeof(conn[sid].dat->out_ReplyData));
#ifdef DEBUG
logdata("\n[[[ BUFFER FLUSHED ]]]\n");
#endif
	return;
}

int prints(const char *format, ...)
{
	unsigned char buffer[2048];
	va_list ap;
	int sid=getsid();

	if (sid==-1) return -1;
	conn[sid].atime=time((time_t*)0);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	if (strlen(conn[sid].dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	strcat(conn[sid].dat->out_ReplyData, buffer);
	if (strlen(conn[sid].dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	return 0;
}

int hexprints(const char *format, ...)
{
	char *hex="0123456789ABCDEF";
	unsigned char buffer[2048];
	int offset=0;
	va_list ap;
	int sid=getsid();

	if (sid==-1) return -1;
	conn[sid].atime=time((time_t*)0);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	if (strlen(conn[sid].dat->out_ReplyData)+strlen(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	while (buffer[offset]) {
		if ((buffer[offset]>32)&&(buffer[offset]<128)) {
			conn[sid].dat->out_ReplyData[strlen(conn[sid].dat->out_ReplyData)]=buffer[offset];
		} else {
			conn[sid].dat->out_ReplyData[strlen(conn[sid].dat->out_ReplyData)]='%';
			conn[sid].dat->out_ReplyData[strlen(conn[sid].dat->out_ReplyData)]=hex[(unsigned int)buffer[offset]/16];
			conn[sid].dat->out_ReplyData[strlen(conn[sid].dat->out_ReplyData)]=hex[(unsigned int)buffer[offset]&15];
		}
		offset++;
	}
	return 0;
}

int sgets(char *buffer, int max, int fd)
{
#ifdef DEBUG
	char *obuffer=buffer;
#endif
	int n=0;
	int rc;
	int sid=getsid();

	if (sid==-1) return -1;
	conn[sid].atime=time((time_t*)0);
	if (RunAsCGI) {
		fgets(buffer, sizeof(buffer)-1, stdin);
		return strlen(buffer);
	}
	while (n<max) {
		rc=recv(conn[sid].socket, buffer, 1, 0);
		if (rc<0) {
#ifdef DEBUG
logdata("[[[ SOCKET %d CLOSED BY PEER ]]]\n", conn[sid].socket);
#endif
			conn[sid].dat->out_headdone=1;
			conn[sid].dat->out_bodydone=1;
			conn[sid].dat->out_flushed=1;
			conn[sid].dat->out_ReplyData[0]='\0';
			closeconnect(sid, 1);
		} else if (rc!=1) {
			n= -n;
			break;
		}
		n++;
		if (*buffer=='\n') {
			buffer++;
			break;
		}
		buffer++;
	}
	*buffer=0;
#ifdef DEBUG
logdata("<< %s", obuffer);
#endif
	return n;
}

int closeconnect(int sid, int exitflag)
{
#ifdef WIN32
	char junk[16];
	int rc;
#endif

	flushbuffer(sid);
	if (RunAsCGI) {
		sqlDisconnect();
		fflush(stdout);
		exit(0);
	} else {
#ifdef WIN32
		/* shutdown(x,0=recv, 1=send, 2=both) */
		shutdown(conn[sid].socket, 1);
		while ((rc=recv(conn[sid].socket, junk-1, sizeof(junk)-1, 0))>0) {
		};
		shutdown(conn[sid].socket, 2);
		closesocket(conn[sid].socket);
#else
		close(conn[sid].socket);
#endif
	}
	if (exitflag) {
	logaccess(4, "Closing [%u][%u]", conn[sid].id, conn[sid].socket);
#ifdef WIN32
		CloseHandle(conn[sid].handle);
#endif
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		memset((char *)&conn[sid], 0, sizeof(conn[sid]));
		pthread_exit(0);
	}
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

void endgame()
{
	logaccess(0, "Stopping %s (Build %d)", SERVER_NAME, BUILD);
	printf("Sentinel is shutting down..\n");
	close(ListenSocket);
	fflush(stdout);
	exit(0);
}
#endif

#ifdef WIN32
void logsegv(int sig)
{
	switch (sig) {
		case 2:
			logerror("SIGINT [%d] Interrupt", sig);
			break;
		case 4:
			logerror("SIGILL [%d] Illegal Instruction", sig);
			break;
		case 8:
			logerror("SIGFPE [%d] Floating Point Exception", sig);
			break;
		case 11:
			logerror("SIGSEGV [%d] Segmentation Violation", sig);
			break;
		case 15:
			logerror("SIGTERM [%d] Software Termination signal from kill", sig);
			break;
		case 22:
			logerror("SIGABRT [%d] Abnormal Termination", sig);
			break;
		default:
			logerror("Unknown signal [%d] received", sig);
	}
//	closeconnect(sid, 1);
	exit(-1);
}
#endif

void setsigs()
{
#ifdef WIN32
#ifndef DEBUG
//	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
	signal(SIGINT, logsegv);
	signal(SIGILL, logsegv);
	signal(SIGFPE, logsegv);
	signal(SIGSEGV, logsegv);
	signal(SIGTERM, logsegv);
	signal(SIGABRT, logsegv);
#else
	sigset_t blockmask;
	sigset_t emptymask;
	struct sigaction sa;

	sigemptyset(&emptymask);
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGCHLD);
	sigaddset(&blockmask, SIGHUP);
	sigaddset(&blockmask, SIGALRM);
	memset(&sa, 0, sizeof(sa));
	sa.sa_mask = blockmask;
	sa.sa_handler = endgame;
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = endgame;
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
#endif
}

int sockinit()
{
	struct hostent *hp;
	struct sockaddr_in sin;
	int i=0;

#ifdef WIN32
	if (WSAStartup(0x202, &wsaData)) {
		MessageBox(0, "Winsock 2 initialization failed.", APPTITLE, MB_ICONERROR);
		WSACleanup();
		exit(0);
	}
#else
	printf("Binding to 'http://%s:%d/'...", config.server_hostname, config.server_port);
#endif
	ListenSocket=socket(AF_INET, SOCK_STREAM, 0);
	hp=gethostbyname(config.server_hostname);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family=AF_INET;
	memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	sin.sin_port=htons((unsigned short)config.server_port);
	while (bind(ListenSocket, (struct sockaddr *)&sin, sizeof(sin))<0) {
		sleep(1);
		i++;
		if (i>30) {
			logerror("bind() error");
#ifdef WIN32
			MessageBox(0, "Bind error: Sentinel could not bind itself to the specified port.", APPTITLE, MB_ICONERROR);
#else
			perror("Bind error");
#endif
			exit(0);
		}
	}
//	option=0;
//	setsockopt(ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&option, sizeof(option));
	if (listen(ListenSocket, 50)<0) {
		logerror("listen() error");
#ifdef WIN32
		closesocket(ListenSocket);
#else
		close(ListenSocket);
#endif
		exit(0);
	}
#ifndef WIN32
	printf("OK.\r\n");
	daemon(0, 0);
#endif
	setsigs();
	return 0;
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
		idleseconds=time((time_t*)0)-conn[0].atime;
		if (idleseconds>config.server_maxidle) break;
	}
	logaccess(4, "CGI is idle for more than %d seconds.  Terminating.", config.server_maxidle);
//	closeconnect(0, 1);
	exit(0);
	return;
}
#endif

/****************************************************************************
 *	cgiinit()
 *
 *	Purpose	: Alternate init() for cgi version of Sentinel
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void cgiinit()
{
	char msgbuffer[200];
	int sqr;
#ifdef WIN32
	char commandline[200];
	char file[100];
	struct stat sb;
#endif
	int sid=0;

	RunAsCGI=1;
#ifdef WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	if (configread()!=0) {
		prints("Content-type: text/html\n\n");
		prints("<HTML><CENTER>\nCannot read or create configuration file\n</CENTER></HTML>\n");
		closeconnect(sid, 1);
		exit(0);
	}
	config.server_maxconn=1;
	config.sql_maxconn=2;
	conn=calloc(config.server_maxconn, sizeof(CONNECTION));
	sqlreply=calloc(config.sql_maxconn, sizeof(SQLRES));
#ifdef WIN32
	if (strcmp(config.sql_type, "ODBC")==0) {
		snprintf(file, sizeof(file)-1, "%s\\sentinel.mdb", config.server_etc_dir);
		if (stat(file, &sb)!=0) {
			snprintf(file, sizeof(file)-1, "%s\\template.mdb", config.server_etc_dir);
			if (stat(file, &sb)==0) {
				snprintf(commandline, sizeof(commandline)-1, "copy \"%s\\template.mdb\" \"%s\\sentinel.mdb\"", config.server_etc_dir, config.server_etc_dir);
				system(commandline);
			}
		}
	}
#endif
	if ((sqr=sqlQuery("SELECT count(username) FROM gw_users"))<0) {
		snprintf(msgbuffer, sizeof(msgbuffer)-1, "%s responded abnormally.", config.sql_type);
		if (strcmp(config.sql_type, "ODBC")==0) {
			strcat(msgbuffer, "\n\nPlease verify the integrity of your data source, ");
			strcat(msgbuffer, "\nand make sure your MDAC and JET drivers are up to date.\n");
		} else if (strcmp(config.sql_type, "MYSQL")==0) {
			strcat(msgbuffer, "\nPlease verify that the MYSQL server is running");
			strcat(msgbuffer, "\nand has been properly configured.");
		}
		printf("Content-type: text/html\n\n");
		printf("<HTML><CENTER>\r\n%s\r\n</CENTER></HTML>\n", msgbuffer);
		sqlFreeconnect(sqr);
		closeconnect(sid, 1);
		exit(0);
	}
	sqlFreeconnect(sqr);
	if ((sqr=sqlQuery("SELECT dbversion, tax1name, tax2name, tax1percent, tax2percent FROM gw_dbinfo"))<0) {
		logerror("Could not read dbinfo from database");
		exit(0);
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		logerror("Missing dbinfo data");
		exit(0);
	}
	snprintf(config.info.version, sizeof(config.info.version)-1, "%s", sqlGetvalue(sqr, 0, 0));
	snprintf(config.info.tax1name, sizeof(config.info.tax1name)-1, "%s", sqlGetvalue(sqr, 0, 1));
	snprintf(config.info.tax2name, sizeof(config.info.tax2name)-1, "%s", sqlGetvalue(sqr, 0, 2));
	config.info.tax1percent=(float)atof(sqlGetvalue(sqr, 0, 3));
	config.info.tax2percent=(float)atof(sqlGetvalue(sqr, 0, 4));
	sqlFreeconnect(sqr);
	conn[sid].dat=calloc(1, sizeof(CONNDATA));
	conn[sid].dat->out_ContentLength=-1;
	conn[sid].atime=time((time_t)0);
	conn[sid].ctime=time((time_t)0);
#ifdef WIN32
	_beginthread(CGIkilltimer, 0, NULL);
#endif
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
#ifdef WIN32
	char commandline[200];
	char file[100];
	struct stat sb;
#endif
	char msgbuffer[200];
	int i=0;
	int sqr;

	printf("%s\r\n", SERVER_NAME);
#ifdef DEBUG
#ifdef WIN32
	MessageBox(0, "This is a DEBUG version", APPTITLE, MB_ICONWARNING);
#endif
#endif
	pthread_mutex_init(&Lock.Crypt, NULL);
	pthread_mutex_init(&Lock.Global, NULL);
	pthread_mutex_init(&Lock.SQL, NULL);
	if (configread()!=0) {
#ifdef WIN32
		MessageBox(0, "Error reading configuration file", APPTITLE, MB_ICONERROR);
#else
		printf("Error reading configuration file\r\n");
#endif
		exit(0);
	}
	logaccess(0, "Starting %s (Build %d)", SERVER_NAME, BUILD);
	conn=calloc(config.server_maxconn, sizeof(CONNECTION));
	sqlreply=calloc(config.sql_maxconn, sizeof(SQLRES));
#ifdef WIN32
	if (strcmp(config.sql_type, "ODBC")==0) {
		snprintf(file, sizeof(file)-1, "%s\\sentinel.mdb", config.server_etc_dir);
		if (stat(file, &sb)!=0) {
			snprintf(file, sizeof(file)-1, "%s\\template.mdb", config.server_etc_dir);
			if (stat(file, &sb)==0) {
				snprintf(commandline, sizeof(commandline)-1, "copy \"%s\\template.mdb\" \"%s\\sentinel.mdb\"", config.server_etc_dir, config.server_etc_dir);
				system(commandline);
			}
		}
	}
#endif
	while ((sqr=sqlQuery("SELECT count(username) FROM gw_users"))<0) {
		sleep(1);
		i++;
		if (i>30) {
			snprintf(msgbuffer, sizeof(msgbuffer)-1, "%s responded abnormally.", config.sql_type);
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcat(msgbuffer, "\n\nPlease verify the integrity of your data source, ");
				strcat(msgbuffer, "and make sure your MDAC and JET drivers are up to date.\n");
			} else if (strcmp(config.sql_type, "MYSQL")==0) {
				strcat(msgbuffer, "\n\nPlease verify that the MYSQL server is running and properly configured.\n");
			}
			strcat(msgbuffer, "\nSee error.log for more information on the nature of this error.");
#ifdef WIN32
			MessageBox(0, msgbuffer, APPTITLE, MB_ICONERROR);
#else
			printf("\r\n%s\r\n", msgbuffer);
#endif
			exit(0);
		}
	}
	if ((strcmp(config.key.LicenseType, "Evaluation")==0)&&(atoi(sqlGetvalue(sqr, 0, 0))>5)) {
		snprintf(msgbuffer, sizeof(msgbuffer)-1, "The evaluation version may not have more than 5 user accounts.");
#ifdef WIN32
		MessageBox(0, msgbuffer, APPTITLE, MB_ICONERROR);
#else
		printf("\r\n%s\r\n", msgbuffer);
#endif
		sqlFreeconnect(sqr);
		exit(0);
	} else if ((strcmp(config.key.LicenseType, "Evaluation")!=0)&&(atoi(sqlGetvalue(sqr, 0, 0))>atoi(config.key.LicenseClass))) {
		snprintf(msgbuffer, sizeof(msgbuffer)-1, "The number of user accounts exceeds the available Client Licenses (%d).", atoi(config.key.LicenseClass));
#ifdef WIN32
		MessageBox(0, msgbuffer, APPTITLE, MB_ICONERROR);
#else
		printf("\r\n%s\r\n", msgbuffer);
#endif
		sqlFreeconnect(sqr);
		exit(0);
	}
	sqlFreeconnect(sqr);
	if ((sqr=sqlQuery("SELECT dbversion, tax1name, tax2name, tax1percent, tax2percent FROM gw_dbinfo"))<0) {
		logerror("Could not read dbinfo from database");
		exit(0);
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		logerror("Missing dbinfo data");
		exit(0);
	}
	snprintf(config.info.version, sizeof(config.info.version)-1, "%s", sqlGetvalue(sqr, 0, 0));
	snprintf(config.info.tax1name, sizeof(config.info.tax1name)-1, "%s", sqlGetvalue(sqr, 0, 1));
	snprintf(config.info.tax2name, sizeof(config.info.tax2name)-1, "%s", sqlGetvalue(sqr, 0, 2));
	config.info.tax1percent=(float)atof(sqlGetvalue(sqr, 0, 3));
	config.info.tax2percent=(float)atof(sqlGetvalue(sqr, 0, 4));
	sqlFreeconnect(sqr);
	sockinit();
}

#ifdef WIN32
void WSAReaper(void *x)
{
	short int connections;
	short int i;
	char junk[10];
	int rc;
	time_t ctime;

	for (;;) {
		connections=0;
		ctime=time((time_t)0);
		for (i=0;i<config.server_maxconn;i++) {
 			if (conn[i].id==0) continue;
			GetExitCodeThread((HANDLE)conn[i].handle, &rc);
			if (rc!=STILL_ACTIVE) continue;
			connections++;
			if ((ctime-conn[i].atime<config.server_maxidle)||(conn[i].atime==0)) continue;
			logaccess(4, "Reaping socket %u from pid %u (runtime ~= %d seconds)", conn[i].socket, conn[i].id, ctime-conn[i].atime);
			shutdown(conn[i].socket, SD_BOTH);
			while (recv(conn[i].socket, junk, sizeof(junk), 0)>0) { };
			closesocket(conn[i].socket);
			TerminateThread(conn[i].handle, (DWORD)&rc);
			CloseHandle(conn[i].handle);
			if (conn[i].PostData!=NULL) free(conn[i].PostData);
			if (conn[i].dat!=NULL) free(conn[i].dat);
			memset((char *)&conn[i], 0, sizeof(conn[i]));
		}
		if (connections==0) {
			TrayIcon(0);
		} else {
			TrayIcon(1);
		}
		Sleep(100);
	}
	return;
}
#endif

#ifdef WIN32
unsigned _stdcall htloop(void *x)
#else
unsigned htloop(void *x)
#endif
{
	int sid=(int)x;

	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	logaccess(4, "New client [%u][%u]", conn[sid].id, conn[sid].socket);
#ifdef WIN32
	TrayIcon(1);
#endif
	for (;;) {
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		conn[sid].dat=calloc(1, sizeof(CONNDATA));
		conn[sid].dat->out_ContentLength=-1;
		conn[sid].atime=time((time_t)0);
		conn[sid].ctime=time((time_t)0);
		conn[sid].PostData=NULL;
		dorequest(sid);
//		if (conn[sid].dat->out_status!=304) {
			prints("\r\n\r\n");
//		}
		conn[sid].dat->out_bodydone=1;
		flushbuffer(sid);
		if (strcasestr(conn[sid].dat->out_Connection, "close")!=NULL) break;
	}
	closeconnect(sid, 0);
	logaccess(4, "Closing [%u][%u]", conn[sid].id, conn[sid].socket);
#ifdef WIN32
	CloseHandle((HANDLE)conn[sid].handle);
#endif
	if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	memset((char *)&conn[sid], 0, sizeof(conn[sid]));
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
void accept_loop(void *x)
{
	pthread_attr_t thr_attr;
	int fromlen;
	int i;

	if (pthread_attr_init(&thr_attr)) {
		logerror("pthread_attr_init()");
		exit(1);
	}
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) {
		logerror("pthread_attr_setstacksize()");
		exit(1);
	}
	for (;;) {
		for (i=0;;i++) {
			if (i>=config.server_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].socket==0) break;
		}
		if (conn[i].PostData!=NULL) free(conn[i].PostData);
		if (conn[i].dat!=NULL) free(conn[i].dat);
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		fromlen=sizeof(conn[i].ClientAddr);
		conn[i].socket=accept(ListenSocket, (struct sockaddr *)&conn[i].ClientAddr, &fromlen);
#ifdef WIN32
		if (conn[i].socket==INVALID_SOCKET) {
			logerror("accept() died...  restarting...");
			closesocket(ListenSocket);
			WSACleanup();
			exit(0);
#else
		if (conn[i].socket<0) {
			continue;
#endif
		} else {
			conn[i].id=1;
			if (pthread_create(&conn[i].handle, &thr_attr, htloop, (void *)i)==-1) {
				logerror("htloop() failed...");
				exit(0);
			}
		}
	}
	return;
}
