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
#include "ghttpd/mod.h"
#include "mod_files.h"

int dbread_file(CONN *sid, short int perm, int index, REC_FILE *file)
{
	int authlevel;
	int sqr;

	memset(file, 0, sizeof(REC_FILE));
	authlevel=auth_priv(sid, "files");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		file->obj_uid=sid->dat->user_uid;
		file->obj_gid=sid->dat->user_gid;
		file->obj_did=sid->dat->user_did;
		file->obj_gperm=1;
		file->obj_operm=1;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_files where fileid = %d AND obj_did = %d", index, sid->dat->user_did))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_files where fileid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	file->fileid     = atoi(sql_getvalue(sqr, 0, 0));
	file->obj_ctime  = time_sql2unix(sql_getvalue(sqr, 0, 1));
	file->obj_mtime  = time_sql2unix(sql_getvalue(sqr, 0, 2));
	file->obj_uid    = atoi(sql_getvalue(sqr, 0, 3));
	file->obj_gid    = atoi(sql_getvalue(sqr, 0, 4));
	file->obj_did    = atoi(sql_getvalue(sqr, 0, 5));
	file->obj_gperm  = atoi(sql_getvalue(sqr, 0, 6));
	file->obj_operm  = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(file->filepath,		sql_getvalue(sqr, 0, 8), sizeof(file->filepath)-1);
	strncpy(file->filename,		sql_getvalue(sqr, 0, 9), sizeof(file->filename)-1);
	strncpy(file->filetype,		sql_getvalue(sqr, 0, 10), sizeof(file->filetype)-1);
	file->uldate=time_sql2unix(sql_getvalue(sqr, 0, 11));
	file->lastdldate=time_sql2unix(sql_getvalue(sqr, 0, 12));
	file->numdownloads=atoi(sql_getvalue(sqr, 0, 13));
	strncpy(file->description,		sql_getvalue(sqr, 0, 14), sizeof(file->description)-1);
	sql_freeresult(sqr);
	return 0;
}
