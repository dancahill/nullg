/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "httpd_main.h"

void htpage_header(CONN *sid, char *title)
{
	obj_t *tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");

	nes_setstr(sid->N, tobj, "theme", sid->dat->theme, strlen(sid->dat->theme));
	nes_setstr(sid->N, tobj, "title", title, strlen(title));
	if (htnes_dotemplate(sid, "html", "header.ns")!=0) {
		/* if the template is broken, then so is the server.
		 * Still, working headers means we can bitch about
		 * it more effectively.
		 */
		prints(sid,
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n"
			"<HTML>\r\n<HEAD>\r\n"
			"<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=iso-8859-1\">\r\n"
			"<TITLE>%s</TITLE>\r\n"
			"</HEAD>\r\n<BODY CLASS=\"MAINBACK\">\r\n"
			, title
		);
	}
	return;
}

void htpage_footer(CONN *sid)
{
	if (htnes_dotemplate(sid, "html", "footer.ns")!=0) {
		prints(sid, "</BODY>\r\n</HTML>\r\n");
	}
	return;
}

int htpage_dirlist(CONN *sid)
{
	obj_t *confobj=nes_getobj(sid->N, &sid->N->g, "_CONFIG");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *cobj1=NULL, *cobj2=NULL;
	char  *RequestURI;
	char   dir[512];
	char   file[512];
	char   index[512];
	struct stat sb;

	if (htobj->val->type!=NT_TABLE) return -1;
	RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	if (strncmp(RequestURI, "/", 1)!=0) return -1;
	if (strstr(RequestURI, "..")!=NULL) return -1;
	snprintf(dir, sizeof(dir)-1, "%s", RequestURI+1);
	while (strlen(dir)>0&&(dir[strlen(dir)-1]=='\\'||dir[strlen(dir)-1]=='/')) dir[strlen(dir)-1]='\0';

	cobj1=nes_getobj(sid->N, confobj, "private_htdocs_path");
	if (!nes_isstr(cobj1)) goto tryshared;
	snprintf(file, sizeof(file)-1, "%s/%s", nes_tostr(sid->N, cobj1), dir);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	decodeurl(file);
	fixslashes(file);
	if (stat(file, &sb)==0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(sid, file);
		/* it exists, and it's a dir */

		snprintf(index, sizeof(index)-1, "%s/%s/index.html", nes_tostr(sid->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return filesend(sid, index);

		snprintf(index, sizeof(index)-1, "%s/%s/index.nsp", nes_tostr(sid->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.nsp");

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nes_tostr(sid->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.ns");
	} else {
		cobj1=NULL;
	}
tryshared:
	cobj2=nes_getobj(sid->N, confobj, "shared_htdocs_path");
	if (!nes_isstr(cobj2)) return -1;
	snprintf(file, sizeof(file)-1, "%s/%s", nes_tostr(sid->N, cobj2), dir);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	decodeurl(file);
	fixslashes(file);
	if (stat(file, &sb)==0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(sid, file);
		/* it exists, and it's a dir */

		snprintf(index, sizeof(index)-1, "%s/%s/index.html", nes_tostr(sid->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return filesend(sid, index);

		snprintf(index, sizeof(index)-1, "%s/%s/index.nsp", nes_tostr(sid->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.nsp");

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nes_tostr(sid->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.ns");
	} else {
		cobj2=NULL;
	}
	if (cobj1==NULL&&cobj2==NULL) return -1;
	/* or just list the dir */
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
//	htpage_header(sid, RequestURI);
	htpage_topmenu(sid, "dir");
	if (htnes_dotemplate(sid, "html", "dir.ns")!=0) {
		prints(sid, "<CENTER>Access Denied - Directory listing not allowed.</CENTER>\r\n");
	}
	htpage_footer(sid);
	return 0;
}

void htpage_login(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");
	obj_t *qobj=NULL;
	char *RequestMethod=nes_getstr(sid->N, htobj, "REQUEST_METHOD");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	char pageuri[255];
	char domain[255];
	char *ptemp;
	int numdomains;

	memset(pageuri, 0, sizeof(pageuri));
	memset(domain, 0, sizeof(domain));
	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", RequestURI);
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, "NullLogic GroupServer Login");
	if (sql_query(proc->N, &qobj, "SELECT COUNT(*) FROM nullgs_entries WHERE class = 'organization'")<0) return;
	numdomains=atoi(sql_getvalue(proc->N, &qobj, 0, 0));
	sql_freeresult(proc->N, &qobj);
	tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");
	nes_setnum(sid->N, tobj, "domains",    numdomains);
	nes_setstr(sid->N, tobj, "domainname", sid->dat->domainname, -1);
	nes_setstr(sid->N, tobj, "pageuri",    pageuri, -1);
	nes_setstr(sid->N, tobj, "software",   SERVER_NAME, -1);
	nes_setstr(sid->N, tobj, "username",   sid->dat->username, -1);
//	nes_exec(sid->N, "printvar(_GLOBALS);");
	if (htnes_dotemplate(sid, "html", "login.ns")==0) {
		htpage_footer(sid);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\nif (self!=parent) open('/','_top');\r\n// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR />\r\n<CENTER>\r\n");
	if (strcmp(RequestMethod, "POST")==0) {
		prints(sid, "<FONT COLOR=RED><B>Incorrect username or password</B></FONT>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=\"/\" AUTOCOMPLETE=OFF NAME=login>\r\n");
	prints(sid, "<INPUT TYPE=hidden NAME=pageuri VALUE=\"%s\">\r\n", pageuri);
	if (numdomains<2) {
		prints(sid, "<INPUT TYPE=hidden NAME=domain VALUE=\"NULL\">\r\n");
	}
	prints(sid, "<TR><TH COLSPAN=2 STYLE='padding:1px'>%s Login</TH></TR>\r\n", SERVER_NAME);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=username SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", sid->dat->username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=password SIZE=25 MAXLENGTH=50 VALUE=''></TD></TR>\r\n");
	if (numdomains>1) {
		if (strlen(sid->dat->domainname)==0) {
			snprintf(domain, sizeof(domain)-1, "%s", nes_getstr(proc->N, htobj, "HTTP_HOST"));
		} else {
			snprintf(domain, sizeof(domain)-1, "%s", sid->dat->domainname);
		}
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Domain&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=domain SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", domain);
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Login'></TD></TR>\r\n");
	prints(sid, "</FORM>\r\n</TABLE>\r\n</CENTER>\r\n");
	htpage_footer(sid);
	return;
}

void htpage_logout(CONN *sid)
{
	db_log_activity(sid, "login", 0, "logout", "%s - Logout: username=%s", sid->socket.RemoteAddr, sid->dat->username);
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, "NullLogic GroupServer Logout");
	if (htnes_dotemplate(sid, "html", "logout.ns")!=0) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('/');\r\n// -->\r\n</SCRIPT>\r\n");
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='/'\"></NOSCRIPT>\r\n");
	}
	htpage_footer(sid);
	return;
}

void htpage_frameset(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, "NullLogic GroupServer Logout");
	if (htnes_dotemplate(sid, "html", "frames.ns")!=0) {
		prints(sid,
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Frameset//EN\">\r\n"
			"<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic GroupServer</TITLE>\r\n"
			"<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n"
			"if (self!=parent) open('/','_top');\r\n"
			"// -->\r\n</SCRIPT>\r\n"
			"</HEAD>\r\n"
			"<FRAMESET COLS=\"125,*\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n"
			"<FRAME BORDER=0 NAME=\"gwmenu\" SRC=/frames/menu MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n");		prints(sid, "<FRAME BORDER=0 NAME=\"gwmain\" SRC=/frames/motd MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=YES>\r\n"
			"</FRAMESET>\r\n"
			"To view this page, you need a web browser capable of displaying frames.\r\n"
			"</HTML>\r\n"
		);
	}
	htpage_motd(sid);
}

void htpage_topmenu(CONN *sid, char *modname)
{
	htpage_header(sid, "NullLogic GroupServer");
	nes_setstr(sid->N, nes_settable(sid->N, &sid->N->g, "_TEMP"), "modname", modname, -1);
	if (htnes_dotemplate(sid, "html", "headmenu.ns")!=0) {
		prints(sid, "error loading head menu");
	}
	return;
}

void htpage_motd(CONN *sid)
{
	obj_t *qobj1=NULL, *tobj;
	char showtime[32];
	time_t t;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, "main");
	t=(time(NULL)+time_tzoffset(sid, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");
	nes_setstr(sid->N, tobj, "showtime", showtime, -1);
	if (htnes_dotemplate(sid, "html", "motd.ns")==0) {
		htpage_footer(sid);
		return;
	}
	prints(sid, "<BR />\r\n<CENTER>\r\n");
	prints(sid, "<TABLE WIDTH=90%% BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TD COLSPAN=2><TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH STYLE='border-style:solid'><TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left>Welcome, %s.&nbsp;</TH>", str2html(sid, sid->dat->username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right>&nbsp;%s</TH>", showtime);
	prints(sid, "</TR></TABLE></TH></TR>\r\n");
	if (sql_queryf(proc->N, &qobj1, "SELECT motd FROM gw_groups where groupid = %d", sid->dat->gid)>-1) {
		if (sql_numtuples(proc->N, &qobj1)==1) {
			if (strlen(sql_getvalue(proc->N, &qobj1, 0, 0))>0) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>");
				prints(sid, "%s", sql_getvalue(proc->N, &qobj1, 0, 0));
				prints(sid, "<BR /></TD></TR>\r\n");
			}
		}
		sql_freeresult(proc->N, &qobj1);
	}
	prints(sid, "</TABLE></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	htpage_footer(sid);
	return;
}

void htpage_menuframe(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (htnes_dotemplate(sid, "html", "menuframe.ns")!=0) {
		prints(sid, "menu frame is unavailable");
	}
	htpage_footer(sid);
	return;
}

void htscript_showpage(CONN *sid, short int pages)
{
	if (pages<1) return;
	if (pages>10) pages=10;
	prints(sid, "function showpage(page) {\r\n");
	prints(sid, "	for (var i=1;i<%d;i++) {\r\n", pages+1);
	prints(sid, "		if (i==page) {\r\n");
	prints(sid, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 0px #000000';\r\n");
	prints(sid, "			document.getElementById('page'+i+'tab').bgColor='#F0F0F0';\r\n");
	prints(sid, "			document.getElementById('page'+i).style.display='block';\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 1px #000000';\r\n");
	prints(sid, "			document.getElementById('page'+i+'tab').bgColor='#E0E0E0';\r\n");
	prints(sid, "			document.getElementById('page'+i).style.display='none';\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	return;
}
