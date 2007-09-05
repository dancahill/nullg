/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "mod_finance.h"

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	char *suburi=RequestURI;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_FINANCE);
	prints(sid, "<BR />\r\n");
	if (strncmp(suburi, "/finance/", 9)!=0) return 0;
	suburi+=9;
	if (strncmp(suburi, "accounts/", 9)==0) {
		suburi+=9;
		if (strncmp(suburi, "edit", 4)==0) {
			account_edit(sid);
		} else if (strncmp(suburi, "view", 4)==0) {
			account_view(sid);
		} else if (strncmp(suburi, "list", 4)==0) {
			account_list(sid);
		} else {
			account_list(sid);
		}
	} else if (strncmp(suburi, "inventory/", 10)==0) {
		suburi+=10;
		if (strncmp(suburi, "edit", 4)==0) {
			inventoryedit(sid);
		} else if (strncmp(suburi, "view", 4)==0) {
			inventoryview(sid);
		} else if (strncmp(suburi, "list", 4)==0) {
			inventorylist(sid);
		} else if (strncmp(suburi, "save", 4)==0) {
			inventorysave(sid);
		}
	} else if (strncmp(suburi, "invoices/", 9)==0) {
		suburi+=9;
		if (strncmp(suburi, "itemedit", 8)==0) {
			invoiceitemedit(sid);
		} else if (strncmp(suburi, "itemsave", 8)==0) {
			invoiceitemsave(sid);
		} else if (strncmp(suburi, "edit", 4)==0) {
			invoiceedit(sid);
		} else if (strncmp(suburi, "view", 4)==0) {
			invoiceview(sid);
		} else if (strncmp(suburi, "save", 4)==0) {
			invoicesave(sid);
		} else {
			invoicelist(sid);
		}
	} else if (strncmp(suburi, "journal/", 8)==0) {
		suburi+=8;
		if (strncmp(suburi, "list", 4)==0) {
			journal_list(sid);
		} else {
			journal_main(sid);
		}
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *tobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "finance");
	nes_exec(proc->N, "GWMODULES.finance={submenu=2,name='FINANCES',pic='finance.png',uri='/finance/',perm='finance',fn_name='mod_main',fn_uri='/finance/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
