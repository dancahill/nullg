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

/* mod_bookmarks_db.c */
int dbread_bookmark(CONN *sid, short int perm, int index, obj_t **qobj);
int dbread_bookmarkfolder(CONN *sid, short int perm, int index, obj_t **qobj);
int dbwrite_bookmark(CONN *sid, int index, obj_t **qobj);
int dbwrite_bookmarkfolder(CONN *sid, int index, obj_t **qobj);
/* mod_bookmarks_export.c */
void bookmarks_export(CONN *sid);
