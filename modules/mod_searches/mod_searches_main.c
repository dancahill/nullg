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
#include "mod_searches.h"

void searchform(CONN *sid)
{
	int i;
	int sqr;

	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function startSearch()\n");
	prints(sid, "{\n");
	prints(sid, "	searchString = document.searchForm.searchText.value;\n");
	prints(sid, "	if(searchString != '') {\n");
	prints(sid, "		searchEngine = document.searchForm.whichEngine.selectedIndex + 1;\n");
	prints(sid, "		finalSearchString = '';\n");
	prints(sid, "		if(searchEngine == 1)\n");
	prints(sid, "			finalSearchString = 'http://www.altavista.com/cgi-bin/query?pg=q&what=web&fmt=.&q=' + searchString;\n");
	prints(sid, "		if(searchEngine == 2)\n");
	prints(sid, "			finalSearchString = 'http://netfind.aol.com/search.gw?search=' + searchString + '&c=web&lk=excite_netfind_us&src=1';\n");
	prints(sid, "		if(searchEngine == 3)\n");
	prints(sid, "			finalSearchString = 'http://www.excite.com/search.gw?trace=a&search=' + searchString;\n");
	prints(sid, "		if(searchEngine == 4)\n");
	prints(sid, "			finalSearchString = 'http://www.google.com/search?q=' + searchString;\n");
	prints(sid, "		if(searchEngine == 5)\n");
	prints(sid, "			finalSearchString = 'http://www.hotbot.com/?SW=web&SM=MC&MT=' + searchString + '&DC=10&DE=2&RG=NA&_v=2&act.search.x=89&act.search.y=7';\n");
	prints(sid, "		if(searchEngine == 6)\n");
	prints(sid, "			finalSearchString = 'http://www.lycos.com/cgi-bin/pursuit?adv=%%26adv%%3B&cat=lycos&matchmode=and&query=' + searchString + '&x=45&y=11';\n");
	prints(sid, "		if(searchEngine == 7)\n");
	prints(sid, "			finalSearchString = 'http://search.msn.com/results.asp?RS=CHECKED&FORM=MSNH&v=1&q=' + searchString;\n");
	prints(sid, "		if(searchEngine == 8)\n");
	prints(sid, "			finalSearchString = 'http://search.yahoo.com/bin/search?p=' + searchString + '&hc=0&hs=0';\n");
	prints(sid, "		window.open(finalSearchString, '_blank');\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (auth_priv(sid, "contacts")&A_READ) {
		prints(sid, "<FORM METHOD=GET ACTION=%s/search/contacts NAME=contactsearch>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Contact Search Form</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", config->colour_editform);
		prints(sid, "<OPTION SELECTED>All Columns\n");
		if ((sqr=sql_query(sid, "SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
		for (i=0;i<sql_numfields(sqr);i++) {
			if (strcmp(sql_getname(sqr, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(sqr, i), "loginip")==0) continue;
			if (strcmp(sql_getname(sqr, i), "logintime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(sqr, i), "password")==0) continue;
			if (strcmp(sql_getname(sqr, i), "enabled")==0) continue;
			if (strcmp(sql_getname(sqr, i), "geozone")==0) continue;
			if (strcmp(sql_getname(sqr, i), "timezone")==0) continue;
			prints(sid, "<OPTION>%s\n", sql_getname(sqr, i));
		}
		sql_freeresult(sqr);
		prints(sid, "</SELECT></TD>\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=string value='' SIZE=20></TD>\n");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\n");
		prints(sid, "</FORM>\n");
		prints(sid, "<TR><TD COLSPAN=3>&nbsp;</TD></TR>\n");
	}
	prints(sid, "<FORM NAME='searchForm' ACTION=javascript:startSearch()>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Internet Search Form</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD>\n", config->colour_editform);
	prints(sid, "<SELECT name='whichEngine'>\n");
	prints(sid, "<OPTION>Altavista\n");
	prints(sid, "<OPTION>AOL Netfind\n");
	prints(sid, "<OPTION>Excite\n");
	prints(sid, "<OPTION SELECTED>Google\n");
	prints(sid, "<OPTION>Hotbot\n");
	prints(sid, "<OPTION>Lycos\n");
	prints(sid, "<OPTION>MSN\n");
	prints(sid, "<OPTION>Yahoo!\n");
	prints(sid, "</SELECT></TD>\n");
	prints(sid, "<TD><INPUT TYPE=TEXT NAME='searchText' SIZE=20></TD>\n");
	prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Search' ONCLICK='startSearch()'></TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.searchForm.searchText.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void searchcontacts(CONN *sid)
{
	char query[2048];
	char column[100];
	char string[100];
	char string2[100];
	char *ptemp;
	int offset=0;
	int i;
	int sqr1;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
	if ((sqr1=sql_query(sid, "SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
	strcpy(query, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (");
	if (strcmp(column, "All Columns")==0) {
		for (i=0;i<sql_numfields(sqr1);i++) {
			if (strcmp(sql_getname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_operm")==0) continue;
			if (strcmp(config->sql_type, "ODBC")==0) {
				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(sqr1, i), str2sql(sid, string2));
			} else {
				strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(sqr1, i), str2sql(sid, string2));
			}
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid like '%s'", str2sql(sid, string2));
	} else {
		if (strcmp(config->sql_type, "ODBC")==0) {
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", column, str2sql(sid, string2));
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, str2sql(sid, string2));
		}
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		strncatf(query, sizeof(query)-strlen(query)-1, ") ORDER BY surname, givenname ASC");
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, ") and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid);
	}
	sql_freeresult(sqr1);
	prints(sid, "<CENTER>\n");
	if ((sqr1=sql_query(sid, query))<0) return;
	if (sql_numtuples(sqr1)<1) {
		prints(sid, "<B>Found %d matching contact%s</B></CENTER>\n", sql_numtuples(sqr1), sql_numtuples(sqr1)==1?"":"s");
		sql_freeresult(sqr1);
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
	prints(sid, "	window.open('%s/mail/write?'+rcptlist,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\n");
	prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP><B>Found %d matching contact%s</B></TD>\n", sql_numtuples(sqr1), sql_numtuples(sqr1)==1?"":"s");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3>\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", config->colour_tabletrim);
	prints(sid, "<FORM METHOD=GET NAME=mailform>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Contact Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Company Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Work Number&nbsp;</FONT></TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP><FONT COLOR=%s>&nbsp;E-Mail&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
	for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		prints(sid, "<TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), str2html(sid, sql_getvalue(sqr1, i, 1)));
		if (strlen(sql_getvalue(sqr1, i, 1))&&strlen(sql_getvalue(sqr1, i, 2))) prints(sid, ", ");
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 2)));
		prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 3)));
		prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 4)));
		prints(sid, "<TD NOWRAP><A HREF=javascript:MsgTo('%s')>%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 5)), str2html(sid, sql_getvalue(sqr1, i, 5)));
		prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", i-offset, sql_getvalue(sqr1, i, 5));
		prints(sid, "<TD NOWRAP STYLE='padding:0px'><SELECT NAME=option%d onchange=MailUpdate(); STYLE='font-size:11px; width:44px'>", i-offset);
		prints(sid, "<OPTION VALUE=''>");
		if (strchr(sql_getvalue(sqr1, i, 5), '@')) {
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
	if (sql_numtuples(sqr1)>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/search/contacts?column=", sid->dat->in_ScriptName);
			printhex(sid, "%s", column);
			prints(sid, "&string=");
			printhex(sid, "%s", string);
			prints(sid, "&offset=%d>Previous Page</A>]", offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
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
	sql_freeresult(sqr1);
	return;
}

/****************************************************************************
 *	queryadd()
 *
 *	Purpose	: Display form for adding or editing queries
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void searchsqladd(CONN *sid)
{
	REC_QUERY query;
	int queryid;

	if (!(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/search/sqladd")==0) {
		queryid=0;
		dbread_query(sid, 2, 0, &query);
	} else {
		if (getgetenv(sid, "QUERYID")==NULL) return;
		queryid=atoi(getgetenv(sid, "QUERYID"));
		if (dbread_query(sid, 2, queryid, &query)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", queryid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/search/sqlsave NAME=queryrun>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=HIDDEN NAME=queryid VALUE=%d>\n", queryid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (queryid!=0) {
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Query Number %d</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, query.queryid);
	} else {
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>New Query</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	}
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Query Name&nbsp;</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><INPUT TYPE=TEXT NAME=queryname VALUE=\"%s\" SIZE=60></TD></TR>\n", config->colour_editform, str2html(sid, query.queryname));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Query&nbsp;</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA NAME=query ROWS=3 COLS=60>%s</TEXTAREA></TD></TR>\n", config->colour_editform, query.query);
	if ((query.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "query")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, query.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "query")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, query.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", query.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", query.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", query.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", query.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", query.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", query.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if (queryid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.queryrun.queryname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

/****************************************************************************
 *	queryrun()
 *
 *	Purpose	: Execute query and display results
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void searchsqlrun(CONN *sid)
{
	REC_QUERY query;
	char querytype[10];
	char *ptemp;
	int queryid;
	int offset;
	int i, j;
	int sqr;
	int min, max;

	if (!(auth_priv(sid, "query")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	ptemp=getgetenv(sid, "QUERYID");
	if (ptemp) {
		queryid=atoi(ptemp);
	} else {
		queryid=0;
	}
	if (queryid==0) {
		if (auth_priv(sid, "query")&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT queryid, queryname, query FROM gw_queries ORDER BY queryname ASC"))<0) return;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT queryid, queryname, query FROM gw_queries where (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY queryname ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
		prints(sid, "<CENTER>\n");
		if (sql_numtuples(sqr)>0) {
			prints(sid, "Saved queries<BR>\n");
			prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n<TR BGCOLOR=%s>\n", config->colour_tabletrim, config->colour_th);
			if (auth_priv(sid, "query")&A_ADMIN) {
				prints(sid, "<TH ALIGN=LEFT>&nbsp;</TH>");
			}
			prints(sid, "<TH ALIGN=LEFT WIDTH=200 NOWRAP><FONT COLOR=%s>Query Name</FONT></TH></TR>\n", config->colour_thtext);
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
				if (auth_priv(sid, "query")&A_ADMIN) {
					prints(sid, "<TD NOWRAP><A HREF=%s/search/sqladd?queryid=%s>edit</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
				}
				prints(sid, "<TD NOWRAP><A HREF=%s/search/sqlrun?queryid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
				prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
			}
			prints(sid, "</TABLE>\n");
		} else {
			prints(sid, "There are no saved queries<BR>\n");
		}
		sql_freeresult(sqr);
		if (auth_priv(sid, "query")&A_ADMIN) {
			prints(sid, "<A HREF=%s/search/sqladd>New Query</A>\n", sid->dat->in_ScriptName);
		}
		prints(sid, "</CENTER>\n");
		return;
	}
	if (dbread_query(sid, 1, queryid, &query)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", queryid);
		return;
	}
	memset(querytype, 0, sizeof(querytype));
	strncpy(querytype, query.query, 6);
	ptemp=querytype;
	while (*ptemp) {
		if (*ptemp==' ') {
			*ptemp='\0';
			break;
		}
		*ptemp=toupper(*ptemp);
		ptemp++;
	}
	if (strcmp(querytype, "SELECT")==0) {
		if ((sqr=sql_query(sid, query.query))<0) {
			prints(sid, "<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "SHOW")==0) {
		if ((sqr=sql_query(sid, query.query))<0) {
			prints(sid, "<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "DELETE")==0) {
		if (sql_update(sid, query.query)<0) {
			prints(sid, "<CENTER>SQL delete failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL delete successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "INSERT")==0) {
		if (sql_update(sid, query.query)<0) {
			prints(sid, "<CENTER>SQL insert failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL insert successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "UPDATE")==0) {
		if (sql_update(sid, query.query)<0) {
			prints(sid, "<CENTER>SQL update failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL update successful.</CENTER>\n");
		}
		return;
	} else {
		prints(sid, "<CENTER>Unknown SQL query type.</CENTER>\n");
		return;
	}
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<CENTER>Query [%s] returned no results.</CENTER>\n", query.queryname);
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<CENTER>\n%s - %d results<BR>\n", query.queryname, sql_numtuples(sqr));
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", config->colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s>", config->colour_th);
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp);
	} else {
		offset=0;
	}
	for (i=0;i<sql_numfields(sqr);i++) {
		prints(sid, "<TH ALIGN=LEFT><FONT COLOR=%s>%s</FONT></TH>", config->colour_thtext, str2html(sid, sql_getname(sqr, i)));
	}
	prints(sid, "</TR>\n");
	for (i=offset;(i<sql_numtuples(sqr))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		for (j=0;j<sql_numfields(sqr);j++)
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, j)));
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE></CENTER>\n");
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>sid->dat->user_maxlist) {
		if (offset>(sid->dat->user_maxlist-1)) {
			prints(sid, "[<A HREF=%s/search/sqlrun?queryid=%d&offset=%d>", sid->dat->in_ScriptName, queryid, offset-sid->dat->user_maxlist);
			min=offset-sid->dat->user_maxlist+1;
			max=offset;
			if (min<0) min=0;
			if (min>sql_numtuples(sqr)) min=sql_numtuples(sqr);
			if (max<0) max=0;
			if (max>sql_numtuples(sqr)) max=sql_numtuples(sqr);
			prints(sid, "Previous (%d to %d)</A>]\n", min, max);
		} else {
			prints(sid, "[Previous]\n");
		}
		if (offset+sid->dat->user_maxlist<sql_numtuples(sqr)) {
			prints(sid, "[<A HREF=%s/search/sqlrun?queryid=%d&offset=%d>", sid->dat->in_ScriptName, queryid, offset+sid->dat->user_maxlist);
			min=offset+sid->dat->user_maxlist+1;
			max=offset+sid->dat->user_maxlist+sid->dat->user_maxlist;
			if (min<0) min=0;
			if (min>sql_numtuples(sqr)) min=sql_numtuples(sqr);
			if (max<0) max=0;
			if (max>sql_numtuples(sqr)) max=sql_numtuples(sqr);
			prints(sid, "Next (%d to %d)</A>]\n", min, max);
		} else {
			prints(sid, "[Next]\n");
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

/****************************************************************************
 *	querysave()
 *
 *	Purpose	: Update query or call queryrun() to run query
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void searchsqlsave(CONN *sid)
{
	REC_QUERY query;
	char *ptemp;
	int queryid;

	if (!(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "QUERYID"))==NULL) return;
	queryid=atoi(ptemp);
	if (dbread_query(sid, 2, queryid, &query)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "query")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) query.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) query.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "query")&A_ADMIN)||(query.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) query.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) query.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "QUERYNAME"))!=NULL) snprintf(query.queryname, sizeof(query.queryname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "QUERY"))!=NULL) snprintf(query.query, sizeof(query.query)-1, "%s", ptemp);
	if (strlen(query.queryname)==0) strncpy(query.queryname, "Unnamed Query", sizeof(query.queryname)-1);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_queries WHERE queryid = %d", queryid)<0) return;
		prints(sid, "<CENTER>Query %d deleted successfully</CENTER><BR>\n", queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "delete", "%s - %s deleted query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	} else if (query.queryid==0) {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((query.queryid=dbwrite_query(sid, 0, &query))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Query %d added successfully</CENTER><BR>\n", query.queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "insert", "%s - %s added query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	} else {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (dbwrite_query(sid, queryid, &query)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Query %d modified successfully</CENTER><BR>\n", query.queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "modify", "%s - %s modified query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/search/sqlrun\">\n", sid->dat->in_ScriptName);
	return;
}

/****************************************************************************
 *	querymain()
 *
 *	Purpose	: Execute appropriate function based on URI
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_SEARCHES);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/search/contacts", 16)==0) {
		searchcontacts(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/search/sqladd", 14)==0) {
		searchsqladd(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/search/sqlrun", 14)==0) {
		searchsqlrun(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/search/sqlsave", 15)==0) {
		searchsqlsave(sid);
	} else {
		searchform(sid);
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
	if (mod_export_main("mod_searches", "SEARCHES", "/search/", "mod_main", "/search/", mod_main)!=0) return -1;
	return 0;
}