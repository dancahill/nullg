/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
	char dirname[512];
	obj_t *cobj, *tobj=NULL;
	short found=0;

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/locale", nes_getstr(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "var_path"));
	fixslashes(dirname);
	dir_list(sid->N, &tobj, dirname);
	if (!nes_istable(tobj)) return;
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->val->type!=NT_TABLE) continue;
		if (strcmp(".", cobj->name)==0) continue;
		if (strcmp("..", cobj->name)==0) continue;
		if (strcmp(cobj->name, selected)==0) {
			prints(sid, "<OPTION VALUE='%s' SELECTED>%s\r\n", cobj->name, cobj->name);
			found=1;
		} else {
			prints(sid, "<OPTION VALUE='%s'>%s\r\n", cobj->name, cobj->name);
		}
	}
	if (!found) prints(sid, "<OPTION VALUE='en' SELECTED>\r\n");
	/*
	 * freetable would leave an empty val to free (plus the var) - unlink just leaves the var.
	 * this saves one line of code, and forces me to add a two line explanation.
	 */
	nes_unlinkval(sid->N, tobj);
	free(tobj);
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
	obj_t *qobj=NULL;
	int i, id;
	short found=0;

	if ((qobj=ldir_getlist(sid->N, "emailaccount", sid->dat->uid, sid->dat->did))==NULL) return;
//	if (sql_queryf(proc->N, &qobj, "SELECT id, name FROM nullgs_entries WHERE class = 'emailaccount' AND did = %d ORDER BY name ASC", sid->dat->did)<0) return;
	// owner check got killed here
//	if (sql_queryf(proc->N, &qobj, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE obj_uid = %d order by accountname ASC", sid->dat->uid)<0) return;
	for (i=0;i<ldir_numentries(&qobj);i++) {
		id=atoi(ldir_getval(&qobj, i, "id"));
		if (id==selected) found=1;
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", id, id==selected?" SELECTED":"", str2html(sid, ldir_getval(&qobj, i, "cn")));
	}
	ldir_freeresult(&qobj);
	if (!found) prints(sid, "<OPTION VALUE='0' SELECTED>\r\n");
	return;
}

void htselect_theme(CONN *sid, char *selected)
{
	char dirname[512];
	obj_t *cobj, *tobj=NULL;
	short found=0;

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/htdocs/themes", nes_getstr(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "var_path"));
	fixslashes(dirname);
	dir_list(sid->N, &tobj, dirname);
	if (!nes_istable(tobj)) return;
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->val->type!=NT_TABLE) continue;
		if (strcmp(".", cobj->name)==0) continue;
		if (strcmp("..", cobj->name)==0) continue;
		if (strcmp(cobj->name, selected)==0) {
			prints(sid, "<OPTION VALUE='%s' SELECTED>%s\r\n", cobj->name, cobj->name);
			found=1;
		} else {
			prints(sid, "<OPTION VALUE='%s'>%s\r\n", cobj->name, cobj->name);
		}
	}
	if (!found) prints(sid, "<OPTION VALUE='default' SELECTED>\r\n");
	nes_unlinkval(sid->N, tobj);
	free(tobj);
	return;
}

void profileedit(CONN *sid)
{
	int accountid=nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (dbread_profile(sid, 2, sid->dat->uid, &qobj)!=0) {
		prints(sid, "<CENTER>Profile information not found</CENTER>\r\n");
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/save NAME=profileedit>\r\n", ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='padding:2px'>User Profile for %s</TH></TR>\r\n", sid->dat->username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password             </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password  VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "userpassword")));
	if (module_exists("calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Start       </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefdaystart style='width:255px'>\r\n");
//		htselect_hour(sid, user.prefdaystart);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Length      </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefdaylength style='width:255px'>\r\n");
//		htselect_number(sid, user.prefdaylength, 0, 24, 1);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefgeozone style='width:255px'>\r\n");
//		htselect_zone(sid, user.prefgeozone, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	if (module_exists("email")&&(auth_priv(sid, "email")>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Current Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
		prints(sid, "<SELECT NAME=prefmailcurrent style='width:255px'>\r\n");
		htselect_mailaccount(sid, accountid);
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Default Mail Account</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmaildefault style='width:255px'>\r\n");
//	prints(sid, "<OPTION VALUE=0%s>External Mail Client\r\n", user.prefmaildefault!=1?" SELECTED":"");
//	if (module_exists("mod_email")&&(auth_priv(sid, "email")>0)) {
//		prints(sid, "<OPTION VALUE=1%s>Web E-Mail\r\n", user.prefmaildefault==1?" SELECTED":"");
//	}
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Results per Page</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmaxlist style='width:255px'>\r\n");
	htselect_number(sid, sid->dat->maxlist, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Menu and Frame Style</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=prefmenustyle style='width:255px'>\r\n");
//	htselect_layout(sid, user.prefmenustyle);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone           </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preftimezone style='width:255px'>\r\n");
	htselect_timezone(sid, sid->dat->timezone);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Language&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preflanguage style='width:255px'>\r\n");
	htselect_lang(sid, sid->dat->language);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Theme&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=preftheme style='width:255px'>\r\n");
	htselect_theme(sid, sid->dat->theme);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</TD></TR>");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (module_exists("mod_email")&&(auth_priv(sid, "email")>0)) {
		prints(sid, "[<A HREF=%s/mail/accounts/list>Edit Mail Accounts</A>]\r\n", ScriptName);
	}
	if (module_exists("mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
		prints(sid, "[<A HREF=%s/profile/timeedit>Edit Availability</A>]\r\n", ScriptName);
	}
	prints(sid, "</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.profileedit.password.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void profilesave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	obj_t *qobj=NULL, *tobj, *tobj2;
//	char curdate[32];
	char opassword[50];
//	char query[4096];
	char *ptemp;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) return;
	if (dbread_profile(sid, 2, sid->dat->uid, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", ldir_getval(&qobj, 0, "userpassword"));
//	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) nes_setstr(sid->N, tobj, "userpassword", ptemp, strlen(ptemp));

	tobj2=nes_settable(proc->N, tobj, "pref");
//	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL)    user.prefdaystart=atoi(ptemp);
//	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL)   user.prefdaylength=atoi(ptemp);
//	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL)     user.prefgeozone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAILCURRENT"))!=NULL) nes_setnum(sid->N, tobj2, "mailcurrent", atoi(ptemp));
//	if ((ptemp=getpostenv(sid, "PREFMAILDEFAULT"))!=NULL) user.prefmaildefault=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL)     nes_setstr(sid->N, tobj2, "maxlist", ptemp, strlen(ptemp));
//	if ((ptemp=getpostenv(sid, "PREFMENUSTYLE"))!=NULL)   user.prefmenustyle=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL)    nes_setstr(sid->N, tobj2, "timezone", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "PREFLANGUAGE"))!=NULL)    nes_setstr(sid->N, tobj2, "language", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "PREFTHEME"))!=NULL)       nes_setstr(sid->N, tobj2, "theme",    ptemp, strlen(ptemp));
/*
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
	strncatf(query, sizeof(query)-strlen(query)-1, " WHERE userid = %d", sid->dat->uid);
	if (sql_update(proc->N, query)<0) goto cleanup;
*/
	if (dbwrite_profile(sid, sid->dat->uid, &qobj)<1) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	prints(sid, "<CENTER>Profile modified successfully</CENTER><BR />\r\n");
	db_log_activity(sid, "profile", 0, "modify", "%s - %s modified profile", sid->socket.RemoteAddr, sid->dat->username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/profile/edit\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\"></NOSCRIPT>\r\n", ScriptName);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void profiletimeedit(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	int i;
	int j;
	obj_t *qobj=NULL;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (sql_queryf(proc->N, &qobj, "SELECT availability FROM gw_users WHERE userid = %d", sid->dat->uid)<0) return;
	if (sql_numtuples(proc->N, &qobj)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\r\n", sid->dat->uid);
		sql_freeresult(proc->N, &qobj);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sql_getvalue(proc->N, &qobj, 0, 0), sizeof(uavailability)-1);
	sql_freeresult(proc->N, &qobj);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if (sql_queryf(proc->N, &qobj, "SELECT availability FROM gw_groups WHERE groupid = %d", sid->dat->gid)<0) return;
	if (sql_numtuples(proc->N, &qobj)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\r\n", sid->dat->gid);
		sql_freeresult(proc->N, &qobj);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sql_getvalue(proc->N, &qobj, 0, 0), sizeof(gavailability)-1);
	sql_freeresult(proc->N, &qobj);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/timesave NAME=availability>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\r\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\r\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Availability for %s</TH></TR>\r\n", sid->dat->username);
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
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char availability[170];
	char curdate[40];
	char qvar[10];
	char *ptemp;
	char *pdest;
	int i;
	int j;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) return;
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
	if (sql_updatef(proc->N, "UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, sid->dat->uid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR />\r\n");
	db_log_activity(sid, "profile", 0, "modify", "%s - %s modified availability", sid->socket.RemoteAddr, sid->dat->username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/profile/edit\");\r\n// -->\r\n</SCRIPT>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\"></NOSCRIPT>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_PROFILE);
	prints(sid, "<BR />\r\n");
	if (strncmp(RequestURI, "/profile/save", 13)==0) {
		profilesave(sid);
	} else if (strncmp(RequestURI, "/profile/edit", 13)==0) {
		profileedit(sid);
	} else if (strncmp(RequestURI, "/profile/timeedit", 17)==0) {
		profiletimeedit(sid);
	} else if (strncmp(RequestURI, "/profile/timesave", 17)==0) {
		profiletimesave(sid);
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
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "profile");
	nes_exec(proc->N, "GWMODULES.profile={submenu=0,perm='profile',fn_name='mod_main',fn_uri='/profile/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
