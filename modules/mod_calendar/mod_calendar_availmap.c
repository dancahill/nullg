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

void calendar_availmap(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *ptemp1, *ptemp2;
	char gavailability[170];
	char uavailability[170];
	char posttime1[32];
	char posttime2[32];
	char showtime[50];
	struct tm today;
	time_t t, t1, t2, t3;
	time_t unixdate;
	int i, j;
	int k=0;
	int x;
	int busyevent;
	int sqr;
	int sqr2;
	int status;
	int userid=-1;
	int groupid=-1;
	int usergroupid;

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
	htselect_eventfilter(sid, userid, groupid, "/calendar/availmap");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n");

	if (userid<1) return; // temp breakage hack

	strftime(posttime1, sizeof(posttime1), "%w %d %m %Y", gmtime(&unixdate));
	sscanf(posttime1, "%d %d %d %d", &today.tm_wday, &today.tm_mday, &today.tm_mon, &today.tm_year);
	unixdate=unixdate-(today.tm_wday*86400);
	t=unixdate;
	t-=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+604800;
	t-=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/availmap?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)-7, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	prints(sid, " Availability for %s", showtime);
	unixdate+=518400;
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	unixdate-=518400;
	prints(sid, " to %s ", showtime);
	prints(sid, "<A HREF=%s/calendar/availmap?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)+7, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&gt;&gt;</A>");
	prints(sid, "</B></FONT><BR>\n");
	if ((sqr=sql_queryf(sid, "SELECT groupid, availability FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\n", userid);
		sql_freeresult(sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	usergroupid=atoi(sql_getvalue(sqr, 0, 0));
	strncpy(uavailability, sql_getvalue(sqr, 0, 1), sizeof(uavailability)-1);
	sql_freeresult(sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if ((sqr=sql_queryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", usergroupid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\n", usergroupid);
		sql_freeresult(sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sql_getvalue(sqr, 0, 0), sizeof(gavailability)-1);
	sql_freeresult(sqr);
	if (strlen(gavailability)!=168) {
		for (i=0;i<168;i++) {
			gavailability[i]='0';
		}
	}
	for (i=0;i<168;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='X';
		}
	}
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish, status, assignedto, eventname FROM gw_events where busy > 0 and eventstart >= '%s' and eventstart < '%s' ORDER BY eventfinish ASC", posttime1, posttime2))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish, status, assignedto, eventname FROM gw_events where busy > 0 and eventstart >= '%s' and eventstart < '%s' and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventfinish ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid, username FROM gw_users"))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2 STYLE='border-style:solid'>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER STYLE='border-style:solid'><B>%02d</B></TD>\n", j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=#F0F0F0>\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s STYLE='border-style:solid'><B>&nbsp;%s&nbsp;</B></TD>\n", config->colour_fieldname, dow[i]);
		for (j=0;j<24;j++) {
			prints(sid, "<TD STYLE='border-style:solid'>");
			for (k=0;k<4;k++) {
				t=unixdate+(i*86400)+(j*3600)+(k*900);
				t1=t;
				t-=time_tzoffset(sid, t);
				busyevent=0;
				for (x=0;x<sql_numtuples(sqr);x++) {
					t2=time_sql2unix(sql_getvalue(sqr, x, 1));
					t3=time_sql2unix(sql_getvalue(sqr, x, 2));
					if (t2>t) continue;
					if (t3<=t) continue;
					if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, x, 3)))) continue;
					if ((userid>0)&&(userid!=atoi(sql_getvalue(sqr, x, 4)))) continue;
//					if (groupid>0) {
//						for (k=0;k<sql_numtuples(sqr2);k++) {
//							if (atoi(sql_getvalue(sqr, x, 4))!=atoi(sql_getvalue(sqr2, k, 0))) continue;
//							if (groupid!=atoi(sql_getvalue(sqr2, k, 1))) continue;
//							break;
//						}
//						if (k==sql_numtuples(sqr2)) continue;
//					}
					busyevent=atoi(sql_getvalue(sqr, x, 0));
					break;
				}
				if (busyevent) {
					prints(sid, "<SPAN STYLE=background-color:#FF4040><A HREF=%s/calendar/view?eventid=%d>&nbsp;&nbsp;</A></SPAN>", sid->dat->in_ScriptName, busyevent);
				} else if (uavailability[i*24+j]=='X') {
					prints(sid, "<SPAN STYLE=background-color:#909090>&nbsp;&nbsp;</SPAN>");
				} else if (uavailability[i*24+j]=='0') {
					prints(sid, "<SPAN STYLE=background-color:#A0A0A0>&nbsp;&nbsp;</SPAN>");
				} else {
					prints(sid, "<SPAN STYLE=background-color:#7070FF><A HREF=%s/calendar/editnew?userid=%d&time=%d>&nbsp;&nbsp;</A></SPAN>", sid->dat->in_ScriptName, userid, t1);
				}
			}
			prints(sid, "</TD>\n");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	return;
}
