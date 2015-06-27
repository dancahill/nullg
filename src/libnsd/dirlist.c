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

int dir_list(nsp_state *N, obj_t **dobj, const char *dirname)
{
#ifdef WIN32
	struct  _finddata_t dentry;
	long    handle;
#else
	struct dirent *dentry;
	DIR    *handle;
#endif
	struct stat sb;
	char   dir[512];
	char   file[512];
	char   *name;
	obj_t  *tobj2;
	int    sym;
	int len;

	if (*dobj==NULL) {
		tobj2=*dobj=calloc(1, sizeof(obj_t));
		nsp_linkval(N, tobj2, NULL);
		tobj2->val->type=NT_TABLE;
		tobj2->val->attr|=NST_AUTOSORT;
	}
	snprintf(dir, sizeof(dir)-1, dirname);
	fixslashes(dir);
	len=strlen(dir);
	if ((len>0)&&(dir[len-1]=='/')) dir[len-1]='\0';
	if (stat(dir, &sb)!=0) return -1;
	if (!(sb.st_mode&S_IFDIR)) return 0;
#ifdef WIN32
	snprintf(file, sizeof(file)-1, "%s/*.*", dir);
	if ((handle=_findfirst(file, &dentry))<0) return 0;
	do {
		name=dentry.name;
		snprintf(file, sizeof(file)-1, "%s/%s", dir, name);
		sym=0;
		if (stat(file, &sb)!=0) continue;
#else
	handle=opendir(dir);
	while ((dentry=readdir(handle))!=NULL) {
		name=dentry->d_name;
		snprintf(file, sizeof(file)-1, "%s/%s", dir, name);
		sym=0;
		if (lstat(file, &sb)!=0) continue;
		if (!(~sb.st_mode&S_IFLNK)) {
			sym=1;
			if (stat(file, &sb)!=0) sym=2;
		}
#endif
		tobj2=nsp_settable(N, *dobj, name);
		nsp_setstr(N, tobj2, "name", name, -1);
		nsp_setnum(N, tobj2, "mtime", sb.st_mtime);
		if (sym==2) {
			nsp_setnum(N, tobj2, "size", 0);
			nsp_setstr(N, tobj2, "type", "broken", 6);
		} else if ((sb.st_mode&S_IFDIR)) {
			nsp_setnum(N, tobj2, "size", 0);
			nsp_setstr(N, tobj2, "type", sym?"dirp":"dir", strlen(sym?"dirp":"dir"));
		} else {
			nsp_setnum(N, tobj2, "size", sb.st_size);
			nsp_setstr(N, tobj2, "type", sym?"filep":"file", strlen(sym?"filep":"file"));
		}
#ifdef WIN32
	} while (_findnext(handle, &dentry)==0);
	_findclose(handle);
#else
	}
	closedir(handle);
#endif
	return 0;
}
