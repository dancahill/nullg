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

#ifdef WIN32
unsigned _stdcall smtp_spool(void *x)
#else
void *smtp_spool(void *x)
#endif
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	struct stat sb;
	DIR  *handle;
	FILE *fp=NULL;
	char dirname[256];
	char tmpname[256];
	short int status;

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/mqueue", config->dir_var_spool);
	fixslashes(dirname);
	while (1) {
		handle=opendir(dirname);
		while ((dentry=readdir(handle))!=NULL) {
			if (strcmp(".", dentry->d_name)==0) continue;
			if (strcmp("..", dentry->d_name)==0) continue;
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/%s", dirname, dentry->d_name);
			fixslashes(tmpname);
			if (stat(tmpname, &sb)!=0) continue;
			if (sb.st_mode&S_IFDIR) continue;
			if ((fp=fopen(tmpname, "rb"))!=NULL) {
				status=smtp_client(fp);
				fclose(fp);
				if (status==0) {
					unlink(tmpname);
				}
			}
		}
		closedir(handle);
		sleep(mod_config.smtp_retrydelay);
	}
	return 0;
}
