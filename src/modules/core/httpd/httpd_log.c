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

int db_log_activity(CONN *sid, char *category, int indexid, char *action, const char *format, ...)
{
	char curdate[32];
	char details[2048];
	char query[2048];
	va_list ap;

return 0;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	memset(details, 0, sizeof(details));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	va_start(ap, format);
	vsnprintf(details, sizeof(details)-1, format, ap);
	va_end(ap);
	strcpy(query, "INSERT INTO gw_activity (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, userid, clientip, category, indexid, action, details) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '0', '0', '%d', '0', '0', ", curdate, curdate, sid->dat->did);
	if (sid!=NULL) {
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", sid->dat->uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", sid->socket.RemoteAddr);
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, category));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", indexid);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, action));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, details));
	return sql_update(proc->N, query);
}

void log_htaccess(CONN *sid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestMethod=nes_getstr(sid->N, htobj, "REQUEST_METHOD");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	char *Protocol=nes_getstr(sid->N, htobj, "SERVER_PROTOCOL");
#ifndef WIN32
	static pthread_mutex_t *LogLock=NULL;
#endif
	char logbuffer[512];
	char timebuffer[50];
	char file[200];
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

#ifndef WIN32
	if (!LogLock) { LogLock=calloc(1, sizeof(pthread_mutex_t)); pthread_mutex_init(LogLock, NULL); }
#endif
	if (sid->dat==NULL) return;
	snprintf(file, sizeof(file)-1, "%s/" MODSHORTNAME "-access.log", nes_getstr(proc->N, confobj, "var_log_path"));
	fixslashes(file);
	memset(logbuffer, 0, sizeof(logbuffer));
	memset(timebuffer, 0, sizeof(timebuffer));
	gettimeofday(&ttime, &tzone);
	strftime(timebuffer, sizeof(timebuffer)-1, "%d/%b/%Y:%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
/*
	snprintf(logbuffer, sizeof(logbuffer)-1, "%s - %s [%s] \"%s %s %s\" %d %d \"-\" \"%s\""
		, sid->socket.RemoteAddr, strlen(sid->dat->username)?sid->dat->username:"-"
		, timebuffer, sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol
		, sid->dat->out_status, sid->dat->out_bytecount, sid->dat->in_UserAgent
	);
*/
	snprintf(logbuffer, sizeof(logbuffer)-1, "%s - %s [%s] \"%s %s %s\" %d %d"
		, sid->socket.RemoteAddr, strlen(sid->dat->username)?sid->dat->username:"-"
		, timebuffer, RequestMethod, RequestURI, Protocol
		, sid->dat->out_status, sid->dat->out_bytecount
	);
	/*
	 * This crashes frequently and violently on openbsd without the mutex.
	 * Can someone please tell me why?
	 */
#ifndef WIN32
	pthread_mutex_lock(LogLock);
#endif
	if ((fp=fopen(file, "a"))==NULL) {
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
