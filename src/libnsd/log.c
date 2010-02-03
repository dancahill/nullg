/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include "libnsd.h"

void log_access(nsp_state *N, char *logsrc, const char *format, ...)
{
	obj_t *tobj=nsp_getobj(N, &N->g, "CONFIG");
	char *logpath=nsp_getstr(N, tobj, "var_log_path");
	char file[200];
	char logbuf[1024];
	char logbuf2[2048];
	char timebuf[100];
	va_list ap;
	int fd;
	struct timeval ttime;
	struct timezone tzone;

	if (tobj->val->type!=NT_TABLE) return;
	snprintf(file, sizeof(file)-1, "%s/%s-access.log", logpath, logsrc);
	fixslashes(file);
	va_start(ap, format);
	vsnprintf(logbuf, sizeof(logbuf)-1, format, ap);
	va_end(ap);
	striprn(logbuf);
	gettimeofday(&ttime, &tzone);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	snprintf(logbuf2, sizeof(logbuf2)-1, "%s - %s\n", timebuf, logbuf);
	fd=open(file, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
	if (fd>-1) {
		write(fd, logbuf2, strlen(logbuf2));
		close(fd);
	}
	return;
}

void log_error(nsp_state *N, char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...)
{
	obj_t *tobj=nsp_getobj(N, &N->g, "CONFIG");
	char *logpath;
	int conflog;
	char file[512];
	char logbuf[1024];
	char logbuf2[2048];
	char timebuf[100];
	va_list ap;
	int fd;
	struct timeval ttime;
	struct timezone tzone;
	char *ptemp;

	if (tobj->val->type!=NT_TABLE) return;
	logpath=nsp_getstr(N, tobj, "var_log_path");
	conflog=(int)nsp_getnum(N, tobj, "log_level");
	if ((loglevel>conflog)||(conflog<1)) return;
	va_start(ap, format);
	vsnprintf(logbuf, sizeof(logbuf)-1, format, ap);
	va_end(ap);
	striprn(logbuf);
	if (strlen(logpath)==0) {
		printf("[%d] %s\n", loglevel, logbuf);
		return;
	}
	snprintf(file, sizeof(file)-1, "%s/%s-error.log", logpath, logsrc);
	fixslashes(file);
	gettimeofday(&ttime, &tzone);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	if ((ptemp=strrchr(srcfile, '/'))!=NULL) srcfile=ptemp+1;
	if ((ptemp=strrchr(srcfile, '\\'))!=NULL) srcfile=ptemp+1;
	if (conflog>1) {
		snprintf(logbuf2, sizeof(logbuf2)-1, "%s - [%d] %s %d %s\n", timebuf, loglevel, srcfile, line, logbuf);
	} else {
		snprintf(logbuf2, sizeof(logbuf2)-1, "%s - [%d] %s\n", timebuf, loglevel, logbuf);
	}
	fd=open(file, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
	if (fd>-1) {
		write(fd, logbuf2, strlen(logbuf2));
		close(fd);
	}
	return;
}
