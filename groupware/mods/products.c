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

void productedit(CONNECTION *sid)
{
	REC_PRODUCT product;
	int productid;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/products/editnew")==0) {
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
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/products/save NAME=productedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=productid VALUE='%d'>\n", product.productid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (productid>0) {
		prints(sid, "Product %d</FONT></TH></TR>\n", product.productid);
	} else {
		prints(sid, "New Product</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Product Name  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=productname  value=\"%s\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, str2html(sid, product.productname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Category      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=category     value=\"%s\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, str2html(sid, product.category));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Discount      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value=\"%1.2f\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, product.discount);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Unit Price    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value=\"%1.2f\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, product.unitprice);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> Internal Cost </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value=\"%1.2f\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, product.internalcost);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value=\"%1.2f\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax1name, product.tax1);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B> %s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value=\"%1.2f\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax2name, product.tax2);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP COLSPAN=2><B>Details</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=3 COLS=40>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, product.details));
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ORDERS)&A_ADMIN)&&(product.productid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.productedit.productname.focus();\n// -->\n</SCRIPT>\n");
}

void productview(CONNECTION *sid)
{
	REC_PRODUCT product;
	int productid;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "PRODUCTID")==NULL) return;
	productid=atoi(getgetenv(sid, "PRODUCTID"));
	if (db_read(sid, 1, DB_PRODUCTS, productid, &product)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", productid);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Product %d", COLOR_TH, COLOR_THTEXT, productid);
	if (auth_priv(sid, AUTH_ORDERS)&A_ADMIN) {
		prints(sid, " [<A HREF=%s/products/edit?productid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, productid, COLOR_THLINK);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Product Name </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, product.productname));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Category     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, product.category));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Discount     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, product.discount);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Unit Price   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, product.unitprice);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Internal Cost</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, product.internalcost);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, config.info.tax1name, COLOR_FVAL, product.tax1);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, config.info.tax2name, COLOR_FVAL, product.tax2);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><B>Details</B></TD></TR>\n", COLOR_FNAME);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FVAL, str2html(sid, product.details));
	prints(sid, "</TABLE>\n<BR>\n</CENTER>\n");
}

void productlist(CONNECTION *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sqlQuery(sid, "SELECT productid, productname, category, unitprice, internalcost FROM gw_products ORDER BY productname ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	prints(sid, "Listing %d products.<BR>\n", sqlNumtuples(sqr));
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TH);
		prints(sid, "<TH NOWRAP><FONT COLOR=%s>Product Name</FONT></TH><TH NOWRAP><FONT COLOR=%s>Category</FONT></TH><TH NOWRAP><FONT COLOR=%s>Unit Price</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD NOWRAP><A HREF=%s/products/view?productid=%s>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr, i, 1)));
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3)));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	if (auth_priv(sid, AUTH_ORDERS)&A_ADMIN) {
		prints(sid, "<A HREF=%s/products/editnew>New Product</A>\n", sid->dat->in_ScriptName);
	}
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void productsave(CONNECTION *sid)
{
	REC_PRODUCT product;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int productid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ORDERS)&A_ADMIN)) {
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
		if (!(auth_priv(sid, AUTH_ORDERS)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_products WHERE productid = %d", product.productid)<0) return;
		prints(sid, "<CENTER>Product %d deleted successfully</CENTER><BR>\n", product.productid);
		logaccess(sid, 1, "%s - %s deleted product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	} else if (product.productid==0) {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sqlQuery(sid, "SELECT max(productid) FROM gw_products"))<0) return;
		product.productid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (product.productid<1) product.productid=1;
		strcpy(query, "INSERT INTO gw_products (productid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, productname, category, discount, unitprice, internalcost, tax1, tax2, details) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '0', '0', '0', ", product.productid, curdate, curdate, sid->dat->user_uid);
		strcatf(query, "'%s', ", str2sql(sid, product.productname));
		strcatf(query, "'%s', ", str2sql(sid, product.category));
		strcatf(query, "'%1.2f', ", product.discount);
		strcatf(query, "'%1.2f', ", product.unitprice);
		strcatf(query, "'%1.2f', ", product.internalcost);
		strcatf(query, "'%1.2f', ", product.tax1);
		strcatf(query, "'%1.2f', ", product.tax2);
		strcatf(query, "'%s')", str2sql(sid, product.details));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Product %d added successfully</CENTER><BR>\n", product.productid);
		logaccess(sid, 1, "%s - %s added product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	} else {
		if (!(auth_priv(sid, AUTH_ORDERS)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_products SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, product.obj_uid);
		strcatf(query, "productname = '%s', ", str2sql(sid, product.productname));
		strcatf(query, "category = '%s', ", str2sql(sid, product.category));
		strcatf(query, "discount = '%1.2f', ", product.discount);
		strcatf(query, "unitprice = '%1.2f', ", product.unitprice);
		strcatf(query, "internalcost = '%1.2f', ", product.internalcost);
		strcatf(query, "tax1 = '%1.2f', ", product.tax1);
		strcatf(query, "tax2 = '%1.2f', ", product.tax2);
		strcatf(query, "details = '%s'", str2sql(sid, product.details));
		strcatf(query, " WHERE productid = %d", product.productid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Product %d modified successfully</CENTER><BR>\n", product.productid);
		logaccess(sid, 1, "%s - %s modified product %d", sid->dat->in_RemoteAddr, sid->dat->user_username, product.productid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/products/list\">\n", sid->dat->in_ScriptName);
	return;
}

void productmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ORDERS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/products/edit", 14)==0) {
		productedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/products/view", 14)==0) {
		productview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/products/list", 14)==0) {
		productlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/products/save", 14)==0) {
		productsave(sid);
	}
	htpage_footer(sid);
	return;
}
