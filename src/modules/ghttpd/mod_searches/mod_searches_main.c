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
#define SRVMOD_MAIN 1
#include "mod_searches.h"

void searchform(CONN *sid)
{
	int i;
	SQLRES sqr;

	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function startSearch()\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	searchString = document.searchForm.searchText.value;\r\n");
	prints(sid, "	if (searchString!='') {\r\n");
	prints(sid, "		searchEngine = document.searchForm.whichEngine.selectedIndex + 1;\r\n");
	prints(sid, "		finalSearchString = '';\r\n");
	prints(sid, "		if(searchEngine == 1)\r\n");
	prints(sid, "			finalSearchString = 'http://www.altavista.com/cgi-bin/query?pg=q&what=web&fmt=.&q=' + searchString;\r\n");
	prints(sid, "		if(searchEngine == 2)\r\n");
	prints(sid, "			finalSearchString = 'http://netfind.aol.com/search.gw?search=' + searchString + '&c=web&lk=excite_netfind_us&src=1';\r\n");
	prints(sid, "		if(searchEngine == 3)\r\n");
	prints(sid, "			finalSearchString = 'http://www.excite.com/search.gw?trace=a&search=' + searchString;\r\n");
	prints(sid, "		if(searchEngine == 4)\r\n");
	prints(sid, "			finalSearchString = 'http://www.google.com/search?q=' + searchString;\r\n");
	prints(sid, "		if(searchEngine == 5)\r\n");
	prints(sid, "			finalSearchString = 'http://www.hotbot.com/?SW=web&SM=MC&MT=' + searchString + '&DC=10&DE=2&RG=NA&_v=2&act.search.x=89&act.search.y=7';\r\n");
	prints(sid, "		if(searchEngine == 6)\r\n");
	prints(sid, "			finalSearchString = 'http://www.lycos.com/cgi-bin/pursuit?adv=%%26adv%%3B&cat=lycos&matchmode=and&query=' + searchString + '&x=45&y=11';\r\n");
	prints(sid, "		if(searchEngine == 7)\r\n");
	prints(sid, "			finalSearchString = 'http://search.msn.com/results.asp?RS=CHECKED&FORM=MSNH&v=1&q=' + searchString;\r\n");
	prints(sid, "		if(searchEngine == 8)\r\n");
	prints(sid, "			finalSearchString = 'http://search.yahoo.com/bin/search?p=' + searchString + '&hc=0&hs=0';\r\n");
	prints(sid, "		window.open(finalSearchString, '_blank');\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	if (auth_priv(sid, "contacts")&A_READ) {
		prints(sid, "<FORM METHOD=GET ACTION=%s/search/contacts NAME=contactsearch>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<TR><TH COLSPAN=3>Contact Search Form</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><SELECT NAME=column>\r\n");
		prints(sid, "<OPTION SELECTED>All Columns\r\n");
		if (sql_query(&sqr, "SELECT * FROM gw_contacts WHERE contactid = 1")<0) return;
		for (i=0;i<sql_numfields(&sqr);i++) {
			if (strcmp(sql_getname(&sqr, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "loginip")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "logintime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "password")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "enabled")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "geozone")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "timezone")==0) continue;
			prints(sid, "<OPTION>%s\r\n", sql_getname(&sqr, i));
		}
		sql_freeresult(&sqr);
		prints(sid, "</SELECT></TD>\r\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=string value='' SIZE=20></TD>\r\n");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\r\n");
		prints(sid, "</FORM>\r\n");
		prints(sid, "<TR><TD COLSPAN=3>&nbsp;</TD></TR>\r\n");
	}
	prints(sid, "<FORM NAME='searchForm' ACTION=javascript:startSearch()>\r\n");
	prints(sid, "<TR><TH COLSPAN=3>Internet Search Form</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD>\r\n");
	prints(sid, "<SELECT name='whichEngine'>\r\n");
	prints(sid, "<OPTION>Altavista\r\n");
	prints(sid, "<OPTION>AOL Netfind\r\n");
	prints(sid, "<OPTION>Excite\r\n");
	prints(sid, "<OPTION SELECTED>Google\r\n");
	prints(sid, "<OPTION>Hotbot\r\n");
	prints(sid, "<OPTION>Lycos\r\n");
	prints(sid, "<OPTION>MSN\r\n");
	prints(sid, "<OPTION>Yahoo!\r\n");
	prints(sid, "</SELECT></TD>\r\n");
	prints(sid, "<TD><INPUT TYPE=TEXT NAME='searchText' SIZE=20></TD>\r\n");
	prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Search'></TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.searchForm.searchText.focus();\r\n// -->\r\n</SCRIPT>\r\n");
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
DllExport int mod_main(CONN *sid)
{
	if ((strncmp(sid->dat->in_RequestURI, "/search/sqlrun", 14)==0)&&(getgetenv(sid, "FORMAT")!=NULL)) {
		searchsqlrun(sid);
		return 0;
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
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
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_searches",		// mod_name
		2,			// mod_submenu
		"SEARCHES",		// mod_menuname
		"searches.png",		// mod_menupic
		"/search/",		// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/search/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
