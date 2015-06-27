/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

void htpage_header(CONN *conn, char *title)
{
	obj_t *tobj=nsp_settable(conn->N, &conn->N->g, "_TEMP");

	nsp_setstr(conn->N, tobj, "theme", conn->dat->theme, strlen(conn->dat->theme));
	nsp_setstr(conn->N, tobj, "title", title, strlen(title));
	if (htnsp_dotemplate(conn, "html", "header.ns")!=0) {
		/* if the template is broken, then so is the server.
		 * Still, working headers means we can bitch about
		 * it more effectively.
		 */
		prints(conn,
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

void htpage_footer(CONN *conn)
{
	if (htnsp_dotemplate(conn, "html", "footer.ns")!=0) {
		prints(conn, "</BODY>\r\n</HTML>\r\n");
	}
	return;
}

int htpage_dirlist(CONN *conn)
{
	obj_t *confobj=nsp_getobj(conn->N, &conn->N->g, "_CONFIG");
	obj_t *htobj=nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj1=NULL, *cobj2=NULL;
	char  *RequestURI;
	char   dir[512];
	char   file[512];
	char   index[512];
	struct stat sb;

	if (htobj->val->type!=NT_TABLE) return -1;
	RequestURI=nsp_getstr(conn->N, htobj, "REQUEST_URI");
	if (strncmp(RequestURI, "/", 1)!=0) return -1;
	if (strstr(RequestURI, "..")!=NULL) return -1;
	snprintf(dir, sizeof(dir)-1, "%s", RequestURI+1);
	while (strlen(dir)>0&&(dir[strlen(dir)-1]=='\\'||dir[strlen(dir)-1]=='/')) dir[strlen(dir)-1]='\0';

	cobj1=nsp_getobj(conn->N, confobj, "private_htdocs_path");
	if (!nsp_isstr(cobj1)) goto tryshared;
	snprintf(file, sizeof(file)-1, "%s/%s", nsp_tostr(conn->N, cobj1), dir);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	decodeurl(file);
	fixslashes(file);
	if (stat(file, &sb)==0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(conn, file);
		/* it exists, and it's a dir */

		snprintf(index, sizeof(index)-1, "%s/%s/index.html", nsp_tostr(conn->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return filesend(conn, index);

		snprintf(index, sizeof(index)-1, "%s/%s/index.nsp", nsp_tostr(conn->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnsp_doscript_htdocs(conn, dir, "index.nsp");

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nsp_tostr(conn->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnsp_doscript_htdocs(conn, dir, "index.ns");
	} else {
		cobj1=NULL;
	}
tryshared:
	cobj2=nsp_getobj(conn->N, confobj, "shared_htdocs_path");
	if (!nsp_isstr(cobj2)) return -1;
	snprintf(file, sizeof(file)-1, "%s/%s", nsp_tostr(conn->N, cobj2), dir);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	decodeurl(file);
	fixslashes(file);
	if (stat(file, &sb)==0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(conn, file);
		/* it exists, and it's a dir */

		snprintf(index, sizeof(index)-1, "%s/%s/index.html", nsp_tostr(conn->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return filesend(conn, index);

		snprintf(index, sizeof(index)-1, "%s/%s/index.nsp", nsp_tostr(conn->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnsp_doscript_htdocs(conn, dir, "index.nsp");

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nsp_tostr(conn->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnsp_doscript_htdocs(conn, dir, "index.ns");
	} else {
		cobj2=NULL;
	}
	if (cobj1==NULL&&cobj2==NULL) return -1;
	/* or just list the dir */
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
//	htpage_header(conn, RequestURI);
	htpage_topmenu(conn, "dir");
	if (htnsp_dotemplate(conn, "html", "dir.ns")!=0) {
		prints(conn, "<CENTER>Access Denied - Directory listing not allowed.</CENTER>\r\n");
	}
	htpage_footer(conn);
	return 0;
}

void htpage_login(CONN *conn)
{
	obj_t *htobj=nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	obj_t *tobj=nsp_settable(conn->N, &conn->N->g, "_TEMP");
	obj_t *qobj=NULL;
	char *RequestMethod=nsp_getstr(conn->N, htobj, "REQUEST_METHOD");
	char *RequestURI=nsp_getstr(conn->N, htobj, "REQUEST_URI");
	char pageuri[255];
	char domain[255];
	char *ptemp;
	int numdomains;

	memset(pageuri, 0, sizeof(pageuri));
	memset(domain, 0, sizeof(domain));
	if ((ptemp=getpostenv(conn, "PAGEURI"))!=NULL) {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", ptemp);
	} else {
		snprintf(pageuri, sizeof(pageuri)-1, "%s", RequestURI);
	}
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	htpage_header(conn, "NullLogic GroupServer Login");
//	if (sql_query(proc->N, &qobj, "SELECT COUNT(*) FROM nullsd_entries WHERE class = 'organization'")<0) return;
	if (sql_query(proc->N, &qobj, "SELECT COUNT(*) FROM gw_domains")<0) return;
	numdomains=atoi(sql_getvalue(proc->N, &qobj, 0, 0));
	sql_freeresult(proc->N, &qobj);
	tobj=nsp_settable(conn->N, &conn->N->g, "_TEMP");
	nsp_setnum(conn->N, tobj, "domains",    numdomains);
	nsp_setstr(conn->N, tobj, "domainname", conn->dat->domainname, -1);
	nsp_setstr(conn->N, tobj, "pageuri",    pageuri, -1);
	nsp_setstr(conn->N, tobj, "software",   SERVER_NAME, -1);
	nsp_setstr(conn->N, tobj, "username",   conn->dat->username, -1);
//	nsp_exec(conn->N, "printvar(_GLOBALS);");
	if (htnsp_dotemplate(conn, "html", "login.ns")==0) {
		htpage_footer(conn);
		return;
	}
	prints(conn, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\nif (self!=parent) open('/','_top');\r\n// -->\r\n</SCRIPT>\r\n");
	prints(conn, "<BR />\r\n<CENTER>\r\n");
	if (strcmp(RequestMethod, "POST")==0) {
		prints(conn, "<FONT COLOR=RED><B>Incorrect username or password</B></FONT>\r\n");
	}
	prints(conn, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(conn, "<FORM METHOD=POST ACTION=\"/\" AUTOCOMPLETE=OFF NAME=login>\r\n");
	prints(conn, "<INPUT TYPE=hidden NAME=pageuri VALUE=\"%s\">\r\n", pageuri);
	if (numdomains<2) {
		prints(conn, "<INPUT TYPE=hidden NAME=domain VALUE=\"NULL\">\r\n");
	}
	prints(conn, "<TR><TH COLSPAN=2 STYLE='padding:1px'>%s Login</TH></TR>\r\n", SERVER_NAME);
	prints(conn, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=username SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", conn->dat->username);
	prints(conn, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=password SIZE=25 MAXLENGTH=50 VALUE=''></TD></TR>\r\n");
	if (numdomains>1) {
		if (strlen(conn->dat->domainname)==0) {
			snprintf(domain, sizeof(domain)-1, "%s", nsp_getstr(proc->N, htobj, "HTTP_HOST"));
		} else {
			snprintf(domain, sizeof(domain)-1, "%s", conn->dat->domainname);
		}
		prints(conn, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Domain&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=domain SIZE=25 MAXLENGTH=50 VALUE='%s'></TD></TR>\r\n", domain);
	}
	prints(conn, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Login'></TD></TR>\r\n");
	prints(conn, "</FORM>\r\n</TABLE>\r\n</CENTER>\r\n");
	htpage_footer(conn);
	return;
}

void htpage_logout(CONN *conn)
{
	db_log_activity(conn, "login", 0, "logout", "%s - Logout: username=%s", conn->socket.RemoteAddr, conn->dat->username);
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	htpage_header(conn, "NullLogic GroupServer Logout");
	if (htnsp_dotemplate(conn, "html", "logout.ns")!=0) {
		prints(conn, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('/');\r\n// -->\r\n</SCRIPT>\r\n");
		prints(conn, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='/'\"></NOSCRIPT>\r\n");
	}
	htpage_footer(conn);
	return;
}

void htpage_frameset(CONN *conn)
{
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	htpage_header(conn, "NullLogic GroupServer Logout");
	if (htnsp_dotemplate(conn, "html", "frames.ns")!=0) {
		prints(conn,
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Frameset//EN\">\r\n"
			"<HTML>\r\n<HEAD>\r\n<TITLE>NullLogic GroupServer</TITLE>\r\n"
			"<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n"
			"if (self!=parent) open('/','_top');\r\n"
			"// -->\r\n</SCRIPT>\r\n"
			"</HEAD>\r\n"
			"<FRAMESET COLS=\"125,*\" BORDER=0 FRAMEBORDER=0 FRAMESPACING=0>\r\n"
			"<FRAME BORDER=0 NAME=\"gwmenu\" SRC=/frames/menu MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=NO>\r\n");
		prints(conn, "<FRAME BORDER=0 NAME=\"gwmain\" SRC=/frames/motd MARGINHEIGHT=0 MARGINWIDTH=0 NORESIZE SCROLLING=YES>\r\n"
			"</FRAMESET>\r\n"
			"To view this page, you need a web browser capable of displaying frames.\r\n"
			"</HTML>\r\n"
		);
	}
	htpage_motd(conn);
}

void htpage_topmenu(CONN *conn, char *modname)
{
	htpage_header(conn, "NullLogic GroupServer");
	nsp_setstr(conn->N, nsp_settable(conn->N, &conn->N->g, "_TEMP"), "modname", modname, -1);
	if (htnsp_dotemplate(conn, "html", "headmenu.ns")!=0) {
		prints(conn, "error loading head menu");
	}
	return;
}

void htpage_motd(CONN *conn)
{
	obj_t *tobj;
	char showtime[32];
	time_t t;

	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(conn, "main");
	t=(time(NULL)+time_tzoffset(conn, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	tobj=nsp_settable(conn->N, &conn->N->g, "_TEMP");
	nsp_setstr(conn->N, tobj, "showtime", showtime, -1);
	if (htnsp_dotemplate(conn, "html", "motd.ns")!=0) {
		prints(conn, "motd is unavailable");
	}
	htpage_footer(conn);
	return;
}

void htpage_menuframe(CONN *conn)
{
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	if (htnsp_dotemplate(conn, "html", "menuframe.ns")!=0) {
		prints(conn, "menu frame is unavailable");
	}
	htpage_footer(conn);
	return;
}

void htscript_showpage(CONN *conn, short int pages)
{
	if (pages<1) return;
	if (pages>10) pages=10;
	prints(conn, "function showpage(page) {\r\n");
	prints(conn, "	for (var i=1;i<%d;i++) {\r\n", pages+1);
	prints(conn, "		if (i==page) {\r\n");
	prints(conn, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 0px #000000';\r\n");
	prints(conn, "			document.getElementById('page'+i+'tab').bgColor='#F0F0F0';\r\n");
	prints(conn, "			document.getElementById('page'+i).style.display='block';\r\n");
	prints(conn, "		} else {\r\n");
	prints(conn, "			document.getElementById('page'+i+'tab').style.borderBottom='solid 1px #000000';\r\n");
	prints(conn, "			document.getElementById('page'+i+'tab').bgColor='#E0E0E0';\r\n");
	prints(conn, "			document.getElementById('page'+i).style.display='none';\r\n");
	prints(conn, "		}\r\n");
	prints(conn, "	}\r\n");
	prints(conn, "}\r\n");
	return;
}
