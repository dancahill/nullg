/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "http_main.h"

void flushheader(CONN *sid)
{
	char line[256];

	if (sid->dat->out_headdone) return;
	if (!sid->dat->out_status) {
		sid->dat->out_headdone=1;
		return;
	}
	if ((sid->dat->out_bodydone)&&(!sid->dat->out_flushed)) {
		sid->dat->out_ContentLength=sid->dat->out_bytecount;
	}
	if ((sid->dat->out_status<200)||(sid->dat->out_status>=300)) {
		snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Close");
	} else if ((strcasecmp(sid->dat->in_Connection, "Keep-Alive")==0)&&(sid->dat->out_bodydone)) {
		snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Keep-Alive");
	} else if ((strlen(sid->dat->in_Connection)==0)&&(sid->dat->out_bodydone)&&(strcasecmp(sid->dat->in_Protocol, "HTTP/1.1")==0)) {
		snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Keep-Alive");
	} else {
		snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Close");
	}
	if (p_strcasestr(sid->dat->in_Protocol, "HTTP/1.1")!=NULL) {
		snprintf(sid->dat->out_Protocol, sizeof(sid->dat->out_Protocol)-1, "HTTP/1.1");
	} else {
		snprintf(sid->dat->out_Protocol, sizeof(sid->dat->out_Protocol)-1, "HTTP/1.0");
	}
	if (proc.RunAsCGI) {
		/* IIS bursts into flames when we do a redirect, so use nph- */
		if (strstr(sid->dat->in_ScriptName, "nph-")!=NULL) {
			snprintf(line, sizeof(line)-1, "%s %d OK\r\n", sid->dat->out_Protocol, sid->dat->out_status);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_CacheControl)) {
			snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", sid->dat->out_CacheControl);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_ContentDisposition)) {
			snprintf(line, sizeof(line)-1, "Content-Disposition: %s\r\n", sid->dat->out_ContentDisposition);
			printf("%s", line);
		}
		if (sid->dat->out_ContentLength>-1) {
			snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", sid->dat->out_ContentLength);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_Date)) {
			snprintf(line, sizeof(line)-1, "Date: %s\r\n", sid->dat->out_Date);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_Expires)) {
			snprintf(line, sizeof(line)-1, "Expires: %s\r\n", sid->dat->out_Expires);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_LastModified)) {
			snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", sid->dat->out_LastModified);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_Location)) {
			snprintf(line, sizeof(line)-1, "Location: %s\r\n", sid->dat->out_Location);
			sid->dat->out_status=303;
			printf("%s", line);
		}
		if (strlen(sid->dat->out_Pragma)) {
			snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", sid->dat->out_Pragma);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_SetCookieUser)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookieUser);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_SetCookiePass)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookiePass);
			printf("%s", line);
		}
		if (sid->dat->out_status) {
			snprintf(line, sizeof(line)-1, "Status: %d", sid->dat->out_status);
			if (sid->dat->out_status==200) {
				strncat(line, " OK", sizeof(line)-1);
			} else if (sid->dat->out_status==303) {
				strncat(line, " See Other", sizeof(line)-1);
			} else {
				strncat(line, " OK", sizeof(line)-1);
			}
			strncat(line, "\r\n", sizeof(line)-1);
			printf("%s", line);
		}
		if (strlen(sid->dat->out_ContentType)) {
			snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", sid->dat->out_ContentType);
			printf("%s", line);
		} else {
			snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
			printf("%s", line);
		}
	} else {
		if (sid->dat->cgi_lite) {
			snprintf(line, sizeof(line)-1, "Status: %d\r\n", sid->dat->out_status);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		} else {
			snprintf(line, sizeof(line)-1, "%s %d OK\r\n", sid->dat->out_Protocol, sid->dat->out_status);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_CacheControl)) {
			snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", sid->dat->out_CacheControl);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_Connection)) {
			snprintf(line, sizeof(line)-1, "Connection: %s\r\n", sid->dat->out_Connection);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (sid->dat->out_bodydone) {
			snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", sid->dat->out_ContentLength);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_ContentDisposition)) {
			snprintf(line, sizeof(line)-1, "Content-Disposition: %s\r\n", sid->dat->out_ContentDisposition);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_Date)) {
			snprintf(line, sizeof(line)-1, "Date: %s\r\n", sid->dat->out_Date);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_Expires)) {
			snprintf(line, sizeof(line)-1, "Expires: %s\r\n", sid->dat->out_Expires);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_LastModified)) {
			snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", sid->dat->out_LastModified);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_Location)) {
			snprintf(line, sizeof(line)-1, "Location: %s\r\n", sid->dat->out_Location);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_Pragma)) {
			snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", sid->dat->out_Pragma);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		snprintf(line, sizeof(line)-1, "Server: %s %s\r\n", SERVER_NAME, SERVER_VERSION);
		if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		if (strlen(sid->dat->out_SetCookieUser)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookieUser);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_SetCookiePass)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookiePass);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
		if (strlen(sid->dat->out_ContentType)) {
			snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", sid->dat->out_ContentType);
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		} else {
			snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
			if (send(sid->socket, line, strlen(line), 0)<0) goto err;
		}
	}
	sid->dat->out_headdone=1;
	return;
err:
	shutdown(sid->socket, 2);
	closesocket(sid->socket);
	sid->socket=-1;
	sid->dat->out_headdone=1;
	return;
}

void flushbuffer(CONN *sid)
{
	char *pTemp=sid->dat->out_ReplyData;
	short int dcount;
	short int str_len;

	flushheader(sid);
	str_len=strlen(pTemp);
	if (str_len<1) return;
	sid->dat->out_flushed=1;
	while (str_len>0) {
		dcount=4096;
		if (str_len<dcount) dcount=str_len;
		if (proc.RunAsCGI) {
			dcount=fwrite(pTemp, sizeof(char), dcount, stdout);
		} else {
			dcount=send(sid->socket, pTemp, dcount, 0);
		}
		if (dcount<0) break;
		str_len-=dcount;
		pTemp+=dcount;
	}
	memset(sid->dat->out_ReplyData, 0, sizeof(sid->dat->out_ReplyData));
	return;
}

int prints(CONN *sid, const char *format, ...)
{
	unsigned char *buffer=getbuffer(sid);
	short int str_len1, str_len2;
	va_list ap;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
	va_start(ap, format);
	str_len1=vsnprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, format, ap);
	str_len2=strlen(sid->dat->out_ReplyData);
	va_end(ap);
	buffer[sizeof(sid->dat->smallbuf[0])-1]='\0';
	sid->dat->out_bytecount+=str_len1;
	if (str_len2+sizeof(sid->dat->smallbuf[0])>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
		str_len2=0;
	}
	strcpy(sid->dat->out_ReplyData+str_len2, buffer);
	str_len2+=str_len1;
	if (str_len2+sizeof(sid->dat->smallbuf[0])>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	return 0;
}

int raw_prints(CONN *sid, const char *format, ...)
{
	unsigned char buffer[2048];
	va_list ap;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	sid->dat->out_bytecount+=strlen(buffer);
	flushbuffer(sid);
	if (proc.RunAsCGI) {
		return fwrite(buffer, sizeof(char), strlen(buffer), stdout);
	} else {
		return send(sid->socket, buffer, strlen(buffer), 0);
	}
}
/*
int sgets(CONN *sid, char *buffer, int max, int fd)
{
	int n=0;
	int rc;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
	if (proc.RunAsCGI) {
		fgets(buffer, sizeof(buffer)-1, stdin);
		return strlen(buffer);
	}
	while (n<max) {
		if ((rc=recv(sid->socket, buffer, 1, 0))<1) {
			return -1;
		} else if (rc<1) {
			msleep(1);
		}
		n++;
		if (*buffer=='\n') {
			buffer++;
			break;
		}
		buffer++;
	}
	*buffer=0;
	return n;
}
*/
int sgets(CONN *sid, char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int rc;
	short int x;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
retry:
	if (!sid->dat->recvbufsize) {
		x=sizeof(sid->dat->recvbuf)-sid->dat->recvbufoffset-sid->dat->recvbufsize-2;
		obuffer=sid->dat->recvbuf+sid->dat->recvbufoffset+sid->dat->recvbufsize;
		if ((rc=recv(fd, obuffer, x, 0))<0) {
			return -1;
		} else if (rc<1) {
			/*
			 * Add a one millisecond delay if zero bytes are
			 * returned.  This should drastically decrease the
			 * cpu usage of threads serving slow clients.
			 */
			msleep(1);
			goto retry;
		}
		sid->dat->recvbufsize+=rc;
	}
	obuffer=sid->dat->recvbuf+sid->dat->recvbufoffset;
	while ((n<max)&&(sid->dat->recvbufsize>0)) {
		sid->dat->recvbufoffset++;
		sid->dat->recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) return n;
	if (!lf) {
		if (sid->dat->recvbufsize>0) {
			memmove(sid->dat->recvbuf, sid->dat->recvbuf+sid->dat->recvbufoffset, sid->dat->recvbufsize);
			memset(sid->dat->recvbuf+sid->dat->recvbufsize, 0, sizeof(sid->dat->recvbuf)-sid->dat->recvbufsize);
			sid->dat->recvbufoffset=0;
		} else {
			memset(sid->dat->recvbuf, 0, sizeof(sid->dat->recvbuf));
			sid->dat->recvbufoffset=0;
			sid->dat->recvbufsize=0;
		}
		goto retry;
	}
	return n;
}

int filesend(CONN *sid, unsigned char *file)
{
	struct stat sb;
	int fp;
	char fileblock[8192];
	int bytesleft;
	int blocksize;

	decodeurl(file);
	fixslashes(file);
	if (strstr(file, "..")!=NULL) return -1;
	if (stat(file, &sb)!=0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
/*	if (strlen(sid->dat->in_IfModifiedSince)) {
		send_fileheader(sid, 1, 304, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
		sid->dat->out_headdone=1;
		sid->dat->out_bodydone=1;
		sid->dat->out_flushed=1;
		sid->dat->out_ReplyData[0]='\0';
		flushbuffer(sid);
		return 0;
	}
*/
	if ((fp=open(file, O_RDONLY|O_BINARY))==-1) return -1;
	sid->dat->out_bytecount=sb.st_size;
	sid->dat->out_bodydone=1;
	bytesleft=sb.st_size;
	send_fileheader(sid, 1, 200, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
	// sendfile() can't be used without screwing up our timeouts
//#ifdef LINUX
//	sendfile(sid->socket, fp, NULL, sb.st_size);
//#else
	for (;;) {
		if (bytesleft<=0) break;
		blocksize=sizeof(fileblock);
		if (bytesleft<blocksize) bytesleft=blocksize;
		blocksize=read(fp, fileblock, blocksize);
		if (blocksize<1) break;
		bytesleft-=blocksize;
		if (proc.RunAsCGI) {
			fwrite(fileblock, sizeof(char), blocksize, stdout);
		} else {
			if (send(sid->socket, fileblock, blocksize, 0)<0) break;
		}
		sid->atime=time(NULL);
	}
//#endif
	close(fp);
	sid->dat->out_headdone=1;
	sid->dat->out_flushed=1;
	sid->dat->out_ReplyData[0]='\0';
	flushbuffer(sid);
	return 0;
}

int tcp_send(CONN *sid, int s, const char *buffer, int len, int flags)
{
	return send(s, buffer, len, flags);
}
