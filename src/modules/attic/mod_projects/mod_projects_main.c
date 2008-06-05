/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_projects.h"

void htselect_projectfilter(CONN *sid, int selected, char *baseuri)
{
/*
	char *option[]={ "Open", "Closed", "All" };
	char *ptemp;
	int i;
	int j;
	SQLRES sqr;
	int status;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (selected<1) {
		selected=sid->dat->uid;
	}
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", sid->dat->did)<0) return;
	prints(sid, "<FORM METHOD=GET NAME=projectfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.projectfilter.userid.options[document.projectfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\r\n", atoi(sql_getvalue(&sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.projectfilter.status.options[document.projectfilter.status.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=status onChange=\"go2()\">');\r\n");
	for (i=0, j=2;i<3;i++, j++) {
		if (j>2) j=0;
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
		prints(sid, "&status=%d\"%s>%s');\r\n", j, j==status?" SELECTED":"", option[j]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\r\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\r\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\r\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(&sqr);
*/
	return;
}

void projectedit(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
//	REC_PROJECT project;
	char projectstart[16];
	char projectfinish[16];
	int id;
//	int editperms=0;
	int priv=auth_priv(sid, "projects");
	char *ptemp;

	if (!(priv&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/projects/editnew", 17)==0) {
		id=0;
		if (dbread_project(sid, 2, 0, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
	} else {
		if ((ptemp=getgetenv(sid, "ID"))==NULL) return;
		id=atoi(ptemp);
		if (dbread_project(sid, 2, id, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", id);
			return;
		}
	}
//	project.projectstart+=time_tzoffset(sid, project.projectstart);
//	project.projectfinish+=time_tzoffset(sid, project.projectfinish);
//	strftime(projectstart, sizeof(projectstart)-1, "%Y-%m-%d", gmtime(&project.projectstart));
//	strftime(projectfinish, sizeof(projectfinish)-1, "%Y-%m-%d", gmtime(&project.projectfinish));
	strncpy(projectstart, ldir_getval(&qobj, 0, "projectstart"), 10);
	strncpy(projectfinish, ldir_getval(&qobj, 0, "projectfinish"), 10);
	prints(sid, "<BR />\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (id>0) {
		prints(sid, "<B><A HREF=%s/projects/view?id=%d>Project Number %d</A></B>\r\n", ScriptName, id, id);
	} else {
		prints(sid, "<B>New Project</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/projects/save NAME=projectedit>\r\n", ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=id VALUE='%d'>\r\n", id);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SETTINGS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>DETAILS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Project Name&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=projectname value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "projectname")), (priv&A_ADMIN)||(atoi(ldir_getval(&qobj, 0, "status"))==0)?"":" DISABLED");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Admin&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectadmin style='width:255px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, atoi(ldir_getval(&qobj, 0, "projectadmin")), sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectstart2 style='width:128px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_month(sid, projectstart);
	prints(sid, "</SELECT><SELECT NAME=projectstart1 style='width:64px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_day(sid, projectstart);
	prints(sid, "</SELECT><SELECT NAME=projectstart3 style='width:63px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, projectstart);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Project Finish&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=projectfinish2 style='width:128px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_month(sid, projectfinish);
	prints(sid, "</SELECT><SELECT NAME=projectfinish1 style='width:64px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_day(sid, projectfinish);
	prints(sid, "</SELECT><SELECT NAME=projectfinish3 style='width:63px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, projectfinish);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:255px'%s>\r\n", priv&A_ADMIN?"":" DISABLED");
	htselect_eventstatus(sid, atoi(ldir_getval(&qobj, 0, "status")));
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50%s>%s</TEXTAREA></TD></TR>\r\n", (priv&A_ADMIN)||(atoi(ldir_getval(&qobj, 0, "status"))==0)?"":" DISABLED", str2html(sid, ldir_getval(&qobj, 0, "details")));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*	if ((atoi(ldir_getval(&qobj, 0, "obj_uid"))==sid->dat->uid)||(priv&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, project.obj_uid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, priv, project.obj_gid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", project.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", project.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", project.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", project.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", project.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", project.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
*/	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (id!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
		if (priv&A_DELETE) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
		}
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Create'>\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	if ((priv&A_ADMIN)||(ldir_getval(&qobj, 0, "status")==0)) {
		prints(sid, "document.projectedit.projectname.focus();\r\n");
	}
	prints(sid, "showpage(1);\r\n");
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
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
//	REC_PROJECT project;
	char *ptemp;
	int id;
//	int i;
//	SQLRES sqr;
	int priv=auth_priv(sid, "projects");

	if (!(priv&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "ID"))==NULL) return;
	id=atoi(ptemp);
	if (dbread_project(sid, 1, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", id);
		return;
	}
//	project.projectstart+=time_tzoffset(sid, project.projectstart);
//	project.projectfinish+=time_tzoffset(sid, project.projectfinish);
	prints(sid, "<BR />\r\n");
	prints(sid, "<CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>%s", str2html(sid, ldir_getval(&qobj, 0, "projectname")));
	if (auth_priv(sid, "projects")&A_MODIFY) {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/projects/edit?id=%d>edit</A>]", ScriptName, id);
//		} else if ((atoi(ldir_getval(&qobj, 0, "projectadmin"))==sid->dat->uid)||(project.obj_uid==sid->dat->uid)||((project.obj_gid==sid->dat->gid)&&(project.obj_gperm>=2))||(project.obj_operm>=2)) {
//			prints(sid, " [<A HREF=%s/projects/edit?id=%d>edit</A>]", ScriptName, id);
		}
	}
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Project Name  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", ldir_getval(&qobj, 0, "projectname"));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Project Admin </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", htview_user(sid, atoi(ldir_getval(&qobj, 0, "projectadmin"))));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Start Date    </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", time_sql2datetext(sid, ldir_getval(&qobj, 0, "projectstart")));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Finish Date   </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", time_sql2datetext(sid, ldir_getval(&qobj, 0, "projectfinish")));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Status        </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", htview_eventstatus(atoi(ldir_getval(&qobj, 0, "status"))));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "details")));
	if (module_exists("calendar")) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Events");
		prints(sid, " [<A HREF=%s/calendar/editnew?id=%d>new</A>]", ScriptName, id);
		prints(sid, "</TH></TR>\r\n");
/*
//		if (auth_priv(sid, "admin")&A_ADMIN) {
			if (sql_queryf(&sqr, "SELECT eventid, eventname FROM gw_events WHERE projectid = %d AND obj_did = %d ORDER BY eventid ASC", project.projectid, sid->dat->did)<0) return;
//		} else {
//			if (sql_queryf(&sqr, "SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
//		}
		if (sql_numtuples(&sqr)>0) {
			for (i=0;i<sql_numtuples(&sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calendar/view?eventid=%d'\">", ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/calendar/view?eventid=%d>%s</A>&nbsp;</TD></TR>\r\n", ScriptName, atoi(sql_getvalue(&sqr, i, 0)), str2html(sid, sql_getvalue(&sqr, i, 1)));
			}
		} else {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\r\n");
		}
		sql_freeresult(&sqr);
*/
	}
	if (module_exists("tasks")) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Tasks");
		prints(sid, " [<A HREF=%s/tasks/editnew?id=%d>new</A>]", ScriptName, id);
		prints(sid, "</TH></TR>\r\n");
/*
//		if (auth_priv(sid, "admin")&A_ADMIN) {
			if (sql_queryf(&sqr, "SELECT taskid, taskname FROM gw_tasks WHERE projectid = %d AND obj_did = %d ORDER BY taskid ASC", project.projectid, sid->dat->did)<0) return;
//		} else {
//			if (sql_queryf(&sqr, "SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
//		}
		if (sql_numtuples(&sqr)>0) {
			for (i=0;i<sql_numtuples(&sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/tasks/view?taskid=%d'\">", ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/tasks/view?taskid=%d>%s</A>&nbsp;</TD></TR>\r\n", ScriptName, atoi(sql_getvalue(&sqr, i, 0)), str2html(sid, sql_getvalue(&sqr, i, 1)));
			}
		} else {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\r\n");
		}
		sql_freeresult(&sqr);
*/
	}
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=projects&index=%d>new</A>]", ScriptName, id);
		prints(sid, "</TH></TR>\r\n");
		mod_notes_sublist(sid, "projects", id, 2);
	}
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
}

void projectlist(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int i;
	int offset=0;
	int priv=auth_priv(sid, "projects");
	int status;
	int total;
	int userid=-1;
	time_t projectstart;

	if (!(priv&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
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
		userid=sid->dat->uid;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_projectfilter(sid, userid, "/projects/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\r\n");
/*	if (status==2) {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			if (sql_queryf(&sqr1, "SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND obj_did = %d ORDER BY projectid DESC", userid, sid->dat->did)<0) return;
		} else {
			if (sql_queryf(&sqr1, "SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND (projectadmin = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY projectid DESC", userid, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		}
	} else {
		if (auth_priv(sid, "projects")&A_ADMIN) {
			if (sql_queryf(&sqr1, "SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND status = %d AND obj_did = %d ORDER BY projectid DESC", userid, status, sid->dat->did)<0) return;
		} else {
			if (sql_queryf(&sqr1, "SELECT projectid, projectname, projectstart, projectfinish, status, projectadmin FROM gw_projects where projectadmin = %d AND status = %d AND (projectadmin = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY projectid DESC", userid, status, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		}
	}*/
	if ((qobj=ldir_getlist(sid->N, "project", 0, sid->dat->did))==NULL) return;
	total=0;
	for (i=0;i<ldir_numentries(&qobj);i++) {
//		if (atoi(ldir_getval(&qobj, i, "projectadmin"))!=userid) continue;
		total++;
	}
	if (total>0) {
		prints(sid, "Found %d matching projects<BR />\r\n", total);
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Project Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Admin&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Start Date&nbsp;</TH>");
		if (status==2) {
			prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Status&nbsp;</TH>");
		}
		prints(sid, "</TR>\r\n");
//		for (i=offset;(i<sql_numtuples(&sqr1))&&(i<offset+sid->dat->maxlist);i++) {
		for (i=offset;(i<ldir_numentries(&qobj))&&(i<offset+sid->dat->maxlist);i++) {
//			if (atoi(ldir_getval(&qobj, i, "projectadmin"))!=userid) continue;
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' ");
			prints(sid, "onClick=\"window.location.href='%s/projects/view?id=%d'\">", ScriptName, atoi(ldir_getval(&qobj, i, "id")));
			prints(sid, "&nbsp;<A HREF=%s/projects/view?id=%d>%s</A>&nbsp;</TD>\r\n", ScriptName, atoi(ldir_getval(&qobj, i, "id")), ldir_getval(&qobj, i, "projectname"));
//			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", sql_getvalue(&sqr1, i, 5));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", htview_user(sid, atoi(ldir_getval(&qobj, i, "projectadmin"))));
			projectstart=time_sql2unix(ldir_getval(&qobj, i, "projectstart"));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, projectstart));
			if (status==2) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(ldir_getval(&qobj, i, "status"))==1?"Closed":"Open");
			}
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
		if (ldir_numentries(&qobj)>sid->dat->maxlist) {
			if (offset>sid->dat->maxlist-1) {
				prints(sid, "[<A HREF=%s/projects/list?offset=%d&status=%d&userid=%d>Previous Page</A>]\r\n", ScriptName, offset-sid->dat->maxlist, status, userid);
			} else {
				prints(sid, "[Previous Page]\r\n");
			}
			if (offset+sid->dat->maxlist<ldir_numentries(&qobj)) {
				prints(sid, "[<A HREF=%s/projects/list?offset=%d&status=%d&userid=%d>Next Page</A>]\r\n", ScriptName, offset+sid->dat->maxlist, status, userid);
			} else {
				prints(sid, "[Next Page]\r\n");
			}
		}
	} else {
		prints(sid, "<B>No ");
		if (status==0) {
			prints(sid, "open");
		} else if (status==1) {
			prints(sid, "closed");
		}
		prints(sid, " projects found</B>\r\n");
	}
	prints(sid, "</CENTER>\r\n");
	ldir_freeresult(&qobj);
	return;
}

void projectsave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char projectstart[40];
	char projectfinish[40];
	char *ptemp;
	int id, pid;
	int priv=auth_priv(sid, "projects");

	prints(sid, "<BR />\r\n");
	if (!(priv&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_project(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (auth_priv(sid, "projects")&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) project.obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) project.obj_gid=atoi(ptemp);
//	}
//	if ((auth_priv(sid, "projects")&A_ADMIN)||(project.obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) project.obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) project.obj_operm=atoi(ptemp);
//	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	if ((ptemp=getpostenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	nes_setnum(sid->N, tobj, "pid", pid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	if ((ptemp=getpostenv(sid, "PROJECTNAME"))!=NULL)  nes_setstr(sid->N, tobj, "projectname", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "PROJECTADMIN"))!=NULL) nes_setnum(sid->N, tobj, "projectadmin", atoi(ptemp));
	memset(projectstart, 0, sizeof(projectstart));
	if ((ptemp=getpostenv(sid, "PROJECTSTART"))!=NULL) {
		nes_setnum(sid->N, tobj, "projectstart", atoi(ptemp));
	} else {
		if ((ptemp=getpostenv(sid, "PROJECTSTART3"))!=NULL) snprintf(projectstart, sizeof(projectstart)-1, "%d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTSTART2"))!=NULL) strncatf(projectstart, sizeof(projectstart)-strlen(projectstart)-1, "%02d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTSTART1"))!=NULL) strncatf(projectstart, sizeof(projectstart)-strlen(projectstart)-1, "%02d", atoi(ptemp));
		if (strlen(projectstart)==10) {
			nes_setstr(sid->N, tobj, "projectstart", projectstart, strlen(projectstart));
		} else {
			memset(projectstart, 0, sizeof(projectstart));
		}
	}
	memset(projectfinish, 0, sizeof(projectfinish));
	if ((ptemp=getpostenv(sid, "PROJECTFINISH"))!=NULL) {
		nes_setnum(sid->N, tobj, "projectfinish", atoi(ptemp));
	} else {
		if ((ptemp=getpostenv(sid, "PROJECTFINISH3"))!=NULL) snprintf(projectfinish, sizeof(projectfinish)-1, "%d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTFINISH2"))!=NULL) strncatf(projectfinish, sizeof(projectfinish)-strlen(projectfinish)-1, "%02d-", atoi(ptemp));
		if ((ptemp=getpostenv(sid, "PROJECTFINISH1"))!=NULL) strncatf(projectfinish, sizeof(projectfinish)-strlen(projectfinish)-1, "%02d", atoi(ptemp));
		if (strlen(projectfinish)==10) {
			nes_setstr(sid->N, tobj, "projectfinish", projectfinish, strlen(projectfinish));
		} else {
			memset(projectfinish, 0, sizeof(projectfinish));
		}
	}
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL)       nes_setnum(sid->N, tobj, "status", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL)      nes_setstr(sid->N, tobj, "details", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(priv&A_DELETE)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (ldir_deleteentry("project", id, sid->dat->did)<0) goto cleanup;
		prints(sid, "<CENTER>Project %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "projects", id, "delete", "%s - %s deleted project %d", sid->socket.RemoteAddr, sid->dat->username, id);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/projects/list\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/list\"></NOSCRIPT>\r\n", ScriptName);
		goto cleanup;
	} else if (id==0) {
		if (!(priv&A_INSERT)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_project(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Project %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "projects", id, "insert", "%s - %s added project %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Save & Close")==0)) {
			time_unix2sql(projectfinish, sizeof(projectfinish)-1, time(NULL));
			nes_setstr(sid->N, tobj, "projectfinish", projectfinish, 10);
			nes_setnum(sid->N, tobj, "status", 1);
		}
		if ((id=dbwrite_project(sid, id, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Project %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "projects", id, "modify", "%s - %s modified project %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/projects/view?id=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/view?id=%d\"></NOSCRIPT>\r\n", ScriptName, id);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, "projects");
	if (strncmp(RequestURI, "/projects/edit", 14)==0) {
		projectedit(sid);
	} else if (strncmp(RequestURI, "/projects/view", 14)==0) {
		projectview(sid);
	} else if (strncmp(RequestURI, "/projects/list", 14)==0) {
		projectlist(sid);
	} else if (strncmp(RequestURI, "/projects/save", 14)==0) {
		projectsave(sid);
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *tobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "projects");
	nes_exec(proc->N, "GWMODULES.projects={submenu=1,name='PROJECTS',pic='default.png',uri='/projects/list',perm='projects',fn_name='mod_main',fn_uri='/projects/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
