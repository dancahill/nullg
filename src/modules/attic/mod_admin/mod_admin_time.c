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
#include "mod_admin.h"

void admingrouptimeedit(CONN *sid)
{
/*
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *ptemp;
	char availability[170];
	REC_GROUP group;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	if (dbread_group(sid, 2, groupid, &group)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", groupid);
		return;
	}
	memset(availability, 0, sizeof(availability));
	strncpy(availability, group.availability, sizeof(availability)-1);
	if (strlen(availability)!=168) {
		for (i=0;i<168;i++) {
			availability[i]='0';
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/grouptimesave NAME=availability>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\r\n", groupid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\r\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\r\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Group availability for <A HREF=%s/admin/groupedit?groupid=%d>%s</A></TH></TR>\r\n", sid->dat->in_ScriptName, groupid, group.groupname);
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
		prints(sid, "<TR CLASS=\"FIELDVAL\">\r\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP CLASS=\"FIELDNAME\" STYLE='border-style:solid'><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\r\n", i, dow[i]);
		for (j=0;j<24;j++) {
			prints(sid, "<TD STYLE='border-style:solid'><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\r\n", i, j, i, j, availability[i*24+j]=='1'?"checked":"");
		}
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
*/
	return;
}

void admingrouptimesave(CONN *sid)
{
/*
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
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
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sql_updatef("UPDATE gw_groups SET obj_mtime = '%s', availability = '%s' WHERE groupid = %d", curdate, availability, groupid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR />\r\n");
	db_log_activity(sid, "groups", groupid, "modify", "%s - %s modified availability for group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "location.replace(\"%s/admin/groupedit?groupid=%d\");\r\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "// -->\r\n</SCRIPT>\r\n<NOSCRIPT>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/groupedit?groupid=%d\">\r\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "</NOSCRIPT>\r\n");
*/
	return;
}

void adminusertimeedit(CONN *sid)
{
/*
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *ptemp;
	char gavailability[170];
	char uavailability[170];
	REC_USER user;
	int userid;
	int i;
	int j;
	SQLRES sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (dbread_user(sid, 2, userid, &user)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", userid);
		return;
	}
	if (sql_queryf(&sqr, "SELECT availability FROM gw_users WHERE userid = %d", user.userid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\r\n", user.userid);
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
	if (sql_queryf(&sqr, "SELECT availability FROM gw_groups WHERE groupid = %d", user.groupid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\r\n", user.groupid);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usertimesave NAME=availability>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\r\n", user.userid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\r\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\r\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Availability for <A HREF=%s/admin/useredit?userid=%d>%s</A></TH></TR>\r\n", sid->dat->in_ScriptName, userid, user.username);
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
		prints(sid, "<TR CLASS=\"FIELDVAL\">\r\n");
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
	prints(sid, "<BR />[<A HREF=%s/admin/grouptimeedit?groupid=%d>Group Availability</A>]\r\n", sid->dat->in_ScriptName, user.groupid);
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
*/
	return;
}

void adminusertimesave(CONN *sid)
{
/*
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int userid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
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
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sql_updatef("UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, userid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR />\r\n");
	db_log_activity(sid, "users", userid, "modify", "%s - %s modified availability for user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, userid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "location.replace(\"%s/admin/useredit?userid=%d\");\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\r\n</SCRIPT>\r\n<NOSCRIPT>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/useredit?userid=%d\">\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "</NOSCRIPT>\r\n");
*/
	return;
}
