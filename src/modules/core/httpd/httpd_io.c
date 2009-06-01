/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "httpd_main.h"
#ifndef WIN32
#include <fcntl.h>
#endif

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

void flushheader(CONN *sid)
{
	obj_t *htobj, *hrobj, *cobj;
	char *Protocol;
	char line[256];
	char tmpnam[64];
	char *p;
	short int ctype=0;

	if (sid->N==NULL) goto err;
	if (sid->dat->out_headdone) return;
	htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	hrobj=nes_getobj(sid->N, &sid->N->g, "_HEADER");
	if ((htobj->val->type!=NT_TABLE)||(hrobj->val->type!=NT_TABLE)) goto err;
	if (!sid->dat->out_status) {
		sid->dat->out_headdone=1;
		return;
	}
	Protocol=nes_getstr(sid->N, htobj, "SERVER_PROTOCOL");
	if ((sid->dat->out_bodydone)&&(!sid->dat->out_flushed)) {
		sid->dat->out_ContentLength=sid->dat->out_bytecount;
	}
	if ((sid->dat->out_status<200)||(sid->dat->out_status>=400)) {
		nes_setstr(sid->N, hrobj, "CONNECTION", "Close", strlen("Close"));
	} else if ((strcasecmp(nes_getstr(sid->N, htobj, "HTTP_CONNECTION"), "Keep-Alive")==0)&&(sid->dat->out_bodydone)) {
		nes_setstr(sid->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	} else if ((strlen(nes_getstr(sid->N, htobj, "HTTP_CONNECTION"))==0)&&(sid->dat->out_bodydone)&&(strcasecmp(Protocol, "HTTP/1.1")==0)) {
		nes_setstr(sid->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	} else {
		nes_setstr(sid->N, hrobj, "CONNECTION", "Close", strlen("Close"));
	}
	if (p_strcasestr(Protocol, "HTTP/1.1")!=NULL) {
		nes_setstr(sid->N, hrobj, "PROTOCOL", "HTTP/1.1", strlen("HTTP/1.1"));
	} else {
		nes_setstr(sid->N, hrobj, "PROTOCOL", "HTTP/1.0", strlen("HTTP/1.0"));
	}
	memset(line, 0, sizeof(line));
	if (tcp_fprintf(&sid->socket, "%s %d OK\r\n", nes_getstr(sid->N, hrobj, "PROTOCOL"), sid->dat->out_status)<0) goto err;
	/* log_error(proc->N, MODSHORTNAME "responses", __FILE__, __LINE__, 1, "%s:%d %s %d OK\r\n", sid->socket.RemoteAddr, sid->socket.RemotePort, nes_getstr(sid->N, hrobj, "PROTOCOL"), sid->dat->out_status); */
	for (cobj=hrobj->val->d.table.f; cobj; cobj=cobj->next) {
		if ((cobj->val->type!=NT_STRING)&&(cobj->val->type!=NT_NUMBER)) continue;
		strncpy(tmpnam, cobj->name, sizeof(tmpnam)-1);
		if (strcmp(tmpnam, "CONTENT_TYPE")==0) ctype=1;
		for (p=tmpnam;p[0];p++) {
			if (p[0]=='_') {
				p[0]='-';
				if (isalpha(p[1])) { p++; continue; }
			} else {
				*p=tolower(*p);
			}
		}
		tmpnam[0]=toupper(tmpnam[0]);
		if (cobj->val->type==NT_STRING) {
			if (tcp_fprintf(&sid->socket, "%s: %s\r\n", tmpnam, cobj->val->d.str?cobj->val->d.str:"")<0) goto err;
		} else {
			if (tcp_fprintf(&sid->socket, "%s: %s\r\n", tmpnam, nes_tostr(sid->N, cobj))<0) goto err;
		}
		/* log_error(proc->N, MODSHORTNAME "responses", __FILE__, __LINE__, 1, "[%s][%s]", tmpnam, nes_tostr(sid->N, cobj)); */
	}
	if (tcp_fprintf(&sid->socket, "Server: %s %s\r\n", SERVER_NAME, PACKAGE_VERSION)<0) goto err;
	if (tcp_fprintf(&sid->socket, "Status: %d\r\n", sid->dat->out_status)<0) goto err;
	if (sid->dat->out_bodydone) {
		if (tcp_fprintf(&sid->socket, "Content-Length: %d\r\n", sid->dat->out_ContentLength)<0) goto err;
	}
	if (!ctype) {
		if (tcp_fprintf(&sid->socket, "Content-Type: text/plain\r\n")<0) goto err;
	}
	if (tcp_fprintf(&sid->socket, "\r\n")<0) goto err;
	sid->dat->out_headdone=1;
	return;
err:
	tcp_close(&sid->socket, 1);
	sid->dat->out_headdone=1;
	return;
}

void flushbuffer(CONN *sid)
{
	char *pTemp=sid->dat->replybuf;
	short int dcount;
	short int str_len;

	flushheader(sid);
	str_len=strlen(pTemp);
	if (str_len<1) return;
	sid->dat->out_flushed=1;
	while (str_len>0) {
		dcount=4096;
		if (str_len<dcount) dcount=str_len;
		dcount=tcp_send(&sid->socket, pTemp, dcount, 0);
		if (dcount<0) break;
		str_len-=dcount;
		pTemp+=dcount;
	}
	memset(sid->dat->replybuf, 0, sizeof(sid->dat->replybuf));
	return;
}

int prints(CONN *sid, const char *format, ...)
{
	char *buffer=getbuffer(sid);
	short int str_len1, str_len2;
	va_list ap;

	if (sid==NULL) return -1;
	sid->socket.atime=time(NULL);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, format, ap);
	str_len1=strlen(buffer);
	str_len2=strlen(sid->dat->replybuf);
	va_end(ap);
	buffer[sizeof(sid->dat->smallbuf[0])-1]='\0';
	sid->dat->out_bytecount+=str_len1;
	if (str_len2+sizeof(sid->dat->smallbuf[0])>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
		str_len2=0;
	}
	strcpy(sid->dat->replybuf+str_len2, buffer);
	str_len2+=str_len1;
	if (str_len2+sizeof(sid->dat->smallbuf[0])>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	return str_len1;
}

int raw_prints(CONN *sid, const char *format, ...)
{
	char buffer[2048];
	va_list ap;

	if (sid==NULL) return -1;
	sid->socket.atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	sid->dat->out_bytecount+=strlen(buffer);
	flushbuffer(sid);
	return tcp_send(&sid->socket, buffer, strlen(buffer), 0);
}

int filesend(CONN *sid, char *file)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *modtime=nes_getstr(sid->N, htobj, "HTTP_IF_MODIFIED_SINCE");
	struct stat sb;
	int fp;
	int bytesleft;
	int blocksize;
	char timebuf[100];
	char fileblock[8192];

	decodeurl(file);
	fixslashes(file);
	if (strstr(file, "..")!=NULL) return -1;
	if (stat(file, &sb)!=0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
	/* need some check for IfModifiedSince here */
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&sb.st_mtime));
	if (strcasecmp(timebuf, modtime)==0) {
		sid->dat->out_bodydone=1;
		send_header(sid, 1, 304, "1", get_mime_type(sid, file), sb.st_size, sb.st_mtime);
		flushheader(sid);
//		goto fileend;
//		sid->dat->out_headdone=1;
//		sid->dat->out_flushed=1;
		sid->dat->replybuf[0]='\0';
//		flushbuffer(sid);
		return 0;
	}
	if ((fp=open(file, O_RDONLY|O_BINARY))==-1) return -1;
	sid->dat->out_bytecount=sb.st_size;
	sid->dat->out_bodydone=1;
	bytesleft=sb.st_size;
	send_header(sid, 1, 200, "1", get_mime_type(sid, file), sb.st_size, sb.st_mtime);
	flushheader(sid);
	/* sendfile() can't be used without screwing up our timeouts, so we do this */
	for (;;) {
		if (bytesleft<=0) break;
		blocksize=sizeof(fileblock);
		if (bytesleft<blocksize) bytesleft=blocksize;
		blocksize=read(fp, fileblock, blocksize);
		if (blocksize<1) break;
		bytesleft-=blocksize;
		if (tcp_send(&sid->socket, fileblock, blocksize, 0)<0) break;
		sid->socket.atime=time(NULL);
	}
	close(fp);
//fileend:
	sid->dat->out_headdone=1;
	sid->dat->out_flushed=1;
	sid->dat->replybuf[0]='\0';
	flushbuffer(sid);
	return 0;
}

int closeconnect(CONN *sid, int exitflag)
{
	if (exitflag<2) {
		flushbuffer(sid);
	}
	if (sid!=NULL) {
		if (sid->dat!=NULL) {
			if (sid->dat->wm!=NULL) {
				tcp_close(&sid->dat->wm->socket, 1);
			}
		}
		tcp_close(&sid->socket, 1);
	}
	if ((exitflag)&&(sid!=NULL)) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", sid->socket.RemoteAddr, sid->socket.RemotePort);
		if (sid->dat!=NULL) { free(sid->dat); sid->dat=NULL; }
		sid->N=nes_endstate(sid->N);
#ifdef WIN32
		CloseHandle(sid->handle);
#endif
		memset(sid, 0, sizeof(htproc.conn[0]));
		sid->socket.socket=-1;
#ifdef WIN32
		_endthread();
#else
		pthread_exit(0);
#endif
	}
	return 0;
}
