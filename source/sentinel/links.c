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

void linksedit(int sid)
{
	int i;
	int linkid;
	int sqr;

	if (auth(sid, "links")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/linkseditnew")==0) {
		linkid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_links WHERE linkid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
			conn[sid].dat->tuple[i].value[0]='\0';
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
		snprintf(conn[sid].dat->tuple[6].value, sizeof(conn[sid].dat->tuple[6].value)-1, "http://");
	} else {
		if (getgetenv(sid, "LINKID")==NULL) return;
		linkid=atoi(getgetenv(sid, "LINKID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_links WHERE linkid = %d", linkid))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", linkid);
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
	prints("<FORM METHOD=POST ACTION=%s/linkssave NAME=linkedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=linkid VALUE='%s'>\n", field(LINKFIELDS, "linkid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(LINKFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(LINKFIELDS, "mdatetime"));
	prints("<INPUT TYPE=hidden NAME=creator VALUE='%s'>\n", field(LINKFIELDS, "creator"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (linkid!=0) {
		prints("<TR BGCOLOR=%s><TH COLSPAN=2>Link Number %d</TH></TR>\n", COLOR_TRIM, linkid);
	} else {
		prints("<TR BGCOLOR=%s><TH COLSPAN=2>New Link\n</TH></TR>", COLOR_TRIM);
	}
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Link Name   </B></TD><TD><INPUT TYPE=TEXT NAME=linkname VALUE='%s' SIZE=30></TD></TR>\n", COLOR_EDITFORM, field(LINKFIELDS, "linkname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Link Type   </B></TD><TD><INPUT TYPE=TEXT NAME=linktype VALUE='%s' SIZE=30></TD></TR>\n", COLOR_EDITFORM, field(LINKFIELDS, "linktype"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Link Address</B></TD><TD><INPUT TYPE=TEXT NAME=linkurl  VALUE='%s' SIZE=30></TD></TR>\n", COLOR_EDITFORM, field(LINKFIELDS, "linkurl"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth(sid, "links")>2)&&(linkid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.linkedit.linkname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void linkslist(int sid)
{
	int authlevel;
	int i;
	int sqr;
	
	if (auth(sid, "links")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	authlevel=auth(sid, "links");
	if ((sqr=sqlQuery("SELECT linkid, linkname, linkurl, linktype FROM gw_links ORDER BY linkname ASC"))<0) return;
	prints("<CENTER>\n");
	if (sqlNumtuples(sqr)>0) {
		prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
		prints("<TR BGCOLOR=%s>", COLOR_TRIM);
		if (authlevel>2) {
			prints("<TH>&nbsp;</TH>");
		}
		prints("<TH ALIGN=left>Web Site</TH><TH ALIGN=left>Link Type</TH></TR>\n");
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			if (authlevel>2) {
				prints("<TD NOWRAP><A HREF=%s/linksedit?linkid=%s>edit</A>&nbsp;</TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			}
			prints("<TD NOWRAP><A HREF=%s TARGET=_blank>", sqlGetvalue(sqr, i, 2));
			prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr, i, 1));
			prints("<TD NOWRAP>%s&nbsp;</TD></TR>\n", sqlGetvalue(sqr, i, 3));
		}
		prints("</TABLE>\n");
	} else {
		prints("There are no saved links<BR>\n");
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	prints("<SCRIPT LANGUAGE='JavaScript'>\n<!--\n");
	prints("function startSearch()\n");
	prints("{\n");
	prints("	searchString = document.searchForm.searchText.value;\n");
	prints("	if(searchString != '') {\n");
	prints("		searchEngine = document.searchForm.whichEngine.selectedIndex + 1;\n");
	prints("		finalSearchString = '';\n");
	prints("		if(searchEngine == 1)\n");
	prints("			finalSearchString = 'http://www.altavista.com/cgi-bin/query?pg=q&what=web&fmt=.&q=' + searchString;\n");
	prints("		if(searchEngine == 2)\n");
	prints("			finalSearchString = 'http://netfind.aol.com/search.gw?search=' + searchString + '&c=web&lk=excite_netfind_us&src=1';\n");
	prints("		if(searchEngine == 3)\n");
	prints("			finalSearchString = 'http://www.excite.com/search.gw?trace=a&search=' + searchString;\n");
	prints("		if(searchEngine == 4)\n");
	prints("			finalSearchString = 'http://www.hotbot.com/?SW=web&SM=MC&MT=' + searchString + '&DC=10&DE=2&RG=NA&_v=2&act.search.x=89&act.search.y=7';\n");
	prints("		if(searchEngine == 5)\n");
	prints("			finalSearchString = 'http://www.lycos.com/cgi-bin/pursuit?adv=%%26adv%%3B&cat=lycos&matchmode=and&query=' + searchString + '&x=45&y=11';\n");
	prints("		if(searchEngine == 6)\n");
	prints("			finalSearchString = 'http://search.msn.com/results.asp?RS=CHECKED&FORM=MSNH&v=1&q=' + searchString;\n");
	prints("		if(searchEngine == 7)\n");
	prints("			finalSearchString = 'http://search.yahoo.com/bin/search?p=' + searchString + '&hc=0&hs=0';\n");
	prints("		window.open(finalSearchString, '_blank');\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n<FORM NAME='searchForm' ACTION=javascript:startSearch()>\n");
	prints("<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0><TR BGCOLOR=%s><TD>\n", COLOR_EDITFORM);
	prints("<SELECT name='whichEngine'>\n");
	prints("<OPTION>Altavista\n");
	prints("<OPTION>AOL Netfind\n");
	prints("<OPTION>Excite\n");
	prints("<OPTION>Hotbot\n");
	prints("<OPTION>Lycos\n");
	prints("<OPTION>MSN\n");
	prints("<OPTION SELECTED>Yahoo!\n");
	prints("</SELECT></TD>\n");
	prints("<TD><INPUT TYPE=TEXT NAME='searchText' SIZE=20></TD>\n");
	prints("<TD><INPUT TYPE='button' VALUE='Search' ONCLICK='startSearch()'></TD></TR>\n");
	prints("</TABLE>\n");
	prints("</FORM>\n</CENTER>\n");
	return;
}

void linkssave(int sid)
{
	time_t t;
	char query[8192];
	int linkid;
	int i;
	int sqr;

	if (auth(sid, "links")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_links WHERE linkid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL)
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
	}
	linkid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(linkid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "category")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "Uncategorized");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "creator")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
	}
	sqlFreeconnect(sqr);
	if (linkid==0) {
		if ((sqr=sqlQuery("SELECT max(linkid) FROM gw_links"))<0) return;
		linkid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (linkid<1) linkid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", linkid);
		strcpy(query, "INSERT INTO gw_links (");
		for (i=0;i<LINKFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<LINKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<LINKFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<LINKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>HyperLink %d added successfully</CENTER><BR>\n", linkid);
		logaccess(1, "%s - %s added hyperlink %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, linkid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "links")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_links WHERE linkid = %d", linkid)<0) return;
		prints("<CENTER>Link %d deleted successfully</CENTER><BR>\n", linkid);
		logaccess(1, "%s - %s deleted hyperlink %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, linkid);
	} else {
		if (auth(sid, "links")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_links SET ");
		for (i=2;i<LINKFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			strcat(query, " = '");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<LINKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE linkid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Hyperlink %d modified successfully</CENTER><BR>\n", linkid);
		logaccess(1, "%s - %s modified hyperlink %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, linkid);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/linkslist\">\n", conn[sid].dat->in_ScriptName);
	return;
}

void linksmain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Links");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch07-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/linkslist>VIEW HYPERLINKS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/linkseditnew>ADD HYPERLINK</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
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
	if (strncmp(conn[sid].dat->in_RequestURI, "/linksedit", 10)==0)
		linksedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/linkslist", 10)==0)
		linkslist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/linkssave", 10)==0)
		linkssave(sid);
	printfooter(sid);
	return;
}
