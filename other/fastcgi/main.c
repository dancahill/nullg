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

static short unsigned int server_port=4110;
static char *server_addr="localhost";
static int htsocket;
#ifdef WIN32
static WSADATA wsaData;
#endif

char *str2hex(const char *format, ...)
{
	char *hex="0123456789ABCDEF";
	char *ptemp1, *ptemp2;
	static char inbuffer[2048];
	static char outbuffer[2048];
	va_list ap;

	memset(inbuffer, 0, sizeof(inbuffer));
	memset(outbuffer, 0, sizeof(outbuffer));
	va_start(ap, format);
	vsnprintf(inbuffer, sizeof(inbuffer)-1, format, ap);
	va_end(ap);
	ptemp1=inbuffer;
	ptemp2=outbuffer;
	while (*ptemp1) {
		if ((*ptemp1>32)&&(*ptemp1<128)&&(*ptemp1!='<')&&(*ptemp1!='>')&&(*ptemp1!='+')) {
			*ptemp2++=*ptemp1++;
		} else {
			*ptemp2++='%';
			*ptemp2++=hex[(unsigned int)*ptemp1/16];
			*ptemp2++=hex[(unsigned int)*ptemp1&15];
			*ptemp1++;
		}
	}
	return outbuffer;
}

/*
static void logdata(const char *format, ...)
{
	FILE *fp;
	va_list ap;
	char buffer[1024];
#ifdef WIN32
	char *file="C:\\webmail.log";
#else
	char *file="/tmp/webmail.log";
#endif

	fp=fopen(file, "a");
	if (fp!=NULL) {
		memset(buffer, 0, sizeof(buffer));
		va_start(ap, format);
		vsnprintf(buffer, sizeof(buffer)-1, format, ap);
		va_end(ap);
//		striprn(buffer);
		fprintf(fp, "%s", buffer);
		fclose(fp);
	}
}
*/

int htprintf(const char *format, ...)
{
	char buffer[1024];
	va_list ap;
	int len;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	len=strlen(buffer);
	send(htsocket, buffer, len, 0);
//	logdata(">> %s", buffer);
	return len;
}

int htfgets(char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	int n=0;

	while (n<max) {
		if (recv(fd, pbuffer, 1, 0)<1) return -1;
		n++;
		if (*pbuffer==10) {
			pbuffer++;
			break;
		}
		pbuffer++;
	}
	*pbuffer=0;
//	striprn(pbuffer);
//	logdata("<< %s", buffer);
	return n;
}

int htserver_connect()
{
	struct sockaddr_in server;
	struct hostent *hp;

#ifdef WIN32
	if (WSAStartup(0x101, &wsaData)) exit(0);
#endif
	if ((hp=gethostbyname(server_addr))==NULL) {
		printf("<CENTER>gethostbyname() '%s'</CENTER>\n", server_addr);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=(unsigned short)htons(server_port);
	if ((htsocket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
	if (connect(htsocket, (struct sockaddr *)&server, sizeof(server))<0) {
		printf("<CENTER>connect() '%s'</CENTER>\n", server_addr);
		return -1;
	}
	return 0;
}

void htserver_disconnect()
{
	shutdown(htsocket, 2);
	closesocket(htsocket);
#ifdef WIN32
	WSACleanup();
#endif
	fflush(stdout);
	return;
}

void ReadPOSTData(CONN *sid)
{
	char buffer[1024];
	int blocksize;
	int bytesleft;
	int rc;

	bytesleft=sid->in_ContentLength;
	if ((bytesleft<0)||(bytesleft>MAX_POSTSIZE)) exit(0);
	while (bytesleft>0) {
		if (bytesleft>sizeof(buffer)) {
			blocksize=sizeof(buffer);
		} else {
			blocksize=bytesleft;
		}
		if ((rc=read(fileno(stdin), buffer, blocksize))<0) return;
		send(htsocket, buffer, rc, 0);
//	buffer[rc]='\0';
//	logdata(">> %s\r\n", buffer);
		bytesleft-=rc;
	}
	return;
}

int main(int argc, char *argv[])
{
	CONN sid;
	char buffer[1024];
	char *ptemp;
	int i;

	setvbuf(stdout, NULL, _IONBF, 0);
#ifdef WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	memset((char *)&sid, 0, sizeof(sid));
	if ((ptemp=getenv("CONTENT_LENGTH"))!=NULL) {
		sid.in_ContentLength=atoi(ptemp);
		if (sid.in_ContentLength<0) {
			sid.in_ContentLength=0;
		}
	}
	if ((ptemp=getenv("HTTP_COOKIE"))!=NULL)     strncpy(sid.in_Cookie, ptemp, sizeof(sid.in_Cookie)-1);
	if ((ptemp=getenv("HTTP_HOST"))!=NULL)       strncpy(sid.in_Host, ptemp, sizeof(sid.in_Host)-1);
	if ((ptemp=getenv("HTTP_USER_AGENT"))!=NULL) strncpy(sid.in_UserAgent, ptemp, sizeof(sid.in_UserAgent)-1);
	if ((ptemp=getenv("PATH_INFO"))!=NULL)       strncpy(sid.in_PathInfo, ptemp, sizeof(sid.in_PathInfo)-1);
	if ((ptemp=getenv("QUERY_STRING"))!=NULL)    strncat(sid.in_QueryString, ptemp, sizeof(sid.in_QueryString)-1);
	if ((ptemp=getenv("REMOTE_ADDR"))!=NULL)     strncat(sid.in_RemoteAddr, ptemp, sizeof(sid.in_RemoteAddr)-1);
	if ((ptemp=getenv("REQUEST_METHOD"))!=NULL)  strncpy(sid.in_RequestMethod, ptemp, sizeof(sid.in_RequestMethod)-1);
	if ((ptemp=getenv("SCRIPT_NAME"))!=NULL)     strncat(sid.in_ScriptName, ptemp, sizeof(sid.in_ScriptName)-1);
	strncpy(sid.in_RequestURI, sid.in_PathInfo, sizeof(sid.in_RequestURI)-1);
	if (strlen(sid.in_QueryString)>0) {
		strncat(sid.in_RequestURI, "?", sizeof(sid.in_RequestURI)-strlen(sid.in_RequestURI)-1);
		strncat(sid.in_RequestURI, sid.in_QueryString, sizeof(sid.in_RequestURI)-strlen(sid.in_RequestURI)-1);
	}
	if (strlen(sid.in_RequestURI)==0) {
		strncpy(sid.in_RequestURI, "/", sizeof(sid.in_RequestURI)-1);
	}
	if (htserver_connect()!=0) exit(0);
	htprintf("%s %s HTTP/1.0\r\n", sid.in_RequestMethod, str2hex("%s", sid.in_RequestURI));
	htprintf("FCGI-URI: %s\r\n", sid.in_ScriptName);
	htprintf("User-Agent: %s\r\n", sid.in_UserAgent);
	htprintf("Cookie: %s\r\n", sid.in_Cookie);
	if (sid.in_ContentLength) {
		htprintf("Content-Length: %d\r\n", sid.in_ContentLength);
	}
	htprintf("\r\n");
	if (strcmp(sid.in_RequestMethod, "POST")==0) {
		ReadPOSTData(&sid);
	}
	/* IIS bursts into flames when we do a redirect, so use nph- */
	if (strstr(sid.in_ScriptName, "nph-")!=NULL) {
		printf("HTTP/1.0 200 OK\r\n");
	}
//	htfgets(buffer, sizeof(buffer)-1, htsocket);
//	if (strncasecmp(buffer, "HTTP/", 5)!=0) {
//		fwrite(buffer, sizeof(char), strlen(buffer), stdout);
//	}
	while (1) {
		if ((i=recv(htsocket, buffer, sizeof(buffer)-1, 0))<1) break;
//		buffer[i]='\0';
//		logdata("<< %s", buffer);
		fwrite(buffer, sizeof(char), i, stdout);
	}
	htserver_disconnect();
	return 0;
}
