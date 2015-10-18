/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#if defined(linux)
#include <execinfo.h>
#endif

#ifndef WIN32
#include <errno.h>

int daemon(int nochdir, int noclose)
{
	int fd;

	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		sleep(1);
		_exit(0);
	}
	if (setsid() == -1) return -1;
	if (noclose) return 0;
	fd = open(_PATH_DEVNULL, O_RDWR, 0);
	if (fd != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > 2) close(fd);
	}
	return 0;
}
#endif

void server_shutdown()
{
	modules_exit();
	return;
}

void server_restart()
{
	return;
}

#ifdef linux
#if __WORDSIZE == 32
#define SIGCONTEXT
#endif
#endif

#ifdef SIGCONTEXT
void sig_catchint(int sig, struct sigcontext context)
#else
void sig_catchint(int sig)
#endif
{
	switch (sig) {
	case 1:
		if ((pthread_t)pthread_self() != proc.DaemonThread) return;
		server_restart();
		return;
	case 2:
		server_shutdown();
		exit(0);
	case 4:
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGILL [%d] Illegal Instruction", sig);
		break;
	case 8:
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGFPE [%d] Floating Point Exception", sig);
		break;
	case 11:
		/*printf("[%s:%d %d]\r\n", __FILE__, __LINE__, pthread_self());*/
#ifdef SIGCONTEXT
//		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGSEGV EIP=0x%08X, PID=%d", context.eip, getpid());
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGSEGV [%d] Segmentation Violation\n"
			"\t  gs: 0x%04X      fs: 0x%04X       es: 0x%04X      ds: 0x%04X\n"
			"\t edi: 0x%08X esi: 0x%08X  ebp: 0x%04X esp: 0x%04X\n"
			"\t ebx: 0x%08X edx: 0x%08X  ecx: 0x%08X eax: 0x%08X\n"
			"\t eip: 0x%08X  cs: 0x%04X     trap: %-8d   err: %d\n"
			"\tflag: 0x%08X  SP: 0x%08X   ss: 0x%04X     cr2: 0x%08X\n",
			sig,
			context.gs, context.fs, context.es, context.ds,
			context.edi, context.esi, context.ebp, context.esp,
			context.ebx, context.edx, context.ecx, context.eax,
			context.eip, context.cs, context.trapno, context.err,
			context.eflags, context.esp_at_signal, context.ss, context.cr2);
#else
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGSEGV [%d] Segmentation Violation", sig);
#if defined(linux)
		{
#define SIZE 100
			void *buffer[SIZE];
			char **strings;
			int j, nptrs;

			nptrs = backtrace(buffer, SIZE);
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGSEGV backtrace() returned %d addresses\n", nptrs);
			strings = backtrace_symbols(buffer, nptrs);
			if (strings != NULL) {
				for (j = 0; j < nptrs; j++) {
					log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGSEGV [%s]", strings[j]);
				}
				free(strings);
			}
		}
#endif
#endif
		exit(-1);
	case 13: /* SIGPIPE */
		return;
	case 15:
		server_shutdown();
		exit(0);
	case 22:
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SIGABRT [%d] Abnormal Termination", sig);
		break;
	default:
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "Unexpected signal [%d] received", sig);
	}
	pthread_exit(0);
}

void setsigs()
{
	short int i;
#ifdef _NSIG
	short int numsigs = _NSIG;
#else
	short int numsigs = NSIG;
#endif

#ifdef WIN32
#ifndef DEBUG
	/*	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX); */
#endif
	for (i = 1;i < numsigs;i++) {
		signal(i, sig_catchint);
	}
#else
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_catchint;
	sigemptyset(&sa.sa_mask);
	for (i = 1;i < numsigs;i++) {
		/* Trapping these sigs could be bad for our health */
		switch (i) {
		case SIGKILL: /* 9 */
		case SIGUSR1: /* 10 */
		case SIGUSR2: /* 12 */
		case SIGCHLD: /* 17 */
		case SIGCONT: /* 18 */
		case SIGSTOP: /* 19 */
			continue;
		}
		sigaction(i, &sa, NULL);
	}
#endif
}

void init(nsp_state *N)
{
	unsigned int mask;

#ifdef WIN32
	if (WSAStartup(0x202, &proc.wsaData)) {
		printf("\r\nWSAStartup failed.\r\n");
		WSACleanup();
		exit(0);
	}
#endif
	umask(007);
	if (proc.debug) printf("Reading config\r\n");
	if (conf_read(N) != 0) {
		printf("\r\nError reading configuration file\r\n");
		exit(-2);
	}
	if (proc.debug) printf("Done reading config\r\n");
	sscanf(nsp_getstr(proc.N, nsp_getobj(proc.N, &proc.N->g, "CONFIG"), "umask"), "%o", &mask);
	mask = mask & 0777;
	umask(mask);
	/*
	 * calling daemon() here is necessary for freebsd.
	 * they don't seem to carry binds across daemon()
	 */
#ifndef WIN32
#ifndef VALGRIND
	daemon(0, 0);
#endif
#endif
#ifndef WIN32
	setsigs();
#endif
	pthread_mutex_init(&Lock.SQL, NULL);
	if (proc.debug) printf("Checking DB\r\n");
	//	if (sanity_checkdb()==-1) {
	//		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SQL subsystem failed sanity check");
	//		printf("\r\nSQL subsystem failed sanity check.\r\n");
	//		exit(-2);
	//	}
	if (proc.debug) printf("Done checking DB\r\n");
#ifdef HAVE_SSL
	if (proc.debug) printf("Initing SSL\r\n");
	ssl_init();
#endif
	return;
}

int addlistener(char *modname, TCP_SOCKET *sock, void *fn_getconn, void *fn_doreq, int use_ssl)
{
#ifdef WIN32
	int opt = 0;
#endif
	int i;

	if (sock == NULL) return -1;
	if (fn_getconn == NULL) return -1;
	if (fn_doreq == NULL) return -1;
	for (i = 0;i < MAXLISTENERS;i++) {
		if (proc.srvlistener[i].socket != NULL) continue;
		if (proc.srvlistener[i].fn_getconn != NULL) continue;
		if (proc.srvlistener[i].fn_doreq != NULL) continue;
		snprintf(proc.srvlistener[i].modname, sizeof(proc.srvlistener[0].modname) - 1, "%s", modname);
		proc.srvlistener[i].socket = sock;
		proc.srvlistener[i].fn_getconn = fn_getconn;
		proc.srvlistener[i].fn_doreq = fn_doreq;
		proc.srvlistener[i].use_ssl = use_ssl;
#ifdef WIN32
		if (ioctlsocket(proc.srvlistener[i].socket->socket, FIONBIO, &opt) == SOCKET_ERROR) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "failed to ioctl new listener");
		}
#else
		if (fcntl(proc.srvlistener[i].socket->socket, F_SETFL, O_NONBLOCK) < 0) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "failed to ioctl new listener");
		}
#endif
		return 0;
	}
	return -1;
}

/* all CONNs looks kinda like this */
typedef struct {
	pthread_t handle;
#ifdef WIN32
	unsigned int id;
#else
	pthread_t id;
#endif
	short int state;
	TCP_SOCKET socket;
	nsp_state *N;
	void *dat;
} COMMCONN;
typedef	void *(*GET_CONN)(void);

#ifdef WIN32
unsigned _stdcall accept_loop(void *x)
#else
void *accept_loop(void *x)
#endif
{
	pthread_attr_t thr_attr;
	struct timeval tv;
	GET_CONN getconn;
	COMMCONN *conn;
	fd_set fdset;
	int maxfd;
	int numfd;
	int rc;
	int i;

	log_error(proc.N, "core", __FILE__, __LINE__, 2, "Starting accept_loop() thread");
#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "pthread_attr_init()");
		exit(-1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, PTHREAD_STACK_SIZE)) exit(-1);
#endif
	FD_ZERO(&fdset);
	maxfd = 0;
	for (numfd = 0;numfd<MAXLISTENERS;numfd++) {
		if (proc.srvlistener[numfd].socket == NULL) break;
		if (proc.srvlistener[numfd].fn_getconn == NULL) break;
		if (proc.srvlistener[numfd].fn_doreq == NULL) break;
		if (proc.srvlistener[numfd].socket->socket>maxfd) {
			maxfd = proc.srvlistener[numfd].socket->socket;
		}
	}
	for (;;) {
		for (i = 0;i < numfd;i++) {
			FD_SET((unsigned int)proc.srvlistener[i].socket->socket, &fdset);
		}
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		rc = select(maxfd + 1, &fdset, NULL, NULL, &tv);
#ifdef WIN32
		if (rc == SOCKET_ERROR) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "select: %.100s", WSAGetLastError());
			exit(-1);
		}
#else
		if ((rc < 0) && (errno != EINTR)) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "select: %.100s", strerror(errno));
			exit(-1);
		}
#endif
		if (rc < 1) {
			/*
			 * no sockets to accept, and no errors to handle.
			 * so either check for shutdown requests, etc...
			 */
			continue;
		}
		for (i = 0;i < numfd;i++) {
			if (!FD_ISSET(proc.srvlistener[i].socket->socket, &fdset)) continue;
			if ((getconn = proc.srvlistener[i].fn_getconn) == NULL) {
				log_error(proc.N, "core", __FILE__, __LINE__, 0, "fn_getconn missing");
				/* no function to handle the conn */
				continue;
			}
			if ((conn = getconn()) == NULL) {
				log_error(proc.N, "core", __FILE__, __LINE__, 0, "getconn failed");
				/* no place to handle the conn */
				continue;
			}
			rc = tcp_accept(proc.srvlistener[i].socket->socket, &conn->socket);
			if (rc < 0) {
#ifdef WIN32
				if (rc == INVALID_SOCKET) {
					log_error(proc.N, "core", __FILE__, __LINE__, 0, "accept_loop() shutting down...");
					return 0;
				}
#else
				if ((errno != EINTR) && (errno != EWOULDBLOCK)) {
					log_error(proc.N, "core", __FILE__, __LINE__, 0, "accept: %.100s", strerror(errno));
					return 0;
				}
#endif
				log_error(proc.N, "core", __FILE__, __LINE__, 0, "accept: %.100s", strerror(errno));
				continue;
			}
			log_error(proc.N, "tcp", __FILE__, __LINE__, 5, "[%s:%d] tcp_accept: new %s connection", conn->socket.RemoteAddr, conn->socket.RemotePort, proc.srvlistener[i].modname);
			log_error(proc.N, "core", __FILE__, __LINE__, 4, "%s connection from %s:%d", proc.srvlistener[i].modname, conn->socket.RemoteAddr, conn->socket.RemotePort);
			if (proc.srvlistener[i].use_ssl) ssl_accept(&conn->socket);
			if (pthread_create(&conn->handle, &thr_attr, proc.srvlistener[i].fn_doreq, conn) == -1) {
				log_error(proc.N, "core", __FILE__, __LINE__, 0, "htloop() failed to start...");
				exit(-1);
			}
		}
	}
	return 0;
}

int startlisteners()
{
	pthread_attr_t thr_attr;

#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (pthread_attr_init(&thr_attr)) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "pthread_attr_init()");
		exit(-1);
	}
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, PTHREAD_STACK_SIZE)) exit(-1);
#endif
	if (pthread_create(&ListenThread, &thr_attr, accept_loop, NULL) == -1) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "accept_loop() failed to start...");
		exit(-1);
	}
	return 0;
}
