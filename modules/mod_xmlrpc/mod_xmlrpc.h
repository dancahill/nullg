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

#include "i18n/mod_xmlrpc.h"

/* mod_xmlrpc_auth.c */
int xmlrpc_auth_checkpass(CONN *sid, char *username, char *password);
int xmlrpc_auth_login(CONN *sid);
void xmlrpc_auth_logout(CONN *sid);
/* mod_xmlrpc_contacts.c */
void xmlrpc_contactread(CONN *sid, int contactid);
void xmlrpc_contactlist(CONN *sid);
void xmlrpc_contactwrite(CONN *sid);
/* mod_xmlrpc_main.c */
void xmlrpc_addmember(CONN *sid, char *name, char *type, const char *format, ...);
void xmlrpc_fault(CONN *sid, int faultid, char *fault);
