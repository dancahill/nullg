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
#include "mod_profile.h"

void htselect_layout(CONN *sid, int selected)
{
	char *option[]={ "Simple (Frameless)", "Standard", "Outlook" };
	int i;

	for (i=0;i<3;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_mailaccount(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by mailaccountid ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void profileedit(CONN *sid)
{
	REC_USER user;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (dbread_profile(sid, 2, sid->dat->user_uid, &user)!=0) {
		prints(sid, "<CENTER>Profile information not found</CENTER>\n");
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/save NAME=profileedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='padding:2px'>User Profile for %s</TH></TR>\n", sid->dat->user_username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password             </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password  VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user.password));
	if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Start       </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<SELECT NAME=prefdaystart style='width:255px'>\n");
		htselect_hour(sid, user.prefdaystart);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Length      </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<SELECT NAME=prefdaylength style='width:255px'>\n");
		htselect_number(sid, user.prefdaylength, 0, 24, 1);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<SELECT NAME=prefgeozone style='width:255px'>\n");
		htselect_zone(sid, user.prefgeozone, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Current Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<SELECT NAME=prefmailcurrent style='width:255px'>\n");
		htselect_mailaccount(sid, user.prefmailcurrent);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Default Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=prefmaildefault style='width:255px'>\n");
	prints(sid, "<OPTION VALUE=0%s>External Mail Client\n", user.prefmaildefault!=1?" SELECTED":"");
	if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
		prints(sid, "<OPTION VALUE=1%s>Web E-Mail\n", user.prefmaildefault==1?" SELECTED":"");
	}
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Maximum Results/Page</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=prefmaxlist style='width:255px'>\n");
	htselect_number(sid, user.prefmaxlist, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Menu and Frame Style</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=prefmenustyle style='width:255px'>\n");
	htselect_layout(sid, user.prefmenustyle);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone           </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=preftimezone style='width:255px'>\n");
	htselect_timezone(sid, user.preftimezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD></TR>");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
		prints(sid, "[<A HREF=%s/mail/accounts/list>Edit Mail Accounts</A>]\n", sid->dat->in_ScriptName);
	}
	if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "[<A HREF=%s/profile/timeedit>Edit Availability</A>]\n", sid->dat->in_ScriptName);
	}
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.profileedit.password.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void profilesave(CONN *sid)
{
	REC_USER user;
	char curdate[32];
	char opassword[50];
	char query[4096];
	char *ptemp;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if (dbread_profile(sid, 2, sid->dat->user_uid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", user.password);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL) user.prefdaystart=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL) user.prefdaylength=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAILCURRENT"))!=NULL) user.prefmailcurrent=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAILDEFAULT"))!=NULL) user.prefmaildefault=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL) user.prefmaxlist=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMENUSTYLE"))!=NULL) user.prefmenustyle=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL) user.preftimezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL) user.prefgeozone=atoi(ptemp);
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (strcmp(opassword, user.password)!=0) {
		snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
	}
	snprintf(query, sizeof(query)-1, "UPDATE gw_users SET obj_mtime = '%s', ", curdate);
	strncatf(query, sizeof(query)-strlen(query)-1, "password = '%s', ", user.password);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefdaystart = '%d', ", user.prefdaystart);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefdaylength = '%d', ", user.prefdaylength);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefmailcurrent = '%d', ", user.prefmailcurrent);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefmaildefault = '%d', ", user.prefmaildefault);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefmaxlist = '%d', ", user.prefmaxlist);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefmenustyle = '%d', ", user.prefmenustyle);
	strncatf(query, sizeof(query)-strlen(query)-1, "preftimezone = '%d', ", user.preftimezone);
	strncatf(query, sizeof(query)-strlen(query)-1, "prefgeozone = '%d'", user.prefgeozone);
	strncatf(query, sizeof(query)-strlen(query)-1, " WHERE userid = %d", sid->dat->user_uid);
	if (sql_update(query)<0) return;
	prints(sid, "<CENTER>Profile modified successfully</CENTER><BR>\n");
	db_log_activity(sid, 1, "profile", 0, "modify", "%s - %s modified profile", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\n", sid->dat->in_ScriptName);
	return;
}

void profiletimeedit(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	int i;
	int j;
	int sqr;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_queryf("SELECT availability FROM gw_users WHERE userid = %d", sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\n", sid->dat->user_uid);
		sql_freeresult(sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sql_getvalue(sqr, 0, 0), sizeof(uavailability)-1);
	sql_freeresult(sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if ((sqr=sql_queryf("SELECT availability FROM gw_groups WHERE groupid = %d", sid->dat->user_gid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\n", sid->dat->user_gid);
		sql_freeresult(sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sql_getvalue(sqr, 0, 0), sizeof(gavailability)-1);
	sql_freeresult(sqr);
	if (strlen(gavailability)!=168) {
		for (i=0;i<168;i++) {
			gavailability[i]='0';
		}
	}
	for (i=0;i<168;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='X';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (document.availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				document.availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (document.availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				document.availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/timesave NAME=availability>\n", sid->dat->in_ScriptName);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Availability for %s</TH></TR>\n", sid->dat->user_username);
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2 STYLE='border-style:solid'>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER STYLE='border-style:solid'><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=#F0F0F0>\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP CLASS=\"FIELDNAME\" STYLE='border-style:solid'><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
			} else {
				prints(sid, "<TD STYLE='border-style:solid'><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void profiletimesave(CONN *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	char *ptemp;
	char *pdest;
	int i;
	int j;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (sql_updatef("UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, sid->dat->user_uid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	db_log_activity(sid, 1, "profile", 0, "modify", "%s - %s modified availability", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/profile/edit\");\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_PROFILE);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/profile/save", 13)==0) {
		profilesave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/edit", 13)==0) {
		profileedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/timeedit", 17)==0) {
		profiletimeedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/timesave", 17)==0) {
		profiletimesave(sid);
	}
	htpage_footer(sid);
	return;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_profile",		// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/profile/",		// fn_uri
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
