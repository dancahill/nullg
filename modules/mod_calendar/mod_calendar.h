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

#include "i18n/mod_calendar.h"

/* mod_calendar_assign.c */
int db_availcheck(CONN *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish);
int db_autoschedule(CONN *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish);
int db_autoassign(CONN *sid, u_avail *uavail, int groupid, int zoneid, int record, int busy, time_t eventstart, time_t eventfinish);
/* mod_calendar_availmap.c */
void calendar_availmap(CONN *sid);
/* mod_calendar_db.c */
int dblist_events(CONN *sid, char *startdate, char *enddate);
int dbread_event(CONN *sid, short int perm, int index, REC_EVENT *event);
int dbwrite_event(CONN *sid, int index, REC_EVENT *event);

/* mod_calendar_list?.c */
void calendarlistday(CONN *sid);
void calendarlistweek(CONN *sid);
void calendarlistmonth(CONN *sid);
void calendarlistyear(CONN *sid);

/* mod_calendar_main.c */
void htselect_eventfilter(CONN *sid, int userid, int groupid, char *baseuri);
/* mod_calendar_mini.c */
void calendarmini(CONN *sid, time_t unixdate, int userid, int groupid);
void calendarmini2(CONN *sid, time_t unixdate, int userid, int groupid);
