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
	prints(sid, "	if (searchString!='') {\n");
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
			if (strcmp(sql_getname(sqr, i), "obj_did")==0) continue;
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
	prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Search'></TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.searchForm.searchText.focus();\n// -->\n</SCRIPT>\n");
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
	if ((strncmp(sid->dat->in_RequestURI, "/search/sqlrun", 14)==0)&&(getgetenv(sid, "FORMAT")!=NULL)) {
		searchsqlrun(sid);
		return;
	}
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
	MODULE_MENU newmod = {
		"mod_searches",		// mod_name
		2,			// mod_submenu
		"SEARCHES",		// mod_menuname
		"/search/",		// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/search/",		// fn_uri
		mod_main		// fn_ptr
	};

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
