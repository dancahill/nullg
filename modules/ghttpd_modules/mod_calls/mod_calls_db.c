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
#include "mod_calls.h"

int dbread_call(CONN *sid, short int perm, int index, REC_CALL *call)
{
	int authlevel;
	int sqr;

	memset(call, 0, sizeof(REC_CALL));
	authlevel=auth_priv(sid, "calls");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		call->obj_uid=sid->dat->user_uid;
		call->obj_gid=sid->dat->user_gid;
		call->obj_did=sid->dat->user_did;
		call->obj_gperm=1;
		call->obj_operm=1;
		call->assignedby=sid->dat->user_uid;
		call->assignedto=sid->dat->user_uid;
		snprintf(call->callname, sizeof(call->callname)-1, "New Call");
		call->callstart=time(NULL);
		call->callfinish=time(NULL);
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_calls where callid = %d AND obj_did = %d", index, sid->dat->user_did))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_calls where callid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	call->callid     = atoi(sql_getvalue(sqr, 0, 0));
	call->obj_ctime  = time_sql2unix(sql_getvalue(sqr, 0, 1));
	call->obj_mtime  = time_sql2unix(sql_getvalue(sqr, 0, 2));
	call->obj_uid    = atoi(sql_getvalue(sqr, 0, 3));
	call->obj_gid    = atoi(sql_getvalue(sqr, 0, 4));
	call->obj_did    = atoi(sql_getvalue(sqr, 0, 5));
	call->obj_gperm  = atoi(sql_getvalue(sqr, 0, 6));
	call->obj_operm  = atoi(sql_getvalue(sqr, 0, 7));
	call->assignedby=atoi(sql_getvalue(sqr, 0, 8));
	call->assignedto=atoi(sql_getvalue(sqr, 0, 9));
	strncpy(call->callname,		sql_getvalue(sqr, 0, 10), sizeof(call->callname)-1);
	call->callstart=time_sql2unix(sql_getvalue(sqr, 0, 11));
	call->callfinish=time_sql2unix(sql_getvalue(sqr, 0, 12));
	call->contactid=atoi(sql_getvalue(sqr, 0, 13));
	call->action=atoi(sql_getvalue(sqr, 0, 14));
	call->status=atoi(sql_getvalue(sqr, 0, 15));
	strncpy(call->details,		sql_getvalue(sqr, 0, 16), sizeof(call->details)-1);
	if (call->callfinish<call->callstart) call->callfinish=call->callstart;
	sql_freeresult(sqr);
	return 0;
}

int dbwrite_call(CONN *sid, int index, REC_CALL *call)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	int sqr;

	authlevel=auth_priv(sid, "calls");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if ((sqr=sql_query("SELECT max(callid) FROM gw_calls"))<0) return -1;
		call->callid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (call->callid<1) call->callid=1;
		strcpy(query, "INSERT INTO gw_calls (callid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, assignedby, assignedto, callname, callstart, callfinish, contactid, action, status, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", call->callid, curdate, curdate, call->obj_uid, call->obj_gid, call->obj_did, call->obj_gperm, call->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", call->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", call->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", call->contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", call->action);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", call->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->details));
		if (sql_update(query)<0) return -1;
		return call->callid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_calls SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, call->obj_uid, call->obj_gid, call->obj_gperm, call->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", call->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", call->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "callname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callname));
		strncatf(query, sizeof(query)-strlen(query)-1, "callstart = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "callfinish = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->callfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", call->contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "action = '%d', ", call->action);
		strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", call->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, call->details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE callid = %d", call->callid);
		if (sql_update(query)<0) return -1;
		return call->callid;
	}
	return -1;
}
