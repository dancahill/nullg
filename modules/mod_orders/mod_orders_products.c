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
#include "mod_substub.h"
#include "mod_orders.h"

void productedit(CONN *sid)
{
	REC_PRODUCT product;
	char *ptemp;
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
		if ((ptemp=getgetenv(sid, "PRODUCTID"))==NULL) return;
		productid=atoi(ptemp);
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
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Product Name  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=productname  value=\"%s\" SIZE=30></TD></TR>\n", config->colour_editform, str2html(sid, product.productname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Category      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=category     value=\"%s\" SIZE=30></TD></TR>\n", config->colour_editform, str2html(sid, product.category));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Discount      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value=\"%1.2f\" SIZE=30></TD></TR>\n", config->colour_editform, product.discount);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Unit Price    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value=\"%1.2f\" SIZE=30></TD></TR>\n", config->colour_editform, product.unitprice);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Internal Cost </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value=\"%1.2f\" SIZE=30></TD></TR>\n", config->colour_editform, product.internalcost);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value=\"%1.2f\" SIZE=30></TD></TR>\n", config->colour_editform, proc->info.tax1name, product.tax1);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value=\"%1.2f\" SIZE=30></TD></TR>\n", config->colour_editform, proc->info.tax2name, product.tax2);
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
	char *ptemp;
	int productid;

	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "PRODUCTID"))==NULL) return;
	productid=atoi(ptemp);
	if (db_read(sid, 1, DB_PRODUCTS, productid, &product)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", productid);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 STYLE='border-style:solid'><FONT COLOR=%s>Product %d", config->colour_th, config->colour_thtext, productid);
	if (auth_priv(sid, "orders")&A_ADMIN) {
		prints(sid, " [<A HREF=%s/orders/productedit?productid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, productid, config->colour_thlink);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Product Name </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, product.productname));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Category     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, str2html(sid, product.category));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Discount     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.discount);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Unit Price   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.unitprice);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Internal Cost</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, product.internalcost);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, proc->info.tax1name, config->colour_fieldval, product.tax1);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%% STYLE='border-style:solid'>%1.2f&nbsp;</TD></TR>\n", config->colour_fieldname, proc->info.tax2name, config->colour_fieldval, product.tax2);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\n", config->colour_fieldname);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", config->colour_fieldval, str2html(sid, product.details));
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
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s>", config->colour_th);
		prints(sid, "<TH NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Product Name</FONT></TH><TH NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Category</FONT></TH><TH NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Unit Price</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/orders/productview?productid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 3)));
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
