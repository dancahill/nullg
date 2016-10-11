/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#include "nsp/nsplib.h"

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

void flushheader(CONN *conn)
{
	obj_t *htobj, *hrobj, *cobj;
	char *Protocol;
	char line[256];
	char tmpnam[64];
	char *p;
	short int ctype = 0;

	if (conn->N == NULL) goto err;
	if (conn->dat->out_headdone) return;
	htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	hrobj = nsp_getobj(conn->N, &conn->N->g, "_HEADER");
	if ((htobj->val->type != NT_TABLE) || (hrobj->val->type != NT_TABLE)) goto err;
	if (!conn->dat->out_status) {
		conn->dat->out_headdone = 1;
		return;
	}
	Protocol = nsp_getstr(conn->N, htobj, "SERVER_PROTOCOL");
	if ((conn->dat->out_bodydone) && (!conn->dat->out_flushed)) {
		conn->dat->out_ContentLength = conn->dat->out_bytecount;
	}
	if ((conn->dat->out_status < 200) || (conn->dat->out_status >= 400)) {
		nsp_setstr(conn->N, hrobj, "CONNECTION", "Close", strlen("Close"));
	}
	else if ((strcasecmp(nsp_getstr(conn->N, htobj, "HTTP_CONNECTION"), "Keep-Alive") == 0) && (conn->dat->out_bodydone)) {
		nsp_setstr(conn->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	}
	else if ((strlen(nsp_getstr(conn->N, htobj, "HTTP_CONNECTION")) == 0) && (conn->dat->out_bodydone) && (strcasecmp(Protocol, "HTTP/1.1") == 0)) {
		nsp_setstr(conn->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	}
	else {
		nsp_setstr(conn->N, hrobj, "CONNECTION", "Close", strlen("Close"));
	}
	if (p_strcasestr(Protocol, "HTTP/1.1") != NULL) {
		nsp_setstr(conn->N, hrobj, "PROTOCOL", "HTTP/1.1", strlen("HTTP/1.1"));
	}
	else {
		nsp_setstr(conn->N, hrobj, "PROTOCOL", "HTTP/1.0", strlen("HTTP/1.0"));
	}
	memset(line, 0, sizeof(line));
	if (tcp_fprintf(&conn->socket, "%s %d OK\r\n", nsp_getstr(conn->N, hrobj, "PROTOCOL"), conn->dat->out_status) < 0) goto err;
	/* log_error(proc->N, MODSHORTNAME "responses", __FILE__, __LINE__, 1, "%s:%d %s %d OK\r\n", conn->socket.RemoteAddr, conn->socket.RemotePort, nsp_getstr(conn->N, hrobj, "PROTOCOL"), conn->dat->out_status); */
	for (cobj = hrobj->val->d.table.f; cobj; cobj = cobj->next) {
		if ((cobj->val->type != NT_STRING) && (cobj->val->type != NT_NUMBER)) continue;
		strncpy(tmpnam, cobj->name, sizeof(tmpnam) - 1);
		if (strcmp(tmpnam, "CONTENT_TYPE") == 0) ctype = 1;
		for (p = tmpnam;p[0];p++) {
			if (p[0] == '_') {
				p[0] = '-';
				if (isalpha(p[1])) { p++; continue; }
			}
			else {
				*p = tolower(*p);
			}
		}
		tmpnam[0] = toupper(tmpnam[0]);
		if (cobj->val->type == NT_STRING) {
			if (tcp_fprintf(&conn->socket, "%s: %s\r\n", tmpnam, cobj->val->d.str ? cobj->val->d.str : "") < 0) goto err;
		}
		else {
			if (tcp_fprintf(&conn->socket, "%s: %s\r\n", tmpnam, nsp_tostr(conn->N, cobj)) < 0) goto err;
		}
		/* log_error(proc->N, MODSHORTNAME "responses", __FILE__, __LINE__, 1, "[%s][%s]", tmpnam, nsp_tostr(conn->N, cobj)); */
	}
	if (tcp_fprintf(&conn->socket, "Server: %s %s\r\n", SERVER_NAME, PACKAGE_VERSION) < 0) goto err;
	if (tcp_fprintf(&conn->socket, "Status: %d\r\n", conn->dat->out_status) < 0) goto err;
	if (conn->dat->out_bodydone) {
		if (tcp_fprintf(&conn->socket, "Content-Length: %d\r\n", conn->dat->out_ContentLength) < 0) goto err;
	}
	if (!ctype) {
		if (tcp_fprintf(&conn->socket, "Content-Type: text/plain\r\n") < 0) goto err;
	}
	if (tcp_fprintf(&conn->socket, "\r\n") < 0) goto err;
	conn->dat->out_headdone = 1;
	return;
err:
	tcp_close(&conn->socket, 1);
	conn->dat->out_headdone = 1;
	return;
}

void flushbuffer(CONN *conn)
{
	char *pTemp = conn->dat->replybuf;
	signed long str_len;
	signed long dcount;

	flushheader(conn);
	str_len = conn->dat->replybuflen;
	if (str_len < 1) return;
	conn->dat->out_flushed = 1;
	while (str_len > 0) {
		dcount = 4096;
		if (str_len < dcount) dcount = str_len;
		dcount = tcp_send(&conn->socket, pTemp, dcount, 0);
		if (dcount < 0) break;
		str_len -= dcount;
		pTemp += dcount;
	}
	memset(conn->dat->replybuf, 0, sizeof(conn->dat->replybuf));
	conn->dat->replybuflen = 0;
	return;
}

int prints(CONN *conn, const char *format, ...)
{
	char *buffer = getbuffer(conn);
	unsigned long str_len1, str_len2;
	va_list ap;

	if (conn == NULL) return -1;
	conn->socket.atime = time(NULL);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(conn->dat->smallbuf[0]) - 1, format, ap);
	str_len1 = strlen(buffer);
	str_len2 = conn->dat->replybuflen;
	va_end(ap);
	buffer[sizeof(conn->dat->smallbuf[0]) - 1] = '\0';
	conn->dat->out_bytecount += str_len1;
	if (str_len2 + sizeof(conn->dat->smallbuf[0]) > MAX_REPLYSIZE - 2) {
		flushbuffer(conn);
		str_len2 = 0;
	}
	strcpy(conn->dat->replybuf + str_len2, buffer);
	conn->dat->replybuflen = str_len2 += str_len1;
	if (str_len2 + sizeof(conn->dat->smallbuf[0]) > MAX_REPLYSIZE - 2) {
		flushbuffer(conn);
	}
	return str_len1;
}

int raw_prints(CONN *conn, const char *format, ...)
{
	char buffer[2048];
	va_list ap;

	if (conn == NULL) return -1;
	conn->socket.atime = time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);
	conn->dat->out_bytecount += strlen(buffer);
	flushbuffer(conn);
	return tcp_send(&conn->socket, buffer, strlen(buffer), 0);
}

#define MAX_FILESEND_WRITE_SIZE 64*1024

int filesend(CONN *conn, char *file)
{
	obj_t *htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	char *modtime = nsp_getstr(conn->N, htobj, "HTTP_IF_MODIFIED_SINCE");
	struct stat sb;
	int fp;
	int bytesleft;
	int blocksize;
	char timebuf[100];
	//char fileblock[8192];
	char *fileblock;
	int bs;

	decodeurl(file);
	fixslashes(file);
	if (strstr(file, "..") != NULL) return -1;
	if (stat(file, &sb) != 0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
	/* need some check for IfModifiedSince here */
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&sb.st_mtime));
	if (strcasecmp(timebuf, modtime) == 0) {
		conn->dat->out_bodydone = 1;
		send_header(conn, 1, 304, "1", get_mime_type(conn, file), sb.st_size, sb.st_mtime);
		flushheader(conn);
		//		goto fileend;
		//		conn->dat->out_headdone=1;
		//		conn->dat->out_flushed=1;
		conn->dat->replybuf[0] = '\0';
		//		flushbuffer(conn);
		return 0;
	}
	if ((fp = open(file, O_RDONLY | O_BINARY)) == -1) return -1;
	conn->dat->out_bytecount = sb.st_size;
	conn->dat->out_bodydone = 1;
	bytesleft = sb.st_size;
	send_header(conn, 1, 200, "1", get_mime_type(conn, file), sb.st_size, sb.st_mtime);
	flushheader(conn);
	/* sendfile() can't be used without screwing up our timeouts, so we do this */
	fileblock = n_alloc(conn->N, MAX_FILESEND_WRITE_SIZE, 1);
	for (;;) {
		if (bytesleft <= 0) break;
		blocksize = MAX_FILESEND_WRITE_SIZE;
		if (bytesleft < blocksize) bytesleft = blocksize;
		blocksize = read(fp, fileblock, blocksize);
		if (blocksize < 1) break;
		bytesleft -= blocksize;
		bs = tcp_send(&conn->socket, fileblock, blocksize, 0);
		if (bs < 0) break;
		if (bs > 0) conn->socket.atime = time(NULL);
	}
	n_free(conn->N, (void *)&fileblock, MAX_FILESEND_WRITE_SIZE);
	close(fp);
	//fileend:
	conn->dat->out_headdone = 1;
	conn->dat->out_flushed = 1;
	conn->dat->replybuf[0] = '\0';
	flushbuffer(conn);
	return 0;
}

int closeconnect(CONN *conn, int exitflag)
{
	if (exitflag < 2) {
		flushbuffer(conn);
	}
	if (conn != NULL) {
		if (conn->dat != NULL) {
			if (conn->dat->wm != NULL) {
				tcp_close(&conn->dat->wm->socket, 1);
			}
		}
		tcp_close(&conn->socket, 1);
		if (exitflag) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
			if (conn->dat != NULL) { free(conn->dat); conn->dat = NULL; }
			conn->N = nsp_endstate(conn->N);
#ifdef WIN32
			CloseHandle(conn->handle);
#endif
			memset(conn, 0, sizeof(htproc.conn[0]));
			conn->socket.socket = -1;
#ifdef WIN32
			_endthread();
#else
			pthread_exit(0);
#endif
		}
	}
	return 0;
}
