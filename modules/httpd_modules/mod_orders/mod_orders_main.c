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
#define SRVMOD_MAIN 1
#include "http_mod.h"
#include "mod_orders.h"

/****************************************************************************
 *	orderedit()
 *
 *	Purpose	: Display edit form for order information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderedit(CONN *sid)
{
	REC_ORDER order;
	char *ptemp;
	int orderid;

	if (!(auth_priv(sid, "orders")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/orders/editnew", 15)==0) {
		orderid=0;
		if (db_read(sid, 2, DB_ORDERS, 0, &order)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) order.contactid=atoi(ptemp);
		prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/orders/save NAME=orderedit>\n", sid->dat->in_ScriptName);
		prints(sid, "<INPUT TYPE=hidden NAME=orderid VALUE='%d'>\n", order.orderid);
		prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP>New Order</TH></TR>\n");
		prints(sid, "<TR><TD NOWRAP><B>Customer</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:217px'>");
		htselect_contact(sid, order.contactid);
		prints(sid, "</SELECT></TD></TR>\n</TABLE>\n");
		prints(sid, "<INPUT TYPE=SUBMIT  CLASS=frmButtonNAME=Submit VALUE='Save'>\n</FORM>\n");
		prints(sid, "</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.orderedit.contactid.focus();\n// -->\n</SCRIPT>\n");
		return;
	} else {
		if ((ptemp=getgetenv(sid, "ORDERID"))==NULL) return;
		orderid=atoi(ptemp);
		if (db_read(sid, 2, DB_ORDERS, orderid, &order)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", orderid);
			return;
		}
	}
	order.orderdate+=time_tzoffset(sid, order.orderdate);
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/orders/save NAME=orderedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=orderid VALUE='%d'>\n", order.orderid);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", order.userid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/orders/view?orderid=%d>Order %d</A></TH></TR>\n", sid->dat->in_ScriptName, order.orderid, order.orderid);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Order Date      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=orderdate       value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.orderdate));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Customer        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:217px'>");
	htselect_contact(sid, order.contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Employee        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=userid style='width:217px'%s>\n", (auth_priv(sid, "orders")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, order.userid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Order Type      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=ordertype style='width:217px'>");
	prints(sid, "<OPTION%s>Sale", strcmp(order.ordertype, "Sale")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>AR Payment", strcmp(order.ordertype, "AR Payment")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Quote", strcmp(order.ordertype, "Quote")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Method of Payment&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=paymentmethod style='width:217px'>");
	prints(sid, "<OPTION%s>Cash", strcmp(order.paymentmethod, "Cash")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Cheque", strcmp(order.paymentmethod, "Cheque")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Credit Card", strcmp(order.paymentmethod, "Credit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Debit Card", strcmp(order.paymentmethod, "Debit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Money Order", strcmp(order.paymentmethod, "Money Order")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:217px'>\n");
	htselect_eventstatus(sid, order.status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", str2html(sid, order.details));
	prints(sid, "<TR><TD ALIGN=center COLSPAN=2>\n");
	orderitemlist(sid, orderid);
	prints(sid, "</TD></TR>");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;SubTotal&nbsp;$</B><INPUT TYPE=TEXT NAME=subtotal        value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\n", order.paymentdue);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;Net Due&nbsp;$</B><INPUT  TYPE=TEXT NAME=paymentdue      value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\n", order.paymentdue);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=right COLSPAN=2 NOWRAP><B>&nbsp;Received&nbsp;$</B><INPUT TYPE=TEXT NAME=paymentreceived value=\"%1.2f\" SIZE=10 style='width:76px'></TD></TR>\n", order.paymentreceived);
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.orderedit.orderdate.focus();\n// -->\n</SCRIPT>\n");
}

/****************************************************************************
 *	orderview()
 *
 *	Purpose	: Display order and order item information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderview(CONN *sid)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_ORDER order;
	char *ptemp;
	int orderid;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "ORDERID"))==NULL) return;
	orderid=atoi(ptemp);
	if (db_read(sid, 1, DB_ORDERS, orderid, &order)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", orderid);
		return;
	}
	order.orderdate+=time_tzoffset(sid, order.orderdate);
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400><TR><TD><CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Order %d", orderid);
	if (auth_priv(sid, "orders")&A_MODIFY) {
		prints(sid, " [<A HREF=%s/orders/edit?orderid=%d>edit</A>]", sid->dat->in_ScriptName, orderid);
	}
	prints(sid, "</TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Order Date</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", time_unix2text(sid, order.orderdate));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Customer  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
	prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, order.contactid);
	prints(sid, "%s</A>&nbsp;</TD></TR>\n", htview_contact(sid, order.contactid));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Employee         </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", str2html(sid, htview_user(sid, order.userid)));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Order Type       </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", str2html(sid, order.ordertype));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Method of Payment</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", str2html(sid, order.paymentmethod));
	if (strlen(order.details)) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" NOWRAP COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", str2html(sid, order.details));
	}
	prints(sid, "<TR STYLE='spacing:0px'><TD NOWRAP COLSPAN=2 STYLE='padding:0px; border-style:solid'>");
	orderitemlist(sid, orderid);
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>SubTotal</B> $%1.2f&nbsp;</TD></TR>\n", order.paymentdue);
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>Net Due </B> $%1.2f&nbsp;</TD></TR>\n", order.paymentdue);
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" ALIGN=right COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><B>Received</B> $%1.2f&nbsp;</TD></TR>\n", order.paymentreceived);
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=orders&index=%d>new</A>]", sid->dat->in_ScriptName, order.orderid);
		prints(sid, "</TH></TR>\n");
		mod_notes_sublist(sid, "orders", order.orderid, 2);
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "</CENTER></TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
}

/****************************************************************************
 *	orderlist()
 *
 *	Purpose	: Display a list of orders matching search criteria
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderlist(CONN *sid)
{
	char *ptemp;
	char column[32];
	char query[8192];
	char searchstring[100];
	int offset=0;
	int i;
	int j;
	int sqr1;
	int sqr2;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) {
		prints(sid, "<CENTER>\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list NAME=ordersearch>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR><TH COLSPAN=3>Order System Search Form</TH></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><SELECT NAME=column>\n");
		prints(sid, "<OPTION SELECTED>All Columns\n");
		if ((sqr1=sql_query("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
		for (i=0;i<sql_numfields(sqr1);i++) {
			if (strcmp(sql_getname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_operm")==0) continue;
			prints(sid, "<OPTION>%s\n", sql_getname(sqr1, i));
		}
		sql_freeresult(sqr1);
		prints(sid, "</SELECT></TD>\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\n");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\n");
		prints(sid, "</FORM></TABLE>\n");
		if (auth_priv(sid, "orders")&A_INSERT) {
			prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
		}
		prints(sid, "</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>document.ordersearch.searchstring.focus();</SCRIPT>\n");
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
	if ((sqr1=sql_query("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
	strcpy(query, "SELECT orderid, contactid, orderdate, paymentdue, paymentreceived from gw_orders WHERE (");
	memset(column, 0, sizeof(column));
	if ((ptemp=getpostenv(sid, "COLUMN"))!=NULL) {
		snprintf(column, sizeof(column)-1, "%s", ptemp);
	}
	if (strcmp(ptemp, "All Columns")==0) {
		for (i=0;i<sql_numfields(sqr1);i++) {
			if (strcmp(sql_getname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_operm")==0) continue;
			if (strcmp(config->sql_type, "ODBC")==0) {
				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(sqr1, i), searchstring);
			} else if (strcmp(sql_getname(sqr1, i), "paymentdue")==0) {
//				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(sqr1, i), searchstring);
			} else if (strcmp(sql_getname(sqr1, i), "paymentreceived")==0) {
//				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(sqr1, i), searchstring);
			} else {
				strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(sqr1, i), searchstring);
			}
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "orderid like '%s'", searchstring);
	} else {
		if (strcmp(config->sql_type, "ODBC")==0) {
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", ptemp, searchstring);
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, searchstring);
		}
	}
	strncatf(query, sizeof(query)-strlen(query)-1, ") AND obj_did = %d ORDER BY orderdate DESC", sid->dat->user_did);
	sql_freeresult(sqr1);
	prints(sid, "<CENTER>\n");
	if ((sqr1=sql_query(query))<0) return;
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	prints(sid, "Found %d matching orders<BR>\n", sql_numtuples(sqr1));
	if (sql_numtuples(sqr1)<1) {
		prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\n");
		sql_freeresult(sqr1);
		return;
	}
	if ((sqr2=sql_query("SELECT contactid, surname, givenname FROM gw_contacts"))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n<TR>");
	prints(sid, "<TH NOWRAP STYLE='border-style:solid'>Order ID</TH><TH NOWRAP STYLE='border-style:solid'>Customer Name</TH><TH NOWRAP STYLE='border-style:solid'>Order Date</TH><TH NOWRAP STYLE='border-style:solid'>Amount Due</TH><TH NOWRAP STYLE='border-style:solid'>Amount Received</TH></TR>\n");
	for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD ALIGN=right NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/orders/view?orderid=%d'\"><A HREF=%s/orders/view?orderid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), atoi(sql_getvalue(sqr1, i, 0)));
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 1))) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(sqr2, j, 0), str2html(sid, sql_getvalue(sqr2, j, 1)));
				if (strlen(sql_getvalue(sqr2, j, 1))&&strlen(sql_getvalue(sqr2, j, 2))) prints(sid, ", ");
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr2, j, 2)));
				break;
			}
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(sqr1, i, 2)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr1, i, 3)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr1, i, 4)));
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n<TABLE><TR>\n");
	if (offset>(sid->dat->user_maxlist-1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list?offset=%d><TD>\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", column);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Previous %d'>\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\n");
	}
	if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list?offset=%d><TD>\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", column);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Next %d'>\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\n");
	}
	prints(sid, "</TR></TABLE>\n");
	prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr1);
	sql_freeresult(sqr2);
	return;
}

/****************************************************************************
 *	ordersave()
 *
 *	Purpose	: Save order information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void ordersave(CONN *sid)
{
	REC_ORDER order;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int orderid;
	int sqr;

	if (!(auth_priv(sid, "orders")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ORDERID"))==NULL) return;
	orderid=atoi(ptemp);
	if (db_read(sid, 2, DB_ORDERS, orderid, &order)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) order.contactid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "USERID"))!=NULL) order.userid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ORDERDATE"))!=NULL) order.orderdate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "ORDERTYPE"))!=NULL) snprintf(order.ordertype, sizeof(order.ordertype)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTMETHOD"))!=NULL) snprintf(order.paymentmethod, sizeof(order.paymentmethod)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTDUE"))!=NULL) order.paymentdue=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "PAYMENTRECEIVED"))!=NULL) order.paymentreceived=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) order.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(order.details, sizeof(order.details)-1, "%s", ptemp);
	order.orderdate-=time_tzoffset(sid, order.orderdate);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "orders")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_orders WHERE orderid = %d", order.orderid)<0) return;
		prints(sid, "<CENTER>Order %d deleted successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, "orders", order.orderid, "delete", "%s - %s deleted order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else if (order.orderid==0) {
		if (!(auth_priv(sid, "orders")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_query("SELECT max(orderid) FROM gw_orders"))<0) return;
		order.orderid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (order.orderid<1) order.orderid=1;
		strcpy(query, "INSERT INTO gw_orders (orderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, contactid, userid, orderdate, ordertype, paymentmethod, paymentdue, paymentreceived, status, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', ", order.orderid, curdate, curdate, sid->dat->user_uid, sid->dat->user_did);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", order.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", order.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.orderdate));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.ordertype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", order.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", order.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", order.status);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.details));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Order %d added successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, "orders", order.orderid, "insert", "%s - %s added order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, order.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", order.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "userid = %d, ", order.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "orderdate = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.orderdate));
		strncatf(query, sizeof(query)-strlen(query)-1, "ordertype = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.ordertype));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentmethod = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentdue = '%1.2f', ", order.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentreceived = '%1.2f', ", order.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "status = %d, ", order.status);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, order.details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE orderid = %d", order.orderid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Order %d modified successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, "orders", order.orderid, "modify", "%s - %s modified order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/orders/view?orderid=%d\");\n", sid->dat->in_ScriptName, order.orderid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orders/view?orderid=%d\">\n", sid->dat->in_ScriptName, order.orderid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

/****************************************************************************
 *	ordermain()
 *
 *	Purpose	: Execute appropriate order function based on URI
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
DllExport int mod_main(CONN *sid)
{
	char *suburi=sid->dat->in_RequestURI;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ORDERS);
	prints(sid, "<BR>\r\n");
	if (strncmp(suburi, "/orders/", 8)!=0) return 0;
	suburi+=8;
	if (strncmp(suburi, "edit", 4)==0) {
		orderedit(sid);
	} else if (strncmp(suburi, "view", 4)==0) {
		orderview(sid);
	} else if (strncmp(suburi, "list", 4)==0) {
		orderlist(sid);
	} else if (strncmp(suburi, "save", 4)==0) {
		ordersave(sid);
	} else if (strncmp(suburi, "itemedit", 8)==0) {
		orderitemedit(sid);
	} else if (strncmp(suburi, "itemsave", 8)==0) {
		orderitemsave(sid);
	} else if (strncmp(suburi, "productedit", 10)==0) {
		productedit(sid);
	} else if (strncmp(suburi, "productview", 10)==0) {
		productview(sid);
	} else if (strncmp(suburi, "productlist", 10)==0) {
		productlist(sid);
	} else if (strncmp(suburi, "productsave", 10)==0) {
		productsave(sid);
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_orders",		// mod_name
		2,			// mod_submenu
		"ORDERS",		// mod_menuname
		"/orders/list",		// mod_menuuri
		"orders",		// mod_menuperm
		"mod_main",		// fn_name
		"/orders/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
