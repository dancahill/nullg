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

#include "i18n/mod_searches.h"

/* mod_searches_contacts.c */
void searchcontacts(CONN *sid);
/* mod_searches_db.c */
int dbread_query(CONN *sid, short int perm, int index, REC_QUERY *query);
int dbwrite_query(CONN *sid, int index, REC_QUERY *query);
/* mod_searches_sql.c */
void searchsqladd(CONN *sid);
void searchsqlrun(CONN *sid);
void searchsqlsave(CONN *sid);
