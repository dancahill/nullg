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

void printlogin(int sid)
{
	char file[100];
	FILE *fp;
	int ich;
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\r\n<HEAD><TITLE>Sentinel Groupware Login</TITLE>\r\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\r\n");
	prints("if (self!=parent) {\r\n");
	prints("	open('%s/','_top');\r\n", conn[sid].dat->in_ScriptName);
	prints("};\r\n");
	prints("// -->\r\n</SCRIPT>\r\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints("var browser_type=navigator.appName;\r\n");
	prints("var browser_version=parseInt(navigator.appVersion);\r\n");
	prints("function onMouseDownHandler(e) {\r\n");
	prints("	if (e.which==3) {\r\n");
	prints("		return false;\r\n");
	prints("	}\r\n");
	prints("}\r\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\r\n");
	prints("	if (document.layers) {\r\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\r\n");
	prints("		window.onmousedown=onMouseDownHandler;\r\n");
	prints("	}\r\n");
	prints("}\r\n");
	prints("// -->\r\n</SCRIPT>\r\n");
	prints("</HEAD>\r\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgmain.gif BGCOLOR=#F0F0F0 COLOR=#000000 LINK=#0000FF ALINK=#0000FF VLINK=#0000FF onContextMenu='return false'>\r\n");
	prints("<CENTER>\r\n<BR>\r\n");
	snprintf(file, sizeof(file)-1, "%s/issue.txt", config.server_etc_dir);
	FixSlashes(file);
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while ((ich=getc(fp))!=EOF) {
			prints("%c", ich);
		}
		fclose(fp);
	}
	prints("<FORM METHOD=POST ACTION=%s/ AUTOCOMPLETE=OFF NAME=login>\r\n", conn[sid].dat->in_ScriptName);
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		prints("<FONT COLOR=RED>Incorrect username or password</FONT>\n");
	}
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Sentinel Groupware Login</TH></TR>\r\n", COLOR_TRIM);
	if ((sqr=sqlQuery("SELECT password FROM gw_users where username = 'administrator'"))<0) return;
	if (sqlNumtuples(sqr)==1) {
		if (strcmp("$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/", sqlGetvalue(sqr, 0, 0))==0) {
			prints("<TR BGCOLOR=%s><TD><B>Username</B></TD><TD><INPUT TYPE=TEXT NAME=USERNAME SIZE=25 MAXLENGTH=50 VALUE='administrator'></TD></TR>\r\n", COLOR_EDITFORM);
			prints("<TR BGCOLOR=%s><TD><B>Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=PASSWORD SIZE=25 MAXLENGTH=50 VALUE='visual'></TD></TR>\r\n", COLOR_EDITFORM);
			prints("<TR BGCOLOR=%s><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\r\n", COLOR_EDITFORM);
			prints("</TABLE></FORM>\r\n");
			prints("<TABLE><TR><TD><FONT COLOR=RED><PRE>\r\n");
			prints("You have not yet changed the administrator's password.\n");
			prints("Please do this as soon as possible for security reasons.\r\n");
			prints("</PRE></FONT></TD></TR></TABLE>\r\n");
			prints("</CENTER>\r\n</BODY>\r\n</HTML>\r\n");
			prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.login.USERNAME.focus();\n// -->\n</SCRIPT>\n");
			sqlFreeconnect(sqr);
			return;
		}
	}
	sqlFreeconnect(sqr);
	prints("<TR BGCOLOR=%s><TD><B>Username</B></TD><TD><INPUT TYPE=TEXT NAME=USERNAME SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\n", COLOR_EDITFORM, conn[sid].dat->in_username);
	prints("<TR BGCOLOR=%s><TD><B>Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=PASSWORD SIZE=25 MAXLENGTH=50></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\n", COLOR_EDITFORM);
	prints("</TABLE></FORM>\r\n</CENTER>\r\n");
	if (strlen(conn[sid].dat->in_username)<1) {
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.login.USERNAME.focus();\n// -->\n</SCRIPT>\n");
	} else {
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.login.PASSWORD.focus();\n// -->\n</SCRIPT>\n");
	}
	prints("</BODY>\r\n</HTML>\r\n");
	return;
}

void printlogout(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD><TITLE>Sentinel Groupware Logout</TITLE></HEAD>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("var browser_type=navigator.appName;\n");
	prints("var browser_version=parseInt(navigator.appVersion);\n");
	prints("function onMouseDownHandler(e) {\n");
	prints("	if (e.which==3) {\n");
	prints("		return false;\n");
	prints("	}\n");
	prints("}\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints("	if (document.layers) {\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\n");
	prints("		window.onmousedown=onMouseDownHandler;\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgmain.gif BGCOLOR=#F0F0F0 COLOR=#000000 LINK=#0000FF ALINK=#0000FF VLINK=#0000FF onContextMenu='return false'>\n");
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL='%s/'\">\n", conn[sid].dat->in_ScriptName);
	prints("<CENTER>\n<BR><BR>\n");
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH>Sentinel Groupware Logout</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD>\n", COLOR_FTEXT);
	prints("For increased security, please allow this window to be closed.<BR><BR>\n");
	prints("By closing this window, you will remove any temporary information stored<BR>\n");
	prints("by your web browser which could otherwise allow unauthorized access.<BR>\n");
	prints("</TD></TR>\n");
	prints("</TABLE>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\nwindow.close('_top');\n// -->\n</SCRIPT>\n");
	prints("</BODY>\n</HTML>\n");
	logaccess(0, "%s - Logout: username=%s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
}

void printmenu(int sid)
{
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD>\n<TITLE>Sentinel Left Side Menu</TITLE>\n");
	prints("<STYLE TYPE=text/css>\n");
	prints("DIV.justify {font-family : Arial, Verdana; text-align : justify}\n");
	prints("A {font-family : Verdana, Arial; color : #FFFFFF; text-decoration : none}\n");
	prints("</STYLE>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("var browser_type=navigator.appName;\n");
	prints("var browser_version=parseInt(navigator.appVersion);\n");
	prints("function onMouseDownHandler(e) {\n");
	prints("	if (e.which==3) {\n");
	prints("		return false;\n");
	prints("	}\n");
	prints("}\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints("	if (document.layers) {\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\n");
	prints("		window.onmousedown=onMouseDownHandler;\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n");
	prints("<!--\n");
	prints("if (document.images) {\n");
	prints("	image1on = new Image();\n");
	prints("	image1on.src = '/sentinel/images/menu/admin_1.gif';\n");
	prints("	image1off = new Image();\n");
	prints("	image1off.src = '/sentinel/images/menu/admin_0.gif';\n");
	prints("	image2on = new Image();\n");
	prints("	image2on.src = '/sentinel/images/menu/calendar_1.gif';\n");
	prints("	image2off = new Image();\n");
	prints("	image2off.src = '/sentinel/images/menu/calendar_0.gif';\n");
	prints("	image3on = new Image();\n");
	prints("	image3on.src = '/sentinel/images/menu/contact_1.gif';\n");
	prints("	image3off = new Image();\n");
	prints("	image3off.src = '/sentinel/images/menu/contact_0.gif';\n");
	prints("	image4on = new Image();\n");
	prints("	image4on.src = '/sentinel/images/menu/files_1.gif';\n");
	prints("	image4off = new Image();\n");
	prints("	image4off.src = '/sentinel/images/menu/files_0.gif';\n");
	prints("	image5on = new Image();\n");
	prints("	image5on.src = '/sentinel/images/menu/forums_1.gif';\n");
	prints("	image5off = new Image();\n");
	prints("	image5off.src = '/sentinel/images/menu/forums_0.gif';\n");
	prints("	image6on = new Image();\n");
	prints("	image6on.src = '/sentinel/images/menu/links_1.gif';\n");
	prints("	image6off = new Image();\n");
	prints("	image6off.src = '/sentinel/images/menu/links_0.gif';\n");
	prints("	image7on = new Image();\n");
	prints("	image7on.src = '/sentinel/images/menu/message_1.gif';\n");
	prints("	image7off = new Image();\n");
	prints("	image7off.src = '/sentinel/images/menu/message_0.gif';\n");
	prints("	image8on = new Image();\n");
	prints("	image8on.src = '/sentinel/images/menu/order_1.gif';\n");
	prints("	image8off = new Image();\n");
	prints("	image8off.src = '/sentinel/images/menu/order_0.gif';\n");
	prints("	image9on = new Image();\n");
	prints("	image9on.src = '/sentinel/images/menu/query_1.gif';\n");
	prints("	image9off = new Image();\n");
	prints("	image9off.src = '/sentinel/images/menu/query_0.gif';\n");
	prints("	imageAon = new Image();\n");
	prints("	imageAon.src = '/sentinel/images/menu/webmail_1.gif';\n");
	prints("	imageAoff = new Image();\n");
	prints("	imageAoff.src = '/sentinel/images/menu/webmail_0.gif';\n");
	prints("}\n");
	prints("\n");
	prints("function changeImages() {\n");
	prints("	if (document.images) {\n");
	prints("		for (var i=0; i<changeImages.arguments.length; i+=2) {\n");
	prints("			document[changeImages.arguments[i]].src = eval(changeImages.arguments[i+1] + \".src\");\n");
	prints("		}\n");
	prints("	}\n");
	prints("}\n");
	prints("function ListMessages() {\n");
	prints("	window.open('%s/messagelist','msgwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName);
	prints("}\n");
	prints("function reminders() {\n");
	prints("	window.parent.frames[\"bottom\"].location.replace(\"%s/mainbottom\");\n", conn[sid].dat->in_ScriptName);
	prints("	setTimeout(\"reminders()\", 300000);\n");
	prints("}\n");
	prints("reminders();\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("</HEAD>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgborder.gif BGCOLOR=#000050 TEXT=#000000 LINK=#FFFFFF ALINK=#FFFFFF VLINK=#FFFFFF onContextMenu='return false'>\n");
	prints("<CENTER><BR>\n");
	if (verifyimage("sentinel.gif")) {
		prints("<A HREF=%s/mainmain target='main' onclick=\"window.open('%s/menu','left')\"><IMG BORDER=0 SRC=/sentinel/images/sentinel.gif HEIGHT=50 WIDTH=125 ALT='%s logged in'></A><BR><BR>\n", conn[sid].dat->in_ScriptName, conn[sid].dat->in_ScriptName, conn[sid].dat->in_username);
	}
	if ((sqr=sqlQueryf("SELECT authadmin, authcalendar, authcontacts, authfiles, authforums, authlinks, authmessages, authorders, authquery, authwebmail FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return;
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>\n");
	if (atoi(sqlGetvalue(sqr, 0, 0))>0) {
		if (verifyimage("menu/admin_0.gif")) {
			prints("<A HREF=%s/admin        TARGET=main ONMOUSEOVER=\"changeImages('image1', 'image1on')\" ONMOUSEOUT=\"changeImages('image1', 'image1off')\"><IMG NAME=image1 SRC=/sentinel/images/menu/admin_0.gif    ALT=\"Administration\" BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/admin        TARGET=main>Administration</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 1))>0) {
		if (verifyimage("menu/calendar_0.gif")) {
			prints("<A HREF=%s/calendarlist TARGET=main ONMOUSEOVER=\"changeImages('image2', 'image2on')\" ONMOUSEOUT=\"changeImages('image2', 'image2off')\"><IMG NAME=image2 SRC=/sentinel/images/menu/calendar_0.gif ALT=\"Calendar\"       BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/calendarlist TARGET=main>Calendar</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 2))>0) {
		if (verifyimage("menu/contact_0.gif")) {
			prints("<A HREF=%s/contactlist  TARGET=main ONMOUSEOVER=\"changeImages('image3', 'image3on')\" ONMOUSEOUT=\"changeImages('image3', 'image3off')\"><IMG NAME=image3 SRC=/sentinel/images/menu/contact_0.gif  ALT=\"Contacts\"       BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/contactlist  TARGET=main>Contacts</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 3))>0) {
		if (verifyimage("menu/files_0.gif")) {
			prints("<A HREF=%s/files/       TARGET=main ONMOUSEOVER=\"changeImages('image4', 'image4on')\" ONMOUSEOUT=\"changeImages('image4', 'image4off')\"><IMG NAME=image4  SRC=/sentinel/images/menu/files_0.gif   ALT=\"Files\"          BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/files/       TARGET=main>Files</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 4))>0) {
		if (verifyimage("menu/forums_0.gif")) {
			prints("<A HREF=%s/forumlist    TARGET=main ONMOUSEOVER=\"changeImages('image5', 'image5on')\" ONMOUSEOUT=\"changeImages('image5', 'image5off')\"><IMG NAME=image5  SRC=/sentinel/images/menu/forums_0.gif  ALT=\"Forums\"         BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/forumlist    TARGET=main>Forums</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 5))>0) {
		if (verifyimage("menu/links_0.gif")) {
			prints("<A HREF=%s/linkslist    TARGET=main ONMOUSEOVER=\"changeImages('image6', 'image6on')\" ONMOUSEOUT=\"changeImages('image6', 'image6off')\"><IMG NAME=image6  SRC=/sentinel/images/menu/links_0.gif   ALT=\"Links\"          BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/linkslist    TARGET=main>Links</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 6))>0) {
		if (verifyimage("menu/message_0.gif")) {
			prints("<A HREF=javascript:ListMessages()   ONMOUSEOVER=\"changeImages('image7', 'image7on')\" ONMOUSEOUT=\"changeImages('image7', 'image7off')\"><IMG NAME=image7  SRC=/sentinel/images/menu/message_0.gif ALT=\"Messages\"       BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=javascript:ListMessages()>Messages</A><BR>\n");
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 7))>0) {
		if (verifyimage("menu/order_0.gif")) {
			prints("<A HREF=%s/orderlist    TARGET=main ONMOUSEOVER=\"changeImages('image8', 'image8on')\" ONMOUSEOUT=\"changeImages('image8', 'image8off')\"><IMG NAME=image8  SRC=/sentinel/images/menu/order_0.gif   ALT=\"Order System\"   BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/orderlist    TARGET=main>Orders</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 8))>0) {
		if (verifyimage("menu/query_0.gif")) {
			prints("<A HREF=%s/queryrun     TARGET=main ONMOUSEOVER=\"changeImages('image9', 'image9on')\" ONMOUSEOUT=\"changeImages('image9', 'image9off')\"><IMG NAME=image9  SRC=/sentinel/images/menu/query_0.gif   ALT=\"SQL Queries\"    BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/queryrun     TARGET=main>Queries</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	if (atoi(sqlGetvalue(sqr, 0, 9))>0) {
		if (verifyimage("menu/webmail_0.gif")) {
			prints("<A HREF=%s/maillist     TARGET=main ONMOUSEOVER=\"changeImages('imageA', 'imageAon')\" ONMOUSEOUT=\"changeImages('imageA', 'imageAoff')\"><IMG NAME=imageA SRC=/sentinel/images/menu/webmail_0.gif  ALT=\"E-Mail\"         BORDER=0 WIDTH=100 HEIGHT=24></A><BR>\n", conn[sid].dat->in_ScriptName);
		} else {
			prints("<A HREF=%s/maillist     TARGET=main>Web E-Mail</A><BR>\n", conn[sid].dat->in_ScriptName);
		}
	}
	prints("</TD></TR></TABLE>\n");
	prints("</CENTER>\n</BODY>\n</HTML>\n");
	sqlFreeconnect(sqr);
	return;
}

void printmain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD>\n<TITLE>Sentinel Groupware</TITLE>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("if (self!=parent) {\n");
	prints("	open('%s/','_top');\n", conn[sid].dat->in_ScriptName);
	prints("};\n");
	prints("// -->\n</SCRIPT>\n");
	prints("</HEAD>\n");
	prints("<FRAMESET COLS=140,*,5 FRAMEBORDER=0 FRAMESPACING=0 BORDER=0>\n");
	prints("<FRAME BORDER=0 SRC=%s/menu       NAME=left   MARGINWIDTH=1 MARGINHEIGHT=1 NORESIZE SCROLLING=NO>\n", conn[sid].dat->in_ScriptName);
	prints("<FRAMESET ROWS=5,*,5 FRAMEBORDER=0 FRAMESPACING=0 BORDER=0>\n");
	prints("<FRAME BORDER=0 SRC=%s/maintop    NAME=top    MARGINWIDTH=1 MARGINHEIGHT=1 NORESIZE SCROLLING=NO>\n", conn[sid].dat->in_ScriptName);
	prints("<FRAME BORDER=0 SRC=%s/mainmain   NAME=main   MARGINWIDTH=1 MARGINHEIGHT=1 NORESIZE SCROLLING=AUTO>\n", conn[sid].dat->in_ScriptName);
	prints("<FRAME BORDER=0 SRC=%s/mainbottom NAME=bottom MARGINWIDTH=1 MARGINHEIGHT=1 NORESIZE SCROLLING=NO>\n", conn[sid].dat->in_ScriptName);
	prints("</FRAMESET>\n");
	prints("<FRAME BORDER=0 SRC=%s/mainright  NAME=right  MARGINWIDTH=1 MARGINHEIGHT=1 NORESIZE SCROLLING=NO>\n", conn[sid].dat->in_ScriptName);
	prints("</FRAMESET>\n");
	prints("To view this page, you need a web browser capable of displaying frames.\n");
	prints("</HTML>\n");
}

void printmainright(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD>\n<TITLE>Sentinel Right Frame</TITLE>\n</HEAD>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("var browser_type=navigator.appName;\n");
	prints("var browser_version=parseInt(navigator.appVersion);\n");
	prints("function onMouseDownHandler(e) {\n");
	prints("	if (e.which==3) {\n");
	prints("		return false;\n");
	prints("	}\n");
	prints("}\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints("	if (document.layers) {\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\n");
	prints("		window.onmousedown=onMouseDownHandler;\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgborder.gif BGCOLOR=#000050 onContextMenu='return false'>\n");
	prints("</BODY>\n</HTML>\n");
}

void printmaintop(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD>\n<TITLE>Sentinel Top Frame</TITLE>\n</HEAD>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("var browser_type=navigator.appName;\n");
	prints("var browser_version=parseInt(navigator.appVersion);\n");
	prints("function onMouseDownHandler(e) {\n");
	prints("	if (e.which==3) {\n");
	prints("		return false;\n");
	prints("	}\n");
	prints("}\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints("	if (document.layers) {\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\n");
	prints("		window.onmousedown=onMouseDownHandler;\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgborder.gif BGCOLOR=#000050 onContextMenu='return false'>\n");
	prints("</BODY>\n</HTML>\n");
}

void printmainbottom(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML>\n<HEAD>\n<TITLE>Sentinel Bottom Frame</TITLE>\n");
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"60; URL=%s/mainbottom\">\n</HEAD>\n", conn[sid].dat->in_ScriptName);
	messageautocheck(sid);
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("var browser_type=navigator.appName;\n");
	prints("var browser_version=parseInt(navigator.appVersion);\n");
	prints("function onMouseDownHandler(e) {\n");
	prints("	if (e.which==3) {\n");
	prints("		return false;\n");
	prints("	}\n");
	prints("}\n");
	prints("if (browser_type == \"Netscape\" && browser_version>=4) {\n");
	prints("	if (document.layers) {\n");
	prints("		window.captureEvents(Event.MOUSEDOWN);\n");
	prints("		window.onmousedown=onMouseDownHandler;\n");
	prints("	}\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgborder.gif BGCOLOR=#000050 onContextMenu='return false'>\n");
//	prints("</BODY>\n</HTML>\n");
}

void printmotd(int sid)
{
	struct timeval ttime;
	struct timezone tzone;
	char timebuffer[50];
	char file[100];
	char line[512];
	time_t t;
	FILE *fp;

	gettimeofday(&ttime, &tzone);
	t=(int)((ttime.tv_sec-tzone.tz_minuteswest*60)/86400)*86400;
	strftime(timebuffer, sizeof(timebuffer), "%A, %B %d, %Y", gmtime(&t));
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Welcome");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/index.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/mainmain>MAIN</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
	prints("</CENTER><BR>\n");
	prints("<CENTER>\n<TABLE WIDTH=80%% BORDER=0 CELLPADDING=2 CELLSPACING=1>\n");
	prints("<TR BGCOLOR=%s><TD>\n", COLOR_TRIM);
	prints("<TABLE WIDTH=100%% BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\n");
	prints("<TD WIDTH=100%% NOWRAP ALIGN=left><B>Welcome, %s.</B>&nbsp;</TD>\n", conn[sid].dat->in_username);
	prints("<TD WIDTH=100%% NOWRAP ALIGN=right>&nbsp;<B>%s</B></TD>\n", timebuffer);
	prints("</TR></TABLE>\n");
	prints("</TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD COLSPAN=2>\n<BR>\n", COLOR_FTEXT);
#ifdef WIN32
	snprintf(file, sizeof(file)-1, "%s\\motd.txt", config.server_etc_dir);
#else
	snprintf(file, sizeof(file)-1, "%s/motd.txt", config.server_etc_dir);
#endif
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			prints("%s", line);
		}
		fclose(fp);
	}
	prints("&nbsp;\n</TD></TR></TABLE>\n");
	prints("</CENTER>\n");
	prints("</BODY>\n</HTML>\n");
}

void printheader(int sid, char *title)
{
	prints("<HTML>\n<HEAD><TITLE>%s</TITLE>\n", title);
	prints("<META HTTP-EQUIV=\"Expires\" CONTENT=\"0\">\n");
	prints("<STYLE TYPE=text/css>\n");
	prints(".JUSTIFY { text-align: justify }\n");
	prints(".TBAR    { color: #505050; text-decoration: none; font-family: Arial,Verdana; font-size: 8pt; }\n");
	prints("A        { color: #0000FF; text-decoration: none }\n");
	prints("TD       { color: #000000; text-decoration: none }\n");
	prints("TH       { color: #000000; text-decoration: none }\n");
	prints("</STYLE>\n");
	prints("</HEAD>\n");
	prints("<BODY BACKGROUND=/sentinel/images/bgmain.gif BGCOLOR=#F0F0F0 COLOR=#000000 LINK=#0000FF ALINK=#0000FF VLINK=#0000FF LEFTMARGIN=0 TOPMARGIN=0 MARGINHEIGHT=0 MARGINWIDTH=0>\n");
}

void printfooter(int sid)
{
	prints("</BODY>\n</HTML>\n");
//	reply.bodydone=1;
//	flushbuffer();
}

void printerror(int sid, int status, char* title, char* text)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n", status, title);
	prints("<BODY BGCOLOR=#F0F0F0 TEXT=#000000 LINK=#0000FF ALINK=#0000FF VLINK=#0000FF onContextMenu='return false'>\n");
	prints("<H1>%d %s</H1>\n", status, title);
	prints("%s\n", text);
	prints("<HR>\n<ADDRESS>%s</ADDRESS>\n</BODY></HTML>\n", SERVER_NAME);
	conn[sid].dat->out_bodydone=1;
	flushbuffer(sid);
	closeconnect(sid, 1);
	return;
}

void yearselect(int sid, int startyear, char *selected)
{
	char year[5];
	int i;

	if (startyear<1900) startyear=1900;
	memset(year, 0, sizeof(year));
	strncpy(year, selected, 4);
	for (i=startyear;i<2038;i++) {
		prints("<OPTION VALUE='%d'", i);
		if (i==atoi(year)) prints(" SELECTED");
		prints(">%d\n", i);
	}
	return;
}

void monthselect(int sid, char *selected)
{
	char *monthoption[]={
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
		prints("<OPTION VALUE='%02d'", i);
		if (i==atoi(month)) prints(" SELECTED");
		prints(">%s\n", monthoption[i-1]);
	}
	return;
}

void dayselect(int sid, char *selected)
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
		prints("<OPTION VALUE='%02d'", i);
		if (i==atoi(day)) prints(" SELECTED");
		prints(">%02d\n", i);
	}
	return;
}

void numberselect(int sid, int selected, int start, int end)
{
	int i;

	for (i=start;i<end+1;i++) {
		prints("<OPTION VALUE='%d'", i);
		if (i==selected) prints(" SELECTED");
		prints(">%d\n", i);
	}
}

void priorityselect(int sid, char *selected)
{
	char *option[]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int i;

	for (i=0;i<5;i++) {
		prints("<OPTION VALUE='%d'", i);
		if (i==atoi(selected)) prints(" SELECTED");
		prints(">%s\n", option[i]);
	}
	return;
}

void timeselect(int sid, char *selected)
{
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

	selected=sqltime2text(selected);
	for (i=0;i<24;i++) {
		for (j=0;j<4;j++) {
			prints("<OPTION VALUE='%02d:%02d:00'", i, j*15);
			if (strcmp(selected, timeoption[i*4+j])==0) prints(" SELECTED");
			prints(">%s\n", timeoption[i*4+j]);
		}
	}
	return;
}

void assigntoselect(int sid, char *selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQuery("SELECT username FROM gw_users order by username ASC"))<0) return;
	prints("<OPTION>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<OPTION");
		if (strcmp(sqlGetvalue(sqr, i, 0), selected)==0) prints(" SELECTED");
		prints(">%s\n", sqlGetvalue(sqr, i, 0));
	}
	sqlFreeconnect(sqr);
	return;
}

void contactselect(int sid, int selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQuery("SELECT contactid, lastname, firstname FROM gw_contacts order by lastname ASC"))<0) return;
	prints("<OPTION VALUE=0>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<OPTION VALUE=%s", sqlGetvalue(sqr, i, 0));
		if (atoi(sqlGetvalue(sqr, i, 0))==selected) prints(" SELECTED");
		prints(">%s", sqlGetvalue(sqr, i, 1));
		if (strlen(sqlGetvalue(sqr, i, 1))&&strlen(sqlGetvalue(sqr, i, 2))) prints(", ");
		prints("%s\n", sqlGetvalue(sqr, i, 2));
	}
	sqlFreeconnect(sqr);
	return;
}

void companyselect(int sid, int selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQuery("SELECT companyid, companyname FROM gw_companies order by companyname ASC"))<0) return;
	prints("<OPTION VALUE='0'>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<OPTION VALUE=%s", sqlGetvalue(sqr, i, 0));
		if (atoi(sqlGetvalue(sqr, i, 0))==selected) prints(" SELECTED");
		prints(">%s\n", sqlGetvalue(sqr, i, 1));
	}
	sqlFreeconnect(sqr);
	return;
}

void reminderselect(int sid, int selected)
{
	int i;

	prints("<OPTION VALUE='0'>No reminder\n");
	for (i=1;i<60;i++) {
		switch(i) {
			case 15:
			case 30:
				prints("<OPTION VALUE='%d'", i);
				if (i==selected) prints(" SELECTED");
				prints(">%s\n", reminder(i));
				continue;
			default:
				continue;
		}
	}
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
				prints("<OPTION VALUE='%d'", i*60);
				if (i*60==selected) prints(" SELECTED");
				prints(">%s\n", reminder(i*60));
				continue;
			default:
				continue;
		}
	}
	return;
}

void repeatselect(int sid, char *selected)
{
	char *option[]={ "Does not repeat", "Daily", "Weekly", "Monthly", "Yearly" };
	int i;

	for (i=0;i<5;i++) {
		prints("<OPTION VALUE='%d'", i);
		if (i==atoi(selected)) prints(" SELECTED");
		prints(">%s\n", option[i]);
	}
	return;
}

void productselect(int sid, char *selected)
{
	int i;
	int sqr;

	if ((sqr=sqlQuery("SELECT productname FROM gw_products order by productname ASC"))<0) return;
	prints("<OPTION VALUE=''>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<OPTION");
		if (strcmp(sqlGetvalue(sqr, i, 0), selected)==0) prints(" SELECTED");
		prints(">%s\n", sqlGetvalue(sqr, i, 0));
	}
	sqlFreeconnect(sqr);
	return;
}
