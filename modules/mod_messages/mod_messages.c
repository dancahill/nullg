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
#include "mod_stub.h"

#ifdef WIN32
unsigned sleep(unsigned seconds)
{
	Sleep(1000*seconds);
	return 0;
}
#else
#include <unistd.h>
#endif

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
	if ((sqr=sql_queryf(sid, "SELECT * FROM gw_messages WHERE obj_uid = %d AND messageid = %d", sid->dat->user_uid, messageid))<0) return;
	if (sql_numtuples(sqr)<1) {
		sql_freeresult(sqr);
		return;
	}
	sql_freeresult(sqr);
	t=time(NULL);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status=='0') {
		sql_updatef(sid, "DELETE FROM gw_messages WHERE obj_uid = %d and messageid = %d", sid->dat->user_uid, messageid);
	} else {
		sql_updatef(sid, "UPDATE gw_messages SET obj_mtime = '%s', status = %c WHERE obj_uid = %d AND messageid = %d", timebuffer, status, sid->dat->user_uid, messageid);
	}
	return;
}

void messages_userlist(CONNECTION *sid)
{
	int i;
	int sqr;

	htpage_header(sid, "Null Messenger");
	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_queryf(sid, "SELECT userid, username, givenname, surname FROM gw_users WHERE userid != %d ORDER BY username ASC", sid->dat->user_uid))<0) return;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ViewHistory(userid, username)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('%s/messages/history?userid='+userid,'hiswin_'+username,'toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=300');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "function SendMessage(userid, username)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('%s/messages/frame?userid='+userid,'msgwin_'+username,'toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=200');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=left WIDTH=100%%><FONT COLOR=%s>&nbsp;User&nbsp;</FONT></TH><TH ALIGN=left><FONT COLOR=%s>&nbsp;History&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext);
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP STYLE='cursor:hand' onClick=\"SendMessage('%d', '%s')\" TITLE='%s %s'><A HREF=\"javascript:SendMessage('%d', '%s')\">%s</A>&nbsp;</TD>\n", config->colour_fieldval, atoi(sql_getvalue(sqr, i, 0)), sql_getvalue(sqr, i, 1), sql_getvalue(sqr, i, 2), sql_getvalue(sqr, i, 3), atoi(sql_getvalue(sqr, i, 0)), sql_getvalue(sqr, i, 1), sql_getvalue(sqr, i, 1));
		prints(sid, "<TD>&nbsp;<A HREF=\"javascript:ViewHistory('%d', '%s')\">history</A>&nbsp;</TD></TR>\n", atoi(sql_getvalue(sqr, i, 0)), sql_getvalue(sqr, i, 1));
	}
	prints(sid, "</TABLE></CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void messages_frameset(CONNECTION *sid)
{
	char *ptemp;
	char username[40];
	int userid=0;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Frameset//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>%s - Null Messenger</TITLE>\r\n", username);
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<FRAMESET ROWS=\"*,22,0\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n");
	prints(sid, "<FRAME BORDER=0 NAME=\"msgwintop_%s\" SRC=%s/messages/white?userid=%d MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=YES>\r\n", username, sid->dat->in_ScriptName, userid);
	prints(sid, "<FRAME BORDER=0 NAME=\"msgwinmid_%s\" SRC=%s/messages/send?userid=%d MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", username, sid->dat->in_ScriptName, userid);
	prints(sid, "<FRAME BORDER=0 NAME=\"msgwinbot_%s\" SRC=%s/messages/null?userid=%d MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", username, sid->dat->in_ScriptName, userid);
	prints(sid, "</FRAMESET>\r\n");
	prints(sid, "To view this page, you need a web browser capable of displaying frames.\r\n");
	prints(sid, "</HTML>\r\n");
}

void messages_white(CONNECTION *sid)
{
	char *ptemp;
	char username[40];
	int userid=0;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>%s - Null Messenger</TITLE>\r\n", username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function doLoad() {\r\n");
	prints(sid, "	setTimeout(\"window.parent.msgwinbot_%s.document.location.replace(\\\"%s/messages/reload?userid=%d\\\")\", 500);\n", username, sid->dat->in_ScriptName, userid);
	prints(sid, "}\r\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "</HEAD>\r\n<BODY onload=\"doLoad();\" onresize=\"window.scrollTo(0, 99999);\">\r\n");
	prints(sid, "<B>Starting session with <FONT COLOR=red>%s</FONT>.</B><BR>\r\n", username);
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void messages_send(CONNECTION *sid)
{
	char *ptemp;
	char curdate[50];
	char message[1024];
	char username[40];
	int messageid;
	int userid=0;
	int sqr;

	if (!(auth_priv(sid, AUTH_MESSAGES)&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	if (strcmp(sid->dat->in_RequestMethod,"POST")==0) {
		if ((sqr=sql_query(sid, "SELECT max(messageid) FROM gw_messages"))<0) return;
		messageid=atoi(sql_getvalue(sqr, 0, 0))+1;
		if (messageid<1) messageid=1;
		sql_freeresult(sqr);
		memset(message, 0, sizeof(message));
		if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) snprintf(message, sizeof(message)-1, "%s", ptemp);
		if (strlen(message)>0) {
			snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
			if (sql_updatef(sid, "INSERT INTO gw_messages (messageid, obj_ctime, obj_mtime, obj_uid, sender, rcpt, status, message) values (%d, '%s', '%s', %d, %d, %d, 3, '%s')", messageid, curdate, curdate, userid, sid->dat->user_uid, userid, str2sql(sid, message))<0) return;
			if (sql_updatef(sid, "INSERT INTO gw_messages (messageid, obj_ctime, obj_mtime, obj_uid, sender, rcpt, status, message) values (%d, '%s', '%s', %d, %d, %d, 3, '%s')", messageid+1, curdate, curdate, sid->dat->user_uid, sid->dat->user_uid, userid, str2sql(sid, message))<0) return;
//			db_log_activity(sid, 1, "messages", messageid, "insert", "%s - %s sent message %d", sid->dat->in_RemoteAddr, sid->dat->user_username, messageid);
		}
	}
//	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
//	prints(sid, "function doupdate() {\n");
//	prints(sid, "	window.parent.msgwinbot_%s.location.replace(\"%s/messages/reload?userid=%d\");\n", username, sid->dat->in_ScriptName, userid);
//	prints(sid, "}\n");
//	prints(sid, "function updates() {\n");
//	prints(sid, "	doupdate();\n");
//	prints(sid, "	setTimeout(\"updates()\", 60000);\n");
//	prints(sid, "}\n");
//	prints(sid, "setTimeout(\"updates()\", 60000);\n");
//	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/messages/send?userid=%d NAME=msgcompose>\n", sid->dat->in_ScriptName, userid);
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_editform);
	prints(sid, "<TD WIDTH=100%%><INPUT TYPE=TEXT NAME=message WIDTH=50 STYLE='width:100%%'></TD>\n");
	prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Send VALUE='Send'></TD>\n");
	prints(sid, "</TR></FORM></TABLE>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.msgcompose.message.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void messages_reload(CONNECTION *sid)
{
	char *ptemp;
	char username[40];
	int i;
	int sqr;
	int userid=0;
	int retry;

	if (!auth_priv(sid, AUTH_MESSAGES)&A_READ) return;
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>%s - Null Messenger</TITLE>\r\n", username);
//	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL=%s/messages/reload?userid=%d\">\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function WriteTop(output) {\r\n");
	prints(sid, "	if (output!=\"\") {\r\n");
	prints(sid, "		if (navigator.userAgent.indexOf('MSIE')>-1) {\r\n");
	prints(sid, "			window.parent.msgwintop_%s.document.body.insertAdjacentHTML(\"BeforeEnd\", output);\r\n", username);
	prints(sid, "		} else if (navigator.userAgent.indexOf('Opera')>-1) {\r\n");
	prints(sid, "			window.parent.msgwintop_%s.document.body.insertAdjacentHTML(\"BeforeEnd\", output);\r\n", username);
	prints(sid, "		} else {\r\n");
	prints(sid, "			window.parent.msgwintop_%s.document.writeln(output);\r\n", username, sid->dat->user_username);
	prints(sid, "		}\r\n");
	prints(sid, "		window.parent.msgwintop_%s.scrollTo(0, 99999);\r\n", username);
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
//	prints(sid, "// -->\n</SCRIPT>\n");
	for (retry=0;retry<5;retry++) {
		if ((sqr=sql_queryf(sid, "SELECT messageid, sender, rcpt, message FROM gw_messages WHERE obj_uid = %d AND status > 2 AND (rcpt = %d OR sender = %d) ORDER BY obj_ctime, messageid ASC", sid->dat->user_uid, userid, userid))<0) return;
		if (sql_numtuples(sqr)>0) {
//			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");

//			prints(sid, "setTimeout(\"\", 1000);\n");

			for (i=0;i<sql_numtuples(sqr);i++) {
				if (atoi(sql_getvalue(sqr, i, 1))==sid->dat->user_uid) {
					prints(sid, "WriteTop(\"<B><FONT COLOR=black>%s: </FONT></B>%s<BR>\");\r\n", sid->dat->user_username, str2html(sid, sql_getvalue(sqr, i, 3)));
				} else {
					prints(sid, "WriteTop(\"<B><FONT COLOR=blue>%s: </FONT></B>%s<BR>\");\r\n", username, str2html(sid, sql_getvalue(sqr, i, 3)));
				}
				messagestatus(sid, atoi(sql_getvalue(sqr, i, 0)), '2');
			}
			sql_freeresult(sqr);
			prints(sid, "window.focus();\r\n");
			prints(sid, "window.parent.msgwinmid_%s.document.msgcompose.message.focus();\r\n", username);
			break;
//			prints(sid, "setTimeout(\"location.replace(\\\"%s/messages/reload?userid=%d\\\")\", 1000);\n", sid->dat->in_ScriptName, userid);
//			prints(sid, "// -->\n</SCRIPT>\n");
//			prints(sid, "</HEAD>\r\n<BODY>\r\n</BODY>\r\n</HTML>\r\n");
//			return;
		}
		sql_freeresult(sqr);
		sleep(1);
	}
//	if (sql_numtuples(sqr)>0) prints(sid, "window.parent.msgwinmid_%s.document.msgcompose.message.focus();\r\n", username);
	prints(sid, "setTimeout(\"location.replace(\\\"%s/messages/reload?userid=%d\\\")\", 2000);\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "</HEAD>\r\n<BODY>\r\n</BODY>\r\n</HTML>\r\n");
	return;
}

void messages_history(CONNECTION *sid)
{
	char *ptemp;
	char username[40];
	int i;
	int sqr;
	int userid=0;
	time_t msgdate;

	htpage_header(sid, "Null Messenger");
	if (!(auth_priv(sid, AUTH_MESSAGES)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function SendMessage(userid, username)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('%s/messages/frame?userid='+userid,'msgwin_'+username,'toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=200');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<BR><B>Message history for %s</B><BR><BR>\n", username);
	if ((sqr=sql_queryf(sid, "SELECT messageid, obj_ctime, sender, rcpt, status, message FROM gw_messages WHERE obj_uid = %d AND status > 0 AND (rcpt = %d OR sender = %d) ORDER BY messageid DESC", sid->dat->user_uid, userid, userid))<0) return;
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<B>No Messages</B><BR>\n");
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH>&nbsp;</TH><TH ALIGN=left WIDTH=100%%><FONT COLOR=%s>&nbsp;Message&nbsp;</FONT></TH><TH ALIGN=left><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext);
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		prints(sid, "<TD NOWRAP VALIGN=TOP><A HREF=%s/messages/delete?userid=%d&messageid=%s>delete</A></TD>", sid->dat->in_ScriptName, userid, sql_getvalue(sqr, i, 0));
		if (atoi(sql_getvalue(sqr, i, 2))==sid->dat->user_uid) {
			prints(sid, "<TD VALIGN=TOP WIDTH=100%%><B><FONT COLOR=black>%s: </FONT></B>%s</TD>", sid->dat->user_username, str2html(sid, sql_getvalue(sqr, i, 5)));
		} else {
			prints(sid, "<TD VALIGN=TOP WIDTH=100%%><B><FONT COLOR=blue>%s: </FONT></B>%s</TD>", username, str2html(sid, sql_getvalue(sqr, i, 5)));
		}
		msgdate=time_sql2unix(sql_getvalue(sqr, i, 1));
		msgdate+=time_tzoffset(sid, msgdate);
		prints(sid, "<TD NOWRAP VALIGN=TOP>%s %s</TD>", time_unix2datetext(sid, msgdate), time_unix2timetext(sid, msgdate));
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE></CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void mod_main(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/messages/userlist", 18)==0) {
		messages_userlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/frame", 15)==0) {
		messages_frameset(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/send", 14)==0) {
		messages_send(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/reload", 16)==0) {
		messages_reload(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/history", 17)==0) {
		messages_history(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/white", 15)==0) {
		messages_white(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/messages/delete", 16)==0) {
		messagestatus(sid, 0, '0');
		messages_history(sid);
	}
}

DllExport int mod_init(CONFIG *cfg, FUNCTION *fns, MODULE_MENU *menu, MODULE_FUNC *func)
{
	config=cfg;
	functions=fns;
	mod_menuitems=menu;
	mod_functions=func;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_messages", "MESSENGER", "javascript:ListUsers()", "mod_main", "/messages/", mod_main)!=0) return -1;
	return 0;
}
