/*
    NESLA NullLogic Embedded Scripting Language
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
#include "nsp/nsplib.h"
#include "net.h"
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#define snprintf _snprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <ctype.h>
#endif

NSP_CLASSMETHOD(libnsp_net_http_client_send)
{
#define __FN__ __FILE__ ":libnsp_net_http_client_send()"
	char tmpbuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET sock;
	unsigned short use_tls = 0;
	unsigned short port;
	char *host;
	char *uri;
	//	char *ctype;
	//	char *body;
	//	int blen=0;
	int cl = -1, len = 0, rc;
	obj_t tobj;
	obj_t *tobj2;
	char *p, *p1, *p2;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "host")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for host");
	host = cobj->val->d.str;
	if (!nsp_isnum((cobj = nsp_getobj(N, thisobj, "port")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for port");
	port = (unsigned short)cobj->val->d.num;
	if (nsp_isbool((cobj = nsp_getobj(N, thisobj, "use_tls")))) use_tls = cobj->val->d.num ? 1 : 0;

	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "uri")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for uri");
	uri = cobj->val->d.str;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "body")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for body");
	//body=cobj->val->d.str;
//	blen=cobj->val->size;
//	if (!nsp_isstr((cobj=nsp_getobj(N, thisobj, "contenttype")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for contenttype");
//	ctype=cobj->val->d.str;

	nc_memset((char *)&sock, 0, sizeof(sock));
	if ((rc = tcp_connect(N, &sock, host, port, use_tls)) < 0) {
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		return -1;
	}
	/* why does php insert random data when i try to use HTTP/1.1? */
	tcp_fprintf(N, &sock, "GET %s HTTP/1.0\r\nUser-Agent: NSP-HTTP-Client/" NSP_VERSION "\r\nConnection: Close\r\nHost: %s\r\nAccept: */*\r\n\r\n", uri, host);
	tobj.val = n_newval(N, NT_TABLE);
	rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	if (rc > 0) {
		cobj = nsp_setstr(N, &tobj, "status", tmpbuf, -1);
		while ((cobj->val->size > 0) && ((cobj->val->d.str[cobj->val->size - 1] == '\r') || (cobj->val->d.str[cobj->val->size - 1] == '\n'))) cobj->val->d.str[--cobj->val->size] = '\0';
		tobj2 = nsp_settable(N, &tobj, "stat");
		striprn(tmpbuf);
		p1 = tmpbuf;
		p2 = nc_strchr(tmpbuf, ' ');
		if ((*p1) && (*p2)) {
			*p2++ = '\0';
			while (nc_isspace(*p2)) p2++;
			nsp_setstr(N, tobj2, "a", p1, -1);
		}
		p1 = p2;
		p2 = nc_strchr(p2, ' ');
		if ((*p1) && (*p2)) {
			*p2++ = '\0';
			while (nc_isspace(*p2)) p2++;
			p = p1; while (nc_isdigit(*p)) p++;
			if (*p) {
				nsp_setstr(N, tobj2, "b", p1, -1);
			}
			else {
				nsp_setnum(N, tobj2, "b", atoi(p1));
			}
			nsp_setstr(N, tobj2, "c", p2, -1);
		}
	}
	cobj = nsp_setstr(N, &tobj, "head", NULL, 0);
	tobj2 = nsp_settable(N, &tobj, "headers");
	for (;;) {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
		if (rc < 1) break;
		/* slow, but at least it's safe */
		nsp_strcat(N, cobj, tmpbuf, rc);
		striprn(tmpbuf);
		if (strlen(tmpbuf) < 1) break;
		/* printf("[%s]\r\n", tmpbuf); */
		p1 = tmpbuf;
		p2 = nc_strchr(tmpbuf, ':');
		if ((*p1) && (*p2)) {
			*p2++ = '\0';
			for (p = p1;*p;p++) *p = nc_tolower(*p);
			while (nc_isspace(*p2)) p2++;
			/* printf("[%s][%s]\n", p1, p2); */
			if (nc_strcmp(p1, "content-length") == 0) {
				p = p2; while (nc_isdigit(*p)) p++;
				if (*p) {
					nsp_setstr(N, tobj2, p1, p2, -1);
				}
				else {
					cl = atoi(p2);
					nsp_setnum(N, tobj2, p1, cl);
				}
			}
			else {
				nsp_setstr(N, tobj2, p1, p2, -1);
			}
		}
	}
	while ((cobj->val->size > 0) && ((cobj->val->d.str[cobj->val->size - 1] == '\r') || (cobj->val->d.str[cobj->val->size - 1] == '\n'))) cobj->val->d.str[--cobj->val->size] = '\0';
	cobj = nsp_setstr(N, &tobj, "body", NULL, 0);
	for (;;) {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
		if (rc<1) break;
		len += rc;
		/* slow, but at least it's safe */
		nsp_strcat(N, cobj, tmpbuf, rc);
		striprn(tmpbuf);
		/* printf("{%s}\r\n", tmpbuf); */
		if ((cl>-1) && (len >= cl)) break;
	}
	tcp_close(N, &sock, 1);
	if (N->debug) n_warn(N, __FN__, "Content-Length: %d/%d", len, cl);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_net_http_client)
{
#define __FN__ __FILE__ ":libnsp_net_http_client()"
	obj_t *cobj;//, *cobj2;
	char *p1, *p2;
	int port = 80;

	nsp_setcfunc(N, &N->l, "send", (NSP_CFUNC)libnsp_net_http_client_send);

	nsp_setbool(N, &N->l, "use_tls", 0);
	nsp_setstr(N, &N->l, "host", "localhost", 9);
	nsp_setnum(N, &N->l, "port", port);
	nsp_setstr(N, &N->l, "uri", "", 0);

	nsp_setstr(N, &N->l, "method", "GET", 3);
	nsp_setstr(N, &N->l, "protocol", "HTTP/1.0", 8);

	nsp_setstr(N, &N->l, "body", "", 0);

	/* parse constructor args */
	if (!nsp_isstr((cobj = nsp_getobj(N, &N->l, "1"))) || cobj->val->d.str == NULL) {
		return 0;
	}
	p1 = cobj->val->d.str;
	if (strncasecmp(p1, "http://", 7) == 0) {
		//		n_warn(N, __FN__, "protocol is http://");
		nsp_setbool(N, &N->l, "use_tls", 0);
		port = 80;
		p1 += 7;
	}
	else if (strncasecmp(p1, "https://", 8) == 0) {
		//		n_warn(N, __FN__, "protocol is https://");
		nsp_setbool(N, &N->l, "use_tls", 1);
		port = 443;
		p1 += 8;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "can't parse protocol");
	}
	p2 = p1;
	while (isalnum(*p2) || *p2 == '-' || *p2 == '_' || *p2 == '.') p2++;
	if (p2 > p1) {
		//		n_warn(N, __FN__, "host is %s", nsp_tostr(N, cobj2));
		nsp_setstr(N, &N->l, "host", p1, p2 - p1);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "can't parse host");
	}
	p1 = p2;
	if (*p1 == ':') {
		p1++;
		port = 0;
		while (isdigit(*p1)) {
			port = port * 10 + (*p1++ - '0');
		}
		if (port) {
			//			n_warn(N, __FN__, "port is %d", port);
			nsp_setnum(N, &N->l, "port", port);
		}
		else {
			n_error(N, NE_SYNTAX, __FN__, "can't parse port");
		}
	}
	p2 = p1;
	while (*p2) p2++;
	if (p2 > p1) {
		//		n_warn(N, __FN__, "uri is %s", nsp_tostr(N, cobj2));
		nsp_setstr(N, &N->l, "uri", p1, p2 - p1);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "can't parse uri");
	}
	return 0;
#undef __FN__
}
