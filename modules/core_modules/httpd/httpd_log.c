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
#include "httpd_main.h"

int db_log_activity(CONN *sid, char *category, int indexid, char *action, const char *format, ...)
{
	char curdate[32];
	char details[2048];
	char query[2048];
	va_list ap;

	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	memset(details, 0, sizeof(details));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	va_start(ap, format);
	vsnprintf(details, sizeof(details)-1, format, ap);
	va_end(ap);
	strcpy(query, "INSERT INTO gw_activity (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, userid, clientip, category, indexid, action, details) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '0', '0', '%d', '0', '0', ", curdate, curdate, sid->dat->user_did);
	if (sid!=NULL) {
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", sid->dat->in_RemoteAddr);
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, category));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", indexid);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, action));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, details));
	return sql_update(query);
}

void log_htaccess(CONN *sid)
{
	char logbuffer[2048];
	char timebuffer[50];
	char file[200];
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/httpd-access.log", config->dir_var_log);
	fixslashes(file);
	memset(logbuffer, 0, sizeof(logbuffer));
	memset(timebuffer, 0, sizeof(timebuffer));
	gettimeofday(&ttime, &tzone);
	strftime(timebuffer, sizeof(timebuffer)-1, "%d/%b/%Y:%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
/*
	LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %T %v" full
	LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %P %T" debug
	LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined
	LogFormat "%h %l %u %t \"%r\" %>s %b" common
	LogFormat "%{Referer}i -> %U" referer
	LogFormat "%{User-agent}i" agent
*/
/*
	snprintf(logbuffer, sizeof(logbuffer)-1, "%s - %s [%s] \"%s %s %s\" %d %d \"-\" \"%s\"", sid->dat->in_RemoteAddr, sid->dat->user_username, timebuffer, sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol, sid->dat->out_status, sid->dat->out_bytecount, sid->dat->in_UserAgent);
*/
	snprintf(logbuffer, sizeof(logbuffer)-1, "%s", sid->dat->in_RemoteAddr);
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " -");
	if (strlen(sid->dat->user_username)==0) {
		strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " -");
	} else {
		strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " %s", sid->dat->user_username);
	}
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " [%s]", timebuffer);
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " \"%s %s %s\"", sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol);
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " %d %d", sid->dat->out_status, sid->dat->out_bytecount);
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " \"-\"");
	strncatf(logbuffer, sizeof(logbuffer)-strlen(logbuffer)-1, " \"%s\"", sid->dat->in_UserAgent);
	if ((fp=fopen(file, "a"))==NULL) return;
	fprintf(fp, "%s\n", logbuffer);
	fclose(fp);
	return;
}
