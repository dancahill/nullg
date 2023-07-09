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

#ifdef _WIN32
#define sleep(x) Sleep(x*1000)
#define msleep(x) Sleep(x)
#define strcasecmp stricmp
typedef int socklen_t;
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#define msleep(x) usleep(x*1000)
#ifdef MISSING_SOCKLEN
typedef int socklen_t;
#endif
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXWAIT 10

/*
 * this is the function that terminates orphans
 */
void tcp_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":tcp_murder()"
	TCP_SOCKET *sock;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || strcmp(cobj->val->d.str, "sock4") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a socket");
	sock = (TCP_SOCKET *)cobj->val->d.str;
	tcp_close(N, sock, 1);
	n_free(N, (void *)&cobj->val->d.str, sizeof(TCP_SOCKET) + 1);
	cobj->val->size = 0;
	return;
#undef __FN__
}

int tcp_bind(nsp_state *N, TCP_SOCKET *sock, char *ifname, unsigned short port)
{
#define __FN__ __FILE__ ":tcp_bind()"
	struct hostent *hp;
	struct sockaddr_in sin;
	int option;
#ifdef _WIN64
	uint64 bindsock;
#else
	int bindsock;
#endif

	nc_memset((char *)&sin, 0, sizeof(sin));
	bindsock = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	if (strcasecmp("INADDR_ANY", ifname) == 0) {
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		if ((hp = gethostbyname(ifname)) == NULL) {
			snprintf(sock->errormsg, sizeof(sock->errormsg) - 1, "Host lookup error for %s", ifname);
			return -1;
		}
		nc_memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	sin.sin_port = htons(port);
	option = 1;
	setsockopt(bindsock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));
	if (bind(bindsock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		snprintf(sock->errormsg, sizeof(sock->errormsg) - 1, "bind() error [%s:%d]", ifname, port);
		return -1;
	}
	if (listen(bindsock, 50) < 0) {
		snprintf(sock->errormsg, sizeof(sock->errormsg) - 1, "listen() error");
		closesocket(bindsock);
		return -1;
	}
	return bindsock;
#undef __FN__
}

int tcp_accept(nsp_state *N, TCP_SOCKET *bsock, TCP_SOCKET *asock)
{
#define __FN__ __FILE__ ":tcp_accept()"
	struct sockaddr addr;
	struct sockaddr_in host;
	struct sockaddr_in peer;
#ifdef _WIN64
	uint64 clientsock;
#else
	int clientsock;
#endif
	socklen_t fromlen;

/*
	int lowat=1;
	struct timeval timeout;
*/
	fromlen = sizeof(addr);
	clientsock = accept(bsock->socket, &addr, &fromlen);
	if (clientsock < 0) {
		asock->LocalPort = 0;
		asock->RemotePort = 0;
		snprintf(bsock->errormsg, sizeof(bsock->errormsg) - 1, "failed tcp_accept");
		return -1;
	}
	asock->socket = clientsock;
#ifdef HAVE_TLS
	if (bsock->use_tls) {
		_tls_accept(N, bsock, asock);
		asock->use_tls = 1;
	}
#endif
	fromlen = sizeof(host);
	getsockname(asock->socket, (struct sockaddr *)&host, &fromlen);
	nc_strncpy(asock->LocalAddr, inet_ntoa(host.sin_addr), sizeof(asock->LocalAddr) - 1);
	asock->LocalPort = ntohs(host.sin_port);
	fromlen = sizeof(peer);
	getpeername(asock->socket, (struct sockaddr *)&peer, &fromlen);
	nc_strncpy(asock->RemoteAddr, inet_ntoa(peer.sin_addr), sizeof(asock->RemoteAddr) - 1);
	asock->RemotePort = ntohs(peer.sin_port);
/*
	n_warn(N, __FN__, "[%s:%d] new connection", asock->RemoteAddr, asock->RemotePort);
	timeout.tv_sec=1;
	timeout.tv_usec=0;
	setsockopt(clientsock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout));
	setsockopt(clientsock, SOL_SOCKET, SO_RCVLOWAT, (void *)&lowat, sizeof(lowat));
*/
	return 0;
#undef __FN__
}

static int tcp_conn(nsp_state *N, TCP_SOCKET *sock, const struct sockaddr_in *serv_addr, socklen_t addrlen, short int use_tls)
{
	struct sockaddr_in host;
	struct sockaddr_in peer;
	socklen_t fromlen;
	int rc;

	rc = connect(sock->socket, (struct sockaddr *)serv_addr, addrlen);
#ifdef HAVE_TLS
	if ((rc == 0) && (use_tls)) {
		rc = _tls_connect(N, sock);
		sock->use_tls = 1;
	}
#endif
	if (rc < 0) {
		sock->LocalPort = 0;
		sock->RemotePort = 0;
		return -1;
	}
	fromlen = sizeof(host);
	getsockname(sock->socket, (struct sockaddr *)&host, &fromlen);
	nc_strncpy(sock->LocalAddr, inet_ntoa(host.sin_addr), sizeof(sock->LocalAddr) - 1);
	sock->LocalPort = ntohs(host.sin_port);
	fromlen = sizeof(peer);
	getpeername(sock->socket, (struct sockaddr *)&peer, &fromlen);
	nc_strncpy(sock->RemoteAddr, inet_ntoa(peer.sin_addr), sizeof(sock->RemoteAddr) - 1);
	sock->RemotePort = ntohs(peer.sin_port);
	return rc;
}

int tcp_connect(nsp_state *N, TCP_SOCKET *sock, char *host, unsigned short port, short int use_tls)
{
#define __FN__ __FILE__ ":tcp_connect()"
	struct hostent *hp;
	struct sockaddr_in serv;

	if ((hp = gethostbyname(host)) == NULL) {
		snprintf(sock->errormsg, sizeof(sock->errormsg) - 1, "Host lookup error for %s", host);
		return -1;
	}
	nc_memset((char *)&serv, 0, sizeof(serv));
	nc_memcpy((char *)&serv.sin_addr, hp->h_addr, hp->h_length);
	serv.sin_family = hp->h_addrtype;
	serv.sin_port = htons(port);
	if ((sock->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2;
	if (tcp_conn(N, sock, &serv, sizeof(serv), use_tls) < 0) {
		return -2;
	}
	return 0;
#undef __FN__
}

int tcp_recv(nsp_state *N, TCP_SOCKET *socket, char *buffer, int max, int flags)
{
#define __FN__ __FILE__ ":tcp_recv()"
	int rc;

	if (socket->socket == -1) return -1;
	if (socket->want_close) {
		tcp_close(N, socket, 1);
		return -1;
	}
	if (max > MAX_TCP_READ_SIZE) max = MAX_TCP_READ_SIZE;
	if (socket->use_tls) {
#ifdef HAVE_TLS
		rc = _tls_read(N, socket, buffer, max);
		if (rc == 0) rc = -1;
#else
		rc = -1;
#endif
	} else {
		rc = recv(socket->socket, buffer, max, flags);
	}
	if (rc < 0) {
#ifdef _WIN32
		int ec = WSAGetLastError();
		switch (ec) {
		case 0:
			return 0;
		case WSAECONNABORTED:
		case WSAETIMEDOUT:
			return 0;
		default:
			n_warn(N, __FN__, "[%s:%d] WSAGetLastError()=%d", socket->RemoteAddr, socket->RemotePort, ec);
			break;
		}
		return -1;
#else
		switch (errno) {
		case EWOULDBLOCK:
			return 0;
		case ECONNRESET:
			n_warn(N, __FN__, "ECONNRESET[%s:%d] %.100s", socket->RemoteAddr, socket->RemotePort, strerror(errno));
			tcp_close(N, socket, 1); errno = 0; break;
		default:
			if (N->debug) n_warn(N, __FN__, "[%s:%d] %.100s", socket->RemoteAddr, socket->RemotePort, strerror(errno));
			errno = 0;
		}
		return -1;
#endif
	} else {
		socket->mtime = time(NULL);
		socket->bytes_in += rc;
	}
	//if (N->debug) n_warn(N, __FN__, "[%s:%d] %d bytes of data", socket->RemoteAddr, socket->RemotePort, rc);
	return rc;
#undef __FN__
}

int tcp_send(nsp_state *N, TCP_SOCKET *socket, const char *buffer, int len, int flags)
{
#define __FN__ __FILE__ ":tcp_send()"
	int rc;

	if (socket->socket == -1) return -1;
	if (socket->want_close) {
		tcp_close(N, socket, 1);
		return -1;
	}
	if (socket->use_tls) {
#ifdef HAVE_TLS
		rc = _tls_write(N, socket, buffer, len);
#else
		rc = -1;
#endif
	} else {
		rc = send(socket->socket, buffer, len, flags);
	}
	if (rc < 0) {
#ifdef _WIN32
		return rc;
#else
		if (errno == EWOULDBLOCK) {
			errno = 0;
			msleep(MAXWAIT);
		} else if (errno) {
			if (N->debug) n_warn(N, __FN__, "[%s:%d] %d%.100s", socket->RemoteAddr, socket->RemotePort, errno, strerror(errno));
			errno = 0;
		}
#endif
	} else if (rc == 0) {
		msleep(MAXWAIT);
	} else {
		socket->mtime = time(NULL);
		socket->bytes_out += rc;
	}
	return rc;
#undef __FN__
}

int nc_vsnprintf(nsp_state *N, char *dest, int max, const char *format, va_list ap);

int tcp_fprintf(nsp_state *N, TCP_SOCKET *socket, const char *format, ...)
{
#define __FN__ __FILE__ ":tcp_fprintf()"
	char *buffer;
	va_list ap;
	int rc;

	if ((buffer = calloc(2048, sizeof(char))) == NULL) {
		n_warn(N, __FN__, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	nc_vsnprintf(N, buffer, 2047, format, ap);
	va_end(ap);
	if (N->debug) n_warn(N, __FN__, "[%s:%d] %s", socket->RemoteAddr, socket->RemotePort, buffer);
	rc = tcp_send(N, socket, buffer, (int)strlen(buffer), 0);
	free(buffer);
	return rc;
#undef __FN__
}

int tcp_fgets(nsp_state *N, TCP_SOCKET *socket, char *buffer, int max)
{
#define __FN__ __FILE__ ":tcp_fgets()"
	char *pbuffer = buffer;
	char *obuffer;
	short int lf = 0;
	short int n = 0;
	int rc;
	int x;

retry:
	if (!socket->recvbufsize) {
		x = sizeof(socket->recvbuf) - socket->recvbufoffset - socket->recvbufsize - 2;
		if (x < 1) {
			nc_memset(socket->recvbuf, 0, sizeof(socket->recvbuf));
			socket->recvbufoffset = 0;
			socket->recvbufsize = 0;
			x = sizeof(socket->recvbuf) - socket->recvbufoffset - socket->recvbufsize - 2;
		}
		obuffer = socket->recvbuf + socket->recvbufoffset + socket->recvbufsize;
		if (x > max) x = max;
		if ((rc = tcp_recv(N, socket, obuffer, x, 0)) < 0) {
			return -1;
		} else if (rc < 1) {
			*pbuffer = '\0';
			if (N->debug) n_warn(N, __FN__, "[%s:%d] %s", socket->RemoteAddr, socket->RemotePort, buffer);
			return n;
		}
		socket->recvbufsize += rc;
	}
	obuffer = socket->recvbuf + socket->recvbufoffset;
	while ((n < max) && (socket->recvbufsize > 0)) {
		socket->recvbufoffset++;
		socket->recvbufsize--;
		n++;
		if (*obuffer == '\n') lf = 1;
		*pbuffer++ = *obuffer++;
		if ((lf) || (*obuffer == '\0')) break;
	}
	*pbuffer = '\0';
	if (n > max - 1) {
		if (N->debug) n_warn(N, __FN__, "[%s:%d] %s", socket->RemoteAddr, socket->RemotePort, buffer);
		return n;
	}
	if (!lf) {
		if (socket->recvbufsize > 0) {
			nc_memcpy(socket->recvbuf, socket->recvbuf + socket->recvbufoffset, socket->recvbufsize);
			nc_memset(socket->recvbuf + socket->recvbufsize, 0, sizeof(socket->recvbuf) - socket->recvbufsize);
			socket->recvbufoffset = 0;
		} else {
			nc_memset(socket->recvbuf, 0, sizeof(socket->recvbuf));
			socket->recvbufoffset = 0;
			socket->recvbufsize = 0;
		}
		goto retry;
	}
	if (N && N->debug) n_warn(N, __FN__, "[%s:%d] %s", socket->RemoteAddr, socket->RemotePort, buffer);
	return n;
#undef __FN__
}

int tcp_close(nsp_state *N, TCP_SOCKET *socket, short int owner_killed)
{
	if (!owner_killed) {
		socket->want_close = 1;
	} else {
		socket->want_close = 0;
#ifdef HAVE_TLS
		if (socket->use_tls) _tls_close(N, socket);
#endif
		if (socket->socket > -1) {
			shutdown(socket->socket, 2);
			closesocket(socket->socket);
			socket->socket = -1;
		}
	}
	return 0;
}
