/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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

int tcp_bind(char *ifname, unsigned short port)
{
	struct hostent *hp;
	struct sockaddr_in sin;
	int i;
	int option;
	int sock;

	sock=socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family=AF_INET;
	if ((strcasecmp("ANY", ifname)==0)||(strcasecmp("INADDR_ANY", ifname)==0)) {
		log_error("core", __FILE__, __LINE__, 2, "Binding to 'INADDR_ANY:%d'", port);
		sin.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
		log_error("core", __FILE__, __LINE__, 2, "Binding to '%s:%d'", ifname, port);
		hp=gethostbyname(ifname);
		if (hp==NULL) {
			log_error("core", __FILE__, __LINE__, 0, "Error binding to '%s:%d'", ifname, port);
			return -1;
		}
		memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	sin.sin_port=htons(port);
	option=1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));
	i=0;
	while (bind(sock, (struct sockaddr *)&sin, sizeof(sin))<0) {
		sleep(1);
		i++;
		if (i>5) {
			log_error("core", __FILE__, __LINE__, 0, "bind() error [%s:%d]", ifname, port);
			perror("\r\nBind error");
			return -1;
		}
	}
	if (listen(sock, 50)<0) {
		log_error("core", __FILE__, __LINE__, 0, "listen() error");
		closesocket(sock);
		return -1;
	}
	return sock;
}

int tcp_accept(int listensock, struct sockaddr_in *addr)
{
	struct sockaddr_in peer;
	int clientsock;
	unsigned int fromlen=sizeof(struct sockaddr_in);
/*	int timeout=500; */

	clientsock=accept(listensock, (struct sockaddr *)&addr, &fromlen);
	getpeername(clientsock, (struct sockaddr *)&peer, &fromlen);
/*	setsockopt(clientsock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)); */
	log_error("tcp", __FILE__, __LINE__, 5, "tcp_accept: connection from %s", inet_ntoa(peer.sin_addr));
	return clientsock;
}

int tcp_recv(TCP_SOCKET *socket, char *buffer, int len, int flags)
{
	int rc;

//	log_error("tcp", __FILE__, __LINE__, 1, "tcp_close: socket=%d, pid=%d, wc=%d", socket->socket, getpid(), socket->want_close);

	if (socket->socket==-1) return -1;
	if (socket->want_close) {
		tcp_close(socket, 1);
		return -1;
	}
#ifdef HAVE_SSL
	if (socket->ssl) {
		rc=ssl_read(socket->ssl, buffer, len);
		if (rc==0) rc=-1;
	} else {
		rc=recv(socket->socket, buffer, len, flags);
	}
#else
	rc=recv(socket->socket, buffer, len, flags);
#endif
//	log_error("tcp", __FILE__, __LINE__, 1, "tcp_close: socket=%d, pid=%d, bytes=%d", socket->socket, getpid(), rc);
	if (rc>0) {
		socket->atime=time(NULL);
		socket->bytes_in+=rc;
	}
	log_error("tcp", __FILE__, __LINE__, 5, "tcp_recv: %d bytes of binary data", rc);
	return rc;
}

int tcp_send(TCP_SOCKET *socket, const char *buffer, int len, int flags)
{
	int rc;

	if (socket->socket==-1) return -1;
	if (socket->want_close) {
		tcp_close(socket, 1);
		return -1;
	}
#ifdef HAVE_SSL
	if (socket->ssl) {
		rc=ssl_write(socket->ssl, buffer, len);
	} else {
		rc=send(socket->socket, buffer, len, flags);
	}
#else
	rc=send(socket->socket, buffer, len, flags);
#endif
	if (rc>0) {
		socket->atime=time(NULL);
		socket->bytes_out+=rc;
	}
	log_error("tcp", __FILE__, __LINE__, 5, "tcp_send: %d bytes of binary data", rc);
	return rc;
}

int tcp_send_nolog(TCP_SOCKET *socket, const char *buffer, int len, int flags)
{
	int rc;

	if (socket->socket==-1) return -1;
	if (socket->want_close) {
		tcp_close(socket, 1);
		return -1;
	}
#ifdef HAVE_SSL
	if (socket->ssl) {
		rc=ssl_write(socket->ssl, buffer, len);
	} else {
		rc=send(socket->socket, buffer, len, flags);
	}
#else
	rc=send(socket->socket, buffer, len, flags);
#endif
	if (rc>0) {
		socket->atime=time(NULL);
		socket->bytes_out+=rc;
	}
	return rc;
}

int tcp_fprintf(TCP_SOCKET *socket, const char *format, ...)
{
	char buffer[2048];
	va_list ap;
	int rc;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	log_error("tcp", __FILE__, __LINE__, 5, "tcp_fprintf: %s", buffer);
	rc=tcp_send_nolog(socket, buffer, strlen(buffer), 0);
	return rc;
}

int tcp_fgets(char *buffer, int max, TCP_SOCKET *socket)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int rc;
	short int x;

retry:
	if (!socket->recvbufsize) {
		x=sizeof(socket->recvbuf)-socket->recvbufoffset-socket->recvbufsize-2;
		obuffer=socket->recvbuf+socket->recvbufoffset+socket->recvbufsize;
		if ((rc=tcp_recv(socket, obuffer, x, 0))<0) {
			return -1;
		} else if (rc<1) {
			/*
			 * Add a one millisecond delay if zero bytes are
			 * returned.  This should drastically decrease the
			 * cpu usage of threads serving slow clients.
			 */
			msleep(1);
			goto retry;
		} else {
			socket->atime=time(NULL);
		}
		socket->recvbufsize+=rc;
	}
	obuffer=socket->recvbuf+socket->recvbufoffset;
	while ((n<max)&&(socket->recvbufsize>0)) {
		socket->recvbufoffset++;
		socket->recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) {
		log_error("tcp", __FILE__, __LINE__, 5, "tcp_fgets: %s", buffer);
		return n;
	}
	if (!lf) {
		if (socket->recvbufsize>0) {
			memmove(socket->recvbuf, socket->recvbuf+socket->recvbufoffset, socket->recvbufsize);
			memset(socket->recvbuf+socket->recvbufsize, 0, sizeof(socket->recvbuf)-socket->recvbufsize);
			socket->recvbufoffset=0;
		} else {
			memset(socket->recvbuf, 0, sizeof(socket->recvbuf));
			socket->recvbufoffset=0;
			socket->recvbufsize=0;
		}
		goto retry;
	}
	log_error("tcp", __FILE__, __LINE__, 5, "tcp_fgets: %s", buffer);
	return n;
}

int tcp_close(TCP_SOCKET *socket, short int owner_killed)
{
	if (!owner_killed) {
		socket->want_close=1;
	} else {
		socket->want_close=0;
#ifdef HAVE_SSL
		if (socket->ssl!=NULL) ssl_close(socket);
#endif
	}
	if (socket->socket>-1) {
		shutdown(socket->socket, 2);
		closesocket(socket->socket);
		socket->socket=-1;
	}
	return 0;
}
