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
#include "ghttpd/mod.h"
#include "mod_calendar.h"

/****************************************************************************
 *	calendarlistmonth()
 *
 *	Purpose	: List calendar events for a given month
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistmonth(CONN *sid)
{
	char *colourkey[]={
		"#0000BF", "#00BF00", "#00BFBF", "#BF0000", "#BF00BF", "#BFBF00", "#707070",
		"#0000FF", "#00FF00", "#00FFFF", "#FF0000", "#FF00FF", "#FFFF00", "#BFBFBF",
		NULL
	};
	int colours=14;
	char *colour;
	typedef struct {
		int day;
		int week;
		int month;
		int year;
 	} daterec;
	daterec today;
	time_t unixdate;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	time_t t, t2;
	char *ptemp1, *ptemp2;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	int now=(time(NULL)+time_tzoffset(sid, time(NULL)))/86400;
	int printdate;
	int i, j;
	int k=0;
	int lines;
	int index=0;
	int sqr;
	int sqr2;
	int status;
	int userid=-1;
	int groupid=-1;
	int tempmonth;
	int tempyear;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(posttime1, 0, sizeof(posttime1));
	if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
		today.month=atoi(ptemp1);
		today.year=atoi(ptemp2);
		today.day=2;
		if (today.year>2037) {
			today.year=2037;
			today.month=12;
			today.day=2;
		}
		snprintf(posttime1, sizeof(posttime1)-1, "%d-%d-%d", today.year, today.month, today.day);
		unixdate=time_sql2unix(posttime1);
		unixdate+=time_tzoffset(sid, unixdate);
		unixdate/=86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else {
		t=time(NULL)+time_tzoffset(sid, time(NULL));
		strftime(posttime1, sizeof(posttime1), "%Y-%m-02", gmtime(&t));
		unixdate=time_sql2unix(posttime1);
		unixdate+=time_tzoffset(sid, unixdate);
		unixdate/=86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
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
	t=unixdate;
	strftime(posttime1, sizeof(posttime1), "%w %d %m %Y", gmtime(&t));
	sscanf(posttime1, "%d %d %d %d", &today.week, &today.day, &today.month, &today.year);
	if ((today.month==2)&&((today.year/4.0f)==(int)(today.year/4))) {
		if (((today.year/100.0f)!=(int)(today.year/100))||((today.year/400.0f)==(int)(today.year/400))) {
			dim[1]++;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, groupid, "/calendar/mlist");
	prints(sid, "<FORM METHOD=GET ACTION=%s/calendar/mlist>", sid->dat->in_ScriptName);
	if (userid>0) prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", userid);
	if (groupid>0) prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupid);
	prints(sid, "<INPUT TYPE=hidden NAME=status VALUE='%d'>\n", status);
	prints(sid, "<TD ALIGN=RIGHT>\n<SELECT NAME=month>\n");
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d", gmtime(&t));
	htselect_month(sid, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=year>\n");
	htselect_year(sid, 2000, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>");
	prints(sid, "</TD\n</FORM>\n</TR></TABLE>\r\n");
	strftime(showtime, sizeof(showtime), "%B, %Y", gmtime(&unixdate));
	tempmonth=today.month-1;
	tempyear=today.year;
	if (tempmonth<1) { tempmonth=12; tempyear--; }
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/mlist?&status=%d&month=%d&year=%d", sid->dat->in_ScriptName, status, tempmonth, tempyear);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	prints(sid, " Calendar Events for %s ", showtime);
	tempmonth=today.month+1;
	tempyear=today.year;
	if (tempmonth>12) { tempmonth=1; tempyear++; }
	prints(sid, "<A HREF=%s/calendar/mlist?&status=%d&month=%d&year=%d", sid->dat->in_ScriptName, status, tempmonth, tempyear);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&gt;&gt;</A>");
	prints(sid, "</B></FONT><BR>\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH WIDTH=200 style='width:14%%; border-style:solid''>Sunday</TH><TH WIDTH=200 style='width:14%%; border-style:solid''>Monday</TH>");
	prints(sid, "<TH WIDTH=200 style='width:14%%; border-style:solid'>Tuesday</TH><TH WIDTH=200 style='width:14%%; border-style:solid'>Wednesday</TH><TH WIDTH=200 style='width:14%%; border-style:solid'>Thursday</TH>");
	prints(sid, "<TH WIDTH=200 style='width:14%%; border-style:solid'>Friday</TH><TH WIDTH=200 style='width:14%%; border-style:solid'>Saturday</TH></TR>\n");
	printdate=today.day-35-today.week;
	unixdate=unixdate/86400-35-today.week;
	while (printdate<-5) { printdate+=7; unixdate+=7; }
	t=unixdate*86400;
	t-=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	j=0;
	for (i=printdate;i<dim[today.month-1]+1;i+=7) { j+=7; }
	t=(unixdate+j)*86400;
	t-=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=dblist_events(sid, posttime1, posttime2))<0) return;
	if ((sqr2=sql_queryf("SELECT userid, groupid, username FROM gw_users WHERE groupid = %d ORDER BY userid ASC", groupid))<0) return;
	for (i=printdate;i<dim[today.month-1]+1;i+=7) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">\n");
		for (j=0;j<7;j++) {
			if (unixdate==now) {
				prints(sid, "<TD BGCOLOR=#E0E0FF");
			} else if ((printdate>0)&&(printdate<=dim[today.month-1])) {
				prints(sid, "<TD CLASS=\"FIELDVAL\"");
			} else {
				prints(sid, "<TD CLASS=\"FIELDNAME\"");
			}
			prints(sid, " VALIGN=TOP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calendar/list?day=%d&status=%d", sid->dat->in_ScriptName, unixdate, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, "'\">");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
			t-=time_tzoffset(sid, t);
			prints(sid, "<NOBR><FONT SIZE=2><A HREF=%s/calendar/list?day=%d&status=%d", sid->dat->in_ScriptName, unixdate, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">%s</A>", showtime);
			if (strlen(htview_holiday(posttime1))>0) {
				prints(sid, "</FONT><FONT SIZE=1 STYLE='font-size:10px'> - %s", htview_holiday(posttime1));
			}
			prints(sid, "</FONT></NOBR><BR>");
			lines=3;
			while (index<sql_numtuples(sqr)) {
				t2=time_sql2unix(sql_getvalue(sqr, index, 1));
				if (t2<t) { index++; continue; }
				if (t2>=t+86400) break;
				if ((status!=2)&&(status!=atoi(sql_getvalue(sqr, index, 3)))) { index++; continue; }
				if ((userid>0)&&(userid!=atoi(sql_getvalue(sqr, index, 4)))) { index++; continue; }
				colour="#000000";
				if (groupid>0) {
					for (k=0;k<sql_numtuples(sqr2);k++) {
						if (atoi(sql_getvalue(sqr, index, 4))!=atoi(sql_getvalue(sqr2, k, 0))) continue;
						if (groupid!=atoi(sql_getvalue(sqr2, k, 1))) continue;
						break;
					}
					if (k==sql_numtuples(sqr2)) { index++; continue; }
					if (k<colours) {
						colour=colourkey[k];
					}
				}
//				t2=time_sql2unix(sql_getvalue(sqr, index, 1))+time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, index, 1)));
				t2+=time_tzoffset(sid, t2);
				prints(sid, "<FONT SIZE=1 STYLE='font-size:10px'><NOBR>%s ", time_unix2lotimetext(sid, t2));
				prints(sid, "<A HREF=%s/calendar/edit?eventid=%d TITLE=\"", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, index, 0)));
				if (groupid>0) {
					prints(sid, "[%s] %s\" STYLE='color:%s'>", sql_getvalue(sqr2, k, 2), sql_getvalue(sqr, index, 5), colour);
				} else {
					prints(sid, "%s\">", sql_getvalue(sqr, index, 5));
				}
				prints(sid, "%-.15s", str2html(sid, sql_getvalue(sqr, index, 5)));
				if (strlen(sql_getvalue(sqr, index, 5))>15) prints(sid, "..");
				prints(sid, "</A>&nbsp;</NOBR></FONT><BR>\n");
				index++;
				if (lines>0) lines--;
			}
			while (lines>0) { prints(sid, "<BR>"); lines--; }
			prints(sid, "</TD>\n");
			printdate++;
			unixdate++;
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "</CENTER>\n");
	if ((groupid>0)&&(sql_numtuples(sqr2)>0)) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH STYLE='border-style:solid'>&nbsp;Colour Key&nbsp;</TH></TR>");
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
		j=0;
		for (i=0;i<sql_numtuples(sqr2);i++) {
			if (j==0) prints(sid, "<TR>");
			colour="#000000";
			if (i<colours) colour=colourkey[i];
			prints(sid, "<TD style='width:20%%'><FONT COLOR=%s><NOBR>%s</NOBR></FONT></TD>\n", colour, sql_getvalue(sqr2, i, 2));
			j++;
			if (j>4) {
				prints(sid, "</TR>\n");
				j=0;
			}
		}
		prints(sid, "</TABLE>");
		prints(sid, "</TD></TR></TABLE>");
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	return;
}
