/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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

int db_log_activity(CONN *conn, char *category, int indexid, char *action, const char *format, ...)
{
	char curdate[32];
	char details[2048];
	char query[2048];
	va_list ap;

	return 0;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	memset(details, 0, sizeof(details));
	time_unix2sql(curdate, sizeof(curdate) - 1, time(NULL));
	va_start(ap, format);
	vsnprintf(details, sizeof(details) - 1, format, ap);
	va_end(ap);
	strcpy(query, "INSERT INTO gw_activity (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, userid, clientip, category, indexid, action, details) values (");
	strncatf(query, sizeof(query) - strlen(query) - 1, "'%s', '%s', '0', '0', '%d', '0', '0', ", curdate, curdate, conn->dat->did);
	if (conn != NULL) {
		strncatf(query, sizeof(query) - strlen(query) - 1, "'%d', ", conn->dat->uid);
		strncatf(query, sizeof(query) - strlen(query) - 1, "'%s', ", conn->socket.RemoteAddr);
	}
	else {
		strncatf(query, sizeof(query) - strlen(query) - 1, "'', ");
		strncatf(query, sizeof(query) - strlen(query) - 1, "'', ");
	}
	strncatf(query, sizeof(query) - strlen(query) - 1, "'%s', ", str2sql(getbuffer(conn), sizeof(conn->dat->smallbuf[0]) - 1, category));
	strncatf(query, sizeof(query) - strlen(query) - 1, "'%d', ", indexid);
	strncatf(query, sizeof(query) - strlen(query) - 1, "'%s', ", str2sql(getbuffer(conn), sizeof(conn->dat->smallbuf[0]) - 1, action));
	strncatf(query, sizeof(query) - strlen(query) - 1, "'%s')", str2sql(getbuffer(conn), sizeof(conn->dat->smallbuf[0]) - 1, details));
	return sql_update(proc->N, NULL, query);
}

void log_htaccess(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	char *RequestMethod = nsp_getstr(conn->N, htobj, "REQUEST_METHOD");
	char *RequestURI = nsp_getstr(conn->N, htobj, "REQUEST_URI");
	char *Protocol = nsp_getstr(conn->N, htobj, "SERVER_PROTOCOL");
#ifndef WIN32
	static pthread_mutex_t *LogLock = NULL;
#endif
	char logbuffer[512];
	char timebuffer[50];
	char timebuf2[20];
	char file[200];
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

#ifndef WIN32
	if (!LogLock) { LogLock = calloc(1, sizeof(pthread_mutex_t)); pthread_mutex_init(LogLock, NULL); }
#endif
	if (conn->dat == NULL) return;
	memset(logbuffer, 0, sizeof(logbuffer));
	memset(timebuffer, 0, sizeof(timebuffer));
	memset(timebuf2, 0, sizeof(timebuf2));
	gettimeofday(&ttime, &tzone);
	strftime(timebuffer, sizeof(timebuffer) - 1, "%d/%b/%Y:%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	strftime(timebuf2, sizeof(timebuf2) - 1, "%Y%m%d", localtime((time_t *)&ttime.tv_sec));
	/*
	snprintf(logbuffer, sizeof(logbuffer)-1, "%s - %s [%s] \"%s %s %s\" %d %d \"-\" \"%s\""
		, conn->socket.RemoteAddr, strlen(conn->dat->username)?conn->dat->username:"-"
		, timebuffer, conn->dat->in_RequestMethod, conn->dat->in_RequestURI, conn->dat->in_Protocol
		, conn->dat->out_status, conn->dat->out_bytecount, conn->dat->in_UserAgent
	);
*/
	snprintf(logbuffer, sizeof(logbuffer) - 1, "%s - %s [%s] \"%s %s %s\" %d %d"
		, conn->socket.RemoteAddr, strlen(conn->dat->username) ? conn->dat->username : "-"
		, timebuffer, RequestMethod, RequestURI, Protocol
		, conn->dat->out_status, conn->dat->out_bytecount
	);
	/*
	 * This crashes frequently and violently on openbsd without the mutex.
	 * Can someone please tell me why?
	 */
#ifndef WIN32
	pthread_mutex_lock(LogLock);
#endif
	snprintf(file, sizeof(file) - 1, "%s/log/" MODSHORTNAME "-access-%s.log", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), timebuf2);
	fixslashes(file);
	if ((fp = fopen(file, "a")) == NULL) {
#ifndef WIN32
		pthread_mutex_unlock(LogLock);
#endif
		return;
	}
	fprintf(fp, "%s\n", logbuffer);
	fclose(fp);
#ifndef WIN32
	pthread_mutex_unlock(LogLock);
#endif
	return;
}
