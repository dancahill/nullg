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
#include "mod_admin.h"

static int dirmake(const char *format, ...)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	struct stat sb;
	char dirname[512];
	va_list ap;

	memset(dirname, 0, sizeof(dirname));
	va_start(ap, format);
	vsnprintf(dirname, sizeof(dirname)-1, format, ap);
	va_end(ap);
	fixslashes(dirname);
	if (stat(dirname, &sb)==0) return 0;
#ifdef WIN32
	if (mkdir(dirname)!=0) {
#else
	if (mkdir(dirname, ~(int)nes_getnum(proc->N, confobj, "umask")&0777)!=0) {
#endif
		return -1;
	}
	return 0;
}

void admindomainedit(CONN *sid)
{
/*
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	REC_DOMAIN domain;
	char *ptemp;
	int domainid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (!(auth_priv(sid, "domainadmin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/admin/domaineditnew")==0) {
		domainid=0;
		dbread_domain(sid, 2, 0, &domain);
	} else {
		if ((ptemp=getgetenv(sid, "DOMAINID"))==NULL) return;
		domainid=atoi(ptemp);
		if (dbread_domain(sid, 2, domainid, &domain)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", domainid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/domainsave NAME=domainedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=domainid VALUE='%d'>\r\n", domain.domainid);
	if (domainid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Domain %d</TH></TR>\r\n", domainid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Domain</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Domain Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=domainname   value=\"%s\" SIZE=50></TD></TR>\r\n", str2html(sid, domain.domainname));
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(domainid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.domainedit.domainname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void admindomainlist(CONN *sid)
{
/*
	int i, j;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (!(auth_priv(sid, "domainadmin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (sql_query(&sqr1, "SELECT domainid, domainname FROM gw_domains ORDER BY domainname ASC")<0) return;
	if (sql_numtuples(&sqr1)<1) {
		sql_freeresult(&sqr1);
		prints(sid, "There are no domains<BR />\r\n");
		prints(sid, "<A HREF=%s/admin/domaineditnew>New Domain</A>\r\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\r\n");
		return;
	}
	if (sql_queryf(&sqr2, "SELECT domainaliasid, domainid, domainname FROM gw_domains_aliases ORDER BY domainname ASC")<0) {
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=200 STYLE='border-style:solid'>&nbsp;Domain Name&nbsp;</TH></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		prints(sid, "<TR CLASS=\"FIELDNAME\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/domainedit?domainid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "<B><A HREF=%s/admin/domainedit?domainid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "%s</A></B>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr1, i, 1)));
		for (j=0;j<sql_numtuples(&sqr2);j++) {
			if (atoi(sql_getvalue(&sqr2, j, 1))==atoi(sql_getvalue(&sqr1, i, 0))) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=4 NOWRAP style='border-style:solid'>");
				prints(sid, "&nbsp;&nbsp;&nbsp;%s</TD>", str2html(sid, sql_getvalue(&sqr2, j, 2)));
			}
		}
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<A HREF=%s/admin/domaineditnew>New Domain</A>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr2);
	sql_freeresult(&sqr1);
*/
	return;
}

void admindomainsave(CONN *sid)
{
/*
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	REC_DOMAIN domain;
	char query[2048];
	char curdate[40];
	char groupname[40];
	char *ptemp;
	time_t t;
	int domainid;
	SQLRES sqr;
	int i, j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (!(auth_priv(sid, "domainadmin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "DOMAINID"))==NULL) return;
	domainid=atoi(ptemp);
	if (dbread_domain(sid, 2, domainid, &domain)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "DOMAINNAME"))!=NULL) snprintf(domain.domainname, sizeof(domain.domainname)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_domains WHERE domainid = %d", domain.domainid)<0) return;
		prints(sid, "<CENTER>Domain %d deleted successfully</CENTER><BR />\r\n", domain.domainid);
		db_log_activity(sid, "domains", domain.domainid, "delete", "%s - %s deleted domain %d", sid->dat->in_RemoteAddr, sid->dat->username, domain.domainid);
	} else if (domain.domainid==0) {
		if (sql_queryf(&sqr, "SELECT domainname FROM gw_domains where domainname = '%s'", domain.domainname)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Domain %s already exists</CENTER><BR />\r\n", domain.domainname);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(domain.domainname)<1) {
			prints(sid, "<CENTER>Domain name is too short</CENTER><BR />\r\n");
			return;
		}
		if (sql_query(&sqr, "SELECT max(domainid) FROM gw_domains")<0) return;
		domain.domainid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (domain.domainid<1) domain.domainid=1;
		strcpy(query, "INSERT INTO gw_domains (domainid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, domainname) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", domain.domainid, curdate, curdate, domain.obj_uid, domain.obj_gid, domain.obj_did, domain.obj_gperm, domain.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain.domainname));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Domain %d added successfully</CENTER><BR />\r\n", domain.domainid);
		if (dirmake("%s/%04d",                             config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/attachments",                 config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/cgi-bin",                     config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/files",                       config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/files/users",                 config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/files/groups",                config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/files/groups/Administrators", config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/files/groups/Users",          config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/htdocs",                      config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/mail",                        config->dir_var_domains, domain.domainid)!=0) return;
		if (dirmake("%s/%04d/mailspool",                   config->dir_var_domains, domain.domainid)!=0) return;
		prints(sid, "<CENTER>Domain file folders added successfully</CENTER><BR />\r\n");
		if (sql_updatef("INSERT INTO gw_groups (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd) VALUES ('%s', '%s', '0', '0', '%d', '0', '0', 'Administrators', '', '<BR />\r\n<DIV CLASS=JUSTIFY>\r\nWelcome to NullLogic GroupServer.\n</DIV>')", curdate, curdate, domain.domainid)<0) return;
		if (sql_updatef("INSERT INTO gw_groups (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd) VALUES ('%s', '%s', '0', '0', '%d', '0', '0', 'Users', '', '<BR />\r\n<DIV CLASS=JUSTIFY>\r\nWelcome to NullLogic GroupServer.\n</DIV>')", curdate, curdate, domain.domainid)<0) return;
		if (sql_updatef("INSERT INTO gw_zones (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, zonename) VALUES ('%s', '%s', '0', '0', '%d', '0', '0', 'default')", curdate, curdate, domain.domainid)<0) return;
		if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '0', '0', '%d', '1', '1', '0', 'users')", curdate, curdate, domain.domainid)<0) return;
		if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '0', '0', '%d', '1', '1', '0', 'groups')", curdate, curdate, domain.domainid)<0) return;
		if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '0', '0', '%d', '1', '1', '/files/', 'users', 'dir', '%s', '%s', '0', '')", curdate, curdate, domain.domainid, curdate, curdate)<0) return;
		if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '0', '0', '%d', '1', '1', '/files/', 'groups', 'dir', '%s', '%s', '0', '')", curdate, curdate, domain.domainid, curdate, curdate)<0) return;
		if (sql_queryf(&sqr, "SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d AND (groupname = 'Administrators' OR groupname = 'Users')", domain.domainid)<0) return;
		for (i=0;i<sql_numtuples(&sqr);i++) {
			j=atoi(sql_getvalue(&sqr, i, 0));
			snprintf(groupname, sizeof(groupname)-1, "%s", sql_getvalue(&sqr, i, 1));
			if (groupname[0]=='A') {
				if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '0', 'Administrators')", curdate, curdate, j, domain.domainid)<0) return;
				if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '/files/groups/', 'Administrators', 'dir', '%s', '%s', '0', '')", curdate, curdate, j, domain.domainid, curdate, curdate)<0) return;
			} else if (groupname[0]=='U') {
				if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '0', 'Users')", curdate, curdate, j, domain.domainid)<0) return;
				if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '/files/groups/', 'Users', 'dir', '%s', '%s', '0', '')", curdate, curdate, j, domain.domainid, curdate, curdate)<0) return;
			}
		}
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders WHERE parentid = 0 AND obj_did = %d AND foldername = 'groups'", domain.domainid)<0) return;
		if (sql_numtuples(&sqr)==1) {
			j=atoi(sql_getvalue(&sqr, 0, 0));
			if (sql_updatef("UPDATE gw_bookmarks_folders SET parentid = %d WHERE obj_did = %d AND parentid = 0 AND (foldername = 'Administrators' OR foldername = 'Users')", j, domain.domainid)<0) return;
		}
		sql_freeresult(&sqr);
		prints(sid, "<CENTER>Domain records added successfully</CENTER><BR />\r\n");
		db_log_activity(sid, "domains", domain.domainid, "insert", "%s - %s added domain %d", sid->dat->in_RemoteAddr, sid->dat->username, domain.domainid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_queryf(&sqr, "SELECT domainname FROM gw_domains where domainname = '%s' AND domainid <> %d", domain.domainname, domain.domainid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Domain %s already exists</CENTER><BR />\r\n", domain.domainname);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(domain.domainname)<1) {
			prints(sid, "<CENTER>Domain name is too short</CENTER><BR />\r\n");
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_domains SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, domain.obj_uid, domain.obj_gid, domain.obj_gperm, domain.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "domainname = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain.domainname));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE domainid = %d", domain.domainid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Domain %d modified successfully</CENTER><BR />\r\n", domain.domainid);
		db_log_activity(sid, "domains", domain.domainid, "modify", "%s - %s modified domain %d", sid->dat->in_RemoteAddr, sid->dat->username, domain.domainid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/domainlist\">\r\n", sid->dat->in_ScriptName);
*/
	return;
}
