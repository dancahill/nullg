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

void calendarmini(CONN *sid, time_t unixdate, int userid, int groupid)
{
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int b;
	int i, j, k;
	int printdate;
	time_t t, t2;
	int sqr;
	int sqr2;
	int status;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
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
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&unixdate));
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=200 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=GET ACTION=%s/calendar/list>", sid->dat->in_ScriptName);
	if (userid>0) prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", userid);
	if (groupid>0) prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupid);
	prints(sid, "<INPUT TYPE=hidden NAME=status VALUE='%d'>\n", status);
	prints(sid, "<TR BGCOLOR=%s><TH NOWRAP STYLE='font-weight:normal; border-style:solid''><NOBR><FONT SIZE=2>", config->colour_th);
	prints(sid, "<SELECT NAME=month>\n");
	htselect_month(sid, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=year>\n");
	htselect_year(sid, 2000, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>");
	prints(sid, "</FONT></NOBR></TH></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "<TR><TD WIDTH=100%% STYLE='border-style:solid'>\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s>", config->colour_th);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>S</FONT></TH><TH><FONT COLOR=%s SIZE=2>M</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>T</FONT></TH><TH><FONT COLOR=%s SIZE=2>W</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>T</FONT></TH><TH><FONT COLOR=%s SIZE=2>F</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>S</FONT></TH></TR>\n", config->colour_thtext);
	printdate=today.tm_mday-today.tm_wday;
	unixdate=unixdate/86400-today.tm_wday;
	while (printdate>1) { printdate-=7; unixdate-=7; }
	t=unixdate*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=(unixdate+42)*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=dblist_events(sid, posttime1, posttime2))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid FROM gw_users"))<0) return;
	for (;;) {
		prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldval);
		for (i=0;i<7;i++) {
			prints(sid, "<TD ALIGN=CENTER><FONT SIZE=2>");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			t-=time_tzoffset(sid, t);
			for (j=0,b=0;j<sql_numtuples(sqr);j++) {
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
				b=1;
			}
			t=atoi(showtime);
			if (b) prints(sid, "<B>");
			prints(sid, "<A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, unixdate);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, "&status=%d", status);
			if (printdate==today.tm_mday) {
				prints(sid, " style='COLOR: #000000;'");
			} else if ((printdate>0)&&(printdate<dim[today.tm_mon-1]+1)) {
				prints(sid, " style='COLOR: #0000FF;'");
			} else {
				prints(sid, " style='COLOR: #B0B0B0;'");
			}
			prints(sid, ">%d</A>", t);
			if (b) prints(sid, "</B>");
			prints(sid, "</FONT></TD>\n");
			printdate++;
			unixdate++;
		}
		prints(sid, "</TR>\n");
		if (printdate>dim[today.tm_mon-1]) break;
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\n</TD></TR></TABLE>\n");
	return;
}

void calendarmini2(CONN *sid, time_t unixdate, int userid, int groupid)
{
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int b;
	int i, j;
	int index=0;
	int printdate;
	time_t t, t2;
	int sqr;
	int sqr2;
	int status;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
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
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	strftime(posttime1, sizeof(posttime1), "%B", gmtime(&unixdate));
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=200 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH NOWRAP STYLE='border-style:solid'><FONT SIZE=2>", config->colour_th);
	prints(sid, "<A HREF=%s/calendar/mlist?month=%d&year=%d", sid->dat->in_ScriptName, today.tm_mon, today.tm_year);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, "&status=%d style='color: %s'>%s</A></FONT></TH></TR>\n", status, config->colour_thtext, posttime1);
	prints(sid, "<TR><TD WIDTH=100%% STYLE='border-style:solid'>\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s>", config->colour_th);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>S</FONT></TH><TH><FONT COLOR=%s SIZE=2>M</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>T</FONT></TH><TH><FONT COLOR=%s SIZE=2>W</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>T</FONT></TH><TH><FONT COLOR=%s SIZE=2>F</FONT></TH>", config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH><FONT COLOR=%s SIZE=2>S</FONT></TH></TR>\n", config->colour_thtext);
	printdate=today.tm_mday-today.tm_wday;
	unixdate=unixdate/86400-today.tm_wday;
	while (printdate>1) { printdate-=7; unixdate-=7; }
	t=unixdate*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=(unixdate+42)*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=dblist_events(sid, posttime1, posttime2))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid FROM gw_users"))<0) return;
	for (;;) {
		prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldval);
		for (i=0;i<7;i++) {
			prints(sid, "<TD ALIGN=CENTER><FONT SIZE=2>");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			t-=time_tzoffset(sid, t);
			b=0;
			while (index<sql_numtuples(sqr)) {
				t2=time_sql2unix(sql_getvalue(sqr, index, 1));
				if (t2<t) { index++; continue; }
				if (t2>t+86399) break;
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
				b=1;
				index++;
			}
			t=atoi(showtime);
			if ((printdate>0)&&(printdate<dim[today.tm_mon-1]+1)) {
				if (b) prints(sid, "<B>");
				prints(sid, "<A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, unixdate);
				if (userid>0) prints(sid, "&userid=%d", userid);
				if (groupid>0) prints(sid, "&groupid=%d", groupid);
				prints(sid, "&status=%d style='COLOR: #0000FF;'>%d</A>", status, t);
				if (b) prints(sid, "</B>");
			}
			prints(sid, "</FONT></TD>\n");
			printdate++;
			unixdate++;
		}
		prints(sid, "</TR>\n");
		if (printdate>dim[today.tm_mon-1]) break;
	}
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\n</TD></TR></TABLE>\n");
	return;
}
