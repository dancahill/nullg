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
#include "libngs.h"

void log_access(nes_state *N, char *logsrc, const char *format, ...)
{
	obj_t *tobj=nes_getobj(N, &N->g, "CONFIG");
	char *logpath=nes_getstr(N, tobj, "var_log_path");
	char file[200];
	char logbuf[1024];
	char timebuf[100];
	va_list ap;
	FILE *fp;
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
	if ((fp=fopen(file, "a"))==NULL) return;
	fprintf(fp, "%s - %s\n", timebuf, logbuf);
	fclose(fp);
	return;
}

void log_error(nes_state *N, char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...)
{
	obj_t *tobj=nes_getobj(N, &N->g, "CONFIG");
	char *logpath;
	int conflog;
	char file[512];
	char logbuf[1024];
	char timebuf[100];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;
	char *ptemp;

	if (tobj->val->type!=NT_TABLE) return;
	logpath=nes_getstr(N, tobj, "var_log_path");
	conflog=(int)nes_getnum(N, tobj, "log_level");
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
	if ((fp=fopen(file, "a"))==NULL) return;
	if (conflog>1) {
		setvbuf(fp, NULL, _IONBF, 0);
		fprintf(fp, "%s - [%d] %s %d %s\n", timebuf, loglevel, srcfile, line, logbuf);
	} else {
		fprintf(fp, "%s - [%d] %s\n", timebuf, loglevel, logbuf);
	}
	fclose(fp);
	return;
}
