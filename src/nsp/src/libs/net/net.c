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

int nspnet_register_all(nsp_state *N)
{
	obj_t *tobj, *tobj2;
#ifdef WIN32
	static WSADATA wsaData;
	if (WSAStartup(0x101, &wsaData)) return -1;
#endif

	tobj = nsp_settable(N, &N->g, "net");
	tobj->val->attr |= NST_HIDDEN;
#ifdef HAVE_TLS
	nsp_setbool(N, tobj, "have_tls", 1);
#if defined HAVE_OPENSSL
	nsp_setstr(N, tobj, "tls_type", "openssl", -1);
#elif defined HAVE_MBEDTLS
	nsp_setstr(N, tobj, "tls_type", "mbedtls", -1);
#endif
#else
	nsp_setbool(N, tobj, "have_tls", 0);
	nsp_setstr(N, tobj, "tls_type", "none", -1);
#endif
#ifdef HAVE_DNS
	tobj2 = nsp_settable(N, tobj, "dns");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "name2addr", (NSP_CFUNC)libnsp_net_dns_name2addr);
	nsp_setcfunc(N, tobj2, "addr2name", (NSP_CFUNC)libnsp_net_dns_addr2name);
#endif
	tobj2 = nsp_settable(N, tobj, "ftp");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "client", (NSP_CFUNC)libnsp_net_ftp_client);
	tobj2 = nsp_settable(N, tobj, "http");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "client", (NSP_CFUNC)libnsp_net_http_client);
	tobj2 = nsp_settable(N, tobj, "mime");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "read", (NSP_CFUNC)libnsp_net_mime_read);
	nsp_setcfunc(N, tobj2, "write", (NSP_CFUNC)libnsp_net_mime_write);
	nsp_setcfunc(N, tobj2, "decode_qp", (NSP_CFUNC)libnsp_net_mime_qp_decode);
	nsp_setcfunc(N, tobj2, "decode_rfc2047", (NSP_CFUNC)libnsp_net_mime_rfc2047_decode);
	tobj2 = nsp_settable(N, tobj, "smtp");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "client", (NSP_CFUNC)libnsp_net_smtp_client);
	tobj2 = nsp_settable(N, tobj, "pop3");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "client", (NSP_CFUNC)libnsp_net_pop3_client);

	tobj2 = nsp_settable(N, tobj, "socket");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "accept",     (NSP_CFUNC)libnsp_net_socket_accept);
	nsp_setcfunc(N, tobj2, "bind",       (NSP_CFUNC)libnsp_net_socket_bind);
	nsp_setcfunc(N, tobj2, "close",      (NSP_CFUNC)libnsp_net_socket_close);
	nsp_setcfunc(N, tobj2, "connect",    (NSP_CFUNC)libnsp_net_socket_connect);
	nsp_setcfunc(N, tobj2, "gets",       (NSP_CFUNC)libnsp_net_socket_gets);
	nsp_setcfunc(N, tobj2, "gettype",    (NSP_CFUNC)libnsp_net_socket_gettype);
	nsp_setcfunc(N, tobj2, "info",       (NSP_CFUNC)libnsp_net_socket_info);
	nsp_setcfunc(N, tobj2, "read",       (NSP_CFUNC)libnsp_net_socket_read);
	nsp_setcfunc(N, tobj2, "setsockopt", (NSP_CFUNC)libnsp_net_socket_setsockopt);
	nsp_setcfunc(N, tobj2, "socket",     (NSP_CFUNC)libnsp_net_socket_socket);
	nsp_setcfunc(N, tobj2, "tlsaccept",  (NSP_CFUNC)libnsp_net_socket_tlsaccept);
	nsp_setcfunc(N, tobj2, "tlsconnect", (NSP_CFUNC)libnsp_net_socket_tlsconnect);
	nsp_setcfunc(N, tobj2, "write",      (NSP_CFUNC)libnsp_net_socket_write);

	tobj2 = nsp_settable(N, tobj, "tnef");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "debug", (NSP_CFUNC)libnsp_net_tnef_debug);

	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspnet_register_all(N);
	return 0;
}
#endif
