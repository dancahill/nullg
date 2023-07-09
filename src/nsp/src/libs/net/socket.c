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
#include "net.h"

NSP_FUNCTION(libnsp_net_socket_accept)
{
#define __FN__ __FILE__ ":libnsp_net_socket_accept()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t tobj;
	TCP_SOCKET *asock, *bsock;
	uint64 rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	bsock = (TCP_SOCKET *)cobj->val->d.str;
	if ((asock = n_alloc(N, sizeof(TCP_SOCKET) + 1, 1)) == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	nc_strncpy(asock->obj_type, "sock4", sizeof(asock->obj_type) - 1);
	asock->obj_term = (NSP_CFREE)tcp_murder;
	if ((rc = tcp_accept(N, bsock, asock)) < 0) {
		n_free(N, (void *)&asock, sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	/* build a new tcp socket 'object' to return */
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_setvaltype(N, &tobj, NT_TABLE);
	cobj = nsp_settable(N, &tobj, "this");
	cobj->val->attr &= ~NST_AUTOSORT;
	//cobj->val->attr|=NST_HIDDEN;
	nsp_linkval(N, cobj, &tobj);
	cobj = nsp_setcdata(N, &tobj, "_socket", NULL, 0);
	cobj->val->d.str = (void *)asock;
	cobj->val->size = sizeof(TCP_SOCKET) + 1;
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "net"), "socket");
	if (nsp_istable(cobj)) nsp_zlink(N, &tobj, cobj);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_bind)
{
#define __FN__ __FILE__ ":libnsp_net_socket_bind()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1"); /* host */
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2"); /* port */
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3"); /* SSL */
	TCP_SOCKET *bsock;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	bsock = (TCP_SOCKET *)cobj->val->d.str;
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	if ((rc = tcp_bind(N, bsock, cobj1->val->d.str, (unsigned short)cobj2->val->d.num)) < 0) {
		n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
		cobj->val->size = 0;
		nsp_setstr(N, &N->r, "", "tcp error", -1);
		return -1;
	}
	bsock->socket = rc;
	bsock->use_tls = 0;
	if (nsp_tobool(N, cobj3)) {
#ifdef HAVE_TLS
		obj_t *tobj = nsp_getobj(N, &N->context->l, "4"); /* ssl opts */
		char *pc = NULL, *pk = NULL, *cf = NULL;

		if (nsp_istable(tobj)) {
			cobj1 = nsp_getobj(N, tobj, "certfile");
			cobj2 = nsp_getobj(N, tobj, "keyfile");
			cobj3 = nsp_getobj(N, tobj, "chainfile");
			if (cobj1->val->type == NT_STRING && cobj2->val->type == NT_STRING && cobj1->val->size > 0 && cobj2->val->size > 0) {
				pc = cobj1->val->d.str;
				pk = cobj2->val->d.str;
			}
			if (nsp_isstr(cobj3) && cobj3->val->size > 0) {
				cf = cobj3->val->d.str;
			}
		}
		rc = _tls_init(N, bsock, 1, pc, pk, cf);
		bsock->use_tls = 1;
#else
		n_error(N, NE_SYNTAX, __FN__, "SSL is not available");
#endif
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_close)
{
#define __FN__ __FILE__ ":libnsp_net_socket_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;

	if (!nsp_istable(thisobj) || nsp_isnull(nsp_getobj(N, thisobj, "_socket")))
		thisobj = nsp_getobj(N, &N->context->l, "1");
	if (!nsp_istable(thisobj))
		return 0;
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		return 0; //n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	tcp_close(N, sock, 1);
	n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
	cobj->val->size = 0;
	cobj->val->d.num = 0;
	cobj->val->type = NT_BOOLEAN;
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_connect)
{
#define __FN__ __FILE__ ":libnsp_net_socket_connect()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1"); /* host */
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2"); /* port */
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3"); /* SSL  */
	obj_t *cobj;
	unsigned short use_tls = 0;
	TCP_SOCKET *sock;
	int rc;

	if (!nsp_istable(thisobj) || nsp_isnull(nsp_getobj(N, thisobj, "_socket"))) {
		thisobj = nsp_getobj(N, &N->context->l, "1");
		cobj1 = nsp_getobj(N, &N->context->l, "2"); /* host */
		cobj2 = nsp_getobj(N, &N->context->l, "3"); /* port */
		cobj3 = nsp_getobj(N, &N->context->l, "4"); /* SSL  */
	}
	if (!nsp_istable(thisobj))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (cobj1->val->type != NT_STRING || cobj1->val->d.str == NULL)
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_NUMBER)
		n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	if (cobj3->val->type == NT_BOOLEAN || cobj3->val->type == NT_NUMBER)
		use_tls = cobj3->val->d.num ? 1 : 0;
	if (use_tls) {
#ifndef HAVE_TLS
		n_error(N, NE_SYNTAX, __FN__, "SSL is not available");
#endif
	}
	if ((rc = tcp_connect(N, sock, cobj1->val->d.str, (unsigned short)cobj2->val->d.num, use_tls)) < 0) {
		n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
		cobj->val->size = 0;
		nsp_setbool(N, &N->r, "", 0);
		return -1;
	}
	nsp_setbool(N, &N->r, "", 1);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_gets)
{
#define __FN__ __FILE__ ":libnsp_net_socket_gets()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *buf;

	if (!nsp_istable(thisobj) || nsp_isnull(nsp_getobj(N, thisobj, "_socket"))) {
		thisobj = nsp_getobj(N, &N->context->l, "1");
	}
	if (!nsp_istable(thisobj))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	buf = n_alloc(N, MAX_TCP_READ_SIZE, 1);
	rc = tcp_fgets(N, sock, buf, MAX_TCP_READ_SIZE - 1);
	if (rc > -1) {
		striprn(buf);
		nsp_setstr(N, &N->r, "", buf, -1);
	} else {
		nsp_setnum(N, &N->r, "", rc);
	}
	n_free(N, (void *)&buf, MAX_TCP_READ_SIZE);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_gettype)
{
#define __FN__ __FILE__ ":libnsp_net_socket_gettype()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		nsp_setstr(N, &N->r, "", "table", -1);
	else
		nsp_setstr(N, &N->r, "", "sock4", -1);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_info)
{
#define __FN__ __FILE__ ":libnsp_net_socket_info()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t tobj;
	TCP_SOCKET *sock;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	nsp_setnum(N, &tobj, "bytes_in", sock->bytes_in);
	nsp_setnum(N, &tobj, "bytes_out", sock->bytes_out);
	nsp_setnum(N, &tobj, "ctime", (num_t)sock->ctime);
	nsp_setnum(N, &tobj, "mtime", (num_t)sock->mtime);
	nsp_setstr(N, &tobj, "local_addr", sock->LocalAddr, -1);
	nsp_setnum(N, &tobj, "local_port", sock->LocalPort);
	nsp_setstr(N, &tobj, "remote_addr", sock->RemoteAddr, -1);
	nsp_setnum(N, &tobj, "remote_port", sock->RemotePort);
	cobj = nsp_setnum(N, &tobj, "use_tls", sock->use_tls ? 1 : 0);
	cobj->val->type = NT_BOOLEAN;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_read)
{
#define __FN__ __FILE__ ":libnsp_net_socket_read()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;
	char *buf;
	char *p;

	if (!nsp_istable(thisobj) || nsp_isnull(nsp_getobj(N, thisobj, "_socket"))) {
		thisobj = nsp_getobj(N, &N->context->l, "1");
	}
	if (!nsp_istable(thisobj))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	buf = n_alloc(N, MAX_TCP_READ_SIZE, 1);
	if (sock->recvbufsize > 0) {
		p = sock->recvbuf + sock->recvbufoffset;
		rc = sock->recvbufsize;
		sock->recvbufoffset = 0;
		sock->recvbufsize = 0;
	} else {
		p = buf;
		rc = tcp_recv(N, sock, buf, MAX_TCP_READ_SIZE - 1, 0);
	}
	if (rc > -1) nsp_setstr(N, &N->r, "", p, rc);
	n_free(N, (void *)&buf, MAX_TCP_READ_SIZE);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_setsockopt)
{
#define __FN__ __FILE__ ":libnsp_net_socket_setsockopt()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj;
	TCP_SOCKET *sock;
	char *opt;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (!nsp_isstr(cobj1) || cobj1->val->d.str == NULL)
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	opt = cobj1->val->d.str;
	if (nc_strcmp(opt, "SO_RCVTIMEO") == 0) {
		if (!nsp_isnum(cobj2)) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
		{
#ifdef _WIN32
			DWORD timeout = (long)(cobj2->val->d.num);
			setsockopt(sock->socket, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout));
#else
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = (long)(cobj2->val->d.num) * 1000;
			setsockopt(sock->socket, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout));
#endif
		}
		{
			int lowat = 1;
			setsockopt(sock->socket, SOL_SOCKET, SO_RCVLOWAT, (void *)&lowat, sizeof(lowat));
		}
	} else if (nc_strcmp(opt, "SO_KEEPALIVE") == 0) {
		int keepalive;

		if (!nsp_isbool(cobj2) && !nsp_isnum(cobj2)) n_error(N, NE_SYNTAX, __FN__, "expected a boolean for arg2");
		keepalive = cobj2->val->d.num ? 1 : 0;
		setsockopt(sock->socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_socket)
{
#define __FN__ __FILE__ ":libnsp_net_socket_socket()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if ((sock = n_alloc(N, sizeof(TCP_SOCKET) + 1, 1)) == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(TCP_SOCKET) + 1);
		return -1;
	}
	nc_strncpy(sock->obj_type, "sock4", sizeof(sock->obj_type) - 1);
	sock->obj_term = (NSP_CFREE)tcp_murder;
	cobj = nsp_setcdata(N, thisobj, "_socket", NULL, 0);
	cobj->val->d.str = (void *)sock;
	cobj->val->size = sizeof(TCP_SOCKET) + 1;
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_socket_tlsaccept)
{
#define __FN__ __FILE__ ":libnsp_net_socket_tlsaccept()"
#ifdef HAVE_TLS
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *tobj = nsp_getobj(N, &N->context->l, "1"); /* ssl opts */
	obj_t *cobj;
	TCP_SOCKET *sock;
	char *pc = NULL, *pk = NULL, *cf = NULL;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (nsp_istable(tobj)) {
		obj_t *cobj1 = nsp_getobj(N, tobj, "certfile");
		obj_t *cobj2 = nsp_getobj(N, tobj, "keyfile");
		obj_t *cobj3 = nsp_getobj(N, tobj, "chainfile");
		if (cobj1->val->type == NT_STRING && cobj2->val->type == NT_STRING && cobj1->val->size > 0 && cobj2->val->size > 0) {
			pc = cobj1->val->d.str;
			pk = cobj2->val->d.str;
		}
		if (nsp_isstr(cobj3) && cobj3->val->size > 0) {
			cf = cobj3->val->d.str;
		}
	}
	_tls_init(N, sock, 1, pc, pk, cf);
	_tls_accept(N, sock, sock);
	sock->use_tls = 1;
#else
	n_error(N, NE_SYNTAX, __FN__, "SSL is not available");
#endif
#undef __FN__
	return 0;
}

NSP_FUNCTION(libnsp_net_socket_tlsconnect)
{
#define __FN__ __FILE__ ":libnsp_net_socket_tlsconnect()"
#ifdef HAVE_TLS
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TCP_SOCKET *sock;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sock4") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	_tls_connect(N, sock);
	sock->use_tls = 1;
#else
	n_error(N, NE_SYNTAX, __FN__, "SSL is not available");
#endif
#undef __FN__
	return 0;
}

NSP_FUNCTION(libnsp_net_socket_write)
{
#define __FN__ __FILE__ ":libnsp_net_socket_write()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj;
	TCP_SOCKET *sock;
	int rc;

	if (!nsp_istable(thisobj) || nsp_isnull(nsp_getobj(N, thisobj, "_socket"))) {
		thisobj = nsp_getobj(N, &N->context->l, "1");
		cobj1 = nsp_getobj(N, &N->context->l, "2");
	}
	if (!nsp_istable(thisobj))
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	cobj = nsp_getobj(N, thisobj, "_socket");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "sock4") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	if (cobj1->val->type != NT_STRING || cobj1->val->d.str == NULL)
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	rc = tcp_send(N, sock, cobj1->val->d.str, cobj1->val->size, 0);
	if (rc > -1) {
		nsp_setnum(N, &N->r, "", rc);
	} else {
		nsp_setnum(N, &N->r, "", rc);
	}
	return 0;
#undef __FN__
}
