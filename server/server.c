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
#include "main.h"

#ifndef WIN32
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
	modules_exit();
	return;
}

void server_restart()
{
	return;
}

#ifdef SIGCONTEXT
void sig_catchint(int sig, struct sigcontext info)
#else
void sig_catchint(int sig)
#endif
{
	switch (sig) {
	case 1:
		if ((pthread_t)pthread_self()!=proc.DaemonThread) return;
		server_restart();
		return;
	case 2:
		server_shutdown();
		exit(0);
	case 4:
		log_error("core", __FILE__, __LINE__, 0, "SIGILL [%d] Illegal Instruction", sig);
		break;
	case 8:
		log_error("core", __FILE__, __LINE__, 0, "SIGFPE [%d] Floating Point Exception", sig);
		break;
	case 11:
		log_error("core", __FILE__, __LINE__, 0, "SIGSEGV [%d] Segmentation Violation", sig);
#ifdef SIGCONTEXT
		log_error("core", __FILE__, __LINE__, 0, "SIGSEGV EIP=0x%08X, PID=%d", info.eip, getpid());
#endif
		exit(-1);
	case 13: // SIGPIPE
		return;
	case 15:
		server_shutdown();
		exit(0);
	case 22:
		log_error("core", __FILE__, __LINE__, 0, "SIGABRT [%d] Abnormal Termination", sig);
		break;
	default:
		log_error("core", __FILE__, __LINE__, 1, "Unexpected signal [%d] received", sig);
	}
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

void init()
{
	umask(007);
	memset((char *)&proc.config, 0, sizeof(GLOBAL_CONFIG));
	if (conf_read()!=0) {
		printf("\r\nError reading configuration file\r\n");
		exit(-2);
	}
	umask(proc.config.umask);
#ifdef WIN32
	if (WSAStartup(0x202, &proc.wsaData)) {
		printf("\r\nWinsock 2 initialization failed.\r\n");
		WSACleanup();
		exit(0);
	}
#endif
// calling daemon() here is necessary for freebsd
#ifndef WIN32
#ifndef VALGRIND
	daemon(0, 0);
#endif
#endif
#ifndef WIN32
	setsigs();
#endif
	pthread_mutex_init(&Lock.Global, NULL);
	pthread_mutex_init(&Lock.DB_mheader, NULL);
	pthread_mutex_init(&Lock.FileList, NULL);
	pthread_mutex_init(&Lock.SQL, NULL);

	sqlreply=calloc(proc.config.sql_maxconn, sizeof(SQLRES));
	if (sqlreply==NULL) {
		printf("\r\nsqlreply calloc(%d, %d) failed\r\n", proc.config.sql_maxconn, sizeof(SQLRES));
		exit(-2);
	}
	if (sanity_checkdb()==-1) {
		log_error("core", __FILE__, __LINE__, 0, "SQL subsystem failed sanity check");
		printf("\r\nSQL subsystem failed sanity check.\r\n");
		exit(-2);
	}
#ifdef HAVE_SSL
	ssl_init();
#endif
	return;
}
