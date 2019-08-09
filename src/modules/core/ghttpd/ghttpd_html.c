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
#include "ghttpd_main.h"
#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#endif

void htpage_header(CONN *sid, char *title)
{
	HTMOD_HTML_HEADER mod_html_header;

	if ((mod_html_header=module_call("mod_html_header"))!=NULL) {
		mod_html_header(sid, title);
		return;
	}
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n");
	prints(sid, "<HEAD>\r\n");
	prints(sid, "<TITLE>%s</TITLE>\r\n", title);
	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=iso-8859-1\">\r\n");
	prints(sid, "<STYLE TYPE=text/css>\r\n");
	prints(sid, ".JUSTIFY { text-align: justify; }\r\n");
	prints(sid, ".TBAR    { color: #505050; text-decoration: none; font-family: Geneva, Arial,Verdana; font-size: 8pt; }\r\n");
	prints(sid, "A        { text-decoration: none; }\r\n");
	prints(sid, "A:HOVER  { text-decoration: underline; }\r\n");
	prints(sid, "</STYLE>\r\n");
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/%s/themes/%s/style.css\">\r\n", SERVER_BASENAME, sid->dat->user_theme);
	prints(sid, "</HEAD>\r\n");
	prints(sid, "<BODY BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\"");
	prints(sid, " STYLE=\"margin:0\" CLASS=\"MAINBACK\">\r\n");
}

void htpage_footer(CONN *sid)
{
	HTMOD_HTML_FOOTER mod_html_footer;

	if ((mod_html_footer=module_call("mod_html_footer"))!=NULL) {
		mod_html_footer(sid);
		return;
	}
	prints(sid, "</BODY>\r\n</HTML>\r\n");
}

int htpage_dirlist(CONN *sid)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR	*handle;
	char	file[1024];
	char	index[1024];
	char	showfile[1024];
	struct	stat sb;
	char	*directory;
	char timebuf[100];
	time_t t;

	if (strncmp(sid->dat->in_RequestURI, "/", 1)!=0) {
		return -1;
	}
	directory=sid->dat->in_RequestURI+1;
//	snprintf(file, sizeof(file)-1, "%s/%s", config->dir_var_htdocs, directory);
	snprintf(file, sizeof(file)-1, "%s/%04d/htdocs/%s", config->dir_var_domains, sid->dat->user_did, directory);
	decodeurl(file);
	fixslashes(file);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	if (strstr(file, "..")!=NULL) return -1;
	snprintf(index, sizeof(index)-1, "%s/%s/index.html", config->dir_var_htdocs, directory);
	decodeurl(index);
	fixslashes(index);
	if (stat(index, &sb)==0) {
		filesend(sid, index);
		return 0;
	}
	snprintf(index, sizeof(index)-1, "%s/%s/index.php", config->dir_var_htdocs, directory);
	decodeurl(index);
	fixslashes(index);
	if (stat(index, &sb)==0) {
		snprintf(sid->dat->in_ScriptName, sizeof(sid->dat->in_ScriptName)-1, "/%s/index.php", directory);
//		cgi_main(sid);
		return 0;
	}
	if (stat(file, &sb)!=0) return -1;
	if (!(sb.st_mode & S_IFDIR)) return filesend(sid, file);
	t=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime(&t));
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, sid->dat->in_RequestURI);
	prints(sid, "<BR><CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=#0000A0><TH COLSPAN=4 STYLE='border-style:solid'><FONT COLOR=white>Index of %s</FONT></TH></TR>\r\n", sid->dat->in_RequestURI);
	prints(sid, "<TR BGCOLOR=#E0E0E0>");
	prints(sid, "<TH width=20%% STYLE='border-style:solid'>Filename</TH><TH width=10%% STYLE='border-style:solid'>Size</TH>");
	prints(sid, "<TH width=10%% STYLE='border-style:solid'>Date</TH><TH width=60%% STYLE='border-style:solid'>Description</TH></TR>\r\n");
	handle=opendir(file);
	while ((dentry=readdir(handle))!=NULL) {
//		snprintf(file, sizeof(file)-1, "%s/%s%s", config->dir_var_htdocs, directory, dentry->d_name);
		snprintf(file, sizeof(file)-1, "%s/%04d/htdocs/%s%s", config->dir_var_domains, sid->dat->user_did, directory, dentry->d_name);
		fixslashes(file);
		stat(file, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if ((strcmp("..", dentry->d_name)==0)&&(strcmp("/files/", sid->dat->in_RequestURI)==0)) continue;
		if (strcmp("..", dentry->d_name)==0) {
			prints(sid, "<TR BGCOLOR=#F0F0F0><TD COLSPAN=4 STYLE='border-style:solid'><IMG SRC=/icons/foldero.gif>");
			prints(sid, "<A HREF=%s/> Parent Directory</A></TD>\r\n", dentry->d_name);
			continue;
		}
		strftime(timebuf, sizeof(timebuf), "%b %d %Y %H:%M", localtime(&sb.st_mtime));
		memset(showfile, 0, sizeof(showfile));
		snprintf(showfile, sizeof(showfile)-1, "%s", dentry->d_name);
		prints(sid, "<TR BGCOLOR=#F0F0F0><TD ALIGN=left NOWRAP STYLE='border-style:solid'>");
		if (sb.st_mode & S_IFDIR) {
			prints(sid, "<IMG SRC=/icons/folder.gif>&nbsp;<A HREF=");
			printhex(sid, "%s", showfile);
			prints(sid, "/>%s/</A></TD>", dentry->d_name);
		} else {
			prints(sid, "<IMG SRC=/icons/default.gif>&nbsp;<A HREF=");
			printhex(sid, "%s", showfile);
			prints(sid, ">%s</A></TD>", dentry->d_name);
		}
		if (sb.st_size>1048576) {
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%10.1f M</TD>\r\n", (float)sb.st_size/1048576.0);
		} else {
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%10.1f K</TD>\r\n", (float)sb.st_size/1024.0);
		}
		prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s</TD>\r\n", timebuf);
		prints(sid, "<TD ALIGN=left NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\r\n");
	}
	closedir(handle);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "</BODY></HTML>\r\n");
	return 0;
}

void htpage_login(CONN *sid)
{
	HTMOD_HTML_LOGIN mod_html_login;
	char pageuri[200];
	char username[32];
	char password[32];
	char domain[64];
	char *ptemp;
	SQLRES sqr;

	if ((mod_html_login=module_call("mod_html_login"))!=NULL) {
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
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, SERVER_NAME);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\n");
	prints(sid, "if (self!=parent) open('%s/','_top');\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		prints(sid, "<FONT COLOR=RED><B>Incorrect username or password</B></FONT>\r\n");
	}
	prints(sid, "<FORM METHOD=POST ACTION=\"%s/\" AUTOCOMPLETE=OFF NAME=login>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
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
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Login'></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</FORM>\r\n");
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

void htpage_logout(CONN *sid)
{
	HTMOD_HTML_LOGOUT mod_html_logout;

	if ((mod_html_logout=module_call("mod_html_logout"))!=NULL) {
		mod_html_logout(sid);
		return;
	}
	db_log_activity(sid, "login", 0, "logout", "%s - Logout: username=%s", sid->dat->in_RemoteAddr, sid->dat->user_username);
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_header(sid, SERVER_NAME);
	prints(sid, "<CENTER>\r\n<BR><BR>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<TR><TH>%s Logout</TH></TR>\r\n", SERVER_NAME);
	prints(sid, "<TR CLASS=FIELDVAL><TD>\r\n");
	prints(sid, "You have successfully logged out.\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace('%s/');\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL='%s/'\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void htpage_frameset(CONN *sid)
{
	HTMOD_HTML_FRAMESET mod_html_frameset;

	if ((mod_html_frameset=module_call("mod_html_frameset"))!=NULL) {
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

	if ((mod_html_topmenu=module_call("mod_html_topmenu"))!=NULL) {
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
	} else if (menu==MENU_EMAIL) {
		prints(sid, "function ComposeMail() {\r\n");
		prints(sid, "	window.open('%s/mail/write','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
		prints(sid, "function MsgTo(msg) {\r\n");
		prints(sid, "	window.open('%s/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	} else if (menu==MENU_FILES) {
		prints(sid, "function FileUpload() {\r\n");
		if ((ptemp=getgetenv(sid, "LOCATION"))==NULL) ptemp=sid->dat->in_RequestURI;
		prints(sid, "	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');\r\n", sid->dat->in_ScriptName, ptemp);
		prints(sid, "}\r\n");
	}
	if (sid->dat->user_menustyle==0) {
		prints(sid, "function ListUsers() {\r\n");
		prints(sid, "	window.open('%s/messages/userlist','msguserlist','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=200,height=300');\r\n", sid->dat->in_ScriptName);
		prints(sid, "}\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if ((menu==MENU_EMAIL)&&(sid->dat->user_menustyle>0)) {
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
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/configedit>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_config);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/activitylist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_logs);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/domainlist>DOMAINS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/syscheck>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_check);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/userlist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_users);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/grouplist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_groups);
			prints(sid, "<A CLASS='TBAR' HREF=%s/admin/zonelist>%s</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, lang.menu_adm_zones);
			break;
		case MENU_BOOKMARKS:
			if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) folderid=atoi(ptemp);
			if ((ptemp=getgetenv(sid, "PARENT"))!=NULL) folderid=atoi(ptemp);
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
		case MENU_EMAIL:
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
		case MENU_FINANCE:
			prints(sid, "<A CLASS='TBAR' HREF=%s/finance/accounts>ACCOUNTS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/finance/journal>JOURNAL</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_FORUMS:
			prints(sid, "<A CLASS='TBAR' HREF=%s/forums/list>LIST FORUMS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			if (auth_priv(sid, "forums")&A_ADMIN) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/grouplist>GROUPS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
				prints(sid, "<A CLASS='TBAR' HREF=%s/forums/post>NEW FORUM</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_MAIN:
			if (module_exists("mod_html")) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/frames/motd>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/>MAIN</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
			break;
		case MENU_NOTES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/list>LIST NOTES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/notes/editnew>NEW NOTE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_INVOICES:
			prints(sid, "<A CLASS='TBAR' HREF=%s/invoices/list>INVOICES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/invoices/inventorylist>INVENTORY</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
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
			prints(sid, "<A CLASS='TBAR' HREF=%s/search/sqlrun>SQL QUERIES</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			break;
		case MENU_WEBLOG:
			prints(sid, "<A CLASS='TBAR' HREF=%s/weblog/list onClick=\"location.replace('%s/weblog/list');return false;\">LIST BLOGS</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
			prints(sid, "<A CLASS='TBAR' HREF=%s/weblog/editnew onClick=\"location.replace('%s/weblog/editnew');return false;\">NEW BLOG</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName, sid->dat->in_ScriptName);
			break;
	}
	prints(sid, "<A CLASS='TBAR' HREF=javascript:window.print()>%s</A>", lang.menu_top_print);
	prints(sid, "&nbsp;</TD>\r\n<TD ALIGN=right NOWRAP>&nbsp;");
	if (sid->dat->user_menustyle>0) {
		if (module_exists("mod_profile")) {
			if (menu==MENU_EMAIL) {
				prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit TARGET=gwmain>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<A CLASS='TBAR' HREF=%s/profile/edit>PROFILE</A>&nbsp;&middot;&nbsp;", sid->dat->in_ScriptName);
			}
		}
		prints(sid, "<A CLASS='TBAR' HREF=%s/logout TARGET=_top>%s</A>", sid->dat->in_ScriptName, lang.menu_top_logout);
	}
	prints(sid, "&nbsp;&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	/* flushbuffer(sid); */
}

void htpage_motd(CONN *sid)
{
	HTMOD_HTML_MOTD mod_html_motd;
	char showtime[32];
	time_t t;
	SQLRES sqr;

	if ((mod_html_motd=module_call("mod_html_motd"))!=NULL) {
		mod_html_motd(sid);
		return;
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_MAIN);
	t=(time(NULL)+time_tzoffset(sid, time(NULL)));
	strftime(showtime, sizeof(showtime), "%A, %B %d, %Y", gmtime(&t));
	prints(sid, "<BR>\r\n<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'><TR><TH STYLE='border-style:solid'>\r\n");
	prints(sid, "<TABLE WIDTH=100%% BORDER=0 CELLPADDING=2 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=left>Welcome, %s.&nbsp;</TH>\r\n", str2html(sid, sid->dat->user_username));
	prints(sid, "<TH WIDTH=100%% NOWRAP ALIGN=right>&nbsp;%s</TH>\r\n", showtime);
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "</TH></TR>\r\n");
	if (sql_queryf(&sqr, "SELECT motd FROM gw_groups where groupid = %d", sid->dat->user_gid)<0) return;
	if (sql_numtuples(&sqr)==1) {
		if (strlen(sql_getvalue(&sqr, 0, 0))>0) {
			prints(sid, "<TR><TD COLSPAN=2 CLASS=\"FIELDVAL\" STYLE='border-style:solid'>\r\n");
			prints(sid, "%s", sql_getvalue(&sqr, 0, 0));
			prints(sid, "<BR></TD></TR>\r\n");
		}
	}
	sql_freeresult(&sqr);
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
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT contactid, surname, givenname FROM gw_contacts WHERE obj_did = %d order by surname, givenname ASC", sid->dat->user_did)<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
		if (strlen(sql_getvalue(&sqr, i, 1))&&strlen(sql_getvalue(&sqr, i, 2))) prints(sid, ", ");
		prints(sid, "%s\r\n", str2html(sid, sql_getvalue(&sqr, i, 2)));
	}
	sql_freeresult(&sqr);
	return;
}

void htselect_domain(CONN *sid, int selected)
{
	int i, j;
	SQLRES sqr;

	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT domainid, domainname FROM gw_domains order by domainname ASC")<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT domainid, domainname FROM gw_domains WHERE domainid = %d ORDER BY domainname ASC", sid->dat->user_did)<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
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
	SQLRES sqr;

	if (perm&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d order by groupname ASC", domainid)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT gw_groups.groupid, gw_groups.groupname FROM gw_groups, gw_groups_members WHERE (gw_groups_members.groupid = gw_groups.groupid AND (gw_groups_members.userid = %d OR gw_groups_members.groupid = %d)) AND gw_groups.obj_did = %d ORDER BY groupname ASC", sid->dat->user_uid, sid->dat->user_gid, domainid)<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
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
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", domainid)<0) return;
	prints(sid, "<OPTION VALUE=0>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

void htselect_zone(CONN *sid, int selected, int domainid)
{
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT zoneid, zonename FROM gw_zones WHERE obj_did = %d order by zonename ASC", domainid)<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

char *htview_callaction(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT actionname FROM gw_calls_actions WHERE actionid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
	}
	sql_freeresult(&sqr);
	return buffer;
}

char *htview_contact(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT surname, givenname FROM gw_contacts WHERE contactid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
		if (strlen(sql_getvalue(&sqr, 0, 0))&&strlen(sql_getvalue(&sqr, 0, 1))) strcat(buffer, ", ");
		strncatf(buffer, sizeof(sid->dat->smallbuf[0])-strlen(buffer)-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 1)));
	}
	sql_freeresult(&sqr);
	return buffer;
}

char *htview_domain(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT domainname FROM gw_domains WHERE domainid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
	}
	sql_freeresult(&sqr);
	return buffer;
}

char *htview_eventclosingstatus(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT closingname FROM gw_events_closings WHERE eventclosingid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
	}
	sql_freeresult(&sqr);
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
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT eventtypename FROM gw_events_types WHERE eventtypeid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
	}
	sql_freeresult(&sqr);
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
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE userid = %d", selected)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 1)));
	}
	sql_freeresult(&sqr);
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
