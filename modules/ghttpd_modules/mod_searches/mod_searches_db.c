/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "mod_searches.h"

int dbread_query(CONN *sid, short int perm, int index, REC_QUERY *query)
{
	int authlevel;
	SQLRES sqr;

	memset(query, 0, sizeof(REC_QUERY));
	authlevel=auth_priv(sid, "query");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		query->obj_uid=sid->dat->user_uid;
		query->obj_gid=sid->dat->user_gid;
		query->obj_did=sid->dat->user_did;
		query->obj_gperm=1;
		query->obj_operm=1;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_queries where queryid = %d", index)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_queries where queryid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	query->queryid   = atoi(sql_getvalue(&sqr, 0, 0));
	query->obj_ctime = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	query->obj_mtime = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	query->obj_uid   = atoi(sql_getvalue(&sqr, 0, 3));
	query->obj_gid   = atoi(sql_getvalue(&sqr, 0, 4));
	query->obj_did   = atoi(sql_getvalue(&sqr, 0, 5));
	query->obj_gperm = atoi(sql_getvalue(&sqr, 0, 6));
	query->obj_operm = atoi(sql_getvalue(&sqr, 0, 7));
	strncpy(query->queryname, sql_getvalue(&sqr, 0, 8), sizeof(query->queryname)-1);
	strncpy(query->query,     sql_getvalue(&sqr, 0, 9), sizeof(query->query)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbwrite_query(CONN *sid, int index, REC_QUERY *query)
{
	char curdate[32];
	char querybuf[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=auth_priv(sid, "query");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(querybuf, 0, sizeof(querybuf));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(queryid) FROM gw_queries")<0) return -1;
		query->queryid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (query->queryid<1) query->queryid=1;
		strcpy(querybuf, "INSERT INTO gw_queries (queryid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, queryname, query) values (");
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", query->queryid, curdate, curdate, query->obj_uid, query->obj_gid, query->obj_did, query->obj_gperm, query->obj_operm);
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, query->queryname));
		query->obj_did=sid->dat->user_did;
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, query->query));
		if (sql_update(querybuf)<0) return -1;
		return query->queryid;
	} else {
		snprintf(querybuf, sizeof(querybuf)-1, "UPDATE gw_queries SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, query->obj_uid, query->obj_gid, query->obj_gperm, query->obj_operm);
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, "queryname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, query->queryname));
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, "query = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, query->query));
		strncatf(querybuf, sizeof(querybuf)-strlen(querybuf)-1, " WHERE queryid = %d", query->queryid);
		if (sql_update(querybuf)<0) return -1;
		return query->queryid;
	}
	return -1;
}
