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
#include "libnsd.h"

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

void log_access(nsp_state *N, char *logsrc, const char *format, ...)
{
	obj_t *tobj=nsp_getobj(N, &N->g, "CONFIG");
	obj_t* varlog;
	char *logpath;
	int conflog;
	char file[200];
	char logbuf[1024];
	char logbuf2[2048];
	char timebuf[100];
	char timebuf2[20];
	va_list ap;
	int fd;
	struct timeval ttime;
	struct timezone tzone;
	struct tm *timeptr;

	if (!nsp_istable(tobj)) return;
	varlog = nsp_getobj(N, nsp_getobj(N, tobj, "paths"), "var_log");
	logpath = nsp_isstr(varlog) ? nsp_tostr(N, varlog) : ".";
	conflog=(int)nsp_getnum(N, tobj, "log_level");
	va_start(ap, format);
	vsnprintf(logbuf, sizeof(logbuf)-1, format, ap);
	va_end(ap);
	striprn(logbuf);
	gettimeofday(&ttime, &tzone);
	timeptr=localtime((time_t *)&ttime.tv_sec);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", timeptr);
	strftime(timebuf2, sizeof(timebuf2), "%Y%m%d", timeptr);
	snprintf(logbuf2, sizeof(logbuf2)-1, "%s - %s\n", timebuf, logbuf);

	if (conflog>1) {
		snprintf(file, sizeof(file)-1, "%s/all-%s.log", logpath, timebuf2);
		fixslashes(file);
		fd=open(file, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
		if (fd>-1) {
			write(fd, "access - ", strlen("access - "));
			write(fd, logbuf2, strlen(logbuf2));
			close(fd);
		}
	}

	snprintf(file, sizeof(file)-1, "%s/%s-access-%s.log", logpath, logsrc, timebuf2);
	fixslashes(file);
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
	obj_t *varlog;
	char *logpath;
	int conflog;
	char file[512];
	char logbuf[1024];
	char logbuf2[2048];
	char timebuf[100];
	char timebuf2[20];
	va_list ap;
	int fd;
	struct timeval ttime;
	struct timezone tzone;
	struct tm *timeptr;
	char *ptemp;

	if (!nsp_istable(tobj)) return;
	varlog = nsp_getobj(N, nsp_getobj(N, tobj, "paths"), "var_log");
	logpath = nsp_isstr(varlog) ? nsp_tostr(N, varlog) : ".";
	conflog = (int)nsp_getnum(N, tobj, "log_level");
	//if ((loglevel>conflog)||(conflog<1)) return;
	va_start(ap, format);
	vsnprintf(logbuf, sizeof(logbuf)-1, format, ap);
	va_end(ap);
	striprn(logbuf);
	if (strlen(logpath)==0) {
		printf("[%d] %s\n", loglevel, logbuf);
		return;
	}
	gettimeofday(&ttime, &tzone);
	timeptr=localtime((time_t *)&(ttime.tv_sec));
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", timeptr);
	strftime(timebuf2, sizeof(timebuf2), "%Y%m%d", timeptr);
	if ((ptemp=strrchr(srcfile, '/'))!=NULL) srcfile=ptemp+1;
	if ((ptemp=strrchr(srcfile, '\\'))!=NULL) srcfile=ptemp+1;
	if (conflog>1) {
		snprintf(logbuf2, sizeof(logbuf2)-1, "%s - [%d] %s %d %s\n", timebuf, loglevel, srcfile, line, logbuf);
	} else {
		snprintf(logbuf2, sizeof(logbuf2)-1, "%s - [%d] %s\n", timebuf, loglevel, logbuf);
	}
	if (conflog>1) {
		snprintf(file, sizeof(file)-1, "%s/all-%s.log", logpath, timebuf2);
		fixslashes(file);
		fd=open(file, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
		if (fd>-1) {
			write(fd, "error  - ", strlen("error  - "));
			write(fd, logbuf2, strlen(logbuf2));
			close(fd);
		}
	}
	if ((loglevel>conflog)||(conflog<1)) return;
	snprintf(file, sizeof(file)-1, "%s/%s-error-%s.log", logpath, logsrc, timebuf2);
	fixslashes(file);
	if (nsp_isnull(varlog)) {
		if (loglevel <= 1) write(STDOUT_FILENO, logbuf2, strlen(logbuf2));
	} else {
		fd = open(file, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
		if (fd > -1) {
			write(fd, logbuf2, strlen(logbuf2));
			close(fd);
		}
	}
	return;
}
