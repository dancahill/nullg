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
#include "http_mod.h"
#include "mod_calendar.h"

/****************************************************************************
 *	calendarlistday()
 *
 *	Purpose	: List calendar events for a given day
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistday(CONN *sid)
{
	HTMOD_TASKS_LIST mod_tasks_list;
	char *ptemp1, *ptemp2;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	time_t t, t2;
	time_t unixdate;
	int index=0;
	int i;
	int j=0;
	int sqr;
	int sqr2;
	int userid=-1;
	int groupid=-1;
	int status;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
		unixdate=atoi(ptemp1);
	} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
		today.tm_mday=1;
		today.tm_mon=atoi(ptemp1);
		today.tm_year=atoi(ptemp2);
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
	htselect_eventfilter(sid, userid, groupid, "/calendar/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	strftime(showtime, sizeof(showtime), "%A %B %d, %Y", gmtime(&unixdate));
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/list?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)-1, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	prints(sid, " Calendar Events for %s ", showtime);
	if (strlen(htview_holiday(posttime1))>0) prints(sid, "- %s ", htview_holiday(posttime1));
	prints(sid, "<A HREF=%s/calendar/list?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)+1, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&gt;&gt;</A>");
	prints(sid, "</B></FONT>\n");
	prints(sid, "<BR>\n<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%><TR>\n");
	prints(sid, "<TD VALIGN=TOP WIDTH=100%%>\n");
	t=unixdate;
	t-=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+86400;
	t-=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=dblist_events(sid, posttime1, posttime2))<0) return;
	if ((sqr2=sql_queryf("SELECT userid, groupid, username FROM gw_users"))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Time&nbsp;</TH><TH ALIGN=LEFT NOWRAP WIDTH=100%% STYLE='border-style:solid'>&nbsp;Event Name&nbsp;</TH></TR>\n");
	for (i=0;i<24;i++) {
		snprintf(showtime, sizeof(showtime)-1, "%02d:00:00", i);
		if ((i>=sid->dat->user_daystart)&&(i<sid->dat->user_daystart+sid->dat->user_daylength)) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>");
			prints(sid, "<FONT SIZE=2><A HREF=%s/calendar/editnew?", sid->dat->in_ScriptName);
			prints(sid, "time=%d>%s</A></FONT></TD>", unixdate+(i*3600), time_sql2timetext(sid, showtime));
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n");
		}
		while (index<sql_numtuples(sqr)) {
			t=unixdate+i*3600;
			t-=time_tzoffset(sid, t);
			t2=time_sql2unix(sql_getvalue(sqr, index, 1));
			if (t2<t) { index++; continue; }
			if (t2>=t+3600) break;
			if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, index, 3)))) { index++; continue; }
			if ((userid>0)&&(userid!=atoi(sql_getvalue(sqr, index, 4)))) { index++; continue; }
			if (groupid>0) {
				for (j=0;j<sql_numtuples(sqr2);j++) {
					if (atoi(sql_getvalue(sqr, index, 4))!=atoi(sql_getvalue(sqr2, j, 0))) continue;
					if (groupid!=atoi(sql_getvalue(sqr2, j, 1))) continue;
					break;
				}
				if (j==sql_numtuples(sqr2)) { index++; continue; }
			}
			if ((i>=sid->dat->user_daystart)&&(i<=sid->dat->user_daystart+sid->dat->user_daylength)) {
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
			} else {
				prints(sid, "<TR CLASS=\"FIELDNAME\">");
			}
			t2+=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, index, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><FONT SIZE=2>");
			prints(sid, "<A HREF=%s/calendar/edit?eventid=%s>%s</A>", sid->dat->in_ScriptName, sql_getvalue(sqr, index, 0), time_unix2timetext(sid, t2));
			prints(sid, "</FONT></TD><TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>");
			if (groupid>0) prints(sid, "[%s] ", str2html(sid, sql_getvalue(sqr2, j, 2)));
			prints(sid, "<A HREF=%s/calendar/edit?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sql_getvalue(sqr, index, 0), str2html(sid, sql_getvalue(sqr, index, 5)));
			prints(sid, "</FONT></TD></TR>\n");
			index++;
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
