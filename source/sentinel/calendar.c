/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

char *getholiday(char *date)
{
	char *holidays[13][2]={
		{ "-01-01", "New Year's Day" },
		{ "-02-02", "Groundhog Day" },
		{ "-02-14", "Valentine's Day" },
		{ "-03-17", "St. Patrick's Day" },
		{ "-05-24", "Victoria Day" },
		{ "-07-01", "Canada Day" },
		{ "-07-04", "Independance Day (US)" },
		{ "-10-31", "Halloween" },
		{ "-11-11", "Rememberance Day" },
		{ "-12-25", "Christmas Day" },
		{ "-12-26", "Boxing Day" },
		{ "-12-31", "New Year's Eve" },
		{ "",       "" }
	};
	int i=0;

	while (strlen(holidays[i][0])>0) {
		if (strstr(date, holidays[i][0])!=NULL) return holidays[i][1];
		i++;
	}
	return "";
}

void calendarreminders(int sid)
{
	char posttime[100];
	char posttime1[100];
	struct timeval ttime;
	struct timezone tzone;
	int reminders=0;
	int i;
	int a, b;
	int sqr;

	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	gettimeofday(&ttime, &tzone);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", localtime(&ttime.tv_sec));
	ttime.tv_sec+=86400*7;
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d", localtime(&ttime.tv_sec));
	prints("<CENTER>\n");
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf("SELECT eventid, eventname, eventdate, reminder FROM gw_events where eventdate >= #%s# and eventdate < #%s# and assignedto like '%s' and reminder > 0 ORDER BY eventdate ASC", posttime, posttime1, conn[sid].dat->in_username))<0) return;
	} else {
		if ((sqr=sqlQueryf("SELECT eventid, eventname, eventdate, reminder FROM gw_events where eventdate >= '%s' and eventdate < '%s' and assignedto like '%s' and reminder > 0 ORDER BY eventdate ASC", posttime, posttime1, conn[sid].dat->in_username))<0) return;
	}
	gettimeofday(&ttime, &tzone);
	prints("<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Sentinel Event Reminder</FONT></B>\n</CENTER><BR>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
//		a=sqldatetime2unix(sqlGetvalue(sqr, i, 2))-ttime.tv_sec+tzone.tz_minuteswest*60;
		a=sqldatetime2unix(sqlGetvalue(sqr, i, 2))-sqldatetime2unix(posttime);
		b=a-atoi(sqlGetvalue(sqr, i, 3))*60;
		if ((a>0)&&(b<0)) {
			reminders++;
			if (reminders==1) {
				prints("<BGSOUND SRC=/sentinel/sounds/reminder.wav>\n");
				prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%%>\n");
				prints("<TR BGCOLOR=%s><TH>&nbsp;</TH><TH ALIGN=left WIDTH=100%%>Event Name</TH><TH>Time</TH><TH>Date</TH></TR>\n", COLOR_TRIM);
			}
			prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
			prints("<TD NOWRAP VALIGN=top><A HREF=%s/calendarreminderreset?eventid=%s>reset</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints("<TD><A HREF=%s/calendarview?eventid=%s TARGET=main>%s</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 1));
			prints("<TD NOWRAP VALIGN=top>%s&nbsp;</TD>", sqltime2text(sqlGetvalue(sqr, i, 2)));
			prints("<TD NOWRAP VALIGN=top>%s&nbsp;</TD>", sqldate2text(sqlGetvalue(sqr, i, 2)));
			prints("</TR>\n");
		}
	}
	if (reminders==0) {
		prints("You have no outstanding reminders.\n</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\nwindow.close('_top');\n// -->\n</SCRIPT>\n");
		sqlFreeconnect(sqr);
		return;
	}
	prints("</TABLE></CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void reminderstatus(int sid)
{
	char timebuffer[100];
	struct timeval ttime;
	struct timezone tzone;
	int eventid;
	int newreminder;
	int oldreminder;
	int sqr;

	if (auth(sid, "calendar")<1) {
		return;
	}
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if ((sqr=sqlQueryf("SELECT reminder FROM gw_events where eventid = %d and (ispublic = 1 or assignedto like '%s')", eventid, conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return;
	}
	oldreminder=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	gettimeofday(&ttime, &tzone);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&ttime.tv_sec));
	switch(oldreminder) {
		case 15:
			newreminder=0;
			break;
		case 30:
			newreminder=15;
			break;
		case 60:
			newreminder=30;
			break;
		case 120: // 2 hours
			newreminder=60;
			break;
		case 180: // 3 hours
			newreminder=120;
			break;
		case 360: // 6 hours
			newreminder=180;
			break;
		case 720: // 12 hours
			newreminder=360;
			break;
		case 1440: // 1 day
			newreminder=720;
			break;
		case 2880: // 2 days
			newreminder=1440;
			break;
		case 4320: // 3 days
			newreminder=2880;
			break;
		case 5760: // 4 days
			newreminder=4320;
			break;
		case 7200: // 5 days
			newreminder=5760;
			break;
		case 8640: // 6 days
			newreminder=7200;
			break;
		case 10080: // 7 days
			newreminder=8640;
			break;
		default:
			newreminder=0;
			break;
	}
	sqlUpdatef("UPDATE gw_events SET mdatetime = '%s', reminder = %d WHERE eventid = %d", timebuffer, newreminder, eventid);
	return;
}

char *reminder(int selected)
{
	int sid=getsid();
	char *buffer=conn[sid].dat->envbuf;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	if (selected==0) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "No reminder");
	} else if ((selected>0)&&(selected<60)) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "%d minutes before", selected);
	} else if ((selected>59)&&(selected<120)) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "%d hour before", selected/60);
	} else if ((selected>119)&&(selected<1440)) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "%d hours before", selected/60);
	} else if ((selected>1439)&&(selected<2880)) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "%d day before", selected/1440);
	} else if (selected>2879) {
		snprintf(buffer, sizeof(conn[sid].dat->envbuf)-1, "%d days before", selected/1440);
	}
	return buffer;
}

/****************************************************************************
 *	calendaredit()
 *
 *	Purpose	: Display edit form for calendar event information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendaredit(int sid)
{
	typedef struct {
		int hours;
		int minutes;
		int day;
		int month;
		int year;
 	} daterec;
	daterec today;
	char timebuffer[100];
	char eventdate[16];
	char eventtime[16];
	char repeatdate[16];
	char eventname[50];
	time_t t;
	int i;
	int eventid;
	int sqr;

	if (auth(sid, "calendar")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strncmp(conn[sid].dat->in_RequestURI, "/calendareditnew", 16)==0) {
		eventid=0;
		if (getgetenv(sid, "TIME")!=NULL) {
			t=atoi(getgetenv(sid, "TIME"));
		} else {
			t=(time((time_t)0)/86400)*86400;
		}
		strftime(timebuffer, sizeof(timebuffer), "%H %M %d %m %Y", gmtime(&t));
		sscanf(timebuffer, "%d %d %d %d %d", &today.hours, &today.minutes, &today.day, &today.month, &today.year);
		snprintf(eventtime, sizeof(eventtime)-1, "%02d:%02d:00", today.hours, today.minutes);
		snprintf(eventdate, sizeof(eventdate)-1, "%d-%d-%d", today.year, today.month, today.day);
		snprintf(repeatdate, sizeof(repeatdate)-1, "%d-%d-%d", today.year, today.month, today.day);
		snprintf(eventname, sizeof(eventname)-1, "New Event");
		if ((sqr=sqlQuery("SELECT * FROM gw_events WHERE eventid = 1"))<0) return;
		strcpy(conn[sid].dat->tuple[0].value, "0");
		strncpy(conn[sid].dat->tuple[0].name, sqlGetfieldname(sqr, 0), sizeof(conn[sid].dat->tuple[0].name)-1);
		for (i=1;i<sqlNumfields(sqr);i++) {
			conn[sid].dat->tuple[i].value[0]='\0';
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		for (i=0;i<sqlNumfields(sqr);i++) {
			if (strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
			if ((strcmp(conn[sid].dat->tuple[i].name, "contactid")==0)&&(getgetenv(sid, "CONTACTID")!=NULL)) {
				snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", getgetenv(sid, "CONTACTID"));
			}
			if (strcmp(conn[sid].dat->tuple[i].name, "eventlength")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
			if (strcmp(conn[sid].dat->tuple[i].name, "priority")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "2");
			if (strcmp(conn[sid].dat->tuple[i].name, "reminder")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
			if (strcmp(conn[sid].dat->tuple[i].name, "repeat")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
			if (strcmp(conn[sid].dat->tuple[i].name, "repeatuntil")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01");
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
	} else {
		if (getgetenv(sid, "EVENTID")==NULL) return;
		eventid=atoi(getgetenv(sid, "EVENTID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_events where eventid = %d and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", eventid, conn[sid].dat->in_username))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", eventid);
			sqlFreeconnect(sqr);
			return;
		}
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		sqlFreeconnect(sqr);
		snprintf(repeatdate, 11, "%s", field(CALFIELDS, "repeatuntil"));
		snprintf(eventdate, 11, "%s", field(CALFIELDS, "eventdate"));
		snprintf(eventtime, sizeof(eventtime)-1, "%s", field(CALFIELDS, "eventdate")+11);
		snprintf(eventname, sizeof(eventname)-1, "%s", field(CALFIELDS, "eventname"));
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/calendarsave NAME=eventedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=eventid VALUE='%s'>\n", field(CALFIELDS, "eventid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(CALFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(CALFIELDS, "mdatetime"));
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s>\n", COLOR_TRIM);
	if (eventid>0)
		prints("<TH COLSPAN=2>Calendar Event %d</TH></TR>\n", eventid);
	else
		prints("<TH COLSPAN=2>New Calendar Event</TH></TR>\n");
	prints("<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Event Name</B></TD>", COLOR_EDITFORM);
	prints("<TD><INPUT TYPE=TEXT NAME=eventname value='%s' SIZE=25></TD></TR>\n", eventname);
	if (auth(sid, "calendar")>2) {
		prints("<TR BGCOLOR=%s><TD><B>Assign to</B></TD>", COLOR_EDITFORM);
		prints("<TD><SELECT NAME=assignedto>\n");
		assigntoselect(sid, field(CALFIELDS, "assignedto"));
		prints("</SELECT></TD></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD><B>Date</B></TD><TD>", COLOR_EDITFORM);
	prints("<SELECT NAME=eventdate2>\n");
	monthselect(sid, eventdate);
	prints("</SELECT>");
	prints("<SELECT NAME=eventdate1>\n");
	dayselect(sid, eventdate);
	prints("</SELECT>");
	prints("<SELECT NAME=eventdate3>\n");
	yearselect(sid, 2000, eventdate);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Time</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=eventtime>\n");
	timeselect(sid, eventtime);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Duration</B></TD>", COLOR_EDITFORM);
	prints("<TD><INPUT TYPE=TEXT NAME=eventlength value='%s' SIZE=25></TD></TR>\n", field(CALFIELDS, "eventlength"));
	prints("<TR BGCOLOR=%s><TD><B>Contact</B></TD><TD><SELECT NAME=contactid>\n", COLOR_EDITFORM);
	contactselect(sid, atoi(field(CALFIELDS, "contactid")));
	prints("</SELECT></TD></TR>\n");
	prints("</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD><B>Status</B></TD>", COLOR_EDITFORM);
	prints("<TD><INPUT TYPE=TEXT NAME=eventstatus value='%s' SIZE=25></TD></TR>\n", field(CALFIELDS, "eventstatus"));
	prints("<TR BGCOLOR=%s><TD><B>Privacy</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=ispublic>\n");
	if (atoi(field(CALFIELDS, "ispublic"))==1) {
		prints("<OPTION VALUE=0>Private\n<OPTION VALUE=1 SELECTED>Public\n");
	} else {
		prints("<OPTION VALUE=0 SELECTED>Private\n<OPTION VALUE=1>Public\n");
	}
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Priority</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=priority>\n");
	priorityselect(sid, field(CALFIELDS, "priority"));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Reminder</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=reminder>\n");
	reminderselect(sid, atoi(field(CALFIELDS, "reminder")));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Repeat Event</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=repeat>\n");
	repeatselect(sid, field(CALFIELDS, "repeat"));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Repeat Until</B></TD><TD>", COLOR_EDITFORM);
	prints("<SELECT NAME=repeatdate2>\n");
	monthselect(sid, repeatdate);
	prints("</SELECT>");
	prints("<SELECT NAME=repeatdate1>\n");
	dayselect(sid, repeatdate);
	prints("</SELECT>");
	prints("<SELECT NAME=repeatdate3>\n");
	yearselect(sid, 2000, repeatdate);
	prints("</SELECT></TD></TR>\n");
	prints("</TABLE></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><B>Notes</B></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=virtual NAME=eventnotes ROWS=4 COLS=40>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, field(CALFIELDS, "eventnotes"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth(sid, "calendar")>2)&&(eventid!=0)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.eventedit.eventname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

/****************************************************************************
 *	calendarview()
 *
 *	Purpose	: Display calendar event information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarview(int sid)
{
	char eventtime[16];
	char eventdate[16];
	char eventname[50];
	int i;
	int eventid;
	int sqr;

	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(eventdate, 0, sizeof(eventdate));
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if ((sqr=sqlQueryf("SELECT * FROM gw_events where eventid = %d and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", eventid, conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %d</CENTER>\n", eventid);
		sqlFreeconnect(sqr);
		return;
	}
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
	}
	sqlFreeconnect(sqr);
	snprintf(eventdate, 11, "%s", field(CALFIELDS, "eventdate"));
	snprintf(eventtime, sizeof(eventtime)-1, "%s", field(CALFIELDS, "eventdate")+11);
	snprintf(eventname, sizeof(eventname)-1, "%s", field(CALFIELDS, "eventname"));
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP>%s", COLOR_TRIM, eventname);
	prints(" [<A HREF=%s/calendaredit?eventid=%d>edit</A>]", conn[sid].dat->in_ScriptName, eventid);
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Assigned By</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CALFIELDS, "assignedby"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Status     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(CALFIELDS, "eventstatus"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Assigned To</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(CALFIELDS, "assignedto"));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Privacy    </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
	if (atoi(field(CALFIELDS, "ispublic"))==1) prints("Public");
	else prints("Private");
	prints("</TD></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Date       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, sqldate2text(eventdate));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Priority   </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
	if (atoi(field(CALFIELDS, "priority"))==0) prints("Lowest");
	else if (atoi(field(CALFIELDS, "priority"))==1) prints("Low");
	else if (atoi(field(CALFIELDS, "priority"))==2) prints("Normal");
	else if (atoi(field(CALFIELDS, "priority"))==3) prints("High");
	else if (atoi(field(CALFIELDS, "priority"))==4) prints("Highest");
	prints("&nbsp;</TD></TR>\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Time        </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, sqltime2text(eventtime));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Reminder    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, reminder(atoi(field(CALFIELDS, "reminder"))));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Duration    </B></TD><TD BGCOLOR=%s NOWRAP>%d Minutes&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, atoi(field(CALFIELDS, "eventlength")));
	prints("    <TD BGCOLOR=%s NOWRAP><B>Repeat Event</B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
	if (atoi(field(CALFIELDS, "repeat"))==0) prints("Does not repeat");
	else if (atoi(field(CALFIELDS, "repeat"))==1) prints("Daily");
	else if (atoi(field(CALFIELDS, "repeat"))==2) prints("Weekly");
	else if (atoi(field(CALFIELDS, "repeat"))==3) prints("Monthly");
	else if (atoi(field(CALFIELDS, "repeat"))==4) prints("Yearly");
	prints("&nbsp;</TD></TR>\n");
	if ((sqr=sqlQueryf("SELECT contactid, lastname, firstname FROM gw_contacts WHERE contactid = %d", atoi(field(CALFIELDS, "contactid"))))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints("<TR><TD BGCOLOR=%s NOWRAP><B>Contact</B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FTEXT);
		prints("<A HREF=%s/contactview?contactid=%d>", conn[sid].dat->in_ScriptName, atoi(sqlGetvalue(sqr, 0, 0)));
		prints("%s", sqlGetvalue(sqr, 0, 1));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(", ");
		prints("%s</A></TD>\n", sqlGetvalue(sqr, 0, 2));
	}
	if (atoi(field(CALFIELDS, "repeat"))>0) {
		if (sqlNumtuples(sqr)<1) {
			prints("<TR><TD BGCOLOR=%s NOWRAP>&nbsp;</TD><TD BGCOLOR=%s NOWRAP>&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT);
		}
		prints("<TD BGCOLOR=%s NOWRAP><B>Repeat Until</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, sqldate2text(field(CALFIELDS, "repeatuntil")));
	} else if (sqlNumtuples(sqr)>0) {
		prints("<TD BGCOLOR=%s NOWRAP>&nbsp;</TD><TD BGCOLOR=%s NOWRAP>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT);
	}
	sqlFreeconnect(sqr);
	prints("<TR><TD BGCOLOR=%s COLSPAN=4><B>Notes</B></TD></TR>\n", COLOR_FNAME);
	prints("<TR><TD BGCOLOR=%s COLSPAN=4><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FTEXT, field(CALFIELDS, "eventnotes"));
	prints("</TABLE>\n<BR>\n</CENTER>\n");
}

/****************************************************************************
 *	calendarlistday()
 *
 *	Purpose	: List calendar events for a given day
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistday(int sid)
{
	typedef struct {
		int day;
		int week;
		int month;
		int year;
 	} daterec;
	daterec today;
	struct timeval ttime;
	struct timezone tzone;
	time_t t;
	time_t tdate;
	char posttime[100];
	char posttime1[100];
	char showtime[100];
	char tmphour[10];
	int dim[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int printdate;
	int unixdate;
	int i, j;
	int sqr;

	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "DAY")!=NULL) {
		unixdate=atoi(getgetenv(sid, "DAY"));
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.year=atoi(getgetenv(sid, "YEAR"));
		today.month=atoi(getgetenv(sid, "MONTH"));
		today.day=1;
		if (today.year>2037) {
			today.year=2037;
			today.month=12;
			today.day=31;
		}
		snprintf(posttime, sizeof(posttime)-1, "%d-%d-%d", today.year, today.month, today.day);
		unixdate=sqldate2unix(posttime)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else {
		gettimeofday(&ttime, &tzone);
		unixdate=(int)((ttime.tv_sec-tzone.tz_minuteswest*60)/86400)*86400;
	}
	t=unixdate;
	tdate=unixdate;
	strftime(posttime, sizeof(posttime), "%w %d %m %Y", gmtime(&t));
	sscanf(posttime, "%d %d %d %d", &today.week, &today.day, &today.month, &today.year);
	if (today.month==2) {
		if ((today.year/4.0f)==(int)(today.year/4)) {
			if ((today.year/400.0f)==(int)(today.year/400)) {
				dim[1]++;
			} else if ((today.year/100.0f)!=(int)(today.year/100)) {
				dim[1]++;
			}
		}
	}
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&t));
	strftime(posttime, sizeof(posttime), "%Y-%m-%d", gmtime(&t));
	t+=86400;
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d", gmtime(&t));
	prints("<CENTER>\n");
	strftime(showtime, sizeof(showtime), "%A %B %d, %Y", gmtime(&tdate));
	prints("<FONT COLOR=#000000><B>Calendar Events for %s", showtime);
	if (strlen(getholiday(posttime))>0) prints(" - %s", getholiday(posttime));
	prints("</B></FONT><BR>\n");
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1><TR><TD VALIGN=TOP>\n");
	prints("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<FORM METHOD=GET ACTION=%s/calendarlist>", conn[sid].dat->in_ScriptName);
	prints("<TR BGCOLOR=%s><TH COLSPAN=7 NOWRAP><FONT SIZE=2>\n", COLOR_TRIM);
	prints("<SELECT NAME=month>\n");
	monthselect(sid, posttime);
	prints("</SELECT>\n");
	prints("<SELECT NAME=year>\n");
	yearselect(sid, 2000, posttime);
	prints("</SELECT>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>");
	prints("</FONT></TH></TR>\n");
	prints("</FORM>\n");
	prints("<TR BGCOLOR=%s>", COLOR_TRIM);
	prints("<TH><FONT SIZE=2>Sun</FONT></TH><TH><FONT SIZE=2>Mon</FONT></TH>");
	prints("<TH><FONT SIZE=2>Tue</FONT></TH><TH><FONT SIZE=2>Wed</FONT></TH>");
	prints("<TH><FONT SIZE=2>Thu</FONT></TH><TH><FONT SIZE=2>Fri</FONT></TH>");
	prints("<TH><FONT SIZE=2>Sat</FONT></TH></TR>\n");
	printdate=today.day-35-today.week;
	unixdate=unixdate/86400-35-today.week;
	while (printdate<-5) { printdate+=7; unixdate+=7; }
	for (;;) {
		prints("<TR BGCOLOR=%s>\n", COLOR_FTEXT);
		for (i=0;i<7;i++) {
			prints("<TD ALIGN=RIGHT><FONT SIZE=2>");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			prints("<A HREF=%s/calendarlist?day=%d style='", conn[sid].dat->in_ScriptName, unixdate);
			if ((printdate>0)&&(printdate<dim[today.month-1]+1)) {
				if (printdate==today.day)
					prints("COLOR: #FF0000;'>%s</A>", showtime);
				else
					prints("COLOR: #0000FF;'>%s</A>", showtime);
			} else {
				prints("COLOR: #B0B0B0;'>%s</A>", showtime);
			}
			prints("</FONT></TD>\n");
			printdate++;
			unixdate++;
		}
		prints("</TR>\n");
		if (printdate>dim[today.month-1]) break;
	}
	prints("</TABLE>\n");
	tasklist(sid);
	prints("</TD><TD VALIGN=TOP>\n");
	prints("<FONT COLOR=#000000>\n");
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf("SELECT eventid, eventdate, eventname, ispublic FROM gw_events where eventdate >= #%s# and eventdate < #%s# and(ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", posttime, posttime1, conn[sid].dat->in_username))<0) return;
	} else {
		if ((sqr=sqlQueryf("SELECT eventid, eventdate, eventname, ispublic FROM gw_events where eventdate like '%s %%' and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", posttime, conn[sid].dat->in_username))<0) return;
	}
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	strftime(showtime, sizeof(showtime), "%A %B %d, %Y", gmtime(&tdate));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Event Time</B></TD><TD NOWRAP WIDTH=300><B>Event Name</B></TD></TR>\n", COLOR_TRIM);
	if (getgetenv(sid, "DAY")!=NULL) {
		unixdate=atoi(getgetenv(sid, "DAY"))*86400;
	} else {
		gettimeofday(&ttime, &tzone);
		unixdate=(int)((ttime.tv_sec-tzone.tz_minuteswest*60)/86400)*86400;
	}
	for (i=0;i<24;i++) {
		snprintf(tmphour, sizeof(tmphour)-1, "%02d:00:00", i);
		prints("<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP>", COLOR_FTEXT);
		prints("<FONT SIZE=2><A HREF=%s/calendareditnew?", conn[sid].dat->in_ScriptName);
		prints("time=%d>%s</A></FONT></TD>", unixdate+(i*3600), sqltime2text(tmphour));
		prints("<TD NOWRAP><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n");
		if (sqlNumtuples(sqr)>0) {
			for (j=0;j<sqlNumtuples(sqr);j++) {
				if (strncmp(sqlGetvalue(sqr, j, 1)+11, tmphour, 2)!=0) continue;
				prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
				prints("<TD ALIGN=RIGHT NOWRAP><FONT SIZE=2><A HREF=%s/calendarview?", conn[sid].dat->in_ScriptName);
				prints("eventid=%s style='TEXT-DECORATION: none'>%s</A></FONT></TD>", sqlGetvalue(sqr, j, 0), sqltime2text(sqlGetvalue(sqr, j, 1)+11));
				prints("<TD NOWRAP><FONT SIZE=2>%s&nbsp;", sqlGetvalue(sqr, j, 2));
				if (!atoi(sqlGetvalue(sqr, j, 3))) {
					prints("<FONT COLOR=#FF0000><I>Private</I></FONT>");
				}
				prints("</FONT></TD>");
				prints("</TR>\n");
			}
		}
	}
	prints("</TABLE>\n");
	prints("</TD></TR></TABLE>\n");
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

/****************************************************************************
 *	calendarlistmonth()
 *
 *	Purpose	: List calendar events for a given month
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistmonth(int sid)
{
	typedef struct {
		int day;
		int week;
		int month;
		int year;
 	} daterec;

	daterec today;
	int unixdate;
	int dim[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	time_t t;
	char posttime[100];
	char posttime1[100];
	char showtime[100];
	int printdate;
	int i, j, k;
	int index=0;
	struct timeval ttime;
	struct timezone tzone;
	int sqr;

	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(posttime, 0, sizeof(posttime));
	if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.year=atoi(getgetenv(sid, "YEAR"));
		today.month=atoi(getgetenv(sid, "MONTH"));
		today.day=1;
		if (today.year>2037) {
			today.year=2037;
			today.month=12;
			today.day=2;
		}
		snprintf(posttime, sizeof(posttime)-1, "%d-%d-%d", today.year, today.month, today.day);
		unixdate=sqldate2unix(posttime)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else {
		gettimeofday(&ttime, &tzone);
		t=ttime.tv_sec-tzone.tz_minuteswest*60;
		strftime(posttime, sizeof(posttime), "%Y-%m-02", gmtime(&t));
		unixdate=sqldate2unix(posttime)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	}
	t=unixdate;
	strftime(posttime, sizeof(posttime), "%w %d %m %Y", gmtime(&t));
	sscanf(posttime, "%d %d %d %d", &today.week, &today.day, &today.month, &today.year);
	if ((today.month==2)&&((today.year/4.0f)==(int)(today.year/4))) {
		if (((today.year/100.0f)!=(int)(today.year/100))||((today.year/400.0f)==(int)(today.year/400))) {
			dim[1]++;
		}
	}
	strftime(posttime, sizeof(posttime), "%Y-%m-%d", gmtime(&t));
	prints("<CENTER>\n");
	prints("<FORM METHOD=GET ACTION=%s/calendarmlist>", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=7>\n", COLOR_TRIM);
	prints("<SELECT NAME=month>\n");
	monthselect(sid, posttime);
	prints("</SELECT>\n");
	prints("<SELECT NAME=year>\n");
	yearselect(sid, 2000, posttime);
	prints("</SELECT>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>");
	prints("</TH></TR>\n");
	prints("<TR BGCOLOR=%s><TH>Sunday</TH><TH>Monday</TH>", COLOR_TRIM);
	prints("<TH>Tuesday</TH><TH>Wednesday</TH><TH>Thursday</TH>");
	prints("<TH>Friday</TH><TH>Saturday</TH></TR>\n");
	printdate=today.day-35-today.week;
	unixdate=unixdate/86400-35-today.week;
	while (printdate<-5) { printdate+=7; unixdate+=7; }
	t=unixdate*86400;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d", gmtime(&t));
	j=0;
	for (i=printdate;i<dim[today.month-1]+1;i+=7) { j+=7; }
	t=(unixdate+j)*86400;
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d", gmtime(&t));
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf("SELECT eventid, eventdate, eventname, ispublic FROM gw_events where eventdate >= #%s# and eventdate < #%s# and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", posttime, posttime1, conn[sid].dat->in_username))<0) return;
	} else {
		if ((sqr=sqlQueryf("SELECT eventid, eventdate, eventname, ispublic FROM gw_events where eventdate >= '%s' and eventdate < '%s' and (ispublic = 1 or assignedto like '%s') ORDER BY eventdate ASC", posttime, posttime1, conn[sid].dat->in_username))<0) return;
	}
	for (i=printdate;i<dim[today.month-1]+1;i+=7) {
		prints("<TR BGCOLOR=%s>\n", COLOR_FTEXT);
		for (j=0;j<7;j++) {
			if ((printdate>0)&&(printdate<=dim[today.month-1])) {
				prints("<TD BGCOLOR=%s VALIGN=TOP>", COLOR_FTEXT);
			} else {
				prints("<TD BGCOLOR=#D0D0D0 VALIGN=TOP>");
			}
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			strftime(posttime, sizeof(posttime), "%Y-%m-%d", gmtime(&t));
			prints("<FONT SIZE=2><A HREF=%s/calendarlist?day=%d style='COLOR: #0000FF'>%s</A>", conn[sid].dat->in_ScriptName, unixdate, showtime);
			if (strlen(getholiday(posttime))>0) {
				prints(" - %s", getholiday(posttime));
			}
			prints("</FONT><BR>");
			k=2;
			while ((index<sqlNumtuples(sqr))&&(strncasecmp(sqlGetvalue(sqr, index, 1), posttime, 10)==0)) {
				prints("<FONT SIZE=1>%s ", sqltime2text(sqlGetvalue(sqr, index, 1)+11));
				prints("<A HREF=%s/calendarview?eventid=%s>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, index, 0));
				prints("%s&nbsp;</A>", sqlGetvalue(sqr, index, 2));
				if (!atoi(sqlGetvalue(sqr, index, 3))) {
					prints("<FONT COLOR=#FF0000><I>P</I></FONT>");
				}
				prints("</FONT><BR>\n");
				index++;
				if (k>0) k--;
			}
			while (k>0) { prints("<BR>"); k--; }
			prints("</TD>\n");
			printdate++;
			unixdate++;
		}
		prints("</TR>\n");
	}
	sqlFreeconnect(sqr);
	prints("</TABLE>\n");
	prints("</FORM>");
	prints("<BR></CENTER>\n");
	return;
}

/****************************************************************************
 *	calendarsave()
 *
 *	Purpose	: Save calendar event information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarsave(int sid)
{
	time_t t;
	char query[8192];
	int authlevel;
	int eventid;
	int i;
	int sqr;

	if (auth(sid, "calendar")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	authlevel=auth(sid, "calendar");
	if ((sqr=sqlQuery("SELECT * FROM gw_events WHERE eventid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (strcmp(conn[sid].dat->tuple[i].name, "eventdate")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "EVENTDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "EVENTDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "EVENTDATE1"));
			strcat(conn[sid].dat->tuple[i].value, " ");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "EVENTTIME"));
		} else if (strcmp(conn[sid].dat->tuple[i].name, "repeatuntil")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "REPEATDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "REPEATDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "REPEATDATE1"));
		} else if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		}
	}
	eventid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(eventid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if (strcmp(conn[sid].dat->tuple[i].name, "assignedby")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0)&&(authlevel<3)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "eventlength")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "priority")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "2");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "reminder")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "repeat")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "repeatuntil")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01");
		}
	}
	sqlFreeconnect(sqr);
	if (eventid==0) {
		if ((sqr=sqlQuery("SELECT max(eventid) FROM gw_events"))<0) return;
		eventid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (eventid<1) eventid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", eventid);
		strcpy(query, "INSERT INTO gw_events (");
		for (i=0;i<CALFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<CALFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<CALFIELDS;i++) {
			strcatf(query, "'%s'", conn[sid].dat->tuple[i].value);
			if (i<CALFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Calendar event %d added successfully</CENTER><BR>\n", eventid);
		logaccess(1, "%s - %s added calendar event %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, eventid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendarview?eventid=%d\">\n", conn[sid].dat->in_ScriptName, eventid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if (auth(sid, "calendar")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_events WHERE eventid = %d", eventid)<0) return;
		prints("<CENTER>Calendar event %d deleted successfully</CENTER><BR>\n", eventid);
		logaccess(1, "%s - %s deleted calendar event %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, eventid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendarlist\">\n", conn[sid].dat->in_ScriptName);
	} else {
		if (auth(sid, "calendar")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_events SET ");
		for (i=2;i<CALFIELDS;i++) {
			strcatf(query, "%s = '%s'", conn[sid].dat->tuple[i].name, conn[sid].dat->tuple[i].value);
			if (i<CALFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE eventid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Event %d modified successfully</CENTER><BR>\n", eventid);
		logaccess(1, "%s - %s modified calendar event %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, eventid);
		prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendarview?eventid=%d\">\n", conn[sid].dat->in_ScriptName, eventid);
	}
	return;
}

/****************************************************************************
 *	calendarmain()
 *
 *	Purpose	: Execute appropriate calendar function based on URI
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarmain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(conn[sid].dat->in_RequestURI, "/calendarreminders", 18)==0) {
		printheader(sid, "Sentinel Event Reminder");
		calendarreminders(sid);
		printfooter(sid);
		return;
	} else if (strncmp(conn[sid].dat->in_RequestURI, "/calendarreminderreset", 22)==0) {
		printheader(sid, "Sentinel Event Reminder");
		reminderstatus(sid);
		calendarreminders(sid);
		printfooter(sid);
		return;
	}
	printheader(sid, "Sentinel Groupware Calendar");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch03-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<A CLASS='TBAR' HREF=%s/calendarlist>DAY VIEW</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/calendarmlist>MONTH VIEW</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/calendareditnew>NEW EVENT</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/calendaredit", 13)==0)
		calendaredit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/calendarview", 13)==0)
		calendarview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/calendarlist", 13)==0)
		calendarlistday(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/calendarmlist", 14)==0)
		calendarlistmonth(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/calendarsave", 13)==0)
		calendarsave(sid);
	printfooter(sid);
	return;
}
