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
#include "main.h"

void flushheader(CONN *sid)
{
	char line[256];

	if (sid->dat->out_headdone) return;
	if (!sid->dat->out_status) {
		sid->dat->out_headdone=1;
		return;
	}
	if ((sid->dat->out_bodydone)&&(!sid->dat->out_flushed)) {
		sid->dat->out_ContentLength=strlen(sid->dat->out_ReplyData);
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
	if (strcasestr(sid->dat->in_Protocol, "HTTP/1.1")!=NULL) {
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
		snprintf(line, sizeof(line)-1, "%s %d OK\r\n", sid->dat->out_Protocol, sid->dat->out_status);
		send(sid->socket, line, strlen(line), 0);
		if (strlen(sid->dat->out_CacheControl)) {
			snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", sid->dat->out_CacheControl);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_Connection)) {
			snprintf(line, sizeof(line)-1, "Connection: %s\r\n", sid->dat->out_Connection);
			send(sid->socket, line, strlen(line), 0);
		}
		if (sid->dat->out_bodydone) {
			snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", sid->dat->out_ContentLength);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_ContentDisposition)) {
			snprintf(line, sizeof(line)-1, "Content-Disposition: %s\r\n", sid->dat->out_ContentDisposition);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_Date)) {
			snprintf(line, sizeof(line)-1, "Date: %s\r\n", sid->dat->out_Date);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_Expires)) {
			snprintf(line, sizeof(line)-1, "Expires: %s\r\n", sid->dat->out_Expires);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_LastModified)) {
			snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", sid->dat->out_LastModified);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_Location)) {
			snprintf(line, sizeof(line)-1, "Location: %s\r\n", sid->dat->out_Location);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_Pragma)) {
			snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", sid->dat->out_Pragma);
			send(sid->socket, line, strlen(line), 0);
		}
		snprintf(line, sizeof(line)-1, "Server: %s %s\r\n", SERVER_NAME, SERVER_VERSION);
		send(sid->socket, line, strlen(line), 0);
		if (strlen(sid->dat->out_SetCookieUser)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookieUser);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_SetCookiePass)) {
			snprintf(line, sizeof(line)-1, "Set-Cookie: %s\r\n", sid->dat->out_SetCookiePass);
			send(sid->socket, line, strlen(line), 0);
		}
		if (strlen(sid->dat->out_ContentType)) {
			snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", sid->dat->out_ContentType);
			send(sid->socket, line, strlen(line), 0);
		} else {
			snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
			send(sid->socket, line, strlen(line), 0);
		}
	}
	sid->dat->out_headdone=1;
	return;
}

void flushbuffer(CONN *sid)
{
	char *pTemp=sid->dat->out_ReplyData;
	unsigned int dcount;

	flushheader(sid);
	if (strlen(pTemp)==0) return;
	sid->dat->out_flushed=1;
	while (strlen(pTemp)) {
		dcount=512;
		if (strlen(pTemp)<dcount) dcount=strlen(pTemp);
		if (proc.RunAsCGI) {
			fwrite(pTemp, sizeof(char), dcount, stdout);
		} else {
			send(sid->socket, pTemp, dcount, 0);
		}
		pTemp+=dcount;
	}
	memset(sid->dat->out_ReplyData, 0, sizeof(sid->dat->out_ReplyData));
	return;
}

int prints(CONN *sid, const char *format, ...)
{
	unsigned char buffer[2048];
	va_list ap;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	if (strlen(sid->dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	strcat(sid->dat->out_ReplyData, buffer);
	if (strlen(sid->dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	return 0;
}

int sgets(CONN *sid, char *buffer, int max, int fd)
{
	int n=0;
	int rc;

//	if (sid==-1) return -1;
	sid->atime=time(NULL);
	if (proc.RunAsCGI) {
		fgets(buffer, sizeof(buffer)-1, stdin);
		return strlen(buffer);
	}
	while (n<max) {
		if ((rc=recv(sid->socket, buffer, 1, 0))<1) {
			closeconnect(sid, 2);
			return -1;
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

int filesend(CONN *sid, unsigned char *file)
{
	struct stat sb;
	FILE *fp;
	char fileblock[2048];
	int blocksize;
	int ich;

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
	sid->dat->out_ContentLength=sb.st_size;
	send_fileheader(sid, 1, 200, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
	fp=fopen(file, "rb");
	if (fp==NULL) return -1;
	blocksize=0;
	while ((ich=getc(fp))!=EOF) {
		if (blocksize>sizeof(fileblock)-1) {
			if (proc.RunAsCGI) {
				fwrite(fileblock, sizeof(char), blocksize, stdout);
			} else {
				send(sid->socket, fileblock, blocksize, 0);
			}
			blocksize=0;
		}
		fileblock[blocksize]=ich;
		blocksize++;
	}
	if (blocksize) {
		if (proc.RunAsCGI) {
			fwrite(fileblock, sizeof(char), blocksize, stdout);
		} else {
			send(sid->socket, fileblock, blocksize, 0);
		}
		blocksize=0;
	}
	fclose(fp);
	sid->dat->out_headdone=1;
	sid->dat->out_bodydone=1;
	sid->dat->out_flushed=1;
	sid->dat->out_ReplyData[0]='\0';
	flushbuffer(sid);
	return 0;
}

int tcp_send(int s, const char *buffer, int len, int flags)
{
	return send(s, buffer, len, flags);
}
