/*
    Null httpd -- simple http server
    Copyright (C) 2001-2003 Dan Cahill

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

#ifdef HAVE_LIBSSL

static SSL_CTX *ctx;
static SSL_METHOD *meth;

int ssl_init()
{
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	meth=SSLv23_server_method();
	ctx=SSL_CTX_new(meth);
	if (!ctx) {
		log_error("core", __FILE__, __LINE__, 0, "SSL Error");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	if (SSL_CTX_use_certificate_file(ctx, "../etc/cert.pem", SSL_FILETYPE_PEM)<=0) {
		log_error("core",  __FILE__, __LINE__, 0, "SSL Error loading cert.pem");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, "../etc/priv.pem", SSL_FILETYPE_PEM)<=0) {
		log_error("core", __FILE__, __LINE__, 0, "SSL Error loading priv.pem");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	if (!SSL_CTX_check_private_key(ctx)) {
		log_error("core", __FILE__, __LINE__, 0, "Private key does not match the certificate public key");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	log_error("core", __FILE__, __LINE__, 2, "ssl_init() completed");
	return 0;
}

int ssl_accept(TCP_SOCKET *sock)
{
	if ((sock->ssl=SSL_new(ctx))==NULL) {
		return -1;
	}
	SSL_set_fd(sock->ssl, sock->socket);
	if (SSL_accept(sock->ssl)==-1) {
		return -1;
	}
	return 0;
}

int ssl_close(TCP_SOCKET *sock)
{
	if (sock->ssl!=NULL) {
		SSL_free(sock->ssl);
		sock->ssl=NULL;
	}
	return 0;
}

int ssl_shutdown()
{
	SSL_CTX_free(ctx);
	return 0;
}

#endif /* HAVE_LIBSSL */
