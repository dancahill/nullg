/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#include "nsp/nsplib.h"
#if defined(linux)
#include <execinfo.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#else
#ifdef __TURBOC__
#else
#include <errno.h>
#include <unistd.h>
#endif
#endif

#include <signal.h>
nsp_state *N;

#if defined(__BORLANDC__)
extern char **_environ;
#define environ _environ
#elif defined(_MSC_VER)
_CRTIMP extern char **environ;
#else
extern char **environ;
#endif

#define striprn(s) { int n=nc_strlen(s)-1; while (n>-1&&(s[n]=='\r'||s[n]=='\n')) s[n--]='\0'; }

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
static int flush(nsp_state *N)
{
	if (N == NULL || N->outbuflen == 0) return 0;
	N->outbuffer[N->outbuflen] = '\0';
#if defined(_WIN32) && defined(_DEBUG)
	OutputDebugStringA(N->outbuffer);
#endif
	if (write(STDOUT_FILENO, N->outbuffer, N->outbuflen) != N->outbuflen) {
		printf("flush() error\r\n");
	}
	N->outbuflen = 0;
	return 0;
}

static void sig_trap(int sig)
{
	flush(N); /* if we die here, we should flush the buffer first */
	switch (sig) {
	case 11:
		printf("Segmentation Violation\r\n");
#ifdef _WIN32
		{
			DWORD rc;

			if ((rc = GetLastError()) != 0) {
				LPVOID lpMsgBuf;

				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, rc, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
				printf("GetLastError = %s\r\n", (char *)lpMsgBuf);
				LocalFree(lpMsgBuf);
			}
		}
#endif
		if ((N) && (n_context_readptr)) printf("[%s][%.40s]\r\n", N->context->tracefn, n_context_readptr);
#if defined(linux)
		{
#define SIZE 100
			void *buffer[SIZE];
			char **strings;
			int j, nptrs;

			nptrs = backtrace(buffer, SIZE);
			printf("SIGSEGV backtrace() returned %d addresses\r\n", nptrs);
			strings = backtrace_symbols(buffer, nptrs);
			if (strings != NULL) {
				for (j = 0; j < nptrs; j++) {
					printf("SIGSEGV [%s]\r\n", strings[j]);
				}
				free(strings);
			}
		}
		if (N && N->context) printf("[%s][%ld]\r\n", N->context->filename, N->context->linenum);
#endif
		exit(-1);
	case 13: /* SIGPIPE */
		return;
	default:
		printf("Unexpected signal [%d] received\r\n", sig);
	}
}

static void setsigs(void)
{
#if defined(_WIN32)
	signal(SIGSEGV, sig_trap);
#elif !defined(__TURBOC__)
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_trap;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
#endif
	return;
}

static void timeout(int i) {
}

static NSP_FUNCTION(neslib_io_gets)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
#if !defined(_WIN32)&&!defined(__TURBOC__)
	struct sigaction sa;
	int err = 0;
#endif
	char buf[1024];
	char *ret;
	int t = 0;

	if (nsp_isnum(cobj1)) {
		t = (int)cobj1->val->d.num;
	}

	flush(N);
#if !defined(_WIN32)&&!defined(__TURBOC__)
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = timeout;
	sigaction(SIGALRM, &sa, NULL);
	alarm(t);
#endif
	ret = fgets(buf, sizeof(buf) - 1, stdin);
#if !defined(_WIN32)&&!defined(__TURBOC__)
	err = errno;
	alarm(0);
	if (ret == NULL) {
		switch (err) {
		case EINTR:
			printf("timed out\r\n");
			return -2;
		default:
			return -1;
		}
	}
#else
	if (ret == NULL) {
		printf("fgets() error\r\n");
	}
#endif
	striprn(buf);
	nsp_setstr(N, &N->r, "", buf, -1);
	return 0;
}

static void preppath(nsp_state *N, char *name)
{
	char buf[1024];
	char *p;
	unsigned int j;

	p = name;
	nc_memset((void *)&buf, 0, sizeof(buf));
	if ((name[0] == '/') || (name[0] == '\\') || (name[1] == ':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf) - 1);
	} else if (name[0] == '.') {
		/* looks relative... */
		if (getcwd(buf, (unsigned long)(sizeof(buf) - strlen(name) - 2)) != NULL) {
			strncat(buf, "/", sizeof(buf) - strlen(buf) - 1);
			strncat(buf, name, sizeof(buf) - strlen(buf) - 1);
		}
	} else {
		if (getcwd(buf, (unsigned long)(sizeof(buf) - strlen(name) - 2)) != NULL) {
			strncat(buf, "/", sizeof(buf) - strlen(buf) - 1);
			strncat(buf, name, sizeof(buf) - strlen(buf) - 1);
		}
	}
	for (j = 0; j < strlen(buf); j++) {
		if (buf[j] == '\\') buf[j] = '/';
	}
	for (j = (unsigned long)strlen(buf) - 1; j > 0; j--) {
		if (buf[j] == '/') { buf[j] = '\0'; p = buf + j + 1; break; }
	}
	nsp_setstr(N, &N->g, "_filename", p, -1);
	nsp_setstr(N, &N->g, "_filepath", buf, -1);
#if defined(_WIN32) && defined(_DEBUG)
	nsp_setbool(N, nsp_settable(N, nsp_settable(N, &N->g, "lib"), "debug"), "attached", 1);
#endif
	return;
}

void set_console_title(nsp_state *N)
{
#ifdef _WIN32
	char tmpbuf[80];

	nc_memset((void *)&tmpbuf, 0, sizeof(tmpbuf));
	_snprintf(tmpbuf, sizeof(tmpbuf) - 1, "NSP %s", nsp_getstr(N, &N->g, "_filename"));
	SetConsoleTitle(tmpbuf);
#endif
	return;
}

void do_banner() {
	printf("\r\nNullLogic Embedded Scripting Language Version " NSP_VERSION);
	printf("\r\nCopyright (C) 2007-2023 Dan Cahill\r\n\r\n");
	return;
}

void do_help(char *arg0) {
	printf("Usage : %s [-e] [-f] file.ns\r\n", arg0);
	printf("  -e  execute string\r\n");
	printf("  -f  execute file\r\n\r\n");
	return;
}

void do_preload(nsp_state *N)
{
	char sbuf[80];
#ifdef _WIN32
	char wdbuf[80];

	if (GetWindowsDirectory(wdbuf, sizeof(wdbuf)) == 0) return;
	_snprintf(sbuf, sizeof(sbuf) - 1, "%s\\NSP\\preload.ns", wdbuf);
#else
	snprintf(sbuf, sizeof(sbuf) - 1, "/usr/lib/nsp/preload.ns");
#endif
	nsp_execfile(N, sbuf);
	return;
}

#define COMMABUFSIZE 32
static char *printcommas(int num, char *buf, short clear)
{
	/* unsafe cosmetic debut output */
	if (clear) buf[0] = 0;
	if (num > 999) printcommas(num / 1000, buf, 0);
	sprintf(buf + strlen(buf), "%0*d%s", num > 999 ? 3 : 1, num % 1000, clear ? "" : ",");
	return buf;
}

static void printstate(nsp_state *N, char *fn)
{
	char buf1[COMMABUFSIZE];
	char buf2[COMMABUFSIZE];

	printf("\r\nINTERNAL STATE REPORT %s\r\n", fn);
	if (N->allocs)   printf("\tallocs   = %s (%s bytes)\r\n", printcommas(N->allocs, buf1, 1), printcommas(N->allocmem, buf2, 1));
	if (N->frees)    printf("\tfrees    = %s (%s bytes)\r\n", printcommas(N->frees, buf1, 1), printcommas(N->freemem, buf2, 1));
	if (N->allocs)   printf("\tdiff     = %s (%s bytes)\r\n", printcommas(labs(N->allocs - N->frees), buf1, 1), printcommas(labs(N->allocmem - N->freemem), buf2, 1));
	if (N->peakmem)  printf("\tpeak     = %s bytes\r\n", printcommas(N->peakmem, buf1, 1));
	if (N->counter1) printf("\tcounter1 = %s\r\n", printcommas(N->counter1, buf1, 1));
	printf("\tsizeof(nsp_state)  = %d\r\n", (int)sizeof(nsp_state));
	printf("\tsizeof(nsp_objrec) = %d\r\n", (int)sizeof(nsp_objrec));
	printf("\tsizeof(nsp_valrec) = %d\r\n", (int)sizeof(nsp_valrec));
	return;
}

/* int main(int argc, char *argv[], char *environ[]) */
int main(int argc, char *argv[])
{
	char tmpbuf[MAX_OBJNAMELEN + 1];
	obj_t *tobj;
	int i;
	char *p;
	char c;
	short intstatus = 0;
	short preload = 1;
	char *fn = NULL;

	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) {
		do_banner();
		do_help(argv[0]);
		return -1;
	}
	if ((N = nsp_newstate()) == NULL) return -1;
	setsigs();
	N->debug = 0;
	//nspbase_register_all(N);
	/* add env */
	tobj = nsp_settable(N, &N->g, "_ENV");
	for (i = 0; environ[i] != NULL; i++) {
		strncpy(tmpbuf, environ[i], MAX_OBJNAMELEN);
		p = strchr(tmpbuf, '=');
		if (!p) continue;
		*p = '\0';
		p = strchr(environ[i], '=') + 1;
		nsp_setstr(N, tobj, tmpbuf, p, -1);
	}
	/* add args */
	tobj = nsp_settable(N, &N->g, "_ARGS");
	for (i = 0; i < argc; i++) {
		n_ntoa(N, tmpbuf, i, 10, 0);
		nsp_setstr(N, tobj, tmpbuf, argv[i], -1);
	}
	tobj = nsp_settable(N, &N->g, "io");
	nsp_setcfunc(N, tobj, "gets", (NSP_CFUNC)neslib_io_gets);
	for (i = 1; i < argc; i++) {
		if (argv[i] == NULL) break;
		if (argv[i][0] == '-') {
			c = argv[i][1];
			if (!c) {
				break;
			} else if ((c == 'd') || (c == 'D')) {
				N->debug = 1;
			} else if ((c == 's') || (c == 'S')) {
				intstatus = 1;
			} else if ((c == 'b') || (c == 'B')) {
				preload = 0;
			} else if ((c == 'e') || (c == 'E')) {
				if (++i < argc) {
					if (preload) do_preload(N);
					nsp_exec(N, argv[i]);
					if (N->err) goto err;
				}
			} else if ((c == 'f') || (c == 'F')) {
				if (++i < argc) {
					preppath(N, argv[i]);
					set_console_title(N);
					if (preload) do_preload(N);
					fn = argv[i];
					nsp_execfile(N, fn);
					if (N->err) goto err;
				}
			} else if ((c == 'v') || (c == 'V')) {
				printf(NSP_VERSION "\r\n");
				return 0;
			} else {
				do_help(argv[0]);
				return -1;
			}
		} else {
			preppath(N, argv[i]);
			set_console_title(N);
			if (preload) do_preload(N);
			fn = argv[i];
			nsp_execfile(N, fn);
			if (N->err) goto err;
		}
	}
err:
	if (N->err) printf("%s\r\n", N->errbuf);
	nsp_freestate(N);
	if (intstatus || N->allocs != N->frees) printstate(N, fn);
	nsp_endstate(N);
	return 0;
}
