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

void contactedit(int sid)
{
	char lastmeetingdate[16];
	int contactid;
	int i;
	int sqr;

	if (auth(sid, "contacts")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/contacteditnew")==0) {
		contactid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
			conn[sid].dat->tuple[i].value[0]='\0';
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
	} else {
		if (getgetenv(sid, "CONTACTID")==NULL) return;
		contactid=atoi(getgetenv(sid, "CONTACTID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_contacts WHERE contactid = %d", contactid))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", contactid);
			sqlFreeconnect(sqr);
			return;
		}
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		sqlFreeconnect(sqr);
		snprintf(lastmeetingdate, 10, "%s", field(CONTACTFIELDS, "lastmeetingdate"));
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/contactsave NAME=contactedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=contactid VALUE='%s'>\n", field(CONTACTFIELDS, "contactid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(CONTACTFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(CONTACTFIELDS, "mdatetime"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4>", COLOR_TRIM);
	if (contactid>0) {
		prints("Contact %d</TH></TR>\n", contactid);
	} else {
		prints("New Contact</TH></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Last Name     </B></TD><TD><INPUT TYPE=TEXT NAME=lastname        value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "lastname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>First Name    </B></TD><TD><INPUT TYPE=TEXT NAME=firstname       value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "firstname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Title         </B></TD><TD><INPUT TYPE=TEXT NAME=jobtitle        value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "jobtitle"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Company       </B></TD><TD><SELECT NAME=companyid WIDTH=20>", COLOR_EDITFORM);
	companyselect(sid, atoi(field(CONTACTFIELDS, "companyid")));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Contact Type  </B></TD><TD><INPUT TYPE=TEXT NAME=contacttype     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "contacttype"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Address       </B></TD><TD><INPUT TYPE=TEXT NAME=address         value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "address"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>City, Province</B></TD><TD><INPUT TYPE=TEXT NAME=city            value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "city"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Country       </B></TD><TD><INPUT TYPE=TEXT NAME=country         value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "country"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Postal Code   </B></TD><TD><INPUT TYPE=TEXT NAME=postal          value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "postal"));
	prints("</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Work Phone    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "worknumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Home Phone    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "homenumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Fax Number    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber       value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "faxnumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Cellular      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "cellnumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Pager         </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "pagernumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail        </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email           value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "email"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Last Meeting  </B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints("<SELECT NAME=lastmeetingdate2>\n");
	monthselect(sid, lastmeetingdate);
	prints("</SELECT>");
	prints("<SELECT NAME=lastmeetingdate1>\n");
	dayselect(sid, lastmeetingdate);
	prints("</SELECT>");
	prints("<SELECT NAME=lastmeetingdate3>\n");
	yearselect(sid, 1900, lastmeetingdate);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Referred By   </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=referredby      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "referredby"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Customer      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=iscustomer      value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "iscustomer"));
	prints("</TABLE>\n</TD></TR>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Notes</TH></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "notes"));
	prints("<TR BGCOLOR=%s><TH COLSPAN=2><TEXTAREA WRAP=virtual NAME=notes ROWS=5 COLS=60>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, field(CONTACTFIELDS, "notes"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth(sid, "contacts")>2)&&(contactid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.contactedit.lastname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void contactview(int sid)
{
	int i;
	int contactid;
	int sqr;

	if (auth(sid, "contacts")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if ((sqr=sqlQueryf("SELECT * FROM gw_contacts WHERE contactid = %d", contactid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %d</CENTER>\n", contactid);
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
	prints("<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP>Contact %d", COLOR_TRIM, contactid);
	if (auth(sid, "contacts")>2) {
		prints(" [<A HREF=%s/contactedit?contactid=%d>edit</A>]", conn[sid].dat->in_ScriptName, contactid);
	}
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Last Name     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "lastname"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Work Phone    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "worknumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>First Name    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "firstname"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Home Phone    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "homenumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Title         </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "jobtitle"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Fax Number    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "faxnumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Company       </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
	if ((sqr=sqlQueryf("SELECT companyid, companyname FROM gw_companies WHERE companyid = %d", atoi(field(CONTACTFIELDS, "companyid"))))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints("<A HREF=%s/companyview?companyid=%d>", conn[sid].dat->in_ScriptName, atoi(sqlGetvalue(sqr, 0, 0)));
		prints("%s&nbsp;</A></TD>\n", sqlGetvalue(sqr, 0, 1));
	} else {
		prints("&nbsp;</TD>\n");
	}
	sqlFreeconnect(sqr);
	prints("    <TD BGCOLOR=%s NOWRAP><B>Cellular      </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "cellnumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Contact Type  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "contacttype"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Pager         </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "pagernumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Address       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "address"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>E-mail        </B></TD><TD BGCOLOR=%s NOWRAP><A HREF=%s/mailwrite?msgto=%s>%s</A>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, conn[sid].dat->in_ScriptName, field(CONTACTFIELDS, "email"), field(CONTACTFIELDS, "email"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>City, Province</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "city"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Last Meeting  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, sqldate2text(field(CONTACTFIELDS, "lastmeetingdate")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Country       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "country"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Referred By   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "referredby"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Postal Code   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "postal"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Customer      </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CONTACTFIELDS, "iscustomer"));
	prints("<TR><TD BGCOLOR=%s COLSPAN=4><B>Notes</B></TD></TR>\n", COLOR_FNAME);
	prints("<TR><TD BGCOLOR=%s COLSPAN=4><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FTEXT, field(CONTACTFIELDS, "notes"));
	prints("</TABLE>\n");
	if (auth(sid, "calendar")>0) {
		prints("[<A HREF=%s/contacteventlist?contactid=%d>View Events</A>]\n", conn[sid].dat->in_ScriptName, contactid);
	}
	if (auth(sid, "orders")>0) {
		prints("[<A HREF=%s/contactorderlist?contactid=%d>View Orders</A>]\n", conn[sid].dat->in_ScriptName, contactid);
	}
	prints("</CENTER>\n");
	return;
}

void contactlist(int sid)
{
	char query[2048];
	char searchstring[100];
	int offset=0;
	int i;
	int j;
	int sqr1;
	int sqr2;

	if (auth(sid, "contacts")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(query, 0, sizeof(query));
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0) {
		prints("<CENTER>\n");
		prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
		prints("<FORM METHOD=POST ACTION=%s/contactlist NAME=contactsearch>\n", conn[sid].dat->in_ScriptName);
		prints("<TR BGCOLOR=%s><TH COLSPAN=3>Contact Search Form</TH></TR>\n", COLOR_TRIM);
		prints("<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", COLOR_EDITFORM);
		prints("<OPTION SELECTED>All Columns\n");
		if ((sqr1=sqlQuery("SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "mdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "lastmeetingdate")==0) continue;
			prints("<OPTION>%s\n", sqlGetfieldname(sqr1, i));
		}
		sqlFreeconnect(sqr1);
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
		if ((sqr1=sqlQuery("SELECT * FROM gw_companies WHERE companyid = 1"))<0) return;
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
		if (auth(sid, "contacts")>1) {
			prints("<A HREF=%s/companyeditnew>New Company</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.contactsearch.searchstring.focus();\n// -->\n</SCRIPT>\n");
		return;
	}
	strncpy(searchstring, getpostenv(sid, "SEARCHSTRING"), sizeof(searchstring)-1);
	SwapChar(searchstring, '*', '%');
	SwapChar(searchstring, '?', '_');
	if ((sqr1=sqlQuery("SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
	strcpy(query, "SELECT contactid, lastname, firstname, companyid, worknumber, email from gw_contacts WHERE (");
	if (strcmp(getpostenv(sid, "COLUMN"), "All Columns")==0) {
		for (i=0;i<sqlNumfields(sqr1);i++) {
			if (strcmp(sqlGetfieldname(sqr1, i), "cdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "mdatetime")==0) continue;
			if (strcmp(sqlGetfieldname(sqr1, i), "lastmeetingdate")==0) continue;
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcatf(query, "%s like '%s' or ", sqlGetfieldname(sqr1, i), searchstring);
			} else {
				strcatf(query, "lower(%s) like lower('%s') or ", sqlGetfieldname(sqr1, i), searchstring);
			}
		}
		strcatf(query, "contactid like '%s'", searchstring);
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			strcatf(query, "%s like '%s'", getpostenv(sid, "COLUMN"), searchstring);
		} else {
			strcatf(query, "lower(%s) like lower('%s')", getpostenv(sid, "COLUMN"), searchstring);
		}
	}
	strcat(query, ") ORDER BY lastname, firstname ASC");
	sqlFreeconnect(sqr1);
	prints("<CENTER>\n");
	if ((sqr1=sqlQuery(query))<0) return;
	if (getgetenv(sid, "OFFSET")!=NULL)
		offset=atoi(getgetenv(sid, "OFFSET"));
	prints("Found %d matching contacts<BR>\n", sqlNumtuples(sqr1));
	if (sqlNumtuples(sqr1)<1) {
		sqlFreeconnect(sqr1);
		if (auth(sid, "contacts")>1) {
			prints("<A HREF=%s/contacteditnew>New Contact</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER><BR>\n");
		return;
	}
	if ((sqr2=sqlQuery("SELECT companyid, companyname FROM gw_companies"))<0) return;
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH>Contact Name</TH><TH>Company Name</TH><TH>Work Number</TH><TH>E-mail</TH></TR>\n", COLOR_TRIM);
	for (i=offset;(i<sqlNumtuples(sqr1))&&(i<offset+50);i++) {
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD NOWRAP><A HREF=%s/contactview?contactid=%s>%s", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr1, i, 0), sqlGetvalue(sqr1, i, 1));
		if (strlen(sqlGetvalue(sqr1, i, 1))&&strlen(sqlGetvalue(sqr1, i, 2))) prints(", ");
		prints("%s</A></TD>", sqlGetvalue(sqr1, i, 2));
		for (j=0;j<sqlNumtuples(sqr2);j++) {
			if (atoi(sqlGetvalue(sqr2, j, 0))==atoi(sqlGetvalue(sqr1, i, 3))) {
				prints("<TD NOWRAP><A HREF=%s/companyview?companyid=%s>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr1, i, 3));
				prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr2, j, 1));
				break;
			}
		}
		if (j==sqlNumtuples(sqr2)) {
			prints("<TD NOWRAP>&nbsp;</TD>");
		}
		prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr1, i, 4));
		prints("<TD NOWRAP><A HREF=%s/mailwrite?msgto=%s>%s</A>&nbsp;</TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr1, i, 5), sqlGetvalue(sqr1, i, 5));
		prints("</TR>\n");
	}
	prints("</TABLE>\n<TABLE><TR>\n");
	if (offset>49) {
		prints("<TD><FORM METHOD=POST ACTION=%s/contactlist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset-50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Previous 50'>\n");
		prints("</FORM></TD>\n");
	}
	if (offset+50<sqlNumtuples(sqr1)) {
		prints("<TD><FORM METHOD=POST ACTION=%s/contactlist?offset=%d>\n", conn[sid].dat->in_ScriptName, offset+50);
		prints("<INPUT TYPE=HIDDEN NAME=column VALUE='%s'>\n", getpostenv(sid, "COLUMN"));
		prints("<INPUT TYPE=HIDDEN NAME=searchstring VALUE='%s'>\n", searchstring);
		prints("<INPUT TYPE=SUBMIT VALUE='Next 50'>\n");
		prints("</FORM></TD>\n");
	}
	prints("</TR></TABLE>\n");
	if (auth(sid, "contacts")>1) {
		prints("<A HREF=%s/contacteditnew>New Contact</A>\n", conn[sid].dat->in_ScriptName);
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr1);
	sqlFreeconnect(sqr2);
	return;
}

/****************************************************************************
 *	contacteventlist()
 *
 *	Purpose	: Display a list of events for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contacteventlist(int sid)
{
	char *ispublic[2]={ "Private", "Public" };
	char *priority[5]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int contactid=0;
	int i;
	int sqr;
	int sqr2;

	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if (contactid<1) {
		prints("<CENTER>No matching events found</CENTER>\n");
		return;
	}
	prints("<CENTER>\n");
	if ((sqr=sqlQueryf("SELECT eventid, eventname, eventdate, eventstatus, ispublic, priority, contactid FROM gw_events WHERE contactid = %d and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate DESC", contactid, conn[sid].dat->in_username))<0) return;
	if ((sqr2=sqlQueryf("SELECT contactid, lastname, firstname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints("Found %d events for <A HREF=%s/contactview?contactid=%s>%s", sqlNumtuples(sqr), conn[sid].dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), sqlGetvalue(sqr2, 0, 1));
	if (strlen(sqlGetvalue(sqr2, 0, 1))&&strlen(sqlGetvalue(sqr2, 0, 2))) prints(", ");
	prints("%s</A><BR>\n", sqlGetvalue(sqr2, 0, 2));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		prints("<TH NOWRAP>Event ID</TH><TH NOWRAP>Event Name</TH><TH NOWRAP>Date</TH><TH NOWRAP>Status</TH><TH NOWRAP>Public</TH><TH NOWRAP>Priority</TH></TR>\n");
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			prints("<TD ALIGN=right NOWRAP><A HREF=%s/calendarview?eventid=%s>%s</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 0));
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 1));
			prints("<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", sqldate2text(sqlGetvalue(sqr, i, 2)));
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, 3));
			prints("<TD NOWRAP>%s&nbsp;</TD>", ispublic[atoi(sqlGetvalue(sqr, i, 4))]);
			prints("<TD NOWRAP>%s&nbsp;</TD>", priority[atoi(sqlGetvalue(sqr, i, 5))]);
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	prints("<A HREF=%s/calendareditnew?contactid=%d>New Event</A>\n", conn[sid].dat->in_ScriptName, contactid);
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

/****************************************************************************
 *	contactorderlist()
 *
 *	Purpose	: Display a list of orders for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contactorderlist(int sid)
{
	int customerid=0;
	int i;
	int sqr;
	int sqr2;

	if (auth(sid, "orders")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	customerid=atoi(getgetenv(sid, "CONTACTID"));
	if (customerid<1) {
		prints("<CENTER>No matching orders found</CENTER>\n");
		return;
	}
	prints("<CENTER>\n");
	if ((sqr=sqlQueryf("SELECT orderid, orderdate, paymentdue, paymentreceived from gw_orders WHERE customerid = %d ORDER BY orderdate DESC", customerid))<0) return;
	if ((sqr2=sqlQueryf("SELECT contactid, lastname, firstname FROM gw_contacts WHERE contactid = %d", customerid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints("Found %d orders for <A HREF=%s/contactview?contactid=%s>%s", sqlNumtuples(sqr), conn[sid].dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), sqlGetvalue(sqr2, 0, 1));
	if (strlen(sqlGetvalue(sqr2, 0, 1))&&strlen(sqlGetvalue(sqr2, 0, 2))) prints(", ");
	prints("%s</A><BR>\n", sqlGetvalue(sqr2, 0, 2));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
		prints("<TH NOWRAP>Order ID</TH><TH NOWRAP>Order Date</TH><TH NOWRAP>Amount Due</TH><TH NOWRAP>Amount Received</TH></TR>\n");
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			prints("<TD ALIGN=right NOWRAP><A HREF=%s/orderview?orderid=%d>%d</A></TD>", conn[sid].dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), atoi(sqlGetvalue(sqr, i, 0)));
			prints("<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", sqldate2text(sqlGetvalue(sqr, i, 1)));
			prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 2)));
			prints("<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3)));
			prints("</TR>\n");
		}
		prints("</TABLE>\n");
	}
	prints("<A HREF=%s/ordereditnew?customerid=%d>New Order</A>\n", conn[sid].dat->in_ScriptName, customerid);
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void contactsave(int sid)
{
	time_t t;
	char query[8192];
	int contactid;
	int i;
	int sqr;

	if (auth(sid, "contacts")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (strcmp(conn[sid].dat->tuple[i].name, "lastmeetingdate")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "LASTMEETINGDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "LASTMEETINGDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "LASTMEETINGDATE1"));
		} else 	if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		}
		if (strcmp(conn[sid].dat->tuple[i].name, "companyid")==0)
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%d", atoi(conn[sid].dat->tuple[i].value));
	}
	contactid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(contactid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "lastmeetingdate")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
	}
	sqlFreeconnect(sqr);
	if (contactid==0) {
		if ((sqr=sqlQuery("SELECT max(contactid) FROM gw_contacts"))<0) return;
		contactid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (contactid<1) contactid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", contactid);
		strcpy(query, "INSERT INTO gw_contacts (");
		for (i=0;i<CONTACTFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<CONTACTFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<CONTACTFIELDS;i++) {
			strcatf(query, "'%s'", conn[sid].dat->tuple[i].value);
			if (i<CONTACTFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Contact %d added successfully</CENTER><BR>\n", contactid);
		logaccess(1, "%s - %s added contact %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, contactid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contactview?contactid=%d\">\n", conn[sid].dat->in_ScriptName, contactid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "contacts")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_contacts WHERE contactid = %d", contactid)<0) return;
		prints("<CENTER>Contact %d deleted successfully</CENTER><BR>\n", contactid);
		logaccess(1, "%s - %s deleted contact %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, contactid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contactlist\">\n", conn[sid].dat->in_ScriptName);
	} else {
		if (auth(sid, "contacts")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_contacts SET ");
		for (i=2;i<CONTACTFIELDS;i++) {
			strcatf(query, "%s = '%s'", conn[sid].dat->tuple[i].name, conn[sid].dat->tuple[i].value);
			if (i<CONTACTFIELDS-1) strcat(query, ", ");
		}
		strcatf(query, " WHERE contactid = %d", atoi(conn[sid].dat->tuple[0].value));
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Contact %d modified successfully</CENTER><BR>\n", contactid);
		logaccess(1, "%s - %s modified contact %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, contactid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contactview?contactid=%d\">\n", conn[sid].dat->in_ScriptName, contactid);
	}
	return;
}

void contactmain(int sid)
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
	if (strncmp(conn[sid].dat->in_RequestURI, "/contactedit", 12)==0)
		contactedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contactview", 12)==0)
		contactview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contactlist", 12)==0)
		contactlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contactorderlist", 17)==0)
		contactorderlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contacteventlist", 17)==0)
		contacteventlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contactsave", 12)==0)
		contactsave(sid);
	printfooter(sid);
	return;
}
