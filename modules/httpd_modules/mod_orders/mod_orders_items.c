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
#include "mod_orders.h"

void htselect_product(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_query("SELECT productid, productname FROM gw_products order by productname ASC"))<0) return;
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

	if ((sqr=sql_queryf("SELECT productid, productname FROM gw_products WHERE productid = %d", selected))<0) return buffer;
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
	char *ptemp;
	int orderitemid;

	if (!(auth_priv(sid, "orders")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemeditnew", 19)==0) {
		orderitemid=0;
		if (db_read(sid, 2, DB_ORDERITEMS, 0, &orderitem)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "ORDERID"))==NULL) return;
		orderitem.orderid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "ORDERITEMID"))==NULL) return;
		orderitemid=atoi(ptemp);
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
	prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/orders/view?orderid=%d>Order %d</A> - ", sid->dat->in_ScriptName, orderitem.orderid, orderitem.orderid);
	if (orderitemid>0) {
		prints(sid, "Item %d</TH></TR>\n", orderitem.orderitemid);
	} else {
		prints(sid, "New Item</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Product&nbsp;      </B></TD><TD ALIGN=RIGHT><SELECT NAME=productid style='width:217px'>");
	htselect_product(sid, orderitem.productid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Quantity&nbsp;     </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=quantity     value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\n", orderitem.quantity);
	if (strncmp(sid->dat->in_RequestURI, "/orders/itemeditnew", 17)!=0) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Discount&nbsp;     </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=discount     value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\n", orderitem.discount*100.0F);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Unit Price&nbsp;   </B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\n", orderitem.unitprice);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Internal Cost&nbsp;</B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\n", orderitem.internalcost);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax1         value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\n", proc->info.tax1name, orderitem.tax1*100.0F);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax2         value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\n", proc->info.tax2name, orderitem.tax2*100.0F);
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

	if ((sqr=sql_queryf("SELECT orderitemid, productid, quantity, unitprice, discount FROM gw_orderitems WHERE orderid = %d", orderid))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">");
	prints(sid, "<TD NOWRAP STYLE='border-style:solid'><B>Product</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Quantity</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Unit Price</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Discount</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Extended</B></TD></TR>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR CLASS=\"EDITFORM\" style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/itemedit?orderitemid=%d'\" TITLE='Edit Item'>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>");
		if (auth_priv(sid, "orders")&A_MODIFY) prints(sid, "<A HREF=%s/orders/itemedit?orderitemid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "%s", htview_product(sid, atoi(sql_getvalue(sqr, i, 1))));
		if (auth_priv(sid, "orders")&A_MODIFY) prints(sid, "</A>");
		prints(sid, "&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 2)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 3)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%1.0f%%&nbsp;</TD>", atof(sql_getvalue(sqr, i, 4))*100.0);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 2))*atof(sql_getvalue(sqr, i, 3)));
		prints(sid, "</TR>\n");
	}
	sql_freeresult(sqr);
	if (auth_priv(sid, "orders")&A_MODIFY) {
		prints(sid, "<TR CLASS=\"EDITFORM\" style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/itemeditnew?orderid=%d'\" TITLE='Add Item'>", sid->dat->in_ScriptName, orderid);
		prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><A HREF=%s/orders/itemeditnew?orderid=%d>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</A></TD>", sid->dat->in_ScriptName, orderid);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ORDERITEMID"))==NULL) return;
	orderitemid=atoi(ptemp);
	if (db_read(sid, 2, DB_ORDERITEMS, orderitemid, &orderitem)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "ORDERID"))!=NULL) orderitem.orderid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PRODUCTID"))!=NULL) orderitem.productid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "QUANTITY"))!=NULL) orderitem.quantity=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "DISCOUNT"))!=NULL) orderitem.discount=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "UNITPRICE"))!=NULL) orderitem.unitprice=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "INTERNALCOST"))!=NULL) orderitem.internalcost=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX1"))!=NULL) orderitem.tax1=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "TAX2"))!=NULL) orderitem.tax2=(float)atof(ptemp)/100.0F;
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "orders")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_orderitems WHERE orderitemid = %d AND obj_did = %d", orderitem.orderitemid, sid->dat->user_did)<0) return;
		prints(sid, "<CENTER>Order item %d deleted successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, "orderitems", orderitem.orderitemid, "delete", "%s - %s deleted order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else if (orderitem.orderitemid==0) {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		if (db_read(sid, 1, DB_PRODUCTS, orderitem.productid, &product)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		orderitem.discount=product.discount;
		orderitem.unitprice=product.unitprice;
		orderitem.internalcost=product.internalcost;
		orderitem.tax1=product.tax1;
		orderitem.tax2=product.tax2;
		if ((sqr=sql_query("SELECT max(orderitemid) FROM gw_orderitems"))<0) return;
		orderitem.orderitemid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (orderitem.orderitemid<1) orderitem.orderitemid=1;
		strcpy(query, "INSERT INTO gw_orderitems (orderitemid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, orderid, productid, quantity, discount, unitprice, internalcost, tax1, tax2) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', ", orderitem.orderitemid, curdate, curdate, sid->dat->user_uid, sid->dat->user_did);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", orderitem.orderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", orderitem.productid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", orderitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", orderitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", orderitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f')", orderitem.tax2);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Order item %d added successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, "orderitems", orderitem.orderitemid, "insert", "%s - %s added order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	} else {
		if (!(auth_priv(sid, "orders")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_orderitems SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, orderitem.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "orderid = '%d', ", orderitem.orderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "productid = '%d', ", orderitem.productid);
		strncatf(query, sizeof(query)-strlen(query)-1, "quantity = '%1.2f', ", orderitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "discount = '%1.3f', ", orderitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "unitprice = '%1.2f', ", orderitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "internalcost = '%1.2f', ", orderitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax1 = '%1.3f', ", orderitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax2 = '%1.3f'", orderitem.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE orderitemid = %d", orderitem.orderitemid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Order item %d modified successfully</CENTER><BR>\n", orderitem.orderitemid);
		db_log_activity(sid, "orderitems", orderitem.orderitemid, "modify", "%s - %s modified order item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, orderitem.orderitemid);
	}
	totaldue=0;
	if ((sqr=sql_queryf("SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_orderitems WHERE orderid = %d", orderitem.orderid))<0) return;
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
	sql_updatef("UPDATE gw_orders SET obj_mtime = '%s', paymentdue = '%1.2f' WHERE orderid = %d", curdate, totaldue, orderitem.orderid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/orders/view?orderid=%d\");\n", sid->dat->in_ScriptName, orderitem.orderid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/orders/view?orderid=%d\">\n", sid->dat->in_ScriptName, orderitem.orderid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
