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

int db_log_activity(CONN *sid, int loglevel, char *category, int indexid, char *action, const char *format, ...)
{
	char curdate[32];
	char details[2048];
	char query[2048];
	va_list ap;

	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	memset(details, 0, sizeof(details));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	va_start(ap, format);
	vsnprintf(details, sizeof(details)-1, format, ap);
	va_end(ap);
	strcpy(query, "INSERT INTO gw_activity (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, userid, clientip, category, indexid, action, details) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '0', '0', '0', '0', ", curdate, curdate);
	if (sid!=NULL) {
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", sid->dat->in_RemoteAddr);
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, category));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", indexid);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, action));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, details));
	return sql_update(sid, query);
}

void logaccess(CONN *sid, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/http-access.log", proc.config.server_dir_var_log);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if ((proc.config.server_loglevel>=4)&&(sid!=NULL)) {
			fprintf(fp, "%s - [0x%08X] %s\n", timebuffer, (unsigned int)sid->id, logbuffer);
		} else {
			fprintf(fp, "%s - %s\n", timebuffer, logbuffer);
		}
		fclose(fp);
	}
}

void logerror(CONN *sid, char *srcfile, int line, int loglevel, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	if ((loglevel>proc.config.server_loglevel)||(proc.config.server_loglevel<1)) return;
	snprintf(file, sizeof(file)-1, "%s/http-error.log", proc.config.server_dir_var_log);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if ((proc.config.server_loglevel>=4)&&(sid!=NULL)) {
			fprintf(fp, "%s - [%d] %s %d [0x%08X] %s\n", timebuffer, loglevel, srcfile, line, (unsigned int)sid->id, logbuffer);
		} else if ((proc.config.server_loglevel>=2)&&(sid!=NULL)) {
			fprintf(fp, "%s - [%d] %s %d %s\n", timebuffer, loglevel, srcfile, line, logbuffer);
		} else {
			fprintf(fp, "%s - [%d] %s\n", timebuffer, loglevel, logbuffer);
		}
		fclose(fp);
	}
}
