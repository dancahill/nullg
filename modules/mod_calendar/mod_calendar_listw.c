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
#include "mod_substub.h"
#include "mod_calendar.h"

/****************************************************************************
 *	calendarlistweek()
 *
 *	Purpose	: List calendar events for a given day
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistweek(CONN *sid)
{
	MOD_TASKS_LIST mod_tasks_list;
	char *ptemp1, *ptemp2;
	char posttime1[32];
	char posttime2[32];
	char showtime[50];
	struct tm today;
	time_t t, t2;
	time_t unixdate;
	int i, j;
	int k=0;
	int line;
	int sqr;
	int sqr2;
	int status;
	int userid=-1;
	int groupid=-1;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
		unixdate=atoi(ptemp1);
	} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
		today.tm_mon=atoi(ptemp1);
		today.tm_year=atoi(ptemp2);
		today.tm_mday=1;
		if (today.tm_year>2037) {
			today.tm_year=2037;
			today.tm_mon=12;
			today.tm_mday=31;
		}
		snprintf(posttime1, sizeof(posttime1)-1, "%d-%d-%d", today.tm_year, today.tm_mon, today.tm_mday);
		unixdate=time_sql2unix(posttime1)/86400;
	} else {
		unixdate=(time(NULL)+time_tzoffset(sid, time(NULL)))/86400;
	}
	if ((ptemp1=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp1);
	} else {
		status=0;
	}
	if ((ptemp1=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp1);
	} else if ((ptemp1=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp1);
	} else {
		userid=sid->dat->user_uid;
	}
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, groupid, "/calendar/wlist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n");
	strftime(posttime1, sizeof(posttime1), "%w %d %m %Y", gmtime(&unixdate));
	sscanf(posttime1, "%d %d %d %d", &today.tm_wday, &today.tm_mday, &today.tm_mon, &today.tm_year);
	unixdate=unixdate-(today.tm_wday*86400);
	t=unixdate;
	t-=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+604800;
	t-=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/wlist?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)-7, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	prints(sid, " Calendar Events for %s", showtime);
	unixdate+=518400;
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	unixdate-=518400;
	prints(sid, " to %s ", showtime);
	prints(sid, "<A HREF=%s/calendar/wlist?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)+7, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&gt;&gt;</A>");
	prints(sid, "</B></FONT><BR>\n");
	if ((sqr=dblist_events(sid, posttime1, posttime2))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid, username FROM gw_users"))<0) return;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%><TR>\n");
	prints(sid, "<TD VALIGN=TOP WIDTH=100%%>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	for (i=0;i<7;i++) {
		t=unixdate+(i*86400);
		strftime(showtime, sizeof(showtime), "%Y-%m-%d", gmtime(&t));
		t-=time_tzoffset(sid, t);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'>", config->colour_th);
		prints(sid, "<FONT SIZE=2><B><A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, (int)(t/86400));
		if (userid>0) prints(sid, "&userid=%d", userid);
		if (groupid>0) prints(sid, "&groupid=%d", groupid);
		prints(sid, "&status=%d", status);
		prints(sid, " STYLE='color: %s'>%s</A></B></FONT></TH></TR>\n", config->colour_thlink, time_sql2datetext(sid, showtime));
		line=1;
		for (j=0;j<sql_numtuples(sqr);j++) {
			t2=time_sql2unix(sql_getvalue(sqr, j, 1));
			if ((t2<t)||(t2>t+86399)) continue;
			if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, j, 3)))) continue;
			if ((userid>0)&&(userid!=atoi(sql_getvalue(sqr, j, 4)))) continue;
			if (groupid>0) {
				for (k=0;k<sql_numtuples(sqr2);k++) {
					if (atoi(sql_getvalue(sqr, j, 4))!=atoi(sql_getvalue(sqr2, k, 0))) continue;
					if (groupid!=atoi(sql_getvalue(sqr2, k, 1))) continue;
					break;
				}
				if (k==sql_numtuples(sqr2)) continue;
			}
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><FONT SIZE=2>");
			t2=time_sql2unix(sql_getvalue(sqr, j, 1))+time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, j, 1)));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(sqr, j, 0), time_unix2timetext(sid, t2));
			t2=time_sql2unix(sql_getvalue(sqr, j, 2))+time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, j, 2)));
			prints(sid, " - %s</A></FONT></TD><TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>", time_unix2timetext(sid, t2));
			if (groupid>0) prints(sid, "[%s] ", str2html(sid, sql_getvalue(sqr2, k, 2)));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, j, 0));
			prints(sid, "%s</A>&nbsp;", str2html(sid, sql_getvalue(sqr, j, 5)));
			prints(sid, "</FONT></TD></TR>\n");
			if (line>0) line--;
		}
		while (line>0) {
			prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n", config->colour_fieldval);
			line--;
		}
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD><TD VALIGN=TOP>\n");
	calendarmini(sid, unixdate, userid, groupid);
	if ((mod_tasks_list=module_call(sid, "mod_tasks_list"))!=NULL) {
		prints(sid, "<BR>\n");
		mod_tasks_list(sid, userid, groupid);
	}
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	return;
}
