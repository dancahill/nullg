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
#include "main.h"

void calendarreminders(CONNECTION *sid)
{
	char posttime[32];
	int a, b;
	int i;
	int reminders=0;
	int sqr;
	time_t eventdate;
	time_t t;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventname, eventstart, reminder FROM gw_events where eventstart < #%s# and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventname, eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
	}
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>%s</FONT></B>\n", CAL_EVENT_TITLE);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		a=time_sql2unix(sqlGetvalue(sqr, i, 2))-time(NULL);
		b=a-atoi(sqlGetvalue(sqr, i, 3))*60;
		if (b<0) {
			reminders++;
			if (reminders==1) {
				prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%%>\n");
				prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT>&nbsp;</TH><TH ALIGN=LEFT WIDTH=100%%>&nbsp;%s&nbsp;</TH><TH ALIGN=LEFT>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT>&nbsp;Time&nbsp;</TH></TR>\n", COLOR_TH, CAL_EVENT_NAME);
			}
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD NOWRAP VALIGN=top><A HREF=%s/calendar/reminderreset?eventid=%s>%s</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), CAL_EVENT_RESET);
			prints(sid, "<TD><A HREF=%s/calendar/view?eventid=%s TARGET=gwmain>%s</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), str2html(sid, sqlGetvalue(sqr, i, 1)));
			eventdate=time_sql2unix(sqlGetvalue(sqr, i, 2));
			eventdate+=time_tzoffset(sid, eventdate);
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2datetext(sid, eventdate));
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2timetext(sid, eventdate));
			prints(sid, "</TR>\n");
		}
	}
	sqlFreeconnect(sqr);
	if (reminders==0) {
		prints(sid, "%s\n</CENTER>\n", CAL_EVENT_NONE);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>window.close('_top');</SCRIPT>\n");
	} else {
		prints(sid, "</TABLE></CENTER>\n");
	}
	return;
}

void reminderstatus(CONNECTION *sid)
{
	char timebuffer[100];
	int eventid;
	int reminder;
	int sqr;
	time_t eventstart;
	time_t now;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		return;
	}
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if ((sqr=sqlQueryf(sid, "SELECT reminder, eventstart FROM gw_events where eventid = %d and assignedto = %d", eventid, sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return;
	}
	reminder=atoi(sqlGetvalue(sqr, 0, 0));
	eventstart=time_sql2unix(sqlGetvalue(sqr, 0, 1));
	now=time(NULL);
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, now));
	sqlFreeconnect(sqr);
	eventstart=(eventstart-now)/60;
	if (eventstart<0) {
		eventstart=0;
		reminder=0;
	}
	switch (reminder) {
	case 10080: // 7 days
		reminder=8640;
		if (reminder<eventstart) break;
	case 8640: // 6 days
		reminder=7200;
		if (reminder<eventstart) break;
	case 7200: // 5 days
		reminder=5760;
		if (reminder<eventstart) break;
	case 5760: // 4 days
		reminder=4320;
		if (reminder<eventstart) break;
	case 4320: // 3 days
		reminder=2880;
		if (reminder<eventstart) break;
	case 2880: // 2 days
		reminder=1440;
		if (reminder<eventstart) break;
	case 1440: // 1 day
		reminder=720;
		if (reminder<eventstart) break;
	case 720: // 12 hours
		reminder=360;
		if (reminder<eventstart) break;
	case 360: // 6 hours
		reminder=180;
		if (reminder<eventstart) break;
	case 180: // 3 hours
		reminder=120;
		if (reminder<eventstart) break;
	case 120: // 2 hours
		reminder=60;
		if (reminder<eventstart) break;
	case 60:
		reminder=30;
		if (reminder<eventstart) break;
	case 30:
		reminder=15;
		if (reminder<eventstart) break;
	case 15:
		reminder=0;
		if (reminder<eventstart) break;
	default:
		reminder=0;
		break;
	}
	sqlUpdatef(sid, "UPDATE gw_events SET obj_mtime = '%s', reminder = %d WHERE eventid = %d", timebuffer, reminder, eventid);
	return;
}

/****************************************************************************
 *	calendaredit()
 *
 *	Purpose	: Display edit form for calendar event information
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendaredit(CONNECTION *sid, REC_EVENT *event)
{
	REC_EVENT eventrec;
	char startdate[30];
	char finishdate[30];
	char *ptemp;
	int autogroup;
	int eventid;
	int duration;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (event==NULL) {
		prints(sid, "<BR>");
		if (strncmp(sid->dat->in_RequestURI, "/calendar/editnew", 17)==0) {
			eventid=0;
			if (db_read(sid, 2, DB_EVENTS, 0, &eventrec)!=0) {
				prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
				return;
			}
			if ((ptemp=getgetenv(sid, "CONTACTID"))!=NULL) eventrec.contactid=atoi(ptemp);
			if ((ptemp=getgetenv(sid, "TIME"))!=NULL) {
				eventrec.eventstart=atoi(ptemp)-time_tzoffset(sid, eventrec.eventstart);
				eventrec.eventfinish=atoi(ptemp)-time_tzoffset(sid, eventrec.eventfinish)+900;
			}
			if (eventrec.eventfinish<eventrec.eventstart) eventrec.eventfinish=eventrec.eventstart;
		} else {
			if (getgetenv(sid, "EVENTID")==NULL) return;
			eventid=atoi(getgetenv(sid, "EVENTID"));
			if (db_read(sid, 2, DB_EVENTS, eventid, &eventrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", eventid);
				return;
			}
		}
		event=&eventrec;
	}
	if ((ptemp=getpostenv(sid, "AUTOGROUP"))!=NULL) {
		autogroup=atoi(ptemp);
	} else {
		autogroup=sid->dat->user_gid;
	}
	event->eventstart+=time_tzoffset(sid, event->eventstart);
	event->eventfinish+=time_tzoffset(sid, event->eventfinish);
	strftime(startdate, sizeof(startdate)-1, "%Y-%m-%d", gmtime(&event->eventstart));
	strftime(finishdate, sizeof(finishdate)-1, "%Y-%m-%d", gmtime(&event->eventfinish));
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "function CloseUpdate() {\n");
	prints(sid, "	if (document.eventedit.status.value==0) {\n");
	prints(sid, "		document.eventedit.closingstatus.value=0;\n");
	prints(sid, "		document.eventedit.closingstatus.disabled=true;\n");
	prints(sid, "	} else {\n");
	prints(sid, "		document.eventedit.closingstatus.disabled=false;\n");
	prints(sid, "	}\n");
	prints(sid, "	return;\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/calendar/save NAME=eventedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=eventid VALUE='%d'>\n", event->eventid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (event->eventid>0) {
		prints(sid, "<A HREF=%s/calendar/view?eventid=%d STYLE='color: %s'>Calendar Event %d</FONT></TH></TR>\n", sid->dat->in_ScriptName, event->eventid, COLOR_THTEXT, event->eventid);
	} else {
		prints(sid, "New Calendar Event</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Event Name&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=eventname value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", str2html(sid, event->eventname));
	if (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN) {
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:182px'>\n", COLOR_EDITFORM);
		htselect_user(sid, event->assignedto);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Event Type&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=eventtype style='width:182px'>\n", COLOR_EDITFORM);
	htselect_eventtype(sid, event->eventtype);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Contact&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>\n", COLOR_EDITFORM);
	htselect_contact(sid, event->contactid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Priority&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=priority style='width:182px'>\n", COLOR_EDITFORM);
	htselect_priority(sid, event->priority);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Reminder&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=reminder style='width:182px'>\n");
	htselect_reminder(sid, event->reminder);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=eventstart2>\n", COLOR_EDITFORM);
	htselect_month(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart1>\n");
	htselect_day(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart3>\n");
	htselect_year(sid, 2000, startdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Start Time&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=starttime style='width:182px'>\n");
	htselect_time(sid, event->eventstart);
	prints(sid, "</SELECT></TD></TR>\n");
	duration=event->eventfinish-event->eventstart;
	if (duration<0) duration=0;
	duration/=60;
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Duration&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=duration1 style='width:91px'>\n");
	htselect_qhours(sid, duration/60);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=duration2 style='width:91px'>\n");
	htselect_qminutes(sid, duration%60);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Availability&nbsp;</B></TD><TD ALIGN=RIGHT>", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=busy style='width:182px'>\n");
	prints(sid, "<OPTION VALUE=0%s>Available\n", event->busy==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE=1%s>Busy\n", event->busy!=0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:182px' onchange=CloseUpdate();>\n", COLOR_EDITFORM);
	htselect_eventstatus(sid, event->status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Closing Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=closingstatus style='width:182px'>\n", COLOR_EDITFORM);
	htselect_eventclosingstatus(sid, event->closingstatus);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=4 COLS=60 style='width:100%%'>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, event->details));
	if ((event->obj_uid==sid->dat->user_uid)||(auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, event->obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, event->obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", event->obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", event->obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", event->obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", event->obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", event->obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", event->obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Autoschedule'>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Autoassign'>");
	prints(sid, "<SELECT NAME=autogroup>\n");
	htselect_group(sid, autogroup);
	prints(sid, "</SELECT><BR>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_CALENDAR)&A_DELETE)&&(event->eventid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.eventedit.eventname.focus();\n");
	prints(sid, "CloseUpdate();\n// -->\n</SCRIPT>\n");
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
void calendarview(CONNECTION *sid)
{
	REC_EVENT event;
	int eventid;
	int i;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if (db_read(sid, 1, DB_EVENTS, eventid, &event)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", eventid);
		return;
	}
	event.eventstart+=time_tzoffset(sid, event.eventstart);
	event.eventfinish+=time_tzoffset(sid, event.eventfinish);
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>%s", COLOR_TH, COLOR_THTEXT, str2html(sid, event.eventname));
	if (auth_priv(sid, AUTH_CALENDAR)&A_MODIFY) {
		prints(sid, " [<A HREF=%s/calendar/edit?eventid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, event.eventid, COLOR_THLINK);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned By </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users"))<0) return;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==event.assignedby) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Date        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100><NOBR>%s&nbsp;</NOBR></TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2datetext(sid, event.eventstart));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Assigned To </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 0))==event.assignedto) {
			prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	sqlFreeconnect(sqr);
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Start Time  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2timetext(sid, event.eventstart));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Event Type  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, htview_eventtype(sid, event.eventtype));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Finish Time </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, time_unix2timetext(sid, event.eventfinish));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Contact     </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if ((sqr=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", event.contactid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, 0, 0)));
		prints(sid, "%s", str2html(sid, sqlGetvalue(sqr, 0, 1)));
		if (strlen(sqlGetvalue(sqr, 0, 1))&&strlen(sqlGetvalue(sqr, 0, 2))) prints(sid, ", ");
		prints(sid, "%s</A>", str2html(sid, sqlGetvalue(sqr, 0, 2)));
	}
	sqlFreeconnect(sqr);
	prints(sid, "&nbsp;</TD>\n", sqlGetvalue(sqr, 0, 2));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Availability</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, event.busy>0?"Busy":"Available");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Priority    </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if (event.priority==0) prints(sid, "Lowest");
	else if (event.priority==1) prints(sid, "Low");
	else if (event.priority==2) prints(sid, "Normal");
	else if (event.priority==3) prints(sid, "High");
	else if (event.priority==4) prints(sid, "Highest");
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Status      </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_eventstatus(sid, event.status));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Reminder    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, htview_reminder(sid, event.reminder));
	if (event.status) {
		prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Closing Status</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, htview_eventclosingstatus(sid, event.closingstatus));
	} else {
		prints(sid, "    <TD BGCOLOR=%s>&nbsp;</TD><TD BGCOLOR=%s>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL);
	}
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=4><B>Details</B></TD></TR>\n", COLOR_FNAME);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=4><PRE>%s&nbsp;</PRE></TD></TR>\n", COLOR_FVAL, str2html(sid, event.details));
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Notes", COLOR_TH, COLOR_THTEXT);
	prints(sid, " [<A HREF=%s/notes/editnew?table=events&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, event.eventid, COLOR_THLINK);
	prints(sid, "</FONT></TH></TR>\n");
	notessublist(sid, "events", event.eventid, 4);
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void calendarmini(CONNECTION *sid, time_t unixdate, int userid)
{
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int b;
	int i, j;
	int printdate;
	time_t t, t2;
	int sqr;
	int status;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
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
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=200>\n");
	prints(sid, "<FORM METHOD=GET ACTION=%s/calendar/list>", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", userid);
	prints(sid, "<INPUT TYPE=hidden NAME=status VALUE='%d'>\n", status);
	prints(sid, "<TR BGCOLOR=%s><TH NOWRAP><FONT SIZE=2>\n", COLOR_TH);
	prints(sid, "<SELECT NAME=month>\n");
	htselect_month(sid, posttime1);
	prints(sid, "</SELECT>\n");
	prints(sid, "<SELECT NAME=year>\n");
	htselect_year(sid, 2000, posttime1);
	prints(sid, "</SELECT>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>");
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "<TR><TD WIDTH=100%%>\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s>", COLOR_FNAME);
	prints(sid, "<TH><FONT SIZE=2>S</FONT></TH><TH><FONT SIZE=2>M</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>T</FONT></TH><TH><FONT SIZE=2>W</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>T</FONT></TH><TH><FONT SIZE=2>F</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>S</FONT></TH></TR>\n");
	printdate=today.tm_mday-today.tm_wday;
	unixdate=unixdate/86400-today.tm_wday;
	while (printdate>1) { printdate-=7; unixdate-=7; }
	t=unixdate*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=(unixdate+42)*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status==2) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= #%s# and eventstart < #%s# and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, userid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= '%s' and eventstart < '%s' and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, userid))<0) return;
		}
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= #%s# and eventstart < #%s# and status = %d and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, status, userid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= '%s' and eventstart < '%s' and status = %d and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, status, userid))<0) return;
		}
	}
	for (;;) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		for (i=0;i<7;i++) {
			prints(sid, "<TD ALIGN=CENTER><FONT SIZE=2>");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			t-=time_tzoffset(sid, t);
			for (j=0,b=0;j<sqlNumtuples(sqr);j++) {
				t2=time_sql2unix(sqlGetvalue(sqr, j, 0));
				if ((t2>=t)&&(t2<t+86400)) b=1;
			}
			t=atoi(showtime);
			if (b) prints(sid, "<B>");
			prints(sid, "<A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, unixdate);
			if (userid>0) prints(sid, "&userid=%d", userid);
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
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE>\n</TD></TR></TABLE>\n");
	return;
}

void calendarmini2(CONNECTION *sid, time_t unixdate, int userid)
{
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int b;
	int i, j;
	int printdate;
	time_t t, t2;
	int sqr;
	int status;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
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
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=200>\n");
	prints(sid, "<TR BGCOLOR=%s><TH NOWRAP><FONT SIZE=2>", COLOR_TH);
	prints(sid, "<A HREF=%s/calendar/mlist?month=%d&year=%d", sid->dat->in_ScriptName, today.tm_mon, today.tm_year);
	if (userid>0) prints(sid, "&userid=%d", userid);
	prints(sid, "&status=%d style='color: %s'>%s</A></FONT></TH></TR>\n", status, COLOR_THTEXT, posttime1);
	prints(sid, "<TR><TD WIDTH=100%%>\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s>", COLOR_FNAME);
	prints(sid, "<TH><FONT SIZE=2>S</FONT></TH><TH><FONT SIZE=2>M</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>T</FONT></TH><TH><FONT SIZE=2>W</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>T</FONT></TH><TH><FONT SIZE=2>F</FONT></TH>");
	prints(sid, "<TH><FONT SIZE=2>S</FONT></TH></TR>\n");
	printdate=today.tm_mday-today.tm_wday;
	unixdate=unixdate/86400-today.tm_wday;
	while (printdate>1) { printdate-=7; unixdate-=7; }
	t=unixdate*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=(unixdate+42)*86400;
	t+=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status==2) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= #%s# and eventstart < #%s# and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, userid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= '%s' and eventstart < '%s' and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, userid))<0) return;
		}
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= #%s# and eventstart < #%s# and status = %d and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, status, userid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT distinct eventstart FROM gw_events where eventstart >= '%s' and eventstart < '%s' and status = %d and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, status, userid))<0) return;
		}
	}
	for (;;) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		for (i=0;i<7;i++) {
			prints(sid, "<TD ALIGN=CENTER><FONT SIZE=2>");
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			t-=time_tzoffset(sid, t);
			for (j=0,b=0;j<sqlNumtuples(sqr);j++) {
				t2=time_sql2unix(sqlGetvalue(sqr, j, 0));
				if ((t2>=t)&&(t2<t+86400)) b=1;
			}
			t=atoi(showtime);
			if ((printdate>0)&&(printdate<dim[today.tm_mon-1]+1)) {
				if (b) prints(sid, "<B>");
				prints(sid, "<A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, unixdate);
				if (userid>0) prints(sid, "&userid=%d", userid);
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
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE>\n</TD></TR></TABLE>\n");
	return;
}

/****************************************************************************
 *	calendarlistday()
 *
 *	Purpose	: List calendar events for a given day
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistday(CONNECTION *sid)
{
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	struct tm today;
	time_t t, t2;
	time_t unixdate;
	int index=0;
	int i;
	int sqr;
	int userid=-1;
	int status;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "DAY")!=NULL) {
		unixdate=atoi(getgetenv(sid, "DAY"));
	} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.tm_year=atoi(getgetenv(sid, "YEAR"));
		today.tm_mon=atoi(getgetenv(sid, "MONTH"));
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
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, "/calendar/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	strftime(showtime, sizeof(showtime), "%A %B %d, %Y", gmtime(&unixdate));
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/list?day=%d&userid=%d&status=%d>&lt;&lt;</A> ", sid->dat->in_ScriptName, (unixdate/86400)-1, userid, status);
	prints(sid, "Calendar Events for %s ", showtime);
	if (strlen(htview_holiday(posttime1))>0) prints(sid, "- %s ", htview_holiday(posttime1));
	prints(sid, "<A HREF=%s/calendar/list?day=%d&userid=%d&status=%d>&gt;&gt;</A>", sid->dat->in_ScriptName, (unixdate/86400)+1, userid, status);
	prints(sid, "</B></FONT>\n");
	prints(sid, "<BR>\n<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%><TR>\n");
	prints(sid, "<TD VALIGN=TOP WIDTH=100%%>\n");
	t=unixdate;
	t-=time_tzoffset(sid, t);
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+86400;
	t-=time_tzoffset(sid, t);
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status==2) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	}
	prints(sid, "<TABLE BGCOLOR=#000000 BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Time&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100%%><FONT COLOR=%s>&nbsp;Event Name&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT);
	for (i=0;i<24;i++) {
		snprintf(showtime, sizeof(showtime)-1, "%02d:00:00", i);
		if ((i>=sid->dat->user_daystart)&&(i<sid->dat->user_daystart+sid->dat->user_daylength)) {
			prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP>", COLOR_FVAL);
			prints(sid, "<FONT SIZE=2><A HREF=%s/calendar/editnew?", sid->dat->in_ScriptName);
			prints(sid, "time=%d>%s</A></FONT></TD>", unixdate+(i*3600), time_sql2timetext(sid, showtime));
			prints(sid, "<TD NOWRAP WIDTH=100%%><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n");
		}
		while (index<sqlNumtuples(sqr)) {
			t=unixdate+i*3600;
			t-=time_tzoffset(sid, t);
			t2=time_sql2unix(sqlGetvalue(sqr, index, 1));
			if (t2<t) { index++; continue; }
			if (t2>=t+3600) break;
			if (atoi(sqlGetvalue(sqr, index, 3))!=userid) { index++; continue; }
			if ((i>=sid->dat->user_daystart)&&(i<=sid->dat->user_daystart+sid->dat->user_daylength)) {
				prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			} else {
				prints(sid, "<TR BGCOLOR=%s>", COLOR_FNAME);
			}
			t2+=time_tzoffset(sid, time_sql2unix(sqlGetvalue(sqr, index, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP><FONT SIZE=2>");
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>", sid->dat->in_ScriptName, sqlGetvalue(sqr, index, 0), time_unix2timetext(sid, t2));
			prints(sid, "</FONT></TD><TD NOWRAP WIDTH=100%%><FONT SIZE=2>");
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sqlGetvalue(sqr, index, 0), str2html(sid, sqlGetvalue(sqr, index, 2)));
			prints(sid, "</FONT></TD></TR>\n");
			index++;
		}
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD><TD VALIGN=TOP>\n");
	calendarmini(sid, unixdate, userid);
	prints(sid, "<BR>\n");
	tasklist(sid, userid);
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

/****************************************************************************
 *	calendarlistweek()
 *
 *	Purpose	: List calendar events for a given day
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarlistweek(CONNECTION *sid)
{
	char *ptemp;
	char posttime1[32];
	char posttime2[32];
	char showtime[50];
	struct tm today;
	time_t t, t2;
	time_t unixdate;
	int i, j, k;
	int sqr;
	int status;
	int userid=-1;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "DAY")!=NULL) {
		unixdate=atoi(getgetenv(sid, "DAY"));
	} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.tm_year=atoi(getgetenv(sid, "YEAR"));
		today.tm_mon=atoi(getgetenv(sid, "MONTH"));
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
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, "/calendar/listweek");
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
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/listweek?day=%d&userid=%d&status=%d>&lt;&lt;</A>", sid->dat->in_ScriptName, (unixdate/86400)-7, userid, status);
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	prints(sid, " Calendar Events for %s", showtime);
	unixdate+=518400;
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	unixdate-=518400;
	prints(sid, " to %s ", showtime);
	prints(sid, "<A HREF=%s/calendar/listweek?day=%d&userid=%d&status=%d>&gt;&gt;</A>", sid->dat->in_ScriptName, (unixdate/86400)+7, userid, status);
	prints(sid, "</B></FONT><BR>\n");
	if (status==2) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%><TR>\n");
	prints(sid, "<TD VALIGN=TOP WIDTH=100%%>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	for (i=0;i<7;i++) {
		t=unixdate+(i*86400);
		strftime(showtime, sizeof(showtime), "%Y-%m-%d", gmtime(&t));
		t-=time_tzoffset(sid, t);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP>", COLOR_TH);
		prints(sid, "<FONT SIZE=2><B><A HREF=%s/calendar/list?day=%d", sid->dat->in_ScriptName, (int)(t/86400));
		if (userid>0) prints(sid, "&userid=%d", userid);
		prints(sid, "&status=%d", status);
		prints(sid, " STYLE='color: %s'>%s</A></B></FONT></TH></TR>\n", COLOR_THLINK, time_sql2datetext(sid, showtime));
		k=1;
		for (j=0;j<sqlNumtuples(sqr);j++) {
			t2=time_sql2unix(sqlGetvalue(sqr, j, 1));
			if ((t2<t)||(t2>=t+86400)) continue;
			if (atoi(sqlGetvalue(sqr, j, 4))!=userid) continue;
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP><FONT SIZE=2>");
			t2=time_sql2unix(sqlGetvalue(sqr, j, 1))+time_tzoffset(sid, time_sql2unix(sqlGetvalue(sqr, j, 1)));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s", sid->dat->in_ScriptName, sqlGetvalue(sqr, j, 0), time_unix2timetext(sid, t2));
			t2=time_sql2unix(sqlGetvalue(sqr, j, 2))+time_tzoffset(sid, time_sql2unix(sqlGetvalue(sqr, j, 2)));
			prints(sid, " - %s</A></FONT></TD><TD NOWRAP WIDTH=100%%><FONT SIZE=2>", time_unix2timetext(sid, t2));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sqlGetvalue(sqr, j, 0), str2html(sid, sqlGetvalue(sqr, j, 3)));
			prints(sid, "</FONT></TD></TR>\n");
			if (k>0) k--;
		}
		while (k>0) {
			prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2 NOWRAP WIDTH=100%%><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n", COLOR_FVAL);
			k--;
		}
	}
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD><TD VALIGN=TOP>\n");
	calendarmini(sid, unixdate, userid);
	prints(sid, "<BR>\n");
	tasklist(sid, userid);
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
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
void calendarlistmonth(CONNECTION *sid)
{
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
	char *ptemp;
	char posttime1[100];
	char posttime2[100];
	char showtime[100];
	int now=time(NULL)/86400;
	int printdate;
	int i, j, k;
	int index=0;
	int sqr;
	int status;
	int userid=-1;
	int tempmonth;
	int tempyear;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(posttime1, 0, sizeof(posttime1));
	if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.year=atoi(getgetenv(sid, "YEAR"));
		today.month=atoi(getgetenv(sid, "MONTH"));
		today.day=1;
		if (today.year>2037) {
			today.year=2037;
			today.month=12;
			today.day=2;
		}
		snprintf(posttime1, sizeof(posttime1)-1, "%d-%d-%d", today.year, today.month, today.day);
		unixdate=time_sql2unix(posttime1)/86400;
		if (unixdate<0) unixdate=0;
		if (unixdate>24836) unixdate=24836;
		unixdate*=86400;
	} else {

		t=time(NULL)+time_tzoffset(sid, time(NULL));
		strftime(posttime1, sizeof(posttime1), "%Y-%m-02", gmtime(&t));
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
	htselect_eventfilter(sid, userid, "/calendar/mlist");
	prints(sid, "<FORM METHOD=GET ACTION=%s/calendar/mlist>", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", userid);
	prints(sid, "<INPUT TYPE=hidden NAME=status VALUE='%d'>\n", status);
	prints(sid, "<TD ALIGN=RIGHT>\n<SELECT NAME=month>\n");
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d", gmtime(&t));
	htselect_month(sid, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=year>\n");
	htselect_year(sid, 2000, posttime1);
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>");
	prints(sid, "</TD\n</FORM>\n</TR></TABLE>\r\n");
	strftime(showtime, sizeof(showtime), "%B, %Y", gmtime(&unixdate));
	tempmonth=today.month-1;
	tempyear=today.year;
	if (tempmonth<1) { tempmonth=12; tempyear--; }
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/mlist?userid=%d&status=%d&month=%d&year=%d>&lt;&lt;</A>", sid->dat->in_ScriptName, userid, status, tempmonth, tempyear);
	prints(sid, " Calendar Events for %s ", showtime);
	tempmonth=today.month+1;
	tempyear=today.year;
	if (tempmonth>12) { tempmonth=1; tempyear++; }
	prints(sid, "<A HREF=%s/calendar/mlist?userid=%d&status=%d&month=%d&year=%d>&gt;&gt;</A>", sid->dat->in_ScriptName, userid, status, tempmonth, tempyear);
	prints(sid, "</B></FONT><BR>\n");
	prints(sid, "<CENTER>\n<TABLE BGCOLOR=#000000 BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Sunday</FONT></TH><TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Monday</FONT></TH>", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT);
	prints(sid, "<TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Tuesday</FONT></TH><TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Wednesday</FONT></TH><TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Thursday</FONT></TH>", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
	prints(sid, "<TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Friday</FONT></TH><TH WIDTH=200 style='width:14%%'><FONT COLOR=%s>Saturday</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT);
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
	if (status==2) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= #%s# and eventstart < #%s# and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventname, assignedto FROM gw_events where eventstart >= '%s' and eventstart < '%s' and status = %d and (assignedby = %d or assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart ASC", posttime1, posttime2, status, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	}
	for (i=printdate;i<dim[today.month-1]+1;i+=7) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		for (j=0;j<7;j++) {
			if (unixdate==now) {
				prints(sid, "<TD BGCOLOR=#FFFFFF VALIGN=TOP>");
			} else if ((printdate>0)&&(printdate<=dim[today.month-1])) {
				prints(sid, "<TD BGCOLOR=%s VALIGN=TOP>", COLOR_FVAL);
			} else {
				prints(sid, "<TD BGCOLOR=#D0D0D0 VALIGN=TOP>");
			}
			t=unixdate*86400;
			strftime(showtime, sizeof(showtime), "%d", gmtime(&t));
			strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
			t-=time_tzoffset(sid, t);
			prints(sid, "<NOBR><FONT SIZE=2><A HREF=%s/calendar/list?day=%d&userid=%d&status=%d>%s</A>", sid->dat->in_ScriptName, unixdate, userid, status, showtime);
			if (strlen(htview_holiday(posttime1))>0) {
				prints(sid, "</FONT><FONT SIZE=1> - %s", htview_holiday(posttime1));
			}
			prints(sid, "</FONT></NOBR><BR>");
			k=3;
			while (index<sqlNumtuples(sqr)) {
				t2=time_sql2unix(sqlGetvalue(sqr, index, 1));
				if (t2<t) { index++; continue; }
				if (t2>=t+86400) break;
				if (atoi(sqlGetvalue(sqr, index, 3))!=userid) { index++; continue; }
				t2+=time_tzoffset(sid, time_sql2unix(sqlGetvalue(sqr, index, 1)));
				prints(sid, "<FONT SIZE=1><NOBR>%s ", time_unix2lotimetext(sid, t2));
				prints(sid, "<A HREF=%s/calendar/view?eventid=%s TITLE=\"%s\">", sid->dat->in_ScriptName, sqlGetvalue(sqr, index, 0), sqlGetvalue(sqr, index, 2));
				prints(sid, "%-.15s", str2html(sid, sqlGetvalue(sqr, index, 2)));
				if (strlen(sqlGetvalue(sqr, index, 2))>15) prints(sid, "..");
				prints(sid, "</A>&nbsp;</NOBR></FONT><BR>\n");
				index++;
				if (k>0) k--;
			}
			while (k>0) { prints(sid, "<BR>"); k--; }
			prints(sid, "</TD>\n");
			printdate++;
			unixdate++;
		}
		prints(sid, "</TR>\n");
	}
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void calendarlistyear(CONNECTION *sid)
{
	char *ptemp;
	char posttime1[100];
	struct tm today;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int tempyear;
	int i, j;
	int status;
	int userid=-1;
	time_t unixdate;
	time_t t;

	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(posttime1, 0, sizeof(posttime1));
	if (getgetenv(sid, "YEAR")!=NULL) {
		today.tm_year=atoi(getgetenv(sid, "YEAR"));
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
	htselect_eventfilter(sid, userid, "/calendar/ylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\n</TR></TABLE>\r\n");
	tempyear=today.tm_year-1;
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/ylist?userid=%d&status=%d&year=%d>&lt;&lt;</A>", sid->dat->in_ScriptName, userid, status, tempyear);
	prints(sid, " Calendar Events for %d ", today.tm_year);
	tempyear=today.tm_year+1;
	prints(sid, "<A HREF=%s/calendar/ylist?userid=%d&status=%d&year=%d>&gt;&gt;</A>", sid->dat->in_ScriptName, userid, status, tempyear);
	prints(sid, "</B></FONT><BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	for (i=0;i<4;i++) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		for (j=0;j<3;j++) {
			prints(sid, "<TD VALIGN=TOP>\n");
			calendarmini2(sid, unixdate, userid);
			unixdate+=(dim[j*i+i]*86400);
			prints(sid, "</TD>\n");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
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
void calendarsave(CONNECTION *sid)
{
	REC_EVENT event;
	char tempdate[40];
	char *ptemp;
	int duration;
	int eventid;
	int rc;
	int sqr;
	int zoneid;
	u_avail uavail;

	prints(sid, "<BR>");
	if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "EVENTID"))==NULL) return;
	eventid=atoi(ptemp);
	if (db_read(sid, 2, DB_EVENTS, eventid, &event)!=0) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, AUTH_CALENDAR)&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) event.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) event.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_CALENDAR)&A_ADMIN)||(event.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) event.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) event.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "EVENTNAME"))!=NULL) snprintf(event.eventname, sizeof(event.eventname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ASSIGNEDBY"))!=NULL) event.assignedby=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ASSIGNEDTO"))!=NULL) event.assignedto=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "EVENTTYPE"))!=NULL) event.eventtype=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "CONTACTID"))!=NULL) event.contactid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PRIORITY"))!=NULL) event.priority=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "REMINDER"))!=NULL) event.reminder=atoi(ptemp);
	memset(tempdate, 0, sizeof(tempdate));
	if ((ptemp=getpostenv(sid, "EVENTSTART3"))!=NULL) snprintf(tempdate, sizeof(tempdate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "EVENTSTART2"))!=NULL) strcatf(tempdate, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "EVENTSTART1"))!=NULL) strcatf(tempdate, "%02d ", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "STARTTIME"))!=NULL) strcatf(tempdate, "%s", ptemp);
	if (strlen(tempdate)!=19) memset(tempdate, 0, sizeof(tempdate));
	event.eventstart=time_sql2unix(tempdate);
	memset(tempdate, 0, sizeof(tempdate));
	duration=0;
	if ((ptemp=getpostenv(sid, "DURATION1"))!=NULL) duration+=atoi(ptemp)*3600;
	if ((ptemp=getpostenv(sid, "DURATION2"))!=NULL) duration+=atoi(ptemp)*60;
	event.eventfinish=event.eventstart;
	if (duration>0) event.eventfinish+=duration;
	if ((ptemp=getpostenv(sid, "BUSY"))!=NULL) event.busy=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "STATUS"))!=NULL) event.status=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "CLOSINGSTATUS"))!=NULL) event.closingstatus=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DETAILS"))!=NULL) snprintf(event.details, sizeof(event.details)-1, "%s", ptemp);
	event.eventstart-=time_tzoffset(sid, event.eventstart);
	event.eventfinish-=time_tzoffset(sid, event.eventfinish);
	if (event.eventfinish<event.eventstart) event.eventfinish=event.eventstart;
	if (event.eventfinish==event.eventstart) event.busy=0;
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Autoschedule")==0)) {
		if (event.assignedto<1) {
			prints(sid, "<CENTER><B>This event must be assigned to a user.</B></CENTER>\n");
			calendaredit(sid, &event);
			return;
		}
		if ((rc=db_autoschedule(event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))<0) {
			prints(sid, "<CENTER><B>Could not find an available time slot for this event.</B></CENTER>\n");
		} else {
			event.eventfinish+=rc-event.eventstart;
			event.eventstart+=rc-event.eventstart;
		}
		calendaredit(sid, &event);
		return;
	}
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Autoassign")==0)) {
		if ((ptemp=getpostenv(sid, "AUTOGROUP"))==NULL) return;
		zoneid=0;
		if ((sqr=sqlQueryf(sid, "SELECT geozone FROM gw_contacts where contactid = %d", event.contactid))<0) return;
		if (sqlNumtuples(sqr)==1) {
			zoneid=atoi(sqlGetvalue(sqr, 0, 0));
		}
		sqlFreeconnect(sqr);
		if (atoi(ptemp)<1) {
			prints(sid, "<CENTER><B>Please specify a group.</B></CENTER>\n");
		} else if ((rc=db_autoassign(&uavail, atoi(ptemp), zoneid, event.eventid, event.busy, event.eventstart, event.eventfinish))<0) {
			prints(sid, "<CENTER><B>Could not find an available user for this event.</B></CENTER>\n");
		} else {
			event.assignedto=uavail.userid;
			event.eventfinish+=uavail.time-event.eventstart;
			event.eventstart+=uavail.time-event.eventstart;
		}
		calendaredit(sid, &event);
		return;
	}
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_events WHERE eventid = %d", event.eventid)<0) return;
		prints(sid, "<CENTER>Calendar event %d deleted successfully</CENTER><BR>\n", event.eventid);
		logaccess(sid, 1, "%s - %s deleted calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/list\">\n", sid->dat->in_ScriptName);
	} else if (event.eventid==0) {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_INSERT)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (event.assignedto<1) {
			prints(sid, "<CENTER><B>This event must be assigned to a user.</B></CENTER>\n");
			calendaredit(sid, &event);
			return;
		}
		if ((rc=db_availcheck(event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
			if (rc==-1) {
				prints(sid, "<CENTER><B>This user is not available for the requested time.</B></CENTER>\n");
			} else {
				prints(sid, "<CENTER><B>There is a scheduling conflict with this event and event <A HREF=%s/calendar/view?eventid=%d>%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, rc, rc);
			}
			calendaredit(sid, &event);
			return;
		}
		if ((event.eventid=db_write(sid, DB_EVENTS, 0, &event))<1) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Calendar event %d added successfully</CENTER><BR>\n", event.eventid);
		logaccess(sid, 1, "%s - %s added calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/view?eventid=%d\">\n", sid->dat->in_ScriptName, event.eventid);
	} else {
		if (!(auth_priv(sid, AUTH_CALENDAR)&A_MODIFY)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (event.assignedto<1) {
			prints(sid, "<CENTER><B>This event must be assigned to a user.</B></CENTER>\n");
			calendaredit(sid, &event);
			return;
		}
		if ((rc=db_availcheck(event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
			if (rc==-1) {
				prints(sid, "<CENTER><B>This user is not available for the requested time.</B></CENTER>\n");
			} else {
				prints(sid, "<CENTER><B>There is a scheduling conflict with this event and event <A HREF=%s/calendar/view?eventid=%d>%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, rc, rc);
			}
			calendaredit(sid, &event);
			return;
		}
		if (db_write(sid, DB_EVENTS, eventid, &event)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Event %d modified successfully</CENTER><BR>\n", event.eventid);
		logaccess(sid, 1, "%s - %s modified calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/view?eventid=%d\">\n", sid->dat->in_ScriptName, event.eventid);
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
void calendarmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/calendar/reminders", 19)==0) {
		htpage_header(sid, "Groupware Event Reminder");
		calendarreminders(sid);
		htpage_footer(sid);
		return;
	} else if (strncmp(sid->dat->in_RequestURI, "/calendar/reminderreset", 23)==0) {
		htpage_header(sid, "Groupware Event Reminder");
		reminderstatus(sid);
		calendarreminders(sid);
		htpage_footer(sid);
		return;
	}
	htpage_topmenu(sid, MENU_CALENDAR);
	if (strncmp(sid->dat->in_RequestURI, "/calendar/edit", 14)==0)
		calendaredit(sid, NULL);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/view", 14)==0)
		calendarview(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/listweek", 18)==0)
		calendarlistweek(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/list", 14)==0)
		calendarlistday(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/mlist", 15)==0)
		calendarlistmonth(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/ylist", 15)==0)
		calendarlistyear(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/save", 14)==0)
		calendarsave(sid);
	htpage_footer(sid);
	return;
}
