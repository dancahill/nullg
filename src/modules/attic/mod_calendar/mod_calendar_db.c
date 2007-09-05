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
#include "mod_calendar.h"

int dblist_events(CONN *sid, obj_t **qobj, char *startdate, char *enddate)
{
/*
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		return sql_queryf(sqr, "SELECT eventid, eventstart, eventfinish, status, assignedto, eventname FROM gw_events where eventstart >= '%s' and eventstart < '%s' and obj_did = %d ORDER BY eventstart ASC", startdate, enddate, sid->dat->did);
	} else {
		return sql_queryf(sqr, "SELECT eventid, eventstart, eventfinish, status, assignedto, eventname FROM gw_events where eventstart >= '%s' and eventstart < '%s' and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) and obj_did = %d  ORDER BY eventstart ASC", startdate, enddate, sid->dat->uid, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did);
	}
*/
}

int dbread_event(CONN *sid, short int perm, int index, obj_t **qobj)
{
/*
	int authlevel;
	SQLRES sqr;

	memset(event, 0, sizeof(REC_EVENT));
	authlevel=auth_priv(sid, "calendar");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		event->obj_uid=sid->dat->uid;
		event->obj_gid=sid->dat->gid;
		event->obj_did=sid->dat->did;
		event->obj_gperm=1;
		event->obj_operm=1;
		snprintf(event->eventname, sizeof(event->eventname)-1, "New Event");
		event->assignedby=sid->dat->uid;
		event->assignedto=sid->dat->uid;
		event->priority=2;
		event->eventstart=(time(NULL)/900)*900;
		event->eventfinish=(time(NULL)/900)*900+900;
		event->busy=1;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_events where eventid = %d and obj_did = %d", index, sid->dat->did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_events where eventid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) and obj_did = %d", index, sid->dat->uid, sid->dat->uid, sid->dat->uid, sid->dat->gid, perm, perm, sid->dat->did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	event->eventid   = atoi(sql_getvalue(&sqr, 0, 0));
	event->obj_ctime = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	event->obj_mtime = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	event->obj_uid   = atoi(sql_getvalue(&sqr, 0, 3));
	event->obj_gid   = atoi(sql_getvalue(&sqr, 0, 4));
	event->obj_did   = atoi(sql_getvalue(&sqr, 0, 5));
	event->obj_gperm = atoi(sql_getvalue(&sqr, 0, 6));
	event->obj_operm = atoi(sql_getvalue(&sqr, 0, 7));
	event->assignedby=atoi(sql_getvalue(&sqr, 0, 8));
	event->assignedto=atoi(sql_getvalue(&sqr, 0, 9));
	strncpy(event->eventname, sql_getvalue(&sqr, 0, 10), sizeof(event->eventname)-1);
	event->eventtype=atoi(sql_getvalue(&sqr, 0, 11));
	event->contactid=atoi(sql_getvalue(&sqr, 0, 12));
	event->projectid=atoi(sql_getvalue(&sqr, 0, 13));
	event->priority=atoi(sql_getvalue(&sqr, 0, 14));
	event->reminder=atoi(sql_getvalue(&sqr, 0, 15));
	event->eventstart=time_sql2unix(sql_getvalue(&sqr, 0, 16));
	event->eventfinish=time_sql2unix(sql_getvalue(&sqr, 0, 17));
	event->busy=atoi(sql_getvalue(&sqr, 0, 18));
	event->status=atoi(sql_getvalue(&sqr, 0, 19));
	event->closingstatus=atoi(sql_getvalue(&sqr, 0, 20));
	strncpy(event->details, sql_getvalue(&sqr, 0, 21), sizeof(event->details)-1);
	if (event->eventfinish<event->eventstart) event->eventfinish=event->eventstart;
	sql_freeresult(&sqr);
*/
	return 0;
}

int dbwrite_event(CONN *sid, int index, obj_t **qobj)
{
/*
	char curdate[32];
	char query[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=auth_priv(sid, "calendar");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(eventid) FROM gw_events")<0) return -1;
		event->eventid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (event->eventid<1) event->eventid=1;
		strcpy(query, "INSERT INTO gw_events (eventid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventname, assignedby, assignedto, eventtype, contactid, projectid, priority, reminder, eventstart, eventfinish, busy, status, closingstatus, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", event->eventid, curdate, curdate, event->obj_uid, event->obj_gid, event->obj_did, event->obj_gperm, event->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->eventtype);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->projectid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->priority);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->reminder);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->busy);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", event->closingstatus);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->details));
		if (sql_update(query)<0) return -1;
		return event->eventid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_events SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, event->obj_uid, event->obj_gid, event->obj_gperm, event->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "eventname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventname));
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", event->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", event->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "eventtype = '%d', ", event->eventtype);
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", event->contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "projectid = '%d', ", event->projectid);
		strncatf(query, sizeof(query)-strlen(query)-1, "priority = '%d', ", event->priority);
		strncatf(query, sizeof(query)-strlen(query)-1, "reminder = '%d', ", event->reminder);
		strncatf(query, sizeof(query)-strlen(query)-1, "eventstart = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "eventfinish = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->eventfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "busy = '%d', ", event->busy);
		strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", event->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "closingstatus = '%d', ", event->closingstatus);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, event->details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE eventid = %d", event->eventid);
		if (sql_update(query)<0) return -1;
		return event->eventid;
	}
*/
	return -1;
}
