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

void recipientselect(int sid, char *selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQuery("SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints("<OPTION>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<OPTION");
		if (strcmp(sqlGetvalue(sqr, i, 1), selected)==0) prints(" SELECTED");
		prints(">%s\n", sqlGetvalue(sqr, i, 1));
	}
	sqlFreeconnect(sqr);
	return;
}

void messagestatus(int sid, int messageid, char status)
{
	char timebuffer[100];
	time_t t;
	int sqr;

	if (auth(sid, "messages")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((messageid==0)&&(getgetenv(sid, "MESSAGEID")!=NULL)) {
		messageid=atoi(getgetenv(sid, "MESSAGEID"));
	}
	if ((sqr=sqlQueryf("SELECT * FROM gw_messages WHERE msgto = '%s' and messageid = %d", conn[sid].dat->in_username, messageid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		return;
	}
	sqlFreeconnect(sqr);
	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	if (status=='0') {
		sqlUpdatef("DELETE FROM gw_messages WHERE msgto = '%s' and messageid = %d", conn[sid].dat->in_username, messageid);
		prints("<CENTER>Message deleted</CENTER><BR>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints("window.open('%s/messagelist','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName);
		if (strncmp(conn[sid].dat->in_RequestURI,"/messagedelete2",15)==0)
			prints("window.close('_top');\n");
		prints("// -->\n</SCRIPT>\n");
	} else {
		sqlUpdatef("UPDATE gw_messages SET mdatetime = '%s', msgstatus = %c WHERE messageid = %d", timebuffer, status, messageid);
	}
	return;
}

void messageautocheck(int sid)
{
	char posttime[100];
	char posttime2[100];
	struct timeval ttime;
	struct timezone tzone;
	int isreminder=0;
	int i;
	int a, b;
	int sqr;

	gettimeofday(&ttime, &tzone);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", localtime(&ttime.tv_sec));
	ttime.tv_sec+=86400*7;
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d", localtime(&ttime.tv_sec));
	if (auth(sid, "calendar")>0) {
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf("SELECT eventdate, reminder FROM gw_events where eventdate >= #%s# and eventdate < #%s# and assignedto like '%s' and reminder > 0 ORDER BY eventdate ASC", posttime, posttime2, conn[sid].dat->in_username))<0) return;
		} else {
			if ((sqr=sqlQueryf("SELECT eventdate, reminder FROM gw_events where eventdate >= '%s' and eventdate < '%s' and assignedto like '%s' and reminder > 0 ORDER BY eventdate ASC", posttime, posttime2, conn[sid].dat->in_username))<0) return;
		}
		gettimeofday(&ttime, &tzone);
		for (i=0;i<sqlNumtuples(sqr);i++) {
//			a=sqldatetime2unix(sqlGetvalue(sqr, i, 0))-ttime.tv_sec+tzone.tz_minuteswest*60;
			a=sqldatetime2unix(sqlGetvalue(sqr, i, 0))-sqldatetime2unix(posttime);
			b=a-atoi(sqlGetvalue(sqr, i, 1))*60;
			if ((a>0)&&(b<0)) isreminder=1;
		}
		if (isreminder) {
			prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints("window.open('%s/calendarreminders','calendarwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", conn[sid].dat->in_ScriptName);
			prints("// -->\n</SCRIPT>\n");
		}
		sqlFreeconnect(sqr);
	}
	if (auth(sid, "messages")>0) {
		if ((sqr=sqlQueryf("SELECT messageid FROM gw_messages WHERE msgto = '%s' and msgstatus = 2", conn[sid].dat->in_username))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints("window.open('%s/messagelist','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName);
			prints("// -->\n</SCRIPT>\n");
		}
		sqlFreeconnect(sqr);
	}
	return;
}

void messagelist(int sid)
{
	int i;
	int sqr;

	if (auth(sid, "messages")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQueryf("SELECT * FROM gw_messages WHERE msgto = '%s' and msgstatus > 0 ORDER BY messageid DESC", conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		prints("<CENTER>You have no messages.</CENTER><BR>\n");
		return;
	}
	prints("<SCRIPT LANGUAGE=JavaScript>\n");
	prints("<!--\n");
	prints("function ReadMessage(msg)\n");
	prints("{\n");
	prints("	window.open('%s/messageread?messageid='+msg,'msgwin2','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');", conn[sid].dat->in_ScriptName);
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TH>&nbsp;</TH><TH ALIGN=left>From</TH><TH ALIGN=left WIDTH=100%%>Subject</TH></TR>\n", COLOR_TRIM);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (atoi(sqlGetvalue(sqr, i, 5))<1) continue;
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD NOWRAP><A HREF=%s/messagedelete?messageid=%s>delete</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints("<TD NOWRAP>%s</TD>", sqlGetvalue(sqr, i, 3));
		prints("<TD NOWRAP WIDTH=100%%><A HREF=javascript:ReadMessage(%d)>", atoi(sqlGetvalue(sqr, i, 0)));
		if (strlen(sqlGetvalue(sqr, i, 6))==0) {
			prints("(No Subject)</A></TD>");
		} else {
			prints("%s</A></TD>", sqlGetvalue(sqr, i, 6));
		}
		prints("</TR>\n");
	}
	prints("</TABLE></CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void messageread(int sid)
{
	int messageid;
	int sqr;

	if (auth(sid, "messages")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "MESSAGEID")==NULL) return;
	messageid=atoi(getgetenv(sid, "MESSAGEID"));
	if ((sqr=sqlQueryf("SELECT * FROM gw_messages WHERE msgto = '%s' and messageid = %d", conn[sid].dat->in_username, messageid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching message found for %d</CENTER>\n", messageid);
		sqlFreeconnect(sqr);
		return;
	}
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR><TD BGCOLOR=%s WIDTH=50><B>From</B></TD>", COLOR_FNAME);
	prints("<TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s</TD></TR>\n", COLOR_FTEXT, sqlGetvalue(sqr, 0, 3));
	prints("<TR><TD BGCOLOR=%s WIDTH=50><B>Subject</B></TD>", COLOR_FNAME);
	if (strlen(sqlGetvalue(sqr, 0, 6))==0) {
		prints("<TD BGCOLOR=%s NOWRAP WIDTH=100%%>(No Subject)</TD></TR>\n", COLOR_FTEXT);
	} else {
		prints("<TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", COLOR_FTEXT, sqlGetvalue(sqr, 0, 6));
	}
	prints("<TR><TD BGCOLOR=%s COLSPAN=2>%s&nbsp;</TD></TR></TABLE>\n", COLOR_FTEXT, sqlGetvalue(sqr, 0, 7));
	prints("[<A HREF=%s/messagesend?replyto=%d style='TEXT-DECORATION: none'>Reply</A>]&nbsp;", conn[sid].dat->in_ScriptName, messageid);
	prints("[<A HREF=%s/messagedelete2?messageid=%d style='TEXT-DECORATION: none'>Delete</A>]\n", conn[sid].dat->in_ScriptName, messageid);
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	messagestatus(sid, messageid, '1');
	return;
}

void messagesend(int sid)
{
	char info[2][50];
	int replyto=0;
	int sqr;

	if (auth(sid, "messages")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(info[0], 0, sizeof(info[0]));
	memset(info[1], 0, sizeof(info[1]));
	if (getgetenv(sid, "REPLYTO")!=NULL)
		replyto=atoi(getgetenv(sid, "REPLYTO"));
	if (replyto>0) {
		if ((sqr=sqlQueryf("SELECT * FROM gw_messages WHERE msgto = '%s' and messageid = %d", conn[sid].dat->in_username, replyto))<0) return;
		if (sqlNumfields(sqr)<1) {
			sqlFreeconnect(sqr);
			return;
		}
		snprintf(info[0], sizeof(info[0])-1, "%s", sqlGetvalue(sqr, 0, 3));
		snprintf(info[1], sizeof(info[1])-1, "Re: %s", sqlGetvalue(sqr, 0, 6));
		sqlFreeconnect(sqr);
	}
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/messagesave NAME=msgcompose>\n", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD><B>To</B></TD><TD><SELECT NAME=msgto>\n", COLOR_EDITFORM);
	recipientselect(sid, info[0]);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Subject</B></TD><TD><INPUT TYPE=TEXT NAME=msgsubject VALUE='%s' SIZE=30></TD></TR>\n", COLOR_EDITFORM, info[1]);
	prints("<TR BGCOLOR=%s>\n", COLOR_EDITFORM);
	prints("<TD ALIGN=CENTER COLSPAN=4><TEXTAREA WRAP=virtual NAME=message ROWS=5 COLS=42></TEXTAREA></TD>\n");
	prints("</TR></TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=Send VALUE='Send'></FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.msgcompose.msgto.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void messagesave(int sid)
{
	char info[2][50];
	char message[1024];
	int messageid;
	char timebuffer[100];
	time_t t;
	int sqr;

	if (auth(sid, "messages")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT max(messageid) FROM gw_messages"))<0) return;
	messageid=atoi(sqlGetvalue(sqr, 0, 0))+1;
	if (messageid<1) messageid=1;
	sqlFreeconnect(sqr);
	if (getpostenv(sid, "MSGTO")==NULL) return;
	if (getpostenv(sid, "MSGSUBJECT")==NULL) return;
	if (getpostenv(sid, "MESSAGE")==NULL) return;
	memset(info[0], 0, sizeof(info[0]));
	memset(info[1], 0, sizeof(info[1]));
	memset(message, 0, sizeof(message));
	strncpy(info[0], getpostenv(sid, "MSGTO"), sizeof(info[0])-1);
	strncpy(info[1], getpostenv(sid, "MSGSUBJECT"), sizeof(info[1])-1);
	if (strlen(info[0])==0) {
		prints("<CENTER>No recipient was specified</CENTER><BR>\n");
		return;
	}
	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	strncpy(message, str2sql(getpostenv(sid, "MESSAGE")), sizeof(message)-1);
	if (sqlUpdatef("INSERT INTO gw_messages (messageid, cdatetime, mdatetime, msgfrom, msgto, msgstatus, msgsubject, msgtext) values (%d, '%s', '%s', '%s', '%s', 2, '%s', '%s')", messageid, timebuffer, timebuffer, conn[sid].dat->in_username, info[0], info[1], message)<0) return;
	logaccess(1, "%s - %s sent message %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, messageid);
	prints("<CENTER>Message sent</CENTER><BR>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("	window.open('%s/messagelist','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName);
	prints("	window.close('_top');\n");
	prints("// -->\n</SCRIPT>\n");
	return;
}

void messagemain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Messenger");
	prints("<SCRIPT LANGUAGE=JavaScript>\n");
	prints("<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch08-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("function SendMessage()\n");
	prints("{\n");
	prints("	window.open('%s/messagesend','msgwin2','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName);
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<A CLASS='TBAR' HREF=%s/messagelist>LIST MESSAGES</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:SendMessage()>SEND MESSAGE</a>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>\n");
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE><BR>\n");
	prints("<CENTER><B><FONT COLOR=#808080 SIZE=3>Sentinel Messenger</FONT></B></CENTER><BR>\n");
	if (strncmp(conn[sid].dat->in_RequestURI, "/messagelist", 12)==0)
		messagelist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/messageread", 12)==0)
		messageread(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/messagesend", 12)==0)
		messagesend(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/messagesave", 12)==0) {
		messagesave(sid);
		messagelist(sid);
	} else if (strncmp(conn[sid].dat->in_RequestURI,"/messagedelete",14)==0) {
		messagestatus(sid, 0, '0');
		messagelist(sid);
	}
	printfooter(sid);
}
