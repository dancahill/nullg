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
#include "mod_stub.h"
#include "mod_orders.h"

void htselect_product(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_query(sid, "SELECT productid, productname FROM gw_products order by productname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

char *htview_product(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT productid, productname FROM gw_products WHERE productid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
	}
	sql_freeresult(sqr);
	return buffer;
}

/****************************************************************************
 *	orderitemedit()
 *
 *	Purpose	: Display edit form for order item information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderitemedit(CONN *sid)
{
	REC_ORDERITEM orderitem;
	int orderitemid;

	if (!(auth_priv(sid, "orders")&A_MODIFY)) {
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Order %d - ", config->colour_th, config->colour_thtext, orderitem.orderid);
	if (orderitemid>0) {
		prints(sid, "Item %d</FONT></TH></TR>\n", orderitem.orderitemid);
	} else {
		prints(sid, "New Item</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Product&nbsp;      </B></TD><TD ALIGN=RIGHT><SELECT NAME=productid style='width:182px'>", config->colour_editform);
	htselect_product(sid, orderitem.productid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Quantity&nbsp;     </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=quantity     value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, orderitem.quantity);
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemeditnew", 17)!=0) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Discount %%&nbsp;  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, orderitem.discount);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Unit Price&nbsp;   </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, orderitem.unitprice);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Internal Cost&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, orderitem.internalcost);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, proc->info.tax1name, orderitem.tax1);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value='%1.2f' SIZE=25></TD></TR>\n", config->colour_editform, proc->info.tax2name, orderitem.tax2);
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, "orders")&A_DELETE)&&(orderitem.orderitemid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
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
void orderitemlist(CONN *sid, int orderid)
{
	int i;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT orderitemid, productid, quantity, unitprice, discount FROM gw_orderitems WHERE orderid = %d", orderid))<0) return;
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%>\r\n<TR BGCOLOR=%s>", config->colour_tabletrim, config->colour_fieldname);
	prints(sid, "<TD NOWRAP><B>Product</B></TD><TD NOWRAP><B>Quantity</B></TD><TD NOWRAP><B>Unit Price</B></TD><TD NOWRAP><B>Discount</B></TD><TD NOWRAP><B>Extended</B></TD></TR>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR BGCOLOR=%s style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/itemedit?orderitemid=%d'\" TITLE='Edit Item'>", config->colour_editform, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "<TD NOWRAP>");
		if (auth_priv(sid, "orders")&A_MODIFY) prints(sid, "<A HREF=%s/orders/itemedit?orderitemid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "%s", htview_product(sid, atoi(sql_getvalue(sqr, i, 1))));
		if (auth_priv(sid, "orders")&A_MODIFY) prints(sid, "</A>");
		prints(sid, "&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 2)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 3)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>%1.0f%%&nbsp;</TD>", atof(sql_getvalue(sqr, i, 4))*100.0);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 2))*atof(sql_getvalue(sqr, i, 3)));
		prints(sid, "</TR>\n");
	}
	sql_freeresult(sqr);
	if (auth_priv(sid, "orders")&A_MODIFY) {
		prints(sid, "<TR BGCOLOR=%s style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/itemeditnew?orderid=%d'\" TITLE='Add Item'>", config->colour_editform, sid->dat->in_ScriptName, orderid);
		prints(sid, "<TD NOWRAP WIDTH=100%%><A HREF=%s/orders/itemeditnew?orderid=%d>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</A></TD>", sid->dat->in_ScriptName, orderid);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD>");
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
}

/****************************************************************************
 *	orderitemsave()
 *
 *	Purpose	: Save order item information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void orderitemsave(CONN *sid)
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

	if (!(auth_priv(sid, "orders")&A_MODIFY)) {
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
		if (!(auth_priv(sid, "orders")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_orderitems WHERE orderitemid = %d", orderitem.orderitemid)<0) return;
		prints(sid, "<CENTER>Order item %d deleted successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, 1, "orderitems", orderitem.orderitemid, "delete", "%s - %s deleted order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else if (orderitem.orderitemid==0) {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
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
		if ((sqr=sql_query(sid, "SELECT max(orderitemid) FROM gw_orderitems"))<0) return;
		orderitem.orderitemid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (orderitem.orderitemid<1) orderitem.orderitemid=1;
		strcpy(query, "INSERT INTO gw_orderitems (orderitemid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, orderid, productid, quantity, discount, unitprice, internalcost, tax1, tax2) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', ", orderitem.orderitemid, curdate, curdate, sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", orderitem.orderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", orderitem.productid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f')", orderitem.tax2);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Order item %d added successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, 1, "orderitems", orderitem.orderitemid, "insert", "%s - %s added order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orderitems SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, orderitem.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "orderid = '%d', ", orderitem.orderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "productid = '%d', ", orderitem.productid);
		strncatf(query, sizeof(query)-strlen(query)-1, "quantity = '%1.2f', ", orderitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "discount = '%1.2f', ", orderitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "unitprice = '%1.2f', ", orderitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "internalcost = '%1.2f', ", orderitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax1 = '%1.2f', ", orderitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax2 = '%1.2f'", orderitem.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE orderitemid = %d", orderitem.orderitemid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Order item %d modified successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, 1, "orderitems", orderitem.orderitemid, "modify", "%s - %s modified order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	}
	totaldue=0;
	if ((sqr=sql_queryf(sid, "SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_orderitems WHERE orderid = %d", orderitem.orderid))<0) return;
	if (sql_numtuples(sqr)>0) {
		for (i=0;i<sql_numtuples(sqr);i++) {
			subtotal=(float)(atof(sql_getvalue(sqr, i, 0))*atof(sql_getvalue(sqr, i, 1)));
			subtotal=(float)(subtotal-subtotal*atof(sql_getvalue(sqr, i, 2)));
			subtotal=(float)(subtotal+subtotal*atof(sql_getvalue(sqr, i, 3))+subtotal*atof(sql_getvalue(sqr, i, 4)));
			totaldue+=subtotal;
		}
		totaldue*=100.0f;
		if ((float)totaldue-(int)totaldue>0.5) totaldue++;
		totaldue=(int)totaldue/100.0f;
	}
	sql_freeresult(sqr);
	sql_updatef(sid, "UPDATE gw_orders SET obj_mtime = '%s', paymentdue = '%1.2f' WHERE orderid = %d", curdate, totaldue, orderitem.orderid);
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
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>New Order</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Customer</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid>", config->colour_editform);
		htselect_contact(sid, order.contactid);
		prints(sid, "</SELECT></TD></TR>\n</TABLE>\n");
		prints(sid, "<INPUT TYPE=SUBMIT  CLASS=frmButtonNAME=Submit VALUE='Save'>\n</FORM>\n");
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s><A HREF=%s/orders/view?orderid=%d STYLE='color: %s'>Order %d</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, sid->dat->in_ScriptName, order.orderid, config->colour_thtext, order.orderid);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Order Date      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=orderdate       value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, time_unix2sql(sid, order.orderdate));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Customer        &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>", config->colour_editform);
	htselect_contact(sid, order.contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Order Type      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=ordertype style='width:182px'>", config->colour_editform);
	prints(sid, "<OPTION%s>Sale", strcmp(order.ordertype, "Sale")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>AR Payment", strcmp(order.ordertype, "AR Payment")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Quote", strcmp(order.ordertype, "Quote")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Method  &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=paymentmethod style='width:182px'>", config->colour_editform);
	prints(sid, "<OPTION%s>Cash", strcmp(order.paymentmethod, "Cash")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Cheque", strcmp(order.paymentmethod, "Cheque")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Credit Card", strcmp(order.paymentmethod, "Credit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Debit Card", strcmp(order.paymentmethod, "Debit Card")==0?" SELECTED":"");
	prints(sid, "<OPTION%s>Money Order", strcmp(order.paymentmethod, "Money Order")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Due     &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=paymentdue      value=\"%1.2f\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, order.paymentdue);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Payment Received&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=paymentreceived value=\"%1.2f\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, order.paymentreceived);
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Details&nbsp;</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><TEXTAREA WRAP=HARD NAME=details ROWS=3 COLS=40>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, order.details));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=center>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TR><TD ALIGN=center>\n");
	orderitemlist(sid, orderid);
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
void orderview(CONN *sid)
{
	REC_ORDER order;
	int orderid;

	if (!(auth_priv(sid, "orders")&A_READ)) {
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
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%>\r\n", config->colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Order %d", config->colour_th, config->colour_thtext, orderid);
	if (auth_priv(sid, "orders")&A_MODIFY) {
		prints(sid, " [<A HREF=%s/orders/edit?orderid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, orderid, config->colour_thlink);
	}
	prints(sid, "</TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Order Date</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2text(sid, order.orderdate));
	prints(sid, "<TR><TD BGCOLOR=%s><B>Customer</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", config->colour_fieldname, config->colour_fieldval);
	prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, order.contactid);
	prints(sid, "%s</A>&nbsp;</TD></TR>\n", htview_contact(sid, order.contactid));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Employee        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, htview_user(sid, order.userid)));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Order Type      </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, order.ordertype));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Method of Payment</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, order.paymentmethod));
	if (strlen(order.details)) {
		prints(sid, "<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><B>Details</B></TD></TR>\n", config->colour_fieldname);
		prints(sid, "<TR><TD BGCOLOR=%s NOWRAP COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", config->colour_fieldval, str2html(sid, order.details));
	}
	prints(sid, "<TR STYLE='spacing:0px'><TD NOWRAP COLSPAN=2 STYLE='padding:0px'>");
	orderitemlist(sid, orderid);
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=right BGCOLOR=%s COLSPAN=2 NOWRAP WIDTH=100%%><B>SubTotal</B> $%1.2f&nbsp;</TD></TR>\n", config->colour_fieldval, order.paymentdue);
	prints(sid, "<TR><TD ALIGN=right BGCOLOR=%s COLSPAN=2 NOWRAP WIDTH=100%%><B>Net Due </B> $%1.2f&nbsp;</TD></TR>\n", config->colour_fieldval, order.paymentdue);
	prints(sid, "<TR><TD ALIGN=right BGCOLOR=%s COLSPAN=2 NOWRAP WIDTH=100%%><B>Received</B> $%1.2f&nbsp;</TD></TR>\n", config->colour_fieldval, order.paymentreceived);
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
	char query[8192];
	char searchstring[100];
	int offset=0;
	int i;
	int j;
	int sqr1;
	int sqr2;
	float due;
	float received;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) {
		prints(sid, "<CENTER>\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list NAME=ordersearch>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Order System Search Form</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", config->colour_editform);
		prints(sid, "<OPTION SELECTED>All Columns\n");
		if ((sqr1=sql_query(sid, "SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
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
	strncpy(searchstring, getpostenv(sid, "SEARCHSTRING"), sizeof(searchstring)-1);
	ptemp=searchstring;
	while (*ptemp) {
		if (*ptemp=='*') *ptemp='%';
		if (*ptemp=='?') *ptemp='_';
		ptemp++;
	}
	if ((sqr1=sql_query(sid, "SELECT * FROM gw_orders WHERE orderid = 1"))<0) return;
	strcpy(query, "SELECT orderid, contactid, orderdate, paymentdue, paymentreceived from gw_orders WHERE (");
	if (strcmp(getpostenv(sid, "COLUMN"), "All Columns")==0) {
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
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", getpostenv(sid, "COLUMN"), searchstring);
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", getpostenv(sid, "COLUMN"), searchstring);
		}
	}
	strncatf(query, sizeof(query)-strlen(query)-1, ") ORDER BY orderdate DESC");
	sql_freeresult(sqr1);
	prints(sid, "<CENTER>\n");
	if ((sqr1=sql_query(sid, query))<0) return;
	if (getgetenv(sid, "OFFSET")!=NULL)
		offset=atoi(getgetenv(sid, "OFFSET"));
	prints(sid, "Found %d matching orders<BR>\n", sql_numtuples(sqr1));
	if (sql_numtuples(sqr1)<1) {
		prints(sid, "<A HREF=%s/orders/editnew>New Order</A>\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\n");
		sql_freeresult(sqr1);
		return;
	}
	if ((sqr2=sql_query(sid, "SELECT contactid, surname, givenname FROM gw_contacts"))<0) return;
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400>\r\n<TR BGCOLOR=%s>", config->colour_tabletrim, config->colour_th);
	prints(sid, "<TH NOWRAP><FONT COLOR=%s>Order ID</FONT></TH><TH NOWRAP><FONT COLOR=%s>Customer Name</FONT></TH><TH NOWRAP><FONT COLOR=%s>Order Date</FONT></TH><TH NOWRAP><FONT COLOR=%s>Amount Due</FONT></TH><TH NOWRAP><FONT COLOR=%s>Amount Received</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
	for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		prints(sid, "<TD ALIGN=right NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/view?orderid=%d'\"><A HREF=%s/orders/view?orderid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), atoi(sql_getvalue(sqr1, i, 0)));
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 1))) {
				prints(sid, "<TD NOWRAP><A HREF=%s/contacts/view?contactid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(sqr2, j, 0), str2html(sid, sql_getvalue(sqr2, j, 1)));
				if (strlen(sql_getvalue(sqr2, j, 1))&&strlen(sql_getvalue(sqr2, j, 2))) prints(sid, ", ");
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr2, j, 2)));
				break;
			}
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<TD NOWRAP>&nbsp;</TD>");
		}
		prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(sqr1, i, 2)));
		due=(int)(atof(sql_getvalue(sqr1, i, 3))*100.0F)/100.0F;
		received=(int)(atof(sql_getvalue(sqr1, i, 4))*100.0F)/100.0F;
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
	if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
		prints(sid, "<FORM METHOD=POST ACTION=%s/orders/list?offset=%d><TD>\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		prints(sid, "<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
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
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(order.details, sizeof(order.details)-1, "%s", ptemp);
	order.orderdate-=time_tzoffset(sid, order.orderdate);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "orders")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_orders WHERE orderid = %d", order.orderid)<0) return;
		prints(sid, "<CENTER>Order %d deleted successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, 1, "orders", order.orderid, "delete", "%s - %s deleted order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else if (order.orderid==0) {
		if (!(auth_priv(sid, "orders")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_query(sid, "SELECT max(orderid) FROM gw_orders"))<0) return;
		order.orderid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (order.orderid<1) order.orderid=1;
		strcpy(query, "INSERT INTO gw_orders (orderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, contactid, userid, orderdate, ordertype, paymentmethod, paymentdue, paymentreceived, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', ", order.orderid, curdate, curdate, sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", order.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", order.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, order.orderdate));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, order.ordertype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, order.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", order.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", order.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, order.details));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Order %d added successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, 1, "orders", order.orderid, "insert", "%s - %s added order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	} else {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, order.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", order.contactid);
		strncatf(query, sizeof(query)-strlen(query)-1, "userid = %d, ", order.userid);
		strncatf(query, sizeof(query)-strlen(query)-1, "orderdate = '%s', ", time_unix2sql(sid, order.orderdate));
		strncatf(query, sizeof(query)-strlen(query)-1, "ordertype = '%s', ", str2sql(sid, order.ordertype));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentmethod = '%s', ", str2sql(sid, order.paymentmethod));
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentdue = '%1.2f', ", order.paymentdue);
		strncatf(query, sizeof(query)-strlen(query)-1, "paymentreceived = '%1.2f', ", order.paymentreceived);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, order.details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE orderid = %d", order.orderid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Order %d modified successfully</CENTER><BR>\n", order.orderid);
		db_log_activity(sid, 1, "orders", order.orderid, "modify", "%s - %s modified order %d", sid->dat->in_RemoteAddr, sid->dat->user_username, order.orderid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/orders/view?orderid=%d\");\n", sid->dat->in_ScriptName, order.orderid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orders/view?orderid=%d\">\n", sid->dat->in_ScriptName, order.orderid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}


void productedit(CONN *sid)
{
	REC_PRODUCT product;
	int productid;

	if (!(auth_priv(sid, "orders")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/orders/producteditnew")==0) {
		productid=0;
		if (db_read(sid, 2, DB_PRODUCTS, 0, &product)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	} else {
		if (getgetenv(sid, "PRODUCTID")==NULL) return;
		productid=atoi(getgetenv(sid, "PRODUCTID"));
		if (db_read(sid, 2, DB_PRODUCTS, productid, &product)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", productid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/orders/productsave NAME=productedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=productid VALUE='%d'>\n", product.productid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (productid>0) {
		prints(sid, "Product %d</FONT></TH></TR>\n", product.productid);
	} else {
		prints(sid, "New Product</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Product Name  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=productname  value=\"%s\" SIZE=25></TD></TR>\n", config->colour_editform, str2html(sid, product.productname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Category      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=category     value=\"%s\" SIZE=25></TD></TR>\n", config->colour_editform, str2html(sid, product.category));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Discount      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value=\"%1.2f\" SIZE=25></TD></TR>\n", config->colour_editform, product.discount);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Unit Price    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value=\"%1.2f\" SIZE=25></TD></TR>\n", config->colour_editform, product.unitprice);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Internal Cost </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value=\"%1.2f\" SIZE=25></TD></TR>\n", config->colour_editform, product.internalcost);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value=\"%1.2f\" SIZE=25></TD></TR>\n", config->colour_editform, proc->info.tax1name, product.tax1);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value=\"%1.2f\" SIZE=25></TD></TR>\n", config->colour_editform, proc->info.tax2name, product.tax2);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP COLSPAN=2><B>Details</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=3 COLS=40>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, product.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, "orders")&A_ADMIN)&&(product.productid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.productedit.productname.focus();\n// -->\n</SCRIPT>\n");
}

void productview(CONN *sid)
{
	REC_PRODUCT product;
	int productid;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "PRODUCTID")==NULL) return;
	productid=atoi(getgetenv(sid, "PRODUCTID"));
	if (db_read(sid, 1, DB_PRODUCTS, productid, &product)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", productid);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400>\r\n", config->colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Product %d", config->colour_th, config->colour_thtext, productid);
	if (auth_priv(sid, "orders")&A_ADMIN) {
		prints(sid, " [<A HREF=%s/orders/productedit?productid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, productid, config->colour_thlink);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Product Name </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, product.productname));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Category     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, product.category));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Discount     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.discount);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Unit Price   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.unitprice);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Internal Cost</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.internalcost);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, proc->info.tax1name, config->colour_fieldval, product.tax1);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, proc->info.tax2name, config->colour_fieldval, product.tax2);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><B>Details</B></TD></TR>\n", config->colour_fieldname);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", config->colour_fieldval, str2html(sid, product.details));
	prints(sid, "</TABLE>\n<BR>\n</CENTER>\n");
}

void productlist(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_query(sid, "SELECT productid, productname, category, unitprice, internalcost FROM gw_products ORDER BY productname ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	prints(sid, "Listing %d products.<BR>\n", sql_numtuples(sqr));
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n<TR BGCOLOR=%s>", config->colour_tabletrim, config->colour_th);
		prints(sid, "<TH NOWRAP><FONT COLOR=%s>Product Name</FONT></TH><TH NOWRAP><FONT COLOR=%s>Category</FONT></TH><TH NOWRAP><FONT COLOR=%s>Unit Price</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD NOWRAP><A HREF=%s/orders/productview?productid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 3)));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	if (auth_priv(sid, "orders")&A_ADMIN) {
		prints(sid, "<A HREF=%s/orders/producteditnew>New Product</A>\n", sid->dat->in_ScriptName);
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void productsave(CONN *sid)
{
	REC_PRODUCT product;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int productid;
	int sqr;

	if (!(auth_priv(sid, "orders")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "PRODUCTID"))==NULL) return;
	productid=atoi(ptemp);
	if (db_read(sid, 2, DB_PRODUCTS, productid, &product)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "PRODUCTNAME"))!=NULL) snprintf(product.productname, sizeof(product.productname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "CATEGORY"))!=NULL) snprintf(product.category, sizeof(product.category)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DISCOUNT"))!=NULL) product.discount=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "UNITPRICE"))!=NULL) product.unitprice=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "INTERNALCOST"))!=NULL) product.internalcost=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX1"))!=NULL) product.tax1=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX2"))!=NULL) product.tax2=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(product.details, sizeof(product.details)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "orders")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_products WHERE productid = %d", product.productid)<0) return;
		prints(sid, "<CENTER>Product %d deleted successfully</CENTER><BR>\n", product.productid);
		db_log_activity(sid, 1, "products", product.productid, "delete", "%s - %s deleted product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	} else if (product.productid==0) {
		if (!(auth_priv(sid, "orders")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_query(sid, "SELECT max(productid) FROM gw_products"))<0) return;
		product.productid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (product.productid<1) product.productid=1;
		strcpy(query, "INSERT INTO gw_products (productid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, productname, category, discount, unitprice, internalcost, tax1, tax2, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', ", product.productid, curdate, curdate, sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, product.productname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, product.category));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", product.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", product.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", product.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", product.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", product.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, product.details));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Product %d added successfully</CENTER><BR>\n", product.productid);
		db_log_activity(sid, 1, "products", product.productid, "insert", "%s - %s added product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	} else {
		if (!(auth_priv(sid, "orders")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_products SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, product.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "productname = '%s', ", str2sql(sid, product.productname));
		strncatf(query, sizeof(query)-strlen(query)-1, "category = '%s', ", str2sql(sid, product.category));
		strncatf(query, sizeof(query)-strlen(query)-1, "discount = '%1.2f', ", product.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "unitprice = '%1.2f', ", product.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "internalcost = '%1.2f', ", product.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax1 = '%1.2f', ", product.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax2 = '%1.2f', ", product.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, product.details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE productid = %d", product.productid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Product %d modified successfully</CENTER><BR>\n", product.productid);
		db_log_activity(sid, 1, "products", product.productid, "modify", "%s - %s modified product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/orders/productlist\">\n", sid->dat->in_ScriptName);
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
void mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ORDERS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemedit", 16)==0) {
		orderitemedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/itemsave", 16)==0) {
		orderitemsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/productedit", 17)==0) {
		productedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/productview", 17)==0) {
		productview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/productlist", 17)==0) {
		productlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/orders/productsave", 17)==0) {
		productsave(sid);
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

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_orders", "ORDERS", "/orders/list", "mod_main", "/orders/", mod_main)!=0) return -1;
	return 0;
}
