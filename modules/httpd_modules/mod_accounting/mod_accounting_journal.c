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
#include "mod_accounting.h"

void journal_listraw(CONN *sid)
{
	char monstr[8];
	int i;
	int sqr;
	float balance, credit, debit;
	float mbalance, mcredit, mdebit;
	float tbalance, tcredit, tdebit;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<CENTER>\n");
	if ((sqr=sql_queryf("SELECT journalentryid, entrydate, accountid, debit, credit, details FROM gw_accounting_journal WHERE obj_did = %d ORDER BY entrydate ASC", sid->dat->user_did))<0) return;
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH STYLE='border-style:solid'>id</TH><TH STYLE='border-style:solid'>date</TH><TH STYLE='border-style:solid'>accountid</TH><TH STYLE='border-style:solid'>details</TH><TH STYLE='border-style:solid'>debit</TH><TH STYLE='border-style:solid'>credit</TH><TH STYLE='border-style:solid'>balance</TH></TR>\n");
		balance=0;
		memset(monstr, 0, sizeof(monstr));
		strncpy(monstr, sql_getvalue(sqr, 0, 1), sizeof(monstr)-1);
		mbalance=0;
		mcredit=0;
		mdebit=0;
		tbalance=0;
		tcredit=0;
		tdebit=0;
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 0)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%.10s</TD>", str2html(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 5)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%s</TD>", str2html(sid, sql_getvalue(sqr, i, 3)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%s</TD>", str2html(sid, sql_getvalue(sqr, i, 4)));
			debit=(float)(atof(sql_getvalue(sqr, i, 3)));
			credit=(float)(atof(sql_getvalue(sqr, i, 4)));
			mdebit+=debit;
			mcredit+=credit;
			tdebit+=debit;
			tcredit+=credit;
			balance-=debit;
			balance+=credit;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f</TD>", balance);
			prints(sid, "</TR>\n");
			if (i+1>=sql_numtuples(sqr)) {
				mbalance=mcredit-mdebit;
				tbalance=tcredit-tdebit;
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", balance);
				prints(sid, "</TR>\n");
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tbalance);
				prints(sid, "</TR>\n");
			} else if (strncmp(monstr, sql_getvalue(sqr, i+1, 1), 7)!=0) {
				mbalance=mcredit-mdebit;
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", balance);
				prints(sid, "</TR>\n");
				mbalance=0;
				mcredit=0;
				mdebit=0;
				memset(monstr, 0, sizeof(monstr));
				strncpy(monstr, sql_getvalue(sqr, i+1, 1), sizeof(monstr)-1);
			}
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no journal entries<BR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "</CENTER>\n");
	return;
}

void journal_list(CONN *sid)
{
	char curdate1[21];
	char curdate2[21];
	char monstr[8];
	char *ptemp;
	int i;
	int sqr;
	int sqr2;
	float balance, credit, debit;
	float mbalance, mcredit, mdebit;
	float tbalance, tcredit, tdebit;
	unsigned short int s1, s2, s3;
	unsigned short int e1, e2, e3;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "S1"))!=NULL) s1=atoi(ptemp); else s1=0;
	if ((ptemp=getgetenv(sid, "S2"))!=NULL) s2=atoi(ptemp); else s2=0;
	if ((ptemp=getgetenv(sid, "S3"))!=NULL) s3=atoi(ptemp); else s3=0;
	if ((ptemp=getgetenv(sid, "E1"))!=NULL) e1=atoi(ptemp); else e1=0;
	if ((ptemp=getgetenv(sid, "E2"))!=NULL) e2=atoi(ptemp); else e2=0;
	if ((ptemp=getgetenv(sid, "E3"))!=NULL) e3=atoi(ptemp); else e3=0;
	memset(curdate1, 0, sizeof(curdate1));
	memset(curdate2, 0, sizeof(curdate2));
	snprintf(curdate1, sizeof(curdate1)-1, "%04d-%02d-%02d 00:00:00", s3, s1, s2);
	snprintf(curdate2, sizeof(curdate2)-1, "%04d-%02d-%02d 23:59:59", e3, e1, e2);
	prints(sid, "<CENTER>\n");
	prints(sid, "%.10s - %.10s<BR>\n", curdate1, curdate2);
	if ((sqr=sql_queryf("SELECT journalentryid, entrydate, accountid, debit, credit, details FROM gw_accounting_journal WHERE obj_did = %d AND entrydate >= '%s' AND entrydate <= '%s' ORDER BY entrydate ASC", sid->dat->user_did, curdate1, curdate2))<0) return;
	if ((sqr2=sql_queryf("SELECT SUM(debit), SUM(credit) FROM gw_accounting_journal WHERE obj_did = %d AND entrydate < '%s' ORDER BY entrydate ASC", sid->dat->user_did, curdate1))<0) return;
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH STYLE='border-style:solid'>id</TH><TH STYLE='border-style:solid'>date</TH><TH STYLE='border-style:solid'>accountid</TH><TH STYLE='border-style:solid'>details</TH><TH STYLE='border-style:solid'>debit</TH><TH STYLE='border-style:solid'>credit</TH><TH STYLE='border-style:solid'>balance</TH></TR>\n");
		balance=0;
		memset(monstr, 0, sizeof(monstr));
		strncpy(monstr, sql_getvalue(sqr, 0, 1), sizeof(monstr)-1);
		mbalance=0;
		mcredit=0;
		mdebit=0;
		tbalance=0;
		tcredit=0;
		tdebit=0;

		debit=(float)(atof(sql_getvalue(sqr2, 0, 0)));
		credit=(float)(atof(sql_getvalue(sqr2, 0, 1)));
		balance-=debit;
		balance+=credit;
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", debit);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", credit);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", balance);
		prints(sid, "</TR>\n");

		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 0)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%.10s</TD>", str2html(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr, i, 5)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%s</TD>", str2html(sid, sql_getvalue(sqr, i, 3)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%s</TD>", str2html(sid, sql_getvalue(sqr, i, 4)));
			debit=(float)(atof(sql_getvalue(sqr, i, 3)));
			credit=(float)(atof(sql_getvalue(sqr, i, 4)));
			mdebit+=debit;
			mcredit+=credit;
			tdebit+=debit;
			tcredit+=credit;
			balance-=debit;
			balance+=credit;
//			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f</TD>", balance);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
			prints(sid, "</TR>\n");
			if (i+1>=sql_numtuples(sqr)) {
				mbalance=mcredit-mdebit;
				tbalance=tcredit-tdebit;
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mbalance);
				prints(sid, "</TR>\n");
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tbalance);
				prints(sid, "</TR>\n");
			} else if (strncmp(monstr, sql_getvalue(sqr, i+1, 1), 7)!=0) {
				mbalance=mcredit-mdebit;
				tbalance=tcredit-tdebit;
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=4>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mdebit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mcredit);
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", mbalance);
				prints(sid, "</TR>\n");
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP STYLE='border-style:solid' COLSPAN=6>&nbsp;</TD>");
				prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><B>$%1.2f</B></TD>", tbalance);
				prints(sid, "</TR>\n");
				mbalance=0;
				mcredit=0;
				mdebit=0;
				memset(monstr, 0, sizeof(monstr));
				strncpy(monstr, sql_getvalue(sqr, i+1, 1), sizeof(monstr)-1);
			}
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no journal entries<BR>\n");
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	prints(sid, "</CENTER>\n");
	return;
}

void journal_main(CONN *sid)
{
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	char curdate1[20];
	char curdate2[20];
	char *ptemp;
	short int year;
	short int month;
	time_t t=time(NULL);

	memset(curdate1, 0, sizeof(curdate1));
	memset(curdate2, 0, sizeof(curdate2));
	strftime(curdate1, sizeof(curdate1)-1, "%Y-%m-01", gmtime(&t));
	strftime(curdate2, sizeof(curdate2)-1, "%Y-%m-", gmtime(&t));
	ptemp=curdate1;
	year=atoi(ptemp);
	ptemp=curdate1+5;
	month=atoi(ptemp);
	if ((month==2)&&((year/4.0f)==(int)(year/4))) {
		if (((year/100.0f)!=(int)(year/100))||((year/400.0f)==(int)(year/400))) {
			dim[1]++;
		}
	}
	strncatf(curdate2, sizeof(curdate2)-1, "%02d", dim[month-1]);
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=GET ACTION=%s/accounting/journal/list NAME=journalsearch>\n", sid->dat->in_ScriptName);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Start Date&nbsp;</B></TD><TD ALIGN=RIGHT NOWRAP><SELECT NAME=s1>\n");
	htselect_month(sid, curdate1);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=s2>\n");
	htselect_day(sid, curdate1);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=s3>\n");
	htselect_year(sid, 2000, curdate1);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;End Date&nbsp;</B></TD><TD ALIGN=RIGHT NOWRAP><SELECT NAME=e1>\n");
	htselect_month(sid, curdate2);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=e2>\n");
	htselect_day(sid, curdate2);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=e3>\n");
	htselect_year(sid, 2000, curdate2);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Search'></TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	return;
}
