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

#ifdef HAVE_SSL

typedef void        (*LIBSSL_LOAD_ERROR_STRINGS)(void);
typedef int         (*LIBSSL_LIBRARY_INIT)(void);
typedef SSL_METHOD *(*LIBSSL_SERVER_METHOD)(void);
typedef SSL_CTX    *(*LIBSSL_CTX_NEW)(SSL_METHOD *);
typedef int         (*LIBSSL_CTX_USE_CERT_FILE)(SSL_CTX *, const char *, int);
typedef int         (*LIBSSL_CTX_USE_KEY_FILE)(SSL_CTX *, const char *, int);
typedef int         (*LIBSSL_CTX_CHECK_KEY)(SSL_CTX *);
typedef SSL        *(*LIBSSL_NEW)(SSL_CTX *);
typedef int         (*LIBSSL_SET_FD)(SSL *, int);
typedef int         (*LIBSSL_ACCEPT)(SSL *);
typedef void        (*LIBSSL_FREE)(SSL *);
typedef int         (*LIBSSL_GET_SHUTDOWN)(SSL *);
typedef int         (*LIBSSL_SHUTDOWN)(SSL *);
typedef int         (*LIBSSL_CLEAR)(SSL *);
typedef void        (*LIBSSL_CTX_FREE)(SSL_CTX *);
typedef int         (*LIBSSL_READ)(SSL *, void *, int);
typedef int         (*LIBSSL_WRITE)(SSL *, const void *, int);

static struct {
	LIBSSL_LOAD_ERROR_STRINGS	SSL_load_error_strings;
	LIBSSL_LIBRARY_INIT		SSL_library_init;
	LIBSSL_SERVER_METHOD		SSLv23_server_method;
	LIBSSL_CTX_NEW			SSL_CTX_new;
	LIBSSL_CTX_USE_CERT_FILE	SSL_CTX_use_certificate_file;
	LIBSSL_CTX_USE_KEY_FILE		SSL_CTX_use_PrivateKey_file;
	LIBSSL_CTX_CHECK_KEY		SSL_CTX_check_private_key;
	LIBSSL_NEW			SSL_new;
	LIBSSL_SET_FD			SSL_set_fd;
	LIBSSL_ACCEPT			SSL_accept;
	LIBSSL_FREE			SSL_free;
	LIBSSL_GET_SHUTDOWN		SSL_get_shutdown;
	LIBSSL_SHUTDOWN			SSL_shutdown;
	LIBSSL_CLEAR			SSL_clear;
	LIBSSL_CTX_FREE			SSL_CTX_free;
	LIBSSL_READ			SSL_read;
	LIBSSL_WRITE			SSL_write;
} libssl;

int ssl_dl_init()
{
#ifdef HAVE_MYSQL
	static int isloaded=0;
#ifdef WIN32
	HINSTANCE hinstLib;
	char *libext="dll";
#else
	void *hinstLib;
	char *libext="so";
#endif
	char libname[255];

	if (isloaded) return 0;
	memset(libname, 0, sizeof(libname));
#ifdef WIN32
	snprintf(libname, sizeof(libname)-1, "%s/libeay32.dll", proc.config.dir_lib);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) {
		snprintf(libname, sizeof(libname)-1, "%s/ssleay32.dll", proc.config.dir_lib);
		fixslashes(libname);
		if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	}
/*
	snprintf(libname, sizeof(libname)-1, "libssl.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
*/
#else
	snprintf(libname, sizeof(libname)-1, "libssl.%s.0.9.7", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
#endif
	goto fail;
found:
	if ((libssl.SSL_load_error_strings       = (void *)dlsym(hinstLib, "SSL_load_error_strings"))==NULL) goto fail;
	if ((libssl.SSL_library_init             = (void *)dlsym(hinstLib, "SSL_library_init"))==NULL) goto fail;
	if ((libssl.SSLv23_server_method         = (void *)dlsym(hinstLib, "SSLv23_server_method"))==NULL) goto fail;
	if ((libssl.SSL_CTX_new                  = (void *)dlsym(hinstLib, "SSL_CTX_new"))==NULL) goto fail;
	if ((libssl.SSL_CTX_use_certificate_file = (void *)dlsym(hinstLib, "SSL_CTX_use_certificate_file"))==NULL) goto fail;
	if ((libssl.SSL_CTX_use_PrivateKey_file  = (void *)dlsym(hinstLib, "SSL_CTX_use_PrivateKey_file"))==NULL) goto fail;
	if ((libssl.SSL_CTX_check_private_key    = (void *)dlsym(hinstLib, "SSL_CTX_check_private_key"))==NULL) goto fail;
	if ((libssl.SSL_new                      = (void *)dlsym(hinstLib, "SSL_new"))==NULL) goto fail;
	if ((libssl.SSL_set_fd                   = (void *)dlsym(hinstLib, "SSL_set_fd"))==NULL) goto fail;
	if ((libssl.SSL_accept                   = (void *)dlsym(hinstLib, "SSL_accept"))==NULL) goto fail;
	if ((libssl.SSL_free                     = (void *)dlsym(hinstLib, "SSL_free"))==NULL) goto fail;
	if ((libssl.SSL_get_shutdown             = (void *)dlsym(hinstLib, "SSL_get_shutdown"))==NULL) goto fail;
	if ((libssl.SSL_shutdown                 = (void *)dlsym(hinstLib, "SSL_shutdown"))==NULL) goto fail;
	if ((libssl.SSL_clear                    = (void *)dlsym(hinstLib, "SSL_clear"))==NULL) goto fail;
	if ((libssl.SSL_CTX_free                 = (void *)dlsym(hinstLib, "SSL_CTX_free"))==NULL) goto fail;
	if ((libssl.SSL_read                     = (void *)dlsym(hinstLib, "SSL_read"))==NULL) goto fail;
	if ((libssl.SSL_write                    = (void *)dlsym(hinstLib, "SSL_write"))==NULL) goto fail;
	isloaded=1;
	return 0;
fail:
	printf("\r\nERROR: Failed to load %s\r\n", libname);
	memset((char *)&libssl, 0, sizeof(libssl));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
#else
	return -1;
#endif
}

int ssl_init()
{
	if (proc.ssl_is_loaded) return 0;
	if (ssl_dl_init()<0) return -1;
	libssl.SSL_load_error_strings();
	libssl.SSLeay_add_ssl_algorithms();
	proc.ssl_meth=libssl.SSLv23_server_method();
	proc.ssl_ctx=libssl.SSL_CTX_new(proc.ssl_meth);
	if (!proc.ssl_ctx) {
		log_error("core", __FILE__, __LINE__, 0, "SSL Error");
		return -1;
	}
	if (libssl.SSL_CTX_use_certificate_file(proc.ssl_ctx, proc.config.ssl_cert, SSL_FILETYPE_PEM)<=0) {
		log_error("core",  __FILE__, __LINE__, 0, "SSL Error loading certificate '%s'", proc.config.ssl_cert);
		return -1;
	}
	if (libssl.SSL_CTX_use_PrivateKey_file(proc.ssl_ctx, proc.config.ssl_key, SSL_FILETYPE_PEM)<=0) {
		log_error("core", __FILE__, __LINE__, 0, "SSL Error loading private key '%s'", proc.config.ssl_key);
		return -1;
	}
	if (!libssl.SSL_CTX_check_private_key(proc.ssl_ctx)) {
		log_error("core", __FILE__, __LINE__, 0, "Private key does not match the public certificate");
		return -1;
	}
	proc.ssl_is_loaded=1;
	log_error("core", __FILE__, __LINE__, 2, "ssl_init() completed");
	return 0;
}

int ssl_accept(TCP_SOCKET *sock)
{
	if ((sock->ssl=libssl.SSL_new(proc.ssl_ctx))==NULL) {
		return -1;
	}
//	SSL_clear(sock->ssl);
	libssl.SSL_set_fd(sock->ssl, sock->socket);
	if (libssl.SSL_accept(sock->ssl)==-1) {
		return -1;
	}
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
	if (sock->ssl!=NULL) {
		if (libssl.SSL_get_shutdown(sock->ssl)&SSL_RECEIVED_SHUTDOWN) {
			libssl.SSL_shutdown(sock->ssl);
		} else {
			libssl.SSL_clear(sock->ssl);
		}
	}
	if (sock->socket>-1) {
		shutdown(sock->socket, 2);
		closesocket(sock->socket);
		sock->socket=-1;
	}
	if (sock->ssl!=NULL) {
		libssl.SSL_free(sock->ssl);
		sock->ssl=NULL;
	}
	return 0;
}

int ssl_shutdown()
{
	libssl.SSL_CTX_free(proc.ssl_ctx);
	proc.ssl_is_loaded=0;
	return 0;
}

#endif /* HAVE_SSL */
