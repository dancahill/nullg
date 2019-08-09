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
#include "mod_email.h"

void wmaddr_list(CONN *sid)
{
	char *ptemp;
	char field[10];
	char realname1[80];
	char realname2[80];
	int i, j;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	memset(field, 0, sizeof(field));
	if ((ptemp=getgetenv(sid, "FIELD"))!=NULL) {
		snprintf(field, sizeof(field)-1, "%s", ptemp);
	}
	prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\r\n");
	prints(sid, "function SendTo(field, name, addr) {\r\n");
	prints(sid, "	var rcpt=\"\\\"\"+name+\"\\\" <\"+addr+\">\";\r\n\r\n");
	prints(sid, "	if (field==\"cc\") {\r\n");
	prints(sid, "		if (window.opener.document.wmcompose.msgcc.value!=\"\") {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgcc.value+=\", \"+rcpt;\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgcc.value=rcpt;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	} else if (field==\"bcc\") {\r\n");
	prints(sid, "		if (window.opener.document.wmcompose.msgbcc.value!=\"\") {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgbcc.value+=\", \"+rcpt;\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgbcc.value=rcpt;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	} else {\r\n");
	prints(sid, "		if (window.opener.document.wmcompose.msgto.value!=\"\") {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgto.value+=\", \"+rcpt;\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			window.opener.document.wmcompose.msgto.value=rcpt;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function SendToAll() {\r\n");
	prints(sid, "	var entries=document.mailform.elements.length/3;\r\n\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='TO') {\r\n");
	prints(sid, "			SendTo('to', document.mailform[\"name\"+i].value, document.mailform[\"addr\"+i].value);\r\n");
	prints(sid, "		} else if (document.mailform[\"option\"+i].value=='CC') {\r\n");
	prints(sid, "			SendTo('cc', document.mailform[\"name\"+i].value, document.mailform[\"addr\"+i].value);\r\n");
	prints(sid, "		} else if (document.mailform[\"option\"+i].value=='BCC') {\r\n");
	prints(sid, "			SendTo('bcc', document.mailform[\"name\"+i].value, document.mailform[\"addr\"+i].value);\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	window.close();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	if (sql_queryf(&sqr1, "SELECT userid, surname, givenname, email from gw_users WHERE domainid = %d ORDER BY surname, givenname ASC", sid->dat->user_did)<0) return;
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname, email from gw_contacts WHERE obj_did = %d ORDER BY surname, givenname ASC", sid->dat->user_did)<0) return;
	} else {
		if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname, email from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did)<0) return;
	}
	if ((sql_numtuples(&sqr1)<1)&&(sql_numtuples(&sqr2)<1)) {
		sql_freeresult(&sqr1);
		sql_freeresult(&sqr2);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP><A HREF=\"javascript:SendToAll()\">Add Recipients</A></TD>");
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailform>\r\n");
	j=0;
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;User Name&nbsp;</TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;E-Mail&nbsp;</TH></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		if (strlen(sql_getvalue(&sqr1, i, 3))<1) continue;
		if (strchr(sql_getvalue(&sqr1, i, 3), '@')==NULL) continue;
		memset(realname1, 0, sizeof(realname1));
		memset(realname2, 0, sizeof(realname2));
		strncpy(realname1, sql_getvalue(&sqr1, i, 1), sizeof(realname1)-1);
		if (strlen(sql_getvalue(&sqr1, i, 1))&&strlen(sql_getvalue(&sqr1, i, 2))) strncat(realname1, ", ", sizeof(realname1)-strlen(realname1)-1);
		strncat(realname1, sql_getvalue(&sqr1, i, 2), sizeof(realname1)-strlen(realname1)-1);
		strncpy(realname2, sql_getvalue(&sqr1, i, 2), sizeof(realname2)-1);
		if (strlen(sql_getvalue(&sqr1, i, 1))&&strlen(sql_getvalue(&sqr1, i, 2))) strncat(realname2, " ", sizeof(realname2)-strlen(realname2)-1);
		strncat(realname2, sql_getvalue(&sqr1, i, 1), sizeof(realname2)-strlen(realname2)-1);
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, realname1));
		prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><A HREF=\"javascript:SendTo('%s', '%s', '%s'); window.close();\">%s</A>&nbsp;</TD>", field, realname2, sql_getvalue(&sqr1, i, 3), str2html(sid, sql_getvalue(&sqr1, i, 3)));
		prints(sid, "<INPUT TYPE=hidden NAME=name%d VALUE=\"%s\">", j, realname2);
		prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", j, sql_getvalue(&sqr1, i, 3));
		prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid'><SELECT NAME=option%d STYLE='font-size:10px; width:44px'>", j);
		prints(sid, "<OPTION VALUE=''>");
		if (strchr(sql_getvalue(&sqr1, i, 3), '@')) {
			prints(sid, "<OPTION VALUE='TO'>TO");
			prints(sid, "<OPTION VALUE='CC'>CC");
			prints(sid, "<OPTION VALUE='BCC'>BCC");
		}
		prints(sid, "</SELECT></TD></TR>\r\n");
		j++;
	}
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Contact Name&nbsp;</TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;E-Mail&nbsp;</TH></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr2);i++) {
		if (strlen(sql_getvalue(&sqr2, i, 3))<1) continue;
		if (strchr(sql_getvalue(&sqr2, i, 3), '@')==NULL) continue;
		memset(realname1, 0, sizeof(realname1));
		memset(realname2, 0, sizeof(realname2));
		strncpy(realname1, sql_getvalue(&sqr2, i, 1), sizeof(realname1)-1);
		if (strlen(sql_getvalue(&sqr2, i, 1))&&strlen(sql_getvalue(&sqr2, i, 2))) strncat(realname1, ", ", sizeof(realname1)-strlen(realname1)-1);
		strncat(realname1, sql_getvalue(&sqr2, i, 2), sizeof(realname1)-strlen(realname1)-1);
		strncpy(realname2, sql_getvalue(&sqr2, i, 2), sizeof(realname2)-1);
		if (strlen(sql_getvalue(&sqr2, i, 1))&&strlen(sql_getvalue(&sqr2, i, 2))) strncat(realname2, " ", sizeof(realname2)-strlen(realname2)-1);
		strncat(realname2, sql_getvalue(&sqr2, i, 1), sizeof(realname2)-strlen(realname2)-1);
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, realname1));
		prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><A HREF=\"javascript:SendTo('%s', '%s', '%s'); window.close();\">%s</A>&nbsp;</TD>", field, realname2, sql_getvalue(&sqr2, i, 3), str2html(sid, sql_getvalue(&sqr2, i, 3)));
		prints(sid, "<INPUT TYPE=hidden NAME=name%d VALUE=\"%s\">", j, realname2);
		prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", j, sql_getvalue(&sqr2, i, 3));
		prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid'><SELECT NAME=option%d STYLE='font-size:10px; width:44px'>", j);
		prints(sid, "<OPTION VALUE=''>");
		if (strchr(sql_getvalue(&sqr2, i, 3), '@')) {
			prints(sid, "<OPTION VALUE='TO'>TO");
			prints(sid, "<OPTION VALUE='CC'>CC");
			prints(sid, "<OPTION VALUE='BCC'>BCC");
		}
		prints(sid, "</SELECT></TD></TR>\r\n");
		j++;
	}
	prints(sid, "</FORM></TABLE>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP><A HREF=\"javascript:SendToAll()\">Add Recipients</A></TD>");
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
	sql_freeresult(&sqr2);
	return;
}
