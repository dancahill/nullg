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

void inventoryedit(CONN *sid)
{
/*
	REC_FIN_INVENTORY inventory;
	char *ptemp;
	int inventoryid;

	if (!(auth_priv(sid, "finance")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/finance/inventory/editnew")==0) {
		inventoryid=0;
		if (db_read(sid, 2, DB_INVENTORY, 0, &inventory)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
	} else {
		if ((ptemp=getgetenv(sid, "INVENTORYID"))==NULL) return;
		inventoryid=atoi(ptemp);
		if (db_read(sid, 2, DB_INVENTORY, inventoryid, &inventory)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", inventoryid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n<FORM METHOD=POST ACTION=%s/finance/inventory/save NAME=inventoryedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=inventoryid VALUE='%d'>\r\n", inventory.inventoryid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	if (inventoryid>0) {
		prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/finance/inventory/view?inventoryid=%d>Inventory Item %d</A></TH></TR>\r\n", sid->dat->in_ScriptName, inventory.inventoryid, inventory.inventoryid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Inventory Item</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> Inventory Item Name</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=itemname  value=\"%s\" SIZE=30></TD></TR>\r\n", str2html(sid, inventory.itemname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> Category           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=category     value=\"%s\" SIZE=30></TD></TR>\r\n", str2html(sid, inventory.category));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> Discount           </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=discount     value=\"%1.1f\" SIZE=30></TD></TR>\r\n", inventory.discount*100.0F);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> Unit Price         </B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=unitprice    value=\"%1.2f\" SIZE=30></TD></TR>\r\n", inventory.unitprice);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> Internal Cost      </B></TD><TD ALIGN=RIGHT>$<INPUT TYPE=TEXT NAME=internalcost value=\"%1.2f\" SIZE=30></TD></TR>\r\n", inventory.internalcost);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> %s                 </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax1         value=\"%1.1f\" SIZE=30></TD></TR>\r\n", proc->info.tax1name, inventory.tax1*100.0F);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B> %s                 </B></TD><TD ALIGN=RIGHT>%%<INPUT TYPE=TEXT NAME=tax2         value=\"%1.1f\" SIZE=30></TD></TR>\r\n", proc->info.tax2name, inventory.tax2*100.0F);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP COLSPAN=2><B>Details</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=3 COLS=40>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, inventory.details));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "finance")&A_ADMIN)&&(inventory.inventoryid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	prints(sid, "</FORM>\r\n</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.inventoryedit.itemname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
}

void inventoryview(CONN *sid)
{
/*
	REC_FIN_INVENTORY inventory;
	char *ptemp;
	int inventoryid;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "INVENTORYID"))==NULL) return;
	inventoryid=atoi(ptemp);
	if (db_read(sid, 1, DB_INVENTORY, inventoryid, &inventory)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", inventoryid);
		return;
	}
	prints(sid, "<CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Inventory Item %d", inventoryid);
	if (auth_priv(sid, "finance")&A_ADMIN) {
		prints(sid, " [<A HREF=%s/finance/inventory/edit?inventoryid=%d>edit</A>]", sid->dat->in_ScriptName, inventoryid);
	}
	prints(sid, "</FONT></TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Inventory Item Name </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, inventory.itemname));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Category     </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", str2html(sid, inventory.category));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Discount     </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.1f%%&nbsp;</TD></TR>\r\n", inventory.discount*100.0F);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Unit Price   </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>$%1.2f&nbsp;</TD></TR>\r\n", inventory.unitprice);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Internal Cost</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>$%1.2f&nbsp;</TD></TR>\r\n", inventory.internalcost);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>%s           </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.1f%%&nbsp;</TD></TR>\r\n", proc->info.tax1name, inventory.tax1*100.0F);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>%s           </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.1f%%&nbsp;</TD></TR>\r\n", proc->info.tax2name, inventory.tax2*100.0F);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\r\n", str2html(sid, inventory.details));
	prints(sid, "</TABLE>\r\n<BR />\r\n</CENTER>\r\n");
*/
}

void inventorylist(CONN *sid)
{
/*
	int i;
	SQLRES sqr;

	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr, "SELECT inventoryid, itemname, category, unitprice, internalcost FROM gw_finance_inventory WHERE obj_did = %d ORDER BY itemname ASC", sid->dat->did)<0) return;
	prints(sid, "<CENTER>\r\n");
	prints(sid, "Listing %d inventory items.<BR />\r\n", sql_numtuples(&sqr));
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>");
		prints(sid, "<TH NOWRAP STYLE='border-style:solid'>Inventory Item Name</TH><TH NOWRAP STYLE='border-style:solid'>Category</TH><TH NOWRAP STYLE='border-style:solid'>Unit Price</TH></TR>\r\n");
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/finance/inventory/view?inventoryid=%s>", sid->dat->in_ScriptName, sql_getvalue(&sqr, i, 0));
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr, i, 1)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr, i, 3)));
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
	}
	if (auth_priv(sid, "finance")&A_ADMIN) {
		prints(sid, "<A HREF=%s/finance/inventory/editnew>New Inventory Item</A>\r\n", sid->dat->in_ScriptName);
	}
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr);
*/
	return;
}

void inventorysave(CONN *sid)
{
/*
	REC_FIN_INVENTORY inventory;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int inventoryid;
	SQLRES sqr;

	if (!(auth_priv(sid, "finance")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "INVENTORYID"))==NULL) return;
	inventoryid=atoi(ptemp);
	if (db_read(sid, 2, DB_INVENTORY, inventoryid, &inventory)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "ITEMNAME"))!=NULL) snprintf(inventory.itemname, sizeof(inventory.itemname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "CATEGORY"))!=NULL) snprintf(inventory.category, sizeof(inventory.category)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DISCOUNT"))!=NULL) inventory.discount=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "UNITPRICE"))!=NULL) inventory.unitprice=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "INTERNALCOST"))!=NULL) inventory.internalcost=(float)atof(ptemp);
	if ((ptemp=getpostenv(sid, "TAX1"))!=NULL) inventory.tax1=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "TAX2"))!=NULL) inventory.tax2=(float)atof(ptemp)/100.0F;
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(inventory.details, sizeof(inventory.details)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "finance")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_finance_inventory WHERE inventoryid = %d AND obj_did = %d", inventory.inventoryid, sid->dat->did)<0) return;
		prints(sid, "<CENTER>Inventory Item %d deleted successfully</CENTER><BR />\r\n", inventory.inventoryid);
		db_log_activity(sid, "inventory", inventory.inventoryid, "delete", "%s - %s deleted inventory item %d", sid->dat->in_RemoteAddr, sid->dat->username, inventory.inventoryid);
	} else if (inventory.inventoryid==0) {
		if (!(auth_priv(sid, "finance")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_query(&sqr, "SELECT max(inventoryid) FROM gw_finance_inventory")<0) return;
		inventory.inventoryid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (inventory.inventoryid<1) inventory.inventoryid=1;
		strcpy(query, "INSERT INTO gw_finance_inventory (inventoryid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, itemname, category, discount, unitprice, internalcost, tax1, tax2, details) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', ", inventory.inventoryid, curdate, curdate, sid->dat->uid, sid->dat->did);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.itemname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.category));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", inventory.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", inventory.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.2f', ", inventory.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", inventory.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%1.3f', ", inventory.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.details));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Inventory Item %d added successfully</CENTER><BR />\r\n", inventory.inventoryid);
		db_log_activity(sid, "inventory", inventory.inventoryid, "insert", "%s - %s added inventory item %d", sid->dat->in_RemoteAddr, sid->dat->username, inventory.inventoryid);
	} else {
		if (!(auth_priv(sid, "finance")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_finance_inventory SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, inventory.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "itemname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.itemname));
		strncatf(query, sizeof(query)-strlen(query)-1, "category = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.category));
		strncatf(query, sizeof(query)-strlen(query)-1, "discount = '%1.3f', ", inventory.discount);
		strncatf(query, sizeof(query)-strlen(query)-1, "unitprice = '%1.2f', ", inventory.unitprice);
		strncatf(query, sizeof(query)-strlen(query)-1, "internalcost = '%1.2f', ", inventory.internalcost);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax1 = '%1.3f', ", inventory.tax1);
		strncatf(query, sizeof(query)-strlen(query)-1, "tax2 = '%1.3f', ", inventory.tax2);
		strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, inventory.details));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE inventoryid = %d AND obj_did = %d", inventory.inventoryid, sid->dat->did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Inventory Item %d modified successfully</CENTER><BR />\r\n", inventory.inventoryid);
		db_log_activity(sid, "inventory", inventory.inventoryid, "modify", "%s - %s modified inventory item %d", sid->dat->in_RemoteAddr, sid->dat->username, inventory.inventoryid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/finance/inventory/list\">\r\n", sid->dat->in_ScriptName);
*/
	return;
}
