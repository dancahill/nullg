/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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
#include "net.h"
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#if _MSC_VER>=1900
#define STDC99
#else
#define snprintf _snprintf
#endif
#define strcasecmp stricmp
#define strncasecmp strnicmp
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static void ftp_lasterr(nsp_state *N, char *msg)
{
	nsp_setstr(N, nsp_getobj(N, &N->l, "this"), "last_err", msg, -1);
	return;
}

static int get_pasvaddr(const char *line, char *ipbuf, unsigned short *port)
{
	uchar ip[4];
	char *p = (char *)line;

	nc_memset((char *)&ip, 0, sizeof(ip));
	while (*p&&*p != '(') p++;
	if (*p != '(') return -1;
	p++;
	ip[0] = atoi(p);
	while (nc_isdigit(*p)) p++;
	if (*p == ',') p++;
	ip[1] = atoi(p);
	while (nc_isdigit(*p)) p++;
	if (*p == ',') p++;
	ip[2] = atoi(p);
	while (nc_isdigit(*p)) p++;
	if (*p == ',') p++;
	ip[3] = atoi(p);
	while (nc_isdigit(*p)) p++;
	if (*p == ',') p++;
	*port = atoi(p) * 256;
	while (nc_isdigit(*p)) p++;
	if (*p == ',') p++;
	*port += atoi(p);
	while (nc_isdigit(*p)) p++;
	snprintf(ipbuf, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return 0;
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_open)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_open()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	int i = 0;

	TCP_SOCKET *sock;
	unsigned short port = 0;
	char *host = NULL;
	char *user = NULL;
	char *pass = NULL;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		host = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "host")))) {
		host = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for host");
	}
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) {
		port = (unsigned short)nsp_tonum(N, cobj);
	}
	else if (nsp_isnum((cobj = nsp_getobj(N, thisobj, "port")))) {
		port = (unsigned short)nsp_tonum(N, cobj);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for port");
	}
	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "3")))) {
		user = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "username")))) {
		user = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for username");
	}
	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "4")))) {
		pass = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "password")))) {
		pass = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for password");
	}
	sock = n_alloc(N, sizeof(TCP_SOCKET) + 1, 1);
	if (sock == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	nc_strncpy(sock->obj_type, "sock4", sizeof(sock->obj_type) - 1);
	sock->obj_term = (NSP_CFREE)tcp_murder;
	if ((rc = tcp_connect(N, sock, host, port, 0)) < 0) {
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "socket", NULL, 0);
	cobj->val->d.str = (void *)sock;
	cobj->val->size = sizeof(TCP_SOCKET) + 1;
	/* now start the ftp dialog */
	/* welcome dialog */
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "220", 3) != 0) {
		ftp_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&N->r.val->d.str, sizeof(TCP_SOCKET) + 1);
		N->r.val->size = 0;
		nsp_unlinkval(N, &N->r);
		return -1;
	}
	/* send username */
	tcp_fprintf(N, sock, "USER %s\r\n", user);
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "331", 3) != 0) {
		ftp_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&N->r.val->d.str, sizeof(TCP_SOCKET) + 1);
		N->r.val->size = 0;
		nsp_unlinkval(N, &N->r);
		return -1;
	}
	/* send password */
	tcp_fprintf(N, sock, "PASS %s\r\n", pass);
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "230", 3) != 0) {
		ftp_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&N->r.val->d.str, sizeof(TCP_SOCKET) + 1);
		N->r.val->size = 0;
		nsp_unlinkval(N, &N->r);
		return -1;
	}
	/* set ascii */
	tcp_fprintf(N, sock, "TYPE A\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "200", 3) != 0) {
		ftp_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&N->r.val->d.str, sizeof(TCP_SOCKET) + 1);
		N->r.val->size = 0;
		nsp_unlinkval(N, &N->r);
		return -1;
	}
	/* feat - though we don't really care */
	tcp_fprintf(N, sock, "FEAT\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp("211-", iobuf, 4) == 0) continue;
		if (iobuf[3] == ' ' || iobuf[3] == '\0') break;
		if (nc_strncmp(" MLST", iobuf, 5) == 0 || nc_strncmp(" MLSD", iobuf, 5) == 0) {
			nsp_setbool(N, thisobj, "use_mlsd", 1);
		}
		nsp_setstr(N, nsp_settable(N, thisobj, "features"), n_ntoa(N, N->numbuf, i++, 10, 0), iobuf, -1);
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "211", 3) != 0) {
		ftp_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&N->r.val->d.str, sizeof(TCP_SOCKET) + 1);
		N->r.val->size = 0;
		nsp_unlinkval(N, &N->r);
		return -1;
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_close)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_close()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* send close */
	tcp_fprintf(N, sock, "QUIT\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	tcp_close(N, sock, 1);
	n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "socket", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_cwd)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_cwd()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *dir = NULL;

	//Command: CWD / dev
	//Response: 250 CWD successful. "/dev" is current directory.

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;

	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		dir = cobj->val->d.str;
	}

	if (dir != NULL) {
		tcp_fprintf(N, sock, "CWD %s\r\n", dir);
	}
	else {
		tcp_fprintf(N, sock, "CWD\r\n");
	}
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "250", 3) != 0) {
		return -1;
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_pwd)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_pwd()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;

	//Command: PWD
	//Response : 257 "/dev" is current directory.

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;

	tcp_fprintf(N, sock, "PWD\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "257", 3) != 0) {
		return -1;
	}
	return 0;
#undef __FN__
}

static void ftp_client_ls_raw(nsp_state *N, TCP_SOCKET *sock2, obj_t *tobj)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_ls_raw()"
	char iobuf[1024];
	char *months[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	//obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *tobj2;
	int rc;
	char *p;
	unsigned short plen;

	do {
		rc = tcp_fgets(N, sock2, iobuf, sizeof(iobuf) - 1);
		if (rc < 1) break;
		striprn(iobuf);
		if (iobuf[0] == '\0') break;
		if (nc_strlen(iobuf) < 56) break;
		if (N->debug) { n_warn(N, __FN__, "got2 %s", iobuf); }
		tobj2 = nsp_settable(N, tobj, iobuf + 56);
		p = iobuf;
		/*
		* This code has the virtue of being both ugly, _and_ probably wrong.
		* It's a two for one boxing week special.
		*/
		/*
		* a dir entry should have these attrs ...
		* x = { mtime = #, size = #, type = "dir":"file" },
		*/
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			nsp_setstr(N, tobj2, "perm", p, plen);
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			/* nsp_setstr(N, tobj2, "subs", p, plen); */
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			nsp_setstr(N, tobj2, "user", p, plen);
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			nsp_setstr(N, tobj2, "group", p, plen);
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			nsp_setnum(N, tobj2, "size", n_aton(N, p));
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			time_t x = time(NULL);
			struct tm *n;
			struct tm t;
			unsigned short m;
			unsigned short y;
			int i;

			n = localtime(&x);
			m = n->tm_mon;
			y = n->tm_year;
			memset((char *)&t, 0, sizeof(t));
			for (i = 0; i < 12; i++) {
				if (strncasecmp(p, months[i], 3) == 0) t.tm_mon = i;
			}
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			if (p[plen] == ' ' || p[plen] == '\t') {
				while (p[plen] == ' ' || p[plen] == '\t') plen++;
				t.tm_mday = atoi(p + plen);
				while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			}
			if (p[plen] == ' ' || p[plen] == '\t') {
				while (p[plen] == ' ' || p[plen] == '\t') plen++;
				if (p[plen + 4] == ' ') {
					t.tm_year = atoi(p + plen) - 1900;
				}
				else if (p[plen + 2] == ':') {
					t.tm_hour = atoi(p + plen);
					t.tm_min = atoi(p + plen + 3);
					/*
					* dates in the future or more than 6
					* months in the past have a year.
					* the rest have times.
					*/
					t.tm_year = y;
					if (t.tm_mon > m) t.tm_year--;
				}
				while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
				nsp_setnum(N, tobj2, "mtime", (num_t)mktime(&t));
			}
			p += plen;
		}
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p) {
			plen = 0;
			while (p[plen] && (p[plen] != ' '&&p[plen] != '\t')) plen++;
			/* we already have the name */
			nsp_setstr(N, tobj2, "name", p, plen);
			p += plen;
		}
		if (iobuf[0] == 'd') {
			nsp_setstr(N, tobj2, "type", "dir", -1);
			nsp_setnum(N, tobj2, "size", 0);
		}
		else if (iobuf[0] == '-') {
			nsp_setstr(N, tobj2, "type", "file", -1);
		}
	} while (rc > 0);
#undef __FN__
}

static int getunixtime(int y, int m, int d, int hh, int mm, int ss)
{
	int dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int unixdate = 0;
	int i;

	for (i = 1970; i < y; i++) {
		unixdate += 365;
		if ((i / 4.0f) == (int)(i / 4)) {
			if ((i / 400.0f) == (int)(i / 400)) {
				unixdate++;
			}
			else if ((i / 100.0f) != (int)(i / 100)) {
				unixdate++;
			}
		}
	}
	for (i = 0; i < m; i++) {
		unixdate += dim[i];
		if (i != 1) continue;
		if ((y / 4.0f) == (int)(y / 4)) {
			if ((y / 400.0f) == (int)(y / 400)) {
				unixdate++;
			}
			else if ((y / 100.0f) != (int)(y / 100)) {
				unixdate++;
			}
		}
	}
	unixdate += d;
	unixdate *= 86400;
	if (unixdate < 0) unixdate = 0;
	unixdate += hh * 3600;
	unixdate += mm * 60;
	unixdate += ss;
	return unixdate;
}

static void ftp_client_ls_mlst(nsp_state *N, TCP_SOCKET *sock2, obj_t *tobj)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_ls_mlst()"
	char iobuf[1024];
	//obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *tobj2;
	char *ss, *se;
	int rc;
	int i = 0;

	/*
	* https://tools.ietf.org/html/rfc3659
	* it can't be worse than what came before, right?
	*/
	/*
	* a dir entry should have these attrs ...
	* x = { mtime = #, size = #, type = "dir":"file" },
	*/
	do {
		rc = tcp_fgets(N, sock2, iobuf, sizeof(iobuf) - 1);
		if (rc < 1) break;
		striprn(iobuf);
		if (iobuf[0] == '\0') break;
		if (N->debug) { n_warn(N, __FN__, "got2 %s", iobuf); }
		tobj2 = nsp_settable(N, tobj, n_ntoa(N, N->numbuf, i++, 10, 0));
		ss = iobuf;
		while (*ss) {
			if (strncasecmp(ss, "type=", 5) == 0) {
				for (se = ss = ss + 5; *se; se++) {
					if (*se != ';') continue;
					nsp_setstr(N, tobj2, "type", ss, se - ss);
					ss = se += 1;
					break;
				}
			}
			else if (strncasecmp(ss, "modify=", 7) == 0) {
				// YYYYMMDDHHMMSS.sss
				// The "." and subsequent digits("sss") are optional.However the "."
				// MUST NOT appear unless at least one following digit also appears.
				// Time values are always represented in UTC(GMT),
				for (se = ss = ss + 7; *se; se++) {
					if (*se != ';') continue;
					nsp_setstr(N, tobj2, "modify", ss, se - ss);
					if (se - ss >= 14) {
						int y = 0, m = 0, d = 0, hh = 0, mm = 0, sec = 0;
						int i;

						for (i = 0; i < 14; i++) {
							int v = (ss[i] - '0');
							switch (i) {
							case 0: y += v * 1000; break;
							case 1: y += v * 100; break;
							case 2: y += v * 10; break;
							case 3: y += v * 1; break;
							case 4: m += v * 10; break;
							case 5: m += v * 1; break;
							case 6: d += v * 10; break;
							case 7: d += v * 1; break;
							case 8: hh += v * 10; break;
							case 9: hh += v * 1; break;
							case 10: mm += v * 10; break;
							case 11: mm += v * 1; break;
							case 12: sec += v * 10; break;
							case 13: sec += v * 1; break;
							}
						}
						m -= 1;
						d -= 1;
						//n_warn(N, __FN__, "dt %d %d %d %d %d %d", y, m, d, hh, mm, sec);
						nsp_setnum(N, tobj2, "mtime", (num_t)getunixtime(y, m, d, hh, mm, sec));
					}
					ss = se += 1;
					break;
				}
			}
			else if (strncasecmp(ss, "size=", 5) == 0) {
				for (se = ss = ss + 5; *se; se++) {
					if (*se != ';') continue;
					nsp_setstr(N, tobj2, "size", ss, se - ss);
					ss = se += 1;
					break;
				}
			}
			else if (strncasecmp(ss, " ", 1) == 0) {
				for (se = ss = ss + 1; *se; se++) {
				}
				nsp_setstr(N, tobj2, "name", ss, se - ss);
				ss = se += 1;
				break;
			}
			else if (*ss) {
				while (*ss && *ss != ';') ss++;
				if (*ss == ';') ss++;
				//n_warn(N, __FN__, "ss=[[%s]]", ss);
			}
		}
	} while (rc > 0);
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_ls)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_ls()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	TCP_SOCKET sock2;
	int rc;
	unsigned short port = 0;
	char ipbuf[20];
	obj_t tobj;
	short use_mlsd = 0;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;

	use_mlsd = nsp_tobool(N, nsp_getobj(N, thisobj, "use_mlsd"));

	/* send pasv */
	tcp_fprintf(N, sock, "PASV\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "227", 3) != 0) {
		return -1;
	}
	if (get_pasvaddr(iobuf, ipbuf, &port) != 0) {
		return -1;
	}
	/* send list */
	tcp_fprintf(N, sock, "%s\r\n", use_mlsd ? "MLSD" : "LIST");
	nc_memset((char *)&sock2, 0, sizeof(sock2));
	if ((rc = tcp_connect(N, &sock2, ipbuf, port, sock->use_tls)) < 0) {
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		return -1;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;

	if (use_mlsd) {
		ftp_client_ls_mlst(N, &sock2, &tobj);
	}
	else {
		ftp_client_ls_raw(N, &sock2, &tobj);
	}

	tcp_close(N, &sock2, 1);
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "150", 3) != 0) {
		return -1;
	}
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "226", 3) != 0) {
		return -1;
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_retr)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_retr()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	TCP_SOCKET *sock;
	TCP_SOCKET sock2;
	int rc;
	char iobuf[1024];
	unsigned short port = 0;
	char ipbuf[20];
	int fd;
	int got = 0;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->size < 1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if ((cobj2->val->type != NT_STRING) || (cobj2->val->size < 1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* send type */
	tcp_fprintf(N, sock, "TYPE I\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "200", 3) != 0) {
		return -1;
	}
	/* send pasv */
	tcp_fprintf(N, sock, "PASV\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "227", 3) != 0) {
		return -1;
	}
	if (get_pasvaddr(iobuf, ipbuf, &port) != 0) {
		return -1;
	}
	/* send retr */
	tcp_fprintf(N, sock, "RETR %s\r\n", cobj1->val->d.str);
	nc_memset((char *)&sock2, 0, sizeof(sock2));
	if ((rc = tcp_connect(N, &sock2, ipbuf, port, sock->use_tls)) < 0) {
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		return -1;
	}
	if ((fd = open(cobj2->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1) {
		n_warn(N, __FN__, "could not open file");
		return -1;
	}
	do {
		rc = tcp_recv(N, &sock2, iobuf, sizeof(iobuf) - 1, 0);
		if (rc < 1) break;
		got += rc;
		if (write(fd, iobuf, rc) != rc) {
			n_warn(N, __FN__, "write() wrote less bytes than expected");
		}
	} while (rc > 0);
	close(fd);
	tcp_close(N, &sock2, 1);
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "150", 3) != 0) {
		return -1;
	}
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "226", 3) != 0) {
		return -1;
	}
	nsp_setnum(N, &N->r, "", got);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ftp_client_stor)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_stor()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	TCP_SOCKET *sock;
	TCP_SOCKET sock2;
	int rc;
	char iobuf[1024];
	unsigned short port = 0;
	char ipbuf[20];
	int fd;
	int sent = 0;
	int r;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->size < 1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if ((cobj2->val->type != NT_STRING) || (cobj2->val->size < 1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* send type */
	tcp_fprintf(N, sock, "TYPE I\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "200", 3) != 0) {
		return -1;
	}
	/* send pasv */
	tcp_fprintf(N, sock, "PASV\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "227", 3) != 0) {
		return -1;
	}
	if (get_pasvaddr(iobuf, ipbuf, &port) != 0) {
		return -1;
	}
	/* send stor */
	tcp_fprintf(N, sock, "STOR %s\r\n", cobj1->val->d.str);
	nc_memset((char *)&sock2, 0, sizeof(sock2));
	if ((rc = tcp_connect(N, &sock2, ipbuf, port, sock->use_tls)) < 0) {
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		return -1;
	}
	if ((fd = open(cobj2->val->d.str, O_RDONLY | O_BINARY)) == -1) {
		n_warn(N, __FN__, "could not open file");
		return -1;
	}
	do {
		r = read(fd, iobuf, sizeof(iobuf) - 1);
		if (r < 1) break;
		rc = tcp_send(N, &sock2, iobuf, r, 0);
		if (rc < 1) break;
		sent += rc;
	} while (rc > 0);
	close(fd);
	tcp_close(N, &sock2, 1);
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "150", 3) != 0) {
		return -1;
	}
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		nsp_setstr(N, thisobj, "response", iobuf, -1);
		if (N->debug) { striprn(iobuf); n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "226", 3) != 0) {
		return -1;
	}
	nsp_setnum(N, &N->r, "", sent);
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_net_ftp_client_client)
{
#define __FN__ __FILE__ ":libnsp_net_ftp_client_client()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	//obj_t *cobj;

	nsp_setbool(N, thisobj, "socket", 0);
	nsp_setstr(N, thisobj, "host", "localhost", 9);
	nsp_setnum(N, thisobj, "port", 21);
	nsp_setstr(N, thisobj, "username", "anonymous", 9);
	nsp_setstr(N, thisobj, "password", "anonymous", 9);
	nsp_setbool(N, thisobj, "pasv", 1);
	nsp_setbool(N, thisobj, "use_mlsd", 0);
	nsp_setstr(N, thisobj, "body", "", 0);
	nsp_setstr(N, thisobj, "response", "", 0);
	//cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "net"), "ftp"), "client");
	//if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	//else n_warn(N, __FN__, "net.ftp.client not found");
	return 0;
#undef __FN__
}
