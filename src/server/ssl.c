/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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

#ifdef HAVE_SSL

typedef void(*LIBSSL_LOAD_ERROR_STRINGS)(void);
typedef int(*LIBSSL_LIBRARY_INIT)(void);
typedef SSL_METHOD *(*LIBSSL_CLIENT_METHOD)(void);
typedef SSL_METHOD *(*LIBSSL_SERVER_METHOD)(void);
typedef SSL_CTX    *(*LIBSSL_CTX_NEW)(SSL_METHOD *);
typedef long(*LIBSSL_CTX_CTRL)(SSL_CTX *, int, long, void *);
typedef int(*LIBSSL_CTX_SET_CIPHER_LIST)(SSL_CTX *, const char *);
typedef int(*LIBSSL_CTX_USE_CERT_FILE)(SSL_CTX *, const char *, int);
typedef int(*LIBSSL_CTX_USE_KEY_FILE)(SSL_CTX *, const char *, int);
typedef int(*LIBSSL_CTX_USE_CERTIFICATE_CHAIN_FILE)(SSL_CTX *, const char *, int);
typedef int(*LIBSSL_CTX_CHECK_KEY)(SSL_CTX *);
typedef SSL *(*LIBSSL_NEW)(SSL_CTX *);
typedef int(*LIBSSL_SET_FD)(SSL *, int);
typedef int(*LIBSSL_ACCEPT)(SSL *);
typedef int(*LIBSSL_CONNECT)(SSL *);
typedef void(*LIBSSL_FREE)(SSL *);
typedef int(*LIBSSL_GET_SHUTDOWN)(SSL *);
typedef int(*LIBSSL_SHUTDOWN)(SSL *);
typedef int(*LIBSSL_CLEAR)(SSL *);
typedef void(*LIBSSL_CTX_FREE)(SSL_CTX *);
typedef int(*LIBSSL_READ)(SSL *, void *, int);
typedef int(*LIBSSL_WRITE)(SSL *, const void *, int);

typedef EC_KEY *(*LIBSSL_EC_KEY_NEW_BY_CURVE_NAME)(int);
typedef void (*LIBSSL_EC_KEY_FREE)(EC_KEY *);

static struct {
	LIBSSL_ACCEPT				SSL_accept;
	LIBSSL_CLEAR				SSL_clear;
	LIBSSL_CONNECT				SSL_connect;
	LIBSSL_CTX_CHECK_KEY			SSL_CTX_check_private_key;
	LIBSSL_CTX_FREE				SSL_CTX_free;
	LIBSSL_CTX_NEW				SSL_CTX_new;
	LIBSSL_CTX_CTRL				SSL_CTX_ctrl;
	LIBSSL_CTX_SET_CIPHER_LIST		SSL_CTX_set_cipher_list;
	LIBSSL_CTX_USE_CERT_FILE		SSL_CTX_use_certificate_file;
	LIBSSL_CTX_USE_KEY_FILE			SSL_CTX_use_PrivateKey_file;
	LIBSSL_CTX_USE_CERTIFICATE_CHAIN_FILE	SSL_CTX_use_certificate_chain_file;
	LIBSSL_FREE				SSL_free;
	LIBSSL_GET_SHUTDOWN			SSL_get_shutdown;
	LIBSSL_LIBRARY_INIT			SSL_library_init;
	LIBSSL_LOAD_ERROR_STRINGS		SSL_load_error_strings;
	LIBSSL_NEW				SSL_new;
	LIBSSL_READ				SSL_read;
	LIBSSL_SET_FD				SSL_set_fd;
	LIBSSL_SHUTDOWN				SSL_shutdown;
	LIBSSL_CLIENT_METHOD			SSLv23_client_method;
	LIBSSL_SERVER_METHOD			SSLv23_server_method;
	//	LIBSSL_CLIENT_METHOD		TLS_client_method;
	//	LIBSSL_SERVER_METHOD		TLS_server_method;
	LIBSSL_WRITE				SSL_write;
	LIBSSL_EC_KEY_NEW_BY_CURVE_NAME		EC_KEY_new_by_curve_name;
	LIBSSL_EC_KEY_FREE			EC_KEY_free;
} libssl;

int ssl_dl_init()
{
	obj_t *tobj = nsp_getobj(proc.N, &proc.N->g, "CONFIG");
#ifdef HAVE_SSL
	static int isloaded = 0;
#ifdef WIN32
	HINSTANCE hinstLib;
	char *libext = "dll";
#else
	void *hinstLib;
	char *libext = "so";
#endif
	char libname[255];

	if (isloaded) return 0;
	if ((strlen(nsp_getstr(proc.N, tobj, "ssl_cert")) == 0) || (strlen(nsp_getstr(proc.N, tobj, "ssl_key")) == 0)) return -1;
	memset(libname, 0, sizeof(libname));
	/*
	 * Look for a local copy or symlink first before hunting.
	 */
#ifdef WIN32
	snprintf(libname, sizeof(libname) - 1, "%s/libeay32.%s", nsp_getstr(proc.N, nsp_settable(proc.N, tobj, "paths"), "lib"), libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) {
		snprintf(libname, sizeof(libname) - 1, "%s/ssleay32.%s", nsp_getstr(proc.N, nsp_settable(proc.N, tobj, "paths"), "lib"), libext);
		fixslashes(libname);
		if ((hinstLib = lib_open(libname)) != NULL) goto found;
	}
	else {
		goto fail;
	}
	snprintf(libname, sizeof(libname) - 1, "libeay32.%s", libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) {
		snprintf(libname, sizeof(libname) - 1, "ssleay32.%s", libext);
		fixslashes(libname);
		if ((hinstLib = lib_open(libname)) != NULL) goto found;
	}
	else {
		goto fail;
	}
#else

	 /* openbsd might like this more */
	snprintf(libname, sizeof(libname) - 1, "%s/libcrypto.%s", nsp_getstr(proc.N, nsp_settable(proc.N, tobj, "paths"), "lib"), libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) {
		snprintf(libname, sizeof(libname) - 1, "%s./libssl.%s", nsp_getstr(proc.N, nsp_settable(proc.N, tobj, "paths"), "lib"), libext);
		fixslashes(libname);
		if ((hinstLib = lib_open(libname)) != NULL) {
			goto found;
		}
		else {
			goto fail;
		}
	}
	snprintf(libname, sizeof(libname) - 1, "%s/libssl.%s", nsp_getstr(proc.N, nsp_settable(proc.N, tobj, "paths"), "lib"), libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) goto found;
	snprintf(libname, sizeof(libname) - 1, "libssl.%s", libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) goto found;
	snprintf(libname, sizeof(libname) - 1, "libssl.%s.1.0.0", libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) goto found;
	snprintf(libname, sizeof(libname) - 1, "libssl.%s.0.9.8", libext);
	fixslashes(libname);
	if ((hinstLib = lib_open(libname)) != NULL) goto found;
#endif
	goto fail;
found:
	if ((libssl.SSL_accept = (LIBSSL_ACCEPT)lib_sym(hinstLib, "SSL_accept")) == NULL) goto fail;
	if ((libssl.SSL_clear = (LIBSSL_CLEAR)lib_sym(hinstLib, "SSL_clear")) == NULL) goto fail;
	if ((libssl.SSL_connect = (LIBSSL_CONNECT)lib_sym(hinstLib, "SSL_connect")) == NULL) goto fail;
	if ((libssl.SSL_CTX_check_private_key = (LIBSSL_CTX_CHECK_KEY)lib_sym(hinstLib, "SSL_CTX_check_private_key")) == NULL) goto fail;
	if ((libssl.SSL_CTX_free = (LIBSSL_CTX_FREE)lib_sym(hinstLib, "SSL_CTX_free")) == NULL) goto fail;
	if ((libssl.SSL_CTX_new = (LIBSSL_CTX_NEW)lib_sym(hinstLib, "SSL_CTX_new")) == NULL) goto fail;
	if ((libssl.SSL_CTX_ctrl = (LIBSSL_CTX_CTRL)lib_sym(hinstLib, "SSL_CTX_ctrl")) == NULL) goto fail;
	if ((libssl.SSL_CTX_set_cipher_list = (LIBSSL_CTX_SET_CIPHER_LIST)lib_sym(hinstLib, "SSL_CTX_set_cipher_list")) == NULL) goto fail;
	if ((libssl.SSL_CTX_use_certificate_file = (LIBSSL_CTX_USE_CERT_FILE)lib_sym(hinstLib, "SSL_CTX_use_certificate_file")) == NULL) goto fail;
	if ((libssl.SSL_CTX_use_PrivateKey_file = (LIBSSL_CTX_USE_KEY_FILE)lib_sym(hinstLib, "SSL_CTX_use_PrivateKey_file")) == NULL) goto fail;
	if ((libssl.SSL_CTX_use_certificate_chain_file = (LIBSSL_CTX_USE_CERTIFICATE_CHAIN_FILE)lib_sym(hinstLib, "SSL_CTX_use_certificate_chain_file")) == NULL) goto fail;
	if ((libssl.SSL_free = (LIBSSL_FREE)lib_sym(hinstLib, "SSL_free")) == NULL) goto fail;
	if ((libssl.SSL_get_shutdown = (LIBSSL_GET_SHUTDOWN)lib_sym(hinstLib, "SSL_get_shutdown")) == NULL) goto fail;
	if ((libssl.SSL_library_init = (LIBSSL_LIBRARY_INIT)lib_sym(hinstLib, "SSL_library_init")) == NULL) goto fail;
	if ((libssl.SSL_load_error_strings = (LIBSSL_LOAD_ERROR_STRINGS)lib_sym(hinstLib, "SSL_load_error_strings")) == NULL) goto fail;
	if ((libssl.SSL_new = (LIBSSL_NEW)lib_sym(hinstLib, "SSL_new")) == NULL) goto fail;
	if ((libssl.SSL_read = (LIBSSL_READ)lib_sym(hinstLib, "SSL_read")) == NULL) goto fail;
	if ((libssl.SSL_set_fd = (LIBSSL_SET_FD)lib_sym(hinstLib, "SSL_set_fd")) == NULL) goto fail;
	if ((libssl.SSL_shutdown = (LIBSSL_SHUTDOWN)lib_sym(hinstLib, "SSL_shutdown")) == NULL) goto fail;
	if ((libssl.SSLv23_client_method = (LIBSSL_CLIENT_METHOD)lib_sym(hinstLib, "SSLv23_client_method")) == NULL) goto fail;
	if ((libssl.SSLv23_server_method = (LIBSSL_SERVER_METHOD)lib_sym(hinstLib, "SSLv23_server_method")) == NULL) goto fail;
	//	if ((libssl.TLS_client_method            = (LIBSSL_CLIENT_METHOD)lib_sym(hinstLib, "TLS_client_method")) == NULL) goto fail;
	//	if ((libssl.TLS_server_method            = (LIBSSL_SERVER_METHOD)lib_sym(hinstLib, "TLS_server_method")) == NULL) goto fail;
	if ((libssl.SSL_write = (LIBSSL_WRITE)lib_sym(hinstLib, "SSL_write")) == NULL) goto fail;

	if ((libssl.EC_KEY_new_by_curve_name = (LIBSSL_EC_KEY_NEW_BY_CURVE_NAME)lib_sym(hinstLib, "EC_KEY_new_by_curve_name")) == NULL) goto fail;
	if ((libssl.EC_KEY_free = (LIBSSL_EC_KEY_FREE)lib_sym(hinstLib, "EC_KEY_free")) == NULL) goto fail;

	isloaded = 1;
	log_error(proc.N, "core", __FILE__, __LINE__, 4, "Loaded %s", libname);
	return 0;
fail:
	log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libssl, 0, sizeof(libssl));
	if (hinstLib != NULL) lib_close(hinstLib);
	hinstLib = NULL;
	return -1;
#else
	return -1;
#endif
}

int ssl_init()
{
	obj_t *tobj = nsp_getobj(proc.N, nsp_getobj(proc.N, &proc.N->g, "CONFIG"), "tls");
	obj_t *cobj0 = nsp_getobj(proc.N, tobj, "ciphers");
	obj_t *cobj1 = nsp_getobj(proc.N, tobj, "cert_file");
	obj_t *cobj2 = nsp_getobj(proc.N, tobj, "key_file");
	obj_t *cobj3 = nsp_getobj(proc.N, tobj, "chain_file");

	if (proc.ssl_is_loaded) return 0;
	//	if (!nsp_isstr(cobj1)) return 0;
	if (!nsp_isstr(cobj1) && !nsp_isstr(cobj2) && !nsp_isstr(cobj3)) return 0;
	//if ((cobj1->val->type!=NT_STRING)||(cobj2->val->type!=NT_STRING)) return 0;
	if (ssl_dl_init() < 0) return -1;
	libssl.SSL_load_error_strings();
	libssl.SSLeay_add_ssl_algorithms();
	proc.ssl_meth = libssl.SSLv23_server_method();
	proc.ssl_ctx = libssl.SSL_CTX_new(proc.ssl_meth);
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1_1;
	libssl.SSL_CTX_ctrl(proc.ssl_ctx, SSL_CTRL_OPTIONS, options, NULL);
#ifdef SSL_CTRL_SET_ECDH_AUTO
	libssl.SSL_CTX_ctrl(proc.ssl_ctx, SSL_CTRL_SET_ECDH_AUTO, 1, NULL);
#else
	EC_KEY *ecdh = libssl.EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (ecdh != NULL) {
		libssl.SSL_CTX_ctrl(proc.ssl_ctx, SSL_CTRL_SET_TMP_ECDH, 0, (char *)ecdh);
		libssl.EC_KEY_free(ecdh);
	}
#endif
	if (!proc.ssl_ctx) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "SSL Error");
		return -1;
	}
	if (nsp_isstr(cobj0)) {
		if (libssl.SSL_CTX_set_cipher_list(proc.ssl_ctx, nsp_tostr(proc.N, cobj0)) != 1) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "SSL_CTX_set_cipher_list failed '%s'", nsp_tostr(proc.N, cobj0));
			return -1;
		}
	}
	if (nsp_isstr(cobj1)) {
		if (libssl.SSL_CTX_use_certificate_file(proc.ssl_ctx, nsp_tostr(proc.N, cobj1), SSL_FILETYPE_PEM) <= 0) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "SSL Error loading certificate '%s'", nsp_tostr(proc.N, cobj1));
			return -1;
		}
	}
	if (nsp_isstr(cobj2)) {
		if (libssl.SSL_CTX_use_PrivateKey_file(proc.ssl_ctx, nsp_tostr(proc.N, cobj2), SSL_FILETYPE_PEM) <= 0) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "SSL Error loading private key '%s'", nsp_tostr(proc.N, cobj2));
			return -1;
		}
	}
	if (nsp_isstr(cobj3)) {
		if (libssl.SSL_CTX_use_certificate_chain_file(proc.ssl_ctx, nsp_tostr(proc.N, cobj3), SSL_FILETYPE_PEM) <= 0) {
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "SSL Error loading certificate chain '%s'", nsp_tostr(proc.N, cobj3));
			return -1;
		}
	}
	if (!libssl.SSL_CTX_check_private_key(proc.ssl_ctx)) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "Private key does not match the public certificate");
		return -1;
	}
	proc.ssl_is_loaded = 1;
	log_error(proc.N, "core", __FILE__, __LINE__, 2, "ssl_init() completed");
	return 0;
}

int ssl_accept(TCP_SOCKET *sock)
{
	if (sock->using_tls == 1) return -1;
	if ((sock->ssl = libssl.SSL_new(proc.ssl_ctx)) == NULL) {
		return -1;
	}
	/* SSL_clear(sock->ssl); */
	libssl.SSL_set_fd(sock->ssl, sock->socket);
	if (libssl.SSL_accept(sock->ssl) == -1) {
		return -1;
	}
	sock->using_tls = 1;
	return 0;
}

int ssl_connect(TCP_SOCKET *sock)
{
	/*
	 * i really need to fix this, but it's hot, and i'm chronically lazy today.
	 */
	static SSL_METHOD *meth = NULL;
	static SSL_CTX *ctx = NULL;
	int rc;

	if (sock->using_tls == 1) return -1;
	if (ctx == NULL) {
		meth = libssl.SSLv23_client_method();
		ctx = libssl.SSL_CTX_new(meth);
		long options = SSL_OP_NO_TLSv1_1 | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
		libssl.SSL_CTX_ctrl(ctx, SSL_CTRL_OPTIONS, options, NULL);
	}

	/*	sock->ssl=libssl.SSL_new(proc.ssl_ctx); */
	sock->ssl = libssl.SSL_new(ctx);
	libssl.SSL_set_fd(sock->ssl, sock->socket);
	rc = libssl.SSL_connect(sock->ssl);
	if (rc == -1) {
		log_error(proc.N, "tcp", __FILE__, __LINE__, 1, "SSL_connect error %d", rc);
		return -1;
	}
	/*
		conn->ssl=SSL_new(conn->ctx);
		SSL_set_fd(conn->ssl, conn->socket);
		if (SSL_connect(conn->ssl)==-1) { perror("socket: "); exit -1; }
	/	* the rest is optional *
	/	printf("SSL connection using %s\r\n", SSL_get_cipher(conn->ssl));
		if ((server_cert=SSL_get_peer_certificate(conn->ssl))==NULL) exit-1;
	/	printf("Server certificate:\r\n");
		X509_free(server_cert);
	*/
	sock->using_tls = 1;
	return 0;
}

int ssl_read(SSL *ssl, void *buf, int len)
{
	return libssl.SSL_read(ssl, buf, len);
}

int ssl_write(SSL *ssl, const void *buf, int len)
{
	return libssl.SSL_write(ssl, buf, len);
}

int ssl_close(TCP_SOCKET *sock)
{
	if (sock->ssl != NULL) {
		if (libssl.SSL_get_shutdown(sock->ssl)&SSL_RECEIVED_SHUTDOWN) {
			libssl.SSL_shutdown(sock->ssl);
		}
		else {
			libssl.SSL_clear(sock->ssl);
		}
	}
	if (sock->socket > -1) {
		shutdown(sock->socket, 2);
		closesocket(sock->socket);
		sock->socket = -1;
	}
	if (sock->ssl != NULL) {
		libssl.SSL_free(sock->ssl);
		sock->ssl = NULL;
	}
	return 0;
}

int ssl_shutdown()
{
	libssl.SSL_CTX_free(proc.ssl_ctx);
	proc.ssl_is_loaded = 0;
	return 0;
}

#endif /* HAVE_SSL */
