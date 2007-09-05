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
#include "mod_finance.h"

void account_edit(CONN *sid)
{
/*
	REC_FIN_ACCOUNT account;
	char *ptemp;
	int accountid;
	int err;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/finance/accounts/editnew", 28)==0) {
		accountid=0;
		if ((err=dbread_account(sid, 2, 0, &account))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", accountid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
	} else {
		if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
		accountid=atoi(ptemp);
		if ((err=dbread_account(sid, 2, accountid, &account))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", accountid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (accountid>0) {
		prints(sid, "<B>account %d</B>\r\n", accountid);
	} else {
		prints(sid, "<B>New Account</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/finance/accounts/save NAME=accountedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=accountid VALUE='%d'>\r\n", account.accountid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SETTINGS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Account Name &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=accountname    VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, account.accountname));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((account.obj_uid==sid->dat->uid)||(auth_priv(sid, "finance")&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "finance")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, account.obj_uid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "finance")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "finance"), account.obj_gid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\r\n", account.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\r\n", account.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\r\n", account.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\r\n", account.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\r\n", account.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\r\n", account.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "finance")&A_DELETE)&&(accountid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.accountedit.accountname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void account_view(CONN *sid)
{
/*
	REC_FIN_ACCOUNT account;
	char *ptemp;
	int accountid;
	int err;


	char monstr[8];
	int i;
	SQLRES sqr;
	float balance, credit, debit;
	float mbalance, mcredit, mdebit;
	float tbalance, tcredit, tdebit;



	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if ((err=dbread_account(sid, 2, accountid, &account))==-2) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", accountid);
		return;
	} else if (err!=0) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>&nbsp;Account %d&nbsp;</TH></TR>\r\n", account.accountid);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>&nbsp;Account Name &nbsp;</B></TD><TD WIDTH=100%% CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, account.accountname));
	prints(sid, "</TABLE>\r\n");
	if (sql_queryf(&sqr, "SELECT entryid, entrydate, accountid, contactid, debit, credit, details FROM gw_finance_journal WHERE accountid = %d AND obj_did = %d ORDER BY entrydate ASC, entryid ASC", account.accountid, sid->dat->did)<0) return;
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "<BR />\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH STYLE='border-style:solid'>id</TH><TH STYLE='border-style:solid'>date</TH><TH STYLE='border-style:solid'>details</TH><TH STYLE='border-style:solid'>contactid</TH><TH STYLE='border-style:solid'>debit</TH><TH STYLE='border-style:solid'>credit</TH><TH STYLE='border-style:solid'>balance</TH></TR>\r\n");
		balance=0;
		memset(monstr, 0, sizeof(monstr));
		strncpy(monstr, sql_getvalue(&sqr, 0, 1), sizeof(monstr)-1);
		mbalance=0;
		mcredit=0;
		mdebit=0;
		tbalance=0;
		tcredit=0;
		tdebit=0;
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(&sqr, i, 0)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%.10s</TD>", str2html(sid, sql_getvalue(&sqr, i, 1)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(&sqr, i, 6)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(&sqr, i, 3)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f</TD>", atof(sql_getvalue(&sqr, i, 4)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f</TD>", atof(sql_getvalue(&sqr, i, 5)));
			debit=(float)(atof(sql_getvalue(&sqr, i, 4)));
			credit=(float)(atof(sql_getvalue(&sqr, i, 5)));
			mdebit+=debit;
			mcredit+=credit;
			tdebit+=debit;
			tcredit+=credit;
			balance-=debit;
			balance+=credit;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f</TD>", balance);
			prints(sid, "</TR>\r\n");
			if (i+1>=sql_numtuples(&sqr)) {
				mbalance=mcredit-mdebit;
				tbalance=tcredit-tdebit;
//				prints(sid, "<TR CLASS=\"FIELDVAL\">");
//				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", balance);
//				prints(sid, "</TR>\r\n");
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tbalance);
				prints(sid, "</TR>\r\n");
			} else if (strncmp(monstr, sql_getvalue(&sqr, i+1, 1), 7)!=0) {
				mbalance=mcredit-mdebit;
//				prints(sid, "<TR CLASS=\"FIELDVAL\">");
//				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
//				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", balance);
//				prints(sid, "</TR>\r\n");
				mbalance=0;
				mcredit=0;
				mdebit=0;
				memset(monstr, 0, sizeof(monstr));
				strncpy(monstr, sql_getvalue(&sqr, i+1, 1), sizeof(monstr)-1);
			}
		}
		prints(sid, "</TABLE>\r\n");
	} else {
		prints(sid, "There are no journal entries<BR />\r\n");
	}
	sql_freeresult(&sqr);
	prints(sid, "</CENTER>\r\n");
*/
	return;
}

void account_list(CONN *sid)
{
/*
	int i;
	SQLRES sqr;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_queryf(&sqr, "SELECT accountid, accountname FROM gw_finance_accounts WHERE obj_did = %d ORDER BY accountid ASC", sid->dat->did)<0) return;
	prints(sid, "Found %d accounts<BR />\r\n", sql_numtuples(&sqr));
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Account Name&nbsp;</TH></TR>\r\n");
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' ");
			prints(sid, "onClick=\"window.location.href='%s/finance/accounts/view?accountid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/finance/accounts/view?accountid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr, i, 0)));
			prints(sid, "<TD ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' ");
			prints(sid, "onClick=\"window.location.href='%s/finance/accounts/view?accountid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/finance/accounts/view?accountid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr);
*/
	return;
}
