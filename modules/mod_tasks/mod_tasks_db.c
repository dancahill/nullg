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
#include "mod_substub.h"
#include "mod_tasks.h"

int dbread_task(CONN *sid, short int perm, int index, REC_TASK *task)
{
	int authlevel;
	int sqr;

	memset(task, 0, sizeof(REC_TASK));
	authlevel=auth_priv(sid, "calendar");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		task->obj_uid=sid->dat->user_uid;
		task->obj_gid=sid->dat->user_gid;
		task->obj_gperm=1;
		task->obj_operm=1;
		task->assignedby=sid->dat->user_uid;
		task->assignedto=sid->dat->user_uid;
		snprintf(task->taskname, sizeof(task->taskname)-1, "New Task");
		task->priority=2;
		task->duedate=(time(NULL)/86400)*86400;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_tasks where taskid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_tasks where taskid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	task->taskid = atoi(sql_getvalue(sqr, 0, 0));
	task->obj_ctime  = time_sql2unix(sql_getvalue(sqr, 0, 1));
	task->obj_mtime  = time_sql2unix(sql_getvalue(sqr, 0, 2));
	task->obj_uid    = atoi(sql_getvalue(sqr, 0, 3));
	task->obj_gid    = atoi(sql_getvalue(sqr, 0, 4));
	task->obj_did    = atoi(sql_getvalue(sqr, 0, 5));
	task->obj_gperm  = atoi(sql_getvalue(sqr, 0, 6));
	task->obj_operm  = atoi(sql_getvalue(sqr, 0, 7));
	task->assignedby=atoi(sql_getvalue(sqr, 0, 8));
	task->assignedto=atoi(sql_getvalue(sqr, 0, 9));
	strncpy(task->taskname,		sql_getvalue(sqr, 0, 10), sizeof(task->taskname)-1);
	task->duedate=time_sql2unix(sql_getvalue(sqr, 0, 11));
	task->priority=atoi(sql_getvalue(sqr, 0, 12));
	task->reminder=atoi(sql_getvalue(sqr, 0, 13));
	task->status=atoi(sql_getvalue(sqr, 0, 14));
	strncpy(task->details,		sql_getvalue(sqr, 0, 15), sizeof(task->details)-1);
	sql_freeresult(sqr);
	return 0;
}

int dbwrite_task(CONN *sid, int index, REC_TASK *task)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	int sqr;

	authlevel=auth_priv(sid, "calendar");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (index==0) {
		if ((sqr=sql_query(sid, "SELECT max(taskid) FROM gw_tasks"))<0) return -1;
		task->taskid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (task->taskid<1) task->taskid=1;
		strcpy(query, "INSERT INTO gw_tasks (taskid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, assignedby, assignedto, taskname, status, priority, reminder, duedate, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", task->taskid, curdate, curdate, task->obj_uid, task->obj_gid, task->obj_gperm, task->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", task->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", task->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, task->taskname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", task->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", task->priority);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", task->reminder);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, task->duedate));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, task->details));
		if (sql_update(sid, query)<0) return -1;
		return task->taskid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_tasks SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, task->obj_uid, task->obj_gid, task->obj_gperm, task->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", task->assignedby);
		strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", task->assignedto);
		strncatf(query, sizeof(query)-strlen(query)-1, "taskname = '%s', ", str2sql(sid, task->taskname));
		strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", task->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "priority = '%d', ", task->priority);
		strncatf(query, sizeof(query)-strlen(query)-1, "reminder = '%d', ", task->reminder);
		strncatf(query, sizeof(query)-strlen(query)-1, "duedate = '%s', ", time_unix2sql(sid, task->duedate));
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, task->details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE taskid = %d", task->taskid);
		if (sql_update(sid, query)<0) return -1;
		return task->taskid;
	}
	return -1;
}
