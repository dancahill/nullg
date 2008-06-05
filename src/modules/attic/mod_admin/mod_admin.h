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

/* mod_admin_config.c */
void adminconfigedit(CONN *sid);
void adminconfigsave(CONN *sid);
/* mod_admin_db.c */
int dbread_domain(CONN *sid, short int perm, int index, obj_t **qobj);
int dbread_group(CONN *sid, short int perm, int index, obj_t **qobj);
int dbread_groupmember(CONN *sid, short int perm, int index, obj_t **qobj);
int dbread_user(CONN *sid, short int perm, int index, obj_t **qobj);
int dbread_zone(CONN *sid, short int perm, int index, obj_t **qobj);
/* mod_admin_domains.c */
void admindomainedit(CONN *sid);
void admindomainlist(CONN *sid);
void admindomainsave(CONN *sid);
/* mod_admin_groups.c */
void admingroupmemberedit(CONN *sid);
void admingroupmembersave(CONN *sid);
void admingroupedit(CONN *sid);
void admingrouplist(CONN *sid);
void admingroupsave(CONN *sid);
/* mod_admin_logs.c */
void adminaccess(CONN *sid);
void adminerror(CONN *sid);
void adminactivitylist(CONN *sid);
void adminactivityview(CONN *sid);
/* mod_admin_syscheck.c */
void admin_syscheck(CONN *sid);
/* mod_admin_time.c */
void admingrouptimeedit(CONN *sid);
void admingrouptimesave(CONN *sid);
void adminusertimeedit(CONN *sid);
void adminusertimesave(CONN *sid);
/* mod_admin_users.c */
void adminuseredit(CONN *sid, obj_t **qobj);
void adminuserlist(CONN *sid);
void adminusersave(CONN *sid);
/* mod_admin_zones.c */
void adminzoneedit(CONN *sid);
void adminzonelist(CONN *sid);
void adminzonesave(CONN *sid);
