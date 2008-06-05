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
#include "mod_files.h"

int dbread_file(CONN *sid, short int perm, int index, obj_t **qobj)
{
/*
	int authlevel;
	SQLRES sqr;

	memset(file, 0, sizeof(REC_FILE));
	authlevel=auth_priv(sid, "files");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		file->obj_uid=sid->dat->uid;
		file->obj_gid=sid->dat->gid;
		file->obj_did=sid->dat->did;
		file->obj_gperm=1;
		file->obj_operm=1;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_files where fileid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_files where fileid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->uid, sid->dat->gid, perm, perm, sid->dat->did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	file->fileid     = atoi(sql_getvalue(&sqr, 0, 0));
	file->obj_ctime  = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	file->obj_mtime  = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	file->obj_uid    = atoi(sql_getvalue(&sqr, 0, 3));
	file->obj_gid    = atoi(sql_getvalue(&sqr, 0, 4));
	file->obj_did    = atoi(sql_getvalue(&sqr, 0, 5));
	file->obj_gperm  = atoi(sql_getvalue(&sqr, 0, 6));
	file->obj_operm  = atoi(sql_getvalue(&sqr, 0, 7));
	strncpy(file->filepath,		sql_getvalue(&sqr, 0, 8), sizeof(file->filepath)-1);
	strncpy(file->filename,		sql_getvalue(&sqr, 0, 9), sizeof(file->filename)-1);
	strncpy(file->filetype,		sql_getvalue(&sqr, 0, 10), sizeof(file->filetype)-1);
	file->uldate=time_sql2unix(sql_getvalue(&sqr, 0, 11));
	file->lastdldate=time_sql2unix(sql_getvalue(&sqr, 0, 12));
	file->numdownloads=atoi(sql_getvalue(&sqr, 0, 13));
	strncpy(file->description,		sql_getvalue(&sqr, 0, 14), sizeof(file->description)-1);
	sql_freeresult(&sqr);
*/
	return 0;
}

int fileperm(CONN *sid, int perm, char *dir, char *file)
{
/*
	struct stat sb;
	char filename[512];
	char subdir[512];
	char subfile[512];
	char *ptemp;
	SQLRES sqr;
	int tuples;
	int x;

	DEBUG_IN(sid, "fileperm()");
//	logerror(sid, __FILE__, __LINE__, 1, "dir[%s]file[%s]", dir, file);
	if (strncmp(dir, "/files/", 7)!=0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	memset(filename, 0, sizeof(filename));
	ptemp=dir;
	if (strlen(ptemp)>=7) ptemp+=7;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->did, ptemp, file);
	fixslashes(filename);
//	logerror(sid, __FILE__, __LINE__, 1, "[%s]", filename);
	x=stat(filename, &sb);
	if ((perm==A_READ)&&(x!=0)) { DEBUG_OUT(sid, "fileperm()"); return -2; }
	if ((perm==A_INSERT)&&(x==0)) { DEBUG_OUT(sid, "fileperm()"); return -3; }
//	if (sb.st_mode&S_IFDIR)
	/ * if A_ADMIN is true and the file exists, then permission is granted * /
	if (auth_priv(sid, "files")&A_ADMIN) { DEBUG_OUT(sid, "fileperm()"); return 0; }
	if ((perm==A_READ)&&(!auth_priv(sid, "files")&A_READ)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_INSERT)&&(!auth_priv(sid, "files")&A_INSERT)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_MODIFY)&&(!auth_priv(sid, "files")&A_MODIFY)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_DELETE)&&(!auth_priv(sid, "files")&A_DELETE)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((strcmp(dir, "/files/")==0)&&((perm==A_INSERT)||(perm==A_DELETE))) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if (perm==A_READ) {
		if (sql_queryf(&sqr, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=1) OR obj_operm>=1) AND obj_did = %d", file, dir, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(&sqr);
		sql_freeresult(&sqr);
	} else if (perm==A_MODIFY) {
		if (sql_queryf(&sqr, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=2) OR obj_operm>=2) AND obj_did = %d", file, dir, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(&sqr);
		sql_freeresult(&sqr);
	} else if ((perm==A_INSERT)||(perm==A_DELETE)) {
		memset(subdir, 0, sizeof(subdir));
		memset(subfile, 0, sizeof(subfile));
		snprintf(subdir, sizeof(subdir)-1, "%s", dir);
		ptemp=subdir;
 		while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
		while (subdir[strlen(subdir)-1]=='/') subdir[strlen(subdir)-1]='\0';
		if ((ptemp=strrchr(subdir, '/'))!=NULL) {
			ptemp++;
			snprintf(subfile, sizeof(subfile)-1, "%s", ptemp);
			*ptemp='\0';
		}
		if (sql_queryf(&sqr, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=2) OR obj_operm>=2) AND obj_did = %d", subfile, subdir, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(&sqr);
		sql_freeresult(&sqr);
	} else {
		tuples=0;
	}
	DEBUG_OUT(sid, "fileperm()");
	if (tuples!=1) return -1;
*/
	return 0;
}
