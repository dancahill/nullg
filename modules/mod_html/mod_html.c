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

void mod_html_header(CONN *sid, char *title)
{
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n");
	prints(sid, "<HEAD>\r\n");
//	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=us-ascii\">\r\n");
	prints(sid, "<TITLE>%s</TITLE>\r\n", title);
	prints(sid, "<STYLE TYPE=text/css>\r\n");
	prints(sid, ".JUSTIFY { text-align: justify; }\r\n");
	prints(sid, ".TBAR    { color: #505050; text-decoration: none; font-family: Geneva, Arial,Verdana; font-size: 8pt; }\r\n");
	prints(sid, "A        { color: %s; text-decoration: none; }\r\n", proc->config.colour_links);
	prints(sid, "A:HOVER  { text-decoration: underline; }\r\n");
//	prints(sid, "INPUT    { color: #000000; font-family: Courier New; font-size: 12px; }\r\n");
//	prints(sid, "SELECT   { color: #000000; font-family: Courier New; font-size: 12px; }\r\n");
	prints(sid, "TD       { color: #000000; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n");
	prints(sid, "TH       { background-color: %s; color: %s; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n", proc->config.colour_th, proc->config.colour_thtext);
	prints(sid, "</STYLE>\r\n");
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/groupware/css/style.css\">\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/mail/write", 11)==0) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript SRC=/groupware/javascript/wmedit.js TYPE=text/javascript></SCRIPT>\r\n");
	}
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<BODY BACKGROUND=\"/groupware/images/bgmain.gif\"");
	prints(sid, " BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\"");
	prints(sid, " TOPMARGIN=0 LEFTMARGIN=0 MARGINHEIGHT=0 MARGINWIDTH=0");
	if (strncmp(sid->dat->in_RequestURI, "/mail/write", 11)==0) {
		prints(sid, " onLoad=init();");
	}
	prints(sid, ">\r\n");
}

void mod_html_footer(CONN *sid)
{
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void mod_html_topmenu(CONN *sid, int menu)
{
	char *ptemp;
	int dot=0;
	int folderid=0;
	int userid=-1;
	int groupid=-1;
	int status=0;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	}
	switch (menu) {
		case MENU_ADMIN:     mod_html_header(sid, "NullLogic Groupware Administration"); break;
		case MENU_BOOKMARKS: mod_html_header(sid, "NullLogic Groupware Bookmarks"); break;
		case MENU_CALENDAR:  mod_html_header(sid, "NullLogic Groupware Calendar"); break;
		case MENU_CALLS:     mod_html_header(sid, "NullLogic Groupware Calls"); break;
		case MENU_CONTACTS:  mod_html_header(sid, "NullLogic Groupware Contacts"); break;
		case MENU_FILES:     mod_html_header(sid, "NullLogic Groupware Files"); break;
		case MENU_FORUMS:    mod_html_header(sid, "NullLogic Groupware Forums"); break;
		case MENU_MAIN:      mod_html_header(sid, "NullLogic Groupware Welcome"); break;
		case MENU_NOTES:     mod_html_header(sid, "NullLogic Groupware Notebook"); break;
		case MENU_ORDERS:    mod_html_header(sid, "NullLogic Groupware Orders"); break;
		case MENU_PROFILE:   mod_html_header(sid, "NullLogic Groupware User Profile"); break;
		case MENU_SEARCHES:  mod_html_header(sid, "NullLogic Groupware Searches"); break;
		case MENU_TASKS:     mod_html_header(sid, "NullLogic Groupware Tasks"); break;
		case MENU_WEBMAIL:   mod_html_header(sid, "NullLogic Groupware Webmail"); break;
		case MENU_XMLRPC:    mod_html_header(sid, "NullLogic Groupware XML-RPC"); break;
		default: mod_html_header(sid, "NullLogic Groupware"); break;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "function ShowHelp()\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('/groupware/help/");
	switch (menu) {
		case MENU_ADMIN:     prints(sid, "ch02-00.html"); break;
		case MENU_BOOKMARKS: prints(sid, "ch03-00.html"); break;
		case MENU_CALENDAR:  prints(sid, "ch04-00.html"); break;
		case MENU_CALLS:     prints(sid, "ch05-00.html"); break;
		case MENU_CONTACTS:  prints(sid, "ch06-00.html"); break;
		case MENU_WEBMAIL:   prints(sid, "ch07-00.html"); break;
		case MENU_FILES:     prints(sid, "ch08-00.html"); break;
		case MENU_FORUMS:    prints(sid, "ch09-00.html"); break;
		case MENU_MAIN:      prints(sid, "index.html"); break;
		case MENU_NOTES:     prints(sid, "ch11-00.html"); break;
		case MENU_ORDERS:    prints(sid, "ch12-00.html"); break;
		case MENU_PROFILE:   prints(sid, "ch13-00.html"); break;
		case MENU_SEARCHES:  prints(sid, "ch14-00.html"); break;
		case MENU_TASKS:     prints(sid, "ch04-00.html"); break;
		default: prints(sid, "index.html"); break;
	}
	prints(sid, "','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\r\n");
	prints(sid, "}\r\n");
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
		prints(sid, "function ReplyTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?replyto='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function ReplyAll(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?replyall='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function Forward(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?forward='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	}
	if (sid->dat->user_menustyle==0) {
		prints(sid, "function ListUsers() {\n");
		prints(sid, "	window.open('%s/messages/userlist','msguserlist','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=200,height=300');\n", sid->dat->in_ScriptName);
		prints(sid, "}\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if ((menu==MENU_WEBMAIL)&&(sid->dat->user_menustyle>0)) {
		if ((strncmp(sid->dat->in_RequestURI, "/mail/list", 10)!=0)&&(strncmp(sid->dat->in_RequestURI, "/mail/sync", 10)!=0)) return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=0 CELLSPACING=1 WIDTH=100%%>\r\n", proc->config.colour_tabletrim);
	if (sid->dat->user_menustyle==0) {
		prints(sid, "<TR><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;&nbsp;", proc->config.colour_topmenu);
		if (module_exists(sid, "mod_admin")&&(auth_priv(sid, "admin")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/admin/>ADMIN</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_bookmarks")&&(auth_priv(sid, "bookmarks")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/bookmarks/list>BOOKMARKS</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/calendar/list>CALENDAR</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_calls")&&(auth_priv(sid, "calls")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/calls/list>CALLS</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_contacts")&&(auth_priv(sid, "contacts")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/contacts/list>CONTACTS</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/mail/list>E-MAIL</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_files")&&(auth_priv(sid, "files")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/files/>FILES</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_forums")&&(auth_priv(sid, "forums")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/forums/list>FORUMS</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_messages")&&(auth_priv(sid, "messages")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=javascript:ListUsers()>MESSENGER</A>", dot?"&nbsp;&middot;&nbsp;":"");
			dot=1;
		}
		if (module_exists(sid, "mod_notes")) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/notes/list>NOTEBOOK</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_orders")&&(auth_priv(sid, "orders")>0)) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/orders/list>ORDERS</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		if (module_exists(sid, "mod_searches")) {
			prints(sid, "%s<A CLASS='TBAR' HREF=%s/search/>SEARCHES</A>", dot?"&nbsp;&middot;&nbsp;":"", sid->dat->in_ScriptName);
			dot=1;
		}
		prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
		prints(sid, "<A CLASS='TBAR' HREF=javascript:ShowHelp()>%s</A>&nbsp;&middot;&nbsp;", MENU_HELP);
		prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, MENU_LOGOUT);
		prints(sid, "&nbsp;&nbsp;</TD>\r\n");
		prints(sid, "</TR></TABLE>\r\n</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle BGCOLOR=%s>\r\n", proc->config.colour_topmenu);
	prints(sid, "<TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
	switch (menu) {
		case MENU_ADMIN:
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/configedit>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_CONFIG);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/activitylist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_LOGS);
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
			if (auth_priv(sid, "bookmarks")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/bookmarks/foldereditnew?parent=%d>NEW FOLDER</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, folderid);
			}
			break;
		case MENU_CALENDAR:
		case MENU_TASKS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/list?status=%d", sid->dat->in_ScriptName, status);
			if (userid>0) prints(sid, "&userid=%d", userid);
			if (groupid>0) prints(sid, "&groupid=%d", groupid);
			prints(sid, ">TODAY</A>&nbsp;&middot;&nbsp;");
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/listweek?status=%d", sid->dat->in_ScriptName, status);
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
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/editnew2>NEW CONTACT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/search1>SEARCH</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/editnew>NEW CONTACT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
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
			prints(sid, "<A CLASS='TBAR' HREF=%s/frames/motd>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
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
			if (auth_priv(sid, "query")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/search/sqlrun>SQL QUERIES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_WEBMAIL:
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/list%s>INBOX</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=wmlist":"");
			if (sid->dat->user_menustyle>0) {
				prints(sid, "<A CLASS='TBAR' HREF=javascript:ComposeMail()>COMPOSE</A>&nbsp;&middot;&nbsp;");
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/mail/write>COMPOSE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/sync>SEND/RECV</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/quit%s>QUIT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=gwmain":"");
			break;
	}
	prints(sid, "<A CLASS='TBAR' HREF=javascript:window.print()>%s</A>", MENU_PRINT);
	prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
	if (sid->dat->user_menustyle>0) {
		prints(sid, "<A CLASS='TBAR' HREF=javascript:ShowHelp()>%s</A>&nbsp;&middot;&nbsp;", MENU_HELP);
		if (menu==MENU_WEBMAIL) {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit TARGET=gwmain>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		} else {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, MENU_LOGOUT);
	}
	prints(sid, "&nbsp;&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
//	flushbuffer(sid);
}

void mod_html_login(CONN *sid)
{
	char pageuri[200];
	char file[100];
	char username[32];
	char password[32];
	char *ptemp;
	FILE *fp;
	int ich;
	int sqr;

	memset(pageuri, 0, sizeof(pageuri));
	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s%s", sid->dat->in_ScriptName, sid->dat->in_RequestURI);
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	mod_html_header(sid, "NullLogic Groupware Login");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR>\r\n");
	snprintf(file, sizeof(file)-1, "%s/issue.txt", proc->config.server_dir_etc);
	fixslashes(file);
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while ((ich=getc(fp))!=EOF) {
			prints(sid, "%c", ich);
		}
		fclose(fp);
	}
	prints(sid, "<CENTER>\r\n");
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		prints(sid, "<FONT COLOR=RED><B>Incorrect username or password</B></FONT>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=\"%s/\" AUTOCOMPLETE=OFF NAME=login>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=pageuri VALUE=\"%s\">\r\n", pageuri);
	prints(sid, "<TR BGCOLOR=\"%s\"><TH COLSPAN=2 STYLE='padding:1px'><FONT COLOR=%s>NullLogic Groupware Login</FONT></TH></TR>\r\n", proc->config.colour_th, proc->config.colour_thtext);
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	if ((sqr=sql_query(sid, "SELECT username, password FROM gw_users WHERE userid = 1"))<0) return;
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
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=username SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", proc->config.colour_editform, username);
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=password SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", proc->config.colour_editform, password);
	prints(sid, "<TR BGCOLOR=\"%s\"><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\r\n", proc->config.colour_editform);
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

void mod_html_logout(CONN *sid)
{
	db_log_activity(sid, 0, "login", 0, "logout", "%s - Logout: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	mod_html_header(sid, "NullLogic Groupware Logout");
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH><FONT COLOR=%s>NullLogic Groupware Logout</FONT></TH></TR>\r\n", proc->config.colour_th, proc->config.colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD>\r\n", proc->config.colour_fieldval);
	prints(sid, "You have successfully logged out.\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('%s/');\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='%s/'\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void mod_html_motd(CONN *sid)
{
	MOD_TASKS_LIST mod_tasks_list;
	char posttime1[32];
	char posttime2[32];
	char showtime[32];
	time_t t, t2;
	time_t unixdate;
	int newmessages;
	int i, j, k;
	int sqr, sqr2;

	t=(time(NULL)+time_tzoffset(sid, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	mod_html_topmenu(sid, MENU_MAIN);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n<TABLE WIDTH=90%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR><TD COLSPAN=2>\r\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%><TR><TH BGCOLOR=%s>\n", proc->config.colour_tabletrim, proc->config.colour_th);
	prints(sid, "<TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left><FONT COLOR=%s>Welcome, %s.&nbsp;</FONT></TH>\r\n", proc->config.colour_thtext, str2html(sid, sid->dat->user_username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right><FONT COLOR=%s>&nbsp;%s</FONT></TH>\r\n", proc->config.colour_thtext, showtime);
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</TH></TR>\r\n");
	if ((sqr=sql_queryf(sid, "SELECT motd FROM gw_groups where groupid = %d", sid->dat->user_gid))<0) return;
	if (sql_numtuples(sqr)==1) {
		if (strlen(sql_getvalue(sqr, 0, 0))>0) {
			prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>\r\n", proc->config.colour_fieldval);
			prints(sid, "%s", sql_getvalue(sqr, 0, 0));
			prints(sid, "<BR></TD></TR>\r\n");
		}
	}
	sql_freeresult(sqr);
	prints(sid, "</TABLE></TD></TR>\r\n");
	if ((auth_priv(sid, "calendar")&A_READ)<1) {
		prints(sid, "</TABLE>\r\n</CENTER>\r\n");
		prints(sid, "</BODY>\r\n</HTML>\r\n");
		return;
	}
	prints(sid, "<TR><TD VALIGN=TOP WIDTH=100%%>\r\n");
	unixdate=(time(NULL)+time_tzoffset(sid, time(NULL)))/86400;
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	unixdate-=time_tzoffset(sid, unixdate);
	t=unixdate;
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+172800;
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (strcmp(proc->config.sql_type, "ODBC")==0) {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish, eventname FROM gw_events where status = 0 and eventstart >= #%s# and eventstart < #%s# and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish, eventname FROM gw_events where status = 0 and eventstart >= '%s' and eventstart < '%s' and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid))<0) return;
	}
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=100%%>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP VALIGN=TOP><FONT COLOR=%s>Calendar</FONT></TH></TR>\n", proc->config.colour_th, proc->config.colour_thtext);
	for (i=0;i<2;i++) {
		t=unixdate+(i*86400);
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT COLSPAN=2 NOWRAP><B><A HREF=%s/calendar/list?day=%d>", proc->config.colour_fieldval, sid->dat->in_ScriptName, (int)(t/86400));
		if (i==0) {
			prints(sid, "Today");
		} else if (i==1) {
			prints(sid, "Tomorrow");
		}
		prints(sid, "</A></B></TD></TR>\n");
		k=1;
		for (j=0;j<sql_numtuples(sqr);j++) {
			t2=time_sql2unix(sql_getvalue(sqr, j, 1));
			if ((t2<t)||(t2>=t+86400)) continue;
			prints(sid, "<TR BGCOLOR=%s>", proc->config.colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP><FONT SIZE=2>");
			t2=time_sql2unix(sql_getvalue(sqr, j, 1))+time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, j, 1)));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(sqr, j, 0), time_unix2timetext(sid, t2));
			t2=time_sql2unix(sql_getvalue(sqr, j, 2))+time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr, j, 2)));
			prints(sid, " - %s</A></FONT></TD><TD NOWRAP WIDTH=100%%><FONT SIZE=2>", time_unix2timetext(sid, t2));
			prints(sid, "<A HREF=%s/calendar/view?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sql_getvalue(sqr, j, 0), str2html(sid, sql_getvalue(sqr, j, 3)));
			prints(sid, "</FONT></TD></TR>\n");
			if (k>0) k--;
		}
		while (k>0) {
			prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2 NOWRAP WIDTH=100%%><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n", proc->config.colour_fieldval);
			k--;
		}
	}
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD><TD VALIGN=TOP>\n");
	if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=1 CELLSPACING=1 WIDTH=100%%>\r\n", proc->config.colour_tabletrim);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%%><FONT SIZE=2 COLOR=%s>&nbsp;E-Mail</FONT></TH></TR>\n", proc->config.colour_th, proc->config.colour_thtext);
		if ((sqr=sql_queryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts where obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid))<0) return;
		for (i=0;i<sql_numtuples(sqr);i++) {
			if ((sqr2=sql_queryf(sid, "SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status = 'n'", sid->dat->user_uid, atoi(sql_getvalue(sqr, i, 0))))<0) continue;
			newmessages=atoi(sql_getvalue(sqr2, 0, 0));
			sql_freeresult(sqr2);
			prints(sid, "<TR BGCOLOR=%s><TD WIDTH=100%%><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", proc->config.colour_fieldval);
			prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT NOWRAP>", proc->config.colour_fieldval);
			if (sid->dat->user_menustyle>0) {
				prints(sid, "<A HREF=%s/mail/main?accountid=%d TARGET=gwmain>%-.25s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
			} else {
				prints(sid, "<A HREF=%s/mail/list?accountid=%d>%-.25s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
			}
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s%d New%s</TD></TR>\n</TABLE></TD></TR>\n", newmessages?"<FONT COLOR=BLUE><B>":"", newmessages, newmessages?"</B></FONT>":"");
		}
		sql_freeresult(sqr);
		prints(sid, "</TABLE>\n");
		prints(sid, "<BR>\n");
	}
	if ((mod_tasks_list=module_call(sid, "mod_tasks_list"))!=NULL) {
		mod_tasks_list(sid, sid->dat->user_uid, -1);
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void mod_html_frameset(CONN *sid)
{
	MOD_MAIL_SYNC mod_mail_sync;
	int i, j;
	int sqr1;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Frameset//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware</TITLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<FRAMESET COLS=\"125,*\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n");
	prints(sid, "<FRAMESET ROWS=\"*,0\"   BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n");
	prints(sid, "<FRAME BORDER=0 NAME=\"gwmenu\" SRC=%s/frames/menu MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<FRAME BORDER=0 NAME=\"gwtemp\" SRC=%s/frames/temp MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</FRAMESET>\r\n");
	prints(sid, "<FRAME BORDER=0 NAME=\"gwmain\" SRC=%s/frames/motd MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=YES>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</FRAMESET>\r\n");
	prints(sid, "To view this page, you need a web browser capable of displaying frames.\r\n");
	prints(sid, "</HTML>\r\n");
	prints(sid, "<!--\r\n");
	if ((mod_mail_sync=module_call(sid, "mod_mail_sync"))!=NULL) {
		if ((sqr1=sql_queryf(sid, "SELECT mailaccountid, accountname, poppassword, lastcount, notify, lastcheck FROM gw_mailaccounts where obj_uid = %d and notify > 0", sid->dat->user_uid))>-1) {
			for (i=0;i<sql_numtuples(sqr1);i++) {
				sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr1, i, 0));
				j=time_sql2unix(sql_getvalue(sqr1, i, 5));
				if ((strlen(sql_getvalue(sqr1, i, 2))>0)&&(j+(atoi(sql_getvalue(sqr1, i, 4))*60)<time(NULL))) {
					mod_mail_sync(sid, 0);
				}
			}
			sql_freeresult(sqr1);
		}
	}
	prints(sid, "-->\r\n");
}

void mod_html_menuframe(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware Menu</TITLE>\r\n");
	prints(sid, "<STYLE TYPE=text/css>\r\n");
	prints(sid, "DIV     { position:absolute; }\n");
	prints(sid, "A       { font-family: Arial, Verdana, sans-serif; font-size=9pt; color: #FFFFFF; text-decoration: none}\n");
	prints(sid, "A:HOVER { font-family: Arial, Verdana, sans-serif; font-size=9pt; color: #399CDE; text-decoration: none}\n");
	prints(sid, "</STYLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\n");
	prints(sid, "var browser_type=navigator.appName;\n");
	prints(sid, "var browser_version=parseInt(navigator.appVersion);\n");
	prints(sid, "function onMouseDownHandler(e) {\n");
	prints(sid, "	if (e.which==3) {\n");
	prints(sid, "		return false;\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints(sid, "	if (document.layers) {\n");
	prints(sid, "		window.captureEvents(Event.MOUSEDOWN);\n");
	prints(sid, "		window.onmousedown=onMouseDownHandler;\n");
	prints(sid, "	}\n");
	prints(sid, "}\n\n");
	prints(sid, "function ListUsers() {\n");
	prints(sid, "	window.open('%s/messages/userlist','msguserlist','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=200,height=300');\n", sid->dat->in_ScriptName);
	prints(sid, "}\n");
	prints(sid, "function doreminder() {\n");
	prints(sid, "	window.parent.gwtemp.location.replace(\"%s/frames/temp\");\n", sid->dat->in_ScriptName);
	prints(sid, "	reminders();\n");
	prints(sid, "}\n");
	prints(sid, "function reminders() {\n");
	prints(sid, "	setTimeout(\"doreminder()\", 300000);\n");
	prints(sid, "}\n");
	prints(sid, "reminders();\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if (sid->dat->user_menustyle==1) {
		prints(sid, "</HEAD>\r\n");
		prints(sid, "<BODY BACKGROUND=/groupware/images/bgborder.gif BGCOLOR=#000050 TEXT=#000000 LINK=#FFFFFF ALINK=#FFFFFF VLINK=#FFFFFF onContextMenu='return false'>\r\n");
		prints(sid, "<CENTER>\r\n");
		prints(sid, "<A HREF=%s/frames/motd target='gwmain' onclick=\"window.open('%s/frames/menu','gwmenu')\"><IMG BORDER=0 SRC=/groupware/images/groupware.gif HEIGHT=50 WIDTH=125 ALT='%s logged in'></A><BR><BR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName, sid->dat->user_username);
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=7 WIDTH=100%%>\r\n");
		if (module_exists(sid, "mod_admin")&&(auth_priv(sid, "admin")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/admin/'\"><A HREF=%s/admin/ TARGET=gwmain><FONT SIZE=2><B>ADMINISTRATION</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_bookmarks")&&(auth_priv(sid, "bookmarks")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/bookmarks/list'\"><A HREF=%s/bookmarks/list TARGET=gwmain><FONT SIZE=2><B>BOOKMARKS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/calendar/list'\"><A HREF=%s/calendar/list TARGET=gwmain><FONT SIZE=2><B>CALENDAR</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_calls")&&(auth_priv(sid, "calls")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/calls/list'\"><A HREF=%s/calls/list TARGET=gwmain><FONT SIZE=2><B>CALLS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_contacts")&&(auth_priv(sid, "contacts")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/contacts/list'\"><A HREF=%s/contacts/list TARGET=gwmain><FONT SIZE=2><B>CONTACTS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/mail/main'\"><A HREF=%s/mail/main TARGET=gwmain><FONT SIZE=2><B>E-MAIL</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_files")&&(auth_priv(sid, "files")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/files/'\"><A HREF=%s/files/ TARGET=gwmain><FONT SIZE=2><B>FILES</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_forums")&&(auth_priv(sid, "forums")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/forums/list'\"><A HREF=%s/forums/list TARGET=gwmain><FONT SIZE=2><B>FORUMS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_messages")&&(auth_priv(sid, "messages")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=ListUsers();><A HREF=javascript:ListUsers();><FONT SIZE=2><B>MESSENGER</B></FONT></A></TD></TR>\r\n");
		}
		if (module_exists(sid, "mod_notes")) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/notes/list'\"><A HREF=%s/notes/list TARGET=gwmain><FONT SIZE=2><B>NOTEBOOK</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_orders")&&(auth_priv(sid, "orders")>0)) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/orders/list'\"><A HREF=%s/orders/list TARGET=gwmain><FONT SIZE=2><B>ORDERS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_searches")) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/search/'\"><A HREF=%s/search/ TARGET=gwmain><FONT SIZE=2><B>SEARCHES</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		prints(sid, "</TABLE>\r\n");
		prints(sid, "</CENTER>\r\n</BODY>\r\n</HTML>\r\n");
	} else if (sid->dat->user_menustyle==2) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript SRC=/groupware/javascript/outlook.js TYPE=text/javascript></SCRIPT>\r\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\n");
		prints(sid, "var o = new createOutlookBar('Bar',0,0,screenSize.width,screenSize.height,'#404040','white')\n");
		prints(sid, "var p\n");
		prints(sid, "//create first panel\n");
		prints(sid, "p = new createPanel('al','Communication');\n");
		if (module_exists(sid, "mod_calls")&&(auth_priv(sid, "calls")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/default.gif',  'Calls',         'parent.gwmain.location=\"%s/calls/list\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_mail")&&(auth_priv(sid, "webmail")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/email.gif',    'E-Mail',        'parent.gwmain.location=\"%s/mail/main\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_forums")&&(auth_priv(sid, "forums")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/default.gif',  'Forums',        'parent.gwmain.location=\"%s/forums/list\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_messages")&&(auth_priv(sid, "messages")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/messenger.gif','Messenger',      'ListUsers()');\n");
		}
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "p = new createPanel('p','Information');\n");
		if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/calendar.gif', 'Calendar',      'parent.gwmain.location=\"%s/calendar/list\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_contacts")&&(auth_priv(sid, "contacts")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/contacts.gif', 'Contacts',      'parent.gwmain.location=\"%s/contacts/list\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_orders")&&(auth_priv(sid, "orders")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/orders.gif',   'Orders',        'parent.gwmain.location=\"%s/orders/list\"');\n", sid->dat->in_ScriptName);
		}
		prints(sid, "p.addButton('/groupware/images/menu2/searches.gif', 'Searches',      'parent.gwmain.location=\"%s/search/\"');\n", sid->dat->in_ScriptName);
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "p = new createPanel('l','Resources');\n");
		if (module_exists(sid, "mod_admin")&&(auth_priv(sid, "admin")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/admin.gif',    'Administration','parent.gwmain.location=\"%s/admin/\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_bookmarks")&&(auth_priv(sid, "bookmarks")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/bookmarks.gif','Bookmarks',     'parent.gwmain.location=\"%s/bookmarks/list\"');\n", sid->dat->in_ScriptName);
		}
		if (module_exists(sid, "mod_files")&&(auth_priv(sid, "files")>0)) {
			prints(sid, "p.addButton('/groupware/images/menu2/default.gif',  'Files',         'parent.gwmain.location=\"%s/files/\"');\n", sid->dat->in_ScriptName);
		}
		prints(sid, "p.addButton('/groupware/images/menu2/default.gif', 'Notebook', 'parent.gwmain.location=\"%s/notes/list\"');\n", sid->dat->in_ScriptName);
		prints(sid, "p.addButton('/groupware/images/menu2/default.gif', 'Preferences', 'parent.gwmain.location=\"%s/profile/edit\"');\n", sid->dat->in_ScriptName);
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "o.draw();         //draw the OutlookBar\n\n");
		prints(sid, "// -->\r\n</SCRIPT>\r\n");
		prints(sid, "</HEAD>\r\n");
		prints(sid, "<BODY BACKGROUND=/groupware/images/bgborder.gif BGCOLOR=#000050 TEXT=#000000 LINK=#FFFFFF ALINK=#FFFFFF VLINK=#FFFFFF onContextMenu='return false' onLoad=resize_op5(); onResize=myOnResize();>\r\n");
		prints(sid, "</BODY>\r\n</HTML>\r\n");
	}
	return;
}

void mod_html_reloadframe(CONN *sid)
{
	MOD_MAIL_SYNC mod_mail_sync;
	char posttime[32];
	int isreminder;
	int i, j;
	int a, b;
	int sqr1, sqr2;
	time_t t;
	int notice=0;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware</TITLE>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"60; URL=%s/frames/temp\">\r\n</HEAD>\r\n", sid->dat->in_ScriptName);
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (strcmp(proc->config.sql_type, "ODBC")==0) {
			if ((sqr1=sql_queryf(sid, "SELECT eventstart, reminder FROM gw_events where eventstart < #%s# and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
		} else {
			if ((sqr1=sql_queryf(sid, "SELECT eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 ORDER BY eventstart ASC", posttime, sid->dat->user_uid))<0) return;
		}
		for (i=0;i<sql_numtuples(sqr1);i++) {
			a=time_sql2unix(sql_getvalue(sqr1, i, 0))-time(NULL);
			b=a-atoi(sql_getvalue(sqr1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/calendar/reminders','calendarwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sql_freeresult(sqr1);
	}
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (strcmp(proc->config.sql_type, "ODBC")==0) {
			if ((sqr1=sql_queryf(sid, "SELECT duedate, reminder FROM gw_tasks where duedate < #%s# and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
		} else {
			if ((sqr1=sql_queryf(sid, "SELECT duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 ORDER BY duedate ASC", posttime, sid->dat->user_uid))<0) return;
		}
		for (i=0;i<sql_numtuples(sqr1);i++) {
			a=time_sql2unix(sql_getvalue(sqr1, i, 0))-time(NULL);
			a-=time_tzoffset(sid, time_sql2unix(sql_getvalue(sqr1, i, 0)));
			b=a-atoi(sql_getvalue(sqr1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/tasks/reminders','taskwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sql_freeresult(sqr1);
	}
	if (auth_priv(sid, "messages")&A_READ) {
		t=time(NULL)-10;
		strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
		if (strcmp(proc->config.sql_type, "ODBC")==0) {
			if ((sqr1=sql_queryf(sid, "SELECT messageid, sender FROM gw_messages WHERE obj_uid = %d AND rcpt = %d and status > 2 AND obj_ctime < #%s#", sid->dat->user_uid, sid->dat->user_uid, posttime))<0) return;
		} else {
			if ((sqr1=sql_queryf(sid, "SELECT messageid, sender FROM gw_messages WHERE obj_uid = %d AND rcpt = %d and status > 2 AND obj_ctime < '%s'", sid->dat->user_uid, sid->dat->user_uid, posttime))<0) return;
		}
		if (sql_numtuples(sqr1)>0) {
			if ((sqr2=sql_queryf(sid, "SELECT userid, username FROM gw_users"))<0) {
				sql_freeresult(sqr1);
				return;
			}
			for (i=0;i<sql_numtuples(sqr2);i++) {
				for (j=0;j<sql_numtuples(sqr1);j++) {
					if (atoi(sql_getvalue(sqr2, i, 0))==atoi(sql_getvalue(sqr1, j, 1))) {
						prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
						prints(sid, "window.open('%s/messages/frame?userid=%d','msgwin_%s','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=200');\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr2, i, 0)), sql_getvalue(sqr2, i, 1));
						prints(sid, "// -->\n</SCRIPT>\n");
					}
				}
			}
			sql_freeresult(sqr2);
		}
		sql_freeresult(sqr1);
	}
	if ((mod_mail_sync=module_call(sid, "mod_mail_sync"))!=NULL) {
		notice=0;
		if ((sqr1=sql_queryf(sid, "SELECT mailaccountid, accountname, poppassword, lastcount, notify, lastcheck FROM gw_mailaccounts where obj_uid = %d and notify > 0", sid->dat->user_uid))<0) return;
		for (i=0;i<sql_numtuples(sqr1);i++) {
			sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr1, i, 0));
			j=time_sql2unix(sql_getvalue(sqr1, i, 5));
			if ((strlen(sql_getvalue(sqr1, i, 2))>0)&&(j+(atoi(sql_getvalue(sqr1, i, 4))*60)<time(NULL))) {
				mod_mail_sync(sid, 0);
			}
			if ((sqr2=sql_queryf(sid, "SELECT mailheaderid FROM gw_mailheaders where obj_uid = %d AND accountid = %d AND status = 'n'", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) break;
			if (sql_numtuples(sqr2)>0) notice=1;
			sql_freeresult(sqr2);
		}
		sql_freeresult(sqr1);
		if (notice) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/mail/notice','mailwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
	}
	prints(sid, "<BODY BGCOLOR=#000050>\r\n</BODY>\r\n</HTML>\r\n");
}

void mod_main(CONN *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/frames/menu", 12)==0) {
		mod_html_menuframe(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/frames/motd", 12)==0) {
		mod_html_motd(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/frames/temp", 12)==0) {
		mod_html_reloadframe(sid);
	}
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_html", "", "", "mod_main", "/frames/", mod_main)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_header",      mod_html_header)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_footer",      mod_html_footer)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_login",       mod_html_login)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_logout",      mod_html_logout)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_motd",        mod_html_motd)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_frameset",    mod_html_frameset)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_menuframe",   mod_html_menuframe)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_reloadframe", mod_html_reloadframe)!=0) return -1;
	if (mod_export_function("mod_html", "mod_html_topmenu",     mod_html_topmenu)!=0) return -1;
	return 0;
}