/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

void taskreminders(CONNECTION *sid)
{
	char posttime[32];
	int a, b;
	int i;
	int reminders=0;
	int sqr;
	time_t duedate;
	time_t t;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, duedate, reminder FROM gw_tasks where duedate < #%s# and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
	}
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Due Tasks</FONT></B>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		a=time_sql2unix(sqlGetvalue(sqr, i, 2))-time(NULL);
		b=a-atoi(sqlGetvalue(sqr, i, 3))*60;
		if (b<0) {
			reminders++;
			if (reminders==1) {
				prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%%>\n");
				prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT>&nbsp;</TH><TH ALIGN=LEFT WIDTH=100%%>&nbsp;Task Name&nbsp;</TH><TH ALIGN=LEFT>&nbsp;Date&nbsp;</TH></TR>\n", COLOR_TH);
			}
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD NOWRAP VALIGN=top><A HREF=%s/tasks/reminderreset?taskid=%s>reset</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints(sid, "<TD><A HREF=%s/tasks/view?taskid=%s TARGET=gwmain>%s</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), str2html(sid, sqlGetvalue(sqr, i, 1)));
			duedate=time_sql2unix(sqlGetvalue(sqr, i, 2));
//			duedate+=time_tzoffset(sid, duedate);
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2datetext(sid, duedate));
			prints(sid, "</TR>\n");
		}
	}
	sqlFreeconnect(sqr);
	if (reminders==0) {
		prints(sid, "<BR>No approaching due tasks\n</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>window.close('_top');</SCRIPT>\n");
	} else {
		prints(sid, "</TABLE></CENTER>\n");
	}
	return;
}

void taskreminderstatus(CONNECTION *sid)
{
	char timebuffer[100];
	int taskid;
	int reminder;
	int sqr;
	time_t duedate;
	time_t now;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		return;
	}
	if (getgetenv(sid, "TASKID")==NULL) return;
	taskid=atoi(getgetenv(sid, "TASKID"));
	if ((sqr=sqlQueryf(sid, "SELECT reminder, duedate FROM gw_tasks where taskid = %d and assignedto = %d", taskid, sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return;
	}
	reminder=atoi(sqlGetvalue(sqr, 0, 0));
	duedate=time_sql2unix(sqlGetvalue(sqr, 0, 1));
	duedate-=time_tzoffset(sid, duedate);
	now=time(NULL);
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, now));
	sqlFreeconnect(sqr);
	duedate=(duedate-now)/60;
	if (duedate<0) {
		duedate=0;
		reminder=0;
	}
	switch (reminder) {
	case 10080: // 7 days
		reminder=8640;
		if (reminder<duedate) break;
	case 8640: // 6 days
		reminder=7200;
		if (reminder<duedate) break;
	case 7200: // 5 days
		reminder=5760;
		if (reminder<duedate) break;
	case 5760: // 4 days
		reminder=4320;
		if (reminder<duedate) break;
	case 4320: // 3 days
		reminder=2880;
		if (reminder<duedate) break;
	case 2880: // 2 days
		reminder=1440;
		if (reminder<duedate) break;
	case 1440: // 1 day
		reminder=0;
		if (reminder<duedate) break;
	default:
		reminder=0;
		break;
	}
	sqlUpdatef(sid, "UPDATE gw_tasks SET obj_mtime = '%s', reminder = %d WHERE taskid = %d", timebuffer, reminder, taskid);
	return;
}

void taskedit(CONNECTION *sid)
{
	REC_TASK task;
	char duedate[16];
	int taskid;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/tasks/editnew")==0) {
		taskid=0;
		if (db_read(sid, 2, DB_TASKS, 0, &task)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	} else {
		if (getgetenv(sid, "TASKID")==NULL) return;
		taskid=atoi(getgetenv(sid, "TASKID"));
		if (db_read(sid, 2, DB_TASKS, taskid, &task)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", taskid);
			return;
		}
	}
	strftime(duedate, sizeof(duedate)-1, "%Y-%m-%d", gmtime(&task.duedate));
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/tasks/save NAME=taskedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=taskid VALUE='%d'>\n", task.taskid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (taskid>0) {
		prints(sid, "<A HREF=%s/tasks/view?taskid=%d STYLE='color: %s'>Task Number %d</FONT></TH></TR>\n", sid->dat->in_ScriptName, task.taskid, COLOR_THTEXT, task.taskid);
	} else {
		prints(sid, "New Task</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Task Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=taskname VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, task.taskname));
	if (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN) {
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:182px'>\n", COLOR_EDITFORM);
		htselect_user(sid, task.assignedto);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Priority&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=priority style='width:182px'>\n", COLOR_EDITFORM);
	htselect_priority(sid, task.priority);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Reminder&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=reminder style='width:182px'>\n");
	htselect_reminder(sid, task.reminder);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Due Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=duedate2>\n", COLOR_EDITFORM);
	htselect_month(sid, duedate);
	prints(sid, "</SELECT><SELECT NAME=duedate1>\n");
	htselect_day(sid, duedate);
	prints(sid, "</SELECT><SELECT NAME=duedate3>\n");
	htselect_year(sid, 2000, duedate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:182px'>\n", COLOR_EDITFORM);
	htselect_eventstatus(sid, task.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, task.details));
	if ((task.obj_uid==sid->dat->user_uid)||(auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, task.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, task.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", task.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", task.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", task.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", task.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", task.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", task.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	if (taskid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save Task'>\n");
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete Task' onClick=\"return ConfirmDelete();\">\n");
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Add Task'>\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.taskedit.taskname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

/****************************************************************************
 *	taskview()
 *
 *	Purpose	: Display task information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void taskview(CONNECTION *sid)
{
	REC_TASK task;
	char duedate[16];
	int taskid;
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "TASKID")==NULL) return;
	taskid=atoi(getgetenv(sid, "TASKID"));
	if (db_read(sid, 1, DB_TASKS, taskid, &task)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", taskid);
		return;
	}
	strftime(duedate, sizeof(duedate)-1, "%Y-%m-%d", gmtime(&task.duedate));
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>%s", COLOR_TH, COLOR_THTEXT, str2html(sid, task.taskname));
	if (auth_priv(sid, AUTH_CALENDAR)&A_MODIFY) {
		prints(sid, " [<A HREF=%s/tasks/edit?taskid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, task.taskid, COLOR_THLINK);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Task Name  </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%><NOBR>%s&nbsp;</NOBR></TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, task.taskname));
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users"))<0) return;
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned By</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==task.assignedby) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned To</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==task.assignedto) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	sqlFreeconnect(sqr);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Priority   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	if (task.priority==0) prints(sid, "Lowest");
	else if (task.priority==1) prints(sid, "Low");
	else if (task.priority==2) prints(sid, "Normal");
	else if (task.priority==3) prints(sid, "High");
	else if (task.priority==4) prints(sid, "Highest");
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Reminder   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_reminder(sid, task.reminder));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Due Date   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_sql2datetext(sid, duedate));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Status     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_eventstatus(sid, task.status));
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><B>Details</B></TD></TR>\n", COLOR_FNAME);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FVAL, str2html(sid, task.details));
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>Notes", COLOR_TH, COLOR_THTEXT);
	prints(sid, " [<A HREF=%s/notes/editnew?table=tasks&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, task.taskid, COLOR_THLINK);
	prints(sid, "</FONT></TH></TR>\n");
	notessublist(sid, "tasks", task.taskid, 2);
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void tasklist(CONNECTION *sid, int userid)
{
	char *ptemp;
	int i;
	int sqr;
	int status;
	int tcount=0;
	
	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (status==2) {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, assignedto FROM gw_tasks WHERE (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY priority DESC, taskid ASC", sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, assignedto FROM gw_tasks WHERE status = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY priority DESC, taskid ASC", status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=200>\n");
	prints(sid, "<TR BGCOLOR=%s><TH  ALIGN=LEFT WIDTH=100%%><FONT SIZE=2 COLOR=%s>&nbsp;Tasks</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 2))!=userid) continue;
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=\"window.location.href='%s/tasks/view?taskid=%s'\">", COLOR_FVAL, sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints(sid, "&nbsp;<A HREF=%s/tasks/view?taskid=%s TITLE=\"%s\">%-.25s", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 1), str2html(sid, sqlGetvalue(sqr, i, 1)));
		if (strlen(sqlGetvalue(sqr, i, 1))>25) prints(sid, "..");
		prints(sid, "</A>&nbsp;</TD></TR>\n");
		tcount++;
	}
	sqlFreeconnect(sqr);
	if (status==2) {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, assignedto FROM gw_tasks WHERE (assignedto = 0 and ((obj_gid = %d and obj_gperm>0) or obj_operm>0)) ORDER BY priority DESC, taskid ASC", sid->dat->user_gid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT taskid, taskname, assignedto FROM gw_tasks WHERE status = %d and (assignedto = 0 and ((obj_gid = %d and obj_gperm>0) or obj_operm>0)) ORDER BY priority DESC, taskid ASC", status, sid->dat->user_gid))<0) return;
	}
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=\"window.location.href='%s/tasks/view?taskid=%s'\">", COLOR_FVAL, sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints(sid, "&nbsp;*&nbsp;<A HREF=%s/tasks/view?taskid=%s TITLE=\"[UNASSIGNED] %s\">%-.25s", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 1), str2html(sid, sqlGetvalue(sqr, i, 1)));
		if (strlen(sqlGetvalue(sqr, i, 1))>25) prints(sid, "..");
		prints(sid, "</A>&nbsp;</TD></TR>\n");
		tcount++;
	}
	sqlFreeconnect(sqr);
	if (!tcount) {
		prints(sid, "<TR BGCOLOR=%s><TD WIDTH=100%%>&nbsp;No tasks assigned&nbsp;</TD></TR>\n", COLOR_FVAL);
	}
	prints(sid, "</TABLE>\n");
	return;
}

void tasksave(CONNECTION *sid)
{
	REC_TASK task;
	char tempdate[40];
	char *ptemp;
	int taskid;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "TASKID"))==NULL) return;
	taskid=atoi(ptemp);
	if (db_read(sid, 2, DB_TASKS, taskid, &task)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) task.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) task.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)||(task.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) task.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) task.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ASSIGNEDBY"))!=NULL) task.assignedby=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ASSIGNEDTO"))!=NULL) task.assignedto=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "TASKNAME"))!=NULL) snprintf(task.taskname, sizeof(task.taskname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) task.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PRIORITY"))!=NULL) task.priority=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "REMINDER"))!=NULL) task.reminder=atoi(ptemp);
	memset(tempdate, 0, sizeof(tempdate));
	if ((ptemp=getpostenv(sid, "DUEDATE3"))!=NULL) snprintf(tempdate, sizeof(tempdate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "DUEDATE2"))!=NULL) strcatf(tempdate, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "DUEDATE1"))!=NULL) strcatf(tempdate, "%02d 00:00:00", atoi(ptemp));
	if (strlen(tempdate)!=19) memset(tempdate, 0, sizeof(tempdate));
	task.duedate=time_sql2unix(tempdate);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(task.details, sizeof(task.details)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete Task")==0)) {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_tasks WHERE taskid = %d", task.taskid)<0) return;
		prints(sid, "<CENTER>Task %d deleted successfully</CENTER><BR>\n", task.taskid);
		logaccess(sid, 1, "%s - %s deleted task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	} else if (task.taskid==0) {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((task.taskid=db_write(sid, DB_TASKS, 0, &task))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Task %d added successfully</CENTER><BR>\n", task.taskid);
		logaccess(sid, 1, "%s - %s added task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	} else {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (db_write(sid, DB_TASKS, taskid, &task)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Task %d modified successfully</CENTER><BR>\n", task.taskid);
		logaccess(sid, 1, "%s - %s modified task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/list\">\n", sid->dat->in_ScriptName);
	return;
}

void taskmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/tasks/reminders", 16)==0) {
		htpage_header(sid, "Groupware Task Reminder");
		taskreminders(sid);
		htpage_footer(sid);
		return;
	} else if (strncmp(sid->dat->in_RequestURI, "/tasks/reminderreset", 20)==0) {
		htpage_header(sid, "Groupware Task Reminder");
		taskreminderstatus(sid);
		taskreminders(sid);
		htpage_footer(sid);
		return;
	}
	htpage_topmenu(sid, MENU_TASKS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/tasks/edit", 11)==0) {
		taskedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/tasks/view", 11)==0) {
		taskview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/tasks/save", 11)==0) {
		tasksave(sid);
	}
	htpage_footer(sid);
	return;
}
