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

#include "mod_orders_lang.h"

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
#define DB_ORDERS		16
#define DB_ORDERITEMS		17
#define DB_PRODUCTS		18
#define DB_PROFILE		19
#define DB_QUERIES		20
#define DB_TASKS		21
#define DB_USERS		22
#define DB_ZONES		23

/* mod_orders_db.c */
int db_read(CONN *sid, short int perm, short int table, int index, void *record);
/* mod_orders_items.c */
void orderitemedit(CONN *sid);
void orderitemlist(CONN *sid, int orderid);
void orderitemsave(CONN *sid);
/* mod_orders_lang.c */
int lang_read(void);
/* mod_orders_products.c */
void productedit(CONN *sid);
void productview(CONN *sid);
void productlist(CONN *sid);
void productsave(CONN *sid);
