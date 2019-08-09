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

void htselect_inventory(CONN *sid, int selected)
{
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT inventoryid, itemname FROM gw_finance_inventory WHERE obj_did = %d ORDER BY itemname ASC", sid->dat->user_did)<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

char *htview_inventory(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT inventoryid, itemname FROM gw_finance_inventory WHERE inventoryid = %d AND obj_did = %d", selected, sid->dat->user_did)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 1)));
	}
	sql_freeresult(&sqr);
	return buffer;
}

/****************************************************************************
 *	invoiceitemedit()
 *
 *	Purpose	: Display edit form for invoice item information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoiceitemedit(CONN *sid)
{
	REC_FIN_INVOICEITEM invoiceitem;
	char *ptemp;
	int invoiceitemid;

	if (!(auth_priv(sid, "finance")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/finance/invoices/itemeditnew", 27)==0) {
		invoiceitemid=0;
		if (db_read(sid, 2, DB_INVOICEITEMS, 0, &invoiceitem)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "INVOICEID"))==NULL) return;
		invoiceitem.invoiceid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "INVOICEITEMID"))==NULL) return;
		invoiceitemid=atoi(ptemp);
		if (db_read(sid, 2, DB_INVOICEITEMS, invoiceitemid, &invoiceitem)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", invoiceitemid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n<FORM METHOD=POST ACTION=%s/finance/invoices/itemsave NAME=itemedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=invoiceitemid VALUE='%d'>\r\n", invoiceitem.invoiceitemid);
	prints(sid, "<INPUT TYPE=hidden NAME=invoiceid VALUE='%d'>\r\n", invoiceitem.invoiceid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/finance/invoices/view?invoiceid=%d>Invoice %d</A> - ", sid->dat->in_ScriptName, invoiceitem.invoiceid, invoiceitem.invoiceid);
	if (invoiceitemid>0) {
		prints(sid, "Item %d</TH></TR>\r\n", invoiceitem.invoiceitemid);
	} else {
		prints(sid, "New Item</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Inventory&nbsp;      </B></TD><TD ALIGN=RIGHT><SELECT NAME=inventoryid style='width:217px'>");
	htselect_inventory(sid, invoiceitem.inventoryid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Quantity&nbsp;     </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=quantity     value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\r\n", invoiceitem.quantity);
	if (strncmp(sid->dat->in_RequestURI, "/invoices/itemeditnew", 17)!=0) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Discount&nbsp;     </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=discount     value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\r\n", invoiceitem.discount*100.0F);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Unit Price&nbsp;   </B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\r\n", invoiceitem.unitprice);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Internal Cost&nbsp;</B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=30 style='width:217px'></TD></TR>\r\n", invoiceitem.internalcost);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax1         value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\r\n", proc->info.tax1name, invoiceitem.tax1*100.0F);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;%s&nbsp;           </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax2         value='%1.1f' SIZE=30 style='width:217px'></TD></TR>\r\n", proc->info.tax2name, invoiceitem.tax2*100.0F);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "finance")&A_DELETE)&&(invoiceitem.invoiceitemid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.itemedit.inventoryid.focus();\r\n// -->\r\n</SCRIPT>\r\n");
}

/****************************************************************************
 *	invoiceitemlist()
 *
 *	Purpose	: Display list of invoice items
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoiceitemlist(CONN *sid, int invoiceid)
{
	int i;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT invoiceitemid, inventoryid, quantity, unitprice, discount FROM gw_finance_invoices_items WHERE invoiceid = %d", invoiceid)<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">");
	prints(sid, "<TD NOWRAP STYLE='border-style:solid'><B>Inventory Item</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Quantity</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Unit Price</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Discount</B></TD><TD NOWRAP STYLE='border-style:solid'><B>Extended</B></TD></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "<TR CLASS=\"EDITFORM\" style=\"cursor:hand\" onClick=\"window.location.href='%s/finance/invoices/itemedit?invoiceitemid=%d'\" TITLE='Edit Item'>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>");
		if (auth_priv(sid, "finance")&A_MODIFY) prints(sid, "<A HREF=%s/finance/invoices/itemedit?invoiceitemid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "%s", htview_inventory(sid, atoi(sql_getvalue(&sqr, i, 1))));
		if (auth_priv(sid, "finance")&A_MODIFY) prints(sid, "</A>");
		prints(sid, "&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr, i, 2)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr, i, 3)));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%1.0f%%&nbsp;</TD>", atof(sql_getvalue(&sqr, i, 4))*100.0);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr, i, 2))*atof(sql_getvalue(&sqr, i, 3)));
		prints(sid, "</TR>\r\n");
	}
	sql_freeresult(&sqr);
	if (auth_priv(sid, "finance")&A_MODIFY) {
		prints(sid, "<TR CLASS=\"EDITFORM\" style=\"cursor:hand\" onClick=\"window.location.href='%s/finance/invoices/itemeditnew?invoiceid=%d'\" TITLE='Add Item'>", sid->dat->in_ScriptName, invoiceid);
		prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><A HREF=%s/finance/invoices/itemeditnew?invoiceid=%d>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</A></TD>", sid->dat->in_ScriptName, invoiceid);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
}

/****************************************************************************
 *	invoiceitemsave()
 *
 *	Purpose	: Save invoice item information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void invoiceitemsave(CONN *sid)
{
	REC_FIN_INVOICEITEM invoiceitem;
	REC_FIN_INVENTORY inventory;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	float subtotal;
	float totaldue;
	int i;
	int invoiceitemid;
	SQLRES sqr;

	if (!(auth_priv(sid, "finance")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "INVOICEITEMID"))==NULL) return;
	invoiceitemid=atoi(ptemp);
	if (db_read(sid, 2, DB_INVOICEITEMS, invoiceitemid, &invoiceitem)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "INVOICEID"))!=NULL) invoiceitem.invoiceid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "INVENTORYID"))!=NULL) invoiceitem.inventoryid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "QUANTITY"))!=NULL) invoiceitem.quantity=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "DISCOUNT"))!=NULL) invoiceitem.discount=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "UNITPRICE"))!=NULL) invoiceitem.unitprice=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "INTERNALCOST"))!=NULL) invoiceitem.internalcost=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX1"))!=NULL) invoiceitem.tax1=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "TAX2"))!=NULL) invoiceitem.tax2=(float)atof(ptemp)/100.0F;
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "finance")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_finance_invoices_items WHERE invoiceitemid = %d AND obj_did = %d", invoiceitem.invoiceitemid, sid->dat->user_did)<0) return;
		prints(sid, "<CENTER>Invoice item %d deleted successfully</CENTER><BR>\r\n", invoiceitem.invoiceitemid);
		db_log_activity(sid, "invoiceitems", invoiceitem.invoiceitemid, "delete", "%s - %s deleted invoice item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoiceitem.invoiceitemid);
	} else if (invoiceitem.invoiceitemid==0) {
		if (!(auth_priv(sid, "finance")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (db_read(sid, 1, DB_INVENTORY, invoiceitem.inventoryid, &inventory)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		invoiceitem.discount=inventory.discount;
		invoiceitem.unitprice=inventory.unitprice;
		invoiceitem.internalcost=inventory.internalcost;
		invoiceitem.tax1=inventory.tax1;
		invoiceitem.tax2=inventory.tax2;
		if (sql_query(&sqr, "SELECT max(invoiceitemid) FROM gw_finance_invoices_items")<0) return;
		invoiceitem.invoiceitemid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (invoiceitem.invoiceitemid<1) invoiceitem.invoiceitemid=1;
		strcpy(query, "INSERT INTO gw_finance_invoices_items (invoiceitemid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, invoiceid, inventoryid, quantity, discount, unitprice, internalcost, tax1, tax2) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', ", invoiceitem.invoiceitemid, curdate, curdate, sid->dat->user_uid, sid->dat->user_did);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", invoiceitem.invoiceid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", invoiceitem.inventoryid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", invoiceitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", invoiceitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", invoiceitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", invoiceitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", invoiceitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f')", invoiceitem.tax2);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Invoice item %d added successfully</CENTER><BR>\r\n", invoiceitem.invoiceitemid);
		db_log_activity(sid, "invoiceitems", invoiceitem.invoiceitemid, "insert", "%s - %s added invoice item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoiceitem.invoiceitemid);
	} else {
		if (!(auth_priv(sid, "finance")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_finance_invoices_items SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, invoiceitem.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "invoiceid = '%d', ", invoiceitem.invoiceid);
		strncatf(query, sizeof(query)-strlen(query)-1, "inventoryid = '%d', ", invoiceitem.inventoryid);
		strncatf(query, sizeof(query)-strlen(query)-1, "quantity = '%1.2f', ", invoiceitem.quantity);
		strncatf(query, sizeof(query)-strlen(query)-1, "discount = '%1.3f', ", invoiceitem.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "unitprice = '%1.2f', ", invoiceitem.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "internalcost = '%1.2f', ", invoiceitem.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax1 = '%1.3f', ", invoiceitem.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax2 = '%1.3f'", invoiceitem.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE invoiceitemid = %d AND obj_did = %d", invoiceitem.invoiceitemid, sid->dat->user_did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Invoice item %d modified successfully</CENTER><BR>\r\n", invoiceitem.invoiceitemid);
		db_log_activity(sid, "invoiceitems", invoiceitem.invoiceitemid, "modify", "%s - %s modified invoice item %d", sid->dat->in_RemoteAddr, sid->dat->user_username, invoiceitem.invoiceitemid);
	}
	totaldue=0;
	if (sql_queryf(&sqr, "SELECT quantity, unitprice, discount, tax1, tax2 FROM gw_finance_invoices_items WHERE invoiceid = %d", invoiceitem.invoiceid)<0) return;
	if (sql_numtuples(&sqr)>0) {
		for (i=0;i<sql_numtuples(&sqr);i++) {
			subtotal=(float)(atof(sql_getvalue(&sqr, i, 0))*atof(sql_getvalue(&sqr, i, 1)));
			subtotal=(float)(subtotal-subtotal*atof(sql_getvalue(&sqr, i, 2)));
			subtotal=(float)(subtotal+subtotal*atof(sql_getvalue(&sqr, i, 3))+subtotal*atof(sql_getvalue(&sqr, i, 4)));
			totaldue+=subtotal;
		}
		totaldue*=100.0f;
		if ((float)totaldue-(int)totaldue>0.5) totaldue++;
		totaldue=(int)totaldue/100.0f;
	}
	sql_freeresult(&sqr);
	sql_updatef("UPDATE gw_finance_invoices SET obj_mtime = '%s', paymentdue = '%1.2f' WHERE invoiceid = %d", curdate, totaldue, invoiceitem.invoiceid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "location.replace(\"%s/finance/invoices/view?invoiceid=%d\");\r\n", sid->dat->in_ScriptName, invoiceitem.invoiceid);
	prints(sid, "// -->\r\n</SCRIPT>\r\n<NOSCRIPT>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/finance/invoices/view?invoiceid=%d\">\r\n", sid->dat->in_ScriptName, invoiceitem.invoiceid);
	prints(sid, "</NOSCRIPT>\r\n");
	return;
}
