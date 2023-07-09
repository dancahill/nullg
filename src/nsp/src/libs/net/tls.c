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

#ifdef HAVE_TLS

#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/socket.h>
#define closesocket close
#endif

#if defined HAVE_OPENSSL
#elif defined HAVE_MBEDTLS


static void print_mbedtls_error(nsp_state *N, const char *name, int err)
{
	char buf[128];
	char buf2[128];

	mbedtls_strerror(err, buf, sizeof(buf));
	snprintf(buf2, sizeof(buf2), "%s() failed: -0x%04x (%d): %s\r\n", name, -err, err, buf);
	n_warn(N, name, "%s", buf2);
}

//static void my_debug(void *ctx, int level, const char *file, int line, const char *str)
//{
//	((void)level);
//	fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
//	fflush((FILE *)ctx);
//}

#endif

int _tls_init(nsp_state *N, TCP_SOCKET *sock, short srvmode, char *certfile, char *keyfile, char *chainfile)
{
#define __FN__ __FILE__ ":_tls_init()"
#if defined HAVE_OPENSSL
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	sock->ssl_ctx = SSL_CTX_new(TLS_client_method());
	if (!sock->ssl_ctx) {
		n_warn(N, __FN__, "SSL Error");
		return -1;
	}
	if ((certfile == NULL) || (keyfile == NULL)) return 0;
	if (SSL_CTX_use_certificate_file(sock->ssl_ctx, certfile, SSL_FILETYPE_PEM) <= 0) {
		n_warn(N, __FN__, "SSL Error loading certificate '%s'", certfile);
		return -1;
	}
	if (SSL_CTX_use_PrivateKey_file(sock->ssl_ctx, keyfile, SSL_FILETYPE_PEM) <= 0) {
		n_warn(N, __FN__, "SSL Error loading private key '%s'", keyfile);
		return -1;
	}
	if (chainfile != NULL) {
		if (SSL_CTX_use_certificate_chain_file(sock->ssl_ctx, chainfile) <= 0) {
			n_warn(N, __FN__, "SSL Error loading certificate chain '%s'", chainfile);
			return -1;
		}
	}
	if (!SSL_CTX_check_private_key(sock->ssl_ctx)) {
		n_warn(N, __FN__, "Private key does not match the public certificate");
		return -1;
	}
	return 0;
#elif defined HAVE_MBEDTLS
	const char *pers = "ssl_server";
	int rc;

	mbedtls_ssl_config_init(&sock->conf);
	if ((rc = mbedtls_ssl_config_defaults(&sock->conf, srvmode ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		n_warn(N, __FN__, "mbedtls_ssl_config_defaults failed.  error %d", rc);
		return -1;
	}
	mbedtls_ssl_conf_authmode(&sock->conf, MBEDTLS_SSL_VERIFY_NONE);
	mbedtls_entropy_init(&sock->entropy);
	mbedtls_ctr_drbg_init(&sock->ctr_drbg);
	if ((rc = mbedtls_ctr_drbg_seed(&sock->ctr_drbg, mbedtls_entropy_func, &sock->entropy, (const unsigned char *)pers, nc_strlen(pers))) != 0) {
		print_mbedtls_error(N, __FN__, rc);
		return -1;
	}
	mbedtls_ssl_conf_rng(&sock->conf, mbedtls_ctr_drbg_random, &sock->ctr_drbg);
	if ((certfile == NULL) || (keyfile == NULL)) return 0;
	if ((rc = mbedtls_x509_crt_parse_file(&sock->srvcert, certfile)) != 0) {
		print_mbedtls_error(N, __FN__, rc);
		return -1;
	}
	if ((rc = mbedtls_pk_parse_keyfile(&sock->pubkey, keyfile, NULL)) != 0) {
		print_mbedtls_error(N, __FN__, rc);
		return -1;
	}
	if (chainfile != NULL && chainfile[0] != '\0') {
		if ((rc = mbedtls_x509_crt_parse_file(&sock->chainfile, chainfile)) != 0) {
			print_mbedtls_error(N, __FN__, rc);
			return -1;
		}
		mbedtls_ssl_conf_ca_chain(&sock->conf, &sock->chainfile, NULL);
	}
	//mbedtls_ssl_conf_ca_chain(&sock->conf, sock->srvcert.next, NULL);
	if ((rc = mbedtls_ssl_conf_own_cert(&sock->conf, &sock->srvcert, &sock->pubkey)) != 0) {
		n_warn(N, __FN__, "mbedtls_ssl_conf_own_cert returned %08x", rc);
		return -1;
	}
	return 0;
#endif
#undef __FN__
}

int _tls_accept(nsp_state *N, TCP_SOCKET *bsock, TCP_SOCKET *asock)
{
#define __FN__ __FILE__ ":_tls_accept()"
#if defined HAVE_OPENSSL
	if ((asock->ssl = SSL_new(bsock->ssl_ctx)) == NULL) {
		return -1;
	}
	/*	SSL_clear(asock->ssl); */
	SSL_set_fd(asock->ssl, asock->socket);
	if (SSL_accept(asock->ssl) == -1) {
		return -1;
	}
	return 0;
#elif defined HAVE_MBEDTLS
	int rc;

	asock->net_context.fd = asock->socket;
	mbedtls_ssl_init(&asock->ssl);
	if ((rc = mbedtls_ssl_setup(&asock->ssl, &bsock->conf)) != 0) {
		print_mbedtls_error(N, __FN__, rc);
		return -1;
	}
	//mbedtls_ssl_session_reset(&asock->ssl);
	mbedtls_ssl_set_bio(&asock->ssl, &asock->net_context, mbedtls_net_send, mbedtls_net_recv, NULL);
	if ((rc = mbedtls_ssl_handshake(&asock->ssl)) < 0) {
		print_mbedtls_error(N, __FN__, rc);
	}
	return rc;
#endif
#undef __FN__
}

int _tls_connect(nsp_state *N, TCP_SOCKET *sock)
{
#define __FN__ __FILE__ ":_tls_connect()"
#if defined HAVE_OPENSSL
	/* X509 *server_cert; */
	int rc;

	_tls_init(N, sock, 0, NULL, NULL, NULL);
	sock->ssl = SSL_new(sock->ssl_ctx);
	SSL_set_fd(sock->ssl, sock->socket);
	if ((rc = SSL_connect(sock->ssl)) == -1) {
		//int err = SSL_get_error(sock->ssl, rc);
		//if (err == SSL_ERROR_SSL) {
		//	char msg[1024];
		//	ERR_error_string_n(ERR_get_error(), msg, sizeof(msg));
		//	printf("%s %s %s %s\n", msg, ERR_lib_error_string(0), ERR_func_error_string(0), ERR_reason_error_string(0));
		//}
		n_warn(N, __FN__, "SSL_connect error %d", rc);
		return -1;
	}
	/* the rest is optional */
/*
	printf("SSL connection using %s\r\n", SSL_get_cipher(sock->ssl));
	if ((server_cert=SSL_get_peer_certificate(sock->ssl))!=NULL) {
		X509_free(server_cert);
	}
*/
	return 0;
#elif defined HAVE_MBEDTLS
	int rc;

	_tls_init(N, sock, 0, NULL, NULL, NULL);
	sock->net_context.fd = sock->socket;
	mbedtls_ssl_init(&sock->ssl);
	if ((rc = mbedtls_ssl_setup(&sock->ssl, &sock->conf)) != 0) {
		print_mbedtls_error(N, __FN__, rc);
		return -1;
	}
	mbedtls_ssl_set_bio(&sock->ssl, &sock->net_context, mbedtls_net_send, mbedtls_net_recv, NULL);
	nc_memset((void *)&sock->ssn, 0, sizeof(mbedtls_ssl_session));
	mbedtls_ssl_set_session(&sock->ssl, &sock->ssn);
	return 0;
#endif
#undef __FN__
}

int _tls_read(nsp_state *N, TCP_SOCKET *sock, void *buf, int max)
{
#define __FN__ __FILE__ ":_tls_read()"
#if defined HAVE_OPENSSL
	return SSL_read(sock->ssl, buf, max);
#elif defined HAVE_MBEDTLS
	int rc;

	do {
		rc = mbedtls_ssl_read(&sock->ssl, (void *)buf, max);
	} while (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE);
	if (rc <= 0)
	{
		switch (rc) {
		case MBEDTLS_ERR_SSL_TIMEOUT:
			return rc;
		case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
			return rc;
		default:
			//printf(" mbedtls_ssl_read returned -0x%x\n\n", -ret);
			//print_mbedtls_error(N, __FN__, rc);
			return rc;
		}
	}
	return rc;
#endif
#undef __FN__
}

int _tls_write(nsp_state *N, TCP_SOCKET *sock, const void *buf, int max)
{
#define __FN__ __FILE__ ":_tls_write()"
#if defined HAVE_OPENSSL
	return SSL_write(sock->ssl, buf, max);
#elif defined HAVE_MBEDTLS
	int rc;

	do {
		rc = mbedtls_ssl_write(&sock->ssl, (void *)buf, max);
	} while (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE);
	return rc;
#endif
#undef __FN__
}

int _tls_close(nsp_state *N, TCP_SOCKET *sock)
{
#define __FN__ __FILE__ ":_tls_close()"
#if defined HAVE_OPENSSL
	if (sock->ssl != NULL) {
		if (SSL_get_shutdown(sock->ssl) & SSL_RECEIVED_SHUTDOWN) {
			SSL_shutdown(sock->ssl);
		} else {
			SSL_clear(sock->ssl);
		}
	}
	if (sock->socket > -1) {
		shutdown(sock->socket, 2);
		closesocket(sock->socket);
		sock->socket = -1;
	}
	if (sock->ssl != NULL) {
		SSL_free(sock->ssl);
		sock->ssl = NULL;
	}
	return 0;
#elif defined HAVE_MBEDTLS
	mbedtls_ssl_close_notify(&sock->ssl);
	if (sock->socket > -1) {
		shutdown(sock->socket, 2);
		closesocket(sock->socket);
		sock->socket = -1;
	}
	return 0;
#endif
#undef __FN__
}

int _tls_shutdown(nsp_state *N, TCP_SOCKET *sock)
{
#define __FN__ __FILE__ ":_tls_shutdown()"
#if defined HAVE_OPENSSL
	if (sock->ssl_ctx) {
		SSL_CTX_free(sock->ssl_ctx);
		sock->ssl_ctx = NULL;
	}
	return 0;
#elif defined HAVE_MBEDTLS
	if (sock->use_tls) {
		/* x509 and rsa for server sockets */
		mbedtls_x509_crt_free(&sock->srvcert);
		mbedtls_x509_crt_free(&sock->chainfile);
		mbedtls_pk_free(&sock->pubkey);
		//rsa_free(&sock->pubkey);
		mbedtls_ssl_free(&sock->ssl);
		sock->use_tls = 0;
	}
	return 0;
#endif
#undef __FN__
}

#endif /* HAVE_TLS */
