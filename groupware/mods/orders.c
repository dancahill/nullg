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
#include "main.h"

/****************************************************************************
 *	orderitemedit()
 *
 *	Purpose	: Display edit form for order item information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderitemedit(CONNECTION *sid)
{
	REC_ORDERITEM orderitem;
	int orderitemid;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemeditnew", 19)==0) {
		orderitemid=0;
		if (db_read(sid, 2, DB_ORDERITEMS, 0, &orderitem)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (getgetenv(sid, "ORDERID")==NULL) return;
		orderitem.orderid=atoi(getgetenv(sid, "ORDERID"));
	} else {
		if (getgetenv(sid, "ORDERITEMID")==NULL) return;
		orderitemid=atoi(getgetenv(sid, "ORDERITEMID"));
		if (db_read(sid, 2, DB_ORDERITEMS, orderitemid, &orderitem)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", orderitemid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/orders/itemsave NAME=itemedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=orderitemid VALUE='%d'>\n", orderitem.orderitemid);
	prints(sid, "<INPUT TYPE=hidden NAME=orderid VALUE='%d'>\n", orderitem.orderid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Order %d - ", COLOR_TH, COLOR_THTEXT, orderitem.orderid);
	if (orderitemid>0) {
		prints(sid, "Item %d</FONT></TH></TR>\n", orderitem.orderitemid);
	} else {
		prints(sid, "New Item</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Product&nbsp;      </B></TD><TD ALIGN=RIGHT><SELECT NAME=productid style='width:182px'>", COLOR_EDITFORM);
	htselect_product(sid, orderitem.productid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Quantity&nbsp;     </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=quantity     value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, orderitem.quantity);
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemeditnew", 17)!=0) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Discount %%&nbsp;  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, orderitem.discount);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Unit Price&nbsp;   </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, orderitem.unitprice);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Internal Cost&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, orderitem.internalcost);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax1name, orderitem.tax1);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax2name, orderitem.tax2);
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ORDERS)&A_DELETE)&&(orderitem.orderitemid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.itemedit.productid.focus();\n// -->\n</SCRIPT>\n");
}

/****************************************************************************
 *	orderitemlist()
 *
 *	Purpose	: Display list of order items
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderitemlist(CONNECTION *sid, int orderid)
{
	int i;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT orderitemid, productid, quantity, discount, unitprice, tax1, tax2 FROM gw_orderitems WHERE orderid = %d", orderid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%><TR BGCOLOR=%s>", COLOR_TH);
		if (auth_priv(sid, AUTH_ORDERS)&A_MODIFY) {
			prints(sid, "<TH>&nbsp;</TH>");
		}
		prints(sid, "<TH><FONT COLOR=%s>Product</FONT></TH><TH><FONT COLOR=%s>Quantity</FONT></TH><TH><FONT COLOR=%s>Discount</FONT></TH><TH NOWRAP><FONT COLOR=%s>Unit Price</FONT></TH><TH><FONT COLOR=%s>%s</FONT></TH><TH><FONT COLOR=%s>%s</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, config.info.tax1name, COLOR_THTEXT, config.info.tax2name);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_EDITFORM);
			if (auth_priv(sid, AUTH_ORDERS)&A_MODIFY) {
				prints(sid, "<TD NOWRAP><A HREF=%s/orders/itemedit?orderitemid=%s>edit</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			}
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", htview_product(sid, atoi(sqlGetvalue(sqr, i, 1))));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 2));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3))*100.0);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 4)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 5))*100.0);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 6))*100.0);
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	sqlFreeconnect(sqr);
}

/****************************************************************************
 *	orderitemsave()
 *
 *	Purpose	: Save order item information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderitemsave(CONNECTION *sid)
{
	REC_ORDERITEM orderitem;
	REC_PRODUCT product;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	float subtotal;
	float totaldue;
	int i;
	int orderitemid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ORDERITEMID"))==NULL) return;
	orderitemid=atoi(ptemp);
	if (db_read(sid, 2, DB_ORDERITEMS, orderitemid, &orderitem)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "ORDERID"))!=NULL) orderitem.orderid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PRODUCTID"))!=NULL) orderitem.productid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "QUANTITY"))!=NULL) orderitem.quantity=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "DISCOUNT"))!=NULL) orderitem.discount=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "UNITPRICE"))!=NULL) orderitem.unitprice=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "INTERNALCOST"))!=NULL) orderitem.internalcost=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX1"))!=NULL) orderitem.tax1=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX2"))!=NULL) orderitem.tax2=(float)atof(ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_orderitems WHERE orderitemid = %d", orderitem.orderitemid)<0) return;
		prints(sid, "<CENTER>Order item %d deleted successfully</CENTER><BR>\n", orderitem.orderitemid);
		logaccess(sid, 1, "%s - %s deleted order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else if (orderitem.orderitemid==0) {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (db_read(sid, 2, DB_PRODUCTS, orderitem.productid, &product)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		orderitem.discount=product.discount;
		orderitem.unitprice=product.unitprice;
		orderitem.internalcost=product.internalcost;
		orderitem.tax1=product.tax1;
		orderitem.tax2=product.tax2;
		if ((sqr=sqlQuery(sid, "SELECT max(orderitemid) FROM gw_orderitems"))<0) return;
		orderitem.orderitemid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (orderitem.orderitemid<1) orderitem.orderitemid=1;
		strcpy(query, "INSERT INTO gw_orderitems (orderitemid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, orderid, productid, quantity, discount, unitprice, internalcost, tax1, tax2) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '0', '0', '0', ", orderitem.orderitemid, curdate, curdate, sid->dat->user_uid);
		strcatf(query, "'%d', ", orderitem.orderid);
		strcatf(query, "'%d', ", orderitem.productid);
		strcatf(query, "'%1.2f', ", orderitem.quantity);
		strcatf(query, "'%1.2f', ", orderitem.discount);
		strcatf(query, "'%1.2f', ", orderitem.unitprice);
		strcatf(query, "'%1.2f', ", orderitem.internalcost);
		strcatf(query, "'%1.2f', ", orderitem.tax1);
		strcatf(query, "'%1.2f')", orderitem.tax2);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Order item %d added successfully</CENTER><BR>\n", orderitem.orderitemid);
		logaccess(sid, 1, "%s - %s added order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orderitems SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, orderitem.obj_uid);
		strcatf(query, "orderid = '%d', ", orderitem.orderid);
		strcatf(query, "productid = '%d', ", orderitem.productid);
		strcatf(query, "quantity = '%1.2f', ", orderitem.quantity);
		strcatf(query, "discount = '%1.2f', ", orderitem.discount);
		strcatf(query, "unitprice = '%1.2f', ", orderitem.unitprice);
		strcatf(query, "internalcost = '%1.2f', ", orderitem.internalcost);
		strcatf(query, "tax1 = '%1.2f', ", orderitem.tax1);
		strcatf(query, "tax2 = '%1.2f'", orderitem.tax2);
		strcatf(query, " WHERE orderitemid = %d", orderitem.orderitemid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Order item %d modified successfully</CENTER><BR>\n", orderitem.orderitemid);
		logaccess(sid, 1, "%s - %s modified order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	}
	totaldue=0;
	if ((sqr=sqlQueryf(sid, "SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_orderitems WHERE orderid = %d", orderitem.orderid))<0) return;
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
	sqlUpdatef(sid, "UPDATE gw_orders SET obj_mtime = '%s', paymentdue = '%1.2f' WHERE orderid = %d", curdate, totaldue, orderitem.orderid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/orders/view?orderid=%d\");\n", sid->dat->in_ScriptName, orderitem.orderid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orders/view?orderid=%d\">\n", sid->dat->in_ScriptName, orderitem.orderid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

/****************************************************************************
 *	orderedit()
 *
 *	Purpose	: Display edit form for order information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderedit(CONNECTION *sid)
{
	REC_ORDER order;
	char *ptemp;
	int orderid;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
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
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>New Order</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Customer</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid>", COLOR_EDITFORM);
		htselect_contact(sid, order.contactid);
		prints(sid, "</SELECT></TD></TR>\n</TABLE>\n");
		prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n</FORM>\n");
		prints(sid, "</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.orderedit.contactid.focus();\n// -->\n</SCRIPT>\n");
		return;
	} else {
		if (getgetenv(sid, "ORDERID")==NULL) return;
		orderid=atoi(getgetenv(sid, "ORDERID"));
		if (db_read(sid, 2, DB_ORDERS, orderid, &order)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", orderid);
			return;
		}
	}
	order.orderdate+=time_tzoffset(sid, order.orderdate);
	prints(sid, "<CENTER>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/orders/save NAME=orderedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=orderid VALUE='%d'>\n", order.orderid);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", order.userid);
	prints(sid, "<TR><TD>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n<TR><TD>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s><A HREF=%s/orders/view?orderid=%d STYLE='color: %s'>Order %d</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, sid->dat->in_ScriptName, order.orderid, COLOR_THTEXT, order.orderid);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Order Date      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=orderdate       value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, time_unix2sql(sid, order.orderdate));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Customer        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>", COLOR_EDITFORM);
	htselect_contact(sid, order.contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Order Type      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=ordertype style='width:182px'>", COLOR_EDITFORM);
	prints(sid, "<OPTION%s>Sale", strcmp(order.ordertype, "Sale")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>AR Payment", strcmp(order.ordertype, "AR Payment")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Quote", strcmp(order.ordertype, "Quote")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Method  &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=paymentmethod style='width:182px'>", COLOR_EDITFORM);
	prints(sid, "<OPTION%s>Cash", strcmp(order.paymentmethod, "Cash")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Cheque", strcmp(order.paymentmethod, "Cheque")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Credit Card", strcmp(order.paymentmethod, "Credit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Debit Card", strcmp(order.paymentmethod, "Debit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Money Order", strcmp(order.paymentmethod, "Money Order")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Due     &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=paymentdue      value=\"%1.2f\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, order.paymentdue);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Received&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=paymentreceived value=\"%1.2f\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, order.paymentreceived);
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><TEXTAREA WRAP=HARD NAME=details ROWS=3 COLS=40>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, order.details));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=center>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TR><TD ALIGN=center>\n");
	orderitemlist(sid, orderid);
	prints(sid, "<A HREF=%s/orders/itemeditnew?orderid=%d>Add Item</A>\n", sid->dat->in_ScriptName, order.orderid);
	prints(sid, "</TD></TR></TABLE>\n");
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
void orderview(CONNECTION *sid)
{
	REC_ORDER order;
	int orderid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "ORDERID")==NULL) return;
	orderid=atoi(getgetenv(sid, "ORDERID"));
	if (db_read(sid, 1, DB_ORDERS, orderid, &order)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", orderid);
		return;
	}
	order.orderdate+=time_tzoffset(sid, order.orderdate);
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400><TR><TD><CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Order %d", COLOR_TH, COLOR_THTEXT, orderid);
	if (auth_priv(sid, AUTH_ORDERS)&A_MODIFY) {
		prints(sid, " [<A HREF=%s/orders/edit?orderid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, orderid, COLOR_THLINK);
	}
	prints(sid, "</TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Order Date      </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2text(sid, order.orderdate));
	prints(sid, "<TR><TD BGCOLOR=%s><B>Customer        </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>", COLOR_FNAME, COLOR_FVAL);
	if ((sqr=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", order.contactid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<A HREF=%s/contacts/view?contactid=%s>", sid->dat->in_ScriptName, sqlGetvalue(sqr, 0, 0));
		prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, 0, 1)));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(sid, ", ");
		prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sqlGetvalue(sqr, 0, 2)));
	} else {
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	sqlFreeconnect(sqr);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Employee        </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, htview_user(sid, order.userid)));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Order Type      </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, order.ordertype));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Payment Method  </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, order.paymentmethod));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Payment Due     </B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>$%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, order.paymentdue);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Payment Received</B></TD><TD ALIGN=right BGCOLOR=%s NOWRAP WIDTH=100%%>$%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, order.paymentreceived);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><B>Details</B></TD></TR>\n", COLOR_FNAME);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FVAL, str2html(sid, order.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "<CENTER>\n");
	orderitemlist(sid, orderid);
	prints(sid, "<A HREF=%s/orders/itemeditnew?orderid=%d>Add Item</A>\n", sid->dat->in_ScriptName, orderid);
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
void orderlist(CONNECTION *sid)
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

	if (!(auth_priv(sid, AUTH_ORDERS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) {
		prints(sid, "<CENTER>\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list NAME=ordersearch>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Order System Search Form</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", COLOR_EDITFORM);
		prints(sid, "<OPTION SELECTED>All Columns\n");
		if ((sqr1=sqlQuery(sid, "SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_operm")==0) continue;
			prints(sid, "<OPTION>%s\n", sqlGetfieldname(sqr1, i));
		}
		sqlFreeconnect(sqr1);
		prints(sid, "</SELECT></TD>\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\n");
		prints(sid, "<TD><INPUT TYPE=SUBMIT NAME=Submit VALUE='Search'></TD></TR>\n");
		prints(sid, "</FORM></TABLE>\n");
		if (auth_priv(sid, AUTH_ORDERS)&A_INSERT) {
			prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
		}
		prints(sid, "</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>document.ordersearch.searchstring.focus();</SCRIPT>\n");
		return;
	}
	strncpy(searchstring, getpostenv(sid, "SEARCHSTRING"), sizeof(searchstring)-1);
	swapchar(searchstring, '*', '%');
	swapchar(searchstring, '?', '_');
	if ((sqr1=sqlQuery(sid, "SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
	strcpy(query, "SELECT orderid, contactid, orderdate, paymentdue, paymentreceived from gw_orders WHERE (");
	if (strcmp(getpostenv(sid, "COLUMN"), "All Columns")==0) {
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "obj_operm")==0) continue;
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr1, i), searchstring);
			} else if (strcmp(sqlGetfieldname(sqr1, i), "paymentdue")==0) {
//				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr1, i), searchstring);
			} else if (strcmp(sqlGetfieldname(sqr1, i), "paymentreceived")==0) {
//				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr1, i), searchstring);
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
	prints(sid, "<CENTER>\n");
	if ((sqr1=sqlQuery(sid, query))<0) return;
	if (getgetenv(sid, "OFFSET")!=NULL)
		offset=atoi(getgetenv(sid, "OFFSET"));
	prints(sid, "Found %d matching orders<BR>\n", sqlNumtuples(sqr1));
	if (sqlNumtuples(sqr1)<1) {
		prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\n");
		sqlFreeconnect(sqr1);
		return;
	}
	if ((sqr2=sqlQuery(sid, "SELECT contactid, surname, givenname FROM gw_contacts"))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TH);
	prints(sid, "<TH NOWRAP><FONT COLOR=%s>Order ID</FONT></TH><TH NOWRAP><FONT COLOR=%s>Customer Name</FONT></TH><TH NOWRAP><FONT COLOR=%s>Order Date</FONT></TH><TH NOWRAP><FONT COLOR=%s>Amount Due</FONT></TH><TH NOWRAP><FONT COLOR=%s>Amount Received</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
	for (i=offset;(i<sqlNumtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
		prints(sid, "<TD ALIGN=right NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/view?orderid=%d'\"><A HREF=%s/orders/view?orderid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)), atoi(sqlGetvalue(sqr1, i, 0)));
		for (j=0;j<sqlNumtuples(sqr2);j++) {
			if (atoi(sqlGetvalue(sqr2, j, 0))==atoi(sqlGetvalue(sqr1, i, 1))) {
				prints(sid, "<TD NOWRAP><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sqlGetvalue(sqr2, j, 0), str2html(sid, sqlGetvalue(sqr2, j, 1)));
				if (strlen(sqlGetvalue(sqr2, j, 1))&&strlen(sqlGetvalue(sqr2, j, 2))) prints(sid, ", ");
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr2, j, 2)));
				break;
			}
		}
		if (j==sqlNumtuples(sqr2)) {
			prints(sid, "<TD NOWRAP>&nbsp;</TD>");
		}
		prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_sql2datetext(sid, sqlGetvalue(sqr1, i, 2)));
		due=(int)(atof(sqlGetvalue(sqr1, i, 3))*100.0F)/100.0F;
		received=(int)(atof(sqlGetvalue(sqr1, i, 4))*100.0F)/100.0F;
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", due);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", received);
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n<TABLE><TR>\n");
	if (offset>(sid->dat->user_maxlist-1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list?offset=%d><TD>\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Previous %d'>\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\n");
	}
	if (offset+sid->dat->user_maxlist<sqlNumtuples(sqr1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list?offset=%d><TD>\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints(sid, "<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Next %d'>\n", sid->dat->user_maxlist);
		prints(sid, "</TD></FORM>\n");
	}
	prints(sid, "</TR></TABLE>\n");
	prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr1);
	sqlFreeconnect(sqr2);
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
void ordersave(CONNECTION *sid)
{
	REC_ORDER order;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int orderid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
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
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(order.details, sizeof(order.details)-1, "%s", ptemp);
	order.orderdate-=time_tzoffset(sid, order.orderdate);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_orders WHERE orderid = %d", order.orderid)<0) return;
		prints(sid, "<CENTER>Order %d deleted successfully</CENTER><BR>\n", order.orderid);
		logaccess(sid, 1, "%s - %s deleted order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else if (order.orderid==0) {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sqlQuery(sid, "SELECT max(orderid) FROM gw_orders"))<0) return;
		order.orderid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (order.orderid<1) order.orderid=1;
		strcpy(query, "INSERT INTO gw_orders (orderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, contactid, userid, orderdate, ordertype, paymentmethod, paymentdue, paymentreceived, details) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '0', '0', '0', ", order.orderid, curdate, curdate, sid->dat->user_uid);
		strcatf(query, "'%d', ", order.contactid);
		strcatf(query, "'%d', ", order.userid);
		strcatf(query, "'%s', ", time_unix2sql(sid, order.orderdate));
		strcatf(query, "'%s', ", str2sql(sid, order.ordertype));
		strcatf(query, "'%s', ", str2sql(sid, order.paymentmethod));
		strcatf(query, "'%1.2f', ", order.paymentdue);
		strcatf(query, "'%1.2f', ", order.paymentreceived);
		strcatf(query, "'%s')", str2sql(sid, order.details));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Order %d added successfully</CENTER><BR>\n", order.orderid);
		logaccess(sid, 1, "%s - %s added order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, order.obj_uid);
		strcatf(query, "contactid = '%d', ", order.contactid);
		strcatf(query, "userid = %d, ", order.userid);
		strcatf(query, "orderdate = '%s', ", time_unix2sql(sid, order.orderdate));
		strcatf(query, "ordertype = '%s', ", str2sql(sid, order.ordertype));
		strcatf(query, "paymentmethod = '%s', ", str2sql(sid, order.paymentmethod));
		strcatf(query, "paymentdue = '%1.2f', ", order.paymentdue);
		strcatf(query, "paymentreceived = '%1.2f', ", order.paymentreceived);
		strcatf(query, "details = '%s'", str2sql(sid, order.details));
		strcatf(query, " WHERE orderid = %d", order.orderid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Order %d modified successfully</CENTER><BR>\n", order.orderid);
		logaccess(sid, 1, "%s - %s modified order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
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
void ordermain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ORDERS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemedit", 16)==0) {
		orderitemedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/itemsave", 16)==0) {
		orderitemsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/edit", 12)==0) {
		orderedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/view", 12)==0) {
		orderview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/list", 12)==0) {
		orderlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/save", 12)==0) {
		ordersave(sid);
	}
	htpage_footer(sid);
	return;
}
