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
#include "mod_substub.h"
#include "mod_mail.h"

void htselect_mailjump(CONNECTION *sid, int selected)
{
	int i;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by accountname ASC", sid->dat->user_uid))<0) return;
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go() {\r\n");
	if (sid->dat->user_menustyle>0) {
		prints(sid, "	top.gwmain.location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	} else {
		prints(sid, "	location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	}
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=accountid onChange=\"go()\">');\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/main?accountid=%d\"%s>%s');\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
		} else {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?accountid=%d\"%s>%s');\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=accountid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<OPTION VALUE='%d'", atoi(sql_getvalue(sqr, i, 0)));
		if (atoi(sql_getvalue(sqr, i, 0))==selected) prints(sid, " SELECTED");
		prints(sid, ">%s\n", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	sql_freeresult(sqr);
	return;
}

void htselect_mailmbox(CONNECTION *sid, char *selected)
{
	char *option[]={ "Inbox", "Drafts", "Outbox", "Saved Items", "Sent Items", "Trash" };
	int i;
//	int sqr;

//	if ((sqr=sql_queryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by accountname ASC", sid->dat->user_uid))<0) return;
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function gombox() {\r\n");
	prints(sid, "	location=document.mailmbox.mbox.options[document.mailmbox.mbox.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=mbox onChange=\"gombox()\">');\r\n");
	for (i=0;i<6;i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?mbox=%s\"%s>%s');\n", sid->dat->in_ScriptName, option[i], strcasecmp(selected, option[i])==0?" SELECTED":"", str2html(sid, option[i]));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=accountid>\r\n");
	for (i=0;i<6;i++) {
		prints(sid, "<OPTION VALUE=\"%s\"%s>%s\n", option[i], strcasecmp(selected, option[i])==0?" SELECTED":"", str2html(sid, option[i]));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
//	sql_freeresult(sqr);
	return;
}
