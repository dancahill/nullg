/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "ghttpd/mod.h"
#include "mod_calendar.h"

void calendarlistyear(CONN *sid)
{
	char *ptemp;
	char posttime1[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int tempyear;
	int i, j;
	int status;
	int userid=-1;
	int groupid=-1;
	time_t unixdate;
	time_t t;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(posttime1, 0, sizeof(posttime1));
	if ((ptemp=getgetenv(sid, "YEAR"))!=NULL) {
		today.tm_year=atoi(ptemp);
		if (today.tm_year<1970) {
			today.tm_year=1970;
		} else if (today.tm_year>2037) {
			today.tm_year=2037;
		}
		snprintf(posttime1, sizeof(posttime1)-1, "%d-01-01", today.tm_year);
		unixdate=time_sql2unix(posttime1)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else {
		t=time(NULL)+time_tzoffset(sid, time(NULL));
		strftime(posttime1, sizeof(posttime1), "%Y-01-01", gmtime(&t));
		unixdate=time_sql2unix(posttime1)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	}
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	strftime(posttime1, sizeof(posttime1), "%w %d %m %Y", gmtime(&unixdate));
	sscanf(posttime1, "%d %d %d %d", &today.tm_wday, &today.tm_mday, &today.tm_mon, &today.tm_year);
	if ((today.tm_year/4.0f)==(int)(today.tm_year/4)) {
		if ((today.tm_year/400.0f)==(int)(today.tm_year/400)) {
			dim[1]++;
		} else if ((today.tm_year/100.0f)!=(int)(today.tm_year/100)) {
			dim[1]++;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, groupid, "/calendar/ylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\n</TR></TABLE>\r\n");
	tempyear=today.tm_year-1;
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/ylist?status=%d&year=%d", sid->dat->in_ScriptName, status, tempyear);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	prints(sid, " Calendar Events for %d ", today.tm_year);
	tempyear=today.tm_year+1;
	prints(sid, "<A HREF=%s/calendar/ylist?status=%d&year=%d", sid->dat->in_ScriptName, status, tempyear);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&gt;&gt;</A>");
	prints(sid, "</B></FONT><BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	for (i=0;i<4;i++) {
		prints(sid, "<TR>\n");
		for (j=0;j<3;j++) {
			prints(sid, "<TD VALIGN=TOP>\n");
			calendarmini2(sid, unixdate, userid, groupid);
			unixdate+=(dim[j*i+i]*86400);
			prints(sid, "</TD>\n");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	return;
}
