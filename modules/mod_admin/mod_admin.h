/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

/* mod_admin_config.c */
void adminconfigedit(CONNECTION *sid);
void adminconfigsave(CONNECTION *sid);
/* mod_admin_groups.c */
void admingroupedit(CONNECTION *sid);
void admingrouplist(CONNECTION *sid);
void admingroupsave(CONNECTION *sid);
void admingrouptimeedit(CONNECTION *sid);
void admingrouptimesave(CONNECTION *sid);
/* mod_admin_logs.c */
void adminaccess(CONNECTION *sid);
void adminerror(CONNECTION *sid);
void adminactivitylist(CONNECTION *sid);
void adminactivityview(CONNECTION *sid);
/* mod_admin_syscheck.c */
void admin_syscheck(CONNECTION *sid);
/* mod_admin_users.c */
void adminuseredit(CONNECTION *sid, REC_USER *user);
void adminuserlist(CONNECTION *sid);
void adminusersave(CONNECTION *sid);
void adminusertimeedit(CONNECTION *sid);
void adminusertimesave(CONNECTION *sid);
