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
/* mod_calendar_auto.c */
int db_availcheck(CONNECTION *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish);
int db_autoschedule(CONNECTION *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish);
int db_autoassign(CONNECTION *sid, u_avail *uavail, int groupid, int zoneid, int record, int busy, time_t eventstart, time_t eventfinish);
/* mod_calendar_mini.c */
void calendarmini(CONNECTION *sid, time_t unixdate, int userid, int groupid);
void calendarmini2(CONNECTION *sid, time_t unixdate, int userid, int groupid);
