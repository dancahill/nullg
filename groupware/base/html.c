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

void htpage_login(CONNECTION *sid)
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
	htpage_header(sid, "NullLogic Groupware Login");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR>\r\n");
	snprintf(file, sizeof(file)-1, "%s/issue.txt", config.server_etc_dir);
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
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=\"%s/\" AUTOCOMPLETE=OFF NAME=login>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=pageuri VALUE=\"%s\">\r\n", pageuri);
	prints(sid, "<TR BGCOLOR=\"%s\"><TH COLSPAN=2><FONT COLOR=%s>NullLogic Groupware Login</FONT></TH></TR>\r\n", COLOR_TH, COLOR_THTEXT);
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	if ((sqr=sqlQuery(sid, "SELECT username, password FROM gw_users WHERE userid = 1"))<0) return;
	if (sqlNumtuples(sqr)==1) {
		if (strcmp("$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/", sqlGetvalue(sqr, 0, 1))==0) {
			snprintf(username, sizeof(username)-1, "%s", sqlGetvalue(sqr, 0, 0));
			snprintf(password, sizeof(password)-1, "visual");
		}
	}
	sqlFreeconnect(sqr);
	if (strlen(username)==0) {
		snprintf(username, sizeof(username)-1, "%s", sid->dat->user_username);
	}
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>Username</B></TD><TD><INPUT TYPE=TEXT NAME=username SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", COLOR_EDITFORM, username);
	prints(sid, "<TR BGCOLOR=\"%s\"><TD><B>Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=password SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", COLOR_EDITFORM, password);
	prints(sid, "<TR BGCOLOR=\"%s\"><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\r\n", COLOR_EDITFORM);
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

void htpage_logout(CONNECTION *sid)
{
	logaccess(sid, 0, "%s - Logout: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_header(sid, "NullLogic Groupware Logout");
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH><FONT COLOR=%s>NullLogic Groupware Logout</FONT></TH></TR>\r\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD>\r\n", COLOR_FVAL);
	prints(sid, "You have successfully logged out.\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('%s/');\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='%s/'\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void htpage_stats(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Start Time </FONT></TD><TD>%s</TD></TR>\r\n", COLOR_FNAME, COLOR_FVALTEXT, time_unix2text(sid, stats.starttime+time_tzoffset(sid, time(NULL))));
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Pages      </FONT></TD><TD>%d</TD></TR>\r\n", COLOR_FNAME, COLOR_FVALTEXT, stats.pages);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Connections</FONT></TD><TD>%d</TD></TR>\r\n", COLOR_FNAME, COLOR_FVALTEXT, stats.conns);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Queries    </FONT></TD><TD>%d</TD></TR>\r\n", COLOR_FNAME, COLOR_FVALTEXT, stats.queries);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Updates    </FONT></TD><TD>%d</TD></TR>\r\n", COLOR_FNAME, COLOR_FVALTEXT, stats.updates);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void htpage_frameset(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Frameset//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware</TITLE>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<FRAMESET COLS=\"125,*\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n");
	prints(sid, "<FRAMESET ROWS=\"*,0\"   BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n");
	prints(sid, "<FRAME BORDER=0 NAME=\"gwmenu\" SRC=%s/main/menu MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<FRAME BORDER=0 NAME=\"gwtemp\" SRC=%s/main/temp MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</FRAMESET>\r\n");
	prints(sid, "<FRAME BORDER=0 NAME=\"gwmain\" SRC=%s/main/main MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=YES>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</FRAMESET>\r\n");
	prints(sid, "To view this page, you need a web browser capable of displaying frames.\r\n");
	prints(sid, "</HTML>\r\n");
}

void htpage_sidemenu(CONNECTION *sid)
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
	prints(sid, "function ListMessages() {\n");
	prints(sid, "	window.open('%s/messages/list','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
	prints(sid, "}\n");
	prints(sid, "function doreminder() {\n");
	prints(sid, "	window.parent.gwtemp.location.replace(\"%s/main/temp\");\n", sid->dat->in_ScriptName);
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
		prints(sid, "<A HREF=%s/main/main target='gwmain' onclick=\"window.open('%s/main/menu','gwmenu')\"><IMG BORDER=0 SRC=/groupware/images/groupware.gif HEIGHT=50 WIDTH=125 ALT='%s logged in'></A><BR><BR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName, sid->dat->user_username);
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=7 WIDTH=100%%>\r\n");
		if (auth_priv(sid, AUTH_ADMIN)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/admin/'\"><A HREF=%s/admin/ TARGET=gwmain><FONT SIZE=2><B>ADMINISTRATION</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_BOOKMARKS)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/bookmarks/list'\"><A HREF=%s/bookmarks/list TARGET=gwmain><FONT SIZE=2><B>BOOKMARKS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CALENDAR)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/calendar/list'\"><A HREF=%s/calendar/list TARGET=gwmain><FONT SIZE=2><B>CALENDAR</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CALLS)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/calls/list'\"><A HREF=%s/calls/list TARGET=gwmain><FONT SIZE=2><B>CALLS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CONTACTS)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/contacts/list'\"><A HREF=%s/contacts/list TARGET=gwmain><FONT SIZE=2><B>CONTACTS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_WEBMAIL)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/mail/main'\"><A HREF=%s/mail/main TARGET=gwmain><FONT SIZE=2><B>E-MAIL</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FILES)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/files/'\"><A HREF=%s/files/ TARGET=gwmain><FONT SIZE=2><B>FILES</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FORUMS)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/forums/list'\"><A HREF=%s/forums/list TARGET=gwmain><FONT SIZE=2><B>FORUMS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_MESSAGES)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=ListMessages();><A HREF=javascript:ListMessages()><FONT SIZE=2><B>MESSAGES</B></FONT></A></TD></TR>\r\n");
		}
		prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/notes/list'\"><A HREF=%s/notes/list TARGET=gwmain><FONT SIZE=2><B>NOTEBOOK</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		if (auth_priv(sid, AUTH_ORDERS)>0) {
			prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/orders/list'\"><A HREF=%s/orders/list TARGET=gwmain><FONT SIZE=2><B>ORDERS</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		}
		prints(sid, "<TR><TD style=\"cursor:hand\" onClick=\"parent.gwmain.location.href='%s/search/'\"><A HREF=%s/search/ TARGET=gwmain><FONT SIZE=2><B>SEARCHES</B></FONT></A></TD></TR>\r\n", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
		prints(sid, "</TABLE>\r\n");
		prints(sid, "</CENTER>\r\n</BODY>\r\n</HTML>\r\n");
	} else if (sid->dat->user_menustyle==2) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript SRC=/groupware/javascript/outlook.js TYPE=text/javascript></SCRIPT>\r\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\n");
		prints(sid, "var o = new createOutlookBar('Bar',0,0,screenSize.width,screenSize.height,'#404040','white')\n");
		prints(sid, "var p\n");
		prints(sid, "//create first panel\n");
		prints(sid, "p = new createPanel('al','Communication');\n");
		if (auth_priv(sid, AUTH_CALLS)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/default.gif',  'Calls',         'parent.gwmain.location=\"%s/calls/list\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_WEBMAIL)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/email.gif',    'E-Mail',        'parent.gwmain.location=\"%s/mail/main\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FORUMS)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/default.gif',  'Forums',        'parent.gwmain.location=\"%s/forums/list\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_MESSAGES)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/messenger.gif','Messages',      'ListMessages()');\n");
		}
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "p = new createPanel('p','Information');\n");
		if (auth_priv(sid, AUTH_CALENDAR)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/calendar.gif', 'Calendar',      'parent.gwmain.location=\"%s/calendar/list\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CONTACTS)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/contacts.gif', 'Contacts',      'parent.gwmain.location=\"%s/contacts/list\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_ORDERS)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/orders.gif',   'Orders',        'parent.gwmain.location=\"%s/orders/list\"');\n", sid->dat->in_ScriptName);
		}
		prints(sid, "p.addButton('/groupware/images/menu2/searches.gif', 'Searches',      'parent.gwmain.location=\"%s/search/\"');\n", sid->dat->in_ScriptName);
		prints(sid, "o.addPanel(p);\n");
		prints(sid, "p = new createPanel('l','Resources');\n");
		if (auth_priv(sid, AUTH_ADMIN)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/admin.gif',    'Administration','parent.gwmain.location=\"%s/admin/\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_BOOKMARKS)>0) {
			prints(sid, "p.addButton('/groupware/images/menu2/bookmarks.gif','Bookmarks',     'parent.gwmain.location=\"%s/bookmarks/list\"');\n", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FILES)>0) {
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

void htpage_topmenu(CONNECTION *sid, int menu)
{
	char *ptemp;
	int dotted=0;
	int folderid=0;

	switch (menu) {
		case MENU_ADMIN:
			htpage_header(sid, "NullLogic Groupware Administration"); break;
		case MENU_BOOKMARKS:
			htpage_header(sid, "NullLogic Groupware Bookmarks"); break;
		case MENU_CALENDAR:
			htpage_header(sid, "NullLogic Groupware Calendar"); break;
		case MENU_CALLS:
			htpage_header(sid, "NullLogic Groupware Calls"); break;
		case MENU_CONTACTS:
			htpage_header(sid, "NullLogic Groupware Contacts"); break;
		case MENU_FILES:
			htpage_header(sid, "NullLogic Groupware Files"); break;
		case MENU_FORUMS:
			htpage_header(sid, "NullLogic Groupware Forums"); break;
		case MENU_MAIN:
			htpage_header(sid, "NullLogic Groupware Welcome"); break;
		case MENU_NOTES:
			htpage_header(sid, "NullLogic Groupware Notebook"); break;
		case MENU_ORDERS:
			htpage_header(sid, "NullLogic Groupware Orders"); break;
		case MENU_PROFILE:
			htpage_header(sid, "NullLogic Groupware User Profile"); break;
		case MENU_SEARCHES:
			htpage_header(sid, "NullLogic Groupware Searches"); break;
		case MENU_TASKS:
			htpage_header(sid, "NullLogic Groupware Tasks"); break;
		case MENU_WEBMAIL:
			htpage_header(sid, "NullLogic Groupware Webmail"); break;
		case MENU_XMLRPC:
			htpage_header(sid, "NullLogic Groupware XML-RPC"); break;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "function ShowHelp()\r\n");
	prints(sid, "{\r\n");
	switch (menu) {
		case MENU_ADMIN:
			prints(sid, "	window.open('/groupware/help/ch02-00.html'"); break;
		case MENU_BOOKMARKS:
			prints(sid, "	window.open('/groupware/help/ch03-00.html'"); break;
		case MENU_CALENDAR:
			prints(sid, "	window.open('/groupware/help/ch04-00.html'"); break;
		case MENU_CALLS:
			prints(sid, "	window.open('/groupware/help/ch05-00.html'"); break;
		case MENU_CONTACTS:
			prints(sid, "	window.open('/groupware/help/ch06-00.html'"); break;
		case MENU_WEBMAIL:
			prints(sid, "	window.open('/groupware/help/ch07-00.html'"); break;
		case MENU_FILES:
			prints(sid, "	window.open('/groupware/help/ch08-00.html'"); break;
		case MENU_FORUMS:
			prints(sid, "	window.open('/groupware/help/ch09-00.html'"); break;
		case MENU_MAIN:
			prints(sid, "	window.open('/groupware/help/index.html'"); break;
		case MENU_NOTES:
			prints(sid, "	window.open('/groupware/help/ch11-00.html'"); break;
		case MENU_ORDERS:
			prints(sid, "	window.open('/groupware/help/ch12-00.html'"); break;
		case MENU_PROFILE:
			prints(sid, "	window.open('/groupware/help/ch13-00.html'"); break;
		case MENU_SEARCHES:
			prints(sid, "	window.open('/groupware/help/ch14-00.html'"); break;
		case MENU_TASKS:
			prints(sid, "	window.open('/groupware/help/ch04-00.html'"); break;
	}
	prints(sid, ",'helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\r\n");
	prints(sid, "}\r\n");
	if ((menu==MENU_CONTACTS)||(menu==MENU_SEARCHES)) {
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?msgto='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	} else if (menu==MENU_FILES) {
		prints(sid, "function FileUpload() {\r\n");
		if ((ptemp=getgetenv(sid, "LOCATION"))!=NULL) {
			prints(sid, "	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');\r\n", sid->dat->in_ScriptName, ptemp);
		} else {
			prints(sid, "	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');\r\n", sid->dat->in_ScriptName, sid->dat->in_RequestURI);
		}
		prints(sid, "}\r\n");
	} else if (menu==MENU_WEBMAIL) {
		prints(sid, "function ComposeMail() {\r\n");
		prints(sid, "	window.open('%s/mail/write','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?msgto='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function ReplyTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?replyto='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function Forward(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?forward='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	}
	if (sid->dat->user_menustyle==0) {
		prints(sid, "function ListMessages() {\n");
		prints(sid, "	window.open('%s/messages/list','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=300');\n", sid->dat->in_ScriptName);
		prints(sid, "}\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if ((menu==MENU_WEBMAIL)&&(sid->dat->user_menustyle>0)&&(strncmp(sid->dat->in_RequestURI, "/mail/list", 10)!=0)) return;
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if (sid->dat->user_menustyle==0) {
		prints(sid, "<TR><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;&nbsp;", COLOR_TMENU);
		if (auth_priv(sid, AUTH_ADMIN)>0) {
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/>ADMIN</A>", sid->dat->in_ScriptName);
			dotted=1;
		}
		if (auth_priv(sid, AUTH_BOOKMARKS)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/bookmarks/list>BOOKMARKS</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CALENDAR)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/list>CALENDAR</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CALLS)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/calls/list>CALLS</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_CONTACTS)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/list>CONTACTS</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_WEBMAIL)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/mail/list>E-MAIL</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FILES)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/files/>FILES</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_FORUMS)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/forums/list>FORUMS</A>", sid->dat->in_ScriptName);
		}
		if (auth_priv(sid, AUTH_MESSAGES)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=javascript:ListMessages()>MESSAGES</A>");
		}
		if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
		dotted=1;
		prints(sid, "<A CLASS='TBAR' HREF=%s/notes/list>NOTEBOOK</A>", sid->dat->in_ScriptName);
		if (auth_priv(sid, AUTH_ORDERS)>0) {
			if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
			dotted=1;
			prints(sid, "<A CLASS='TBAR' HREF=%s/orders/list>ORDERS</A>", sid->dat->in_ScriptName);
		}
		if (dotted==1) prints(sid, "&nbsp;&middot;&nbsp;");
		dotted=1;
		prints(sid, "<A CLASS='TBAR' HREF=%s/search/>SEARCHES</A>", sid->dat->in_ScriptName);
		prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
		prints(sid, "<A CLASS='TBAR' HREF=javascript:ShowHelp()>%s</A>&nbsp;&middot;&nbsp;", MENU_HELP);
		prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, MENU_LOGOUT);
		prints(sid, "&nbsp;&nbsp;</TD>\r\n");
		prints(sid, "</TR></TABLE>\r\n</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR VALIGN=middle BGCOLOR=%s>\r\n", COLOR_TMENU);
	prints(sid, "<TD ALIGN=left NOWRAP>&nbsp;&nbsp;");
	switch (menu) {
		case MENU_ADMIN:
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/access>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_ACCESS);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/error>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_ERROR);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/configedit>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_CONFIG);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/grouplist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_GROUPS);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/userlist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, ADM_MENU_USERS);
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
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/list>TODAY</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/listweek>WEEK</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/mlist>MONTH</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/ylist>YEAR</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calendar/editnew>NEW EVENT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/tasks/editnew>NEW TASK</a>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_CALLS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/calls/list>CALLS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/calls/editnew>NEW CALL</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_CONTACTS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/list>CONTACTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/contacts/editnew>NEW CONTACT</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_FILES:
			if ((ptemp=getgetenv(sid, "LOCATION"))==NULL) ptemp=sid->dat->in_RequestURI;
			prints(sid, "<A CLASS='TBAR' HREF=%s", sid->dat->in_ScriptName);
			printhex(sid, "%s", ptemp);
			prints(sid, ">REFRESH</A>&nbsp;&middot;&nbsp;");
			if (auth_priv(sid, AUTH_FILES)&A_MODIFY) {
				prints(sid, "<A CLASS='TBAR' HREF=javascript:FileUpload()>UPLOAD FILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_RequestURI);
				prints(sid, "<A CLASS='TBAR' HREF=%s/filemkdir?location=", sid->dat->in_ScriptName);
				printhex(sid, "%s", ptemp);
				prints(sid, ">NEW FOLDER</A>&nbsp;&middot;&nbsp;");
			}
			break;
		case MENU_FORUMS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/forums/list>LIST FORUMS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			if (auth_priv(sid, AUTH_FORUMS)&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/grouplist>GROUPS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/post>NEW FORUM</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_MAIN:
			prints(sid, "<A CLASS='TBAR' HREF=%s/main/main>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_NOTES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/list>LIST NOTES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/editnew>NEW NOTE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_ORDERS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/orders/list>VIEW ORDERS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/products/list>VIEW PRODUCTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
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

void htpage_reload(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic Groupware</TITLE>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"60; URL=%s/main/temp\">\r\n</HEAD>\r\n", sid->dat->in_ScriptName);
	messageautocheck(sid);
	prints(sid, "<BODY BGCOLOR=#000050>\r\n</BODY>\r\n</HTML>\r\n");
}

void htpage_motd(CONNECTION *sid)
{
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
	htpage_topmenu(sid, MENU_MAIN);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n<TABLE WIDTH=90%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR><TD COLSPAN=2>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%><TR><TH>\n");
	prints(sid, "<TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left><FONT COLOR=%s>Welcome, %s.&nbsp;</FONT></TH>\r\n", COLOR_THTEXT, str2html(sid, sid->dat->user_username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right><FONT COLOR=%s>&nbsp;%s</FONT></TH>\r\n", COLOR_THTEXT, showtime);
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</TH></TR>\r\n");
	if ((sqr=sqlQueryf(sid, "SELECT motd FROM gw_groups where groupid = %d", sid->dat->user_gid))<0) return;
	if (sqlNumtuples(sqr)==1) {
		if (strlen(sqlGetvalue(sqr, 0, 0))>0) {
			prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>\r\n", COLOR_FVAL);
			prints(sid, "%s", sqlGetvalue(sqr, 0, 0));
			prints(sid, "<BR></TD></TR>\r\n");
		}
	}
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE></TD></TR>\r\n");
	prints(sid, "<TR><TD VALIGN=TOP WIDTH=100%%>\r\n");
	if ((auth_priv(sid, AUTH_CALENDAR)&A_READ)<1) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	unixdate=(time(NULL)+time_tzoffset(sid, time(NULL)))/86400;
	if (unixdate<0) unixdate=0;
	if (unixdate>24836) unixdate=24836;
	unixdate*=86400;
	unixdate-=time_tzoffset(sid, unixdate);
	t=unixdate;
	strftime(posttime1, sizeof(posttime1), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	t=unixdate+172800;
	strftime(posttime2, sizeof(posttime2), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname FROM gw_events where status = 0 and eventstart >= #%s# and eventstart < #%s# and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish, eventname FROM gw_events where status = 0 and eventstart >= '%s' and eventstart < '%s' and assignedto = %d ORDER BY eventstart ASC", posttime1, posttime2, sid->dat->user_uid))<0) return;
	}
	prints(sid, "<TABLE BGCOLOR=#000000 BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP VALIGN=TOP><FONT COLOR=%s>Calendar</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	for (i=0;i<2;i++) {
		t=unixdate+(i*86400);
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT COLSPAN=2 NOWRAP><B><A HREF=%s/calendar/list?day=%d>", COLOR_FVAL, sid->dat->in_ScriptName, (int)(t/86400));
		if (i==0) {
			prints(sid, "Today");
		} else if (i==1) {
			prints(sid, "Tomorrow");
		}
		prints(sid, "</A></B></TD></TR>\n");
		k=1;
		for (j=0;j<sqlNumtuples(sqr);j++) {
			t2=time_sql2unix(sqlGetvalue(sqr, j, 1));
			if ((t2<t)||(t2>=t+86400)) continue;
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
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=100%%><FONT SIZE=2 COLOR=%s>&nbsp;E-Mail</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname, poppassword, lastcount FROM gw_mailaccounts where obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if ((sqr2=sqlQueryf(sid, "SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status = 'n'", sid->dat->user_uid, atoi(sqlGetvalue(sqr, i, 0))))<0) continue;
		newmessages=atoi(sqlGetvalue(sqr2, 0, 0));
		sqlFreeconnect(sqr2);
		prints(sid, "<TR BGCOLOR=%s><TD WIDTH=100%%><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", COLOR_FVAL);
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT NOWRAP>", COLOR_FVAL);
		if (sid->dat->user_menustyle>0) {
			prints(sid, "<A HREF=%s/mail/main?accountid=%d TARGET=gwmain>%-.25s</A>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), str2html(sid, sqlGetvalue(sqr, i, 1)));
		} else {
			prints(sid, "<A HREF=%s/mail/list?accountid=%d>%-.25s</A>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
		prints(sid, "&nbsp;</TD><TD ALIGN=RIGHT NOWRAP>%d New</TD></TR>\n</TABLE></TD></TR>\n", newmessages);
	}
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE>\n");
	prints(sid, "<BR>\n");
	tasklist(sid, sid->dat->user_uid);
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void htpage_header(CONNECTION *sid, char *title)
{
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n");
	prints(sid, "<HEAD>\r\n");
//	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=us-ascii\">\r\n");
	prints(sid, "<TITLE>%s</TITLE>\r\n", title);
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/groupware/css/groupware.css\">\r\n");
	prints(sid, "<STYLE TYPE=text/css>\r\n");
	prints(sid, ".JUSTIFY { text-align: justify; }\r\n");
	prints(sid, ".TBAR    { color: #505050; text-decoration: none; font-family: Geneva, Arial,Verdana; font-size: 8pt; }\r\n");
	prints(sid, "A        { color: %s; text-decoration: none; }\r\n", COLOR_LINKS);
	prints(sid, "A:HOVER  { text-decoration: underline; }\r\n");
	prints(sid, "INPUT    { color: #000000; font-family: Courier New; font-size: 12px; }\r\n");
	prints(sid, "SELECT   { color: #000000; font-family: Courier New; font-size: 12px; }\r\n");
	prints(sid, "TD       { color: #000000; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n");
	prints(sid, "TH       { background-color: %s; color: %s; font-family: Arial, Helvetica; font-size: 12px; font-style: normal; }\r\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "</STYLE>\r\n");
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<BODY BACKGROUND=\"/groupware/images/bgmain.gif\" BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\" TOPMARGIN=0 LEFTMARGIN=0 MARGINHEIGHT=0 MARGINWIDTH=0>\r\n");
}

void htpage_footer(CONNECTION *sid)
{
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

void htselect_callfilter(CONNECTION *sid, int selected, char *baseuri)
{
	char *option[]={ "Open", "Closed", "All" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int status;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (selected<1) {
		selected=sid->dat->user_uid;
	}
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints(sid, "<FORM METHOD=GET NAME=callfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.callfilter.userid.options[document.callfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sqlGetvalue(sqr, i, 0)));
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\n", atoi(sqlGetvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.callfilter.status.options[document.callfilter.status.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=status onChange=\"go2()\">');\r\n");
	for (i=0, j=2;i<3;i++, j++) {
		if (j>2) j=0;
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
		prints(sid, "&status=%d\"%s>%s');\n", j, j==status?" SELECTED":"", option[j]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sqlFreeconnect(sqr);
	return;
}

void htselect_eventfilter(CONNECTION *sid, int selected, char *baseuri)
{
	char *option[]={ "Open", "Closed", "All" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int status;

	if ((ptemp=getgetenv(sid, "STATUS"))!=NULL) {
		status=atoi(ptemp);
	} else {
		status=0;
	}
	if (selected<1) {
		selected=sid->dat->user_uid;
	}
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints(sid, "<FORM METHOD=GET NAME=eventfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.eventfilter.userid.options[document.eventfilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sqlGetvalue(sqr, i, 0)));
		if ((ptemp=getgetenv(sid, "DAY"))!=NULL) {
			prints(sid, "&day=%d", atoi(ptemp));
		} else if ((getgetenv(sid, "MONTH")!=NULL)&&(getgetenv(sid, "YEAR")!=NULL)) {
			prints(sid, "&month=%d", atoi(getgetenv(sid, "MONTH")));
			prints(sid, "&year=%d", atoi(getgetenv(sid, "YEAR")));
		}
		prints(sid, "&status=%d", status);
		prints(sid, "\"%s>%s');\n", atoi(sqlGetvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.eventfilter.status.options[document.eventfilter.status.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=status onChange=\"go2()\">');\r\n");
	for (i=0, j=2;i<3;i++, j++) {
		if (j>2) j=0;
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
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
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=status>\r\n");
	prints(sid, "<OPTION VALUE='2'%s>%s\n", status==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='0'%s>%s\n", status==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", status==1?" SELECTED":"", option[1]);
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sqlFreeconnect(sqr);
	return;
}

void htselect_notefilter(CONNECTION *sid, int selected, char *baseuri)
{
	char *option[]={ "All", "Personal", "Calls", "Contacts", "Events", "Notes", "Tasks", "Users" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int table;

	if (selected<1) {
		selected=sid->dat->user_uid;
	}
	if ((ptemp=getgetenv(sid, "TABLE"))!=NULL) {
		table=atoi(ptemp);
	} else {
		table=1;
	}
	prints(sid, "<FORM METHOD=GET NAME=notefilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.notefilter.userid.options[document.notefilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&table=%d", sid->dat->in_ScriptName, baseuri, atoi(sqlGetvalue(sqr, i, 0)), table);
		prints(sid, "\"%s>%s');\n", atoi(sqlGetvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.notefilter.table.options[document.notefilter.table.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=table onChange=\"go2()\">');\r\n");
	for (i=0;i<8;i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&table=%d", sid->dat->in_ScriptName, baseuri, selected, i);
		prints(sid, "\"%s>%s');\n", i==table?" SELECTED":"", option[i]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=table>\r\n");
	prints(sid, "<OPTION VALUE='0'%s>%s\n", table==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", table==1?" SELECTED":"", option[1]);
	prints(sid, "<OPTION VALUE='2'%s>%s\n", table==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='3'%s>%s\n", table==3?" SELECTED":"", option[3]);
	prints(sid, "<OPTION VALUE='4'%s>%s\n", table==4?" SELECTED":"", option[4]);
	prints(sid, "<OPTION VALUE='5'%s>%s\n", table==5?" SELECTED":"", option[5]);
	prints(sid, "<OPTION VALUE='6'%s>%s\n", table==6?" SELECTED":"", option[6]);
	prints(sid, "<OPTION VALUE='7'%s>%s\n", table==7?" SELECTED":"", option[7]);
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sqlFreeconnect(sqr);
	return;
}

void htselect_mailjump(CONNECTION *sid, int selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by accountname ASC", sid->dat->user_uid))<0) return;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailjump ACTION=%s/mail/list onChange=\"mailjump()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TR><TD ALIGN=LEFT>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go() {\r\n");
	if (sid->dat->user_menustyle>0) {
		prints(sid, "	top.gwmain.location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	} else {
		prints(sid, "	location=document.mailjump.accountid.options[document.mailjump.accountid.selectedIndex].value\r\n");
	}
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=accountid onChange=\"go()\">');\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/main?accountid=%d\"%s>%s');\n", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), atoi(sqlGetvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
		} else {
			prints(sid, "document.write('<OPTION VALUE=\"%s/mail/list?accountid=%d\"%s>%s');\n", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), atoi(sqlGetvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=accountid>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints(sid, "<OPTION VALUE='%d'", atoi(sqlGetvalue(sqr, i, 0)));
		if (atoi(sqlGetvalue(sqr, i, 0))==selected) prints(sid, " SELECTED");
		prints(sid, ">%s\n", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT><INPUT TYPE=SUBMIT NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></TR></FORM></TABLE>\r\n");
	sqlFreeconnect(sqr);
	return;
}

void htselect_day(CONNECTION *sid, char *selected)
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

void htselect_hour(CONNECTION *sid, int selected)
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

void htselect_month(CONNECTION *sid, char *selected)
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

void htselect_qhours(CONNECTION *sid, int selected)
{
	int i;

	for (i=0;i<24;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d hour%s\n", i, i==selected?" SELECTED":"", i, i==1?"":"s");
	}
}

void htselect_qminutes(CONNECTION *sid, int selected)
{
	int i;

	for (i=0;i<60;i+=15) {
		prints(sid, "<OPTION VALUE='%d'%s>%d minutes\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_time(CONNECTION *sid, time_t unixtime)
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

void htselect_year(CONNECTION *sid, int startyear, char *selected)
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
/*
void htselect_bookmarkfolder(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders ORDER BY foldername ASC"))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY foldername ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>Root Folder\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 2)));
	}
	sqlFreeconnect(sqr);
	return;
}
*/

typedef struct {
	int lastref;
	int printed;
} _btree;
typedef struct {
	int id;
	int depth;
	int numchildren;
} _ptree;

void htselect_bookmarkfolder(CONNECTION *sid, int selected)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j;
	int x;
	int sqr;

	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders ORDER BY parentid ASC, foldername ASC"))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY parentid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>Root Folder\n");
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		return;
	}
	btree=calloc(sqlNumtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sqlNumtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sqlNumtuples(sqr);i++) {
		if (btree[atoi(sqlGetvalue(sqr, i, 0))].printed) continue;
		if (atoi(sqlGetvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sqlGetvalue(sqr, i, 0));
			btree[atoi(sqlGetvalue(sqr, i, 0))].printed=1;
			depth++;
		}
	}
	if (depth>0) {
		depth--;
		goto widthloop;
	}
	base++;
	btree[depth].lastref=0;
	depth=0;
	if (base<sqlNumtuples(sqr)) {
		goto widthloop;
	}
//	logerror(sid, __FILE__, __LINE__, "[%d][================]", sqlNumtuples(sqr));
	for (i=0;i<sqlNumtuples(sqr);i++) {
//		logerror(sid, __FILE__, __LINE__, "[%d][%s][%s][%s]", i, sqlGetvalue(sqr, ptree[i].id, 0), sqlGetvalue(sqr, ptree[i].id, 1), sqlGetvalue(sqr, ptree[i].id, 2));
		x=atoi(sqlGetvalue(sqr, ptree[i].id, 0));
		prints(sid, "<OPTION VALUE='%d'%s>", x, x==selected?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;");
		prints(sid, "%s\n", str2html(sid, sqlGetvalue(sqr, ptree[i].id, 2)));
	}
//	logerror(sid, __FILE__, __LINE__, "[================]");
	free(ptree);
	free(btree);
	sqlFreeconnect(sqr);
	return;
}

void htselect_callaction(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT callactionid, actionname FROM gw_callactions ORDER BY actionname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_contact(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQuery(sid, "SELECT contactid, surname, givenname FROM gw_contacts order by surname, givenname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
		if (strlen(sqlGetvalue(sqr, i, 1))&&strlen(sqlGetvalue(sqr, i, 2))) prints(sid, ", ");
		prints(sid, "%s\n", str2html(sid, sqlGetvalue(sqr, i, 2)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_eventclosingstatus(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT eventclosingid, closingname FROM gw_eventclosings ORDER BY closingname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_eventstatus(CONNECTION *sid, int selected)
{
	char *option[]={ "Open", "Closed", };
	int i;

	for (i=0;i<2;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_eventtype(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT eventtypeid, eventtypename FROM gw_eventtypes ORDER BY eventtypename ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_forumgroup(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT forumgroupid, title FROM gw_forumgroups ORDER BY title ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_group(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQuery(sid, "SELECT groupid, groupname FROM gw_groups order by groupname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_layout(CONNECTION *sid, int selected)
{
	char *option[]={ "Simple (Frameless)", "Standard", "Outlook" };
	int i;

	for (i=0;i<3;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_mailaccount(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE obj_uid = %d order by mailaccountid ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_number(CONNECTION *sid, int selected, int start, int end)
{
	int i;

	for (i=start;i<end+1;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_priority(CONNECTION *sid, int selected)
{
	char *option[]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int i;

	for (i=0;i<5;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_product(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQuery(sid, "SELECT productid, productname FROM gw_products order by productname ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_reminder(CONNECTION *sid, int selected)
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

void htselect_user(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQuery(sid, "SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	prints(sid, "<OPTION VALUE=0>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

void htselect_zone(CONNECTION *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sqlQuery(sid, "SELECT zoneid, zonename FROM gw_zones order by zonename ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sqlGetvalue(sqr, i, 1)));
	}
	sqlFreeconnect(sqr);
	return;
}

char *htview_callaction(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT actionname FROM gw_callactions WHERE callactionid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 0)));
	}
	sqlFreeconnect(sqr);
	return buffer;
}

char *htview_eventclosingstatus(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT closingname FROM gw_eventclosings WHERE eventclosingid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 0)));
	}
	sqlFreeconnect(sqr);
	return buffer;
}

char *htview_eventstatus(CONNECTION *sid, int selected)
{
	char *option[]={ "Open", "Closed" };

	if ((selected<0)||(selected>1)) return "";
	return option[selected];
}

char *htview_eventtype(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT eventtypename FROM gw_eventtypes WHERE eventtypeid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 0)));
	}
	sqlFreeconnect(sqr);
	return buffer;
}

char *htview_forumgroup(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT title FROM gw_forumgroups WHERE forumgroupid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 0)));
	}
	sqlFreeconnect(sqr);
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

	while (holidays[i][0]!=NULL) {
		if (strncmp(date+4, holidays[i][0], 6)==0) return holidays[i][1];
		i++;
	}
	return "";
}

char *htview_product(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT productid, productname FROM gw_products WHERE productid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 1)));
	}
	sqlFreeconnect(sqr);
	return buffer;
}

char *htview_reminder(CONNECTION *sid, int selected)
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

char *htview_user(CONNECTION *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sqlQueryf(sid, "SELECT userid, username FROM gw_users WHERE userid = %d", selected))<0) return buffer;
	if (sqlNumtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sqlGetvalue(sqr, 0, 1)));
	}
	sqlFreeconnect(sqr);
	return buffer;
}
