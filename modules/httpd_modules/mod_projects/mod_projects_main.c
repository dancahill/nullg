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
#include "mod_projects.h"

void htselect_projectfilter(CONN *sid, int selected, char *baseuri)
{
	char *option[]={ "Open", "Closed", "All" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int status;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (selected<1) {
		selected=sid->dat->user_uid;
	}
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", sid->dat->user_did))<0) return;
	prints(sid, "<FORM METHOD=GET NAME=projectfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.projectfilter.userid.options[document.projectfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\n", atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.projectfilter.status.options[document.projectfilter.status.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=status onChange=\"go2()\">');\r\n");
	for (i=0, j=2;i<3;i++, j++) {
		if (j>2) j=0;
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
		prints(sid, "&status=%d\"%s>%s');\n", j, j==status?" SELECTED":"", option[j]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(sqr);
	return;
}

void projectedit(CONN *sid)
{
	REC_PROJECT project;
	char projectstart[16];
	char projectfinish[16];
	int projectid;
	int editperms=0;

	if (!(auth_priv(sid, "projects")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/projects/editnew", 17)==0) {
		projectid=0;
		if (dbread_project(sid, 2, 0, &project)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	} else {
		if (getgetenv(sid, "PROJECTID")==NULL) return;
		projectid=atoi(getgetenv(sid, "PROJECTID"));
		if (dbread_project(sid, 2, projectid, &project)!=0) {
			prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", projectid);
			return;
		}
	}
//	project.projectstart+=time_tzoffset(sid, project.projectstart);
//	project.projectfinish+=time_tzoffset(sid, project.projectfinish);
	strftime(projectstart, sizeof(projectstart)-1, "%Y-%m-%d", gmtime(&project.projectstart));
	strftime(projectfinish, sizeof(projectfinish)-1, "%Y-%m-%d", gmtime(&project.projectfinish));
	prints(sid, "<BR>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (projectid>0) {
		prints(sid, "<B><A HREF=%s/projects/view?projectid=%d>Project Number %d</A></B>\n", sid->dat->in_ScriptName, project.projectid, project.projectid);
	} else {
		prints(sid, "<B>New Project</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/projects/save NAME=projectedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=projectid VALUE='%d'>\n", project.projectid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SETTINGS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>DETAILS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Project Name&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=projectname value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", str2html(sid, project.projectname), (auth_priv(sid, "projects")&A_ADMIN)||(project.status==0)?"":" DISABLED");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Admin&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectadmin style='width:255px'%s>\n", (auth_priv(sid, "projects")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, project.projectadmin);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectstart2 style='width:128px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_month(sid, projectstart);
	prints(sid, "</SELECT><SELECT NAME=projectstart1 style='width:64px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_day(sid, projectstart);
	prints(sid, "</SELECT><SELECT NAME=projectstart3 style='width:63px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, projectstart);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Finish&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectfinish2 style='width:128px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_month(sid, projectfinish);
	prints(sid, "</SELECT><SELECT NAME=projectfinish1 style='width:64px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_day(sid, projectfinish);
	prints(sid, "</SELECT><SELECT NAME=projectfinish3 style='width:63px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, projectfinish);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:255px'%s>\n", auth_priv(sid, "projects")&A_ADMIN?"":" DISABLED");
	htselect_eventstatus(sid, project.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50%s>%s</TEXTAREA></TD></TR>\n", (auth_priv(sid, "projects")&A_ADMIN)||(project.status==0)?"":" DISABLED", str2html(sid, project.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if ((project.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "projects")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "projects")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, project.obj_uid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "projects")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "projects"), project.obj_gid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\n", project.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\n", project.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\n", project.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\n", project.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\n", project.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\n", project.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	if (projectid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
		if (auth_priv(sid, "projects")&A_DELETE) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
		}
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Create'>\n");
	}
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	if ((auth_priv(sid, "projects")&A_ADMIN)||(project.status==0)) {
		prints(sid, "document.projectedit.projectname.focus();\n");
	}
	prints(sid, "showpage(1);\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	return;
}

/****************************************************************************
 *	projectview()
 *
 *	Purpose	: Display project information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void projectview(CONN *sid)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_PROJECT project;
	char *ptemp;
	int projectid;
	int i;
	int sqr;

	if (!(auth_priv(sid, "projects")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "PROJECTID"))==NULL) return;
	projectid=atoi(ptemp);
	if (dbread_project(sid, 1, projectid, &project)!=0) {
		prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", projectid);
		return;
	}
//	project.projectstart+=time_tzoffset(sid, project.projectstart);
//	project.projectfinish+=time_tzoffset(sid, project.projectfinish);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>%s", str2html(sid, project.projectname));
	if (auth_priv(sid, "projects")&A_MODIFY) {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/projects/edit?projectid=%d>edit</A>]", sid->dat->in_ScriptName, project.projectid);
		} else if ((project.projectadmin==sid->dat->user_uid)||(project.obj_uid==sid->dat->user_uid)||((project.obj_gid==sid->dat->user_gid)&&(project.obj_gperm>=2))||(project.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/projects/edit?projectid=%d>edit</A>]", sid->dat->in_ScriptName, project.projectid);
		}
	}
	prints(sid, "</TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Project Name  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", project.projectname);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Project Admin </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", htview_user(sid, project.projectadmin));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Start Date    </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", time_unix2datetext(sid, project.projectstart));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Finish Date   </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", time_unix2datetext(sid, project.projectfinish));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Status        </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", htview_eventstatus(sid, project.status));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", str2html(sid, project.details));
	if (module_exists(sid, "mod_calendar")) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Events");
		prints(sid, " [<A HREF=%s/calendar/editnew?projectid=%d>new</A>]", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "</TH></TR>\n");
//		if (auth_priv(sid, "admin")&A_ADMIN) {
			if ((sqr=sql_queryf("SELECT eventid, eventname FROM gw_events WHERE projectid = %d AND obj_did = %d ORDER BY eventid ASC", project.projectid, sid->dat->user_did))<0) return;
//		} else {
//			if ((sqr=sql_queryf("SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
//		}
		if (sql_numtuples(sqr)>0) {
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calendar/view?eventid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/calendar/view?eventid=%d>%s</A>&nbsp;</TD></TR>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
			}
		} else {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\n");
		}
		sql_freeresult(sqr);
	}
	if (module_exists(sid, "mod_tasks")) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Tasks");
		prints(sid, " [<A HREF=%s/tasks/editnew?projectid=%d>new</A>]", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "</TH></TR>\n");
//		if (auth_priv(sid, "admin")&A_ADMIN) {
			if ((sqr=sql_queryf("SELECT taskid, taskname FROM gw_tasks WHERE projectid = %d AND obj_did = %d ORDER BY taskid ASC", project.projectid, sid->dat->user_did))<0) return;
//		} else {
//			if ((sqr=sql_queryf("SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
//		}
		if (sql_numtuples(sqr)>0) {
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/tasks/view?taskid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/tasks/view?taskid=%d>%s</A>&nbsp;</TD></TR>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
			}
		} else {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\n");
		}
		sql_freeresult(sqr);
	}
	if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=projects&index=%d>new</A>]", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "</TH></TR>\n");
		mod_notes_sublist(sid, "projects", project.projectid, 2);
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void projectlist(CONN *sid)
{
	char *ptemp;
	int i;
	int offset=0;
	int sqr1;
	int total;
	int userid=-1;
	int status;
	time_t projectstart;

	if (!(auth_priv(sid, "projects")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "OFFSET")!=NULL) {
		offset=atoi(getgetenv(sid, "OFFSET"));
	}
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_projectfilter(sid, userid, "/projects/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\n");
	if (status==2) {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			if ((sqr1=sql_queryf("SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND obj_did = %d ORDER BY projectid DESC", userid, sid->dat->user_did))<0) return;
		} else {
			if ((sqr1=sql_queryf("SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND (projectadmin = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY projectid DESC", userid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
		}
	} else {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			if ((sqr1=sql_queryf("SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND status = %d AND obj_did = %d ORDER BY projectid DESC", userid, status, sid->dat->user_did))<0) return;
		} else {
			if ((sqr1=sql_queryf("SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND status = %d AND (projectadmin = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY projectid DESC", userid, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
		}
	}
	total=0;
	for (i=0;i<sql_numtuples(sqr1);i++) {
		if (atoi(sql_getvalue(sqr1, i, 5))==userid) total++;
	}
	if (total>0) {
		prints(sid, "Found %d matching projects<BR>\n", total);
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Project Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Admin&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Start Date&nbsp;</TH>");
		if (status==2) {
			prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Status&nbsp;</TH>");
		}
		prints(sid, "</TR>\n");
		for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
			if (atoi(sql_getvalue(sqr1, i, 5))!=userid) continue;
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' ");
			prints(sid, "onClick=\"window.location.href='%s/projects/view?projectid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/projects/view?projectid=%d>%s</A>&nbsp;</TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), sql_getvalue(sqr1, i, 1));
//			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", sql_getvalue(sqr1, i, 5));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", htview_user(sid, atoi(sql_getvalue(sqr1, i, 5))));
			projectstart=time_sql2unix(sql_getvalue(sqr1, i, 2));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, projectstart));
			if (status==2) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(sql_getvalue(sqr1, i, 4))==1?"Closed":"Open");
			}
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
		if (sql_numtuples(sqr1)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/projects/list?offset=%d&status=%d&userid=%d>Previous Page</A>]\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, status, userid);
			} else {
				prints(sid, "[Previous Page]\n");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
				prints(sid, "[<A HREF=%s/projects/list?offset=%d&status=%d&userid=%d>Next Page</A>]\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, status, userid);
			} else {
				prints(sid, "[Next Page]\n");
			}
		}
	} else {
		prints(sid, "<B>No ");
		if (status==0) {
			prints(sid, "open");
		} else if (status==1) {
			prints(sid, "closed");
		}
		prints(sid, " projects found</B>\n");
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr1);
	return;
}

void projectsave(CONN *sid)
{
	REC_PROJECT project;
	char projectstart[40];
	char projectfinish[40];
	char *ptemp;
	int projectid;

	if (!(auth_priv(sid, "projects")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "PROJECTID"))==NULL) return;
	projectid=atoi(ptemp);
	if (dbread_project(sid, 2, projectid, &project)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "projects")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) project.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) project.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "projects")&A_ADMIN)||(project.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) project.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) project.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "PROJECTNAME"))!=NULL) snprintf(project.projectname, sizeof(project.projectname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PROJECTADMIN"))!=NULL) project.projectadmin=atoi(ptemp);
	memset(projectstart, 0, sizeof(projectstart));
	if ((ptemp=getpostenv(sid, "PROJECTSTART"))!=NULL) {
		project.projectstart=atoi(ptemp);
	} else {
		if ((ptemp=getpostenv(sid, "PROJECTSTART3"))!=NULL) snprintf(projectstart, sizeof(projectstart)-1, "%d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTSTART2"))!=NULL) strncatf(projectstart, sizeof(projectstart)-strlen(projectstart)-1, "%02d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTSTART1"))!=NULL) strncatf(projectstart, sizeof(projectstart)-strlen(projectstart)-1, "%02d", atoi(ptemp));
		if (strlen(projectstart)==10) {
			project.projectstart=time_sql2unix(projectstart);
		} else {
			memset(projectstart, 0, sizeof(projectstart));
		}
	}
	memset(projectfinish, 0, sizeof(projectfinish));
	if ((ptemp=getpostenv(sid, "PROJECTFINISH"))!=NULL) {
		project.projectfinish=atoi(ptemp);
	} else {
		if ((ptemp=getpostenv(sid, "PROJECTFINISH3"))!=NULL) snprintf(projectfinish, sizeof(projectfinish)-1, "%d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTFINISH2"))!=NULL) strncatf(projectfinish, sizeof(projectfinish)-strlen(projectfinish)-1, "%02d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTFINISH1"))!=NULL) strncatf(projectfinish, sizeof(projectfinish)-strlen(projectfinish)-1, "%02d", atoi(ptemp));
		if (strlen(projectfinish)==10) {
			project.projectfinish=time_sql2unix(projectfinish);
		} else {
			memset(projectfinish, 0, sizeof(projectfinish));
		}
	}
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) project.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(project.details, sizeof(project.details)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "projects")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_projects WHERE projectid = %d", project.projectid)<0) return;
		prints(sid, "<BR><CENTER>Project %d deleted successfully</CENTER><BR>\n", project.projectid);
		db_log_activity(sid, 1, "projects", project.projectid, "delete", "%s - %s deleted project %d", sid->dat->in_RemoteAddr, sid->dat->user_username, project.projectid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/projects/list\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/list\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
	} else if (project.projectid==0) {
		if (!(auth_priv(sid, "projects")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((project.projectid=dbwrite_project(sid, 0, &project))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Project %d added successfully</CENTER><BR>\n", project.projectid);
		db_log_activity(sid, 1, "projects", project.projectid, "insert", "%s - %s added project %d", sid->dat->in_RemoteAddr, sid->dat->user_username, project.projectid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/projects/view?projectid=%d\");\n", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/view?projectid=%d\">\n", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "</NOSCRIPT>\n");
	} else {
		if (!(auth_priv(sid, "projects")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Save & Close")==0)) {
			project.projectfinish=time(NULL);
			project.status=1;
		}
		if (dbwrite_project(sid, projectid, &project)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Project %d modified successfully</CENTER><BR>\n", project.projectid);
		db_log_activity(sid, 1, "projects", project.projectid, "modify", "%s - %s modified project %d", sid->dat->in_RemoteAddr, sid->dat->user_username, project.projectid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/projects/view?projectid=%d\");\n", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/view?projectid=%d\">\n", sid->dat->in_ScriptName, project.projectid);
		prints(sid, "</NOSCRIPT>\n");
	}
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_PROJECTS);
	if (strncmp(sid->dat->in_RequestURI, "/projects/edit", 14)==0) {
		projectedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/projects/view", 14)==0) {
		projectview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/projects/list", 14)==0) {
		projectlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/projects/save", 14)==0) {
		projectsave(sid);
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_projects",		// mod_name
		1,			// mod_submenu
		"PROJECTS",		// mod_menuname
		"/projects/list",	// mod_menuuri
		"projects",		// mod_menuperm
		"mod_main",		// fn_name
		"/projects/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
