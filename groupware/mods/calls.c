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

void calledit(CONNECTION *sid)
{
	REC_CALL call;
	char calldate[16];
	char *ptemp;
	int callid;

	if (!(auth_priv(sid, AUTH_CALLS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/calls/editnew", 14)==0) {
		callid=0;
		if (db_read(sid, 2, DB_CALLS, 0, &call)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) call.contactid=atoi(ptemp);
	} else {
		if (getgetenv(sid, "CALLID")==NULL) return;
		callid=atoi(getgetenv(sid, "CALLID"));
		if (db_read(sid, 2, DB_CALLS, callid, &call)!=0) {
			prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", callid);
			return;
		}
	}
	call.calldate+=time_tzoffset(sid, call.calldate);
	strftime(calldate, sizeof(calldate)-1, "%Y-%m-%d", gmtime(&call.calldate));
	prints(sid, "<BR>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/calls/save NAME=calledit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=callid VALUE='%d'>\n", call.callid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (callid>0) {
		prints(sid, "<A HREF=%s/calls/view?callid=%d STYLE='color: %s'>Call Number %d</FONT></TH></TR>\n", sid->dat->in_ScriptName, call.callid, COLOR_THTEXT, call.callid);
	} else {
		prints(sid, "New Call</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Call Name&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=callname value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", str2html(sid, call.callname));
	if (auth_priv(sid, AUTH_CALLS)&A_ADMIN) {
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:182px'>\n", COLOR_EDITFORM);
		htselect_user(sid, call.assignedto);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Contact&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>\n", COLOR_EDITFORM);
	htselect_contact(sid, call.contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Call Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=calldate2>\n", COLOR_EDITFORM);
	htselect_month(sid, calldate);
	prints(sid, "</SELECT><SELECT NAME=calldate1>\n");
	htselect_day(sid, calldate);
	prints(sid, "</SELECT><SELECT NAME=calldate3>\n");
	htselect_year(sid, 2000, calldate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Call Time&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=calltime style='width:182px'>\n");
	htselect_time(sid, call.calldate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Action&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=action style='width:182px'>\n", COLOR_EDITFORM);
	htselect_callaction(sid, call.action);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:182px'>\n", COLOR_EDITFORM);
	htselect_eventstatus(sid, call.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=5 COLS=40>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, call.details));
	if ((call.obj_uid==sid->dat->user_uid)||(auth_priv(sid, AUTH_CALLS)&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALLS)&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, call.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALLS)&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, call.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", call.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", call.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", call.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", call.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", call.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", call.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	if (callid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save Call'>\n");
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete Call' onClick=\"return ConfirmDelete();\">\n");
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Add Call'>\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.calledit.callname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

/****************************************************************************
 *	callview()
 *
 *	Purpose	: Display call information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void callview(CONNECTION *sid)
{
	REC_CALL call;
	char contactname[50];
	char *ptemp;
	int callid;
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_CALLS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "CALLID"))==NULL) return;
	callid=atoi(ptemp);
	if (db_read(sid, 1, DB_CALLS, callid, &call)!=0) {
		prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", callid);
		return;
	}
	call.calldate+=time_tzoffset(sid, call.calldate);
	memset(contactname, 0, sizeof(contactname));
	if ((sqr=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", call.contactid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		snprintf(contactname, sizeof(contactname)-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 1)));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) strncat(contactname, ", ", sizeof(contactname)-strlen(contactname)-1);
		strncat(contactname, str2html(sid, sqlGetvalue(sqr, 0, 2)), sizeof(contactname)-strlen(contactname)-1);
	}
	sqlFreeconnect(sqr);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>%s", COLOR_TH, COLOR_THTEXT, str2html(sid, call.callname));
	if (auth_priv(sid, AUTH_CALLS)&A_MODIFY) {
		prints(sid, " [<A HREF=%s/calls/edit?callid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, call.callid, COLOR_THLINK);
	}
	prints(sid, "</FONT></TH></TR>\n");
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users"))<0) return;
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned By </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==call.assignedby) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned To </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==call.assignedto) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD></TR>\n");
	sqlFreeconnect(sqr);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Contact Name</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%><A HREF=%s/contacts/view?contactid=%d>%s</A>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, sid->dat->in_ScriptName, call.contactid, contactname);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Date        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%><NOBR>%s&nbsp;</NOBR></TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2datetext(sid, call.calldate));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Time        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2timetext(sid, call.calldate));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Action      </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_callaction(sid, call.action));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Status      </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_eventstatus(sid, call.status));
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><B>Details</B></TD></TR>\n", COLOR_FNAME);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FVAL, str2html(sid, call.details));
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>Notes", COLOR_TH, COLOR_THTEXT);
	prints(sid, " [<A HREF=%s/notes/editnew?table=calls&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, call.callid, COLOR_THLINK);
	prints(sid, "</FONT></TH></TR>\n");
	notessublist(sid, "calls", call.callid, 2);
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void calllist(CONNECTION *sid)
{
	char *ptemp;
	int i;
	int j;
	int offset=0;
	int sqr1;
	int sqr2;
	int total;
	time_t calldate;
	int userid=-1;
	int status;

	if (!(auth_priv(sid, AUTH_CALLS)&A_READ)) {
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
	htselect_callfilter(sid, userid, "/calls/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\n");
	if (status==2) {
		if (auth_priv(sid, AUTH_CALLS)&A_ADMIN) {
			if ((sqr1=sqlQueryf(sid, "SELECT callid, action, contactid, calldate, assignedto FROM gw_calls where assignedto = %d ORDER BY callid DESC", userid))<0) return;
		} else {
			if ((sqr1=sqlQueryf(sid, "SELECT callid, action, contactid, calldate, assignedto FROM gw_calls where assignedto = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callid DESC", userid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (auth_priv(sid, AUTH_CALLS)&A_ADMIN) {
			if ((sqr1=sqlQueryf(sid, "SELECT callid, action, contactid, calldate, assignedto FROM gw_calls where assignedto = %d AND status = %d ORDER BY callid DESC", userid, status))<0) return;
		} else {
			if ((sqr1=sqlQueryf(sid, "SELECT callid, action, contactid, calldate, assignedto FROM gw_calls where assignedto = %d AND status = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callid DESC", userid, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	}
	if ((sqr2=sqlQuery(sid, "SELECT contactid, surname, givenname FROM gw_contacts"))<0) return;
	total=0;
	for (i=0;i<sqlNumtuples(sqr1);i++) {
		if (atoi(sqlGetvalue(sqr1, i, 4))==userid) total++;
	}
	if (total>0) {
		prints(sid, "Found %d matching calls<BR>\n", total);
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400>\n");
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Call ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Action&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Contact&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH><TH ALIGN=LEFT><FONT COLOR=%s>&nbsp;Time&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
		for (i=offset;(i<sqlNumtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
			if (atoi(sqlGetvalue(sqr1, i, 4))!=userid) continue;
			prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/calls/view?callid=%d'\">", COLOR_FVAL, sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%d>%d</A>&nbsp;</TD>\n", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)), atoi(sqlGetvalue(sqr1, i, 0)));
			prints(sid, "<TD NOWRAP WIDTH=100%%>%s&nbsp;</TD>", htview_callaction(sid, atoi(sqlGetvalue(sqr1, i, 1))));
			for (j=0;j<sqlNumtuples(sqr2);j++) {
				if (atoi(sqlGetvalue(sqr2, j, 0))==atoi(sqlGetvalue(sqr1, i, 2))) {
					prints(sid, "<TD NOWRAP><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sqlGetvalue(sqr2, j, 0), str2html(sid, sqlGetvalue(sqr2, j, 1)));
					if (strlen(sqlGetvalue(sqr2, j, 1))&&strlen(sqlGetvalue(sqr2, j, 2))) prints(sid, ", ");
					prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr2, j, 2)));
					break;
				}
			}
			if (j==sqlNumtuples(sqr2)) {
				prints(sid, "<TD NOWRAP>&nbsp;</TD>");
			}
			calldate=time_sql2unix(sqlGetvalue(sqr1, i, 3));
			calldate+=time_tzoffset(sid, calldate);
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2datetext(sid, calldate));
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2timetext(sid, calldate));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
		if (sqlNumtuples(sqr1)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Previous Page</A>]\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, status, userid);
			} else {
				prints(sid, "[Previous Page]\n");
			}
			if (offset+sid->dat->user_maxlist<sqlNumtuples(sqr1)) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Next Page</A>]\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, status, userid);
			} else {
				prints(sid, "[Next Page]\n");
			}
		}
	} else {
		prints(sid, "<B>No calls found</B>\n");
	}
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr2);
	sqlFreeconnect(sqr1);
	return;
}

void callsave(CONNECTION *sid)
{
	REC_CALL call;
	char calldate[40];
	char *ptemp;
	int callid;

	if (!(auth_priv(sid, AUTH_CALLS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "CALLID"))==NULL) return;
	callid=atoi(ptemp);
	if (db_read(sid, 2, DB_CALLS, callid, &call)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, AUTH_CALLS)&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) call.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) call.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_CALLS)&A_ADMIN)||(call.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) call.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) call.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ASSIGNEDBY"))!=NULL) call.assignedby=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ASSIGNEDTO"))!=NULL) call.assignedto=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "CALLNAME"))!=NULL) snprintf(call.callname, sizeof(call.callname)-1, "%s", ptemp);
	memset(calldate, 0, sizeof(calldate));
	if ((ptemp=getpostenv(sid, "CALLDATE3"))!=NULL) snprintf(calldate, sizeof(calldate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLDATE2"))!=NULL) strcatf(calldate, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLDATE1"))!=NULL) strcatf(calldate, "%02d ", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLTIME"))!=NULL) strcatf(calldate, "%s", ptemp);
	if (strlen(calldate)!=19) memset(calldate, 0, sizeof(calldate));
	call.calldate=time_sql2unix(calldate);
	call.calldate-=time_tzoffset(sid, call.calldate);
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) call.contactid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ACTION"))!=NULL) call.action=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) call.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(call.details, sizeof(call.details)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete Call")==0)) {
		if (!(auth_priv(sid, AUTH_CALLS)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_calls WHERE callid = %d", call.callid)<0) return;
		prints(sid, "<BR><CENTER>Call %d deleted successfully</CENTER><BR>\n", call.callid);
		logaccess(sid, 1, "%s - %s deleted call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/list\">\n", sid->dat->in_ScriptName);
	} else if (call.callid==0) {
		if (!(auth_priv(sid, AUTH_CALLS)&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((call.callid=db_write(sid, DB_CALLS, 0, &call))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Call %d added successfully</CENTER><BR>\n", call.callid);
		logaccess(sid, 1, "%s - %s added call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\">\n", sid->dat->in_ScriptName, call.callid);
	} else {
		if (!(auth_priv(sid, AUTH_CALLS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (db_write(sid, DB_CALLS, callid, &call)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Call %d modified successfully</CENTER><BR>\n", call.callid);
		logaccess(sid, 1, "%s - %s modified call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\">\n", sid->dat->in_ScriptName, call.callid);
	}
	return;
}

void callsmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_CALLS);
	if (strncmp(sid->dat->in_RequestURI, "/calls/edit", 11)==0) {
		calledit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/calls/view", 11)==0) {
		callview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/calls/list", 11)==0) {
		calllist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/calls/save", 11)==0) {
		callsave(sid);
	}
	htpage_footer(sid);
	return;
}
