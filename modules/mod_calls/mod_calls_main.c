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
#include "mod_calls.h"

void htselect_callaction(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT callactionid, actionname FROM gw_callactions ORDER BY actionname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_callfilter(CONN *sid, int selected, char *baseuri)
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
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints(sid, "<FORM METHOD=GET NAME=callfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.callfilter.userid.options[document.callfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\n", atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.callfilter.status.options[document.callfilter.status.selectedIndex].value\r\n");
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

void calledit(CONN *sid)
{
	REC_CALL call;
	char callstart[16];
	char callfinish[16];
	char *ptemp;
	int calltime;
	int callid;
	int duration;
	int editperms=0;
	int temp;
	int i;

	if (!(auth_priv(sid, "calls")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/calls/editnew", 14)==0) {
		callid=0;
		if (dbread_call(sid, 2, 0, &call)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) call.contactid=atoi(ptemp);
	} else {
		if (getgetenv(sid, "CALLID")==NULL) return;
		callid=atoi(getgetenv(sid, "CALLID"));
		if (dbread_call(sid, 2, callid, &call)!=0) {
			prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", callid);
			return;
		}
	}
	call.callstart+=time_tzoffset(sid, call.callstart);
	call.callfinish+=time_tzoffset(sid, call.callfinish);
	strftime(callstart, sizeof(callstart)-1, "%Y-%m-%d", gmtime(&call.callstart));
	strftime(callfinish, sizeof(callfinish)-1, "%Y-%m-%d", gmtime(&call.callfinish));
	prints(sid, "<BR>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ContactView() {\r\n");
	prints(sid, "	var contactid=document.calledit.contactid.value;\r\n");
	prints(sid, "	if (contactid<1) {\r\n");
	prints(sid, "		window.open('%s/contacts/search1','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	} else {\r\n");
	prints(sid, "		window.open('%s/contacts/view?contactid='+contactid,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (callid>0) {
		prints(sid, "<B><A HREF=%s/calls/view?callid=%d>Call Number %d</A></B>\n", sid->dat->in_ScriptName, call.callid, call.callid);
	} else {
		prints(sid, "<B>New Call</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/calls/save NAME=calledit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=callid VALUE='%d'>\n", call.callid);
	prints(sid, "<INPUT TYPE=hidden NAME=callstart VALUE='%d'>\n", call.callstart);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR BGCOLOR=\"%s\">\n", config->colour_fieldname);
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>CALL INFO</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>DETAILS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD VALIGN=TOP STYLE='padding:3px'>", config->colour_editform);
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Call Name&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=callname value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", str2html(sid, call.callname), (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:255px'%s>\n", config->colour_editform, (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, call.assignedto);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;<A HREF=javascript:ContactView() STYLE='color: %s'>Contact Name</A>&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:255px'%s>\n", config->colour_editform, config->colour_fieldnametext, (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	htselect_contact(sid, call.contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Action&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=action style='width:255px'%s>\n", config->colour_editform, (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	htselect_callaction(sid, call.action);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Call Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=callstart2 style='width:128px'%s>\n", config->colour_editform, auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_month(sid, callstart);
	prints(sid, "</SELECT><SELECT NAME=callstart1 style='width:64px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_day(sid, callstart);
	prints(sid, "</SELECT><SELECT NAME=callstart3 style='width:63px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, callstart);
	prints(sid, "</SELECT></TD></TR>\n");
	calltime=call.callstart%86400;
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Call Start&nbsp;</B></TD><TD ALIGN=RIGHT>", config->colour_editform);
	prints(sid, "<SELECT NAME=callstart4 style='width:64px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime/3600;
	if (temp>12) temp-=12;
	if (temp==0) temp=12;
	for (i=1;i<13;i++) {
		prints(sid, "<OPTION VALUE='%d'%s%s>%s%d:\n", i, i==temp?" SELECTED":"", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED", i<10?"&nbsp;":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart5 style='width:64px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime%3600/60;
	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%02d:\n", i, i==temp?" SELECTED":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart6 style='width:64px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime%60;
	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%02d\n", i, i==temp?" SELECTED":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart7 style='width:63px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	prints(sid, "<OPTION VALUE='am'%s>AM\n", calltime/3600<12?" SELECTED":"");
	prints(sid, "<OPTION VALUE='pm'%s>PM\n", calltime/3600>=12?" SELECTED":"");
	prints(sid, "</SELECT>\n");
	prints(sid, "</TD></TR>\n");
	if ((!(auth_priv(sid, "calls")&A_ADMIN))&&(call.status==0)) {
		call.callfinish=time(NULL);
		call.callfinish+=time_tzoffset(sid, call.callfinish);
	}
	duration=call.callfinish-call.callstart;
	if (duration<0) duration=0;
	duration/=60;
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Duration&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=duration1 style='width:128px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_qhours(sid, duration/60);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=duration2 style='width:127px'%s>\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_minutes(sid, duration%60);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:255px'%s>\n", config->colour_editform, auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_eventstatus(sid, call.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=\"%s\"><TD ALIGN=CENTER><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50%s>%s</TEXTAREA></TD></TR>\n", config->colour_editform, (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED", str2html(sid, call.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if ((call.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "calls")&A_ADMIN)) editperms=1;
	prints(sid, "<TR BGCOLOR=\"%s\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, call.obj_uid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, call.obj_gid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\n", call.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\n", call.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\n", call.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\n", call.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\n", call.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\n", call.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	if (callid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
		if (call.status==0) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save & Close'>\n");
		}
		if (auth_priv(sid, "calls")&A_DELETE) {
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
	if ((auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)) {
		prints(sid, "document.calledit.callname.focus();\n");
	}
	prints(sid, "showpage(1);\n");
	if ((!(auth_priv(sid, "calls")&A_ADMIN))&&(call.status==0)) {
		prints(sid, "var x=document.calledit.duration2.value-1;\n");
		prints(sid, "function setDuration() {\n");
		prints(sid, "	x++;\n");
		prints(sid, "	if (x>59) {\n");
		prints(sid, "		if (document.calledit.duration1.value<23) {\n");
		prints(sid, "			document.calledit.duration1.value++;\n");
		prints(sid, "		}\n");
		prints(sid, "		x=0;\n");
		prints(sid, "	}\n");
		prints(sid, "	document.calledit.duration2.value=x;\n");
		prints(sid, "	setTimeout(\"setDuration()\", 60000);\n");
		prints(sid, "}\n");
		prints(sid, "setDuration();\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
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
void callview(CONN *sid)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_CALL call;
	char contactname[50];
	char *ptemp;
	int callid;
	int i;
	int sqr;

	if (!(auth_priv(sid, "calls")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "CALLID"))==NULL) return;
	callid=atoi(ptemp);
	if (dbread_call(sid, 1, callid, &call)!=0) {
		prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", callid);
		return;
	}
	call.callstart+=time_tzoffset(sid, call.callstart);
	call.callfinish+=time_tzoffset(sid, call.callfinish);
	memset(contactname, 0, sizeof(contactname));
	if ((sqr=sql_queryf("SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", call.contactid))<0) return;
	if (sql_numtuples(sqr)>0) {
		snprintf(contactname, sizeof(contactname)-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
		if (strlen(sql_getvalue(sqr, 0, 1))&&strlen(sql_getvalue(sqr, 0, 2))) strncat(contactname, ", ", sizeof(contactname)-strlen(contactname)-1);
		strncat(contactname, str2html(sid, sql_getvalue(sqr, 0, 2)), sizeof(contactname)-strlen(contactname)-1);
	}
	sql_freeresult(sqr);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>%s", config->colour_th, config->colour_thtext, str2html(sid, call.callname));
	if (auth_priv(sid, "calls")&A_MODIFY) {
		if (auth_priv(sid, "calls")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/calls/edit?callid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, call.callid, config->colour_thlink);
		} else if ((call.assignedby==sid->dat->user_uid)||(call.assignedto==sid->dat->user_uid)||(call.obj_uid==sid->dat->user_uid)||((call.obj_gid==sid->dat->user_gid)&&(call.obj_gperm>=2))||(call.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/calls/edit?callid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, call.callid, config->colour_thlink);
		}
	}
	prints(sid, "</FONT></TH></TR>\n");
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users"))<0) return;
	prints(sid, "<TR><TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Assigned By </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>", config->colour_fieldname, config->colour_fieldval);
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==call.assignedby) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</NOBR></TD>\n");
	prints(sid, "<TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Call Date       </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2datetext(sid, call.callstart));
	prints(sid, "<TR><TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Assigned To </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>", config->colour_fieldname, config->colour_fieldval);
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==call.assignedto) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	sql_freeresult(sqr);
	prints(sid, "    <TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Call Start  </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2timetext(sid, call.callstart));
	prints(sid, "<TR><TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Contact Name</B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%d>%s</A>&nbsp;</TD>\n", config->colour_fieldname, config->colour_fieldval, sid->dat->in_ScriptName, call.contactid, contactname);
	prints(sid, "    <TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Call Finish </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2timetext(sid, call.callfinish));
	prints(sid, "<TR><TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Action      </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD>\n", config->colour_fieldname, config->colour_fieldval, htview_callaction(sid, call.action));
	prints(sid, "    <TD BGCOLOR=\"%s\" NOWRAP STYLE='border-style:solid'><B>Status      </B></TD><TD BGCOLOR=\"%s\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, htview_eventstatus(sid, call.status));
	prints(sid, "<TR><TD BGCOLOR=\"%s\" COLSPAN=4 STYLE='border-style:solid'><B>Details</B></TD></TR>\n", config->colour_fieldname);
	prints(sid, "<TR><TD BGCOLOR=\"%s\" COLSPAN=4 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", config->colour_fieldval, str2html(sid, call.details));
	if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR BGCOLOR=\"%s\"><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Notes", config->colour_th, config->colour_thtext);
		prints(sid, " [<A HREF=%s/notes/editnew?table=calls&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, call.callid, config->colour_thlink);
		prints(sid, "</FONT></TH></TR>\n");
		mod_notes_sublist(sid, "calls", call.callid, 4);
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void calllist(CONN *sid)
{
	char *ptemp;
	int i;
	int j;
	int offset=0;
	int sqr1;
	int sqr2;
	int total;
	int userid=-1;
	int status;
	int duration;
	time_t callstart;

	if (!(auth_priv(sid, "calls")&A_READ)) {
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
		if (auth_priv(sid, "calls")&A_ADMIN) {
			if ((sqr1=sql_queryf("SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d ORDER BY callid DESC", userid))<0) return;
		} else {
			if ((sqr1=sql_queryf("SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callid DESC", userid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (auth_priv(sid, "calls")&A_ADMIN) {
			if ((sqr1=sql_queryf("SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND status = %d ORDER BY callid DESC", userid, status))<0) return;
		} else {
			if ((sqr1=sql_queryf("SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND status = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callid DESC", userid, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	}
	if ((sqr2=sql_query("SELECT contactid, surname, givenname FROM gw_contacts"))<0) return;
	total=0;
	for (i=0;i<sql_numtuples(sqr1);i++) {
		if (atoi(sql_getvalue(sqr1, i, 6))==userid) total++;
	}
	if (total>0) {
		prints(sid, "Found %d matching calls<BR>\n", total);
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR BGCOLOR=\"%s\"><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Call ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Action&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Contact&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH><TH ALIGN=LEFT STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Time&nbsp;</FONT></TH><TH ALIGN=LEFT STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Duration&nbsp;</FONT></TH>", config->colour_th, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
		if (status==2) {
			prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Status&nbsp;</FONT></TH>", config->colour_thtext);
		}
		prints(sid, "</TR>\n");
		for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
			if (atoi(sql_getvalue(sqr1, i, 6))!=userid) continue;
			prints(sid, "<TR BGCOLOR=\"%s\"><TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' ", config->colour_fieldval);
			if ((userid==sid->dat->user_uid)&&(atoi(sql_getvalue(sqr1, i, 5))==0)) {
				prints(sid, "onClick=\"window.location.href='%s/calls/edit?callid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/calls/edit?callid=%d>%d</A>&nbsp;</TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), atoi(sql_getvalue(sqr1, i, 0)));
			} else {
				prints(sid, "onClick=\"window.location.href='%s/calls/view?callid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
				prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%d>%d</A>&nbsp;</TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), atoi(sql_getvalue(sqr1, i, 0)));
			}
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", htview_callaction(sid, atoi(sql_getvalue(sqr1, i, 1))));
			for (j=0;j<sql_numtuples(sqr2);j++) {
				if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 2))) {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(sqr2, j, 0), str2html(sid, sql_getvalue(sqr2, j, 1)));
					if (strlen(sql_getvalue(sqr2, j, 1))&&strlen(sql_getvalue(sqr2, j, 2))) prints(sid, ", ");
					prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr2, j, 2)));
					break;
				}
			}
			if (j==sql_numtuples(sqr2)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
			}
			callstart=time_sql2unix(sql_getvalue(sqr1, i, 3));
			callstart+=time_tzoffset(sid, callstart);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, callstart));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2timetext(sid, callstart));
			duration=time_sql2unix(sql_getvalue(sqr1, i, 4))-time_sql2unix(sql_getvalue(sqr1, i, 3));
			if (duration<0) duration=0;
			duration/=60;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%d Minutes</TD>", duration);
			if (status==2) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(sql_getvalue(sqr1, i, 5))==1?"Closed":"Open");
			}
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
		if (sql_numtuples(sqr1)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Previous Page</A>]\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, status, userid);
			} else {
				prints(sid, "[Previous Page]\n");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Next Page</A>]\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, status, userid);
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
		prints(sid, " calls found</B>\n");
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr2);
	sql_freeresult(sqr1);
	return;
}

void callsave(CONN *sid)
{
	REC_CALL call;
	char callstart[40];
	char *ptemp;
	int callid;
	int duration;
	int calltime=0;

	if (!(auth_priv(sid, "calls")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "CALLID"))==NULL) return;
	callid=atoi(ptemp);
	if (dbread_call(sid, 2, callid, &call)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "calls")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) call.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) call.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "calls")&A_ADMIN)||(call.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) call.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) call.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ASSIGNEDBY"))!=NULL) call.assignedby=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ASSIGNEDTO"))!=NULL) call.assignedto=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "CALLNAME"))!=NULL) snprintf(call.callname, sizeof(call.callname)-1, "%s", ptemp);
	memset(callstart, 0, sizeof(callstart));
	if ((ptemp=getpostenv(sid, "CALLSTART3"))!=NULL) snprintf(callstart, sizeof(callstart)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLSTART2"))!=NULL) strncatf(callstart, sizeof(callstart)-strlen(callstart)-1, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLSTART1"))!=NULL) strncatf(callstart, sizeof(callstart)-strlen(callstart)-1, "%02d ", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLSTART4"))!=NULL) calltime=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "CALLSTART7"))!=NULL) {
		if (strcasecmp(ptemp, "pm")==0) {
			if (calltime<12) calltime+=12;
		} else if (strcasecmp(ptemp, "am")==0) {
			if (calltime==12) calltime=0;
		}
	}
	strncatf(callstart, sizeof(callstart)-strlen(callstart)-1, "%02d:", calltime);
	if ((ptemp=getpostenv(sid, "CALLSTART5"))!=NULL) strncatf(callstart, sizeof(callstart)-strlen(callstart)-1, "%02d:", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLSTART6"))!=NULL) strncatf(callstart, sizeof(callstart)-strlen(callstart)-1, "%02d", atoi(ptemp));
	if (strlen(callstart)==19) {
		call.callstart=time_sql2unix(callstart);
	} else {
		memset(callstart, 0, sizeof(callstart));
		if ((ptemp=getpostenv(sid, "CALLSTART"))!=NULL) call.callstart=atoi(ptemp);
	}
	duration=0;
	if ((ptemp=getpostenv(sid, "DURATION1"))!=NULL) duration+=atoi(ptemp)*3600;
	if ((ptemp=getpostenv(sid, "DURATION2"))!=NULL) duration+=atoi(ptemp)*60;
	call.callfinish=call.callstart;
	if (duration>0) call.callfinish+=duration;
	call.callstart-=time_tzoffset(sid, call.callstart);
	call.callfinish-=time_tzoffset(sid, call.callfinish);
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) call.contactid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ACTION"))!=NULL) call.action=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) call.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(call.details, sizeof(call.details)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "calls")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_calls WHERE callid = %d", call.callid)<0) return;
		prints(sid, "<BR><CENTER>Call %d deleted successfully</CENTER><BR>\n", call.callid);
		db_log_activity(sid, 1, "calls", call.callid, "delete", "%s - %s deleted call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/list\">\n", sid->dat->in_ScriptName);
	} else if (call.callid==0) {
		if (!(auth_priv(sid, "calls")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((call.callid=dbwrite_call(sid, 0, &call))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Call %d added successfully</CENTER><BR>\n", call.callid);
		db_log_activity(sid, 1, "calls", call.callid, "insert", "%s - %s added call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\">\n", sid->dat->in_ScriptName, call.callid);
	} else {
		if (!(auth_priv(sid, "calls")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Save & Close")==0)) {
			call.callfinish=time(NULL);
			call.status=1;
		}
		if (dbwrite_call(sid, callid, &call)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<BR><CENTER>Call %d modified successfully</CENTER><BR>\n", call.callid);
		db_log_activity(sid, 1, "calls", call.callid, "modify", "%s - %s modified call %d", sid->dat->in_RemoteAddr, sid->dat->user_username, call.callid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\">\n", sid->dat->in_ScriptName, call.callid);
	}
	return;
}

void mod_main(CONN *sid)
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

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_calls",		// mod_name
		1,			// mod_submenu
		"CALLS",		// mod_menuname
		"/calls/list",		// mod_menuuri
		"calls",		// mod_menuperm
		"mod_main",		// fn_name
		"/calls/",		// fn_uri
		mod_main		// fn_ptr
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
