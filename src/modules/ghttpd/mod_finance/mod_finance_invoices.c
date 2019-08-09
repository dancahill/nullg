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
#include "mod_finance.h"

/****************************************************************************
 *	invoiceedit()
 *
 *	Purpose	: Display edit form for invoice information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoiceedit(CONN *sid)
{
	REC_FIN_INVOICE invoice;
	char *ptemp;
	int invoiceid;

	if (!(auth_priv(sid, "finance")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/finance/invoices/editnew", 23)==0) {
		invoiceid=0;
		if (db_read(sid, 2, DB_INVOICE, 0, &invoice)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) invoice.contactid=atoi(ptemp);
		prints(sid, "<CENTER>\r\n<FORM METHOD=POST ACTION=%s/finance/invoices/save NAME=invoiceedit>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<INPUT TYPE=hidden NAME=invoiceid VALUE='%d'>\r\n", invoice.invoiceid);
		prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP>New Invoice</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Customer</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:217px'>");
		htselect_contact(sid, invoice.contactid);
		prints(sid, "</SELECT></TD></TR>\r\n</TABLE>\r\n");
		prints(sid, "<INPUT TYPE=SUBMIT  CLASS=frmButtonNAME=Submit VALUE='Save'>\r\n</FORM>\r\n");
		prints(sid, "</CENTER>\r\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.invoiceedit.contactid.focus();\r\n// -->\r\n</SCRIPT>\r\n");
		return;
	} else {
		if ((ptemp=getgetenv(sid, "INVOICEID"))==NULL) return;
		invoiceid=atoi(ptemp);
		if (db_read(sid, 2, DB_INVOICE, invoiceid, &invoice)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", invoiceid);
			return;
		}
	}
	invoice.invoicedate+=time_tzoffset(sid, invoice.invoicedate);
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/finance/invoices/save NAME=invoiceedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=invoiceid VALUE='%d'>\r\n", invoice.invoiceid);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\r\n", invoice.userid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/finance/invoices/view?invoiceid=%d>Invoice %d</A></TH></TR>\r\n", sid->dat->in_ScriptName, invoice.invoiceid, invoice.invoiceid);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Invoice Date    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=invoicedate       value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.invoicedate));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Customer        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:217px'>");
	htselect_contact(sid, invoice.contactid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Employee        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=userid style='width:217px'%s>\r\n", (auth_priv(sid, "finance")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, invoice.userid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Invoice Type      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=invoicetype style='width:217px'>");
	prints(sid, "<OPTION%s>Sale", strcmp(invoice.invoicetype, "Sale")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>AR Payment", strcmp(invoice.invoicetype, "AR Payment")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Quote", strcmp(invoice.invoicetype, "Quote")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Method of Payment&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=paymentmethod style='width:217px'>");
	prints(sid, "<OPTION%s>Cash", strcmp(invoice.paymentmethod, "Cash")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Cheque", strcmp(invoice.paymentmethod, "Cheque")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Credit Card", strcmp(invoice.paymentmethod, "Credit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Debit Card", strcmp(invoice.paymentmethod, "Debit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Money Order", strcmp(invoice.paymentmethod, "Money Order")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:217px'>\r\n");
	htselect_eventstatus(sid, invoice.status);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, invoice.details));
	prints(sid, "<TR><TD ALIGN=center COLSPAN=2>\r\n");
	invoiceitemlist(sid, invoiceid);
	prints(sid, "</TD></TR>");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;SubTotal&nbsp;$</B><INPUT TYPE=TEXT NAME=subtotal        value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\r\n", invoice.paymentdue);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;Net Due&nbsp;$</B><INPUT  TYPE=TEXT NAME=paymentdue      value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\r\n", invoice.paymentdue);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;Received&nbsp;$</B><INPUT TYPE=TEXT NAME=paymentreceived value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\r\n", invoice.paymentreceived);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.invoiceedit.invoicedate.focus();\r\n// -->\r\n</SCRIPT>\r\n");
}

/****************************************************************************
 *	invoiceview()
 *
 *	Purpose	: Display invoice and invoice item information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoiceview(CONN *sid)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_FIN_INVOICE invoice;
	char *ptemp;
	int invoiceid;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "INVOICEID"))==NULL) return;
	invoiceid=atoi(ptemp);
	if (db_read(sid, 1, DB_INVOICE, invoiceid, &invoice)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", invoiceid);
		return;
	}
	invoice.invoicedate+=time_tzoffset(sid, invoice.invoicedate);
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400><TR><TD><CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Invoice %d", invoiceid);
	if (auth_priv(sid, "finance")&A_MODIFY) {
		prints(sid, " [<A HREF=%s/finance/invoices/edit?invoiceid=%d>edit</A>]", sid->dat->in_ScriptName, invoiceid);
	}
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Invoice Date</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", time_unix2text(sid, invoice.invoicedate));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Customer  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
	prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, invoice.contactid);
	prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", htview_contact(sid, invoice.contactid));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Employee         </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, htview_user(sid, invoice.userid)));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Invoice Type     </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, invoice.invoicetype));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Method of Payment</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, invoice.paymentmethod));
	if (strlen(invoice.details)) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" NOWRAP COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\r\n", str2html(sid, invoice.details));
	}
	prints(sid, "<TR STYLE='spacing:0px'><TD NOWRAP COLSPAN=2 STYLE='padding:0px; border-style:solid'>");
	invoiceitemlist(sid, invoiceid);
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>SubTotal</B> $%1.2f&nbsp;</TD></TR>\r\n", invoice.paymentdue);
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>Net Due </B> $%1.2f&nbsp;</TD></TR>\r\n", invoice.paymentdue);
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>Received</B> $%1.2f&nbsp;</TD></TR>\r\n", invoice.paymentreceived);
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=invoices&index=%d>new</A>]", sid->dat->in_ScriptName, invoice.invoiceid);
		prints(sid, "</TH></TR>\r\n");
		mod_notes_sublist(sid, "invoices", invoice.invoiceid, 2);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "</CENTER></TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
}

/****************************************************************************
 *	invoicelist()
 *
 *	Purpose	: Display a list of invoices matching search criteria
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoicelist(CONN *sid)
{
	char *ptemp;
	char column[32];
	char query[8192];
	char searchstring[100];
	int offset=0;
	int i;
	int j;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) {
		prints(sid, "<CENTER>\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/finance/invoices/list NAME=invoicesearch>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<TR><TH COLSPAN=3>Invoice Search Form</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><SELECT NAME=column>\r\n");
		prints(sid, "<OPTION SELECTED>All Columns\r\n");
		if (sql_query(&sqr1, "SELECT * FROM gw_finance_invoices WHERE invoiceid = 1")<0) return;
		for (i=0;i<sql_numfields(&sqr1);i++) {
			if (strcmp(sql_getname(&sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_operm")==0) continue;
			prints(sid, "<OPTION>%s\r\n", sql_getname(&sqr1, i));
		}
		sql_freeresult(&sqr1);
		prints(sid, "</SELECT></TD>\r\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\r\n");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\r\n");
		prints(sid, "</FORM></TABLE>\r\n");
		if (auth_priv(sid, "finance")&A_INSERT) {
			prints(sid, "<A HREF=%s/finance/invoices/editnew>New Invoice</A>\r\n", sid->dat->in_ScriptName);
		}
		prints(sid, "</CENTER>\r\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>document.invoicesearch.searchstring.focus();</SCRIPT>\r\n");
		return;
	}
	memset(searchstring, 0, sizeof(searchstring));
	if ((ptemp=getpostenv(sid, "SEARCHSTRING"))!=NULL) {
		strncpy(searchstring, ptemp, sizeof(searchstring)-1);
	}
	ptemp=searchstring;
	while (*ptemp) {
		if (*ptemp=='*') *ptemp='%';
		if (*ptemp=='?') *ptemp='_';
		ptemp++;
	}
	if (sql_query(&sqr1, "SELECT * FROM gw_finance_invoices WHERE invoiceid = 1")<0) return;
	strcpy(query, "SELECT invoiceid, contactid, invoicedate, paymentdue, paymentreceived from gw_finance_invoices WHERE (");
	memset(column, 0, sizeof(column));
	if ((ptemp=getpostenv(sid, "COLUMN"))!=NULL) {
		snprintf(column, sizeof(column)-1, "%s", ptemp);
	}
	if (strcmp(ptemp, "All Columns")==0) {
		for (i=0;i<sql_numfields(&sqr1);i++) {
			if (strcmp(sql_getname(&sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_operm")==0) continue;
			if (strcmp(config->sql_type, "ODBC")==0) {
				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(&sqr1, i), searchstring);
			} else if (strcmp(sql_getname(&sqr1, i), "paymentdue")==0) {
//				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(&sqr1, i), searchstring);
			} else if (strcmp(sql_getname(&sqr1, i), "paymentreceived")==0) {
//				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(&sqr1, i), searchstring);
			} else {
				strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(&sqr1, i), searchstring);
			}
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "invoiceid like '%s'", searchstring);
	} else {
		if (strcmp(config->sql_type, "ODBC")==0) {
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", ptemp, searchstring);
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, searchstring);
		}
	}
	strncatf(query, sizeof(query)-strlen(query)-1, ") AND obj_did = %d ORDER BY invoicedate DESC", sid->dat->user_did);
	sql_freeresult(&sqr1);
	prints(sid, "<CENTER>\r\n");
	if (sql_query(&sqr1, query)<0) return;
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	prints(sid, "Found %d matching invoices<BR>\r\n", sql_numtuples(&sqr1));
	if (sql_numtuples(&sqr1)<1) {
		prints(sid, "<A HREF=%s/finance/invoices/editnew>New Invoice</A>\r\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\r\n");
		sql_freeresult(&sqr1);
		return;
	}
	if (sql_query(&sqr2, "SELECT contactid, surname, givenname FROM gw_contacts")<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n<TR>");
	prints(sid, "<TH NOWRAP STYLE='border-style:solid'>Invoice ID</TH><TH NOWRAP STYLE='border-style:solid'>Customer Name</TH><TH NOWRAP STYLE='border-style:solid'>Invoice Date</TH><TH NOWRAP STYLE='border-style:solid'>Amount Due</TH><TH NOWRAP STYLE='border-style:solid'>Amount Received</TH></TR>\r\n");
	for (i=offset;(i<sql_numtuples(&sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD ALIGN=right NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/finance/invoices/view?invoiceid=%d'\"><A HREF=%s/finance/invoices/view?invoiceid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), atoi(sql_getvalue(&sqr1, i, 0)));
		for (j=0;j<sql_numtuples(&sqr2);j++) {
			if (atoi(sql_getvalue(&sqr2, j, 0))==atoi(sql_getvalue(&sqr1, i, 1))) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(&sqr2, j, 0), str2html(sid, sql_getvalue(&sqr2, j, 1)));
				if (strlen(sql_getvalue(&sqr2, j, 1))&&strlen(sql_getvalue(&sqr2, j, 2))) prints(sid, ", ");
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr2, j, 2)));
				break;
			}
		}
		if (j==sql_numtuples(&sqr2)) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(&sqr1, i, 2)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr1, i, 3)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr1, i, 4)));
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n<TABLE><TR>\r\n");
	if (offset>(sid->dat->user_maxlist-1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/finance/invoices/list?offset=%d><TD>\r\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\r\n", column);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\r\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Previous %d'>\r\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\r\n");
	}
	if (offset+sid->dat->user_maxlist<sql_numtuples(&sqr1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/finance/invoices/list?offset=%d><TD>\r\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\r\n", column);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\r\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Next %d'>\r\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\r\n");
	}
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "<A HREF=%s/finance/invoices/editnew>New Invoice</A>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
	sql_freeresult(&sqr2);
	return;
}

/****************************************************************************
 *	invoicesave()
 *
 *	Purpose	: Save invoice information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoicesave(CONN *sid)
{
	REC_FIN_INVOICE invoice;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int invoiceid;
	SQLRES sqr;

	if (!(auth_priv(sid, "finance")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "INVOICEID"))==NULL) return;
	invoiceid=atoi(ptemp);
	if (db_read(sid, 2, DB_INVOICE, invoiceid, &invoice)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) invoice.contactid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "USERID"))!=NULL) invoice.userid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "INVOICEDATE"))!=NULL) invoice.invoicedate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "INVOICETYPE"))!=NULL) snprintf(invoice.invoicetype, sizeof(invoice.invoicetype)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTMETHOD"))!=NULL) snprintf(invoice.paymentmethod, sizeof(invoice.paymentmethod)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTDUE"))!=NULL) invoice.paymentdue=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTRECEIVED"))!=NULL) invoice.paymentreceived=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) invoice.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(invoice.details, sizeof(invoice.details)-1, "%s", ptemp);
	invoice.invoicedate-=time_tzoffset(sid, invoice.invoicedate);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "finance")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_finance_invoices WHERE invoiceid = %d", invoice.invoiceid)<0) return;
		prints(sid, "<CENTER>Invoice %d deleted successfully</CENTER><BR>\r\n", invoice.invoiceid);
		db_log_activity(sid, "invoices", invoice.invoiceid, "delete", "%s - %s deleted invoice %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoice.invoiceid);
	} else if (invoice.invoiceid==0) {
		if (!(auth_priv(sid, "finance")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_query(&sqr, "SELECT max(invoiceid) FROM gw_finance_invoices")<0) return;
		invoice.invoiceid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (invoice.invoiceid<1) invoice.invoiceid=1;
		strcpy(query, "INSERT INTO gw_finance_invoices (invoiceid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, contactid, userid, invoicedate, invoicetype, paymentmethod, paymentdue, paymentreceived, status, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', ", invoice.invoiceid, curdate, curdate, sid->dat->user_uid, sid->dat->user_did);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", invoice.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", invoice.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.invoicedate));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.invoicetype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", invoice.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", invoice.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", invoice.status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.details));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Invoice %d added successfully</CENTER><BR>\r\n", invoice.invoiceid);
		db_log_activity(sid, "invoices", invoice.invoiceid, "insert", "%s - %s added invoice %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoice.invoiceid);
	} else {
		if (!(auth_priv(sid, "finance")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_finance_invoices SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, invoice.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", invoice.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "userid = %d, ", invoice.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "invoicedate = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.invoicedate));
		strncatf(query, sizeof(query)-strlen(query)-1, "invoicetype = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.invoicetype));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentmethod = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentdue = '%1.2f', ", invoice.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentreceived = '%1.2f', ", invoice.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "status = %d, ", invoice.status);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, invoice.details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE invoiceid = %d", invoice.invoiceid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Invoice %d modified successfully</CENTER><BR>\r\n", invoice.invoiceid);
		db_log_activity(sid, "invoices", invoice.invoiceid, "modify", "%s - %s modified invoice %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoice.invoiceid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "location.replace(\"%s/finance/invoices/view?invoiceid=%d\");\r\n", sid->dat->in_ScriptName, invoice.invoiceid);
	prints(sid, "// -->\r\n</SCRIPT>\r\n<NOSCRIPT>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/finance/invoices/view?invoiceid=%d\">\r\n", sid->dat->in_ScriptName, invoice.invoiceid);
	prints(sid, "</NOSCRIPT>\r\n");
	return;
}
