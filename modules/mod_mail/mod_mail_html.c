/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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

void htselect_mailaccount(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by mailaccountid ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_mailjump(CONN *sid, int selected)
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

void htselect_mailfolder(CONN *sid, int selected)
{
	_btree *btree;
	_ptree *ptree;
	char curdate[32];
	int base=0;
	int depth=1;
	int indent=0;
	int i, j;
	int x;
	int sqr;

	if (!(auth_priv(sid, "webmail")&A_READ)) return;
	if ((sqr=sql_queryf(sid, "SELECT mailfolderid, parentfolderid, foldername FROM gw_mailfolders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) return;
	if (sql_numtuples(sqr)<1) {
		sql_freeresult(sqr);
		memset(curdate, 0, sizeof(curdate));
		snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('1', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Inbox');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('2', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Outbox');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('3', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Sent Items');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('4', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Trash');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('5', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Drafts');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		sql_updatef(sid, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values ('6', '%s', '%s', '%d', '0', '0', '0', '0', '%d', '0', 'Saved Items');", curdate, curdate, sid->dat->user_uid, sid->dat->user_mailcurrent);
		if ((sqr=sql_queryf(sid, "SELECT mailfolderid, parentfolderid, foldername FROM gw_mailfolders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) return;
	}
	btree=calloc(sql_numtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(sqr);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr, i, 0));
			btree[i].printed=1;
			depth++;
		}
	}
	if (depth>0) {
		depth--;
		goto widthloop;
	}
	base++;
	btree[depth].lastref=0;
	depth=0;
	if (base<sql_numtuples(sqr)) {
		goto widthloop;
	}
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function gombox() {\r\n");
	prints(sid, "	location=document.mailmbox.mbox.options[document.mailmbox.mbox.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=mbox onChange=\"gombox()\">');\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		x=atoi(sql_getvalue(sqr, ptree[i].id, 0));
		prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?folderid=%d\"%s>", sid->dat->in_ScriptName, x, selected==x?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;");
		prints(sid, "%s');\n", str2html(sid, sql_getvalue(sqr, ptree[i].id, 2)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=folderid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		x=atoi(sql_getvalue(sqr, ptree[i].id, 0));
		prints(sid, "<OPTION VALUE='%d'%s>", x, x==selected?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;");
		prints(sid, "%s\n", str2html(sid, sql_getvalue(sqr, ptree[i].id, 2)));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	free(ptree);
	free(btree);
	sql_freeresult(sqr);
	return;
}

void htselect_mailfolder2(CONN *sid, int selected)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j;
	int x;
	int sqr;

	if (!(auth_priv(sid, "webmail")&A_READ)) return;
	if ((sqr=sql_queryf(sid, "SELECT mailfolderid, parentfolderid, foldername FROM gw_mailfolders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	if (sql_numtuples(sqr)<1) {
		sql_freeresult(sqr);
		return;
	}
	btree=calloc(sql_numtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(sqr);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr, i, 0));
			btree[i].printed=1;
			depth++;
		}
	}
	if (depth>0) {
		depth--;
		goto widthloop;
	}
	base++;
	btree[depth].lastref=0;
	depth=0;
	if (base<sql_numtuples(sqr)) {
		goto widthloop;
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		x=atoi(sql_getvalue(sqr, ptree[i].id, 0));
		prints(sid, "<OPTION VALUE='%d'%s>", x, x==selected?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;");
		prints(sid, "%s\n", str2html(sid, sql_getvalue(sqr, ptree[i].id, 2)));
	}
	free(ptree);
	free(btree);
	sql_freeresult(sqr);
	return;
}
