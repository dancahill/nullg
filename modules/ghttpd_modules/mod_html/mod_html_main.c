/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "ghttpd/mod.h"
#include "mod_html.h"

void mod_html_header(CONN *sid, char *title)
{
	short int mailwrite=0;

	if (strncmp(sid->dat->in_RequestURI, "/mail/write", 11)==0) mailwrite=1;
	prints(sid,
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n"
		"<HTML>\r\n"
		"<HEAD>\r\n"
		"<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=iso-8859-1\">\r\n"
		"<TITLE>%s</TITLE>\r\n"
		"<STYLE TYPE=text/css>\r\n"
		".JUSTIFY { text-align: justify; }\r\n"
		".TBAR    { color: #505050; text-decoration: none; font-family: Geneva, Arial, Verdana; font-size: 8pt; }\r\n"
		"A        { color: #0000FF; text-decoration: none; }\r\n"
		"A:HOVER  { text-decoration: underline; }\r\n"
		"</STYLE>\r\n"
		"<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/groupware/themes/%s/style.css\">\r\n"
		"%s"
		"</HEAD>\r\n"
		"<BODY BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\" MARGINHEIGHT=0 MARGINWIDTH=0 TOPMARGIN=0 LEFTMARGIN=0 CLASS=\"MAINBACK\"%s>\r\n"
		, title
		, sid->dat->user_theme
		, mailwrite?"<SCRIPT LANGUAGE=\"JavaScript\" SRC=\"/groupware/javascript/wmedit.js\" TYPE=\"text/javascript\"></SCRIPT>\r\n":""
		, mailwrite?" onLoad=init();":""
	);
	return;
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
	int i;

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
		case MENU_PROJECTS:  mod_html_header(sid, "NullLogic Groupware Projects"); break;
		case MENU_SEARCHES:  mod_html_header(sid, "NullLogic Groupware Searches"); break;
		case MENU_TASKS:     mod_html_header(sid, "NullLogic Groupware Tasks"); break;
		case MENU_WEBMAIL:   mod_html_header(sid, "NullLogic Groupware Webmail"); break;
		case MENU_XMLRPC:    mod_html_header(sid, "NullLogic Groupware XML-RPC"); break;
		default: mod_html_header(sid, "NullLogic Groupware"); break;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "function ShowHelp()\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	window.open('/groupware/help/%s/", sid->dat->user_language);
	switch (menu) {
		case MENU_ADMIN:     prints(sid, "ch-02.html"); break;
		case MENU_BOOKMARKS: prints(sid, "ch-03.html"); break;
		case MENU_CALENDAR:  prints(sid, "ch-04.html"); break;
		case MENU_CALLS:     prints(sid, "ch-05.html"); break;
		case MENU_CONTACTS:  prints(sid, "ch-06.html"); break;
		case MENU_WEBMAIL:   prints(sid, "ch-07.html"); break;
		case MENU_FILES:     prints(sid, "ch-08.html"); break;
		case MENU_FORUMS:    prints(sid, "ch-09.html"); break;
		case MENU_MAIN:      prints(sid, "index.html"); break;
		case MENU_NOTES:     prints(sid, "ch-11.html"); break;
		case MENU_ORDERS:    prints(sid, "ch-12.html"); break;
		case MENU_PROFILE:   prints(sid, "ch-13.html"); break;
		case MENU_SEARCHES:  prints(sid, "ch-14.html"); break;
		case MENU_TASKS:     prints(sid, "ch-04.html"); break;
		default: prints(sid, "index.html"); break;
	}
	prints(sid, "','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=780,height=550');\r\n");
	prints(sid, "}\r\n");
	if ((menu==MENU_ADMIN)||(menu==MENU_CONTACTS)||(menu==MENU_SEARCHES)) {
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	} else if (menu==MENU_FILES) {
		prints(sid, "function FileUpload() {\r\n");
		if ((ptemp=getgetenv(sid, "LOCATION"))==NULL) ptemp=sid->dat->in_RequestURI;
		prints(sid, "	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');\r\n", sid->dat->in_ScriptName, ptemp);
		prints(sid, "}\r\n");
	} else if (menu==MENU_WEBMAIL) {
		prints(sid, "function ComposeMail() {\r\n");
		prints(sid, "	window.open('%s/mail/write','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName);
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
		if (strncmp(sid->dat->in_RequestURI, "/mail/search", 12)==0) goto domenu;
		if (strncmp(sid->dat->in_RequestURI, "/mail/sync", 10)==0) goto domenu;
		return;
	}
domenu:
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	if (sid->dat->user_menustyle==0) {
		prints(sid, "<TR><TD STYLE='border-style:solid'><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle CLASS=\"TBAR\"><TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
		for (i=0;;i++) {
			if (strlen(http_proc->mod_menuitems[i].mod_name)==0) break;
			if (strlen(http_proc->mod_menuitems[i].mod_menuname)==0) continue;
			if (strlen(http_proc->mod_menuitems[i].mod_menuperm)!=0) {
				if (!auth_priv(sid, http_proc->mod_menuitems[i].mod_menuperm)) continue;
			}
			prints(sid, "%s<A CLASS='TBAR' HREF=", dot?"&nbsp;&middot;&nbsp;":"");
			if (strncasecmp(http_proc->mod_menuitems[i].mod_menuuri, "javascript:", 11)==0) {
				prints(sid, "%s", http_proc->mod_menuitems[i].mod_menuuri);
			} else {
				prints(sid, "\"%s%s\"", sid->dat->in_ScriptName, http_proc->mod_menuitems[i].mod_menuuri);
			}
			prints(sid, ">%s</A>", http_proc->mod_menuitems[i].mod_menuname);
			dot=1;
		}
		prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
		prints(sid, "<A CLASS='TBAR' HREF=javascript:ShowHelp()>%s</A>&nbsp;&middot;&nbsp;", lang.menu_top_help);
		if (module_exists("mod_profile")) {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, lang.menu_top_logout);
		prints(sid, "&nbsp;&nbsp;</TD>\r\n");
		prints(sid, "</TR></TABLE>\r\n</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD STYLE='border-style:solid'><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle CLASS=\"TBAR\">\r\n");
	prints(sid, "<TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
	switch (menu) {
		case MENU_ADMIN:
			if (auth_priv(sid, "domainadmin")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/admin/configedit>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_config);
			}
			if (auth_priv(sid, "domainadmin")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/admin/domainlist>DOMAINS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/activitylist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_logs);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/syscheck>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_check);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/userlist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_users);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/grouplist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_groups);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/zonelist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_zones);
			break;
		case MENU_ACCOUNTING:
			prints(sid, "<A CLASS='TBAR' HREF=%s/accounting/accounts>ACCOUNTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/accounting/journal>JOURNAL</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
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
		case MENU_PROJECTS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/projects/list>PROJECTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/projects/editnew>NEW PROJECT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_SEARCHES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/search/>SEARCHES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			if (auth_priv(sid, "domainadmin")&A_ADMIN) {
				if (auth_priv(sid, "query")&A_ADMIN) {
					prints(sid, "<A CLASS='TBAR' HREF=%s/search/sqlrun>SQL QUERIES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				}
			}
			break;
		case MENU_WEBMAIL:
			if (sid->dat->user_menustyle>0) {
				if (sid->dat->user_mailcurrent>0) {
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/main TARGET=gwmain>INBOX</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
					prints(sid, "<A CLASS='TBAR' HREF=javascript:ComposeMail()>COMPOSE</A>&nbsp;&middot;&nbsp;");
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/sync TARGET=gwmain onClick=\"parent.location.replace('%s/mail/sync');return false;\">SEND/RECV</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/search TARGET=gwmain>SEARCH</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				}
				prints(sid, "<A CLASS='TBAR' HREF=%s/mail/accounts/list TARGET=gwmain>ACCOUNTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				if (sid->dat->user_mailcurrent>0) {
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/list>INBOX</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/write>COMPOSE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/sync>SEND/RECV</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
					prints(sid, "<A CLASS='TBAR' HREF=%s/mail/search>SEARCH</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				}
				prints(sid, "<A CLASS='TBAR' HREF=%s/mail/accounts/list>ACCOUNTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			if (sid->dat->user_mailcurrent>0) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/mail/quit%s>QUIT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=gwmain":"");
			}
			break;
	}
	prints(sid, "<A CLASS='TBAR' HREF=javascript:window.print()>%s</A>", lang.menu_top_print);
	prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
	if (sid->dat->user_menustyle>0) {
		prints(sid, "<A CLASS='TBAR' HREF=javascript:ShowHelp()>%s</A>&nbsp;&middot;&nbsp;", lang.menu_top_help);
		if (menu==MENU_WEBMAIL) {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit TARGET=gwmain>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		} else {
			prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, lang.menu_top_logout);
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
	char domain[64];
	char *ptemp;
	FILE *fp;
	int ich;
	SQLRES sqr;

	memset(pageuri, 0, sizeof(pageuri));
	memset(domain, 0, sizeof(domain));
	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s%s", sid->dat->in_ScriptName, sid->dat->in_RequestURI);
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	mod_html_header(sid, "NullLogic Groupware Login");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\nif (self!=parent) open('%s/','_top');\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<BR>\r\n");
	snprintf(file, sizeof(file)-1, "%s/issue.txt", proc->config.dir_etc);
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
	prints(sid, "<TR><TH COLSPAN=2 STYLE='padding:1px'>%s Login</TH></TR>\r\n", SERVER_NAME);
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	if (sql_query(&sqr, "SELECT username, password FROM gw_users WHERE userid = 1")<0) return;
	if (sql_numtuples(&sqr)==1) {
		if (strcmp("$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/", sql_getvalue(&sqr, 0, 1))==0) {
			snprintf(username, sizeof(username)-1, "%s", sql_getvalue(&sqr, 0, 0));
			snprintf(password, sizeof(password)-1, "visual");
		}
	}
	sql_freeresult(&sqr);
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
	if (sql_query(&sqr, "SELECT COUNT(*) FROM gw_domains")<0) return;
	if (atoi(sql_getvalue(&sqr, 0, 0))>1) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Domain&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=domain SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", domain);
	} else {
		prints(sid, "<INPUT TYPE=hidden NAME=domain VALUE=\"NULL\">\r\n");
	}
	sql_freeresult(&sqr);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=SUBMIT VALUE='Login'></TD></TR>\r\n");
	prints(sid, "</FORM>\r\n</TABLE>\r\n");
	if (strcmp(password, "visual")==0) {
		prints(sid, "<BR><TABLE>\r\n<TR><TD><FONT COLOR=RED><PRE>\r\n");
		prints(sid, "You have not yet changed the administrator's password.\r\n");
		prints(sid, "Please do this as soon as possible for security reasons.\r\n");
		prints(sid, "</PRE></FONT></TD></TR>\r\n</TABLE>\r\n");
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
	db_log_activity(sid, "login", 0, "logout", "%s - Logout: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	mod_html_header(sid, "NullLogic Groupware Logout");
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TH>NullLogic Groupware Logout</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"FIELDVAL\"><TD>\r\n");
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
	HTMOD_TASKS_LIST mod_tasks_list;
	char posttime1[32];
	char posttime2[32];
	char showtime[32];
	time_t t, t2;
	time_t unixdate;
	int newmessages;
	int i, j, k;
	SQLRES sqr1;
	SQLRES sqr2;

	t=(time(NULL)+time_tzoffset(sid, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	mod_html_topmenu(sid, MENU_MAIN);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n<TABLE WIDTH=90%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR><TD COLSPAN=2>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'><TR><TH STYLE='border-style:solid'>\n");
	prints(sid, "<TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left>Welcome, %s.&nbsp;</TH>\r\n", str2html(sid, sid->dat->user_username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right>&nbsp;%s</TH>\r\n", showtime);
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</TH></TR>\r\n");
	if (sql_queryf(&sqr1, "SELECT motd FROM gw_groups where groupid = %d", sid->dat->user_gid)<0) return;
	if (sql_numtuples(&sqr1)==1) {
		if (strlen(sql_getvalue(&sqr1, 0, 0))>0) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>\r\n");
			prints(sid, "%s", sql_getvalue(&sqr1, 0, 0));
			prints(sid, "<BR></TD></TR>\r\n");
		}
	}
	sql_freeresult(&sqr1);
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
	if (sql_queryf(&sqr1, "SELECT eventid, eventstart, eventfinish, eventname FROM gw_events where status = 0 and eventstart >= '%s' and eventstart < '%s' and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid)<0) return;
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT COLSPAN=2 NOWRAP VALIGN=TOP STYLE='border-style:solid'>Calendar</TH></TR>\n");
	for (i=0;i<2;i++) {
		t=unixdate+(i*86400);
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'><B><A HREF=%s/calendar/list?day=%d>", sid->dat->in_ScriptName, (int)(t/86400));
		if (i==0) {
			prints(sid, "Today");
		} else if (i==1) {
			prints(sid, "Tomorrow");
		}
		prints(sid, "</A></B></TD></TR>\n");
		k=1;
		for (j=0;j<sql_numtuples(&sqr1);j++) {
			t2=time_sql2unix(sql_getvalue(&sqr1, j, 1));
			if ((t2<t)||(t2>=t+86400)) continue;
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'><FONT SIZE=2>");
			t2=time_sql2unix(sql_getvalue(&sqr1, j, 1))+time_tzoffset(sid, time_sql2unix(sql_getvalue(&sqr1, j, 1)));
			prints(sid, "<A HREF=%s/calendar/edit?eventid=%s>%s", sid->dat->in_ScriptName, sql_getvalue(&sqr1, j, 0), time_unix2timetext(sid, t2));
			t2=time_sql2unix(sql_getvalue(&sqr1, j, 2))+time_tzoffset(sid, time_sql2unix(sql_getvalue(&sqr1, j, 2)));
			prints(sid, " - %s</A></FONT></TD><TD NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>", time_unix2timetext(sid, t2));
			prints(sid, "<A HREF=%s/calendar/edit?eventid=%s>%s</A>&nbsp;", sid->dat->in_ScriptName, sql_getvalue(&sqr1, j, 0), str2html(sid, sql_getvalue(&sqr1, j, 3)));
			prints(sid, "</FONT></TD></TR>\n");
			if (k>0) k--;
		}
		while (k>0) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT SIZE=2>&nbsp;</FONT></TD></TR>\n");
			k--;
		}
	}
	sql_freeresult(&sqr1);
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD><TD VALIGN=TOP>\n");
	if (module_exists("mod_mail")&&(auth_priv(sid, "webmail")>0)) {
		if (sql_queryf(&sqr1, "SELECT mailaccountid, accountname FROM gw_mailaccounts where obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid)<0) return;
		if (sql_numtuples(&sqr1)>0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
			prints(sid, "<FONT SIZE=2>&nbsp;E-Mail</FONT></TH></TR>\n");
			for (i=0;i<sql_numtuples(&sqr1);i++) {
				if (sql_queryf(&sqr2, "SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status = 'n'", sid->dat->user_uid, atoi(sql_getvalue(&sqr1, i, 0)))<0) continue;
				newmessages=atoi(sql_getvalue(&sqr2, 0, 0));
				sql_freeresult(&sqr2);
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD WIDTH=100%% STYLE='border-style:solid'><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT NOWRAP>");
				if (sid->dat->user_menustyle>0) {
					prints(sid, "<A HREF=%s/mail/main?accountid=%d TARGET=gwmain>%-.25s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), str2html(sid, sql_getvalue(&sqr1, i, 1)));
				} else {
					prints(sid, "<A HREF=%s/mail/list?accountid=%d>%-.25s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), str2html(sid, sql_getvalue(&sqr1, i, 1)));
				}
				prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s%d New%s</TD></TR>\n</TABLE></TD></TR>\n", newmessages?"<FONT COLOR=BLUE><B>":"", newmessages, newmessages?"</B></FONT>":"");
			}
			prints(sid, "</TABLE>\n<BR>\n");
		}
		sql_freeresult(&sqr1);
	}
	if ((mod_tasks_list=module_call("mod_tasks_list"))!=NULL) {
		mod_tasks_list(sid, sid->dat->user_uid, -1);
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void mod_html_frameset(CONN *sid)
{
//	MOD_MAIL_SYNC mod_mail_sync;
//	int i, j;
//	SQLRES sqr1;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
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
/*
	if ((mod_mail_sync=module_call("mod_mail_sync"))!=NULL) {
		if ((&sqr1=sql_queryf(sid, "SELECT mailaccountid, accountname, poppassword, lastcount, notify, lastcheck FROM gw_mailaccounts where obj_uid = %d and notify > 0", sid->dat->user_uid))>-1) {
			for (i=0;i<sql_numtuples(&sqr1);i++) {
				sid->dat->user_mailcurrent=atoi(sql_getvalue(&sqr1, i, 0));
				j=time_sql2unix(sql_getvalue(&sqr1, i, 5));
				if ((strlen(sql_getvalue(&sqr1, i, 2))>0)&&(j+(atoi(sql_getvalue(&sqr1, i, 4))*60)<time(NULL))) {
					mod_mail_sync(sid, 0);
				}
			}
			sql_freeresult(&sqr1);
		}
	}
*/
	prints(sid, "-->\r\n");
}

void mod_html_menuframe(CONN *sid)
{
	int i, j;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware Menu</TITLE>\r\n");
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/groupware/themes/%s/style.css\">\r\n", sid->dat->user_theme);
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
		prints(sid, "<BODY BGCOLOR=#808080 TEXT=#000000 LINK=#FFFFFF ALINK=#FFFFFF VLINK=#FFFFFF CLASS=\"MENUBACK\" onContextMenu='return false'>\r\n");
		prints(sid, "<CENTER>\r\n");
		prints(sid, "<A HREF=%s/frames/motd target='gwmain' onclick=\"window.open('%s/frames/menu','gwmenu')\"><IMG BORDER=0 SRC=/groupware/images/groupware.png HEIGHT=50 WIDTH=125 ALT='%s logged in'></A><BR><BR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName, sid->dat->user_username);
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=7 WIDTH=100%%>\r\n");
		for (i=0;;i++) {
			if (strlen(http_proc->mod_menuitems[i].mod_name)==0) break;
			if (strlen(http_proc->mod_menuitems[i].mod_menuname)==0) continue;
			if (strlen(http_proc->mod_menuitems[i].mod_menuperm)!=0) {
				if (!auth_priv(sid, http_proc->mod_menuitems[i].mod_menuperm)) continue;
			}
			// Mozilla treats this as double click.  Bad gecko.
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=");
			if (strncasecmp(http_proc->mod_menuitems[i].mod_menuuri, "javascript:", 11)==0) {
				prints(sid, "%s><A HREF=%s CLASS='MENULINK'>", http_proc->mod_menuitems[i].mod_menuuri, http_proc->mod_menuitems[i].mod_menuuri);
			} else {
				prints(sid, "\"parent.gwmain.location.href='%s%s'\"><A HREF=%s%s TARGET=gwmain CLASS='MENULINK'>", sid->dat->in_ScriptName, http_proc->mod_menuitems[i].mod_menuuri, sid->dat->in_ScriptName, http_proc->mod_menuitems[i].mod_menuuri);
			}
			prints(sid, "<FONT SIZE=2><B>%s</B></FONT></A></TD></TR>\r\n", http_proc->mod_menuitems[i].mod_menuname);
		}
		prints(sid, "</TABLE>\r\n");
		prints(sid, "</CENTER>\r\n</BODY>\r\n</HTML>\r\n");
	} else if (sid->dat->user_menustyle==2) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript SRC=/groupware/javascript/outlook.js TYPE=text/javascript></SCRIPT>\r\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\n");
		prints(sid, "var o = new createOutlookBar('Bar',0,0,screenSize.width,screenSize.height,'#404040','white')\n");
		prints(sid, "var p\n");
		for (i=1;i<4;i++) {
			if (i==1) {
				prints(sid, "//create first panel\n");
				prints(sid, "p = new createPanel('c','Communication');\n");
			} else if (i==2) {
				prints(sid, "o.addPanel(p);\n");
				prints(sid, "p = new createPanel('i','Information');\n");
			} else if (i==3) {
				prints(sid, "o.addPanel(p);\n");
				prints(sid, "p = new createPanel('r','Resources');\n");
			}
			for (j=0;;j++) {
				if (strlen(http_proc->mod_menuitems[j].mod_name)==0) break;
				if (http_proc->mod_menuitems[j].mod_submenu!=i) continue;
				if (strlen(http_proc->mod_menuitems[j].mod_menuname)==0) continue;
				if (strlen(http_proc->mod_menuitems[j].mod_menuperm)!=0) {
					if (!auth_priv(sid, http_proc->mod_menuitems[j].mod_menuperm)) continue;
				}
				prints(sid, "p.addButton('/groupware/images/menu2/");
				if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_admin")==0)          prints(sid, "admin.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_bookmarks")==0) prints(sid, "bookmarks.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_calendar")==0)  prints(sid, "calendar.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_calls")==0)     prints(sid, "default.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_contacts")==0)  prints(sid, "contacts.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_mail")==0)      prints(sid, "email.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_forums")==0)    prints(sid, "default.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_messages")==0)  prints(sid, "messenger.png");
				else if (strcmp(http_proc->mod_menuitems[j].mod_name, "mod_orders")==0)    prints(sid, "orders.png");
				else prints(sid, "default.png");
				if (strncasecmp(http_proc->mod_menuitems[j].mod_menuuri, "javascript:", 11)==0) {
					prints(sid, "', '%s', '%s');\n", http_proc->mod_menuitems[j].mod_menuname, http_proc->mod_menuitems[j].mod_menuuri);
				} else {
					prints(sid, "', '%s', 'parent.gwmain.location=\"%s%s\"');\n", http_proc->mod_menuitems[j].mod_menuname, sid->dat->in_ScriptName, http_proc->mod_menuitems[j].mod_menuuri);
				}
			}
		}
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "o.draw(); //draw the OutlookBar\n");
		prints(sid, "o.showPanel(0);\n");
		prints(sid, "// -->\r\n</SCRIPT>\r\n");
		prints(sid, "</HEAD>\r\n");
		prints(sid, "<BODY BGCOLOR=#000050 TEXT=#000000 LINK=#FFFFFF ALINK=#FFFFFF VLINK=#FFFFFF CLASS=\"MENUBACK\" onContextMenu='return false' onLoad=resize_op5(); onResize=myOnResize();>\r\n");
		prints(sid, "</BODY>\r\n</HTML>\r\n");
	}
	return;
}

void mod_html_reloadframe(CONN *sid)
{
	HTMOD_MAIL_SYNC mod_mail_sync;
	char posttime[32];
	int isreminder;
	int i, j;
	int a, b;
	SQLRES sqr1;
	SQLRES sqr2;
	time_t t;
	int notice=0;

	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	sql_updatef("UPDATE gw_usersessions SET obj_mtime = '%s' WHERE token = '%s' AND userid = %d AND domainid = %d", posttime, sid->dat->user_token, sid->dat->user_uid, sid->dat->user_did);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware</TITLE>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"60; URL=%s/frames/temp\">\r\n</HEAD>\r\n", sid->dat->in_ScriptName);
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (sql_queryf(&sqr1, "SELECT eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 AND obj_did = %d ORDER BY eventstart ASC", posttime, sid->dat->user_uid, sid->dat->user_did)<0) return;
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			a=time_sql2unix(sql_getvalue(&sqr1, i, 0))-time(NULL);
			b=a-atoi(sql_getvalue(&sqr1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/calendar/reminders','calendarwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sql_freeresult(&sqr1);
	}
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (sql_queryf(&sqr1, "SELECT duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 AND obj_did = %d ORDER BY duedate ASC", posttime, sid->dat->user_uid, sid->dat->user_did)<0) return;
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			a=time_sql2unix(sql_getvalue(&sqr1, i, 0))-time(NULL);
			a-=time_tzoffset(sid, time_sql2unix(sql_getvalue(&sqr1, i, 0)));
			b=a-atoi(sql_getvalue(&sqr1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/tasks/reminders','taskwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
		sql_freeresult(&sqr1);
	}
	if (auth_priv(sid, "messages")&A_READ) {
		t=time(NULL)-10;
		strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
		if (sql_queryf(&sqr1, "SELECT messageid, sender FROM gw_messages WHERE obj_uid = %d AND rcpt = %d and status > 2 AND obj_ctime < '%s' AND obj_did = %d", sid->dat->user_uid, sid->dat->user_uid, posttime, sid->dat->user_did)<0) return;
		if (sql_numtuples(&sqr1)>0) {
			if (sql_queryf(&sqr2, "SELECT userid, username FROM gw_users WHERE obj_did = %d", sid->dat->user_did)<0) {
				sql_freeresult(&sqr1);
				return;
			}
			for (i=0;i<sql_numtuples(&sqr2);i++) {
				for (j=0;j<sql_numtuples(&sqr1);j++) {
					if (atoi(sql_getvalue(&sqr2, i, 0))==atoi(sql_getvalue(&sqr1, j, 1))) {
						prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
						prints(sid, "window.open('%s/messages/frame?userid=%d','msgwin_%s','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=yes,width=400,height=200');\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr2, i, 0)), sql_getvalue(&sqr2, i, 1));
						prints(sid, "// -->\n</SCRIPT>\n");
					}
				}
			}
			sql_freeresult(&sqr2);
		}
		sql_freeresult(&sqr1);
	}
	if ((mod_mail_sync=module_call("mod_mail_sync"))!=NULL) {
		notice=0;
		if (sql_queryf(&sqr1, "SELECT mailaccountid, accountname, poppassword, lastcount, notify, lastcheck FROM gw_mailaccounts where obj_uid = %d and notify > 0 AND obj_did = %d", sid->dat->user_uid, sid->dat->user_did)<0) return;
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			sid->dat->user_mailcurrent=atoi(sql_getvalue(&sqr1, i, 0));
			j=time_sql2unix(sql_getvalue(&sqr1, i, 5));
			if ((strlen(sql_getvalue(&sqr1, i, 2))>0)&&(j+(atoi(sql_getvalue(&sqr1, i, 4))*60)<time(NULL))) {
				mod_mail_sync(sid, 0);
			}
			if (sql_queryf(&sqr2, "SELECT mailheaderid FROM gw_mailheaders where obj_uid = %d AND obj_did = %d AND accountid = %d AND status = 'n'", sid->dat->user_uid, sid->dat->user_did, sid->dat->user_mailcurrent)<0) break;
			if (sql_numtuples(&sqr2)>0) notice=1;
			sql_freeresult(&sqr2);
		}
		sql_freeresult(&sqr1);
		if (notice) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "window.open('%s/mail/notice','mailwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=200');\n", sid->dat->in_ScriptName);
			prints(sid, "// -->\n</SCRIPT>\n");
		}
	}
	prints(sid, "<BODY BGCOLOR=#000050>\r\n</BODY>\r\n</HTML>\r\n");
}

DllExport int mod_main(CONN *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/frames/menu", 12)==0) {
		mod_html_menuframe(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/frames/motd", 12)==0) {
		mod_html_motd(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/frames/temp", 12)==0) {
		mod_html_reloadframe(sid);
	} else {
		mod_html_motd(sid);
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
		"mod_html",		// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/frames/",		// fn_uri
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
