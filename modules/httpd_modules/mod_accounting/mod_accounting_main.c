/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_accounting.h"

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ACCOUNTING);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/accounting/journal/list", 24)==0) {
		journal_list(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/accounting/journal", 19)==0) {
		journal_main(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/accounting/accounts/list", 25)==0) {
		account_list(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/accounting/accounts", 20)==0) {
		account_list(sid);
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
		"mod_accounting",	// mod_name
		3,			// mod_submenu
		"ACCOUNTING",		// mod_menuname
		"/accounting/",		// mod_menuuri
		"orders",		// mod_menuperm
		"mod_main",		// fn_name
		"/accounting/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
