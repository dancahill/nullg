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
#define SRVMOD_MAIN 1
#include "http_mod.h"
#include "mod_calendar.h"

void htselect_eventclosingstatus(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT eventclosingid, closingname FROM gw_eventclosings ORDER BY closingname ASC"))<0) return;
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
	char *ptemp1, *ptemp2;
	int i;
	int j;
	int sqr;
	int sqr2;
	int status;

	if ((ptemp1=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp1);
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
	if ((sqr=sql_queryf("SELECT userid, groupid, username FROM gw_users WHERE domainid = %d order by username ASC", sid->dat->user_did))<0) return;
	if ((sqr2=sql_queryf("SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d order by groupname ASC", sid->dat->user_did))<0) return;
	for (i=0;i<sql_numtuples(sqr2);i++) {
		prints(sid, "document.write(\"<OPTION VALUE='%s%s?groupid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr2, i, 0)));
		if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp1));
		} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
			prints(sid, "&month=%d", atoi(ptemp1));
			prints(sid, "&year=%d", atoi(ptemp2));
		}
		prints(sid, "&status=%d", status);
		prints(sid, "'%s>%s\");\n", atoi(sql_getvalue(sqr2, i, 0))==groupid?" SELECTED":"", str2html(sid, sql_getvalue(sqr2, i, 1)));
		for (j=0;j<sql_numtuples(sqr);j++) {
			if (atoi(sql_getvalue(sqr, j, 1))!=atoi(sql_getvalue(sqr2, i, 0))) continue;
			prints(sid, "document.write(\"<OPTION VALUE='%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, j, 0)));
			if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
				prints(sid, "&day=%d", atoi(ptemp1));
			} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
				prints(sid, "&month=%d", atoi(ptemp1));
				prints(sid, "&year=%d", atoi(ptemp2));
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
		if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp1));
		} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
			prints(sid, "&month=%d", atoi(ptemp1));
			prints(sid, "&year=%d", atoi(ptemp2));
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
		if ((ptemp1=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp1));
		} else if (((ptemp1=getgetenv(sid, "MONTH"))!=NULL)&&((ptemp2=getgetenv(sid, "YEAR"))!=NULL)) {
			prints(sid, "&month=%d", atoi(ptemp1));
			prints(sid, "&year=%d", atoi(ptemp2));
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
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==userid?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 2)));
	}
	prints(sid, "</SELECT>");
//	prints(sid, "<SELECT NAME=groupid>\r\n");
//	for (i=0;i<sql_numtuples(sqr2);i++) {
//		j=atoi(sql_getvalue(sqr2, i, 0));
//		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==groupid?" SELECTED":"", str2html(sid, sql_getvalue(sqr2, i, 1)));
//	}
//	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>");
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

	if ((sqr=sql_queryf("SELECT eventtypeid, eventtypename FROM gw_eventtypes ORDER BY eventtypename ASC"))<0) return;
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
	if ((sqr=sql_queryf("SELECT eventid, eventname, eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 AND obj_did = %d ORDER BY eventstart ASC", posttime, sid->dat->user_uid, sid->dat->user_did))<0) return;
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>%s</FONT></B>\n", CAL_EVENT_TITLE);
	for (i=0;i<sql_numtuples(sqr);i++) {
		a=time_sql2unix(sql_getvalue(sqr, i, 2))-time(NULL);
		b=a-atoi(sql_getvalue(sqr, i, 3))*60;
		if (b<0) {
			reminders++;
			if (reminders==1) {
				prints(sid, "<EMBED SRC=/groupware/sounds/reminder.wav AUTOSTART=TRUE HIDDEN=true VOLUME=100>\n");
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
				prints(sid, "<TR><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH><TH ALIGN=LEFT WIDTH=100%% STYLE='border-style:solid'>&nbsp;%s&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;Time&nbsp;</TH></TR>\n", CAL_EVENT_NAME);
			}
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
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
		prints(sid, "<BR>" CAL_EVENT_NONE "</CENTER>\n");
//		prints(sid, "<SCRIPT LANGUAGE=JavaScript>window.close('_top');</SCRIPT>\n");
	} else {
		prints(sid, "</TABLE></CENTER>\n");
	}
	return;
}

void reminderstatus(CONN *sid)
{
	char timebuffer[50];
	char *ptemp;
	int eventid;
	int reminder;
	int sqr;
	time_t eventstart;
	time_t now;

	if (!(auth_priv(sid, "calendar")&A_READ)) {
		return;
	}
	if ((ptemp=getgetenv(sid, "EVENTID"))==NULL) return;
	eventid=atoi(ptemp);
	if ((sqr=sql_queryf("SELECT reminder, eventstart FROM gw_events where eventid = %d and assignedto = %d AND obj_did = %d", eventid, sid->dat->user_uid, sid->dat->user_did))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	reminder=atoi(sql_getvalue(sqr, 0, 0));
	eventstart=time_sql2unix(sql_getvalue(sqr, 0, 1));
	now=time(NULL);
	time_unix2sql(timebuffer, sizeof(timebuffer)-1, now);
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
	sql_updatef("UPDATE gw_events SET obj_mtime = '%s', reminder = %d WHERE eventid = %d", timebuffer, reminder, eventid);
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
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_EVENT eventrec;
	char startdate[30];
	char finishdate[30];
	char *ptemp;
	int autogroup;
	int copyid=0;
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
			if ((ptemp=getgetenv(sid, "PROJECTID"))!=NULL) eventrec.projectid=atoi(ptemp);
			if ((ptemp=getgetenv(sid, "USERID"))!=NULL) eventrec.assignedto=atoi(ptemp);
			if ((ptemp=getgetenv(sid, "TIME"))!=NULL) {
				eventrec.eventstart=atoi(ptemp)-time_tzoffset(sid, eventrec.eventstart);
				eventrec.eventfinish=atoi(ptemp)-time_tzoffset(sid, eventrec.eventfinish)+900;
			}
			if (eventrec.eventfinish<eventrec.eventstart) eventrec.eventfinish=eventrec.eventstart;
		} else {
			eventid=0;
			if ((ptemp=getgetenv(sid, "EVENTID"))!=NULL) eventid=atoi(ptemp);
			if ((ptemp=getgetenv(sid, "COPYEVENTID"))!=NULL) { copyid=atoi(ptemp); eventid=copyid; }
			if (dbread_event(sid, 2, eventid, &eventrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", eventid);
				return;
			}
			if (copyid) { eventid=0; eventrec.eventid=0; }
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
	prints(sid, "function ShowCalendar() {\n");
	prints(sid, "	show_calendar('eventedit.eventstart', document.eventedit.eventstart3.value, document.eventedit.eventstart2.value, document.eventedit.eventstart1.value);\n");
	prints(sid, "	return;\n");
	prints(sid, "}\n");
	htscript_showpage(sid, 4);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<script language=JavaScript src=\"%s/groupware/javascript/calpopup.js\"></script>\n", sid->dat->in_ScriptName);
	prints(sid, "<CENTER>\n");
	if (event->eventid>0) {
		prints(sid, "<B><A HREF=%s/calendar/view?eventid=%d>Calendar Event %d</A></B>\n", sid->dat->in_ScriptName, event->eventid, event->eventid);
	} else {
		prints(sid, "<B>New Calendar Event</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=550>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/calendar/save NAME=eventedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=eventid VALUE='%d'>\n", event->eventid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>SETTINGS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>DETAILS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Event Name&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
	prints(sid, "%s", str2html(sid, event->eventname));
	prints(sid, "</TD>\n");
	if (event->eventid>0) {
		prints(sid, "<TD ALIGN=right><A HREF=%s/calendar/vcardexport?eventid=%d>iCalendar</A></TD>\n", sid->dat->in_ScriptName, event->eventid);
	} else {
		prints(sid, "<TD ALIGN=right>&nbsp;</TD>\n");
	}
	prints(sid, "</TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Event Name&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=eventname value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", str2html(sid, event->eventname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Assign to&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=assignedto style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, event->assignedto);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Event Type&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=eventtype style='width:182px'>\n");
	htselect_eventtype(sid, event->eventtype);
	prints(sid, "</SELECT></TD></TR>\n");
	if (module_exists("mod_contacts")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<A HREF=javascript:ContactView() CLASS=\"EDITFORM\">Contact Name</A>&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=contactid style='width:182px'>\n");
		htselect_contact(sid, event->contactid);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	if ((module_exists("mod_projects"))&&(event->projectid>0)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Project&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=projectid value=\"%d\" SIZE=25 style='width:182px'></TD></TR>\n", event->projectid);
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Priority&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=priority style='width:182px'>\n");
	htselect_priority(sid, event->priority);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Reminder&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=reminder style='width:182px'>\n");
	htselect_reminder(sid, event->reminder);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;<A HREF=javascript:ShowCalendar() CLASS=\"EDITFORM\">Date</A>&nbsp;</B></TD><TD ALIGN=RIGHT NOWRAP><SELECT NAME=eventstart2>\n");
	htselect_month(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart1>\n");
	htselect_day(sid, startdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=eventstart3>\n");
	htselect_year(sid, 2000, startdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Start Time&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=starttime style='width:182px'>\n");
	htselect_time(sid, event->eventstart);
	prints(sid, "</SELECT></TD></TR>\n");
	duration=event->eventfinish-event->eventstart;
	if (duration<0) duration=0;
	duration/=60;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Duration&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=duration1 style='width:91px'>\n");
	htselect_qhours(sid, duration/60);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=duration2 style='width:91px'>\n");
	htselect_qminutes(sid, duration%60);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Availability&nbsp;</B></TD><TD ALIGN=RIGHT>");
	prints(sid, "<SELECT NAME=busy style='width:182px'>\n");
	prints(sid, "<OPTION VALUE=0%s>Available\n", event->busy==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE=1%s>Busy\n", event->busy!=0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=status style='width:182px' onchange=CloseUpdate();>\n");
	htselect_eventstatus(sid, event->status);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Closing Status&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=closingstatus style='width:182px'>\n");
	htselect_eventclosingstatus(sid, event->closingstatus);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=details ROWS=4 COLS=60 style='width:100%%'>%s</TEXTAREA></TD></TR>\n", str2html(sid, event->details));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if ((event->obj_uid==sid->dat->user_uid)||(auth_priv(sid, "calendar")&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, event->obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "calendar")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "calendar"), event->obj_gid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", event->obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", event->obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", event->obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", event->obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", event->obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", event->obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	if (event->eventid>0) {
		if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
			prints(sid, "<TR><TD NOWRAP>");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH NOWRAP STYLE='border-style:solid'>Notes");
			prints(sid, " [<A HREF=%s/notes/editnew?table=events&index=%d>new</A>]", sid->dat->in_ScriptName, event->eventid);
			prints(sid, "</FONT></TH></TR>\n");
			mod_notes_sublist(sid, "events", event->eventid, 1);
			prints(sid, "</TABLE>\n");
			prints(sid, "</TD></TR>\n");
		}
	}
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Autoschedule'>\n");
	if (auth_priv(sid, "calendar")&A_ADMIN) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Autoassign'>");
		prints(sid, "<SELECT NAME=autogroup>\n");
		htselect_group(sid, auth_priv(sid, "calendar"), autogroup, sid->dat->user_did);
		prints(sid, "</SELECT><BR>\n");
	}
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, "calendar")&A_DELETE)&&(event->eventid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	if ((auth_priv(sid, "calendar")&A_INSERT)&&(event->eventid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Copy'>\n");
	}
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	if (event->eventid>0) {
		prints(sid, "showpage(1);\n");
	} else {
		prints(sid, "showpage(2);\n");
		prints(sid, "document.eventedit.eventname.focus();\n");
	}
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
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_EVENT event;
	char *ptemp;
	int eventid;
	int i;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "EVENTID"))==NULL) return;
	eventid=atoi(ptemp);
	if (dbread_event(sid, 1, eventid, &event)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", eventid);
		return;
	}
	event.eventstart+=time_tzoffset(sid, event.eventstart);
	event.eventfinish+=time_tzoffset(sid, event.eventfinish);
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>%s", str2html(sid, event.eventname));
	if (auth_priv(sid, "calendar")&A_MODIFY) {
		if (auth_priv(sid, "contacts")&A_ADMIN) {
			prints(sid, " [<A HREF=%s/calendar/edit?eventid=%d>edit</A>]", sid->dat->in_ScriptName, event.eventid);
		} else if ((event.assignedby==sid->dat->user_uid)||(event.assignedto==sid->dat->user_uid)||(event.obj_uid==sid->dat->user_uid)||((event.obj_gid==sid->dat->user_gid)&&(event.obj_gperm>=2))||(event.obj_operm>=2)) {
			prints(sid, " [<A HREF=%s/calendar/edit?eventid=%d>edit</A>]", sid->dat->in_ScriptName, event.eventid);
		}
	}
	prints(sid, "</TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned By </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>");
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users"))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==event.assignedby) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Date        </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100 STYLE='border-style:solid'><NOBR>%s&nbsp;</NOBR></TD></TR>\n", time_unix2datetext(sid, event.eventstart));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Assigned To </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>");
	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 0))==event.assignedto) {
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	prints(sid, "&nbsp;</TD>\n");
	sql_freeresult(sqr);
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Start Time  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", time_unix2timetext(sid, event.eventstart));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Event Type  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>\n", htview_eventtype(sid, event.eventtype));
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Finish Time </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", time_unix2timetext(sid, event.eventfinish));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Contact     </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>");
	if ((sqr=sql_queryf("SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", event.contactid))<0) return;
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, 0, 0)));
		prints(sid, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
		if (strlen(sql_getvalue(sqr, 0, 1))&&strlen(sql_getvalue(sqr, 0, 2))) prints(sid, ", ");
		prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 2)));
	}
	sql_freeresult(sqr);
	prints(sid, "&nbsp;</TD>\n", sql_getvalue(sqr, 0, 2));
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Availability</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", event.busy>0?"Busy":"Available");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Priority    </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>");
	if (event.priority==0) prints(sid, "Lowest");
	else if (event.priority==1) prints(sid, "Low");
	else if (event.priority==2) prints(sid, "Normal");
	else if (event.priority==3) prints(sid, "High");
	else if (event.priority==4) prints(sid, "Highest");
	prints(sid, "&nbsp;</TD>\n");
	prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Status      </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", htview_eventstatus(event.status));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Reminder    </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>\n", htview_reminder(sid, event.reminder));
	if (event.status) {
		prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Closing Status</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", htview_eventclosingstatus(sid, event.closingstatus));
	} else {
		prints(sid, "    <TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'>&nbsp;</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>&nbsp;</TD></TR>\n");
	}
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=4 STYLE='border-style:solid'><B>Details</B></TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=4 STYLE='border-style:solid'><PRE>%s&nbsp;</PRE></TD></TR>\n", str2html(sid, event.details));
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TH COLSPAN=4 NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=events&index=%d>new</A>]", sid->dat->in_ScriptName, event.eventid);
		prints(sid, "</TH></TR>\n");
		mod_notes_sublist(sid, "events", event.eventid, 4);
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
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
	if ((ptemp=getpostenv(sid, "PROJECTID"))!=NULL) event.projectid=atoi(ptemp);
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
		if ((sqr=sql_queryf("SELECT geozone FROM gw_contacts where contactid = %d", event.contactid))<0) return;
		if (sql_numtuples(sqr)==1) {
			zoneid=atoi(sql_getvalue(sqr, 0, 0));
		}
		sql_freeresult(sqr);
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
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Copy")==0)) {
		prints(sid, "<CENTER><B>Copy of event <A HREF=%s/calendar/edit?eventid=%d>%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, event.eventid, event.eventid);
		event.eventid=0;
		calendaredit(sid, &event);
		return;
	} else if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "calendar")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_events WHERE eventid = %d", event.eventid)<0) return;
		prints(sid, "<CENTER>Calendar event %d deleted successfully</CENTER><BR>\n", event.eventid);
		db_log_activity(sid, "events", event.eventid, "delete", "%s - %s deleted calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
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
		if ((rc=db_availcheck(event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
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
		db_log_activity(sid, "events", event.eventid, "insert", "%s - %s added calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
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
		if ((rc=db_availcheck(event.assignedto, event.eventid, event.busy, event.eventstart, event.eventfinish))!=0) {
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
		db_log_activity(sid, "events", event.eventid, "modify", "%s - %s modified calendar event %d", sid->dat->in_RemoteAddr, sid->dat->user_username, event.eventid);
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
DllExport int mod_main(CONN *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/calendar/vcardexport", 21)==0) {
		event_vcalexport(sid);
		return 0;
/*
	} else if (strncmp(sid->dat->in_RequestURI, "/calendar/vcardimport", 21)==0) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		event_vcalimport(sid);
		return 0;
*/
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/calendar/reminders", 19)==0) {
		htpage_header(sid, "Groupware Event Reminder");
		calendarreminders(sid);
		htpage_footer(sid);
		return 0;
	} else if (strncmp(sid->dat->in_RequestURI, "/calendar/reminderreset", 23)==0) {
		htpage_header(sid, "Groupware Event Reminder");
		reminderstatus(sid);
		calendarreminders(sid);
		htpage_footer(sid);
		return 0;
	}
	htpage_topmenu(sid, MENU_CALENDAR);
	if (strncmp(sid->dat->in_RequestURI, "/calendar/edit", 14)==0)
		calendaredit(sid, NULL);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/view", 14)==0)
		calendarview(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/list", 14)==0)
		calendarlistday(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/wlist", 15)==0)
		calendarlistweek(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/mlist", 15)==0)
		calendarlistmonth(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/ylist", 15)==0)
		calendarlistyear(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/availmap", 18)==0)
		calendar_availmap(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/calendar/save", 14)==0)
		calendarsave(sid);
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_calendar",		// mod_name
		2,			// mod_submenu
		"CALENDAR",		// mod_menuname
		"/calendar/list",	// mod_menuuri
		"calendar",		// mod_menuperm
		"mod_main",		// fn_name
		"/calendar/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}