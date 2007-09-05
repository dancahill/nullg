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
#include "nullgs/httpd_mod.h"

/* mod_finance_db.c */
int dbread_account(CONN *sid, short int perm, int index, obj_t **qobj);
/* mod_finance_accounts.c */
void account_edit(CONN *sid);
void account_view(CONN *sid);
void account_list(CONN *sid);
/* mod_finance_inventory.c */
void inventoryedit(CONN *sid);
void inventoryview(CONN *sid);
void inventorylist(CONN *sid);
void inventorysave(CONN *sid);
/* mod_finance_invoices.c */
void invoiceedit(CONN *sid);
void invoiceview(CONN *sid);
void invoicelist(CONN *sid);
void invoicesave(CONN *sid);
/* mod_finance_invoiceitems.c */
void invoiceitemedit(CONN *sid);
void invoiceitemlist(CONN *sid, int invoiceid);
void invoiceitemsave(CONN *sid);
/* mod_finance_journal.c */
void journal_list(CONN *sid);
void journal_main(CONN *sid);
/* mod_finance_lang.c */
int lang_read(void);

/* stuff below must go */
typedef struct {
	int recordid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
} REC_HEADER;

#define DB_DBINFO		1
#define DB_BOOKMARKFOLDERS	2
#define DB_BOOKMARKS		3
#define DB_CALLS		4
#define DB_CONTACTS		5
#define DB_EVENTS		6
#define DB_FILES		7
#define DB_FORUMS		8
#define DB_FORUMGROUPS		9
#define DB_FORUMPOSTS		10
#define DB_GROUPS		11
#define DB_MAILACCOUNTS		12
#define DB_MAILHEADERS		13
#define DB_MESSAGES		14
#define DB_NOTES		15
#define DB_INVOICE		16
#define DB_INVOICEITEMS		17
#define DB_INVENTORY		18
#define DB_PROFILE		19
#define DB_QUERIES		20
#define DB_TASKS		21
#define DB_USERS		22
#define DB_ZONES		23
