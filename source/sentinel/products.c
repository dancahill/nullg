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

void productedit(int sid)
{
	int i;
	int productid;
	int sqr;

	if (auth(sid, "orders")<3) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/producteditnew")==0) {
		productid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_products WHERE productid = 1"))<0) return;
		strcpy(conn[sid].dat->tuple[0].value, "0");
		strncpy(conn[sid].dat->tuple[0].name, sqlGetfieldname(sqr, 0), sizeof(conn[sid].dat->tuple[0].name)-1);
		for (i=1;i<sqlNumfields(sqr);i++) {
			conn[sid].dat->tuple[i].value[0]='\0';
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", 0);
		for (i=1;i<sqlNumfields(sqr);i++) {
			if (strcmp(conn[sid].dat->tuple[i].name, "discount")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0.00");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "unitprice")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0.00");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "internalcost")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0.00");
			} else if (strcmp(conn[sid].dat->tuple[i].name, "tax1")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%0.2f", config.info.tax1percent);
			} else if (strcmp(conn[sid].dat->tuple[i].name, "tax2")==0) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%0.2f", config.info.tax2percent);
			}
		}
		sqlFreeconnect(sqr);
	} else {
		if (getgetenv(sid, "PRODUCTID")==NULL) return;
		productid = atoi(getgetenv(sid, "PRODUCTID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_products WHERE productid = %d", productid))<0) return;
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
	prints("<FORM METHOD=POST ACTION=%s/productsave NAME=productedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=productid VALUE='%s'>\n", field(PRODUCTFIELDS, "productid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(PRODUCTFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(PRODUCTFIELDS, "mdatetime"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>", COLOR_TRIM);
	if (productid>0) {
		prints("Product %d</TH></TR>\n", productid);
	} else {
		prints("New Product</TH></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Product Name  </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=productname  value='%s' SIZE=25></TD></TR>\n", COLOR_EDITFORM, field(PRODUCTFIELDS, "productname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Category      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=category     value='%s' SIZE=25></TD></TR>\n", COLOR_EDITFORM, field(PRODUCTFIELDS, "category"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Discount      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=discount     value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, atof(field(PRODUCTFIELDS, "discount")));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Unit Price    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=unitprice    value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, atof(field(PRODUCTFIELDS, "unitprice")));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Internal Cost </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=internalcost value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, atof(field(PRODUCTFIELDS, "internalcost")));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>%s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax1         value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax1name, atof(field(PRODUCTFIELDS, "tax1")));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>%s            </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=tax2         value='%1.2f' SIZE=25></TD></TR>\n", COLOR_EDITFORM, config.info.tax2name, atof(field(PRODUCTFIELDS, "tax2")));
	prints("<TR BGCOLOR=%s><TH ALIGN=left COLSPAN=2>Description</TH></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TH COLSPAN=2><TEXTAREA WRAP=virtual NAME=description ROWS=3 COLS=40>%s</TEXTAREA></TH></TR>\n", COLOR_EDITFORM, field(PRODUCTFIELDS, "description"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth(sid, "orders")>2)&&(productid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.productedit.productname.focus();\n// -->\n</SCRIPT>\n");
}

void productview(int sid)
{
	int i;
	int productid;
	int sqr;

	if (auth(sid, "orders")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "PRODUCTID")==NULL) return;
	productid = atoi(getgetenv(sid, "PRODUCTID"));
	if ((sqr=sqlQueryf("SELECT * FROM gw_products WHERE productid = %d", productid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %d</CENTER>\n", productid);
		sqlFreeconnect(sqr);
		return;
	}
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
	}
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Product %d", COLOR_TRIM, productid);
	prints(" [<A HREF=%s/productedit?productid=%d>edit</A>]", conn[sid].dat->in_ScriptName, productid);
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Product Name </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(PRODUCTFIELDS, "productname"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Category     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(PRODUCTFIELDS, "category"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Discount     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, atof(field(PRODUCTFIELDS, "discount")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Unit Price   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, atof(field(PRODUCTFIELDS, "unitprice")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Internal Cost</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, atof(field(PRODUCTFIELDS, "internalcost")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, config.info.tax1name, COLOR_FTEXT, atof(field(PRODUCTFIELDS, "tax1")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>%s           </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%1.2f&nbsp;</TD></TR>\n", COLOR_FNAME, config.info.tax2name, COLOR_FTEXT, atof(field(PRODUCTFIELDS, "tax2")));
	prints("<TR><TD BGCOLOR=%s COLSPAN=2><B>Description</B></TD></TR>\n", COLOR_FNAME);
	prints("<TR><TD BGCOLOR=%s COLSPAN=2><PRE>%s</PRE>&nbsp;</TD></TR>\n", COLOR_FTEXT, field(PRODUCTFIELDS, "description"));
	prints("</TABLE>\n<BR>\n</CENTER>\n");
}

void productlist(int sid)
{
	int i;
	int sqr;

	if (auth(sid, "orders")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQuery("SELECT productid, productname, category, unitprice, internalcost FROM gw_products ORDER BY productname ASC"))<0) return;
	prints("<CENTER>\n");
	prints("Listing %d products.<BR>\n", sqlNumtuples(sqr));
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		prints("<TH NOWRAP>Product Name</TH><TH NOWRAP>Category</TH><TH NOWRAP>Unit Price</TH></TR>\n");
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			prints("<TD NOWRAP><A HREF=%s/productview?productid=%s style='TEXT-DECORATION: NONE'>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr, i, 1));
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 2));
			prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3)));
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	if (auth(sid, "orders")>2) {
		prints("<A HREF=%s/producteditnew>New Product</A>\n", conn[sid].dat->in_ScriptName);
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void productsave(int sid)
{
	time_t t;
	char query[8192];
	int productid;
	int i;
	int sqr;

	if (auth(sid, "orders")<3) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_products WHERE productid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL)
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
	}
	productid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(productid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
	}
	sqlFreeconnect(sqr);
	if (productid==0) {
		if ((sqr=sqlQuery("SELECT max(productid) FROM gw_products"))<0) return;
		productid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (productid<1) productid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", productid);
		strcpy(query, "INSERT INTO gw_products (");
		for (i=0;i<PRODUCTFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<PRODUCTFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<PRODUCTFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<PRODUCTFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Product %d added successfully</CENTER><BR>\n", productid);
		logaccess(1, "%s - %s added product %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, productid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "orders")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_products WHERE productid = %d", productid)<0) return;
		prints("<CENTER>Product %d deleted successfully</CENTER><BR>\n", productid);
		logaccess(1, "%s - %s deleted product %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, productid);
	} else {
		if (auth(sid, "orders")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_products SET ");
		for (i=2;i<PRODUCTFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			strcat(query, " = '");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<PRODUCTFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE productid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Product %d modified successfully</CENTER><BR>\n", productid);
		logaccess(1, "%s - %s modified product %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, productid);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/productlist\">\n", conn[sid].dat->in_ScriptName);
	return;
}

void productmain(int sid)
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
	if (strncmp(conn[sid].dat->in_RequestURI, "/productedit", 12)==0)
		productedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/productview", 12)==0)
		productview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/productlist", 12)==0)
		productlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/productsave", 12)==0)
		productsave(sid);
	printfooter(sid);
	return;
}
