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

void companyedit(int sid)
{
	int companyid;
	int i;
	int sqr;

	if (auth(sid, "contacts")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/companyeditnew")==0) {
		companyid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_companies WHERE companyid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
			conn[sid].dat->tuple[i].value[0]='\0';
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
	} else {
		if (getgetenv(sid, "COMPANYID")==NULL) return;
		companyid=atoi(getgetenv(sid, "COMPANYID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_companies WHERE companyid = %d", companyid))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", companyid);
			sqlFreeconnect(sqr);
			return;
		}
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
	prints("<FORM METHOD=POST ACTION=%s/companysave NAME=companyedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=companyid VALUE='%s'>\n", field(COMPANYFIELDS, "companyid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(COMPANYFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(COMPANYFIELDS, "mdatetime"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4>", COLOR_TRIM);
	if (companyid>0) {
		prints("Company %d</TH></TR>\n", companyid);
	} else {
		prints("New Company</TH></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Company Name   </B></TD><TD><INPUT TYPE=TEXT NAME=companyname value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "companyname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Company Type   </B></TD><TD><INPUT TYPE=TEXT NAME=companytype value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "companytype"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Phone Number   </B></TD><TD><INPUT TYPE=TEXT NAME=phonenumber value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "phonenumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Fax Number     </B></TD><TD><INPUT TYPE=TEXT NAME=faxnumber   value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "faxnumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Primary Contact</B></TD><TD><SELECT NAME=contactid WIDTH=20>", COLOR_EDITFORM);
	contactselect(sid, atoi(field(COMPANYFIELDS, "contactid")));
	prints("</SELECT></TD></TR>\n");
	prints("</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Address        </B></TD><TD><INPUT TYPE=TEXT NAME=address     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "address"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>City, Province </B></TD><TD><INPUT TYPE=TEXT NAME=city        value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "city"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Country        </B></TD><TD><INPUT TYPE=TEXT NAME=country     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "country"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Postal Code    </B></TD><TD><INPUT TYPE=TEXT NAME=postal      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "postal"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Web Site       </B></TD><TD><INPUT TYPE=TEXT NAME=website     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(COMPANYFIELDS, "website"));
	prints("</TABLE>\n</TD></TR>\n");
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth(sid, "contacts")>2)&&(companyid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.companyedit.companyname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void companyview(int sid)
{
	int companyid;
	int i;
	int sqr;

	if (auth(sid, "contacts")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "COMPANYID")==NULL) return;
	companyid=atoi(getgetenv(sid, "COMPANYID"));
	if ((sqr=sqlQueryf("SELECT * FROM gw_companies WHERE companyid = %d", companyid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %d</CENTER>\n", companyid);
		sqlFreeconnect(sqr);
		return;
	}
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
	}
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP>Company %d", COLOR_TRIM, companyid);
	if (auth(sid, "contacts")>2) {
		prints(" [<A HREF=%s/companyedit?companyid=%d>edit</A>]", conn[sid].dat->in_ScriptName, companyid);
	}
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Company Name   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "companyname"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Address        </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "address"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Company Type   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "companytype"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>City, Province </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "city"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Phone Number   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "phonenumber"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Country        </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "country"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Fax Number     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "faxnumber"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Postal Code    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "postal"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Primary Contact</B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
	if ((sqr=sqlQueryf("SELECT contactid, lastname, firstname FROM gw_contacts WHERE contactid = %d", atoi(field(COMPANYFIELDS, "contactid"))))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints("<A HREF=%s/contactview?contactid=%d>", conn[sid].dat->in_ScriptName, atoi(sqlGetvalue(sqr, 0, 0)));
		prints("%s", sqlGetvalue(sqr, 0, 1));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(", ");
		prints("%s&nbsp;</A></TD>\n", sqlGetvalue(sqr, 0, 2));
	} else {
		prints("&nbsp;</TD>\n");
	}
	sqlFreeconnect(sqr);
	prints("    <TD BGCOLOR=%s NOWRAP><B>Web Site      </B></TD><TD BGCOLOR=%s NOWRAP><A HREF=%s TARGET=_new>%s</A>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(COMPANYFIELDS, "website"), field(COMPANYFIELDS, "website"));
	prints("</TABLE>\n");
	prints("[<A HREF=%s/companycontactlist?companyid=%d>View Contacts</A>]\n", conn[sid].dat->in_ScriptName, companyid);
	prints("<BR>\n</CENTER>\n");
}

void companylist(int sid)
{
	char query[2048];
	char searchstring[100];
	int offset=0;
	int i;
	int sqr;
	
	if (auth(sid, "contacts")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0) {
		prints("<CENTER>\n");
		prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints("<FORM METHOD=POST ACTION=%s/contactlist NAME=contactsearch>\n", conn[sid].dat->in_ScriptName);
		prints("<TR BGCOLOR=%s><TH COLSPAN=3>Contact Search Form</TH></TR>\n", COLOR_TRIM);
		prints("<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", COLOR_EDITFORM);
		prints("<OPTION SELECTED>All Columns\n");
		if ((sqr=sqlQuery("SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			if (strcmp(sqlGetfieldname(sqr, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr, i), "mdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr, i), "lastmeetingdate")==0) continue;
			prints("<OPTION>%s\n", sqlGetfieldname(sqr, i));
		}
		sqlFreeconnect(sqr);
		prints("</SELECT></TD>\n");
		prints("<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\n");
		prints("<TD><INPUT TYPE=SUBMIT NAME=Submit VALUE='Search'></TD></TR>\n");
		prints("</FORM></TABLE>\n");
		if (auth(sid, "contacts")>1) {
			prints("<A HREF=%s/contacteditnew>New Contact</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
		prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints("<FORM METHOD=POST ACTION=%s/companylist NAME=companysearch>\n", conn[sid].dat->in_ScriptName);
		prints("<TR BGCOLOR=%s><TH COLSPAN=3>Company Search Form</TH></TR>\n", COLOR_TRIM);
		prints("<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", COLOR_EDITFORM);
		prints("<OPTION SELECTED>All Columns\n");
		if ((sqr=sqlQuery("SELECT * FROM gw_companies WHERE companyid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			if (strcmp(sqlGetfieldname(sqr, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr, i), "mdatetime")==0) continue;
			prints("<OPTION>%s\n", sqlGetfieldname(sqr, i));
		}
		sqlFreeconnect(sqr);
		prints("</SELECT></TD>\n");
		prints("<TD><INPUT TYPE=TEXT NAME=searchstring value='*' SIZE=20></TD>\n");
		prints("<TD><INPUT TYPE=SUBMIT NAME=Submit VALUE='Search'></TD></TR>\n");
		prints("</FORM></TABLE>\n");
		if (auth(sid, "contacts")>1) {
			prints("<A HREF=%s/companyeditnew>New Company</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.companysearch.searchstring.focus();\n// -->\n</SCRIPT>\n");
		return;
	}
	strncpy(searchstring, getpostenv(sid, "SEARCHSTRING"), sizeof(searchstring)-1);
	SwapChar(searchstring, '*', '%');
	SwapChar(searchstring, '?', '_');
	if ((sqr=sqlQuery("SELECT * FROM gw_companies WHERE companyid = 1"))<0) return;
	strcpy(query, "SELECT companyid, companyname, phonenumber, website FROM gw_companies WHERE ");
	if (strcmp(getpostenv(sid, "COLUMN"), "All Columns")==0) {
		for (i=0;i<sqlNumfields(sqr);i++) {
			if (strcmp(sqlGetfieldname(sqr, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr, i), "mdatetime")==0) continue;
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr, i), searchstring);
			} else {
				strcatf(query, "lower(%s) like lower('%s') or ", sqlGetfieldname(sqr, i), searchstring);
			}
		}
		strcatf(query, "companyid like '%s'", searchstring);
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			strcatf(query, "%s like '%s'", getpostenv(sid, "COLUMN"), searchstring);
		} else {
			strcatf(query, "lower(%s) like lower('%s')", getpostenv(sid, "COLUMN"), searchstring);
		}
	}
	strcat(query, " ORDER BY companyname ASC");
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	if ((sqr=sqlQuery(query))<0) return;
	if (getgetenv(sid, "OFFSET")!=NULL)
		offset=atoi(getgetenv(sid, "OFFSET"));
	prints("Found %d matching companies<BR>\n", sqlNumtuples(sqr));
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		if (auth(sid, "contacts")>1) {
			prints("<A HREF=%s/companyeditnew>New Company</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER><BR>\n");
		return;
	}
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH>Company Name</TH><TH>Phone Number</TH><TH>Web Site</TH></TR>\n", COLOR_TRIM);
	for (i=offset;(i<sqlNumtuples(sqr))&&(i<offset+50);i++) {
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD NOWRAP><A HREF=%s/companyview?companyid=%s>%s&nbsp;</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 1));
		prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 2));
		prints("<TD NOWRAP><A HREF=%s target=new>%s</A>&nbsp;</TD>", sqlGetvalue(sqr, i, 3), sqlGetvalue(sqr, i, 3));
		prints("</TR>\n");
	}
	prints("</TABLE>\n<TABLE><TR>\n");
	if (offset>49) {
		prints("<TD><FORM METHOD=POST ACTION=%s/companylist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset-50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Previous 50'>\n");
		prints("</FORM></TD>\n");
	}
	if (offset+50<sqlNumtuples(sqr)) {
		prints("<TD><FORM METHOD=POST ACTION=%s/companylist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset+50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Next 50'>\n");
		prints("</FORM></TD>\n");
	}
	prints("</TR></TABLE>\n");
	if (auth(sid, "contacts")>1) {
		prints("<A HREF=%s/companyeditnew>New Company</A>\n", conn[sid].dat->in_ScriptName);
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void companycontactlist(int sid)
{
	int companyid=0;
	int i;
	int sqr;
	int sqr2;

	if (auth(sid, "contacts")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "COMPANYID")==NULL) return;
	companyid=atoi(getgetenv(sid, "COMPANYID"));
	prints("<CENTER>\n");
	if ((sqr=sqlQueryf("SELECT contactid, lastname, firstname, worknumber, email from gw_contacts WHERE companyid = %d ORDER BY lastname DESC", companyid))<0) return;
	if ((sqr2=sqlQueryf("SELECT companyid, companyname FROM gw_companies WHERE companyid = %d", companyid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints("Found %d contacts for <A HREF=%s/companyview?companyid=%s>%s</A><BR>\n", sqlNumtuples(sqr), conn[sid].dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), sqlGetvalue(sqr2, 0, 1));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		prints("<TH NOWRAP>Contact Name</TH><TH NOWRAP>Work Number</TH><TH NOWRAP>E-Mail Address</TH></TR>\n");
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			prints("<TD NOWRAP><A HREF=%s/contactview?contactid=%s>%s", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 1));
			if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(", ");
			prints("%s</A></TD>", sqlGetvalue(sqr, i, 2));
			prints("<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 3));
			prints("<TD NOWRAP><A HREF=%s/mailwrite?msgto=%s>%s</A>&nbsp;</TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 4), sqlGetvalue(sqr, i, 4));
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void companysave(int sid)
{
	time_t t;
	char query[8192];
	int companyid;
	int i;
	int sqr;

	if (auth(sid, "contacts")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_companies WHERE companyid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		}
	}
	companyid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(companyid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
	}
	sqlFreeconnect(sqr);
	if (companyid==0) {
		if ((sqr=sqlQuery("SELECT max(companyid) FROM gw_companies"))<0) return;
		companyid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (companyid<1) companyid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", companyid);
		strcpy(query, "INSERT INTO gw_companies (");
		for (i=0;i<COMPANYFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<COMPANYFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<COMPANYFIELDS;i++) {
			strcatf(query, "'%s'", conn[sid].dat->tuple[i].value);
			if (i<COMPANYFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Company %d added successfully</CENTER><BR>\n", companyid);
		logaccess(1, "%s - %s added company %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, companyid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/companyview?companyid=%d\">\n", conn[sid].dat->in_ScriptName, companyid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "contacts")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_companies WHERE companyid = %d", companyid)<0) return;
		prints("<CENTER>Company %d deleted successfully</CENTER><BR>\n", companyid);
		logaccess(1, "%s - %s deleted company %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, companyid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/companylist\">\n", conn[sid].dat->in_ScriptName);
	} else {
		if (auth(sid, "contacts")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_companies SET ");
		for (i=2;i<COMPANYFIELDS;i++) {
			strcatf(query, "%s = '%s'", conn[sid].dat->tuple[i].name, conn[sid].dat->tuple[i].value);
			if (i<COMPANYFIELDS-1) strcat(query, ", ");
		}
		strcatf(query, " WHERE companyid = %d", atoi(conn[sid].dat->tuple[0].value));
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Company %d modified successfully</CENTER><BR>\n", companyid);
		logaccess(1, "%s - %s modified company %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, companyid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/companyview?companyid=%d\">\n", conn[sid].dat->in_ScriptName, companyid);
	}
	return;
}

void companymain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Contacts");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch04-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<A CLASS='TBAR' HREF=%s/contactlist>CONTACTS</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/companylist>COMPANIES</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
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
	if (strncmp(conn[sid].dat->in_RequestURI, "/companyedit", 12)==0)
		companyedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/companyview", 12)==0)
		companyview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/companylist", 12)==0)
		companylist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/companycontactlist", 19)==0)
		companycontactlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/companysave", 12)==0)
		companysave(sid);
	printfooter(sid);
	return;
}
