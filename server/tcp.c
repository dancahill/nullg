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

int tcp_accept(int listensock, struct sockaddr *addr)
{
	int clientsock;
	int fromlen;

	fromlen=sizeof(struct sockaddr);
	clientsock=accept(listensock, addr, &fromlen);
	return clientsock;
}

int tcp_recv(TCP_SOCKET *socket, char *buffer, int len, int flags)
{
	int rc;

	if (socket->socket==-1) return -1;
	if (socket->want_close) {
		tcp_close(socket, 1);
		return -1;
	}
#ifdef HAVE_LIBSSL
	if (socket->ssl) {
		rc=SSL_read(socket->ssl, buffer, len);
		if (rc==0) rc=-1;
	} else {
		rc=recv(socket->socket, buffer, len, flags);
	}
#else
	rc=recv(socket->socket, buffer, len, flags);
#endif
	if (rc>0) {
		socket->atime=time(NULL);
		socket->bytes_in+=rc;
	}
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
#ifdef HAVE_LIBSSL
	if (socket->ssl) {
		rc=SSL_write(socket->ssl, buffer, len);
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
	unsigned char buffer[2048];
	va_list ap;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	return tcp_send(socket, buffer, strlen(buffer), 0);
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
	if (n>max-1) return n;
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
	return n;
}

int tcp_close(TCP_SOCKET *socket, short int owner_killed)
{
	short int tmpsock;

	if (socket->socket<0) return 0;
	if (!owner_killed) {
		socket->want_close=1;
	} else {
		socket->want_close=0;
		tmpsock=socket->socket;
		socket->socket=-1;
		/* shutdown(x,0=recv, 1=send, 2=both) */
		shutdown(tmpsock, 2);
		closesocket(tmpsock);
#ifdef HAVE_LIBSSL
		ssl_close(socket);
#endif
	}
	return 0;
}
