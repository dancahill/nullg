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
#include "http_mod.h"
#include "mod_projects.h"

int dbread_project(CONN *sid, short int perm, int index, REC_PROJECT *project)
{
	int authlevel;
	int sqr;

	memset(project, 0, sizeof(REC_PROJECT));
	authlevel=auth_priv(sid, "projects");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		project->obj_uid=sid->dat->user_uid;
		project->obj_gid=sid->dat->user_gid;
		project->obj_did=sid->dat->user_did;
		project->obj_gperm=1;
		project->obj_operm=1;
		project->projectadmin=sid->dat->user_uid;
		snprintf(project->projectname, sizeof(project->projectname)-1, "New Project");
		project->projectstart=time(NULL);
		project->projectfinish=time(NULL);
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_projects where projectid = %d AND obj_did = %d", index, sid->dat->user_did))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_projects where projectid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	project->projectid     = atoi(sql_getvalue(sqr, 0, 0));
	project->obj_ctime     = time_sql2unix(sql_getvalue(sqr, 0, 1));
	project->obj_mtime     = time_sql2unix(sql_getvalue(sqr, 0, 2));
	project->obj_uid       = atoi(sql_getvalue(sqr, 0, 3));
	project->obj_gid       = atoi(sql_getvalue(sqr, 0, 4));
	project->obj_did       = atoi(sql_getvalue(sqr, 0, 5));
	project->obj_gperm     = atoi(sql_getvalue(sqr, 0, 6));
	project->obj_operm     = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(project->projectname,	sql_getvalue(sqr, 0, 8), sizeof(project->projectname)-1);
	project->projectadmin  = atoi(sql_getvalue(sqr, 0, 9));
	project->projectstart  = time_sql2unix(sql_getvalue(sqr, 0, 10));
	project->projectfinish = time_sql2unix(sql_getvalue(sqr, 0, 11));
	project->status        = atoi(sql_getvalue(sqr, 0, 12));
	strncpy(project->details,	sql_getvalue(sqr, 0, 13), sizeof(project->details)-1);
	if (project->projectfinish<project->projectstart) project->projectfinish=project->projectstart;
	sql_freeresult(sqr);
	return 0;
}

int dbwrite_project(CONN *sid, int index, REC_PROJECT *project)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	int sqr;

	authlevel=auth_priv(sid, "projects");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if ((sqr=sql_query("SELECT max(projectid) FROM gw_projects"))<0) return -1;
		project->projectid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (project->projectid<1) project->projectid=1;
		strcpy(query, "INSERT INTO gw_projects (projectid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, projectname, projectadmin, projectstart, projectfinish, status, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", project->projectid, curdate, curdate, project->obj_uid, project->obj_gid, project->obj_did, project->obj_gperm, project->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", project->projectadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", project->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->details));
		if (sql_update(query)<0) return -1;
		return project->projectid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_projects SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, project->obj_uid, project->obj_gid, project->obj_gperm, project->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "projectname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectname));
		strncatf(query, sizeof(query)-strlen(query)-1, "projectadmin = '%d', ", project->projectadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "projectstart = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectstart));
		strncatf(query, sizeof(query)-strlen(query)-1, "projectfinish = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->projectfinish));
		strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", project->status);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, project->details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE projectid = %d", project->projectid);
		if (sql_update(query)<0) return -1;
		return project->projectid;
	}
	return -1;
}
