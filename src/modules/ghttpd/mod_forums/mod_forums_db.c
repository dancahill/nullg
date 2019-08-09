/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_forums.h"

int dbread_forumgroup(CONN *sid, short int perm, int index, REC_FORUMGROUP *forumgroup)
{
	int authlevel;
	SQLRES sqr;

	memset(forumgroup, 0, sizeof(REC_FORUMGROUP));
	authlevel=auth_priv(sid, "forums");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		forumgroup->obj_uid=sid->dat->user_uid;
		forumgroup->obj_gid=sid->dat->user_gid;
		forumgroup->obj_did=sid->dat->user_did;
		forumgroup->obj_gperm=1;
		forumgroup->obj_operm=1;
		snprintf(forumgroup->title, sizeof(forumgroup->title)-1, "New Forum Group");
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_forums_groups where forumgroupid = %d AND obj_did = %d", index, sid->dat->user_did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_forums_groups where forumgroupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	forumgroup->forumgroupid = atoi(sql_getvalue(&sqr, 0, 0));
	forumgroup->obj_ctime    = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	forumgroup->obj_mtime    = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	forumgroup->obj_uid      = atoi(sql_getvalue(&sqr, 0, 3));
	forumgroup->obj_gid      = atoi(sql_getvalue(&sqr, 0, 4));
	forumgroup->obj_did      = atoi(sql_getvalue(&sqr, 0, 5));
	forumgroup->obj_gperm    = atoi(sql_getvalue(&sqr, 0, 6));
	forumgroup->obj_operm    = atoi(sql_getvalue(&sqr, 0, 7));
	strncpy(forumgroup->title,       sql_getvalue(&sqr, 0, 8), sizeof(forumgroup->title)-1);
	strncpy(forumgroup->description, sql_getvalue(&sqr, 0, 9), sizeof(forumgroup->description)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbwrite_forumgroup(CONN *sid, int index, REC_FORUMGROUP *forumgroup)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=auth_priv(sid, "forums");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(forumgroupid) FROM gw_forums_groups")<0) return -1;
		forumgroup->forumgroupid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (forumgroup->forumgroupid<1) forumgroup->forumgroupid=1;
		strcpy(query, "INSERT INTO gw_forums_groups (forumgroupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, title, description) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", forumgroup->forumgroupid, curdate, curdate, forumgroup->obj_uid, forumgroup->obj_gid, forumgroup->obj_did, forumgroup->obj_gperm, forumgroup->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, forumgroup->title));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, forumgroup->description));
		if (sql_update(query)<0) return -1;
		return forumgroup->forumgroupid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_forums_groups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, forumgroup->obj_uid, forumgroup->obj_gid, forumgroup->obj_gperm, forumgroup->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "title = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, forumgroup->title));
		strncatf(query, sizeof(query)-strlen(query)-1, "description = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, forumgroup->description));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE forumgroupid = %d", forumgroup->forumgroupid);
		if (sql_update(query)<0) return -1;
		return forumgroup->forumgroupid;
	}
	return -1;
}
