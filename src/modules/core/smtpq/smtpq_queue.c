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
#include "smtpq_main.h"
#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#endif

#ifdef WIN32
unsigned _stdcall smtp_spool(void *x)
#else
void *smtp_spool(void *x)
#endif
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
#ifdef WIN32
	struct  _finddata_t dentry;
	long handle;
#else
	struct dirent *dentry;
	DIR *handle;
#endif
	struct stat sb;
	char *ptemp1;
	FILE *fp = NULL;
	char dirname1[256];
	char dirname2[256];
	char filename1[256];
	char filename2[256];
	char inbuffer[1024];
	char from[128];
	char rcpt[128];
	short int status;
	time_t t = time(NULL);

	log_error(proc->N, "core", __FILE__, __LINE__, 2, "Starting smtp_spool() thread");
	memset(dirname1, 0, sizeof(dirname1));
	memset(dirname2, 0, sizeof(dirname2));
	memset(from, 0, sizeof(from));
	memset(rcpt, 0, sizeof(rcpt));
	snprintf(dirname1, sizeof(dirname1) - 1, "%s/spool/mqinfo", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"));
	snprintf(dirname2, sizeof(dirname2) - 1, "%s/spool/mqueue", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"));
	fixslashes(dirname1);
	fixslashes(dirname2);
	while (1) {
		//		handle=opendir(dirname1);
		//		while ((dentry=readdir(handle))!=NULL) {
#ifdef WIN32
		if ((handle = _findfirst(dirname1, &dentry)) < 0) {
			sleep(mod_config.smtp_retrydelay);
			continue;
			//return;
		}
		do {
			char *name = dentry.name;
#else
		handle = opendir(dirname1);
		while ((dentry = readdir(handle)) != NULL) {
			char *name = dentry->d_name;
#endif
			if (strcmp(".", name) == 0) continue;
			if (strcmp("..", name) == 0) continue;
			memset(filename1, 0, sizeof(filename1));
			memset(filename2, 0, sizeof(filename2));
			snprintf(filename1, sizeof(filename1) - 1, "%s/%s", dirname1, name);
			snprintf(filename2, sizeof(filename2) - 1, "%s/%s", dirname2, name);
			fixslashes(filename1);
			fixslashes(filename2);
			if ((ptemp1 = strrchr(filename2, '.')) == NULL) continue;
			*ptemp1 = '\0';
			strcat(filename2, ".msg");
			if (stat(filename1, &sb) != 0) continue;
			if (sb.st_mode&S_IFDIR) continue;
			if ((fp = fopen(filename1, "rb")) == NULL) continue;
			if (fgets(inbuffer, sizeof(inbuffer) - 1, fp) != NULL) {
				if (strncasecmp(inbuffer, "MAIL:", 5) == 0) {
					striprn(inbuffer);
					ptemp1 = inbuffer + 5;
					while ((*ptemp1 == ' ') || (*ptemp1 == '\t')) ptemp1++;
					snprintf(from, sizeof(from) - 1, "%s", ptemp1);
				}
			}
			memset(rcpt, 0, sizeof(rcpt));
			if (fgets(inbuffer, sizeof(inbuffer) - 1, fp) != NULL) {
				if (strncasecmp(inbuffer, "RCPT:", 5) == 0) {
					striprn(inbuffer);
					ptemp1 = inbuffer + 5;
					while ((*ptemp1 == ' ') || (*ptemp1 == '\t')) ptemp1++;
					snprintf(rcpt, sizeof(rcpt) - 1, "%s", ptemp1);
				}
			}
			if (fgets(inbuffer, sizeof(inbuffer) - 1, fp) != NULL) {
				if (strncasecmp(inbuffer, "DATE:", 5) == 0) {
					striprn(inbuffer);
					ptemp1 = inbuffer + 5;
					while ((*ptemp1 == ' ') || (*ptemp1 == '\t')) ptemp1++;
					t -= time_sql2unix(ptemp1);
				}
			}
			fclose(fp);
			if ((fp = fopen(filename2, "rb")) == NULL) continue;
			status = smtp_client(fp, filename2, from, rcpt);
			fclose(fp);
			if ((status == 0) || (status == -2)) {
				unlink(filename1);
				unlink(filename2);
			}
			//		}
			//		closedir(handle);
#ifdef WIN32
		} while (_findnext(handle, &dentry) == 0);
		_findclose(handle);
#else
		}
		closedir(handle);
#endif

		sleep(mod_config.smtp_retrydelay);
	}
	return 0;
}
