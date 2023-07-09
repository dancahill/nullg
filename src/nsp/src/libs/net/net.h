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

#if defined(_WIN32)
#include "nsp/config-win.h"
#else
#include "nsp/config.h"
#endif

#ifdef _WIN32
#define msleep(x) Sleep(x)
#define strcasecmp stricmp
//#define snprintf _snprintf
//#define vsnprintf _vsnprintf
typedef int socklen_t;
#else
#include <unistd.h>
#define closesocket close
#define msleep(x) usleep(x*1000)
#ifdef MISSING_SOCKLEN
typedef int socklen_t;
#endif
#endif

#if defined(CONFIG_HAVE_DNS) && !defined(HAVE_DNS)
#define HAVE_DNS
#endif

#if defined(CONFIG_HAVE_MBEDTLS)
#undef CONFIG_HAVE_OPENSSL
#undef HAVE_OPENSSL
#endif

#if defined(CONFIG_HAVE_OPENSSL)
#if !defined(HAVE_OPENSSL)
#define HAVE_OPENSSL
#endif
#if defined(_WIN32)
#pragma comment(lib, "libcrypto64MD.lib")
#pragma comment(lib, "libssl64MD.lib")
#endif
#elif defined(CONFIG_HAVE_MBEDTLS)
#if !defined(HAVE_MBEDTLS)
#define HAVE_MBEDTLS
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#  pragma comment(lib, "ws2_32.lib")
#  ifdef __CYGWIN__
#    include <ws2tcpip.h>
#  endif
#endif

#ifdef _WIN32
#else
#include <sys/socket.h>
#endif

#ifdef HAVE_MBEDTLS
/* uncomment this to prioritize mbedtls over openssl */
/* #undef HAVE_OPENSSL */
#endif

#if defined(HAVE_OPENSSL)
#  include <openssl/ssl.h>
#  define HAVE_TLS
#elif defined(HAVE_MBEDTLS)
#  include "mbedtls/certs.h"
#  include "mbedtls/ctr_drbg.h"
#  include "mbedtls/entropy.h"
#  include "mbedtls/error.h"
#  include "mbedtls/ssl.h"
#  include "mbedtls/net.h"
//#  include "mbedtls/havege.h"
#  include "mbedtls/ctr_drbg.h"
#  define HAVE_TLS
#endif

void tcp_murder(nsp_state *N, obj_t *cobj);

#define MAX_TCP_READ_SIZE 65536

typedef struct {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	/* mbedtls dies a horrible and stupid death when using short ints.. */
//	short int socket;
#if defined _WIN32 && !defined _WIN64
	int socket;
#else
	uint64 socket;
#endif
	short use_tls;
#if defined(HAVE_OPENSSL)
	SSL *ssl;
	SSL_CTX *ssl_ctx;
#elif defined(HAVE_MBEDTLS)
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_ssl_session ssn;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_net_context net_context;//server_fd...
	/* server stuff */
	mbedtls_x509_crt srvcert;
	mbedtls_x509_crt chainfile;
	mbedtls_pk_context pubkey;
	unsigned char session_table[sizeof(mbedtls_ssl_session)];
#endif
	char LocalAddr[16];
	int  LocalPort;
	char RemoteAddr[16];
	int  RemotePort;
	time_t ctime; /* Creation time */
	time_t mtime; /* Last Modified time */
	unsigned int bytes_in;
	unsigned int bytes_out;
	short int want_close;
	char errormsg[256];
	/* TCP INPUT BUFFER */
	int recvbufsize;
	int recvbufoffset;
	char recvbuf[MAX_TCP_READ_SIZE];
} TCP_SOCKET;

/* tls.c functions */
#ifdef HAVE_TLS
int _tls_init    (nsp_state *N, TCP_SOCKET *sock, short srvmode, char *certfile, char *keyfile, char *chainfile);
int _tls_accept  (nsp_state *N, TCP_SOCKET *bsock, TCP_SOCKET *asock);
int _tls_connect (nsp_state *N, TCP_SOCKET *sock);
int _tls_read    (nsp_state *N, TCP_SOCKET *sock, void *buf, int max);
int _tls_write   (nsp_state *N, TCP_SOCKET *sock, const void *buf, int len);
int _tls_close   (nsp_state *N, TCP_SOCKET *sock);
int _tls_shutdown(nsp_state *N, TCP_SOCKET *sock);
#endif

/* tcp.c functions */
int tcp_bind   (nsp_state *N, TCP_SOCKET *sock, char *ifname, unsigned short port);
int tcp_accept (nsp_state *N, TCP_SOCKET *bsock, TCP_SOCKET *asock);
int tcp_connect(nsp_state *N, TCP_SOCKET *socket, char *host, unsigned short port, short int use_tls);
int tcp_fgets  (nsp_state *N, TCP_SOCKET *socket, char *buffer, int max);
int tcp_fprintf(nsp_state *N, TCP_SOCKET *socket, const char *format, ...);
int tcp_recv   (nsp_state *N, TCP_SOCKET *socket, char *buffer, int max, int flags);
int tcp_send   (nsp_state *N, TCP_SOCKET *socket, const char *buffer, int len, int flags);
int tcp_close  (nsp_state *N, TCP_SOCKET *socket, short int owner_killed);

#ifdef HAVE_DNS
NSP_FUNCTION(libnsp_net_dns_addr2name);
NSP_FUNCTION(libnsp_net_dns_name2addr);
#endif
/* ftp.c */
NSP_CLASSMETHOD(libnsp_net_ftp_client_client);
NSP_CLASSMETHOD(libnsp_net_ftp_client_open);
NSP_CLASSMETHOD(libnsp_net_ftp_client_close);
NSP_CLASSMETHOD(libnsp_net_ftp_client_cwd);
NSP_CLASSMETHOD(libnsp_net_ftp_client_pwd);
NSP_CLASSMETHOD(libnsp_net_ftp_client_ls);
NSP_CLASSMETHOD(libnsp_net_ftp_client_retr);
NSP_CLASSMETHOD(libnsp_net_ftp_client_stor);
/* http.c */
NSP_CLASSMETHOD(libnsp_net_http_client_client);
NSP_CLASSMETHOD(libnsp_net_http_client_send);
/* httpd.c */
NSP_CLASSMETHOD(libnsp_net_http_server_constructor);
NSP_CLASSMETHOD(libnsp_net_http_server_start);
NSP_CLASSMETHOD(libnsp_net_http_server_stop);
/* mime.c */
NSP_FUNCTION(libnsp_net_mime_read);
NSP_FUNCTION(libnsp_net_mime_write);
NSP_FUNCTION(libnsp_net_mime_base64_decode);
NSP_FUNCTION(libnsp_net_mime_base64_encode);
NSP_FUNCTION(libnsp_net_mime_qp_decode);
NSP_FUNCTION(libnsp_net_mime_qp_encode);
NSP_FUNCTION(libnsp_net_mime_rfc2047_decode);
NSP_FUNCTION(libnsp_net_mime_rfc2047_encode);
/* pop3.c */
NSP_CLASSMETHOD(libnsp_net_pop3_client_client);
NSP_CLASSMETHOD(libnsp_net_pop3_client_open);
NSP_CLASSMETHOD(libnsp_net_pop3_client_close);
NSP_CLASSMETHOD(libnsp_net_pop3_client_stat);
NSP_CLASSMETHOD(libnsp_net_pop3_client_uidl);
NSP_CLASSMETHOD(libnsp_net_pop3_client_top);
NSP_CLASSMETHOD(libnsp_net_pop3_client_retr);
NSP_CLASSMETHOD(libnsp_net_pop3_client_dele);
NSP_CLASSMETHOD(libnsp_net_pop3_client_list);
/* smtp.c */
NSP_CLASSMETHOD(libnsp_net_smtp_client_client);
NSP_CLASSMETHOD(libnsp_net_smtp_client_attach);
NSP_CLASSMETHOD(libnsp_net_smtp_client_send);

/* socket.c */
NSP_FUNCTION(libnsp_net_socket_accept);
NSP_FUNCTION(libnsp_net_socket_bind);
NSP_FUNCTION(libnsp_net_socket_close);
NSP_FUNCTION(libnsp_net_socket_connect);
NSP_FUNCTION(libnsp_net_socket_gets);
NSP_FUNCTION(libnsp_net_socket_gettype);
NSP_FUNCTION(libnsp_net_socket_info);
NSP_FUNCTION(libnsp_net_socket_read);
NSP_FUNCTION(libnsp_net_socket_setsockopt);
NSP_FUNCTION(libnsp_net_socket_socket);
NSP_FUNCTION(libnsp_net_socket_tlsaccept);
NSP_FUNCTION(libnsp_net_socket_tlsconnect);
NSP_FUNCTION(libnsp_net_socket_write);
/* tnef.c */
NSP_FUNCTION(libnsp_net_tnef_debug);

int nspnet_register_all(nsp_state *N);

#ifdef __cplusplus
}
#endif
