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
#ifdef WIN32
#endif

static void pop3_lasterr(nsp_state *N, char *msg)
{
	nsp_setstr(N, nsp_settable(N, nsp_settable(N, &N->g, "net"), "pop3"), "last_err", msg, -1);
	return;
}

NSP_CLASSMETHOD(libnsp_net_pop3_open)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_open()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	unsigned short use_tls = 0;
	unsigned short port = 0;
	char *host = NULL;
	char *user = NULL;
	char *pass = NULL;
	int rc;

	nsp_setbool(N, &N->r, "", 0);
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
	if (nsp_isbool((cobj = nsp_getobj(N, thisobj, "use_tls")))) use_tls = cobj->val->d.num ? 1 : 0;
	sock = n_alloc(N, sizeof(TCP_SOCKET) + 1, 1);
	if (sock == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	nc_strncpy(sock->obj_type, "sock4", sizeof(sock->obj_type) - 1);
	sock->obj_term = (NSP_CFREE)tcp_murder;
	if ((rc = tcp_connect(N, sock, host, port, use_tls)) < 0) {
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		n_error(N, NE_SYNTAX, __FN__, "cannot connect to %s:%d", host, port);
		return -1;
	}
	/* welcome dialog */
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
			tcp_close(N, sock, 1);
			n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
			return -1;
		}
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	if (nc_strncmp(iobuf, "+OK", 3) != 0) {
		pop3_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	/* send username */
	tcp_fprintf(N, sock, "USER %s\r\n", user);
	//	do {
	if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
		tcp_close(N, sock, 1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	striprn(iobuf);
	if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
	//	} while (iobuf[3]!=' '&&iobuf[3]!='\0');
	if (nc_strncmp(iobuf, "+OK", 3) != 0) {
		pop3_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	/* send password */
	tcp_fprintf(N, sock, "PASS %s\r\n", pass);
	//	do {
	if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
		tcp_close(N, sock, 1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	striprn(iobuf);
	if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
	//	} while (iobuf[3]!=' '&&iobuf[3]!='\0');
	if (nc_strncmp(iobuf, "+OK", 3) != 0) {
		pop3_lasterr(N, iobuf);
		tcp_close(N, sock, 1);
		n_free(N, (void *)&sock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "socket", NULL, 0);
	cobj->val->d.str = (void *)sock;
	cobj->val->size = sizeof(TCP_SOCKET) + 1;
	nsp_setbool(N, &N->r, "", 1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_close)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_close()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* send close */
	tcp_fprintf(N, sock, "QUIT\r\n");
	do {
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
	} while (iobuf[3] != ' '&&iobuf[3] != '\0');
	tcp_close(N, sock, 1);
	n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "socket", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_stat)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_stat()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *p;
	unsigned count, size;
	obj_t tobj;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* stat */
	tcp_fprintf(N, sock, "STAT\r\n");
	if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
	striprn(iobuf);
	if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
	if (nc_strncmp(iobuf, "+OK", 3) != 0) {
		pop3_lasterr(N, iobuf);
		return -1;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	p = iobuf;
	while ((*p) && (*p != ' ')) p++;
	if (*p) p++;
	count = atoi(p);
	nsp_setnum(N, &tobj, "count", count);
	while ((*p) && (*p != ' ')) p++;
	if (*p) p++;
	size = atoi(p);
	nsp_setnum(N, &tobj, "size", size);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_uidl)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_uidl()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *p;
	unsigned short msg;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		msg = (unsigned short)nsp_tonum(N, cobj);
		tcp_fprintf(N, sock, "UIDL %d\r\n", msg);
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		p = iobuf;
		while ((*p) && (*p != ' ')) p++;
		p++;
		while ((*p) && (*p != ' ')) p++;
		p++;
		nsp_setstr(N, &N->r, "", p, -1);
	}
	else {
		obj_t tobj;
		unsigned msg;
		char numbuf[8];

		tcp_fprintf(N, sock, "UIDL\r\n");
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		nc_memset((void *)&tobj, 0, sizeof(obj_t));
		tobj.val = n_newval(N, NT_TABLE);
		tobj.val->attr &= ~NST_AUTOSORT;
		do {
			if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
				nsp_unlinkval(N, &tobj);
				return -1;
			}
			striprn(iobuf);
			if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
			if (nc_strcmp(iobuf, ".") == 0) break;
			p = iobuf;
			msg = atoi(p);
			while ((*p) && (*p != ' ')) p++;
			if (*p) p++;
			nsp_setstr(N, &tobj, n_ntoa(N, numbuf, msg, 10, 0), p, -1);
		} while (nc_strcmp(iobuf, ".") != 0);
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_top)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_top()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	unsigned int msg;
	unsigned int lines = 0;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		msg = (unsigned short)nsp_tonum(N, cobj);
		if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) {
			lines = (unsigned short)nsp_tonum(N, cobj);
		}
		tcp_fprintf(N, sock, "TOP %d %d\r\n", msg, lines);
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		cobj = nsp_setstr(N, &N->r, "", NULL, 0);
		do {
			if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
				nsp_unlinkval(N, cobj);
				return -1;
			}
			if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
			if (iobuf[0] == '.') {
				if ((nc_strcmp(iobuf, ".\r\n") == 0) || (nc_strcmp(iobuf, ".\n") == 0)) {
					break;
				}
				else if (iobuf[1] == '.') {
					nsp_strcat(N, cobj, iobuf + 1, -1);
					continue;
				}
			}
			nsp_strcat(N, cobj, iobuf, -1);
		} while (nc_strcmp(iobuf, ".") != 0);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for msg");
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_retr)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_retr()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	unsigned msg;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		msg = (unsigned short)nsp_tonum(N, cobj);
		tcp_fprintf(N, sock, "RETR %d\r\n", msg);
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		cobj = nsp_setstr(N, &N->r, "", NULL, 0);
		do {
			if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
				nsp_unlinkval(N, cobj);
				return -1;
			}
			if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
			if (iobuf[0] == '.') {
				if ((nc_strcmp(iobuf, ".\r\n") == 0) || (nc_strcmp(iobuf, ".\n") == 0)) {
					break;
				}
				else if (iobuf[1] == '.') {
					nsp_strcat(N, cobj, iobuf + 1, -1);
					continue;
				}
			}
			nsp_strcat(N, cobj, iobuf, -1);
		} while (nc_strcmp(iobuf, ".") != 0);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for msg");
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_dele)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_dele()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *p;
	unsigned short msg;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	/* dele */
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		msg = (unsigned short)nsp_tonum(N, cobj);
		tcp_fprintf(N, sock, "DELE %d\r\n", msg);
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		p = iobuf;
		while ((*p) && (*p != ' ')) p++;
		p++;
		nsp_setnum(N, &N->r, "", atoi(p));
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for msg");
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_pop3_list)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_list()"
	char iobuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *p;
	unsigned short msg;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		msg = (unsigned short)nsp_tonum(N, cobj);
		tcp_fprintf(N, sock, "LIST %d\r\n", msg);
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		p = iobuf;
		while ((*p) && (*p != ' ')) p++;
		p++;
		while ((*p) && (*p != ' ')) p++;
		p++;
		nsp_setnum(N, &N->r, "", atoi(p));
	}
	else {
		obj_t tobj;
		unsigned msg, size;
		char numbuf[8];

		tcp_fprintf(N, sock, "LIST\r\n");
		if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) return -1;
		striprn(iobuf);
		if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
		if (nc_strncmp(iobuf, "+OK", 3) != 0) {
			pop3_lasterr(N, iobuf);
			return -1;
		}
		nc_memset((void *)&tobj, 0, sizeof(obj_t));
		tobj.val = n_newval(N, NT_TABLE);
		tobj.val->attr &= ~NST_AUTOSORT;
		do {
			if ((rc = tcp_fgets(N, sock, iobuf, sizeof(iobuf) - 1)) < 0) {
				nsp_unlinkval(N, &tobj);
				return -1;
			}
			striprn(iobuf);
			if (N->debug) { n_warn(N, __FN__, "got %s", iobuf); }
			if (nc_strcmp(iobuf, ".") == 0) break;
			p = iobuf;
			msg = atoi(p);
			while ((*p) && (*p != ' ')) p++;
			if (*p) p++;
			size = atoi(p);
			nsp_setnum(N, &tobj, n_ntoa(N, numbuf, msg, 10, 0), size);
		} while (nc_strcmp(iobuf, ".") != 0);
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
	}
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_net_pop3_client)
{
#define __FN__ __FILE__ ":libnsp_net_pop3_client()"
	nsp_setcfunc(N, &N->l, "open", (NSP_CFUNC)libnsp_net_pop3_open);
	nsp_setcfunc(N, &N->l, "close", (NSP_CFUNC)libnsp_net_pop3_close);
	nsp_setcfunc(N, &N->l, "stat", (NSP_CFUNC)libnsp_net_pop3_stat);
	nsp_setcfunc(N, &N->l, "uidl", (NSP_CFUNC)libnsp_net_pop3_uidl);
	nsp_setcfunc(N, &N->l, "top", (NSP_CFUNC)libnsp_net_pop3_top);
	nsp_setcfunc(N, &N->l, "retr", (NSP_CFUNC)libnsp_net_pop3_retr);
	nsp_setcfunc(N, &N->l, "dele", (NSP_CFUNC)libnsp_net_pop3_dele);
	nsp_setcfunc(N, &N->l, "list", (NSP_CFUNC)libnsp_net_pop3_list);
	nsp_setbool(N, &N->l, "socket", 0);
	nsp_setstr(N, &N->l, "host", "localhost", 9);
	nsp_setnum(N, &N->l, "port", 110);
	nsp_setbool(N, &N->l, "use_tls", 0);
	nsp_setstr(N, &N->l, "username", "anonymous", 9);
	nsp_setstr(N, &N->l, "password", "anonymous", 9);
	return 0;
#undef __FN__
}
