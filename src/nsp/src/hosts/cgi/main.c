/*
    nsp.cgi -- simple Nesla CGI host
    Copyright (C) 2007-2015 Dan Cahill

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
// libbase->base.c
//int nspbase_register_all(nsp_state *N);

#ifdef WIN32
#include <direct.h>
#endif
#include <fcntl.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

nsp_state *N;

#if defined(__BORLANDC__)
extern char **_environ;
#define environ _environ
#elif defined(_MSC_VER)
_CRTIMP extern char **environ;
#else
extern char **environ;
#endif

static NSP_FUNCTION(nescgi_flush)
{
	static short headersent = 0;

	if (N == NULL || N->outbuflen == 0) return 0;
	if (!headersent) {
		send_header(0, "text/html", -1, -1);
		headersent = 1;
	}
	N->outbuf[N->outbuflen] = '\0';
	write(STDOUT_FILENO, N->outbuf, N->outbuflen);
	N->outbuflen = 0;
	return 0;
}

static NSP_FUNCTION(nescgi_sendfile)
{
	char tmppath[512];
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *headobj = nsp_settable(N, &N->g, "_HEADER");
	obj_t *cobj;
	struct stat sb;
	int bl;
	int fd;
	int r;
	char *p;

	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL)) {
		send_header(0, "text/html", -1, -1);
		printf("invalid filename [%s]\r\n\r\n", nsp_tostr(N, cobj1));
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (stat(cobj1->val->d.str, &sb) != 0) {
		send_header(0, "text/html", -1, -1);
		printf("couldn't stat [%s]\r\n\r\n", cobj1->val->d.str);
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd = open(cobj1->val->d.str, O_RDONLY | O_BINARY)) < 0) {
		send_header(0, "text/html", -1, -1);
		printf("couldn't open [%s]\r\n\r\n", cobj1->val->d.str);
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	cobj = nsp_getobj(N, headobj, "CONTENT_DISPOSITION");
	if (cobj->val->type == NT_NULL) {
		if ((p = strrchr(cobj1->val->d.str, '/')) != NULL) p++; else p = cobj1->val->d.str;
		snprintf(tmppath, sizeof(tmppath) - 1, "attachment; filename=\"%s\"", p);
		nsp_setstr(N, headobj, "CONTENT_DISPOSITION", tmppath, -1);
	}
	cobj = nsp_getobj(N, headobj, "CONTENT_LENGTH");
	if (cobj->val->type == NT_NULL) {
		nsp_setnum(N, headobj, "CONTENT_LENGTH", sb.st_size);
	}
	cobj = nsp_getobj(N, headobj, "CONTENT_TYPE");
	p = NULL;
	if (cobj->val->type == NT_NULL) {
		p = get_mime_type(cobj1->val->d.str);
		nsp_setstr(N, headobj, "CONTENT_TYPE", p, -1);
	}
	send_header(0, p, sb.st_size, sb.st_mtime);
	bl = sb.st_size;
	p = malloc(8192);
	for (;;) {
		r = read(fd, p, bl < 8192 ? bl : 8192);
		write(STDOUT_FILENO, p, r);
		bl -= r;
		if (bl < 1) break;
	}
	free(p);
	close(fd);
	nsp_setnum(N, &N->r, "", sb.st_size);
	return 0;
}

static NSP_FUNCTION(htnsp_convertnsp)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	char *ss, *se;

	if (cobj1->val->type != NT_STRING) {
		printf("%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	if (cobj1->val->d.str == NULL) return 0;
	nsp_setstr(N, &N->r, "", "print(\"", 7);
	se = ss = cobj1->val->d.str;
	for (;*se;se++) {
		if (strncmp(se, "?>", 2) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "print(\"", 7);
			ss = se += 2;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "\"", 1) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\\\"", 2);
			ss = se += 1;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "\\", 1) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\\\\", 2);
			ss = se += 1;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "<?nsp", 5) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\");", 3);
			ss = se += 5;
			while (se[0] != '?' || se[1] != '>') ++se;
			if (*se) { --se; continue; }
			break;
		}
	}
	if (se > ss) nsp_strcat(N, &N->r, ss, se - ss);
	nsp_strcat(N, &N->r, "\");", 3);
	return 0;
}

static void preppath(nsp_state *N, char *name)
{
	char buf[512];
	char *p;
	unsigned int j;

	p = name;
	if ((name[0] == '/') || (name[0] == '\\') || (name[1] == ':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf) - 1);
	}
	else if (name[0] == '.') {
		/* looks relative... */
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	else {
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	for (j = 0;j < strlen(buf);j++) {
		if (buf[j] == '\\') buf[j] = '/';
	}
	for (j = strlen(buf) - 1;j > 0;j--) {
		if (buf[j] == '/') { buf[j] = '\0'; p = buf + j + 1; break; }
	}
	nsp_setstr(N, &N->g, "_filename", p, -1);
	nsp_setstr(N, &N->g, "_filepath", buf, -1);
	return;
}

static void sig_timeout()
{
	send_header(0, "text/html", -1, -1);
	printf("<B>Connection timed out.</B>\n");
#ifndef WIN32
	if (nsp_getstr(N, nsp_settable(N, &N->g, "_CONFIG"), "use_syslog")[0] == 'y') closelog();
#endif
	exit(0);
}

static void sig_catchint(int sig)
{
	send_header(0, "text/html", -1, -1);
	nescgi_flush(N);
	printf("<B>Caught signal %d.</B>\n", sig);
#ifndef WIN32
	if (nsp_getstr(N, nsp_settable(N, &N->g, "_CONFIG"), "use_syslog")[0] == 'y') closelog();
#endif
	exit(0);
}

static void setsigs()
{
#ifdef _NSIG
	short int numsigs = _NSIG;
#else
	short int numsigs = NSIG;
#endif
	short int i;

	if (numsigs > 16) numsigs = 16;
	for (i = 0;i < numsigs;i++) signal(i, sig_catchint);
#ifdef WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#else
	signal(SIGALRM, sig_timeout);
	alarm(nsp_getnum(N, nsp_settable(N, &N->g, "_CONFIG"), "max_runtime"));
#endif
}

int htnsp_runscript(char *file)
{
	char *p;

	nsp_setcfunc(N, nsp_settable(N, &N->g, "io"), "flush", nescgi_flush);
	nsp_setcfunc(N, &N->g, "sendfile", nescgi_sendfile);
	nsp_setcfunc(N, &N->g, "convertnsp", (NSP_CFUNC)htnsp_convertnsp);
	preppath(N, file);

	p = strrchr(file, '.');
	if (p != NULL&&strcmp(p, ".nsp") == 0) {
		char hackbuf[512];

		for (p = file;*p;p++) if (*p == '\\') *p = '/';
		snprintf(hackbuf, sizeof(hackbuf) - 1, "exec(convertnsp(file.readall(\"%s\")));", file);
		nsp_exec(N, hackbuf);
	}
	else {
		nsp_execfile(N, file);
	}

	// nsp_execfile(N, file);
	if (N->err) printf("<HR /><B>[errno=%d :: %s]</B>\r\n", N->err, N->errbuf);
	return 0;
}

int main(int argc, char *argv[])
{
	char tmpbuf[MAX_OBJNAMELEN + 1];
	obj_t *cobj, *confobj, *headobj, *servobj, *tobj;
	char *PathTranslated;
	char *p;
	int i;
	struct stat sb;

	if (getenv("REQUEST_METHOD") == NULL) {
		printf("This program must be run as a CGI.\r\n");
		exit(0);
	}
	if ((N = nsp_newstate()) == NULL) {
		printf("nsp_newstate() error\r\n");
		return -1;
	}
	confobj = nsp_settable(N, &N->g, "_CONFIG");
	servobj = nsp_settable(N, &N->g, "_SERVER");
	headobj = nsp_settable(N, &N->g, "_HEADER");
	//nspbase_register_all(N);
	/*
		nspcrypto_register_all(N);
		nspnet_register_all(N);
	#ifdef HAVE_REGEX
		nspregex_register_all(N);
	#endif
	*/
	/* add env */
	for (i = 0;environ[i] != NULL;i++) {
		strncpy(tmpbuf, environ[i], MAX_OBJNAMELEN);
		p = strchr(tmpbuf, '=');
		if (!p) continue;
		*p = '\0';
		p = strchr(environ[i], '=') + 1;
		nsp_setstr(N, servobj, tmpbuf, p, -1);
	}
	/* add args */
	tobj = nsp_settable(N, &N->g, "_ARGS");
	for (i = 0;i < argc;i++) {
		n_ntoa(N, tmpbuf, i, 10, 0);
		nsp_setstr(N, tobj, tmpbuf, argv[i], -1);
	}
	config_read();
	setsigs();
	setvbuf(stdout, NULL, _IONBF, 0);
#ifdef WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#else
	if (nsp_getstr(N, confobj, "use_syslog")[0] == 'y') openlog("nsp.cgi", LOG_PID, LOG_MAIL);
#endif
	cgi_readenv();
	PathTranslated = nsp_getstr(N, servobj, "PATH_TRANSLATED");
	if ((PathTranslated) && (strlen(PathTranslated))) {
		p = nsp_getstr(N, servobj, "SCRIPT_NAME");
		nsp_setstr(N, servobj, "SCRIPT_NAME_ORIG", p, -1);
		cobj = nsp_getobj(N, servobj, "PATH_INFO");
		if ((cobj->val->type != NT_STRING) || (cobj->val->d.str == NULL)) {
			send_header(0, "text/html", -1, -1);
			printf("no PATH_INFO<BR>");
			goto err;
		}
		p = nsp_tostr(N, cobj);
		nsp_setstr(N, servobj, "SCRIPT_NAME", p, -1);
		if (stat(PathTranslated, &sb) != 0) {
			send_header(0, "text/html", -1, -1);
			printf("no script found<BR>");
			goto err;
		}
		htnsp_runscript(PathTranslated);
	}
	else {
	err:
		send_header(0, "text/html", -1, -1);
		/* nsp_exec(N, "print('<pre>',serialize(),'</pre>');"); */
		printf("Unknown command");
	}
	fflush(stdout);
#ifndef WIN32
	if (nsp_getstr(N, confobj, "use_syslog")[0] == 'y') closelog();
#endif
	N = nsp_endstate(N);
	return 0;
}
