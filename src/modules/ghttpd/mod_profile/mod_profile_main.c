/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_profile.h"
#ifndef WIN32
#include <dirent.h>
#endif

void htselect_lang(CONN *sid, char *selected)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR  *handle;
	char dirname[512];

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/locale", config->dir_var);
	fixslashes(dirname);
	handle=opendir(dirname);
	while ((dentry=readdir(handle))!=NULL) {
//		stat(dentry->d_name, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;
		prints(sid, "<OPTION VALUE='%s'%s>%s\r\n", dentry->d_name, (strcmp(dentry->d_name, selected)==0)?" SELECTED":"", dentry->d_name);
	}
	closedir(handle);
	return;
}

void htselect_layout(CONN *sid, int selected)
{
	char *option[]={ "Simple (Frameless)", "Standard", "Outlook" };
	int i;

	for (i=0;i<3;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_mailaccount(CONN *sid, int selected)
{
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE obj_uid = %d order by accountname ASC", sid->dat->user_uid)<0) return;
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

void htselect_theme(CONN *sid, char *selected)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR  *handle;
	char dirname[512];

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/htdocs/groupware/themes", config->dir_var);
	fixslashes(dirname);
	handle=opendir(dirname);
	while ((dentry=readdir(handle))!=NULL) {
//		stat(dentry->d_name, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;

		prints(sid, "<OPTION VALUE='%s'%s>%s\r\n", dentry->d_name, (strcmp(dentry->d_name, selected)==0)?" SELECTED":"", dentry->d_name);

	}
	closedir(handle);
	return;
}

void profileedit(CONN *sid)
{
	REC_USER user;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (dbread_profile(sid, 2, sid->dat->user_uid, &user)!=0) {
		prints(sid, "<CENTER>Profile information not found</CENTER>\r\n");
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/save NAME=profileedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='padding:2px'>User Profile for %s</TH></TR>\r\n", sid->dat->user_username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password             </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password  VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user.password));
	if (module_exists("mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Start       </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefdaystart style='width:255px'>\r\n");
		htselect_hour(sid, user.prefdaystart);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Length      </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefdaylength style='width:255px'>\r\n");
		htselect_number(sid, user.prefdaylength, 0, 24, 1);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefgeozone style='width:255px'>\r\n");
		htselect_zone(sid, user.prefgeozone, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	if (module_exists("mod_email")&&(auth_priv(sid, "email")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Current Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefmailcurrent style='width:255px'>\r\n");
		htselect_mailaccount(sid, user.prefmailcurrent);
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Default Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmaildefault style='width:255px'>\r\n");
	prints(sid, "<OPTION VALUE=0%s>External Mail Client\r\n", user.prefmaildefault!=1?" SELECTED":"");
	if (module_exists("mod_email")&&(auth_priv(sid, "email")>0)) {
		prints(sid, "<OPTION VALUE=1%s>Web E-Mail\r\n", user.prefmaildefault==1?" SELECTED":"");
	}
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Maximum Results/Page</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmaxlist style='width:255px'>\r\n");
	htselect_number(sid, user.prefmaxlist, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Menu and Frame Style</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmenustyle style='width:255px'>\r\n");
	htselect_layout(sid, user.prefmenustyle);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone           </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preftimezone style='width:255px'>\r\n");
	htselect_timezone(sid, user.preftimezone);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Language&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preflanguage style='width:255px'>\r\n");
	htselect_lang(sid, user.preflanguage);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Theme&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preftheme style='width:255px'>\r\n");
	htselect_theme(sid, user.preftheme);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</TD></TR>");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (module_exists("mod_email")&&(auth_priv(sid, "email")>0)) {
		prints(sid, "[<A HREF=%s/mail/accounts/list>Edit Mail Accounts</A>]\r\n", sid->dat->in_ScriptName);
	}
	if (module_exists("mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "[<A HREF=%s/profile/timeedit>Edit Availability</A>]\r\n", sid->dat->in_ScriptName);
	}
	prints(sid, "</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.profileedit.password.focus();\r\n// -->\r\n</SCRIPT>\r\n");
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if (dbread_profile(sid, 2, sid->dat->user_uid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
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
	if ((ptemp=getpostenv(sid, "PREFLANGUAGE"))!=NULL) snprintf(user.preflanguage, sizeof(user.preflanguage)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PREFTHEME"))!=NULL) snprintf(user.preftheme, sizeof(user.preftheme)-1, "%s", ptemp);
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
	strncatf(query, sizeof(query)-strlen(query)-1, "prefgeozone = '%d', ", user.prefgeozone);
	strncatf(query, sizeof(query)-strlen(query)-1, "preflanguage = '%s', ", user.preflanguage);
	strncatf(query, sizeof(query)-strlen(query)-1, "preftheme = '%s'", user.preftheme);
	strncatf(query, sizeof(query)-strlen(query)-1, " WHERE userid = %d", sid->dat->user_uid);
	if (sql_update(query)<0) return;
	prints(sid, "<CENTER>Profile modified successfully</CENTER><BR>\r\n");
	db_log_activity(sid, "profile", 0, "modify", "%s - %s modified profile", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\r\n", sid->dat->in_ScriptName);
	return;
}

void profiletimeedit(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	int i;
	int j;
	SQLRES sqr;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr, "SELECT availability FROM gw_users WHERE userid = %d", sid->dat->user_uid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\r\n", sid->dat->user_uid);
		sql_freeresult(&sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sql_getvalue(&sqr, 0, 0), sizeof(uavailability)-1);
	sql_freeresult(&sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if (sql_queryf(&sqr, "SELECT availability FROM gw_groups WHERE groupid = %d", sid->dat->user_gid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\r\n", sid->dat->user_gid);
		sql_freeresult(&sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sql_getvalue(&sqr, 0, 0), sizeof(gavailability)-1);
	sql_freeresult(&sqr);
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
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function toggle(b,n)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	if (b == \"t\" ) {\r\n");
	prints(sid, "		if (document.availability[\"t\" + n].value == 'true') {\r\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'false'\r\n");
	prints(sid, "			var bool = true\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'true'\r\n");
	prints(sid, "			var bool = false\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "		for (x=0;x<7;x++) {\r\n");
	prints(sid, "			if (document.availability[\"d\" + x + \"t\" + n]) {\r\n");
	prints(sid, "				document.availability[\"d\" + x + \"t\" + n].checked = bool\r\n");
	prints(sid, "			}\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	} else {\r\n");
	prints(sid, "		if (document.availability[\"d\" + n].value == 'true') {\r\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'false'\r\n");
	prints(sid, "			var bool = true\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'true'\r\n");
	prints(sid, "			var bool = false\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "		for (x=0;x<24;x++) {\r\n");
	prints(sid, "			if (document.availability[\"d\" + n + \"t\" + x]) {\r\n");
	prints(sid, "				document.availability[\"d\" + n + \"t\" + x].checked = bool\r\n");
	prints(sid, "			}\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/timesave NAME=availability>\r\n", sid->dat->in_ScriptName);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\r\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\r\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Availability for %s</TH></TR>\r\n", sid->dat->user_username);
	prints(sid, "<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2 STYLE='border-style:solid'>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>A.M.</B></TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>P.M.</B></TD>\r\n");
	prints(sid, "</TR>\r\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\">\r\n");
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER STYLE='border-style:solid'><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\r\n", i, j);
	}
	prints(sid, "</TR>\r\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=#F0F0F0>\r\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP CLASS=\"FIELDNAME\" STYLE='border-style:solid'><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\r\n", i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\r\n");
			} else {
				prints(sid, "<TD STYLE='border-style:solid'><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\r\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
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
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\r\n");
	db_log_activity(sid, "profile", 0, "modify", "%s - %s modified availability", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/profile/edit\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName);
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
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
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_profile",		// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menupic
		"",			// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/profile/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
