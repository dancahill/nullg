/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "mail_basher.h"

#ifdef WIN32
static WSADATA wsaData;
#endif

char *host="localhost";
#ifdef HAVE_SSL
short int port=465;
#else
short int port=25;
#endif

short int numconn;
short int perconn;

CONN *conn;

static void striprn(char *string)
{
	int i;

	while (1) {
		i=strlen(string);
		if (i<1) break;
		if (string[i-1]=='\r') { string[i-1]='\0'; continue; }
		if (string[i-1]=='\n') { string[i-1]='\0'; continue; }
		break;
	};
}

static int wmprintf(CONN *sid, const char *format, ...)
{
	char buffer[16384];
	va_list ap;
	int len;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
#ifdef HAVE_SSL
	SSL_write(sid->ssl, buffer, strlen(buffer));
#else
	send(sid->socket, buffer, strlen(buffer), 0);
#endif
	len=strlen(buffer);
//printf("%s", buffer);
	return len;
}

static int wmfgets(CONN *sid, char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int rc;
	short int x;
	short int retries=10;

retry:
	if (!sid->recvbufsize) {
		x=sizeof(sid->recvbuf)-sid->recvbufoffset-sid->recvbufsize-2;
		obuffer=sid->recvbuf+sid->recvbufoffset+sid->recvbufsize;
#ifdef HAVE_SSL
		rc=SSL_read(sid->ssl, obuffer, x);
#else
		rc=recv(fd, obuffer, x, 0);
#endif
		if (rc<0) {
			return -1;
		} else if (rc<1) {
//			msleep(1);
			if (retries-->0) goto retry;
		}
		sid->recvbufsize+=rc;
	}
	obuffer=sid->recvbuf+sid->recvbufoffset;
	while ((n<max)&&(sid->recvbufsize>0)) {
		sid->recvbufoffset++;
		sid->recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) return n;
	if (!lf) {
		if (sid->recvbufsize>0) {
			memmove(sid->recvbuf, sid->recvbuf+sid->recvbufoffset, sid->recvbufsize);
			memset(sid->recvbuf+sid->recvbufsize, 0, sizeof(conn[0].recvbuf)-sid->recvbufsize);
			sid->recvbufoffset=0;
		} else {
			memset(sid->recvbuf, 0, sizeof(conn[0].recvbuf));
			sid->recvbufoffset=0;
			sid->recvbufsize=0;
		}
		if (retries>0) goto retry;
	}
//printf("%s", buffer);
	striprn(buffer);
	return n;
}

static int smtp_connect(CONN *sid, char *host, short int port)
{
	struct hostent *hp;
	struct sockaddr_in server;
#ifdef HAVE_SSL
	X509 *server_cert;
#endif

	if ((hp=gethostbyname(host))==NULL) {
		printf("Host lookup error for %s\r\n", host);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=htons(port);
	if ((sid->socket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
	if (connect(sid->socket, (struct sockaddr *)&server, sizeof(server))<0) {
		printf("Connect error for %s:%d\r\n", host, port);
		return -1;
	}
	sid->recvbufsize=0;
	sid->recvbufoffset=0;
	memset(sid->recvbuf, 0, sizeof(conn[0].recvbuf));
#ifdef HAVE_SSL
	sid->ssl=SSL_new(sid->ctx);
	SSL_set_fd(sid->ssl, sid->socket);
	if (SSL_connect(sid->ssl)==-1) { perror("socket: "); exit -1; }
	/* the rest is optional */
	printf("SSL connection using %s\r\n", SSL_get_cipher(sid->ssl));
	if ((server_cert=SSL_get_peer_certificate(sid->ssl))==NULL) exit-1;
//	printf("Server certificate:\r\n");
	X509_free(server_cert);
#endif
	return 0;
}

#ifdef WIN32
unsigned _stdcall smtp_test1(void *x)
#else
void *smtp_test1(void *x)
#endif
{
	int sid=(int)x;
	char inbuffer[8192];
	char outbuffer[16384];
	int i;
#ifdef HAVE_SSL
	SSL_METHOD *meth;
#endif

#ifdef HAVE_SSL
	SSLeay_add_ssl_algorithms();
	meth=SSLv2_client_method();
	SSL_load_error_strings();
	conn[sid].ctx=SSL_CTX_new(meth);
#endif
	for (i=0;i<perconn;i++) {
		if (smtp_connect(&conn[sid], host, port)<0) {
			conn[sid].socket=-1;
			return 0;
		}
		memset(outbuffer, 'x', sizeof(outbuffer));
		outbuffer[sizeof(outbuffer)-1]='\0';
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
//		wmprintf(&conn[sid], "HELO %s", outbuffer);
//		wmprintf(&conn[sid], "\r\n");
		wmprintf(&conn[sid], "HELO localhost\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
		wmprintf(&conn[sid], "MAIL FROM: <me@here.com>\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
		wmprintf(&conn[sid], "RCPT TO: <nobody@here.com>\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
		wmprintf(&conn[sid], "DATA\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
		wmprintf(&conn[sid], "Subject: Testing\r\n");
		wmprintf(&conn[sid], "\r\n");
		wmprintf(&conn[sid], "this is a test.\r\n");
		wmprintf(&conn[sid], ".\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
		wmprintf(&conn[sid], "QUIT\r\n");
		if (wmfgets(&conn[sid], inbuffer, sizeof(inbuffer)-1, conn[sid].socket)<0) goto done;
done:
		closesocket(conn[sid].socket);
		printf(".");
	}
	conn[sid].socket=-1;
	return 0;
}

int main(int argc, char *argv[])
{
	pthread_attr_t thr_attr;
	pthread_t blah;
	int i;
	short int found;
	struct timeval ttime;
	struct timezone tzone;
	int x, y;

	printf("SMTP Basher\r\n");
#ifdef WIN32
	if (WSAStartup(0x101, &wsaData)) {
		printf("Winsock init error\r\n");
		return -1;
	}
#endif
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc!=3) {
		printf("progname numconn perconn\r\n");
		return 0;
	}
	numconn=atoi(argv[1]);
	perconn=atoi(argv[2]);
	if (numconn<1) numconn=1;
	if (perconn<1) perconn=1;
	conn=calloc(numconn, sizeof(CONN));
	if (conn==NULL) {
		printf("\r\nconn calloc(%d, %d) failed\r\n", numconn, sizeof(CONN));
		exit(-2);
	}
	gettimeofday(&ttime, &tzone);
	x=ttime.tv_sec; y=ttime.tv_usec;
	for (i=0;i<numconn;i++) {
		conn[i].socket=-1;
		if (pthread_attr_init(&thr_attr)) exit(-2);
#ifndef OLDLINUX
		if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(-2);
#endif
		if (pthread_create(&blah, &thr_attr, smtp_test1, (void *)i)==-1) {
			printf("thread failed to start.\r\n");
			exit(-2);
		}
	}
sleeper:
	sleep(1);
	found=0;
	for (i=0;i<numconn;i++) {
		if (conn[i].socket>0) found=1;
	}
	if (found) goto sleeper;
	gettimeofday(&ttime, &tzone);
	x=((ttime.tv_sec-x)*1000000)-y+ttime.tv_usec;
	printf("\r\nQuery speed test finished\r\n");
	printf("queries   = %d\r\n", numconn*perconn);
	printf("time      = %1.3f seconds\r\n", (float)x/(float)1000000);
	printf("time/conn = %1.3f seconds\r\n", (float)x/(float)1000000/(float)(numconn*perconn));
	free(conn);
	return 0;
}
