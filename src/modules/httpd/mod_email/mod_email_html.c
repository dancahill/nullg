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

/*
static int htselect_mailfolder_r(CONN *sid, obj_t **qobj, int depth, int parentid, int selected)
{
	int foldercount=0;
	int fid, pid;
	int i, j;

	for (i=0;i<sql_numtuples(sqr);i++) {
		pid=atoi(sql_getvalue(sqr, i, 1));
		if (pid==parentid) {
			fid=atoi(sql_getvalue(sqr, i, 0));
			prints(sid, "<OPTION VALUE='%d'%s>", fid, fid==selected?" SELECTED":"");
			for (j=0;j<depth;j++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
			prints(sid, "%s\r\n", str2html(sid, sql_getvalue(sqr, i, 2)));
			foldercount+=htselect_mailfolder_r(sid, sqr, depth+1, fid, selected);
		}
	}
	return foldercount;
}
*/

void htselect_mailfolder(CONN *sid, int selected, short int allow_zero, short int show_root)
{
/*
	SQLRES sqr;

	if (!(auth_priv(sid, "email")&A_READ)) return;
	if (sql_queryf(&sqr, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, sid->dat->mailcurrent)<0) return;
	if (allow_zero) prints(sid, "<OPTION VALUE='0'>%s\r\n", show_root?"Root folder":"");
	if (sql_numtuples(&sqr)<1) {
		sql_freeresult(&sqr);
		return;
	}
	if (allow_zero) {
		if (show_root) {
			htselect_mailfolder_r(sid, &sqr, 1, 0, selected);
		} else {
			htselect_mailfolder_r(sid, &sqr, 0, 0, selected);
		}
	} else {
		htselect_mailfolder_r(sid, &sqr, 0, 0, selected);
	}
	sql_freeresult(&sqr);
*/
	return;
}

void htselect_mailaccount(CONN *sid, int selected)
{
/*
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE obj_uid = %d AND obj_did = %d ORDER BY mailaccountid ASC", sid->dat->uid, sid->dat->did)<0) return;
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	if ((sql_numtuples(&sqr)>0)&&(sid->dat->mailcurrent==0)) {
		sid->dat->mailcurrent=atoi(sql_getvalue(&sqr, 0, 0));
		sql_updatef("UPDATE gw_users SET prefmailcurrent = %d WHERE userid = %d AND domainid = %d", sid->dat->mailcurrent, sid->dat->uid, sid->dat->did);
	}
	sql_freeresult(&sqr);
*/
	return;
}
/*
void htselect_mailjump(CONN *sid, int selected)
{
	int i;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE obj_uid = %d AND obj_did = %d ORDER BY accountname ASC", sid->dat->uid, sid->dat->did)<0) return;
	if ((sql_numtuples(&sqr)>0)&&(sid->dat->mailcurrent==0)) {
		sid->dat->mailcurrent=atoi(sql_getvalue(&sqr, 0, 0));
		sql_updatef("UPDATE gw_users SET prefmailcurrent = %d WHERE userid = %d AND domainid = %d", sid->dat->mailcurrent, sid->dat->uid, sid->dat->did);
	}
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go() {\r\n");
	if (sid->dat->menustyle>0) {
		prints(sid, "	top.gwmain.location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	} else {
		prints(sid, "	location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	}
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=accountid style=\"width:180px\" onChange=\"go()\">');\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		if (sid->dat->menustyle>0) {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/main?accountid=%d\"%s>%s');\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)), atoi(sql_getvalue(&sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
		} else {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?accountid=%d\"%s>%s');\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)), atoi(sql_getvalue(&sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=accountid>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "<OPTION VALUE='%d'", atoi(sql_getvalue(&sqr, i, 0)));
		if (atoi(sql_getvalue(&sqr, i, 0))==selected) prints(sid, " SELECTED");
		prints(sid, ">%s\r\n", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	sql_freeresult(&sqr);
	return;
}
*/
void htselect_mailjump(CONN *sid, int accountid, int folderid)
{
	char *ScriptName=nes_getstr(sid->N, nes_settable(sid->N, &sid->N->g, "_SERVER"), "SCRIPT_NAME");
	_btree *btree;
	_ptree *ptree;
//	char curdate[32];
	int account;
	int base;
	int depth;
	int indent;
	int menustyle=0;
	int i, j, k;
	int x;
	obj_t *qobj1=NULL;
	obj_t *qobj2=NULL;

	if (!(auth_priv(sid, "email")&A_READ)) return;
//	if (sql_queryf(&sqr1, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE obj_uid = %d order by accountname ASC", sid->dat->uid)<0) return;
	if ((qobj1=ldir_getlist(sid->N, "emailaccount", sid->dat->uid, sid->dat->did))==NULL) {
		prints(sid, "error listing accounts"); return;
	}
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function ChangeMail() {\r\n");
	prints(sid, "	%slocation.replace('%s/mail/%s?'+document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value);\r\n", menustyle?"top.gwmain.":"", ScriptName, menustyle?"main":"list");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=accountid style=\"width:250px\" onChange=\"ChangeMail()\">');\r\n");
	for (i=0;i<ldir_numentries(&qobj1);i++) {
		account=atoi(ldir_getval(&qobj1, i, "id"));
		base=0;
		depth=1;
		indent=0;
//		if (sql_queryf(&sqr2, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, account)<0) continue;
		if ((qobj2=ldir_getlist(sid->N, "emailfolder", account, sid->dat->did))==NULL) {
			prints(sid, "error listing folders"); break;
		}
		if (ldir_numentries(&qobj2)<1) {
			ldir_freeresult(&qobj2);
			wmfolder_makedefaults(sid, account);
//			if (sql_queryf(&sqr2, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, sid->dat->mailcurrent)<0) continue;
			if ((qobj2=ldir_getlist(sid->N, "emailfolder", account, sid->dat->did))==NULL) {
				prints(sid, "error listing folders"); break;
			}
		}
		btree=calloc(ldir_numentries(&qobj2)+2, sizeof(_btree));
		ptree=calloc(ldir_numentries(&qobj2)+2, sizeof(_ptree));
		k=0;
widthloop:
		for (j=base;j<ldir_numentries(&qobj2);j++) {
			if (btree[j].printed) continue;
			if (atoi(ldir_getval(&qobj2, j, "parentfolderid"))==btree[depth].lastref) {
				ptree[k].id=j;
				ptree[k].depth=depth-1;
				k++;
				btree[depth+1].lastref=atoi(ldir_getval(&qobj2, j, "id"));
				btree[j].printed=1;
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
		if (base<ldir_numentries(&qobj2)) {
			goto widthloop;
		}
		x=-1;
		for (j=0;j<ldir_numentries(&qobj2);j++) {
			if (atoi(ldir_getval(&qobj2, j, "id"))==folderid) x=j;
		}
		prints(sid, "document.write('<OPTION VALUE=\"accountid=%d&folderid=1\"", account);
		if (account==accountid) prints(sid, " SELECTED CLASS=selBold");
		prints(sid, ">[ %s ]", str2html(sid, ldir_getval(&qobj1, i, "cn")));
		if (account==accountid) prints(sid, " - %s", x>-1?str2html(sid, ldir_getval(&qobj2, x, "cn")):"");
		prints(sid, "');\r\n");
		if (account==accountid) {
			for (j=0;j<ldir_numentries(&qobj2);j++) {
				x=atoi(ldir_getval(&qobj2, ptree[j].id, "id"));
				prints(sid, "document.write('<OPTION VALUE=\"accountid=%d&folderid=%d\">&nbsp;&nbsp;&nbsp;&nbsp;", account, x);
				for (indent=0;indent<ptree[j].depth;indent++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
				prints(sid, "%s", str2html(sid, ldir_getval(&qobj2, ptree[j].id, "cn")));
				if ((account==accountid)&&(x==folderid)) prints(sid, " - (Current)");
				prints(sid, "');\r\n");
			}
		}
		free(ptree);
		free(btree);
		ldir_freeresult(&qobj2);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=accountid>\r\n");
	for (i=0;i<ldir_numentries(&qobj1);i++) {
		prints(sid, "<OPTION VALUE='%d'", atoi(ldir_getval(&qobj1, i, "id")));
		if (atoi(ldir_getval(&qobj1, i, "id"))==accountid) prints(sid, " SELECTED");
		prints(sid, ">%s\r\n", str2html(sid, ldir_getval(&qobj1, i, "cn")));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit1 VALUE='GO'>\r\n");
	prints(sid, "<SELECT NAME=folderid>\r\n");
//	//	void htselect_mailfolder(CONN *sid, int selected, short int allow_zero, short int show_root)
	htselect_mailfolder(sid, folderid, 0, 0);
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit2 VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	ldir_freeresult(&qobj1);
	return;
}

void htselect_mailfolderjump(CONN *sid, int selected)
{
/*
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j;
	int x;
	SQLRES sqr;

	if (!(auth_priv(sid, "email")&A_READ)) return;
	if (sql_queryf(&sqr, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, sid->dat->mailcurrent)<0) return;
	if (sql_numtuples(&sqr)<1) {
		sql_freeresult(&sqr);
		wmfolder_makedefaults(sid, sid->dat->mailcurrent);
		if (sql_queryf(&sqr, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, sid->dat->mailcurrent)<0) return;
	}
	btree=calloc(sql_numtuples(&sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(&sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(&sqr);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(&sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(&sqr, i, 0));
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
	if (base<sql_numtuples(&sqr)) {
		goto widthloop;
	}
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function gombox() {\r\n");
	prints(sid, "	location=document.mailmbox.mbox.options[document.mailmbox.mbox.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=mbox style=\"width:180px\" onChange=\"gombox()\">');\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		x=atoi(sql_getvalue(&sqr, ptree[i].id, 0));
		prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?folderid=%d\"%s>", sid->dat->in_ScriptName, x, selected==x?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
		prints(sid, "%s');\r\n", str2html(sid, sql_getvalue(&sqr, ptree[i].id, 2)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=folderid>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		x=atoi(sql_getvalue(&sqr, ptree[i].id, 0));
		prints(sid, "<OPTION VALUE='%d'%s>", x, x==selected?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
		prints(sid, "%s\r\n", str2html(sid, sql_getvalue(&sqr, ptree[i].id, 2)));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	free(ptree);
	free(btree);
	sql_freeresult(&sqr);
*/
	return;
}
