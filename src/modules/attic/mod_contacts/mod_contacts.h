/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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

/* mod_contacts_db.c */
int dblist_contacts(CONN *sid, obj_t **qobj, char *searchfield, char *searchstring);
int dbread_contact(CONN *sid, short int perm, int index, obj_t **qobj);
int dbwrite_contact(CONN *sid, int index, obj_t **qobj);
/* mod_contacts_lang.c */
int lang_read(void);
/* mod_contacts_vcard.c */
void contacts_vcardexport(CONN *sid);
void contacts_vcardimport(CONN *sid);
