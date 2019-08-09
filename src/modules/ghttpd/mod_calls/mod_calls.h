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
#include "nullgw/ghttpd/mod.h"
#include "mod_calls_lang.h"

/* mod_calls_db.c */
int dbread_call(CONN *sid, short int perm, int index, REC_CALL *call);
int dbwrite_call(CONN *sid, int index, REC_CALL *call);
/* mod_calls_lang.c */
int lang_read(void);