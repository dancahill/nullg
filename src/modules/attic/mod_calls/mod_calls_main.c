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
#include "mod_calls.h"

void htselect_callaction(CONN *sid, int selected)
{
/*	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT actionid, actionname FROM gw_calls_actions ORDER BY actionname ASC")<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
*/	return;
}

void htselect_callfilter(CONN *sid, int selected, char *baseuri)
{
/*	char *option[]={ "Open", "Closed", "All" };
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
	prints(sid, "<FORM METHOD=GET NAME=callfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.callfilter.userid.options[document.callfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\r\n", atoi(sql_getvalue(&sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.callfilter.status.options[document.callfilter.status.selectedIndex].value\r\n");
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
*/	return;
}

void calledit(CONN *sid)
{
/*	REC_CALL call;
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
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/calls/editnew", 14)==0) {
		callid=0;
		if (dbread_call(sid, 2, 0, &call)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) call.contactid=atoi(ptemp);
	} else {
		if (getgetenv(sid, "CALLID")==NULL) return;
		callid=atoi(getgetenv(sid, "CALLID"));
		if (dbread_call(sid, 2, callid, &call)!=0) {
			prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", callid);
			return;
		}
	}
	call.callstart+=time_tzoffset(sid, call.callstart);
	call.callfinish+=time_tzoffset(sid, call.callfinish);
	strftime(callstart, sizeof(callstart)-1, "%Y-%m-%d", gmtime(&call.callstart));
	strftime(callfinish, sizeof(callfinish)-1, "%Y-%m-%d", gmtime(&call.callfinish));
	prints(sid, "<BR />\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ContactView() {\r\n");
	prints(sid, "	var contactid=document.calledit.contactid.value;\r\n");
	prints(sid, "	if (contactid<1) {\r\n");
	prints(sid, "		window.open('%s/contacts/search1','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	} else {\r\n");
	prints(sid, "		window.open('%s/contacts/view?contactid='+contactid,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (callid>0) {
		prints(sid, "<B><A HREF=%s/calls/view?callid=%d>Call Number %d</A></B>\r\n", sid->dat->in_ScriptName, call.callid, call.callid);
	} else {
		prints(sid, "<B>New Call</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/calls/save NAME=calledit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=callid VALUE='%d'>\r\n", call.callid);
	prints(sid, "<INPUT TYPE=hidden NAME=callstart VALUE='%d'>\r\n", call.callstart);
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
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Call Name&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=callname value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, call.callname), (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:255px'%s>\r\n", (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, call.assignedto, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<A HREF=javascript:ContactView() CLASS=\"EDITFORM\">Contact Name</A>&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:255px'%s>\r\n", (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	htselect_contact(sid, call.contactid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Action&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=action style='width:255px'%s>\r\n", (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED");
	htselect_callaction(sid, call.action);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Call Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=callstart2 style='width:128px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_month(sid, callstart);
	prints(sid, "</SELECT><SELECT NAME=callstart1 style='width:64px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_day(sid, callstart);
	prints(sid, "</SELECT><SELECT NAME=callstart3 style='width:63px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_year(sid, 2000, callstart);
	prints(sid, "</SELECT></TD></TR>\r\n");
	calltime=call.callstart%86400;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Call Start&nbsp;</B></TD><TD ALIGN=RIGHT>");
	prints(sid, "<SELECT NAME=callstart4 style='width:64px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime/3600;
	if (temp>12) temp-=12;
	if (temp==0) temp=12;
	for (i=1;i<13;i++) {
		prints(sid, "<OPTION VALUE='%d'%s%s>%s%d:\r\n", i, i==temp?" SELECTED":"", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED", i<10?"&nbsp;":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart5 style='width:64px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime%3600/60;
	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%02d:\r\n", i, i==temp?" SELECTED":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart6 style='width:64px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	temp=calltime%60;
	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%02d\r\n", i, i==temp?" SELECTED":"", i);
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=callstart7 style='width:63px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	prints(sid, "<OPTION VALUE='am'%s>AM\r\n", calltime/3600<12?" SELECTED":"");
	prints(sid, "<OPTION VALUE='pm'%s>PM\r\n", calltime/3600>=12?" SELECTED":"");
	prints(sid, "</SELECT>\r\n");
	prints(sid, "</TD></TR>\r\n");
	if ((!(auth_priv(sid, "calls")&A_ADMIN))&&(call.status==0)) {
		call.callfinish=time(NULL);
		call.callfinish+=time_tzoffset(sid, call.callfinish);
	}
	duration=call.callfinish-call.callstart;
	if (duration<0) duration=0;
	duration/=60;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Duration&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=duration1 style='width:128px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_qhours(sid, duration/60);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=duration2 style='width:127px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_minutes(sid, duration%60);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:255px'%s>\r\n", auth_priv(sid, "calls")&A_ADMIN?"":" DISABLED");
	htselect_eventstatus(sid, call.status);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50%s>%s</TEXTAREA></TD></TR>\r\n", (auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)?"":" DISABLED", str2html(sid, call.details));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((call.obj_uid==sid->dat->uid)||(auth_priv(sid, "calls")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, call.obj_uid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "calls")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "calls"), call.obj_gid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", call.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", call.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", call.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", call.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", call.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", call.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (callid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
		if (call.status==0) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save & Close'>\r\n");
		}
		if (auth_priv(sid, "calls")&A_DELETE) {
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
	if ((auth_priv(sid, "calls")&A_ADMIN)||(call.status==0)) {
		prints(sid, "document.calledit.callname.focus();\r\n");
	}
	prints(sid, "showpage(1);\r\n");
	if ((!(auth_priv(sid, "calls")&A_ADMIN))&&(call.status==0)) {
		prints(sid, "var x=document.calledit.duration2.value-1;\r\n");
		prints(sid, "function setDuration() {\r\n");
		prints(sid, "	x++;\r\n");
		prints(sid, "	if (x>59) {\r\n");
		prints(sid, "		if (document.calledit.duration1.value<23) {\r\n");
		prints(sid, "			document.calledit.duration1.value++;\r\n");
		prints(sid, "		}\r\n");
		prints(sid, "		x=0;\r\n");
		prints(sid, "	}\r\n");
		prints(sid, "	document.calledit.duration2.value=x;\r\n");
		prints(sid, "	setTimeout(\"setDuration()\", 60000);\r\n");
		prints(sid, "}\r\n");
		prints(sid, "setDuration();\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
*/	return;
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
/*	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_CALL call;
	char contactname[50];
	char *ptemp;
	int callid;
	int i;
	SQLRES sqr;

	if (!(auth_priv(sid, "calls")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "CALLID"))==NULL) return;
	callid=atoi(ptemp);
	if (dbread_call(sid, 1, callid, &call)!=0) {
		prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", callid);
		return;
	}
	call.callstart+=time_tzoffset(sid, call.callstart);
	call.callfinish+=time_tzoffset(sid, call.callfinish);
	memset(contactname, 0, sizeof(contactname));
	if (sql_queryf(&sqr, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d AND obj_did = %d", call.contactid, sid->dat->did)<0) return;
	if (sql_numtuples(&sqr)>0) {
		snprintf(contactname, sizeof(contactname)-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 1)));
		if (strlen(sql_getvalue(&sqr, 0, 1))&&strlen(sql_getvalue(&sqr, 0, 2))) strncat(contactname, ", ", sizeof(contactname)-strlen(contactname)-1);
		strncat(contactname, str2html(sid, sql_getvalue(&sqr, 0, 2)), sizeof(contactname)-strlen(contactname)-1);
	}
	sql_freeresult(&sqr);
	prints(sid, "<BR />\r\n");
	prints(sid, "<CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>%s", str2html(sid, call.callname));
	if (auth_priv(sid, "calls")&A_MODIFY) {
		if (auth_priv(sid, "calls")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/calls/edit?callid=%d>edit</A>]", sid->dat->in_ScriptName, call.callid);
		} else if ((call.assignedby==sid->dat->uid)||(call.assignedto==sid->dat->uid)||(call.obj_uid==sid->dat->uid)||((call.obj_gid==sid->dat->gid)&&(call.obj_gperm>=2))||(call.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/calls/edit?callid=%d>edit</A>]", sid->dat->in_ScriptName, call.callid);
		}
	}
	prints(sid, "</TH></TR>\r\n");
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE obj_did = %d", sid->dat->did)<0) return;
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned By </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		if (atoi(sql_getvalue(&sqr, i, 0))==call.assignedby) {
			prints(sid, "%s", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</NOBR></TD>\r\n");
	prints(sid, "<TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Call Date       </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", time_unix2datetext(sid, call.callstart));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned To </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		if (atoi(sql_getvalue(&sqr, i, 0))==call.assignedto) {
			prints(sid, "%s", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\r\n");
	sql_freeresult(&sqr);
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Call Start  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", time_unix2timetext(sid, call.callstart));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Contact Name</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%d>%s</A>&nbsp;</TD>\r\n", sid->dat->in_ScriptName, call.contactid, contactname);
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Call Finish </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\r\n", time_unix2timetext(sid, call.callfinish));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Action      </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD>\r\n", htview_callaction(sid, call.action));
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Status      </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=50%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", htview_eventstatus(call.status));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=4 STYLE='border-style:solid'><B>Details</B></TD></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=4 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\r\n", str2html(sid, call.details));
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=calls&index=%d>new</A>]", sid->dat->in_ScriptName, call.callid);
		prints(sid, "</TH></TR>\r\n");
		mod_notes_sublist(sid, "calls", call.callid, 4);
	}
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
*/
}

void calllist(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int i;
//	int j;
	int offset=0;
//	SQLRES sqr1;
//	SQLRES sqr2;
	int total;
	int userid=-1;
	int status;
	int duration;
	time_t callstart;
	int id;

	if (!(auth_priv(sid, "calls")&A_READ)) {
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
	htselect_callfilter(sid, userid, "/calls/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\r\n");
/*	if (status==2) {
		if (auth_priv(sid, "calls")&A_ADMIN) {
			if (sql_queryf(&sqr1, "SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND obj_did = %d ORDER BY callid DESC", userid, sid->dat->did)<0) return;
		} else {
			if (sql_queryf(&sqr1, "SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY callid DESC", userid, sid->dat->uid, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		}
	} else {
		if (auth_priv(sid, "calls")&A_ADMIN) {
			if (sql_queryf(&sqr1, "SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND status = %d AND obj_did = %d ORDER BY callid DESC", userid, status, sid->dat->did)<0) return;
		} else {
			if (sql_queryf(&sqr1, "SELECT callid, action, contactid, callstart, callfinish, status, assignedto FROM gw_calls where assignedto = %d AND status = %d AND (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY callid DESC", userid, status, sid->dat->uid, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		}
	}*/
	if ((qobj=ldir_getlist(sid->N, "call", 0, sid->dat->did))==NULL) return;
//	if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname FROM gw_contacts WHERE obj_did = %d", sid->dat->did)<0) return;
	total=0;
	for (i=0;i<ldir_numentries(&qobj);i++) {
//		if (atoi(ldir_getval(&qobj, i, "assignedto"))!=userid) continue;
		total++;
	}
	if (total>0) {
		prints(sid, "Found %d matching calls<BR />\r\n", total);
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Call ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Action&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Contact&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Time&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Duration&nbsp;</TH>");
		if (status==2) {
			prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Status&nbsp;</TH>");
		}
		prints(sid, "</TR>\r\n");
//		for (i=offset;(i<sql_numtuples(&sqr1))&&(i<offset+sid->dat->maxlist);i++) {
		for (i=offset;(i<ldir_numentries(&qobj))&&(i<offset+sid->dat->maxlist);i++) {
//			if (atoi(sql_getvalue(&sqr1, i, 6))!=userid) continue;
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' ");
			id=atoi(ldir_getval(&qobj, i, "id"));
			if ((userid==sid->dat->uid)&&(atoi(ldir_getval(&qobj, i, "status"))==0)) {
				prints(sid, "onClick=\"window.location.href='%s/calls/edit?callid=%d'\">", ScriptName, id);
				prints(sid, "&nbsp;<A HREF=%s/calls/edit?callid=%d>%d</A>&nbsp;</TD>\r\n", ScriptName, id, id);
			} else {
				prints(sid, "onClick=\"window.location.href='%s/calls/view?callid=%d'\">", ScriptName, id);
				prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%d>%d</A>&nbsp;</TD>\r\n", ScriptName, id, id);
			}
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", htview_callaction(sid, atoi(ldir_getval(&qobj, i, "action"))));
/*			for (j=0;j<sql_numtuples(&sqr2);j++) {
				if (atoi(sql_getvalue(&sqr2, j, 0))==atoi(sql_getvalue(&sqr1, i, 2))) {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(&sqr2, j, 0), str2html(sid, sql_getvalue(&sqr2, j, 1)));
					if (strlen(sql_getvalue(&sqr2, j, 1))&&strlen(sql_getvalue(&sqr2, j, 2))) prints(sid, ", ");
					prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr2, j, 2)));
					break;
				}
			}
			if (j==sql_numtuples(&sqr2)) {*/
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
//			}
			callstart=time_sql2unix(ldir_getval(&qobj, i, "callstart"));
			callstart+=time_tzoffset(sid, callstart);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, callstart));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2timetext(sid, callstart));
			duration=time_sql2unix(ldir_getval(&qobj, i, "callfinish"))-time_sql2unix(ldir_getval(&qobj, i, "callstart"));
			if (duration<0) duration=0;
			duration/=60;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%d Minutes</TD>", duration);
			if (status==2) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(ldir_getval(&qobj, i, "status"))==1?"Closed":"Open");
			}
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
		if (ldir_numentries(&qobj)>sid->dat->maxlist) {
			if (offset>sid->dat->maxlist-1) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Previous Page</A>]\r\n", ScriptName, offset-sid->dat->maxlist, status, userid);
			} else {
				prints(sid, "[Previous Page]\r\n");
			}
			if (offset+sid->dat->maxlist<ldir_numentries(&qobj)) {
				prints(sid, "[<A HREF=%s/calls/list?offset=%d&status=%d&userid=%d>Next Page</A>]\r\n", ScriptName, offset+sid->dat->maxlist, status, userid);
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
		prints(sid, " calls found</B>\r\n");
	}
	prints(sid, "</CENTER>\r\n");
//	sql_freeresult(&sqr2);
//	sql_freeresult(&sqr1);
	ldir_freeresult(&qobj);
	return;
}

void callsave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char callstart[40];
	int duration;
	int calltime=0;
	char *ptemp;
	int id, pid;
	int priv=auth_priv(sid, "calls");

	prints(sid, "<BR />\r\n");
	if (!(priv&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "CALLID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_call(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (auth_priv(sid, "calls")&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) call.obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) call.obj_gid=atoi(ptemp);
//	}
//	if ((auth_priv(sid, "calls")&A_ADMIN)||(call.obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) call.obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) call.obj_operm=atoi(ptemp);
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
	if ((ptemp=getpostenv(sid, "ASSIGNEDBY"))!=NULL) nes_setnum(sid->N, tobj, "assignedby", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "ASSIGNEDTO"))!=NULL) nes_setnum(sid->N, tobj, "assignedto", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "CALLNAME"))!=NULL)   nes_setstr(sid->N, tobj, "callname", ptemp, strlen(ptemp));
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
		nes_setstr(sid->N, tobj, "callstart", callstart, strlen(callstart));
	} else {
		memset(callstart, 0, sizeof(callstart));
		if ((ptemp=getpostenv(sid, "CALLSTART"))!=NULL) nes_setstr(sid->N, tobj, "callstart", ptemp, strlen(ptemp));
	}
	duration=0;
	if ((ptemp=getpostenv(sid, "DURATION1"))!=NULL) duration+=atoi(ptemp)*3600;
	if ((ptemp=getpostenv(sid, "DURATION2"))!=NULL) duration+=atoi(ptemp)*60;
//	call.callfinish=call.callstart;
//	if (duration>0) call.callfinish+=duration;
//	call.callstart-=time_tzoffset(sid, call.callstart);
//	call.callfinish-=time_tzoffset(sid, call.callfinish);
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) nes_setnum(sid->N, tobj, "contactid", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "ACTION"))!=NULL)    nes_setnum(sid->N, tobj, "action", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL)    nes_setnum(sid->N, tobj, "status", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL)   nes_setstr(sid->N, tobj, "details", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(priv&A_DELETE)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (ldir_deleteentry("call", id, sid->dat->did)<0) goto cleanup;
		prints(sid, "<BR /><CENTER>Call %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "calls", id, "delete", "%s - %s deleted call %d", sid->socket.RemoteAddr, sid->dat->username, id);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/calls/list\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/list\"></NOSCRIPT>\r\n", ScriptName);
		goto cleanup;
	} else if (id==0) {
		if (!(priv&A_INSERT)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_call(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Call %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "calls", id, "insert", "%s - %s added call %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Save & Close")==0)) {
			time_unix2sql(callstart, sizeof(callstart)-1, time(NULL));
			nes_setstr(sid->N, tobj, "callfinish", callstart, 10);
			nes_setnum(sid->N, tobj, "status", 1);
		}
		if ((id=dbwrite_call(sid, id, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>Call %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "calls", id, "modify", "%s - %s modified call %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/calls/view?callid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\"></NOSCRIPT>\r\n", ScriptName, id);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_CALLS);
	if (strncmp(RequestURI, "/calls/edit", 11)==0) {
		calledit(sid);
	} else if (strncmp(RequestURI, "/calls/view", 11)==0) {
		callview(sid);
	} else if (strncmp(RequestURI, "/calls/list", 11)==0) {
		calllist(sid);
	} else if (strncmp(RequestURI, "/calls/save", 11)==0) {
		callsave(sid);
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
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "calls");
	nes_exec(proc->N, "GWMODULES.calls={submenu=1,name='CALLS',pic='default.png',uri='/calls/list',perm='calls',fn_name='mod_main',fn_uri='/calls/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
