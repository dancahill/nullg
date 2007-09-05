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
#include "mod_email.h"

char *wmsearch_makestring(CONN *sid)
{
	char *searchstring=getbuffer(sid);
/*
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
*/
	return searchstring;
}

int wmsearch_doquery(CONN *sid, obj_t **qobj, const char *order_by, int folderid)
{
/*
	char *ptemp1;
	char *ptemp2;

	ptemp1=getgetenv(sid, "C");
	ptemp2=getgetenv(sid, "TEXT");
	if ((ptemp1!=NULL)&&(ptemp2!=NULL)) {
		if (strcmp(ptemp1, "addr")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' OR hdr_to LIKE '%%%s%%' OR hdr_cc LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, ptemp2, ptemp2, order_by);
		} else if (strcmp(ptemp1, "from")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, order_by);
		} else if (strcmp(ptemp1, "to")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_to LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, order_by);
		} else if (strcmp(ptemp1, "cc")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_cc LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, order_by);
		} else if (strcmp(ptemp1, "subject")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_subject LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, order_by);
		} else if (strcmp(ptemp1, "body")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND msg_text LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, order_by);
		} else if (strcmp(ptemp1, "all")==0) {
			return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' OR hdr_to LIKE '%%%s%%' OR hdr_cc LIKE '%%%s%%' OR hdr_subject LIKE '%%%s%%' OR msg_text LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->uid, ptemp2, ptemp2, ptemp2, ptemp2, ptemp2, order_by);
		}
	}
	return sql_queryf(sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d and accountid = %d AND folder = %d AND status != 'd' ORDER BY %s", sid->dat->uid, sid->dat->mailcurrent, folderid, order_by);
*/
	return -1;
}

void wmsearch_form(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");

	if (!(auth_priv(sid, "email")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	prints(sid, "<BR /><CENTER>\r\n");
	prints(sid, "<FORM METHOD=GET ACTION=%s/mail/main NAME=search>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<TR><TH COLSPAN=2>Mail Search</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP COLSPAN=2>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*
//	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Mail Account &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=accountid style='width:217px'>\r\n");
//	htselect_mailaccount(sid, sid->dat->mailcurrent);
//	prints(sid, "</SELECT></TD></TR>\r\n");
*/
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Search Header &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=c style='width:217px'>\r\n");
	prints(sid, "<OPTION VALUE='from'>From Address\r\n");
	prints(sid, "<OPTION VALUE='to'>To Address\r\n");
	prints(sid, "<OPTION VALUE='cc'>CC Address\r\n");
	prints(sid, "<OPTION VALUE='addr'>From, To, or CC Address\r\n");
	prints(sid, "<OPTION VALUE='subject'>Message Subject\r\n");
	prints(sid, "<OPTION VALUE='body' SELECTED>Message Body\r\n");
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Search String &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=text value=\"\" SIZE=30 style='width:217px'></TD></TR>\r\n");
	prints(sid, "</TABLE></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Search'>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.search.text.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}
