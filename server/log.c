/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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

void log_access(char *logsrc, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/%s-access.log", proc.config.dir_var_log, logsrc);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		fprintf(fp, "%s - %s\n", timebuffer, logbuffer);
		fclose(fp);
	}
}

void log_error(char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;
	char *ptemp;

	if ((loglevel>proc.config.loglevel)||(proc.config.loglevel<1)) return;
	snprintf(file, sizeof(file)-1, "%s/%s-error.log", proc.config.dir_var_log, logsrc);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		striprn(logbuffer);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if ((ptemp=strrchr(srcfile, '/'))!=NULL) srcfile=ptemp+1;
		if ((ptemp=strrchr(srcfile, '\\'))!=NULL) srcfile=ptemp+1;
		if (proc.config.loglevel>1) {
			fprintf(fp, "%s - [%d] %s %d %s\n", timebuffer, loglevel, srcfile, line, logbuffer);
		} else {
			fprintf(fp, "%s - [%d] %s\n", timebuffer, loglevel, logbuffer);
		}
		fclose(fp);
	}
}
