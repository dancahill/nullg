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
#include "main.h"

void htpage_header(CONN *sid, char *title)
{
	HTMOD_HTML_HEADER mod_html_header;

	if ((mod_html_header=module_call(sid, "mod_html_header"))!=NULL) {
		mod_html_header(sid, title);
		return;
	}
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n");
	prints(sid, "<HEAD>\r\n");
	prints(sid, "<TITLE>%s</TITLE>\r\n", title);
//	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=us-ascii\">\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=iso-8859-1\">\r\n");
	prints(sid, "<STYLE TYPE=text/css>\r\n");
	prints(sid, ".JUSTIFY { text-align: justify; }\r\n");
	prints(sid, ".TBAR    { color: #505050; text-decoration: none; font-family: Geneva, Arial,Verdana; font-size: 8pt; }\r\n");
	prints(sid, "A        { text-decoration: none; }\r\n");
	prints(sid, "A:HOVER  { text-decoration: underline; }\r\n");
	prints(sid, "TD       { color: #000000; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n");
	prints(sid, "TH       { background-color: 0000A0; color: FFFFFF; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n");
	prints(sid, "</STYLE>\r\n");
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/%s/themes/%s/style.css\">\r\n", SERVER_BASENAME, sid->dat->user_theme);
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<BODY BACKGROUND=\"/%s/themes/%s/bgmain.gif\" BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\"", SERVER_BASENAME, sid->dat->user_theme);
	prints(sid, " TOPMARGIN=0 LEFTMARGIN=0 MARGINHEIGHT=0 MARGINWIDTH=0 CLASS=\"MAINBACK\">\r\n", SERVER_BASENAME);
}

void htpage_footer(CONN *sid)
{
	HTMOD_HTML_FOOTER mod_html_footer;

	if ((mod_html_footer=module_call(sid, "mod_html_footer"))!=NULL) {
		mod_html_footer(sid);
		return;
	}
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void htpage_login(CONN *sid)
{
	HTMOD_HTML_LOGIN mod_html_login;
	char pageuri[200];
	char username[32];
	char password[32];
	char domain[64];
	char *ptemp;
	int sqr;

	if ((mod_html_login=module_call(sid, "mod_html_login"))!=NULL) {
		mod_html_login(sid);
		return;
	}
	memset(pageuri, 0, sizeof(pageuri));
	memset(domain, 0, sizeof(domain));
	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s%s", sid->dat->in_ScriptName, sid->dat->in_RequestURI);
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_header(sid, SERVER_NAME);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		prints(sid, "<FONT COLOR=RED><B>Incorrect username or password</B></FONT>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=\"%s/\" AUTOCOMPLETE=OFF NAME=login>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=pageuri VALUE=\"%s\">\r\n", pageuri);
	prints(sid, "<TR><TH COLSPAN=2 STYLE='padding:1px'>%s Login</TH></TR>\r\n", SERVER_NAME);
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	if ((sqr=sql_query("SELECT username, password FROM gw_users WHERE userid = 1"))<0) return;
	if (sql_numtuples(sqr)==1) {
		if (strcmp("$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/", sql_getvalue(sqr, 0, 1))==0) {
			snprintf(username, sizeof(username)-1, "%s", sql_getvalue(sqr, 0, 0));
			snprintf(password, sizeof(password)-1, "visual");
		}
	}
	sql_freeresult(sqr);
	if (strlen(username)==0) {
		snprintf(username, sizeof(username)-1, "%s", sid->dat->user_username);
	}
	if (strlen(domain)==0) {
		if (strlen(sid->dat->user_domainname)==0) {
			snprintf(domain, sizeof(domain)-1, "%s", sid->dat->in_Host);
		} else {
			snprintf(domain, sizeof(domain)-1, "%s", sid->dat->user_domainname);
		}
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=username SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=password SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", password);
	if ((sqr=sql_query("SELECT COUNT(*) FROM gw_domains"))<0) return;
	if (atoi(sql_getvalue(sqr, 0, 0))>1) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Domain&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=domain SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", domain);
	} else {
		prints(sid, "<INPUT TYPE=hidden NAME=domain VALUE=\"NULL\">\r\n");
	}
	sql_freeresult(sqr);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\r\n");
	prints(sid, "</FORM>\r\n</TABLE>\r\n");
	if (strcmp(password, "visual")==0) {
		prints(sid, "<BR><TABLE>\r\n");
		prints(sid, "<TR><TD><FONT COLOR=RED><PRE>\r\n");
		prints(sid, "You have not yet changed the administrator's password.\r\n");
		prints(sid, "Please do this as soon as possible for security reasons.\r\n");
		prints(sid, "</PRE></FONT></TD></TR>\r\n");
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "</CENTER>\r\n<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	if (strlen(username)>0) {
		prints(sid, "document.login.password.focus();\r\n");
	} else {
		prints(sid, "document.login.username.focus();\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n</BODY>\r\n</HTML>\r\n");
	return;
}

void htpage_logout(CONN *sid)
{
	HTMOD_HTML_LOGOUT mod_html_logout;

	if ((mod_html_logout=module_call(sid, "mod_html_logout"))!=NULL) {
		mod_html_logout(sid);
		return;
	}
	db_log_activity(sid, 0, "login", 0, "logout", "%s - Logout: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_header(sid, SERVER_NAME);
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TH>%s Logout</TH></TR>\r\n", SERVER_NAME);
	prints(sid, "<TR CLASS=FIELDVAL><TD>\r\n");
	prints(sid, "You have successfully logged out.\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('%s/');\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='%s/'\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void htpage_frameset(CONN *sid)
{
	HTMOD_HTML_FRAMESET mod_html_frameset;

	if ((mod_html_frameset=module_call(sid, "mod_html_frameset"))!=NULL) {
		mod_html_frameset(sid);
		return;
	}
	htpage_motd(sid);
}

void htpage_topmenu(CONN *sid, int menu)
{
	HTMOD_HTML_TOPMENU mod_html_topmenu;
	char *ptemp;
	int dot=0;
	int folderid=0;
	int userid=-1;
	int groupid=-1;
	int status=0;
	int i;

	if ((mod_html_topmenu=module_call(sid, "mod_html_topmenu"))!=NULL) {
		mod_html_topmenu(sid, menu);
		return;
	}
	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	}
	htpage_header(sid, SERVER_NAME);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	if ((menu==MENU_CONTACTS)||(menu==MENU_SEARCHES)) {
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	} else if (menu==MENU_FILES) {
		prints(sid, "function FileUpload() {\r\n");
		if ((ptemp=getgetenv(sid, "LOCATION"))==NULL) ptemp=sid->dat->in_RequestURI;
		prints(sid, "	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');\r\n", sid->dat->in_ScriptName, ptemp);
		prints(sid, "}\r\n");
	} else if (menu==MENU_WEBMAIL) {
		prints(sid, "function ComposeMail() {\r\n");
		prints(sid, "	window.open('%s/mail/write','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	}
	if (sid->dat->user_menustyle==0) {
		prints(sid, "function ListUsers() {\n");
		prints(sid, "	window.open('%s/messages/userlist','msguserlist','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=200,height=300');\n", sid->dat->in_ScriptName);
		prints(sid, "}\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if ((menu==MENU_WEBMAIL)&&(sid->dat->user_menustyle>0)) {
		if (strncmp(sid->dat->in_RequestURI, "/mail/accounts", 14)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/filters", 13)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/folders", 13)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/list", 10)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/purge", 11)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/quit", 10)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/sync", 10)==0) goto domenu;
		return;
	}
domenu:
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	if (sid->dat->user_menustyle==0) {
		prints(sid, "<TR><TD STYLE='border-style:solid'><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle CLASS=\"TBAR\"><TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
		for (i=0;;i++) {
			if (strlen(http_proc.mod_menuitems[i].mod_name)==0) break;
			if (strlen(http_proc.mod_menuitems[i].mod_menuname)==0) continue;
			if (strlen(http_proc.mod_menuitems[i].mod_menuperm)!=0) {
				if (!auth_priv(sid, http_proc.mod_menuitems[i].mod_menuperm)) continue;
			}
			prints(sid, "%s<A CLASS='TBAR' HREF=", dot?"&nbsp;&middot;&nbsp;":"");
			if (strncasecmp(http_proc.mod_menuitems[i].mod_menuuri, "javascript:", 11)==0) {
				prints(sid, "%s", http_proc.mod_menuitems[i].mod_menuuri);
			} else {
				prints(sid, "\"%s%s\"", sid->dat->in_ScriptName, http_proc.mod_menuitems[i].mod_menuuri);
			}
			prints(sid, ">%s</A>", http_proc.mod_menuitems[i].mod_menuname);
			dot=1;
		}
		prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
		if (module_exists(sid, "mod_profile")) {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, MENU_LOGOUT);
		prints(sid, "&nbsp;&nbsp;</TD>\r\n");
		prints(sid, "</TR></TABLE>\r\n</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD STYLE='border-style:solid'><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle CLASS=\"TBAR\">\r\n");
	prints(sid, "<TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
	switch (menu) {
		case MENU_ADMIN:
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/configedit>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_CONFIG);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/activitylist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_LOGS);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/domainlist>DOMAINS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/status>STATUS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/syscheck>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_CHECK);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/userlist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_USERS);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/grouplist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_GROUPS);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/zonelist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_ZONES);
			break;
		case MENU_BOOKMARKS:
			if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) folderid=atoi(ptemp);
			prints(sid, "<A CLASS='TBAR' HREF=%s/bookmarks/list>LIST BOOKMARKS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/bookmarks/editnew?folder=%d>NEW BOOKMARK</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, folderid);
			prints(sid, "<A CLASS='TBAR' HREF=%s/bookmarks/foldereditnew?parent=%d>NEW FOLDER</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, folderid);
			break;
		case MENU_CALENDAR:
		case MENU_TASKS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/list?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">TODAY</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/wlist?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">WEEK</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/mlist?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">MONTH</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/ylist?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">YEAR</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/availmap?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">AVAILMAP</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/editnew>NEW EVENT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/tasks/editnew>NEW TASK</a>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_CALLS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/calls/list>CALLS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calls/editnew>NEW CALL</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_CONTACTS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/list>CONTACTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			if (strncmp(sid->dat->in_RequestURI, "/contacts/search", 16)==0) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/viewnew2>NEW CONTACT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/search1>SEARCH</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/viewnew>NEW CONTACT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_FILES:
			if ((ptemp=getgetenv(sid, "LOCATION"))==NULL) ptemp=sid->dat->in_RequestURI;
			prints(sid, "<A CLASS='TBAR' HREF=%s", sid->dat->in_ScriptName);
			printhex(sid, "%s", ptemp);
			prints(sid, ">REFRESH</A>&nbsp;&middot;&nbsp;");
			if (auth_priv(sid, "files")&A_MODIFY) {
				prints(sid, "<A CLASS='TBAR' HREF=javascript:FileUpload()>UPLOAD FILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_RequestURI);
				prints(sid, "<A CLASS='TBAR' HREF=%s/filemkdir?location=", sid->dat->in_ScriptName);
				printhex(sid, "%s", ptemp);
				prints(sid, ">NEW FOLDER</A>&nbsp;&middot;&nbsp;");
			}
			break;
		case MENU_FORUMS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/forums/list>LIST FORUMS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			if (auth_priv(sid, "forums")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/grouplist>GROUPS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/post>NEW FORUM</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_MAIN:
			if (module_exists(sid, "mod_html")) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/frames/motd>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_NOTES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/list>LIST NOTES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/editnew>NEW NOTE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_ORDERS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/orders/list>ORDERS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/orders/productlist>PRODUCTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_PROFILE:
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>EDIT PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_SEARCHES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/search/>SEARCHES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/search/sqlrun>SQL QUERIES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_WEBMAIL:
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/list%s>INBOX</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=wmlist":"");
			if (sid->dat->user_menustyle>0) {
				prints(sid, "<A CLASS='TBAR' HREF=javascript:ComposeMail()>COMPOSE</A>&nbsp;&middot;&nbsp;");
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/mail/write>COMPOSE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/sync>SEND/RECV</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/accounts/list>ACCOUNTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/quit%s>QUIT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=gwmain":"");
			break;
	}
	prints(sid, "<A CLASS='TBAR' HREF=javascript:window.print()>%s</A>", MENU_PRINT);
	prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
	if (sid->dat->user_menustyle>0) {
		if (module_exists(sid, "mod_profile")) {
			if (menu==MENU_WEBMAIL) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit TARGET=gwmain>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, MENU_LOGOUT);
	}
	prints(sid, "&nbsp;&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
//	flushbuffer(sid);
}

void htpage_motd(CONN *sid)
{
	HTMOD_HTML_MOTD mod_html_motd;
	char showtime[32];
	time_t t;
	int sqr;

	if ((mod_html_motd=module_call(sid, "mod_html_motd"))!=NULL) {
		mod_html_motd(sid);
		return;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_MAIN);
	t=(time(NULL)+time_tzoffset(sid, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	prints(sid, "<BR>\r\n<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'><TR><TH STYLE='border-style:solid'>\n");
	prints(sid, "<TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left>Welcome, %s.&nbsp;</TH>\r\n", str2html(sid, sid->dat->user_username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right>&nbsp;%s</TH>\r\n", showtime);
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</TH></TR>\r\n");
	if ((sqr=sql_queryf("SELECT motd FROM gw_groups where groupid = %d", sid->dat->user_gid))<0) return;
	if (sql_numtuples(sqr)==1) {
		if (strlen(sql_getvalue(sqr, 0, 0))>0) {
			prints(sid, "<TR><TD COLSPAN=2 CLASS=\"FIELDVAL\" STYLE='border-style:solid'>\r\n");
			prints(sid, "%s", sql_getvalue(sqr, 0, 0));
			prints(sid, "<BR></TD></TR>\r\n");
		}
	}
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void htselect_day(CONN *sid, char *selected)
{
	char *pdate=selected;
	char day[3];
	int i;

	while ((*pdate)&&(*pdate!='-')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	while ((*pdate)&&(*pdate!='-')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	if ((pdate[0]=='0')&&(isdigit(pdate[1]))) pdate++;
	memset(day, 0, sizeof(day));
	while ((*pdate)&&(isdigit(*pdate))&&(strlen(day)<2)) {
		day[strlen(day)]=*pdate;
		pdate++;
	}
	for (i=1;i<32;i++) {
		prints(sid, "<OPTION VALUE='%02d'%s>%02d\n", i, i==atoi(day)?" SELECTED":"", i);
	}
	return;
}

void htselect_hour(CONN *sid, int selected)
{
	char *timeoption[]={
		"12:00 AM",  "1:00 AM", "2:00 AM", "3:00 AM",  "4:00 AM",  "5:00 AM",
		 "6:00 AM",  "7:00 AM", "8:00 AM", "9:00 AM", "10:00 AM", "11:00 AM",
		"12:00 PM",  "1:00 PM", "2:00 PM", "3:00 PM",  "4:00 PM",  "5:00 PM",
		 "6:00 PM",  "7:00 PM", "8:00 PM", "9:00 PM", "10:00 PM", "11:00 PM"
	};
	int i;

	for (i=0;i<24;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", timeoption[i]);
	}
	return;
}

void htselect_month(CONN *sid, char *selected)
{
	char *option[]={
		"January", "February", "March",     "April",   "May",      "June",
		"July",    "August",   "September", "October", "November", "December"
	};
	char *pdate=selected;
	char month[3];
	int i;

	while ((*pdate)&&(*pdate!='-')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	if ((pdate[0]=='0')&&(isdigit(pdate[1]))) pdate++;
	memset(month, 0, sizeof(month));
	while ((*pdate)&&(isdigit(*pdate))&&(strlen(month)<2)) {
		month[strlen(month)]=*pdate;
		pdate++;
	}
	for (i=1;i<13;i++) {
		prints(sid, "<OPTION VALUE='%02d'%s>%s\n", i, i==atoi(month)?" SELECTED":"", option[i-1]);
	}
	return;
}

void htselect_qhours(CONN *sid, int selected)
{
	int i;

	for (i=0;i<24;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d hour%s\n", i, i==selected?" SELECTED":"", i, i==1?"":"s");
	}
}

void htselect_qminutes(CONN *sid, int selected)
{
	int i;

	for (i=0;i<60;i+=15) {
		prints(sid, "<OPTION VALUE='%d'%s>%d minutes\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_minutes(CONN *sid, int selected)
{
	int i;

	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d minutes\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_time(CONN *sid, time_t unixtime)
{
	char selected[10];
	char temptime[10];
	char *timeoption[]={
		"12:00 AM", "12:15 AM", "12:30 AM", "12:45 AM",
		 "1:00 AM",  "1:15 AM",  "1:30 AM",  "1:45 AM",
		 "2:00 AM",  "2:15 AM",  "2:30 AM",  "2:45 AM",
		 "3:00 AM",  "3:15 AM",  "3:30 AM",  "3:45 AM",
		 "4:00 AM",  "4:15 AM",  "4:30 AM",  "4:45 AM",
		 "5:00 AM",  "5:15 AM",  "5:30 AM",  "5:45 AM",
		 "6:00 AM",  "6:15 AM",  "6:30 AM",  "6:45 AM",
		 "7:00 AM",  "7:15 AM",  "7:30 AM",  "7:45 AM",
		 "8:00 AM",  "8:15 AM",  "8:30 AM",  "8:45 AM",
		 "9:00 AM",  "9:15 AM",  "9:30 AM",  "9:45 AM",
		"10:00 AM", "10:15 AM", "10:30 AM", "10:45 AM",
		"11:00 AM", "11:15 AM", "11:30 AM", "11:45 AM",
		"12:00 PM", "12:15 PM", "12:30 PM", "12:45 PM",
		 "1:00 PM",  "1:15 PM",  "1:30 PM",  "1:45 PM",
		 "2:00 PM",  "2:15 PM",  "2:30 PM",  "2:45 PM",
		 "3:00 PM",  "3:15 PM",  "3:30 PM",  "3:45 PM",
		 "4:00 PM",  "4:15 PM",  "4:30 PM",  "4:45 PM",
		 "5:00 PM",  "5:15 PM",  "5:30 PM",  "5:45 PM",
		 "6:00 PM",  "6:15 PM",  "6:30 PM",  "6:45 PM",
		 "7:00 PM",  "7:15 PM",  "7:30 PM",  "7:45 PM",
		 "8:00 PM",  "8:15 PM",  "8:30 PM",  "8:45 PM",
		 "9:00 PM",  "9:15 PM",  "9:30 PM",  "9:45 PM",
		"10:00 PM", "10:15 PM", "10:30 PM", "10:45 PM",
		"11:00 PM", "11:15 PM", "11:30 PM", "11:45 PM"
	};
	int i, j;

	memset(selected, 0, sizeof(selected));
	snprintf(selected, sizeof(selected)-1, "%s", time_unix2sqltime(sid, unixtime));
	for (i=0;i<24;i++) {
		for (j=0;j<4;j++) {
			snprintf(temptime, 9, "%02d:%02d:00", i, j*15);
			prints(sid, "<OPTION VALUE='%s'%s>%s\n", temptime, strcmp(selected, temptime)==0?" SELECTED":"", timeoption[i*4+j]);
		}
	}
	return;
}

void htselect_year(CONN *sid, int startyear, char *selected)
{
	char year[5];
	int i;

	if (startyear<1900) startyear=1900;
	memset(year, 0, sizeof(year));
	strncpy(year, selected, 4);
	for (i=startyear;i<2038;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d\n", i, i==atoi(year)?" SELECTED":"", i);
	}
	return;
}

void htselect_contact(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT contactid, surname, givenname FROM gw_contacts WHERE obj_did = %d order by surname, givenname ASC", sid->dat->user_did))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
		if (strlen(sql_getvalue(sqr, i, 1))&&strlen(sql_getvalue(sqr, i, 2))) prints(sid, ", ");
		prints(sid, "%s\n", str2html(sid, sql_getvalue(sqr, i, 2)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_domain(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT domainid, domainname FROM gw_domains order by domainname ASC"))<0) return;
	} else {
		if ((sqr=sql_queryf("SELECT domainid, domainname FROM gw_domains WHERE domainid = %d ORDER BY domainname ASC", sid->dat->user_did))<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_eventstatus(CONN *sid, int selected)
{
	char *option[]={ "Open", "Closed", };
	int i;

	for (i=0;i<2;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_group(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d order by groupname ASC", sid->dat->user_did))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_number(CONN *sid, int selected, int start, int end, int increment)
{
	int i;

	if (increment<1) increment=1;
	for (i=start;i<end+1;i+=increment) {
		prints(sid, "<OPTION VALUE='%d'%s>%d\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_priority(CONN *sid, int selected)
{
	char *option[]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int i;

	for (i=0;i<5;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_reminder(CONN *sid, int selected)
{
	int i;

	prints(sid, "<OPTION VALUE='0'>No reminder\n");
	prints(sid, "<OPTION VALUE='15'%s>15 minutes before\n", (selected==15)?" SELECTED":"");
	prints(sid, "<OPTION VALUE='30'%s>30 minutes before\n", (selected==30)?" SELECTED":"");
	for (i=1;i<169;i++) {
		switch(i) {
		case 1:
		case 2:
		case 3:
		case 6:
		case 12:
		case 24:
		case 48:
		case 72:
		case 96:
		case 120:
		case 144:
		case 168:
			prints(sid, "<OPTION VALUE='%d'%s>%s\n", i*60, i*60==selected?" SELECTED":"", htview_reminder(sid, i*60));
			continue;
		default:
			continue;
		}
	}
	return;
}

void htselect_user(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", sid->dat->user_did))<0) return;
	prints(sid, "<OPTION VALUE=0>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

void htselect_zone(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf("SELECT zoneid, zonename FROM gw_zones WHERE obj_did = %d order by zonename ASC", sid->dat->user_did))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

char *htview_callaction(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT actionname FROM gw_callactions WHERE callactionid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
	}
	sql_freeresult(sqr);
	return buffer;
}

char *htview_contact(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT surname, givenname FROM gw_contacts WHERE contactid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
		if (strlen(sql_getvalue(sqr, 0, 0))&&strlen(sql_getvalue(sqr, 0, 1))) strcat(buffer, ", ");
		strncatf(buffer, sizeof(sid->dat->smallbuf[0])-strlen(buffer)-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
	}
	sql_freeresult(sqr);
	return buffer;
}

char *htview_domain(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT domainname FROM gw_domains WHERE domainid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
	}
	sql_freeresult(sqr);
	return buffer;
}

char *htview_eventclosingstatus(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT closingname FROM gw_eventclosings WHERE eventclosingid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
	}
	sql_freeresult(sqr);
	return buffer;
}

char *htview_eventstatus(CONN *sid, int selected)
{
	char *option[]={ "Open", "Closed" };

	if ((selected<0)||(selected>1)) return "";
	return option[selected];
}

char *htview_eventtype(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT eventtypename FROM gw_eventtypes WHERE eventtypeid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
	}
	sql_freeresult(sqr);
	return buffer;
}

char *htview_holiday(char *date)
{
	char *holidays[13][2]={
		{ "-01-01", "New Year's Day" },
		{ "-02-02", "Groundhog Day" },
		{ "-02-14", "Valentine's Day" },
		{ "-03-17", "St. Patrick's Day" },
		{ "-05-24", "Victoria Day" },
		{ "-07-01", "Canada Day" },
		{ "-07-04", "Independance Day" },
		{ "-10-31", "Halloween" },
		{ "-11-11", "Rememberance Day" },
		{ "-12-25", "Christmas Day" },
		{ "-12-26", "Boxing Day" },
		{ "-12-31", "New Year's Eve" },
		{ NULL,     NULL }
	};
	int i=0;

	if ((date==NULL)||(strlen(date)<10)) return "";
	while (holidays[i][0]!=NULL) {
		if (strncmp(date+4, holidays[i][0], 6)==0) return holidays[i][1];
		i++;
	}
	return "";
}

char *htview_reminder(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);

	if (selected==0) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "No reminder");
	} else if ((selected>0)&&(selected<60)) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%d minutes before", selected);
	} else if ((selected>59)&&(selected<120)) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%d hour before", selected/60);
	} else if ((selected>119)&&(selected<1440)) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%d hours before", selected/60);
	} else if ((selected>1439)&&(selected<2880)) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%d day before", selected/1440);
	} else if (selected>2879) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%d days before", selected/1440);
	}
	return buffer;
}

char *htview_user(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE userid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
	}
	sql_freeresult(sqr);
	return buffer;
}

void htscript_showpage(CONN *sid, short int pages)
{
	if (pages<1) return;
	if (pages>10) pages=10;
	prints(sid, "function showpage(page) {\n");
	prints(sid, "	for (var i=1;i<%d;i++) {\n", pages+1);
	prints(sid, "		if (i==page) {\n");
	prints(sid, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 0px #000000';\n");
	prints(sid, "			document.getElementById('page'+i+'tab').bgColor='#F0F0F0';\n");
	prints(sid, "			document.getElementById('page'+i).style.display='block';\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 1px #000000';\n");
	prints(sid, "			document.getElementById('page'+i+'tab').bgColor='#E0E0E0';\n");
	prints(sid, "			document.getElementById('page'+i).style.display='none';\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	return;
}
