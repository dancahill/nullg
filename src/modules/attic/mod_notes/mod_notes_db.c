/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "mod_notes.h"

int dbread_note(CONN *sid, short int perm, int index, obj_t **qobj)
{
/*
	int authlevel;
	SQLRES sqr;

	memset(note, 0, sizeof(REC_NOTE));
	authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE;
	if (auth_priv(sid, "admin")&A_ADMIN) {
		authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	}
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		note->obj_uid=sid->dat->uid;
		note->obj_gid=sid->dat->gid;
		note->obj_did=sid->dat->did;
		note->obj_gperm=1;
		note->obj_operm=1;
		snprintf(note->notetitle, sizeof(note->notetitle)-1, "New Note");
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_notes where noteid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_notes where noteid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->uid, sid->dat->gid, perm, perm, sid->dat->did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	note->noteid     = atoi(sql_getvalue(&sqr, 0, 0));
	note->obj_ctime  = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	note->obj_mtime  = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	note->obj_uid    = atoi(sql_getvalue(&sqr, 0, 3));
	note->obj_gid    = atoi(sql_getvalue(&sqr, 0, 4));
	note->obj_did    = atoi(sql_getvalue(&sqr, 0, 5));
	note->obj_gperm  = atoi(sql_getvalue(&sqr, 0, 6));
	note->obj_operm  = atoi(sql_getvalue(&sqr, 0, 7));
	strncpy(note->tablename,		sql_getvalue(&sqr, 0, 8), sizeof(note->tablename)-1);
	note->tableindex=atoi(sql_getvalue(&sqr, 0, 9));
	strncpy(note->notetitle,		sql_getvalue(&sqr, 0, 10), sizeof(note->notetitle)-1);
	strncpy(note->notetext,		sql_getvalue(&sqr, 0, 11), sizeof(note->notetext)-1);
	if (strlen(note->notetitle)==0) strncpy(note->notetitle, "unnamed", sizeof(note->notetitle)-1);
	sql_freeresult(&sqr);
*/
	return 0;
}

int dbwrite_note(CONN *sid, int index, obj_t **qobj)
{
/*
	char curdate[32];
	char query[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE;
	if (auth_priv(sid, "admin")&A_ADMIN) {
		authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	}
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(noteid) FROM gw_notes")<0) return -1;
		note->noteid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (note->noteid<1) note->noteid=1;
		strcpy(query, "INSERT INTO gw_notes (noteid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, tablename, tableindex, notetitle, notetext) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", note->noteid, curdate, curdate, note->obj_uid, note->obj_gid, note->obj_did, note->obj_gperm, note->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, note->tablename));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", note->tableindex);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, note->notetitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "'");
		strncat(query, note->notetext, sizeof(query)-strlen(query)-3);
		strncat(query, "')", sizeof(query)-strlen(query)-1);
		if (sql_update(query)<0) return -1;
		return note->noteid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_notes SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, note->obj_uid, note->obj_gid, note->obj_gperm, note->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "tablename = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, note->tablename));
		strncatf(query, sizeof(query)-strlen(query)-1, "tableindex = '%d', ", note->tableindex);
		strncatf(query, sizeof(query)-strlen(query)-1, "notetitle = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, note->notetitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "notetext = '");
		strncat(query, note->notetext, sizeof(query)-strlen(query)-3);
		strncat(query, "'", sizeof(query)-strlen(query)-27);
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE noteid = %d", note->noteid);
		if (sql_update(query)<0) return -1;
		return note->noteid;
	}
*/
	return -1;
}
