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
#include "mod_stub.h"
#include "mod_calendar.h"

void htselect_eventclosingstatus(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT eventclosingid, closingname FROM gw_eventclosings ORDER BY closingname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_eventfilter(CONN *sid, int userid, int groupid, char *baseuri)
{
	char *option[]={ "Open", "Closed", "All" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int sqr2;
	int status;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if ((userid<1)&&(groupid<1)) {
		userid=sid->dat->user_uid;
	}
	prints(sid, "<FORM METHOD=GET NAME=eventfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	if (document.eventfilter.userid.options[document.eventfilter.userid.selectedIndex].value!=\"\") {\r\n");
	prints(sid, "		location=document.eventfilter.userid.options[document.eventfilter.userid.selectedIndex].value\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.eventfilter.status.options[document.eventfilter.status.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	if ((sqr=sql_queryf(sid, "SELECT userid, groupid, username FROM gw_users order by username ASC"))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT groupid, groupname FROM gw_groups order by groupname ASC"))<0) return;
	for (i=0;i<sql_numtuples(sqr2);i++) {
		prints(sid, "document.write(\"<OPTION VALUE='%s%s?groupid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr2, i, 0)));
		if ((ptemp=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp));
		} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
			prints(sid, "&month=%d", atoi(getgetenv(sid, "MONTH")));
			prints(sid, "&year=%d", atoi(getgetenv(sid, "YEAR")));
		}
		prints(sid, "&status=%d", status);
		prints(sid, "'%s>%s\");\n", atoi(sql_getvalue(sqr2, i, 0))==groupid?" SELECTED":"", str2html(sid, sql_getvalue(sqr2, i, 1)));
		for (j=0;j<sql_numtuples(sqr);j++) {
			if (atoi(sql_getvalue(sqr, j, 1))!=atoi(sql_getvalue(sqr2, i, 0))) continue;
			prints(sid, "document.write(\"<OPTION VALUE='%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, j, 0)));
			if ((ptemp=getgetenv(sid, "DAY"))!=NULL) {
				prints(sid, "&day=%d", atoi(ptemp));
			} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
				prints(sid, "&month=%d", atoi(getgetenv(sid, "MONTH")));
				prints(sid, "&year=%d", atoi(getgetenv(sid, "YEAR")));
			}
			prints(sid, "&status=%d", status);
			prints(sid, "'%s>&nbsp;&nbsp;%s\");\n", atoi(sql_getvalue(sqr, j, 0))==userid?" SELECTED":"", str2html(sid, sql_getvalue(sqr, j, 2)));
		}
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvalue(sqr, i, 1))==atoi(sql_getvalue(sqr2, j, 0))) break;
		}
		if (j<sql_numtuples(sqr2)) continue;
		prints(sid, "document.write(\"<OPTION VALUE='%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, i, 0)));
		if ((ptemp=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp));
		} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
			prints(sid, "&month=%d", atoi(getgetenv(sid, "MONTH")));
			prints(sid, "&year=%d", atoi(getgetenv(sid, "YEAR")));
		}
		prints(sid, "&status=%d", status);
		prints(sid, "'%s>*%s\");\n", atoi(sql_getvalue(sqr, i, 0))==userid?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 2)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "document.write('<SELECT NAME=status onChange=\"go2()\">');\r\n");
	for (i=0, j=2;i<3;i++, j++) {
		if (j>2) j=0;
		if (userid>0) {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, userid);
		} else {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?groupid=%d", sid->dat->in_ScriptName, baseuri, groupid);
		}
		if ((ptemp=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp));
		} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
			prints(sid, "&month=%d", atoi(getgetenv(sid, "MONTH")));
			prints(sid, "&year=%d", atoi(getgetenv(sid, "YEAR")));
		}
		prints(sid, "&status=%d\"%s>%s');\n", j, j==status?" SELECTED":"", option[j]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==userid?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=groupid>\r\n");
	for (i=0;i<sql_numtuples(sqr2);i++) {
		j=atoi(sql_getvalue(sqr2, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==groupid?" SELECTED":"", str2html(sid, sql_getvalue(sqr2, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(sqr2);
	sql_freeresult(sqr);
	return;
}

void htselect_eventtype(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT eventtypeid, eventtypename FROM gw_eventtypes ORDER BY eventtypename ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void calendarreminders(CONN *sid)
{
	char posttime[32];
	int a, b;
	int i;
	int reminders=0;
	int sqr;
	time_t eventdate;
	time_t t;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if ((sqr=sql_queryf(sid, "SELECT eventid, eventname, eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>%s</FONT></B>\n", CAL_EVENT_TITLE);
	for (i=0;i<sql_numtuples(sqr);i++) {
		a=time_sql2unix(sql_getvalue(sqr, i, 2))-time(NULL);
		a-=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, i, 2)));
		b=a-atoi(sql_getvalue(sqr, i, 3))*60;
		if (b<0) {
			reminders++;
			if (reminders==1) {
				prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
				prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH><TH ALIGN=LEFT WIDTH=100%% STYLE='border-style:solid'>&nbsp;%s&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Time&nbsp;</TH></TR>\n", config->colour_th, CAL_EVENT_NAME);
			}
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD NOWRAP VALIGN=top STYLE='border-style:solid'><A HREF=%s/calendar/reminderreset?eventid=%s>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), CAL_EVENT_RESET);
			prints(sid, "<TD STYLE='border-style:solid'><A HREF=%s/calendar/view?eventid=%s TARGET=gwmain>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), str2html(sid, sql_getvalue(sqr, i, 1)));
			eventdate=time_sql2unix(sql_getvalue(sqr, i, 2));
			eventdate+=time_tzoffset(sid, eventdate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, eventdate));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2timetext(sid, eventdate));
			prints(sid, "</TR>\n");
		}
	}
	sql_freeresult(sqr);
	if (reminders==0) {
		prints(sid, "%s\n</CENTER>\n", CAL_EVENT_NONE);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>window.close('_top');</SCRIPT>\n");
	} else {
		prints(sid, "</TABLE></CENTER>\n");
	}
	return;
}

void reminderstatus(CONN *sid)
{
	char timebuffer[100];
	int eventid;
	int reminder;
	int sqr;
	time_t eventstart;
	time_t now;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		return;
	}
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if ((sqr=sql_queryf(sid, "SELECT reminder, eventstart FROM gw_events where eventid = %d and assignedto = %d", eventid, sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	reminder=atoi(sql_getvalue(sqr, 0, 0));
	eventstart=time_sql2unix(sql_getvalue(sqr, 0, 1));
	now=time(NULL);
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, now));
	sql_freeresult(sqr);
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
	sql_updatef(sid, "UPDATE gw_events SET obj_mtime = '%s', reminder = %d WHERE eventid = %d", timebuffer, reminder, eventid);
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
void calendaredit(CONN *sid, REC_EVENT *event)
{
	REC_EVENT eventrec;
	char startdate[30];
	char finishdate[30];
	char *ptemp;
	int autogroup;
	int eventid;
	int duration;

	if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (event==NULL) {
		prints(sid, "<BR>");
		if (strncmp(sid->dat->in_RequestURI, "/calendar/editnew", 17)==0) {
			eventid=0;
			if (dbread_event(sid, 2, 0, &eventrec)!=0) {
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
			if ((ptemp=getgetenv(sid, "EVENTID"))==NULL) return;
			eventid=atoi(ptemp);
			if (dbread_event(sid, 2, eventid, &eventrec)!=0) {
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
	prints(sid, "function ContactView() {\r\n");
	prints(sid, "	var contactid=document.eventedit.contactid.value;\r\n");
	prints(sid, "	if (contactid<1) {\r\n");
	prints(sid, "		window.open('%s/contacts/search1','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	} else {\r\n");
	prints(sid, "		window.open('%s/contacts/view?contactid='+contactid,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=610,height=410');\r\n", sid->dat->in_ScriptName);
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (event->eventid>0) {
		prints(sid, "<A HREF=%s/calendar/view?eventid=%d STYLE='color: %s'>Calendar Event %d</FONT></TH></TR>\n", sid->dat->in_ScriptName, event->eventid, config->colour_thtext, event->eventid);
	} else {
		prints(sid, "New Calendar Event</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Event Name&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=eventname value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", str2html(sid, event->eventname));
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:182px'%s>\n", config->colour_editform, (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, event->assignedto);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Event Type&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=eventtype style='width:182px'>\n", config->colour_editform);
	htselect_eventtype(sid, event->eventtype);
	prints(sid, "</SELECT></TD></TR>\n");
	if (module_exists(sid, "mod_contacts")) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;<A HREF=javascript:ContactView() STYLE='color: %s'>Contact Name</A>&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>\n", config->colour_editform, config->colour_fieldnametext);
		htselect_contact(sid, event->contactid);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Priority&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=priority style='width:182px'>\n", config->colour_editform);
	htselect_priority(sid, event->priority);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Reminder&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=reminder style='width:182px'>\n");
	htselect_reminder(sid, event->reminder);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Date&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=eventstart2>\n", config->colour_editform);
	htselect_month(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart1>\n");
	htselect_day(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart3>\n");
	htselect_year(sid, 2000, startdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Start Time&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=starttime style='width:182px'>\n");
	htselect_time(sid, event->eventstart);
	prints(sid, "</SELECT></TD></TR>\n");
	duration=event->eventfinish-event->eventstart;
	if (duration<0) duration=0;
	duration/=60;
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Duration&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=duration1 style='width:91px'>\n");
	htselect_qhours(sid, duration/60);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=duration2 style='width:91px'>\n");
	htselect_qminutes(sid, duration%60);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Availability&nbsp;</B></TD><TD ALIGN=RIGHT>", config->colour_editform);
	prints(sid, "<SELECT NAME=busy style='width:182px'>\n");
	prints(sid, "<OPTION VALUE=0%s>Available\n", event->busy==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE=1%s>Busy\n", event->busy!=0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:182px' onchange=CloseUpdate();>\n", config->colour_editform);
	htselect_eventstatus(sid, event->status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Closing Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=closingstatus style='width:182px'>\n", config->colour_editform);
	htselect_eventclosingstatus(sid, event->closingstatus);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Details&nbsp;</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=details ROWS=4 COLS=60 style='width:100%%'>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, event->details));
	if ((event->obj_uid==sid->dat->user_uid)||(auth_priv(sid, "calendar")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, event->obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, event->obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", event->obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", event->obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", event->obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", event->obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", event->obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", event->obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Autoschedule'>\n");
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Autoassign'>");
		prints(sid, "<SELECT NAME=autogroup>\n");
		htselect_group(sid, autogroup);
		prints(sid, "</SELECT><BR>\n");
	}
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, "calendar")&A_DELETE)&&(event->eventid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
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
void calendarview(CONN *sid)
{
	MOD_NOTES_SUBLIST mod_notes_sublist;
	REC_EVENT event;
	int eventid;
	int i;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "EVENTID")==NULL) return;
	eventid=atoi(getgetenv(sid, "EVENTID"));
	if (dbread_event(sid, 1, eventid, &event)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", eventid);
		return;
	}
	event.eventstart+=time_tzoffset(sid, event.eventstart);
	event.eventfinish+=time_tzoffset(sid, event.eventfinish);
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>%s", config->colour_th, config->colour_thtext, str2html(sid, event.eventname));
	if (auth_priv(sid, "calendar")&A_MODIFY) {
		if (auth_priv(sid, "contacts")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/calendar/edit?eventid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, event.eventid, config->colour_thlink);
		} else if ((event.assignedby==sid->dat->user_uid)||(event.assignedto==sid->dat->user_uid)||(event.obj_uid==sid->dat->user_uid)||((event.obj_gid==sid->dat->user_gid)&&(event.obj_gperm>=2))||(event.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/calendar/edit?eventid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, event.eventid, config->colour_thlink);
		}
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Assigned By </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>", config->colour_fieldname, config->colour_fieldval);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users"))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==event.assignedby) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Date        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100 STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2datetext(sid, event.eventstart));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Assigned To </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>", config->colour_fieldname, config->colour_fieldval);
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==event.assignedto) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	sql_freeresult(sqr);
	prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Start Time  </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2timetext(sid, event.eventstart));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Event Type  </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>\n", config->colour_fieldname, config->colour_fieldval, htview_eventtype(sid, event.eventtype));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Finish Time </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2timetext(sid, event.eventfinish));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Contact     </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>", config->colour_fieldname, config->colour_fieldval);
	if ((sqr=sql_queryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", event.contactid))<0) return;
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, 0, 0)));
		prints(sid, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
		if (strlen(sql_getvalue(sqr, 0, 1))&&strlen(sql_getvalue(sqr, 0, 2))) prints(sid, ", ");
		prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 2)));
	}
	sql_freeresult(sqr);
	prints(sid, "&nbsp;</TD>\n", sql_getvalue(sqr, 0, 2));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Availability</B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, event.busy>0?"Busy":"Available");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Priority    </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>", config->colour_fieldname, config->colour_fieldval);
	if (event.priority==0) prints(sid, "Lowest");
	else if (event.priority==1) prints(sid, "Low");
	else if (event.priority==2) prints(sid, "Normal");
	else if (event.priority==3) prints(sid, "High");
	else if (event.priority==4) prints(sid, "Highest");
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Status      </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, htview_eventstatus(sid, event.status));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Reminder    </B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>\n", config->colour_fieldname, config->colour_fieldval, htview_reminder(sid, event.reminder));
	if (event.status) {
		prints(sid, "    <TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'><B>Closing Status</B></TD><TD BGCOLOR=%s NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, htview_eventclosingstatus(sid, event.closingstatus));
	} else {
		prints(sid, "    <TD BGCOLOR=%s STYLE='border-style:solid'>&nbsp;</TD><TD BGCOLOR=%s STYLE='border-style:solid'>&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval);
	}
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=4 STYLE='border-style:solid'><B>Details</B></TD></TR>\n", config->colour_fieldname);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=4 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", config->colour_fieldval, str2html(sid, event.details));
	if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Notes", config->colour_th, config->colour_thtext);
		prints(sid, " [<A HREF=%s/notes/editnew?table=events&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, event.eventid, config->colour_thlink);
		prints(sid, "</FONT></TH></TR>\n");
		mod_notes_sublist(sid, "events", event.eventid, 4);
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
}

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
	MOD_TASKS_LIST mod_tasks_list;
	char *ptemp;
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
	} else if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
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
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid, username FROM gw_users"))<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Time&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Event Name&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext);
	for (i=0;i<24;i++) {
		snprintf(showtime, sizeof(showtime)-1, "%02d:00:00", i);
		if ((i>=sid->dat->user_daystart)&&(i<sid->dat->user_daystart+sid->dat->user_daylength)) {
			prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>", config->colour_fieldval);
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
				prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			} else {
				prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldname);
			}
			t2+=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, index, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><FONT SIZE=2>");
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>", sid->dat->in_ScriptName, sql_getvalue(sqr, index, 0), time_unix2timetext(sid, t2));
			prints(sid, "</FONT></TD><TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>");
			if (groupid>0) prints(sid, "[%s] ", str2html(sid, sql_getvalue(sqr2, j, 2)));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sql_getvalue(sqr, index, 0), str2html(sid, sql_getvalue(sqr, index, 5)));
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
	char *ptemp;
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
	} else if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_eventfilter(sid, userid, groupid, "/calendar/listweek");
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
	prints(sid, "<CENTER>\n<FONT COLOR=#000000><B><A HREF=%s/calendar/listweek?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)-7, status);
	if (userid>0) prints(sid, "&userid=%d", userid);
	if (groupid>0) prints(sid, "&groupid=%d", groupid);
	prints(sid, ">&lt;&lt;</A>");
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	prints(sid, " Calendar Events for %s", showtime);
	unixdate+=518400;
	strftime(showtime, sizeof(showtime), "%B %d, %Y", gmtime(&unixdate));
	unixdate-=518400;
	prints(sid, " to %s ", showtime);
	prints(sid, "<A HREF=%s/calendar/listweek?day=%d&status=%d", sid->dat->in_ScriptName, (unixdate/86400)+7, status);
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
	char *ptemp;
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(posttime1, 0, sizeof(posttime1));
	if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
		today.year=atoi(getgetenv(sid, "YEAR"));
		today.month=atoi(getgetenv(sid, "MONTH"));
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
	prints(sid, "<TR BGCOLOR=%s><TH WIDTH=200 style='width:14%%; border-style:solid''><FONT COLOR=%s>Sunday</FONT></TH><TH WIDTH=200 style='width:14%%; border-style:solid''><FONT COLOR=%s>Monday</FONT></TH>", config->colour_th, config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH WIDTH=200 style='width:14%%; border-style:solid'><FONT COLOR=%s>Tuesday</FONT></TH><TH WIDTH=200 style='width:14%%; border-style:solid'><FONT COLOR=%s>Wednesday</FONT></TH><TH WIDTH=200 style='width:14%%; border-style:solid'><FONT COLOR=%s>Thursday</FONT></TH>", config->colour_thtext, config->colour_thtext, config->colour_thtext);
	prints(sid, "<TH WIDTH=200 style='width:14%%; border-style:solid'><FONT COLOR=%s>Friday</FONT></TH><TH WIDTH=200 style='width:14%%; border-style:solid'><FONT COLOR=%s>Saturday</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext);
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
	if ((sqr2=sql_queryf(sid, "SELECT userid, groupid, username FROM gw_users WHERE groupid = %d ORDER BY userid ASC", groupid))<0) return;
	for (i=printdate;i<dim[today.month-1]+1;i+=7) {
		prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldval);
		for (j=0;j<7;j++) {
			if (unixdate==now) {
				prints(sid, "<TD BGCOLOR=#E0E0FF");
			} else if ((printdate>0)&&(printdate<=dim[today.month-1])) {
				prints(sid, "<TD BGCOLOR=%s", config->colour_fieldval);
			} else {
				prints(sid, "<TD BGCOLOR=#D0D0D0");
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
				t2+=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, index, 1)));
				prints(sid, "<FONT SIZE=1 STYLE='font-size:10px'><NOBR>%s ", time_unix2lotimetext(sid, t2));
				prints(sid, "<A HREF=%s/calendar/view?eventid=%d TITLE=\"", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, index, 0)));
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
		prints(sid, "<TR BGCOLOR=%s><TH STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Colour Key&nbsp;</FONT></TH></TR>", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP STYLE='border-style:solid'>", config->colour_fieldval);
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

/****************************************************************************
 *	calendarsave()
 *
 *	Purpose	: Save calendar event information to the database
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void calendarsave(CONN *sid)
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
	if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "EVENTID"))==NULL) return;
	eventid=atoi(ptemp);
	if (dbread_event(sid, 2, eventid, &event)!=0) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) event.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) event.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "calendar")&A_ADMIN)||(event.obj_uid==sid->dat->user_uid)) {
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
	if ((ptemp=getpostenv(sid, "EVENTSTART2"))!=NULL) strncatf(tempdate, sizeof(tempdate)-strlen(tempdate)-1, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "EVENTSTART1"))!=NULL) strncatf(tempdate, sizeof(tempdate)-strlen(tempdate)-1, "%02d ", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "STARTTIME"))!=NULL) strncatf(tempdate, sizeof(tempdate)-strlen(tempdate)-1, "%s", ptemp);
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
		if ((rc=db_autoschedule(sid, event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))<0) {
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
		if ((sqr=sql_queryf(sid, "SELECT geozone FROM gw_contacts where contactid = %d", event.contactid))<0) return;
		if (sql_numtuples(sqr)==1) {
			zoneid=atoi(sql_getvalue(sqr, 0, 0));
		}
		sql_freeresult(sqr);
		if (atoi(ptemp)<1) {
			prints(sid, "<CENTER><B>Please specify a group.</B></CENTER>\n");
		} else if ((rc=db_autoassign(sid, &uavail, atoi(ptemp), zoneid, event.eventid, event.busy, event.eventstart, event.eventfinish))<0) {
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
		if (!(auth_priv(sid, "calendar")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_events WHERE eventid = %d", event.eventid)<0) return;
		prints(sid, "<CENTER>Calendar event %d deleted successfully</CENTER><BR>\n", event.eventid);
		db_log_activity(sid, 1, "events", event.eventid, "delete", "%s - %s deleted calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/list\">\n", sid->dat->in_ScriptName);
	} else if (event.eventid==0) {
		if (!(auth_priv(sid, "calendar")&A_INSERT)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (event.assignedto<1) {
			prints(sid, "<CENTER><B>This event must be assigned to a user.</B></CENTER>\n");
			calendaredit(sid, &event);
			return;
		}
		if ((rc=db_availcheck(sid, event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
			if (rc==-1) {
				prints(sid, "<CENTER><B>This user is not available for the requested time.</B></CENTER>\n");
			} else {
				prints(sid, "<CENTER><B>There is a scheduling conflict with this event and event <A HREF=%s/calendar/view?eventid=%d>%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, rc, rc);
			}
			calendaredit(sid, &event);
			return;
		}
		if ((event.eventid=dbwrite_event(sid, 0, &event))<1) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Calendar event %d added successfully</CENTER><BR>\n", event.eventid);
		db_log_activity(sid, 1, "events", event.eventid, "insert", "%s - %s added calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/view?eventid=%d\">\n", sid->dat->in_ScriptName, event.eventid);
	} else {
		if (!(auth_priv(sid, "calendar")&A_MODIFY)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (event.assignedto<1) {
			prints(sid, "<CENTER><B>This event must be assigned to a user.</B></CENTER>\n");
			calendaredit(sid, &event);
			return;
		}
		if ((rc=db_availcheck(sid, event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
			if (rc==-1) {
				prints(sid, "<CENTER><B>This user is not available for the requested time.</B></CENTER>\n");
			} else {
				prints(sid, "<CENTER><B>There is a scheduling conflict with this event and event <A HREF=%s/calendar/view?eventid=%d>%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, rc, rc);
			}
			calendaredit(sid, &event);
			return;
		}
		if (dbwrite_event(sid, eventid, &event)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Event %d modified successfully</CENTER><BR>\n", event.eventid);
		db_log_activity(sid, 1, "events", event.eventid, "modify", "%s - %s modified calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
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
void mod_main(CONN *sid)
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

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod;

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	memset((char *)&newmod, 0, sizeof(newmod));
	newmod.mod_submenu=2;
	snprintf(newmod.mod_name,     sizeof(newmod.mod_name)-1,     "mod_calendar");
	snprintf(newmod.mod_menuname, sizeof(newmod.mod_menuname)-1, "CALENDAR");
	snprintf(newmod.mod_menuperm, sizeof(newmod.mod_menuperm)-1, "calendar");
	snprintf(newmod.mod_menuuri,  sizeof(newmod.mod_menuuri)-1,  "/calendar/list");
	snprintf(newmod.fn_name,      sizeof(newmod.fn_name)-1,      "mod_main");
	snprintf(newmod.fn_uri,       sizeof(newmod.fn_uri)-1,       "/calendar/");
	newmod.fn_ptr=mod_main;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
