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
#define SRVMOD_MAIN 1
#include "http_mod.h"
#include "mod_tasks.h"

void taskreminders(CONN *sid)
{
	char posttime[32];
	int a, b;
	int i;
	int reminders=0;
	int sqr;
	time_t duedate;
	time_t t;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=sql_queryf("SELECT taskid, taskname, duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Due Tasks</FONT></B>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		a=time_sql2unix(sql_getvalue(sqr, i, 2))-time(NULL);
		a-=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, i, 2)));
		b=a-atoi(sql_getvalue(sqr, i, 3))*60;
		if (b<0) {
			reminders++;
			if (reminders==1) {
				prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
				prints(sid, "<TR><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH><TH ALIGN=LEFT WIDTH=100%% STYLE='border-style:solid'>&nbsp;Task Name&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH></TR>\n");
			}
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP VALIGN=top STYLE='border-style:solid'><A HREF=%s/tasks/reminderreset?taskid=%s>reset</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			prints(sid, "<TD STYLE='border-style:solid'><A HREF=%s/tasks/view?taskid=%s TARGET=gwmain>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), str2html(sid, sql_getvalue(sqr, i, 1)));
			duedate=time_sql2unix(sql_getvalue(sqr, i, 2));
//			duedate+=time_tzoffset(sid, duedate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, duedate));
			prints(sid, "</TR>\n");
		}
	}
	sql_freeresult(sqr);
	if (reminders==0) {
		prints(sid, "<BR>No approaching due tasks\n</CENTER>\n");
//		prints(sid, "<SCRIPT LANGUAGE=JavaScript>window.close('_top');</SCRIPT>\n");
	} else {
		prints(sid, "</TABLE></CENTER>\n");
	}
	return;
}

void taskreminderstatus(CONN *sid)
{
	char timebuffer[100];
	char *ptemp;
	int taskid;
	int reminder;
	int sqr;
	time_t duedate;
	time_t now;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		return;
	}
	if ((ptemp=getgetenv(sid, "TASKID"))==NULL) return;
	taskid=atoi(ptemp);
	if ((sqr=sql_queryf("SELECT reminder, duedate FROM gw_tasks where taskid = %d and assignedto = %d", taskid, sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	reminder=atoi(sql_getvalue(sqr, 0, 0));
	duedate=time_sql2unix(sql_getvalue(sqr, 0, 1));
	duedate-=time_tzoffset(sid, duedate);
	now=time(NULL);
	time_unix2sql(timebuffer, sizeof(timebuffer)-1, now);
	sql_freeresult(sqr);
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
	sql_updatef("UPDATE gw_tasks SET obj_mtime = '%s', reminder = %d WHERE taskid = %d", timebuffer, reminder, taskid);
	return;
}

void taskedit(CONN *sid)
{
	REC_TASK task;
	char duedate[16];
	char *ptemp;
	int editperms=0;
	int taskid;

	if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/tasks/editnew")==0) {
		taskid=0;
		if (dbread_task(sid, 2, 0, &task)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	} else {
		if ((ptemp=getgetenv(sid, "TASKID"))==NULL) return;
		taskid=atoi(ptemp);
		if (dbread_task(sid, 2, taskid, &task)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", taskid);
			return;
		}
	}
	strftime(duedate, sizeof(duedate)-1, "%Y-%m-%d", gmtime(&task.duedate));
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (taskid>0) {
		prints(sid, "<B><A HREF=%s/tasks/view?taskid=%d>Task Number %d</A></B>\n", sid->dat->in_ScriptName, task.taskid, task.taskid);
	} else {
		prints(sid, "<B>New Task</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/tasks/save NAME=taskedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=taskid VALUE='%d'>\n", task.taskid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>TASK INFO</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>DETAILS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Task Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=taskname VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", str2html(sid, task.taskname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:255px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, task.assignedto);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Priority&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=priority style='width:255px'>\n");
	htselect_priority(sid, task.priority);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Reminder&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=reminder style='width:255px'>\n");
	htselect_reminder(sid, task.reminder);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Due Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=duedate2 style='width:127px'>\n");
	htselect_month(sid, duedate);
	prints(sid, "</SELECT><SELECT NAME=duedate1 style='width:64px'>\n");
	htselect_day(sid, duedate);
	prints(sid, "</SELECT><SELECT NAME=duedate3 style='width:64px'>\n");
	htselect_year(sid, 2000, duedate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:255px'>\n");
	htselect_eventstatus(sid, task.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", str2html(sid, task.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if ((task.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "calendar")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, task.obj_uid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, task.obj_gid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\n", task.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\n", task.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\n", task.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\n", task.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\n", task.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\n", task.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	if (taskid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save Task'>\n");
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete Task' onClick=\"return ConfirmDelete();\">\n");
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Add Task'>\n");
	}
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "showpage(1);\n");
	prints(sid, "document.taskedit.taskname.focus();\n");
	prints(sid, "// -->\n</SCRIPT>\n");
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
void taskview(CONN *sid)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_TASK task;
	char duedate[16];
	char *ptemp;
	int taskid;
	int i;
	int sqr;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "TASKID"))==NULL) return;
	taskid=atoi(ptemp);
	if (dbread_task(sid, 1, taskid, &task)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", taskid);
		return;
	}
	strftime(duedate, sizeof(duedate)-1, "%Y-%m-%d", gmtime(&task.duedate));
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>%s", str2html(sid, task.taskname));
	if (auth_priv(sid, "calendar")&A_MODIFY) {
		if ((task.assignedby==sid->dat->user_uid)||(task.assignedto==sid->dat->user_uid)||(task.obj_uid==sid->dat->user_uid)||((task.obj_gid==sid->dat->user_gid)&&(task.obj_gperm>=2))||(task.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/tasks/edit?taskid=%d>edit</A>]", sid->dat->in_ScriptName, task.taskid);
		}
	}
	prints(sid, "</FONT></TH></TR>\n");
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users"))<0) return;
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned By&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==task.assignedby) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned To&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==task.assignedto) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	sql_freeresult(sqr);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Priority&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
	if (task.priority==0) prints(sid, "Lowest");
	else if (task.priority==1) prints(sid, "Low");
	else if (task.priority==2) prints(sid, "Normal");
	else if (task.priority==3) prints(sid, "High");
	else if (task.priority==4) prints(sid, "Highest");
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Reminder&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", htview_reminder(sid, task.reminder));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Due Date&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", time_sql2datetext(sid, duedate));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Status&nbsp;</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", htview_eventstatus(sid, task.status));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Details&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", str2html(sid, task.details));
	if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=tasks&index=%d>new</A>]", sid->dat->in_ScriptName, task.taskid);
		prints(sid, "</FONT></TH></TR>\n");
		mod_notes_sublist(sid, "tasks", task.taskid, 2);
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void tasks_list(CONN *sid, int userid, int groupid)
{
	char *ptemp;
	int i;
	int j=0;
	int sqr;
	int sqr2;
	int status;
	int tcount=0;
	
	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT taskid, status, assignedto, taskname FROM gw_tasks ORDER BY priority DESC, taskid ASC"))<0) return;
	} else {
		if ((sqr=sql_queryf("SELECT taskid, status, assignedto, taskname FROM gw_tasks WHERE (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY priority DESC, taskid ASC", sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if ((sqr2=sql_queryf("SELECT userid, groupid, username FROM gw_users"))<0) {
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=200 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH  ALIGN=LEFT WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>&nbsp;Tasks</FONT></TH></TR>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, i, 1)))) continue;
		if ((userid>0)&&(userid!=atoi(sql_getvalue(sqr, i, 2)))) continue;
		if (groupid>0) {
			for (j=0;j<sql_numtuples(sqr2);j++) {
				if (atoi(sql_getvalue(sqr, i, 2))!=atoi(sql_getvalue(sqr2, j, 0))) continue;
				if (groupid!=atoi(sql_getvalue(sqr2, j, 1))) continue;
				break;
			}
			if (j==sql_numtuples(sqr2)) continue;
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/tasks/view?taskid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
		prints(sid, "<SPAN STYLE='width:196px;overflow:hidden'>&nbsp;<A HREF=%s/tasks/view?taskid=%s TITLE=\"", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
		if (groupid>0) prints(sid, "[%s] ", str2html(sid, sql_getvalue(sqr2, j, 2)));
		prints(sid, "%s\">%s</A>&nbsp;</SPAN></TD></TR>\n", sql_getvalue(sqr, i, 3), str2html(sid, sql_getvalue(sqr, i, 3)));
		tcount++;
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, i, 1)))) continue;
		if (atoi(sql_getvalue(sqr, i, 2))!=0) continue;
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/tasks/view?taskid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
		prints(sid, "<SPAN STYLE='width:196px;overflow:hidden'>&nbsp;*&nbsp;<A HREF=%s/tasks/view?taskid=%s TITLE=\"[UNASSIGNED] %s\">%s", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), sql_getvalue(sqr, i, 3), str2html(sid, sql_getvalue(sqr, i, 3)));
		prints(sid, "</A>&nbsp;</SPAN></TD></TR>\n");
		tcount++;
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	if (!tcount) {
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD WIDTH=100%% STYLE='border-style:solid'>&nbsp;No tasks assigned&nbsp;</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	return;
}

void tasksave(CONN *sid)
{
	REC_TASK task;
	char tempdate[40];
	char *ptemp;
	int taskid;

	if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "TASKID"))==NULL) return;
	taskid=atoi(ptemp);
	if (dbread_task(sid, 2, taskid, &task)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) task.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) task.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "calendar")&A_ADMIN)||(task.obj_uid==sid->dat->user_uid)) {
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
	if ((ptemp=getpostenv(sid, "DUEDATE2"))!=NULL) strncatf(tempdate, sizeof(tempdate)-strlen(tempdate)-1, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "DUEDATE1"))!=NULL) strncatf(tempdate, sizeof(tempdate)-strlen(tempdate)-1, "%02d 00:00:00", atoi(ptemp));
	if (strlen(tempdate)!=19) memset(tempdate, 0, sizeof(tempdate));
	task.duedate=time_sql2unix(tempdate);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(task.details, sizeof(task.details)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete Task")==0)) {
		if (!(auth_priv(sid, "calendar")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_tasks WHERE taskid = %d", task.taskid)<0) return;
		prints(sid, "<CENTER>Task %d deleted successfully</CENTER><BR>\n", task.taskid);
		db_log_activity(sid, 1, "tasks", task.taskid, "delete", "%s - %s deleted task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	} else if (task.taskid==0) {
		if (!(auth_priv(sid, "calendar")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((task.taskid=dbwrite_task(sid, 0, &task))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Task %d added successfully</CENTER><BR>\n", task.taskid);
		db_log_activity(sid, 1, "tasks", task.taskid, "insert", "%s - %s added task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	} else {
		if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (dbwrite_task(sid, taskid, &task)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Task %d modified successfully</CENTER><BR>\n", task.taskid);
		db_log_activity(sid, 1, "tasks", task.taskid, "modify", "%s - %s modified task %d", sid->dat->in_RemoteAddr, sid->dat->user_username, task.taskid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/list\">\n", sid->dat->in_ScriptName);
	return;
}

void mod_main(CONN *sid)
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

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_tasks",		// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menuuri
		"calendar",		// mod_menuperm
		"mod_main",		// fn_name
		"/tasks/",		// fn_uri
		mod_main		// fn_ptr
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	if (mod_export_function("mod_tasks", "mod_tasks_list", tasks_list)!=0) return -1;
	return 0;
}
