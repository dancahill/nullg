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
#include "http_mod.h"
#include "mod_mail.h"

char *wmsearch_makestring(CONN *sid)
{
	char *searchstring=getbuffer(sid);
	char *ptemp1;
	char *ptemp2;

	ptemp1=getgetenv(sid, "C");
	ptemp2=getgetenv(sid, "TEXT");
	searchstring[0]='\0';
	if ((ptemp1!=NULL)&&(ptemp2!=NULL)) {
		if (strcmp(ptemp1, "addr")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=addr&text=%s", ptemp2);
		} else if (strcmp(ptemp1, "from")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=from&text=%s", ptemp2);
		} else if (strcmp(ptemp1, "to")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=to&text=%s", ptemp2);
		} else if (strcmp(ptemp1, "cc")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=cc&text=%s", ptemp2);
		} else if (strcmp(ptemp1, "subject")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=subject&text=%s", ptemp2);
		} else if (strcmp(ptemp1, "body")==0) {
			snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=body&text=%s", ptemp2);
		}
	}
	return searchstring;
}

int wmsearch_doquery(CONN *sid, const char *order_by, int folderid)
{
	char *ptemp1;
	char *ptemp2;
	int sqr;

	ptemp1=getgetenv(sid, "C");
	ptemp2=getgetenv(sid, "TEXT");
	if ((ptemp1!=NULL)&&(ptemp2!=NULL)) {
		if (strcmp(ptemp1, "addr")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' OR hdr_to LIKE '%%%s%%' OR hdr_cc LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, ptemp2, ptemp2, order_by);
			return sqr;
		} else if (strcmp(ptemp1, "from")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, order_by);
			return sqr;
		} else if (strcmp(ptemp1, "to")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_to LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, order_by);
			return sqr;
		} else if (strcmp(ptemp1, "cc")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_cc LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, order_by);
			return sqr;
		} else if (strcmp(ptemp1, "subject")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_subject LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, order_by);
			return sqr;
		} else if (strcmp(ptemp1, "body")==0) {
			sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND msg_text LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp2, order_by);
			return sqr;
		}
	}
	sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d AND folder = '%d' AND status != 'd' ORDER BY %s", sid->dat->user_uid, sid->dat->user_mailcurrent, folderid, order_by);
	return sqr;
}

void wmsearch_form(CONN *sid)
{
	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=GET ACTION=%s/mail/main NAME=search>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n");
	prints(sid, "<TR><TH COLSPAN=2>Mail Search</TH></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP COLSPAN=2>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
//	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Mail Account &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=accountid style='width:217px'>\n");
//	htselect_mailaccount(sid, sid->dat->user_mailcurrent);
//	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Search Header &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=c style='width:217px'>\n");
	prints(sid, "<OPTION VALUE='from'>From Address\n");
	prints(sid, "<OPTION VALUE='to'>To Address\n");
	prints(sid, "<OPTION VALUE='cc'>CC Address\n");
	prints(sid, "<OPTION VALUE='addr'>From, To, or CC Address\n");
	prints(sid, "<OPTION VALUE='subject'>Message Subject\n");
	prints(sid, "<OPTION VALUE='body' SELECTED>Message Body\n");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Search String &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=text value=\"\" SIZE=30 style='width:217px'></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Search'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.search.text.focus();\n// -->\n</SCRIPT>\n");
	return;
}
