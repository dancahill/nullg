/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "ghttpd/mod.h"
#include "mod_searches.h"

void searchcontacts(CONN *sid)
{
	char query[2048];
	char column[100];
	char string[100];
	char string2[100];
	char *ptemp;
	int offset=0;
	int i;
	SQLRES sqr1;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(column, 0, sizeof(column));
	memset(string, 0, sizeof(string));
	memset(string2, 0, sizeof(string2));
	if ((ptemp=getgetenv(sid, "COLUMN"))!=NULL) snprintf(column, sizeof(column)-1, "%s", ptemp);
	if ((ptemp=getgetenv(sid, "STRING"))!=NULL) snprintf(string, sizeof(string)-1, "%s", ptemp);
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if (offset<0) offset=0;
	snprintf(string2, sizeof(string2)-1, "%%%s%%", string);
	ptemp=string2;
	while (*ptemp) {
		if (*ptemp=='*') *ptemp='%';
		if (*ptemp=='?') *ptemp='_';
		ptemp++;
	}
	if (sql_query(&sqr1, "SELECT * FROM gw_contacts WHERE contactid = 1")<0) return;
	strcpy(query, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (");
	if (strcmp(column, "All Columns")==0) {
		for (i=0;i<sql_numfields(&sqr1);i++) {
			if (strcmp(sql_getname(&sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_operm")==0) continue;
			if (strcmp(config->sql_type, "ODBC")==0) {
				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(&sqr1, i), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
			} else {
				strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(&sqr1, i), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
			}
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid like '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
	} else {
		if (strcmp(config->sql_type, "ODBC")==0) {
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", column, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
		}
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		strncatf(query, sizeof(query)-strlen(query)-1, ") AND obj_did = %d ORDER BY surname, givenname ASC", sid->dat->user_did);
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, ") and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did);
	}
	sql_freeresult(&sqr1);
	prints(sid, "<CENTER>\n");
	if (sql_query(&sqr1, query)<0) return;
	if (sql_numtuples(&sqr1)<1) {
		prints(sid, "<B>Found %d matching contact%s</B></CENTER>\n", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s");
		sql_freeresult(&sqr1);
		return;
	} else if (sql_numtuples(&sqr1)==1) {
		prints(sid, "<B>Found %d matching contact%s</B>\n", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\n<!--\n");
		prints(sid, "location.replace(\"%s/contacts/view?contactid=%d\");\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, 0, 0)));
		prints(sid, "// -->\n</SCRIPT>\n");
		prints(sid, "<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, 0, 0)));
		prints(sid, "</NOSCRIPT>\n");
		prints(sid, "<BR><A HREF=\"%s/contacts/view?contactid=%d\">Click here</A>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, 0, 0)));
		prints(sid, "</CENTER>\n");
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\r\n");
	prints(sid, "function findMailtoLink(text) {\r\n");
	prints(sid, "	for (var i=0;i<document.links.length;i++) {\r\n");
	prints(sid, "		if (document.links[i].href==text) return i;\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	return null;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailUpdate() {\r\n");
	prints(sid, "	var entries=document.mailform.elements.length/2;\r\n");
	if (sid->dat->user_maildefault==0) {
		prints(sid, "	var output=\"mailto:\";\r\n");
	} else {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "	var output=\"javascript:MailTo('\";\r\n");
		} else {
			prints(sid, "	var output=\"%s/mail/write?\";\r\n", sid->dat->in_ScriptName);
		}
	}
	prints(sid, "	var cc=0;\r\n");
	prints(sid, "	var j=0;\r\n\r\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "	output+='to=';\r\n");
	}
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='TO') {\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='CC') {\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			if (j==0) output+='&cc=';\r\n");
	} else {
		prints(sid, "			if (j==0) output+='?cc=';\r\n");
	}
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			else if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			else if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			cc=1;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='BCC') {\n");
	prints(sid, "			if (j==0) {\r\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "				if (j==0) output+='&bcc=';\r\n");
	} else {
		prints(sid, "				if (cc==1) {\r\n");
		prints(sid, "					output+='&bcc=';\r\n");
		prints(sid, "				} else {\r\n");
		prints(sid, "					output+='?bcc=';\r\n");
		prints(sid, "				}\r\n");
	}
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			} else if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			} else if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	if ((sid->dat->user_maildefault>0)&&(sid->dat->user_menustyle>0)) {
		prints(sid, "	output+=\"')\";\r\n");
	}
	prints(sid, "	document.links[mailtoLink2].href=output;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailTo(rcptlist) {\r\n");
	prints(sid, "	window.open('%s/mail/write?'+rcptlist,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\n");
	prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP><B>Found %d matching contact%s</B></TD>\n", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailform>\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Contact Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Company Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Work Number&nbsp;</TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;E-Mail&nbsp;</TH></TR>\n");
	for (i=offset;(i<sql_numtuples(&sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), str2html(sid, sql_getvalue(&sqr1, i, 1)));
		if (strlen(sql_getvalue(&sqr1, i, 1))&&strlen(sql_getvalue(&sqr1, i, 2))) prints(sid, ", ");
		if (strlen(sql_getvalue(&sqr1, i, 1))==0&&strlen(sql_getvalue(&sqr1, i, 2))==0) prints(sid, "&nbsp;");
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 2)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 3)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 4)));
		if (strlen(sql_getvalue(&sqr1, i, 5))==0) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		} else if (sid->dat->user_maildefault==0) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"mailto:%s\">%s</A>&nbsp;</TD>", sql_getvalue(&sqr1, i, 5), sql_getvalue(&sqr1, i, 5));
		} else {
			if (sid->dat->user_menustyle>0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"javascript:MsgTo('");
				prints(sid, "&quot;%s%s%s&quot;", str2html(sid, sql_getvalue(&sqr1, i, 2)), strlen(sql_getvalue(&sqr1, i, 2))?" ":"", str2html(sid, sql_getvalue(&sqr1, i, 1)));
				prints(sid, " <%s>')\">%s</A>&nbsp;</TD>", sql_getvalue(&sqr1, i, 5), str2html(sid, sql_getvalue(&sqr1, i, 5)));
			} else {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"%s/mail/write?to=%s\">%s</A>&nbsp;</TD>", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 5), str2html(sid, sql_getvalue(&sqr1, i, 5)));
			}
		}
		prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", i-offset, sql_getvalue(&sqr1, i, 5));
		prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid'><SELECT NAME=option%d onchange=MailUpdate(); STYLE='font-size:11px; width:44px'>", i-offset);
		prints(sid, "<OPTION VALUE=''>");
		if (strchr(sql_getvalue(&sqr1, i, 5), '@')) {
			prints(sid, "<OPTION VALUE='TO'>TO");
			prints(sid, "<OPTION VALUE='CC'>CC");
			prints(sid, "<OPTION VALUE='BCC'>BCC");
		}
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR>\n<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP>");
	if (sql_numtuples(&sqr1)>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/search/contacts?column=", sid->dat->in_ScriptName);
			printhex(sid, "%s", column);
			prints(sid, "&string=");
			printhex(sid, "%s", string);
			prints(sid, "&offset=%d>Previous Page</A>]", offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<sql_numtuples(&sqr1)) {
			prints(sid, "[<A HREF=%s/search/contacts?column=", sid->dat->in_ScriptName);
			printhex(sid, "%s", column);
			prints(sid, "&string=");
			printhex(sid, "%s", string);
			prints(sid, "&offset=%d>Next Page</A>]", offset+sid->dat->user_maxlist);
		} else {
			prints(sid, "[Next Page]");
		}
	}
	prints(sid, "</TD>\n<TD ALIGN=RIGHT NOWRAP><A HREF=\"mailto:list\" onClick=\"MailUpdate()\">Send E-Mail</A></TD>\n</TR>");
	prints(sid, "</TABLE>\n");
	prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\n");
	prints(sid, "var mailtoLink2 = findMailtoLink('mailto:list');\n");
	prints(sid, "//--></SCRIPT>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(&sqr1);
	return;
}
