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

void messagestatus(CONNECTION *sid, int messageid, char status)
{
	char timebuffer[100];
	time_t t;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((messageid==0)&&(getgetenv(sid, "MESSAGEID")!=NULL)) {
		messageid=atoi(getgetenv(sid, "MESSAGEID"));
	}
	if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_messages WHERE rcpt = %d and messageid = %d", sid->dat->user_uid, messageid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		return;
	}
	sqlFreeconnect(sqr);
	t=time(NULL);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status=='0') {
		sqlUpdatef(sid, "DELETE FROM gw_messages WHERE rcpt = %d and messageid = %d", sid->dat->user_uid, messageid);
		prints(sid, "<CENTER>Message deleted</CENTER><BR>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "window.open('%s/messages/list','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
		if (strncmp(sid->dat->in_RequestURI, "/messages/delete2", 17)==0) {
			prints(sid, "window.close('_top');\n");
		}
		prints(sid, "// -->\n</SCRIPT>\n");
	} else {
		sqlUpdatef(sid, "UPDATE gw_messages SET obj_mtime = '%s', status = %c WHERE messageid = %d", timebuffer, status, messageid);
	}
	return;
}

void messageautocheck(CONNECTION *sid)
{
	char **uidls;
	char posttime[32];
	int isreminder=0;
	int i, j;
	int a, b;
	int sqr;
	int lastcount;
	int nummessages;
	time_t t;

	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (auth_priv(sid, AUTH_CALENDAR)&A_READ) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventstart, reminder FROM gw_events where eventstart < #%s# and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
		}
		for (i=0;i<sqlNumtuples(sqr);i++) {
			a=time_sql2unix(sqlGetvalue(sqr, i, 0))-time(NULL);
			b=a-atoi(sqlGetvalue(sqr, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/calendar/reminders','calendarwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sqlFreeconnect(sqr);
	}
	if (auth_priv(sid, AUTH_CALENDAR)&A_READ) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT duedate, reminder FROM gw_tasks where duedate < #%s# and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
		}
		for (i=0;i<sqlNumtuples(sqr);i++) {
			a=time_sql2unix(sqlGetvalue(sqr, i, 0))-time(NULL);
			a-=time_tzoffset(sid, time_sql2unix(sqlGetvalue(sqr, i, 0)));
			b=a-atoi(sqlGetvalue(sqr, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/tasks/reminders','taskwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sqlFreeconnect(sqr);
	}
	if (auth_priv(sid, AUTH_MESSAGES)&A_READ) {
		if ((sqr=sqlQueryf(sid, "SELECT messageid FROM gw_messages WHERE rcpt = %d and status > 2", sid->dat->user_uid))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/messages/list','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sqlFreeconnect(sqr);
	}
	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname, poppassword, lastcount FROM gw_mailaccounts where obj_uid = %d and notify = 1", sid->dat->user_uid))<0) return;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (strlen(sqlGetvalue(sqr, i, 2))==0) continue;
		sid->dat->user_mailcurrent=atoi(sqlGetvalue(sqr, i, 0));
		if ((nummessages=wmserver_mlistsync(sid, &uidls))<0) continue;
		for (j=0;j<nummessages;j++) {
//			logerror(sid, __FILE__, __LINE__, "[%d %d %s]", j, uidls[j], uidls[j]);
			free(uidls[j]);
		}
//		logerror(sid, __FILE__, __LINE__, "[%d]", uidls);
//		logerror(sid, __FILE__, __LINE__, "[%d]", *uidls);
		free(uidls);
		lastcount=atoi(sqlGetvalue(sqr, i, 3));
		if (lastcount>nummessages) {
			lastcount=nummessages;
			sqlUpdatef(sid, "UPDATE gw_mailaccounts SET lastcount = %d WHERE obj_uid = %d AND mailaccountid = %d", nummessages, sid->dat->user_uid, sid->dat->user_mailcurrent);
		}
		if (nummessages-lastcount>0) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/mail/notice','mailwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
	}
	sqlFreeconnect(sqr);
	return;
}

void messagelist(CONNECTION *sid)
{
	int i;
	int sqr;
	time_t msgdate;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sqlQueryf(sid, "SELECT messageid, sender, subject, obj_ctime, status FROM gw_messages WHERE rcpt = %d and status > 0 ORDER BY messageid DESC", sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		prints(sid, "<CENTER>You have no messages.</CENTER><BR>\n");
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function ReadMessage(msg)\n");
	prints(sid, "{\n");
	prints(sid, "	window.open('%s/messages/read?messageid='+msg,'msgwin2','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');", sid->dat->in_ScriptName);
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH>&nbsp;</TH><TH ALIGN=left><FONT COLOR=%s>&nbsp;From&nbsp;</FONT></TH><TH ALIGN=left WIDTH=100%%><FONT COLOR=%s>&nbsp;Subject&nbsp;</FONT></TH><TH ALIGN=LEFT><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 4))<1) continue;
		if (atoi(sqlGetvalue(sqr, i, 4))>1) {
			prints(sid, "<TR BGCOLOR=#D0D0FF>");
		} else {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
		}
		prints(sid, "<TD NOWRAP><A HREF=%s/messages/delete?messageid=%s>delete</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints(sid, "<TD NOWRAP>%s</TD>", str2html(sid, htview_user(sid, atoi(sqlGetvalue(sqr, i, 1)))));
		prints(sid, "<TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=ReadMessage(%d)>", atoi(sqlGetvalue(sqr, i, 0)));
		if (strlen(sqlGetvalue(sqr, i, 2))==0) {
			prints(sid, "<A HREF=\"javascript:ReadMessage(%d)\" TITLE=\"No Subject\">", atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "(No Subject)</A></TD>");
		} else {
			prints(sid, "<A HREF=\"javascript:ReadMessage(%d)\" TITLE=\"%s\">", atoi(sqlGetvalue(sqr, i, 0)), str2html(sid, sqlGetvalue(sqr, i, 2)));
			prints(sid, "%.25s", str2html(sid, sqlGetvalue(sqr, i, 2)));
			if (strlen(sqlGetvalue(sqr, i, 2))>25) prints(sid, "..");
			prints(sid, "</A></TD>");
		}
		msgdate=time_sql2unix(sqlGetvalue(sqr, i, 3));
		msgdate+=time_tzoffset(sid, msgdate);
		prints(sid, "<TD NOWRAP>%s", time_unix2datetext(sid, msgdate));
		prints(sid, " %s</TD></TR>\n", time_unix2timetext(sid, msgdate));
		if (atoi(sqlGetvalue(sqr, i, 4))>2) {
			messagestatus(sid, atoi(sqlGetvalue(sqr, i, 0)), '2');
		}
	}
	prints(sid, "</TABLE></CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void messageread(CONNECTION *sid)
{
	int messageid;
	int sqr;
	time_t msgdate;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "MESSAGEID")==NULL) return;
	messageid=atoi(getgetenv(sid, "MESSAGEID"));
	if ((sqr=sqlQueryf(sid, "SELECT obj_ctime, messageid, sender, subject, message FROM gw_messages WHERE rcpt = %d and messageid = %d", sid->dat->user_uid, messageid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints(sid, "<CENTER>No matching message found for %d</CENTER>\n", messageid);
		sqlFreeconnect(sqr);
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR><TD BGCOLOR=%s WIDTH=50><B>From</B></TD>", COLOR_FNAME);
	prints(sid, "<TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s</TD></TR>\n", COLOR_FVAL, str2html(sid, htview_user(sid, atoi(sqlGetvalue(sqr, 0, 2)))));
	prints(sid, "<TR><TD BGCOLOR=%s WIDTH=50><B>Subject</B></TD>", COLOR_FNAME);
	if (strlen(sqlGetvalue(sqr, 0, 3))==0) {
		prints(sid, "<TD BGCOLOR=%s NOWRAP WIDTH=100%%>(No Subject)</TD></TR>\n", COLOR_FVAL);
	} else {
		prints(sid, "<TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FVAL, str2html(sid, sqlGetvalue(sqr, 0, 3)));
	}
	prints(sid, "<TR><TD BGCOLOR=%s WIDTH=50><B>Date</B></TD>", COLOR_FNAME);
	msgdate=time_sql2unix(sqlGetvalue(sqr, 0, 0));
	msgdate+=time_tzoffset(sid, msgdate);
	prints(sid, "<TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s", COLOR_FVAL, time_unix2datetext(sid, msgdate));
	prints(sid, " %s</TD></TR>\n", time_unix2timetext(sid, msgdate));
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2>%s&nbsp;</TD></TR></TABLE>\n", COLOR_FVAL, str2html(sid, sqlGetvalue(sqr, 0, 4)));
	prints(sid, "[<A HREF=%s/messages/send?replyto=%d>Reply</A>]&nbsp;", sid->dat->in_ScriptName, messageid);
	prints(sid, "[<A HREF=%s/messages/delete2?messageid=%d>Delete</A>]\n", sid->dat->in_ScriptName, messageid);
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
	messagestatus(sid, messageid, '1');
	return;
}

void messagesend(CONNECTION *sid)
{
	char subject[50];
	char *ptemp;
	int rcpt=0;
	int replyto=0;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(subject, 0, sizeof(subject));
	if ((ptemp=getgetenv(sid, "REPLYTO"))!=NULL) replyto=atoi(ptemp);
	if (replyto>0) {
		if ((sqr=sqlQueryf(sid, "SELECT sender, subject FROM gw_messages WHERE rcpt = %d and messageid = %d", sid->dat->user_uid, replyto))<0) return;
		if (sqlNumfields(sqr)<1) {
			sqlFreeconnect(sqr);
			return;
		}
		rcpt=atoi(sqlGetvalue(sqr, 0, 0));
		if (strncasecmp(sqlGetvalue(sqr, 0, 1), "RE:", 3)!=0) {
			snprintf(subject, sizeof(subject)-1, "Re: %s", sqlGetvalue(sqr, 0, 1));
		} else {
			snprintf(subject, sizeof(subject)-1, "%s", sqlGetvalue(sqr, 0, 1));
		}
		sqlFreeconnect(sqr);
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/messages/save NAME=msgcompose>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;To&nbsp;</B></TD><TD><SELECT NAME=rcpt style='width:287px'>\n", COLOR_EDITFORM);
	htselect_user(sid, rcpt);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Subject&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=subject VALUE=\"%s\" SIZE=40 style='width:287px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, subject));
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER COLSPAN=4><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=8 COLS=50></TEXTAREA></TD>\n");
	prints(sid, "</TR></TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Send VALUE='Send Message'></FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.msgcompose.rcpt.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void messagesave(CONNECTION *sid)
{
	char curdate[50];
	char message[1024];
	char subject[50];
	char *ptemp;
	int messageid;
	int rcpt;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((sqr=sqlQuery(sid, "SELECT max(messageid) FROM gw_messages"))<0) return;
	messageid=atoi(sqlGetvalue(sqr, 0, 0))+1;
	if (messageid<1) messageid=1;
	sqlFreeconnect(sqr);
	rcpt=0;
	memset(subject, 0, sizeof(subject));
	memset(message, 0, sizeof(message));
	if ((ptemp=getpostenv(sid, "RCPT"))!=NULL) rcpt=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL) snprintf(subject, sizeof(subject)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) snprintf(message, sizeof(message)-1, "%s", ptemp);
	if (rcpt==0) {
		prints(sid, "<CENTER>No recipient was specified</CENTER><BR>\n");
		return;
	}
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (sqlUpdatef(sid, "INSERT INTO gw_messages (messageid, obj_ctime, obj_mtime, sender, rcpt, status, subject, message) values (%d, '%s', '%s', %d, %d, 3, '%s', '%s')", messageid, curdate, curdate, sid->dat->user_uid, rcpt, str2sql(sid, subject), str2sql(sid, message))<0) return;
	logaccess(sid, 1, "%s - %s sent message %d", sid->dat->in_RemoteAddr, sid->dat->user_username, messageid);
	prints(sid, "<CENTER>Message sent</CENTER><BR>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "window.open('%s/messages/list','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
	prints(sid, "window.close('_top');\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}

void messagemain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_header(sid, "Groupware Messenger");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\nfunction SendMessage()\n{\n");
	prints(sid, "	window.open('%s/messages/send','msgwin2','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
	prints(sid, "}\n// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle BGCOLOR=%s>\n", COLOR_TMENU);
	prints(sid, "<TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
	prints(sid, "<A CLASS='TBAR' HREF=%s/messages/list>LIST MESSAGES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
	prints(sid, "<A CLASS='TBAR' HREF=javascript:SendMessage()>SEND MESSAGE</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>\n</TD></TR></TABLE>\n");
	prints(sid, "</CENTER><BR>\n");
	if (strncmp(sid->dat->in_RequestURI, "/messages/list", 14)==0)
		messagelist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/messages/read", 14)==0)
		messageread(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/messages/send", 14)==0)
		messagesend(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/messages/save", 14)==0) {
		messagesave(sid);
		messagelist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/delete", 16)==0) {
		messagestatus(sid, 0, '0');
		messagelist(sid);
	}
	htpage_footer(sid);
}
