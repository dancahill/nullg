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
#include "http_mod.h"
#include "mod_admin.h"

void admindomainedit(CONN *sid)
{
	REC_DOMAIN domain;
	char *ptemp;
	int domainid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/domaineditnew")==0) {
		domainid=0;
		dbread_domain(sid, 2, 0, &domain);
	} else {
		if ((ptemp=getgetenv(sid, "DOMAINID"))==NULL) return;
		domainid=atoi(ptemp);
		if (dbread_domain(sid, 2, domainid, &domain)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", domainid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/domainsave NAME=domainedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=domainid VALUE='%d'>\n", domain.domainid);
	if (domainid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Domain %d</TH></TR>\n", domainid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Domain</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Domain Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=domainname   value=\"%s\" SIZE=50></TD></TR>\n", str2html(sid, domain.domainname));
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(domainid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.domainedit.domainname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void admindomainlist(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_query("SELECT domainid, domainname FROM gw_domains ORDER BY domainid ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=150 STYLE='border-style:solid'>&nbsp;Domain Name&nbsp;</TH></TR>\n");
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/domainedit?domainid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/domainedit?domainid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no domains<BR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "<A HREF=%s/admin/domaineditnew>New Domain</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void admindomainsave(CONN *sid)
{
	REC_DOMAIN domain;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int domainid;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "DOMAINID"))==NULL) return;
	domainid=atoi(ptemp);
	if (dbread_domain(sid, 2, domainid, &domain)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "DOMAINNAME"))!=NULL) snprintf(domain.domainname, sizeof(domain.domainname)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_domains WHERE domainid = %d", domain.domainid)<0) return;
		prints(sid, "<CENTER>Domain %d deleted successfully</CENTER><BR>\n", domain.domainid);
		db_log_activity(sid, "domains", domain.domainid, "delete", "%s - %s deleted domain %d", sid->dat->in_RemoteAddr, sid->dat->user_username, domain.domainid);
	} else if (domain.domainid==0) {
		if ((sqr=sql_queryf("SELECT domainname FROM gw_domains where domainname = '%s'", domain.domainname))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER>Domain %s already exists</CENTER><BR>\n", domain.domainname);
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if (strlen(domain.domainname)<1) {
			prints(sid, "<CENTER>Domain name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_query("SELECT max(domainid) FROM gw_domains"))<0) return;
		domain.domainid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (domain.domainid<1) domain.domainid=1;
		strcpy(query, "INSERT INTO gw_domains (domainid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, domainname) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", domain.domainid, curdate, curdate, domain.obj_uid, domain.obj_gid, domain.obj_gperm, domain.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain.domainname));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Domain %d added successfully</CENTER><BR>\n", domain.domainid);
		db_log_activity(sid, "domains", domain.domainid, "insert", "%s - %s added domain %d", sid->dat->in_RemoteAddr, sid->dat->user_username, domain.domainid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_queryf("SELECT domainname FROM gw_domains where domainname = '%s' AND domainid <> %d", domain.domainname, domain.domainid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER>Domain %s already exists</CENTER><BR>\n", domain.domainname);
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if (strlen(domain.domainname)<1) {
			prints(sid, "<CENTER>Domain name is too short</CENTER><BR>\n");
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_domains SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, domain.obj_uid, domain.obj_gid, domain.obj_gperm, domain.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "domainname = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain.domainname));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE domainid = %d", domain.domainid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Domain %d modified successfully</CENTER><BR>\n", domain.domainid);
		db_log_activity(sid, "domains", domain.domainid, "modify", "%s - %s modified domain %d", sid->dat->in_RemoteAddr, sid->dat->user_username, domain.domainid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/domainlist\">\n", sid->dat->in_ScriptName);
	return;
}
