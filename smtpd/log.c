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
#include "smtp_main.h"

void logaccess(CONN *sid, int loglevel, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	if ((loglevel>proc.config.server_loglevel)||(proc.config.server_loglevel<1)) return;
	snprintf(file, sizeof(file)-1, "%s/smtp-access.log", proc.config.server_dir_var_log);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if ((proc.config.server_loglevel>=4)&&(sid!=NULL)) {
			fprintf(fp, "%s - [%d][0x%08X] %s\n", timebuffer, loglevel, (unsigned int)sid->id, logbuffer);
		} else {
			fprintf(fp, "%s - [%d] %s\n", timebuffer, loglevel, logbuffer);
		}
		fclose(fp);
	}
}

void logerror(CONN *sid, char *srcfile, int line, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/smtp-error.log", proc.config.server_dir_var_log);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if ((proc.config.server_loglevel>=4)&&(sid!=NULL)) {
			fprintf(fp, "%s - %s %d [0x%08X] %s\n", timebuffer, srcfile, line, (unsigned int)sid->id, logbuffer);
		} else {
			fprintf(fp, "%s - %s %d %s\n", timebuffer, srcfile, line, logbuffer);
		}
		fclose(fp);
	}
}

void logdebug(CONN *sid, char *srcfile, int line, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/smtp-debug.log", proc.config.server_dir_var_log);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
		if (sid!=NULL) {
			fprintf(fp, "%s - %s %d [0x%08X] %s\n", timebuffer, srcfile, line, (unsigned int)sid->id, logbuffer);
		} else {
			fprintf(fp, "%s - %s %d %s\n", timebuffer, srcfile, line, logbuffer);
		}
		fclose(fp);
	}
}
