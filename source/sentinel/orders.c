/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

/****************************************************************************
 *	orderdetailedit()
 *
 *	Purpose	: Display edit form for order detail information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderdetailedit(int sid)
{
	int orderdetailid;
	int i;
	int sqr;

	if (auth(sid, "orders")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strncmp(conn[sid].dat->in_RequestURI, "/orderdetaileditnew", 19)==0) {
		orderdetailid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_orderdetails WHERE orderdetailid = 1"))<0) return;
		strcpy(conn[sid].dat->tuple[0].value, "0");
		strncpy(conn[sid].dat->tuple[0].name, sqlGetfieldname(sqr, 0), sizeof(conn[sid].dat->tuple[0].name)-1);
		for (i=1;i<sqlNumfields(sqr);i++) {
			conn[sid].dat->tuple[i].value[0]='\0';
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[0].name)-1);
			if ((strcmp(conn[sid].dat->tuple[i].name, "orderid")==0)&&(getgetenv(sid, "ORDERID")!=NULL)) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", getgetenv(sid, "ORDERID"));
			}
			if (strcmp(conn[sid].dat->tuple[i].name, "quantity")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1");
			}
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
	} else {
		if (getgetenv(sid, "ORDERDETAILID")==NULL) return;
		orderdetailid=atoi(getgetenv(sid, "ORDERDETAILID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_orderdetails WHERE orderdetailid = %d", orderdetailid))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		sqlFreeconnect(sqr);
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/orderdetailsave NAME=detailedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=orderdetailid VALUE='%s'>\n", field(ORDERDETAILFIELDS, "orderdetailid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(ORDERDETAILFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(ORDERDETAILFIELDS, "mdatetime"));
	prints("<INPUT TYPE=hidden NAME=orderid VALUE='%s'>\n", field(ORDERDETAILFIELDS, "orderid"));
	prints("<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Order %s - ", COLOR_TRIM, field(ORDERDETAILFIELDS, "orderid"));
	if (orderdetailid>0) {
		prints("Item %d</TH></TR>\n", orderdetailid);
	} else {
		prints("New Item</TH></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Product      </B></TD><TD><SELECT NAME=productname WIDTH=25>", COLOR_EDITFORM);
	productselect(sid, field(ORDERDETAILFIELDS, "productname"));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Quantity     </B></TD><TD><INPUT TYPE=TEXT NAME=quantity     value='%s' SIZE=20></TD></TR>\n", COLOR_EDITFORM, field(ORDERDETAILFIELDS, "quantity"));
	if (strncmp(conn[sid].dat->in_RequestURI, "/orderdetaileditnew", 19)!=0) {
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>Discount %%  </B></TD><TD><INPUT TYPE=TEXT NAME=discount     value='%1.2f' SIZE=20></TD></TR>\n", COLOR_EDITFORM, atof(field(ORDERDETAILFIELDS, "discount")));
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>Unit Price   </B></TD><TD><INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=20></TD></TR>\n", COLOR_EDITFORM, atof(field(ORDERDETAILFIELDS, "unitprice")));
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>Internal Cost</B></TD><TD><INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=20></TD></TR>\n", COLOR_EDITFORM, atof(field(ORDERDETAILFIELDS, "internalcost")));
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>%s           </B></TD><TD><INPUT TYPE=TEXT NAME=tax1         value='%1.2f' SIZE=20></TD></TR>\n", COLOR_EDITFORM, config.info.tax1name, atof(field(ORDERDETAILFIELDS, "tax1")));
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>%s           </B></TD><TD><INPUT TYPE=TEXT NAME=tax2         value='%1.2f' SIZE=20></TD></TR>\n", COLOR_EDITFORM, config.info.tax2name, atof(field(ORDERDETAILFIELDS, "tax2")));
	}
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth(sid, "orders")>2)&&(orderdetailid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.detailedit.productname.focus();\n// -->\n</SCRIPT>\n");
}

/****************************************************************************
 *	orderedit()
 *
 *	Purpose	: Display edit form for order information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderedit(int sid)
{
	float subtotal;
	float totaldue;
	time_t t;
	int i;
	int orderid;
	int sqr;
	float due;
	float received;

	if (auth(sid, "orders")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(conn[sid].dat->tuple, 0, sizeof(conn[sid].dat->tuple));
	if (strncmp(conn[sid].dat->in_RequestURI, "/ordereditnew", 13)==0) {
		orderid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			snprintf(conn[sid].dat->tuple[i].name, sizeof(conn[sid].dat->tuple[i].name)-1, "%s", sqlGetfieldname(sqr, i));
		}
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", 0);
		for (i=1;i<sqlNumfields(sqr);i++) {
			if ((strcmp(conn[sid].dat->tuple[i].name, "customerid")==0)&&(getgetenv(sid, "CUSTOMERID")!=NULL)) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", getgetenv(sid, "CUSTOMERID"));
			}
			if (strcmp(conn[sid].dat->tuple[i].name, "employee")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
			} else if (strcmp(conn[sid].dat->tuple[i].name, "orderdate")==0) {
				t=time((time_t*)0);
				strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
			} else if (strcmp(conn[sid].dat->tuple[i].name, "ordertype")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "Sale");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "paymentmethod")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "Cash");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "paymentdue")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0.00");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "paymentreceived")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0.00");
			}
		}
		sqlFreeconnect(sqr);
		due=(int)(atof(field(ORDERFIELDS, "paymentdue"))*100.0F)/100.0F;
		received=(int)(atof(field(ORDERFIELDS, "paymentreceived"))*100.0F)/100.0F;
		prints("<CENTER>\n");
		prints("<FORM METHOD=POST ACTION=%s/ordersave NAME=orderedit>\n", conn[sid].dat->in_ScriptName);
		prints("<INPUT TYPE=hidden NAME=orderid VALUE='%s'>\n", field(ORDERFIELDS, "orderid"));
		prints("<INPUT TYPE=hidden NAME=employee        VALUE='%s'>\n", field(ORDERFIELDS, "employee"));
		prints("<INPUT TYPE=hidden NAME=orderdate       VALUE='%s'>\n", field(ORDERFIELDS, "orderdate"));
		prints("<INPUT TYPE=hidden NAME=ordertype       VALUE='%s'>\n", field(ORDERFIELDS, "ordertype"));
		prints("<INPUT TYPE=hidden NAME=paymentmethod   VALUE='%s'>\n", field(ORDERFIELDS, "paymentmethod"));
		prints("<INPUT TYPE=hidden NAME=paymentdue      VALUE='%1.2f'>\n", due);
		prints("<INPUT TYPE=hidden NAME=paymentreceived VALUE='%1.2f'>\n", received);
		prints("<INPUT TYPE=hidden NAME=notes           VALUE='%s'>\n", field(ORDERFIELDS, "notes"));
		prints("<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
		prints("<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP>Order Manager - New Order</TH></TR>\n", COLOR_TRIM);
		prints("<TR BGCOLOR=%s><TD NOWRAP><B>Customer ID</B></TD><TD ALIGN=RIGHT><SELECT NAME=customerid WIDTH=25>", COLOR_EDITFORM);
		contactselect(sid, atoi(field(ORDERFIELDS, "customerid")));
		prints("</SELECT></TD></TR>\n");
		prints("</TABLE>\n");
		prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
		prints("</FORM>\n");
		prints("</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.orderedit.customerid.focus();\n// -->\n</SCRIPT>\n");
		return;
	}
	if (getgetenv(sid, "ORDERID")==NULL) return;
	orderid=atoi(getgetenv(sid, "ORDERID"));
	totaldue=0;
	if ((sqr=sqlQueryf("SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_orderdetails WHERE orderid = %d", orderid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		for (i=0;i<sqlNumtuples(sqr);i++) {
			subtotal=(float)(atof(sqlGetvalue(sqr, i, 0))*atof(sqlGetvalue(sqr, i, 1)));
			subtotal=(float)(subtotal-subtotal*atof(sqlGetvalue(sqr, i, 2)));
			subtotal=(float)(subtotal+subtotal*atof(sqlGetvalue(sqr, i, 3))+subtotal*atof(sqlGetvalue(sqr, i, 4)));
			totaldue+=subtotal;
		}
		totaldue*=100.0f;
		if ((float)totaldue-(int)totaldue>0.5) totaldue++;
		totaldue=(int)totaldue/100.0f;
	}
	sqlFreeconnect(sqr);
	sqlUpdatef("UPDATE gw_orders SET paymentdue = '%1.2f' WHERE orderid = %d", totaldue, orderid);
	if ((sqr=sqlQueryf("SELECT * FROM gw_orders WHERE orderid = %d", orderid))<0) return;
	if (sqlNumtuples(sqr)<1) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		snprintf(conn[sid].dat->tuple[i].name, sizeof(conn[sid].dat->tuple[i].name)-1, "%s", sqlGetfieldname(sqr, i));
		snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, i));
	}
	sqlFreeconnect(sqr);
	due=(int)(atof(field(ORDERFIELDS, "paymentdue"))*100.0F)/100.0F;
	received=(int)(atof(field(ORDERFIELDS, "paymentreceived"))*100.0F)/100.0F;
	prints("<CENTER>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD><CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/ordersave NAME=orderedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=orderid VALUE='%s'>\n", field(ORDERFIELDS, "orderid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(ORDERFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(ORDERFIELDS, "mdatetime"));
	prints("<INPUT TYPE=hidden NAME=employee VALUE='%s'>\n", field(ORDERFIELDS, "employee"));
	prints("<TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Order %d</TH></TR>\n", COLOR_TRIM, orderid);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Order Date      </B></TD><TD><INPUT TYPE=TEXT NAME=orderdate       value='%s' SIZE=25></TD></TR>\n", COLOR_EDITFORM, field(ORDERFIELDS, "orderdate"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Customer ID     </B></TD><TD><SELECT NAME=customerid WIDTH=25>", COLOR_EDITFORM);
	contactselect(sid, atoi(field(ORDERFIELDS, "customerid")));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Order Type      </B></TD><TD><SELECT NAME=ordertype>", COLOR_EDITFORM);
	if (strcmp(field(ORDERFIELDS, "ordertype"), "AR Payment")==0) {
		prints("<OPTION>Sale<OPTION SELECTED>AR Payment<OPTION>Quote");
	} else if (strcmp(field(ORDERFIELDS, "ordertype"), "Quote")==0) {
		prints("<OPTION>Sale<OPTION>AR Payment<OPTION SELECTED>Quote");
	} else {
		prints("<OPTION SELECTED>Sale<OPTION>AR Payment<OPTION>Quote");
	}
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Payment Method  </B></TD><TD><SELECT NAME=paymentmethod>", COLOR_EDITFORM);
	if (strcmp(field(ORDERFIELDS, "paymentmethod"), "Cheque")==0) {
		prints("<OPTION>Cash<OPTION SELECTED>Cheque<OPTION>Credit Card<OPTION>Debit Card<OPTION>Money Order");
	} else if (strcmp(field(ORDERFIELDS, "paymentmethod"), "Credit Card")==0) {
		prints("<OPTION>Cash<OPTION>Cheque<OPTION SELECTED>Credit Card<OPTION>Debit Card<OPTION>Money Order");
	} else if (strcmp(field(ORDERFIELDS, "paymentmethod"), "Debit Card")==0) {
		prints("<OPTION>Cash<OPTION>Cheque<OPTION>Credit Card<OPTION SELECTED>Debit Card<OPTION>Money Order");
	} else if (strcmp(field(ORDERFIELDS, "paymentmethod"), "Money Order")==0) {
		prints("<OPTION>Cash<OPTION>Cheque<OPTION>Credit Card<OPTION>Debit Card<OPTION SELECTED>Money Order");
	} else {
		prints("<OPTION SELECTED>Cash<OPTION>Cheque<OPTION>Credit Card<OPTION>Debit Card<OPTION>Money Order");
	}
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Payment Due     </B></TD><TD><INPUT TYPE=TEXT NAME=paymentdue      value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, due);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Payment Received</B></TD><TD><INPUT TYPE=TEXT NAME=paymentreceived value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, received);
	prints("<TR BGCOLOR=%s><TH ALIGN=left COLSPAN=2>Invoice Notes</TH></TR>\n", COLOR_EDITFORM, field(ORDERFIELDS, "notes"));
	prints("<TR BGCOLOR=%s><TH COLSPAN=2><TEXTAREA WRAP=virtual NAME=notes ROWS=3 COLS=40>%s</TEXTAREA></TH></TR>\n", COLOR_EDITFORM, field(ORDERFIELDS, "notes"));
	prints("<TR BGCOLOR=%s><TD ALIGN=center COLSPAN=2>\n", COLOR_EDITFORM);
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</TD></TR>\n");
	prints("</FORM>\n");
	prints("</TABLE>\n");
	prints("</TD></TR>\n");
	if ((sqr=sqlQueryf("SELECT orderdetailid, productname, quantity, discount, unitprice, tax1, tax2 FROM gw_orderdetails WHERE orderid = %d", orderid))<0) return;
	prints("<CENTER>\n");
	prints("<TR><TD ALIGN=center>\n");
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		prints("<TH>&nbsp;</TH><TH>Product</TH><TH>Quantity</TH><TH>Discount</TH><TH NOWRAP>Unit Price</TH><TH>%s</TH><TH>%s</TH></TR>\n", config.info.tax1name, config.info.tax2name);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_EDITFORM);
			prints("<TD NOWRAP><A HREF=%s/orderdetailedit?orderdetailid=%s>edit</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 1));
			prints("<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 2));
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3))*100.0);
			prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 4)));
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 5))*100.0);
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 6))*100.0);
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	prints("<A HREF=%s/orderdetaileditnew?orderid=%d>Add Item</A>\n", conn[sid].dat->in_ScriptName, orderid);
	prints("</TD></TR></TABLE>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.orderedit.orderdate.focus();\n// -->\n</SCRIPT>\n");
	sqlFreeconnect(sqr);
}

/****************************************************************************
 *	orderview()
 *
 *	Purpose	: Display order and order detail information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderview(int sid)
{
	int authlevel;
	int i;
	int orderid;
	float subtotal;
	float totaldue;
	char timebuffer[100];
	time_t t;
	int sqr;
	float due;
	float received;

	if (auth(sid, "orders")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(conn[sid].dat->tuple, 0, sizeof(conn[sid].dat->tuple));
	if (getgetenv(sid, "ORDERID")==NULL) return;
	orderid=atoi(getgetenv(sid, "ORDERID"));
	totaldue=0;
	if ((sqr=sqlQueryf("SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_orderdetails WHERE orderid = %d", orderid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		for (i=0;i<sqlNumtuples(sqr);i++) {
			subtotal=(float)(atof(sqlGetvalue(sqr, i, 0))*atof(sqlGetvalue(sqr, i, 1)));
			subtotal=(float)(subtotal-subtotal*atof(sqlGetvalue(sqr, i, 2)));
			subtotal=(float)(subtotal+subtotal*atof(sqlGetvalue(sqr, i, 3))+subtotal*atof(sqlGetvalue(sqr, i, 4)));
			totaldue+=subtotal;
		}
		totaldue*=100.0f;
		if ((float)totaldue-(int)totaldue>0.5) totaldue++;
		totaldue=(int)totaldue/100.0f;
	}
	sqlFreeconnect(sqr);
	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	sqlUpdatef("UPDATE gw_orders SET mdatetime = '%s', paymentdue = '%1.2f' WHERE orderid = %d", timebuffer, totaldue, orderid);
	if ((sqr=sqlQueryf("SELECT * FROM gw_orders WHERE orderid = %d", orderid))<0) return;
	if (sqlNumtuples(sqr)<1) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		snprintf(conn[sid].dat->tuple[i].name, sizeof(conn[sid].dat->tuple[i].name)-1, "%s", sqlGetfieldname(sqr, i));
		snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, i));
	}
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400><TR><TD><CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Order %d", COLOR_TRIM, orderid);
	if (auth(sid, "orders")>1) {
		prints(" [<A HREF=%s/orderedit?orderid=%d>edit</A>]", conn[sid].dat->in_ScriptName, orderid);
	}
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Order Date      </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s ", COLOR_FNAME, COLOR_FTEXT, sqldate2text(field(ORDERFIELDS, "orderdate")));
	prints("%s&nbsp;</TD></TR>\n", sqltime2text(field(ORDERFIELDS, "orderdate")));
	prints("<TR><TD BGCOLOR=%s><B>Customer        </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FTEXT);
	if ((sqr=sqlQueryf("SELECT contactid, lastname, firstname FROM gw_contacts WHERE contactid = %s", field(ORDERFIELDS, "customerid")))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints("<A HREF=%s/contactview?contactid=%s>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, 0, 0));
		prints("%s", sqlGetvalue(sqr, 0, 1));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(", ");
		prints("%s</A>&nbsp;</TD></TR>\n", sqlGetvalue(sqr, 0, 2));
		sqlFreeconnect(sqr);
	} else {
		prints("&nbsp;</TD></TR>\n");
	}
	due=(int)(atof(field(ORDERFIELDS, "paymentdue"))*100.0F)/100.0F;
	received=(int)(atof(field(ORDERFIELDS, "paymentreceived"))*100.0F)/100.0F;
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Employee        </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(ORDERFIELDS, "employee"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Order Type      </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(ORDERFIELDS, "ordertype"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Payment Method  </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(ORDERFIELDS, "paymentmethod"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Payment Due     </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>$%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, due);
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Payment Received</B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>$%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, received);
	prints("<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><B>Invoice Notes</B></TD></TR>\n", COLOR_FNAME);
	prints("<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><PRE>%s</PRE>&nbsp;</TD></TR>\n", COLOR_FTEXT, field(ORDERFIELDS, "notes"));
	prints("</TABLE>\n");
	authlevel=auth(sid, "orders");
	if ((sqr=sqlQueryf("SELECT orderdetailid, productname, quantity, discount, unitprice, tax1, tax2 FROM gw_orderdetails WHERE orderid = %d", orderid))<0) return;
	prints("<CENTER>\n");
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		if (authlevel>1) {
			prints("<TH>&nbsp;</TH>");
		}
		prints("<TH>Product</TH><TH>Quantity</TH><TH>Discount</TH><TH NOWRAP>Unit Price</TH><TH>%s</TH><TH>%s</TH></TR>\n", config.info.tax1name, config.info.tax2name);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			if (authlevel>1) {
				prints("<TD NOWRAP><A HREF=%s/orderdetailedit?orderdetailid=%s>edit</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			}
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 1));
			prints("<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 2));
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3))*100.0);
			prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 4)));
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 5))*100.0);
			prints("<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 6))*100.0);
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	prints("<A HREF=%s/orderdetaileditnew?orderid=%d>Add Item</A>\n", conn[sid].dat->in_ScriptName, orderid);
	prints("</CENTER></TD></TR></TABLE>\n");
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
}

/****************************************************************************
 *	orderlist()
 *
 *	Purpose	: Display a list of orders matching search criteria
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderlist(int sid)
{
	char query[8192];
	char searchstring[100];
	int offset=0;
	int i;
	int j;
	int sqr1;
	int sqr2;
	float due;
	float received;

	if (auth(sid, "orders")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0) {
		prints("<CENTER>\n");
		prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints("<FORM METHOD=POST ACTION=%s/orderlist NAME=ordersearch>\n", conn[sid].dat->in_ScriptName);
		prints("<TR BGCOLOR=%s><TH COLSPAN=3>Order System Search Form</TH></TR>\n", COLOR_TRIM);
		prints("<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", COLOR_EDITFORM);
		prints("<OPTION SELECTED>All Columns\n");
		if ((sqr1=sqlQuery("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "mdatetime")==0) continue;
			prints("<OPTION>%s\n", sqlGetfieldname(sqr1, i));
		}
		sqlFreeconnect(sqr1);
		prints("</SELECT></TD>\n");
		prints("<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\n");
		prints("<TD><INPUT TYPE=SUBMIT NAME=Submit VALUE='Search'></TD></TR>\n");
		prints("</FORM></TABLE>\n");
		if (auth(sid, "orders")>1) {
			prints("<A HREF=%s/ordereditnew>New Order</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.ordersearch.searchstring.focus();\n// -->\n</SCRIPT>\n");
		return;
	}
	strncpy(searchstring, getpostenv(sid, "SEARCHSTRING"), sizeof(searchstring)-1);
	SwapChar(searchstring, '*', '%');
	SwapChar(searchstring, '?', '_');
	if ((sqr1=sqlQuery("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
	strcpy(query, "SELECT orderid, customerid, orderdate, paymentdue, paymentreceived from gw_orders WHERE (");
	if (strcmp(getpostenv(sid, "COLUMN"), "All Columns")==0) {
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "mdatetime")==0) continue;
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr1, i), searchstring);
			} else {
				strcatf(query, "lower(%s) like lower('%s') or ", sqlGetfieldname(sqr1, i), searchstring);
			}
		}
		strcatf(query, "orderid like '%s'", searchstring);
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			strcatf(query, "%s like '%s'", getpostenv(sid, "COLUMN"), searchstring);
		} else {
			strcatf(query, "lower(%s) like lower('%s')", getpostenv(sid, "COLUMN"), searchstring);
		}
	}
	strcat(query, ") ORDER BY orderdate DESC");
	sqlFreeconnect(sqr1);
	prints("<CENTER>\n");
	if ((sqr1=sqlQuery(query))<0) return;
	if (getgetenv(sid, "OFFSET")!=NULL)
		offset=atoi(getgetenv(sid, "OFFSET"));
	prints("Found %d matching orders<BR>\n", sqlNumtuples(sqr1));
	if (sqlNumtuples(sqr1)<1) {
		prints("<A HREF=%s/ordereditnew>New Order</A>\n", conn[sid].dat->in_ScriptName);
		prints("</CENTER>\n");
		sqlFreeconnect(sqr1);
		return;
	}
	if ((sqr2=sqlQuery("SELECT contactid, lastname, firstname FROM gw_contacts"))<0) return;
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
	prints("<TH NOWRAP>Order ID</TH><TH NOWRAP>Customer Name</TH><TH NOWRAP>Order Date</TH><TH NOWRAP>Amount Due</TH><TH NOWRAP>Amount Received</TH></TR>\n");
	for (i=offset;(i<sqlNumtuples(sqr1))&&(i<offset+50);i++) {
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD ALIGN=right NOWRAP><A HREF=%s/orderview?orderid=%d>&nbsp;&nbsp;&nbsp;%d</A></TD>", conn[sid].dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)), atoi(sqlGetvalue(sqr1, i, 0)));
		for (j=0;j<sqlNumtuples(sqr2);j++) {
			if (atoi(sqlGetvalue(sqr2, j, 0))==atoi(sqlGetvalue(sqr1, i, 1))) {
				prints("<TD NOWRAP><A HREF=%s/contactview?contactid=%s>%s", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr2, j, 0), sqlGetvalue(sqr2, j, 1));
				if (strlen(sqlGetvalue(sqr2, j, 1))&&strlen(sqlGetvalue(sqr2, j, 2))) prints(", ");
				prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr2, j, 2));
				break;
			}
		}
		if (j==sqlNumtuples(sqr2)) {
			prints("<TD NOWRAP>&nbsp;</TD>");
		}
		prints("<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", sqldate2text(sqlGetvalue(sqr1, i, 2)));
		due=(int)(atof(sqlGetvalue(sqr1, i, 3))*100.0F)/100.0F;
		received=(int)(atof(sqlGetvalue(sqr1, i, 4))*100.0F)/100.0F;
		prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", due);
		prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", received);
		prints("</TR>\n");
	}
	prints("</TABLE>\n<TABLE><TR>\n");
	if (offset>49) {
		prints("<TD><FORM METHOD=POST ACTION=%s/orderlist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset-50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Previous 50'>\n");
		prints("</FORM></TD>\n");
	}
	if (offset+50<sqlNumtuples(sqr1)) {
		prints("<TD><FORM METHOD=POST ACTION=%s/orderlist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset+50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Next 50'>\n");
		prints("</FORM></TD>\n");
	}
	prints("</TR></TABLE>\n");
	prints("<A HREF=%s/ordereditnew>New Order</A>\n", conn[sid].dat->in_ScriptName);
	prints("</CENTER>\n");
	sqlFreeconnect(sqr1);
	sqlFreeconnect(sqr2);
	return;
}

/****************************************************************************
 *	orderdetailsave()
 *
 *	Purpose	: Save order detail information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderdetailsave(int sid)
{
	time_t t;
	char query[8192];
	int orderdetailid;
	int i;
	int sqr;

	if (auth(sid, "orders")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	memset(conn[sid].dat->tuple, 0, sizeof(conn[sid].dat->tuple));
	if ((sqr=sqlQuery("SELECT * FROM gw_orderdetails WHERE orderdetailid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL)
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
	}
	orderdetailid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(orderdetailid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
	}
	sqlFreeconnect(sqr);
	if (orderdetailid==0) {
		if ((sqr=sqlQueryf("SELECT discount, unitprice, internalcost, tax1, tax2 FROM gw_products WHERE productname = '%s'", field(ORDERDETAILFIELDS, "productname")))<0) return;
		if (sqlNumtuples(sqr)>0) {
			for (i=1;i<ORDERDETAILFIELDS;i++) {
				if (strcmp(conn[sid].dat->tuple[i].name, "discount")==0) {
					snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, 0));
				} else if (strcmp(conn[sid].dat->tuple[i].name, "unitprice")==0) {
					snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, 1));
				} else if (strcmp(conn[sid].dat->tuple[i].name, "internalcost")==0) {
					snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, 2));
				} else if (strcmp(conn[sid].dat->tuple[i].name, "tax1")==0) {
					snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, 3));
				} else if (strcmp(conn[sid].dat->tuple[i].name, "tax2")==0) {
					snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", sqlGetvalue(sqr, 0, 4));
				}
			}

		}
		sqlFreeconnect(sqr);
		if ((sqr=sqlQuery("SELECT max(orderdetailid) FROM gw_orderdetails"))<0) return;
		orderdetailid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (orderdetailid<1) orderdetailid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", orderdetailid);
		strcpy(query, "INSERT INTO gw_orderdetails (");
		for (i=0;i<ORDERDETAILFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<ORDERDETAILFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<ORDERDETAILFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<ORDERDETAILFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Order detail %d added successfully</CENTER><BR>\n", orderdetailid);
		logaccess(1, "%s - %s added order detail %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderdetailid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "orders")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_orderdetails WHERE orderdetailid = %d", orderdetailid)<0) return;
		prints("<CENTER>Order detail %d deleted successfully</CENTER><BR>\n", orderdetailid);
		logaccess(1, "%s - %s deleted order detail %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderdetailid);
	} else {
		if (auth(sid, "orders")<2) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_orderdetails SET ");
		for (i=2;i<ORDERDETAILFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			strcat(query, " = '");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<ORDERDETAILFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE orderdetailid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Order detail %d modified successfully</CENTER><BR>\n", orderdetailid);
		logaccess(1, "%s - %s modified order detail %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderdetailid);
	}
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("location.replace(\"%s/orderview?orderid=%s\");\n", conn[sid].dat->in_ScriptName, field(ORDERDETAILFIELDS, "orderid"));
	prints("// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orderview?orderid=%s\">\n", conn[sid].dat->in_ScriptName, field(ORDERDETAILFIELDS, "orderid"));
	prints("</NOSCRIPT>\n");
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
void ordersave(int sid)
{
	time_t t;
	char query[8192];
	int orderid;
	int i;
	int sqr;

	if (auth(sid, "orders")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL)
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
	}
	orderid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(orderid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
	}
	sqlFreeconnect(sqr);
	if (orderid==0) {
		if ((sqr=sqlQuery("SELECT max(orderid) FROM gw_orders"))<0) return;
		orderid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (orderid<1) orderid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", orderid);
		strcpy(query, "INSERT INTO gw_orders (");
		for (i=0;i<ORDERFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<ORDERFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<ORDERFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<ORDERFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Order %d added successfully</CENTER><BR>\n", orderid);
		logaccess(1, "%s - %s added order %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "orders")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_orders WHERE orderid = %d", orderid)<0) return;
		prints("<CENTER>Order %d deleted successfully</CENTER><BR>\n", orderid);
		logaccess(1, "%s - %s deleted order %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderid);
	} else {
		if (auth(sid, "orders")<2) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_orders SET ");
		for (i=2;i<ORDERFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			strcat(query, " = '");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<ORDERFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE orderid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Order %d modified successfully</CENTER><BR>\n", orderid);
		logaccess(1, "%s - %s modified order %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, orderid);
	}
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("location.replace(\"%s/orderview?orderid=%d\");\n", conn[sid].dat->in_ScriptName, orderid);
	prints("// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orderview?orderid=%d\">\n", conn[sid].dat->in_ScriptName, orderid);
	prints("</NOSCRIPT>\n");
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
void ordermain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Orders");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch09-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/orderlist>VIEW ORDERS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/productlist>VIEW PRODUCTS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/orderdetailedit", 16)==0)
		orderdetailedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/orderdetailsave", 16)==0)
		orderdetailsave(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/orderedit", 10)==0)
		orderedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/orderview", 10)==0)
		orderview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/orderlist", 10)==0)
		orderlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/ordersave", 10)==0)
		ordersave(sid);
	printfooter(sid);
	return;
}
