/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_messages.h"

void htselect_groupfilter(CONN *sid, int groupid, char *baseuri)
{
	int i, j;
	SQLRES sqr;

	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=groupfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.groupfilter.groupid.options[document.groupfilter.groupid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=groupid onChange=\"go2()\">');\r\n");
	if (sql_queryf(&sqr, "SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d order by groupname ASC", sid->dat->user_did)<0) return;
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "document.write(\"<OPTION VALUE='%s%s?groupid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "'%s>%s\");\r\n", atoi(sql_getvalue(&sqr, i, 0))==groupid?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=groupid>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==groupid?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	prints(sid, "</TR></TABLE>\r\n");
	sql_freeresult(&sqr);
	return;
}

void messagestatus(CONN *sid, int messageid, char status)
{
	char timebuffer[100];
	char *ptemp;
	time_t t;
	SQLRES sqr;

	if (!(auth_priv(sid, "messages")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((messageid==0)&&((ptemp=getgetenv(sid, "MESSAGEID"))!=NULL)) {
		messageid=atoi(ptemp);
	}
	if (sql_queryf(&sqr, "SELECT messageid FROM gw_messages WHERE obj_uid = %d AND obj_did = %d AND messageid = %d", sid->dat->user_uid, sid->dat->user_did, messageid)<0) return;
	if (sql_numtuples(&sqr)<1) {
		sql_freeresult(&sqr);
		return;
	}
	sql_freeresult(&sqr);
	t=time(NULL);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (status=='0') {
		sql_updatef("DELETE FROM gw_messages WHERE obj_uid = %d AND obj_did = %d and messageid = %d", sid->dat->user_uid, sid->dat->user_did, messageid);
	} else {
		sql_updatef("UPDATE gw_messages SET obj_mtime = '%s', status = %c WHERE obj_uid = %d AND messageid = %d", timebuffer, status, sid->dat->user_uid, messageid);
	}
	return;
}

void messages_userlist(CONN *sid)
{
	char *ptemp;
	int i;
	SQLRES sqr;
	int groupid=-1;

	htpage_header(sid, "Null Messenger");
	if (!(auth_priv(sid, "messages")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	} else {
		groupid=sid->dat->user_gid;
	}
	htselect_groupfilter(sid, groupid, "/messages/userlist");
	if (sql_queryf(&sqr, "SELECT userid, username, givenname, surname FROM gw_users WHERE userid != %d AND groupid = %d AND domainid = %d ORDER BY username ASC", sid->dat->user_uid, groupid, sid->dat->user_did)<0) return;
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
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=left WIDTH=100%% STYLE='border-style:solid'>&nbsp;User&nbsp;</TH><TH ALIGN=left STYLE='border-style:solid'>&nbsp;History&nbsp;</TH></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP STYLE='cursor:hand; border-style:solid' onClick=\"SendMessage('%d', '%s')\" TITLE='%s %s'><A HREF=\"javascript:SendMessage('%d', '%s')\">%s</A>&nbsp;</TD>\r\n", atoi(sql_getvalue(&sqr, i, 0)), sql_getvalue(&sqr, i, 1), sql_getvalue(&sqr, i, 2), sql_getvalue(&sqr, i, 3), atoi(sql_getvalue(&sqr, i, 0)), sql_getvalue(&sqr, i, 1), sql_getvalue(&sqr, i, 1));
		prints(sid, "<TD STYLE='border-style:solid'>&nbsp;<A HREF=\"javascript:ViewHistory('%d', '%s')\">history</A>&nbsp;</TD></TR>\r\n", atoi(sql_getvalue(&sqr, i, 0)), sql_getvalue(&sqr, i, 1));
	}
	prints(sid, "</TABLE></CENTER>\r\n");
	sql_freeresult(&sqr);
	return;
}

void messages_frameset(CONN *sid)
{
	char *ptemp;
	char username[40];
	int userid=0;
	SQLRES sqr;

	if (!(auth_priv(sid, "messages")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE userid = %d", userid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(&sqr, 0, 1));
	sql_freeresult(&sqr);
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

void messages_white(CONN *sid)
{
	char *ptemp;
	char username[40];
	int userid=0;
	SQLRES sqr;

	if (!(auth_priv(sid, "messages")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE userid = %d", userid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(&sqr, 0, 1));
	sql_freeresult(&sqr);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>%s - Null Messenger</TITLE>\r\n", username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function doLoad() {\r\n");
	prints(sid, "	setTimeout(\"window.parent.msgwinbot_%s.document.location.replace(\\\"%s/messages/reload?userid=%d\\\")\", 500);\r\n", username, sid->dat->in_ScriptName, userid);
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "</HEAD>\r\n<BODY onload=\"doLoad();\" onresize=\"window.scrollTo(0, 99999);\">\r\n");
	prints(sid, "<B>Starting session with <FONT COLOR=red>%s</FONT>.</B><BR>\r\n", username);
	prints(sid, "<SPAN ID=text></SPAN>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void messages_send(CONN *sid)
{
	char *ptemp;
	char curdate[50];
	char message[1024];
	int userid=0;

	if (!(auth_priv(sid, "messages")&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if (strcmp(sid->dat->in_RequestMethod,"POST")==0) {
		memset(message, 0, sizeof(message));
		if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) snprintf(message, sizeof(message)-1, "%s", ptemp);
		if (strlen(message)>0) {
			time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
			if (sql_updatef("INSERT INTO gw_messages (obj_ctime, obj_mtime, obj_uid, obj_did, sender, rcpt, status, message) values ('%s', '%s', %d, %d, %d, %d, 3, '%s')", curdate, curdate, userid, sid->dat->user_did, sid->dat->user_uid, userid, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, message))<0) return;
			if (sql_updatef("INSERT INTO gw_messages (obj_ctime, obj_mtime, obj_uid, obj_did, sender, rcpt, status, message) values ('%s', '%s', %d, %d, %d, %d, 3, '%s')", curdate, curdate, sid->dat->user_uid, sid->dat->user_did, sid->dat->user_uid, userid, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, message))<0) return;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/messages/send?userid=%d NAME=msgcompose>\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "<TR CLASS=\"EDITFORM\">\r\n");
	prints(sid, "<TD WIDTH=100%%><INPUT TYPE=TEXT NAME=message WIDTH=50 STYLE='width:100%%'></TD>\r\n");
	prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Send VALUE='Send'></TD>\r\n");
	prints(sid, "</TR></FORM></TABLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.msgcompose.message.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}

void messages_reload(CONN *sid)
{
	char *ptemp;
	char username[40];
	int i;
	SQLRES sqr;
	int userid=0;
	int retry;

	if (!auth_priv(sid, "messages")&A_READ) return;
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE userid = %d", userid)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(&sqr, 0, 1));
	sql_freeresult(&sqr);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>%s - Null Messenger</TITLE>\r\n", username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function WriteTop(output) {\r\n");
	prints(sid, "	if (output!=\"\") {\r\n");
	prints(sid, "		window.parent.msgwintop_%s.document.getElementById('text').innerHTML+=output;\r\n", username);
	prints(sid, "		window.parent.msgwintop_%s.scrollTo(0, 99999);\r\n", username);
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	flushbuffer(sid);
	for (retry=0;retry<10;retry++) {
		if (sql_queryf(&sqr, "SELECT messageid, sender, rcpt, message FROM gw_messages WHERE obj_uid = %d AND obj_did = %d AND status > 2 AND (rcpt = %d OR sender = %d) ORDER BY obj_ctime, messageid ASC", sid->dat->user_uid, sid->dat->user_did, userid, userid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			for (i=0;i<sql_numtuples(&sqr);i++) {
				if (atoi(sql_getvalue(&sqr, i, 1))==sid->dat->user_uid) {
					if (tcp_fprintf(&sid->socket, "WriteTop(\"<B><FONT COLOR=black>%s: </FONT></B>%s<BR>\");\r\n", sid->dat->user_username, str2html(sid, sql_getvalue(&sqr, i, 3)))<0) break;
				} else {
					if (tcp_fprintf(&sid->socket, "WriteTop(\"<B><FONT COLOR=blue>%s: </FONT></B>%s<BR>\");\r\n", username, str2html(sid, sql_getvalue(&sqr, i, 3)))<0) break;
				}
				messagestatus(sid, atoi(sql_getvalue(&sqr, i, 0)), '2');
			}
			sql_freeresult(&sqr);
			prints(sid, "window.parent.msgwinmid_%s.document.msgcompose.message.focus();\r\n", username);
			break;
		}
		sql_freeresult(&sqr);
		sleep(1);
	}
//	prints(sid, "setTimeout(\"location.replace(\\\"%s/messages/reload?userid=%d\\\")\", 2000);\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "location.replace(\"%s/messages/reload?userid=%d\");\r\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "</HEAD>\r\n<BODY>\r\n</BODY>\r\n</HTML>\r\n");
	return;
}

void messages_history(CONN *sid)
{
	char *ptemp;
	char username[40];
	int i;
	SQLRES sqr;
	int userid=0;
	time_t lastdate;
	time_t msgdate;

	htpage_header(sid, "Null Messenger");
	if (!(auth_priv(sid, "messages")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	memset(username, 0, sizeof(username));
	if ((ptemp=getgetenv(sid, "userid"))!=NULL) userid=atoi(ptemp);
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE userid = %d AND domainid = %d", userid, sid->dat->user_did)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return;
	}
	snprintf(username, sizeof(username)-1, "%s", sql_getvalue(&sqr, 0, 1));
	sql_freeresult(&sqr);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function SendMessage(userid, username)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('%s/messages/frame?userid='+userid,'msgwin_'+username,'toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=200');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<BR><B>Message history for %s</B><BR>\r\n", username);
	if (sql_queryf(&sqr, "SELECT messageid, obj_ctime, sender, rcpt, status, message FROM gw_messages WHERE obj_uid = %d AND obj_did = %d AND status > 0 AND (rcpt = %d OR sender = %d) ORDER BY messageid ASC", sid->dat->user_uid, sid->dat->user_did, userid, userid)<0) return;
	if (sql_numtuples(&sqr)<1) {
		prints(sid, "<BR><B>No Messages</B><BR>\r\n");
		sql_freeresult(&sqr);
		return;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\r\n");
	lastdate=0;
	for (i=0;i<sql_numtuples(&sqr);i++) {
		msgdate=time_sql2unix(sql_getvalue(&sqr, i, 1));
		msgdate+=time_tzoffset(sid, msgdate);
		if (lastdate+86399<msgdate) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP><B>%s</B></TD></TR>\r\n", time_unix2datetext(sid, msgdate));
			lastdate=(int)(msgdate/86400)*86400;
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP VALIGN=TOP><A HREF=%s/messages/delete?userid=%d&messageid=%s>delete</A></TD>", sid->dat->in_ScriptName, userid, sql_getvalue(&sqr, i, 0));
		prints(sid, "<TD VALIGN=TOP WIDTH=100%%><B><FONT COLOR=%s>", atoi(sql_getvalue(&sqr, i, 2))==sid->dat->user_uid?"black":"blue");
		if (atoi(sql_getvalue(&sqr, i, 2))==sid->dat->user_uid) {
			prints(sid, "%s", str2html(sid, sid->dat->user_username));
		} else {
			prints(sid, "%s", str2html(sid, username));
		}
		prints(sid, " (%s): </FONT></B>%s</TD>", time_unix2timetext(sid, msgdate), str2html(sid, sql_getvalue(&sqr, i, 5)));
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE></CENTER>\r\n");
	sql_freeresult(&sqr);
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
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
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_messages",		// mod_name
		1,			// mod_submenu
		"MESSENGER",		// mod_menuname
		"messenger.png",	// mod_menupic
		"javascript:ListUsers();",// mod_menuuri
		"messages",		// mod_menuperm
		"mod_main",		// fn_name
		"/messages/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
