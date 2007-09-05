/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nes_tostr(sid->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.ns");

		snprintf(index, sizeof(index)-1, "%s/%s/index.nes", nes_tostr(sid->N, cobj1), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.nes");
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

		snprintf(index, sizeof(index)-1, "%s/%s/index.ns", nes_tostr(sid->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.ns");

		snprintf(index, sizeof(index)-1, "%s/%s/index.nes", nes_tostr(sid->N, cobj2), dir);
		decodeurl(index);
		fixslashes(index);
		if (stat(index, &sb)==0) return htnes_doscript_htdocs(sid, dir, "index.nes");
	} else {
		cobj2=NULL;
	}
	if (cobj1==NULL&&cobj2==NULL) return -1;
	/* or just list the dir */
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, RequestURI);
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

void htpage_reloadframe(CONN *sid)
{
	HTMOD_EMAIL_SYNC mod_email_sync;
	char posttime[32];
	int isreminder;
	int i, j;
	int a, b;
	obj_t *qobj1=NULL;
	obj_t *qobj2=NULL;
	time_t t;
	int notice=0;

	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	sql_updatef(proc->N, "UPDATE gw_users_sessions SET obj_mtime = '%s' WHERE token = '%s' AND userid = %d AND domainid = %d", posttime, sid->dat->token, sid->dat->uid, sid->dat->did);
	prints(sid, "document.getElementById('remstatus').innerHTML=\"\";\r\n");
	t=time(NULL)+604800;
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (sql_queryf(proc->N, &qobj1, "SELECT eventstart, reminder FROM gw_events where eventstart < '%s' and assignedto = %d and reminder > 0 AND obj_did = %d ORDER BY eventstart ASC", posttime, sid->dat->uid, sid->dat->did)<0) return;
		for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
			a=time_sql2unix(sql_getvalue(proc->N, &qobj1, i, 0))-time(NULL);
			b=a-atoi(sql_getvalue(proc->N, &qobj1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "document.getElementById('remstatus').innerHTML+=\"<A HREF=javascript:ListEvents();><FONT COLOR=WHITE>New Event Reminders</FONT></A><BR />\";\r\n");
			prints(sid, "ListEvents();\r\n");
		}
		sql_freeresult(proc->N, &qobj1);
	}
	if (auth_priv(sid, "calendar")&A_READ) {
		isreminder=0;
		if (sql_queryf(proc->N, &qobj1, "SELECT duedate, reminder FROM gw_tasks where duedate < '%s' and assignedto = %d and reminder > 0 AND obj_did = %d ORDER BY duedate ASC", posttime, sid->dat->uid, sid->dat->did)<0) return;
		for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
			a=time_sql2unix(sql_getvalue(proc->N, &qobj1, i, 0))-time(NULL);
			a-=time_tzoffset(sid, time_sql2unix(sql_getvalue(proc->N, &qobj1, i, 0)));
			b=a-atoi(sql_getvalue(proc->N, &qobj1, i, 1))*60;
			if (b<0) isreminder=1;
		}
		if (isreminder) {
			prints(sid, "document.getElementById('remstatus').innerHTML+=\"<A HREF=javascript:ListTasks();><FONT COLOR=WHITE>New Task Reminders</FONT></A><BR />\";\r\n");
			prints(sid, "ListTasks();\r\n");
		}
		sql_freeresult(proc->N, &qobj1);
	}
	if (auth_priv(sid, "messages")&A_READ) {
		if (sql_queryf(proc->N, &qobj1, "SELECT sender FROM gw_messages WHERE obj_uid = %d AND obj_did = %d AND rcpt = %d AND status > 2", sid->dat->uid, sid->dat->did, sid->dat->uid)<0) return;
		if (sql_numtuples(proc->N, &qobj1)>0) {
			if (sql_queryf(proc->N, &qobj2, "SELECT userid, username FROM gw_users WHERE domainid = %d", sid->dat->did)<0) {
				sql_freeresult(proc->N, &qobj1);
				return;
			}
			for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
				for (j=0;j<sql_numtuples(proc->N, &qobj2);j++) {
					if (atoi(sql_getvalue(proc->N, &qobj1, i, 0))==atoi(sql_getvalue(proc->N, &qobj2, j, 0))) {
						prints(sid, "document.getElementById('remstatus').innerHTML+=\"<FONT COLOR=WHITE>New Messages</FONT><BR />\";\r\n");
						prints(sid, "ListMsgs(%d, \"%s\");\r\n", atoi(sql_getvalue(proc->N, &qobj2, j, 0)), sql_getvalue(proc->N, &qobj2, j, 1));
					}
				}
			}
			sql_freeresult(proc->N, &qobj2);
		}
		sql_freeresult(proc->N, &qobj1);
	}
	if ((mod_email_sync=(HTMOD_EMAIL_SYNC)module_call("mod_email_sync"))!=NULL) {
		notice=0;
		if (sql_queryf(proc->N, &qobj1, "SELECT mailaccountid, accountname, poppassword, lastcount, notify, lastcheck FROM gw_email_accounts where obj_uid = %d and notify > 0 AND obj_did = %d", sid->dat->uid, sid->dat->did)<0) return;
		for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
			/* sid->dat->mailcurrent=atoi(sql_getvalue(proc->N, &qobj1, i, 0)); */
			j=time_sql2unix(sql_getvalue(proc->N, &qobj1, i, 5));
			if ((strlen(sql_getvalue(proc->N, &qobj1, i, 2))>0)&&(j+(atoi(sql_getvalue(proc->N, &qobj1, i, 4))*60)<time(NULL))) {
				mod_email_sync(sid, 0);
			}
/*
			if (sql_queryf(proc->N, &qobj2, "SELECT mailheaderid FROM gw_email_headers where obj_uid = %d AND obj_did = %d AND accountid = %d AND folder > 0 AND status = 'n'", sid->dat->uid, sid->dat->did, sid->dat->mailcurrent)<0) break;
			if (sql_numtuples(proc->N, &qobj2)>0) notice=1;
			sql_freeresult(proc->N, &qobj2);
*/
		}
		sql_freeresult(proc->N, &qobj1);
		if (notice) {
			prints(sid, "document.getElementById('remstatus').innerHTML+=\"<A HREF=javascript:ListEmail();><FONT COLOR=WHITE>New Mail</FONT></A><BR />\";\r\n");
			prints(sid, "ListEmail();\r\n");
		}
	}
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
		prints(sid, "<OPTION VALUE='%d'%s>%02d\r\n", i, i==atoi(day)?" SELECTED":"", i);
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
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, i==selected?" SELECTED":"", timeoption[i]);
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
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, i==atoi(month)?" SELECTED":"", option[i-1]);
	}
	return;
}

void htselect_qhours(CONN *sid, int selected)
{
	int i;

	for (i=0;i<24;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d hour%s\r\n", i, i==selected?" SELECTED":"", i, i==1?"":"s");
	}
}

void htselect_qminutes(CONN *sid, int selected)
{
	int i;

	for (i=0;i<60;i+=15) {
		prints(sid, "<OPTION VALUE='%d'%s>%d minutes\r\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_minutes(CONN *sid, int selected)
{
	int i;

	for (i=0;i<60;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%d minutes\r\n", i, i==selected?" SELECTED":"", i);
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
			prints(sid, "<OPTION VALUE='%s'%s>%s\r\n", temptime, strcmp(selected, temptime)==0?" SELECTED":"", timeoption[i*4+j]);
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
		prints(sid, "<OPTION VALUE='%d'%s>%d\r\n", i, i==atoi(year)?" SELECTED":"", i);
	}
	return;
}

void htselect_contact(CONN *sid, int selected)
{
	int i, j;
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT contactid, surname, givenname FROM gw_contacts WHERE obj_did = %d order by surname, givenname ASC", sid->dat->did)<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj, i, 1)));
		if (strlen(sql_getvalue(proc->N, &qobj, i, 1))&&strlen(sql_getvalue(proc->N, &qobj, i, 2))) prints(sid, ", ");
		prints(sid, "%s\r\n", str2html(sid, sql_getvalue(proc->N, &qobj, i, 2)));
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

void htselect_domain(CONN *sid, int selected)
{
	int i, j;
	obj_t *qobj=NULL;

	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (sql_queryf(proc->N, &qobj, "SELECT domainid, domainname FROM gw_domains order by domainname ASC")<0) return;
	} else {
		if (sql_queryf(proc->N, &qobj, "SELECT domainid, domainname FROM gw_domains WHERE domainid = %d ORDER BY domainname ASC", sid->dat->did)<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj, i, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

void htselect_eventstatus(CONN *sid, int selected)
{
	char *option[]={ "Open", "Closed", };
	int i;

	for (i=0;i<2;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_group(CONN *sid, int perm, int selected, int domainid)
{
	int i, j;
	obj_t *qobj=NULL;

	if (perm&A_ADMIN) {
		if (sql_queryf(proc->N, &qobj, "SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d order by groupname ASC", domainid)<0) return;
	} else {
		if (sql_queryf(proc->N, &qobj, "SELECT gw_groups.groupid, gw_groups.groupname FROM gw_groups, gw_groups_members WHERE (gw_groups_members.groupid = gw_groups.groupid AND (gw_groups_members.userid = %d OR gw_groups_members.groupid = %d)) AND gw_groups.obj_did = %d ORDER BY groupname ASC", sid->dat->uid, sid->dat->gid, domainid)<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj, i, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

void htselect_number(CONN *sid, int selected, int start, int end, int increment)
{
	int i;

	if (increment<1) increment=1;
	for (i=start;i<end+1;i+=increment) {
		prints(sid, "<OPTION VALUE='%d'%s>%d\r\n", i, i==selected?" SELECTED":"", i);
	}
}

void htselect_priority(CONN *sid, int selected)
{
	char *option[]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int i;

	for (i=0;i<5;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, i==selected?" SELECTED":"", option[i]);
	}
	return;
}

void htselect_reminder(CONN *sid, int selected)
{
	int i;

	prints(sid, "<OPTION VALUE='0'>No reminder\r\n");
	prints(sid, "<OPTION VALUE='15'%s>15 minutes before\r\n", (selected==15)?" SELECTED":"");
	prints(sid, "<OPTION VALUE='30'%s>30 minutes before\r\n", (selected==30)?" SELECTED":"");
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
			prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i*60, i*60==selected?" SELECTED":"", htview_reminder(sid, i*60));
			continue;
		default:
			continue;
		}
	}
	return;
}

void htselect_user(CONN *sid, int selected, int domainid)
{
	int i, j;
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", domainid)<0) return;
	prints(sid, "<OPTION VALUE=0>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj, i, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

void htselect_zone(CONN *sid, int selected, int domainid)
{
	int i, j;
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT zoneid, zonename FROM gw_zones WHERE obj_did = %d order by zonename ASC", domainid)<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj, i, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

char *htview_callaction(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT actionname FROM gw_calls_actions WHERE actionid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 0)));
	}
	sql_freeresult(proc->N, &qobj);
	return buffer;
}

char *htview_contact(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT surname, givenname FROM gw_contacts WHERE contactid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 0)));
		if (strlen(sql_getvalue(proc->N, &qobj, 0, 0))&&strlen(sql_getvalue(proc->N, &qobj, 0, 1))) strcat(buffer, ", ");
		strncatf(buffer, sizeof(sid->dat->smallbuf[0])-strlen(buffer)-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return buffer;
}

char *htview_domain(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT domainname FROM gw_domains WHERE domainid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 0)));
	}
	sql_freeresult(proc->N, &qobj);
	return buffer;
}

char *htview_eventclosingstatus(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT closingname FROM gw_events_closings WHERE eventclosingid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 0)));
	}
	sql_freeresult(proc->N, &qobj);
	return buffer;
}

char *htview_eventstatus(int selected)
{
	char *option[]={ "Open", "Closed" };

	if ((selected<0)||(selected>1)) return "";
	return option[selected];
}

char *htview_eventtype(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT eventtypename FROM gw_events_types WHERE eventtypeid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 0)));
	}
	sql_freeresult(proc->N, &qobj);
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
		{ "-07-04", "Independence Day" },
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
	obj_t *qobj=NULL;

	if (sql_queryf(proc->N, &qobj, "SELECT userid, username FROM gw_users WHERE userid = %d", selected)<0) return buffer;
	if (sql_numtuples(proc->N, &qobj)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(proc->N, &qobj, 0, 1)));
	}
	sql_freeresult(proc->N, &qobj);
	return buffer;
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
