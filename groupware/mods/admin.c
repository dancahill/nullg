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

void adminaccess(CONNECTION *sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(file, sizeof(file)-1, "%s/access.log", config.server_etc_dir);
	fixslashes(file);
	prints(sid, "<HR><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			printht(sid, "%s", line);
			flushbuffer(sid);
		}
		fclose(fp);
	}
	prints(sid, "</PRE><HR>\n");
}

void adminerror(CONNECTION *sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(file, sizeof(file)-1, "%s/error.log", config.server_etc_dir);
	fixslashes(file);
	prints(sid, "<HR><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			printht(sid, "%s", line);
			flushbuffer(sid);
		}
		fclose(fp);
	}
	prints(sid, "</PRE><HR>\n");
}

void adminconfigedit(CONNECTION *sid)
{
	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function PortUpdate() {\n");
	prints(sid, "	if (document.configedit.sql_type.value=='MYSQL') {\n");
	prints(sid, "		document.configedit.sql_port.value='3306';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "	if (document.configedit.sql_type.value=='ODBC') {\n");
	prints(sid, "		document.configedit.sql_port.value='0';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "	if (document.configedit.sql_type.value=='PGSQL') {\n");
	prints(sid, "		document.configedit.sql_port.value='5432';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "	if (document.configedit.sql_type.value=='SQLITE') {\n");
	prints(sid, "		document.configedit.sql_port.value='0';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "function SQLTypeUpdate() {\n");
	prints(sid, "	if (document.configedit.sql_type.value=='SQLITE') {\n");
	prints(sid, "		document.configedit.sql_hostname.disabled=true;\n");
	prints(sid, "		document.configedit.sql_port.disabled=true;\n");
	prints(sid, "		document.configedit.sql_dbname.disabled=true;\n");
	prints(sid, "		document.configedit.sql_username.disabled=true;\n");
	prints(sid, "		document.configedit.sql_password.disabled=true;\n");
#ifdef WIN32
	prints(sid, "		document.configedit.sql_odbc_dsn.disabled=true;\n");
	prints(sid, "	} else if (document.configedit.sql_type.value=='ODBC') {\n");
	prints(sid, "		document.configedit.sql_hostname.disabled=true;\n");
	prints(sid, "		document.configedit.sql_port.disabled=true;\n");
	prints(sid, "		document.configedit.sql_dbname.disabled=true;\n");
	prints(sid, "		document.configedit.sql_username.disabled=true;\n");
	prints(sid, "		document.configedit.sql_password.disabled=true;\n");
	prints(sid, "		document.configedit.sql_odbc_dsn.disabled=false;\n");
#endif
	prints(sid, "	} else {\n");
	prints(sid, "		document.configedit.sql_hostname.disabled=false;\n");
	prints(sid, "		document.configedit.sql_port.disabled=false;\n");
	prints(sid, "		document.configedit.sql_dbname.disabled=false;\n");
	prints(sid, "		document.configedit.sql_username.disabled=false;\n");
	prints(sid, "		document.configedit.sql_password.disabled=false;\n");
#ifdef WIN32
	prints(sid, "		document.configedit.sql_odbc_dsn.disabled=true;\n");
#endif
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "function FormUpdate() {\n");
	prints(sid, "	PortUpdate();\n");
	prints(sid, "	SQLTypeUpdate();\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/configsave NAME=configedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4><FONT COLOR=%s>%s</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, ADM_CFG_TITLE);
	prints(sid, "<TR BGCOLOR=%s><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_base_dir   VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVBASE, str2html(sid, config.server_base_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_bin_dir    VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVBIN,  str2html(sid, config.server_bin_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_cgi_dir    VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVCGI,  str2html(sid, config.server_cgi_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_etc_dir    VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVETC,  str2html(sid, config.server_etc_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_file_dir   VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVFILE, str2html(sid, config.server_file_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_htdocs_dir VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVHTTP, str2html(sid, config.server_htdocs_dir));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><SELECT NAME=server_loglevel style='width:120px'>", COLOR_EDITFORM, ADM_CFG_SRVLOG);
	htselect_number(sid, config.server_loglevel, 0, 4);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_hostname   VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVHOST, str2html(sid, config.server_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_port       VALUE=\"%d\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SRVPORT, config.server_port);
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>%s</B>&nbsp;</TD><TD>", COLOR_EDITFORM, ADM_CFG_SQLTYPE);
	prints(sid, "<SELECT NAME=sql_type onchange=FormUpdate(); style='width:120px'>\n");
#ifdef HAVE_MYSQL
	prints(sid, "<OPTION VALUE='MYSQL'%s>MYSQL\n", strcmp(config.sql_type, "MYSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_ODBC
	prints(sid, "<OPTION VALUE='ODBC'%s>ODBC\n", strcmp(config.sql_type, "ODBC")==0?" SELECTED":"");
#endif
#ifdef HAVE_PGSQL
	prints(sid, "<OPTION VALUE='PGSQL'%s>PGSQL\n", strcmp(config.sql_type, "PGSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_SQLITE
	prints(sid, "<OPTION VALUE='SQLITE'%s>SQLITE\n", strcmp(config.sql_type, "SQLITE")==0?" SELECTED":"");
#endif
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_hostname      VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLHOST, str2html(sid, config.sql_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_port          VALUE=\"%d\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLPORT, config.sql_port);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_dbname        VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLNAME, str2html(sid, config.sql_dbname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_username      VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLUSER, str2html(sid, config.sql_username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=PASSWORD NAME=sql_password  VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLPASS, str2html(sid, config.sql_password));
#ifdef WIN32
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_odbc_dsn      VALUE=\"%s\" SIZE=40></TD></TR>\n", COLOR_EDITFORM, ADM_CFG_SQLODBC, str2html(sid, config.sql_odbc_dsn));
#endif
	prints(sid, "</TABLE></TD></TR></TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='%s'>\n", FORM_SAVE);
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='%s'>\n", FORM_RESET);
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.configedit.server_base_dir.focus();\n");
	prints(sid, "SQLTypeUpdate();\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}

void adminconfigsave(CONNECTION *sid)
{
	char *pval;
	FILE *fp;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	/*
	 * Lock the SQL subsystem and shut it down before changing the active
	 * config.  Better safe than sorry.  This whole block of code needs a
	 * rewrite, but I'm lazy.
	 */
	if (!RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	sqlUnsafeDisconnect(sid);
	if ((pval=getpostenv(sid, "SERVER_BASE_DIR"))!=NULL)   strncpy(config.server_base_dir, pval, sizeof(config.server_base_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_BIN_DIR"))!=NULL)    strncpy(config.server_bin_dir,  pval, sizeof(config.server_bin_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_CGI_DIR"))!=NULL)    strncpy(config.server_cgi_dir,  pval, sizeof(config.server_cgi_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_ETC_DIR"))!=NULL)    strncpy(config.server_etc_dir,  pval, sizeof(config.server_etc_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_FILE_DIR"))!=NULL)   strncpy(config.server_file_dir, pval, sizeof(config.server_file_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_HTDOCS_DIR"))!=NULL) strncpy(config.server_htdocs_dir, pval, sizeof(config.server_htdocs_dir)-1);
	if ((pval=getpostenv(sid, "SERVER_LOGLEVEL"))!=NULL)   config.server_loglevel=atoi(pval);
	if ((pval=getpostenv(sid, "SERVER_HOSTNAME"))!=NULL)   strncpy(config.server_hostname, pval, sizeof(config.server_hostname)-1);
	if ((pval=getpostenv(sid, "SERVER_PORT"))!=NULL)       config.server_port=atoi(pval);
	if ((pval=getpostenv(sid, "SQL_TYPE"))!=NULL)          strncpy(config.sql_type, pval, sizeof(config.sql_type)-1);
	if ((pval=getpostenv(sid, "SQL_HOSTNAME"))!=NULL)      strncpy(config.sql_hostname, pval, sizeof(config.sql_hostname)-1);
	if ((pval=getpostenv(sid, "SQL_PORT"))!=NULL)          config.sql_port=atoi(pval);
	if ((pval=getpostenv(sid, "SQL_DBNAME"))!=NULL)        strncpy(config.sql_dbname, pval, sizeof(config.sql_dbname)-1);
	if ((pval=getpostenv(sid, "SQL_USERNAME"))!=NULL)      strncpy(config.sql_username, pval, sizeof(config.sql_username)-1);
	if ((pval=getpostenv(sid, "SQL_PASSWORD"))!=NULL)      strncpy(config.sql_password, pval, sizeof(config.sql_password)-1);
#ifdef WIN32
	if ((pval=getpostenv(sid, "SQL_ODBC_DSN"))!=NULL)      strncpy(config.sql_odbc_dsn, pval, sizeof(config.sql_odbc_dsn)-1);
#endif
	fp=fopen(config.config_filename, "w");
	if (fp==NULL) {
		logerror(sid, __FILE__, __LINE__, ADM_CFG_NOFILE, config.config_filename);
		prints(sid, ADM_CFG_NOFILE, config.config_filename);
		return;
	}
	fprintf(fp, "# %s\n\n", ADM_CFG_HEAD);
	fprintf(fp, "SERVER_BASE_DIR   = \"%s\"\n", config.server_base_dir);
	fprintf(fp, "SERVER_BIN_DIR    = \"%s\"\n", config.server_bin_dir);
	fprintf(fp, "SERVER_CGI_DIR    = \"%s\"\n", config.server_cgi_dir);
	fprintf(fp, "SERVER_ETC_DIR    = \"%s\"\n", config.server_etc_dir);
	fprintf(fp, "SERVER_FILE_DIR   = \"%s\"\n", config.server_file_dir);
	fprintf(fp, "SERVER_HTDOCS_DIR = \"%s\"\n", config.server_htdocs_dir);
	fprintf(fp, "SERVER_LOGLEVEL   = \"%d\"\n", config.server_loglevel);
	fprintf(fp, "SERVER_HOSTNAME   = \"%s\"\n", config.server_hostname);
	fprintf(fp, "SERVER_PORT       = \"%d\"\n", config.server_port);
	fprintf(fp, "SERVER_MAXCONN    = \"%d\"\n", config.server_maxconn);
	fprintf(fp, "SERVER_MAXIDLE    = \"%d\"\n", config.server_maxidle);
	fprintf(fp, "SQL_TYPE          = \"%s\"\n", config.sql_type);
	fprintf(fp, "SQL_HOSTNAME      = \"%s\"\n", config.sql_hostname);
	fprintf(fp, "SQL_PORT          = \"%d\"\n", config.sql_port);
	fprintf(fp, "SQL_DBNAME        = \"%s\"\n", config.sql_dbname);
	fprintf(fp, "SQL_USERNAME      = \"%s\"\n", config.sql_username);
	fprintf(fp, "SQL_PASSWORD      = \"%s\"\n", config.sql_password);
#ifdef WIN32
	fprintf(fp, "SQL_ODBC_DSN      = \"%s\"\n", config.sql_odbc_dsn);
#endif
	fclose(fp);
	prints(sid, "<CENTER>\n<TABLE><TR><TD><PRE>");
	prints(sid, ADM_CFG_SAVED);
	prints(sid, "</PRE></TD><TR></TABLE>\n</CENTER>\n");
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	logaccess(sid, 1, ADM_CFG_LOG, sid->dat->in_RemoteAddr, sid->dat->user_username);
	return;
}

void admingroupedit(CONNECTION *sid)
{
	REC_GROUP group;
	int groupid;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/groupeditnew")==0) {
		groupid=0;
		db_read(sid, 2, DB_GROUPS, 0, &group);
	} else {
		if (getgetenv(sid, "GROUPID")==NULL) return;
		groupid=atoi(getgetenv(sid, "GROUPID"));
		if (db_read(sid, 2, DB_GROUPS, groupid, &group)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", groupid);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/groupsave NAME=groupedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", group.groupid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (groupid!=0) {
		prints(sid, "Group %d</FONT></TH></TR>\n", groupid);
	} else {
		prints(sid, "New Group</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Group Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=groupname   value=\"%s\" SIZE=50></TD></TR>\n", COLOR_EDITFORM, str2html(sid, group.groupname));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>Message of the Day</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=motd ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, group.motd));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ADMIN)&A_ADMIN)&&(groupid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (groupid!=0) {
		prints(sid, "[<A HREF=%s/admin/grouptimeedit?groupid=%d>Edit Availability</A>]\n", sid->dat->in_ScriptName, groupid);
	}
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.groupedit.groupname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void admingrouplist(CONNECTION *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sqlQuery(sid, "SELECT groupid, groupname FROM gw_groups ORDER BY groupid ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=150><FONT COLOR=%s>&nbsp;Group Name&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/groupedit?groupid=%d'\">", COLOR_FVAL, sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no groups<BR>\n");
	}
	sqlFreeconnect(sqr);
	prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void admingroupsave(CONNECTION *sid)
{
	REC_GROUP group;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int groupid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	if (db_read(sid, 2, DB_GROUPS, groupid, &group)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "GROUPNAME"))!=NULL) snprintf(group.groupname, sizeof(group.groupname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MOTD"))!=NULL) snprintf(group.motd, sizeof(group.motd)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MEMBERS"))!=NULL) snprintf(group.members, sizeof(group.members)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_groups WHERE groupid = %d", group.groupid)<0) return;
		prints(sid, "<CENTER>Group %d deleted successfully</CENTER><BR>\n", group.groupid);
		logaccess(sid, 1, "%s - %s deleted group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else if (group.groupid==0) {
		if ((sqr=sqlQueryf(sid, "SELECT groupname FROM gw_groups where groupname = '%s'", group.groupname))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<CENTER>Group %s already exists</CENTER><BR>\n", group.groupname);
			sqlFreeconnect(sqr);
			return;
		}
		sqlFreeconnect(sqr);
		if (strlen(group.groupname)<1) {
			prints(sid, "<CENTER>Group name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sqlQuery(sid, "SELECT max(groupid) FROM gw_groups"))<0) return;
		group.groupid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (group.groupid<1) group.groupid=1;
		strcpy(query, "INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, groupname, availability, motd, members) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", group.groupid, curdate, curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strcatf(query, "'%s', ", str2sql(sid, group.groupname));
		strcatf(query, "'%s', ", str2sql(sid, group.availability));
		strcatf(query, "'%s', ", str2sql(sid, group.motd));
		strcatf(query, "'%s')", str2sql(sid, group.members));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Group %d added successfully</CENTER><BR>\n", group.groupid);
		logaccess(sid, 1, "%s - %s added group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_groups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strcatf(query, "groupname = '%s', ", str2sql(sid, group.groupname));
		strcatf(query, "motd = '%s', ", str2sql(sid, group.motd));
		strcatf(query, "members = '%s'", str2sql(sid, group.members));
		strcatf(query, " WHERE groupid = %d", group.groupid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Group %d modified successfully</CENTER><BR>\n", group.groupid);
		logaccess(sid, 1, "%s - %s modified group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/grouplist\">\n", sid->dat->in_ScriptName);
	return;
}

void admingrouptimeedit(CONNECTION *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char availability[170];
	REC_GROUP group;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "GROUPID")==NULL) return;
	groupid=atoi(getgetenv(sid, "GROUPID"));
	if (db_read(sid, 2, DB_GROUPS, groupid, &group)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", groupid);
		return;
	}
	memset(availability, 0, sizeof(availability));
	strncpy(availability, group.availability, sizeof(availability)-1);
	if (strlen(availability)!=168) {
		for (i=0;i<168;i++) {
			availability[i]='0';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/grouptimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=25><FONT COLOR=%s>Group availability for <A HREF=%s/admin/groupedit?groupid=%d STYLE='color: %s'>%s</A></FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, sid->dat->in_ScriptName, groupid, COLOR_THLINK, group.groupname);
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", COLOR_FNAME, i, dow[i]);
		for (j=0;j<24;j++) {
			prints(sid, "<TD><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, availability[i*24+j]=='1'?"checked":"");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void admingrouptimesave(CONNECTION *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sqlUpdatef(sid, "UPDATE gw_groups SET obj_mtime = '%s', availability = '%s' WHERE groupid = %d", curdate, availability, groupid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	logaccess(sid, 1, "%s - %s modified availability for group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/groupedit?groupid=%d\");\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/groupedit?groupid=%d\">\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void adminuseredit(CONNECTION *sid, REC_USER *user)
{
	REC_USER userrec;
	int userid;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (user==NULL) {
		if (strcmp(sid->dat->in_RequestURI, "/admin/usereditnew")==0) {
			userid=0;
			db_read(sid, 2, DB_USERS, 0, &userrec);
		} else {
			if (getgetenv(sid, "USERID")==NULL) return;
			userid=atoi(getgetenv(sid, "USERID"));
			if (db_read(sid, 2, DB_USERS, userid, &userrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
				return;
			}
		}
		user=&userrec;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usersave NAME=useredit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user->userid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>ACCOUNT INFORMATION - ", COLOR_TH, COLOR_THTEXT);
	if (user->userid!=0) {
		prints(sid, "User %d</FONT></TH></TR>\n", user->userid);
	} else {
		prints(sid, "New User</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=left VALIGN=top>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Username&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->password));
	prints(sid, "</TABLE>\n</TD><TD ALIGN=center VALIGN=top><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:182px'>\n", COLOR_EDITFORM);
	htselect_group(sid, user->groupid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Allow Login&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:182px'%s>\n", COLOR_EDITFORM, user->userid==1?" DISABLED":"");
	if ((user->userid==1)||(user->enabled)) {
		prints(sid, "<OPTION VALUE=0>No\n<OPTION VALUE=1 SELECTED>Yes\n");
	} else {
		prints(sid, "<OPTION VALUE=0 SELECTED>No\n<OPTION VALUE=1>Yes\n");
	}
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PERMISSIONS</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=left VALIGN=top>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%>&nbsp;</TD><TD ALIGN=CENTER><B>Read&nbsp;</B></TD><TD ALIGN=CENTER><B>Modify&nbsp;</B></TD><TD ALIGN=CENTER><B>Insert&nbsp;</B></TD><TD ALIGN=CENTER><B>Delete&nbsp;</B></TD><TD ALIGN=CENTER><B>Admin</B></TD></TR>", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Administration&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	if (user->userid==1) {
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authadmin_a VALUE='1' CHECKED DISABLED></TD>");
	} else {
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authadmin_a VALUE='1' %s></TD>", (user->authadmin&A_ADMIN)?"checked":"");
	}
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Bookmarks&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_r VALUE='1' %s></TD>", (user->authbookmarks&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_m VALUE='1' %s></TD>", (user->authbookmarks&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_i VALUE='1' %s></TD>", (user->authbookmarks&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_d VALUE='1' %s></TD>", (user->authbookmarks&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_a VALUE='1' %s></TD>", (user->authbookmarks&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Calendar&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_r VALUE='1' %s></TD>", (user->authcalendar&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_m VALUE='1' %s></TD>", (user->authcalendar&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_i VALUE='1' %s></TD>", (user->authcalendar&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_d VALUE='1' %s></TD>", (user->authcalendar&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_a VALUE='1' %s></TD>", (user->authcalendar&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Calls&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_r VALUE='1' %s></TD>", (user->authcalls&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_m VALUE='1' %s></TD>", (user->authcalls&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_i VALUE='1' %s></TD>", (user->authcalls&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_d VALUE='1' %s></TD>", (user->authcalls&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_a VALUE='1' %s></TD>", (user->authcalls&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Contacts&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_r VALUE='1' %s></TD>", (user->authcontacts&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_m VALUE='1' %s></TD>", (user->authcontacts&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_i VALUE='1' %s></TD>", (user->authcontacts&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_d VALUE='1' %s></TD>", (user->authcontacts&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_a VALUE='1' %s></TD>", (user->authcontacts&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;E-Mail&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_r VALUE='1' %s></TD>", (user->authwebmail&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_m VALUE='1' %s></TD>", (user->authwebmail&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_i VALUE='1' %s></TD>", (user->authwebmail&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_d VALUE='1' %s></TD>", (user->authwebmail&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "</TABLE>\n</TD><TD ALIGN=center VALIGN=top><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%>&nbsp;</TD><TD ALIGN=CENTER><B>Read&nbsp;</B></TD><TD ALIGN=CENTER><B>Modify&nbsp;</B></TD><TD ALIGN=CENTER><B>Insert&nbsp;</B></TD><TD ALIGN=CENTER><B>Delete&nbsp;</B></TD><TD ALIGN=CENTER><B>Admin</B></TD></TR>", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Files&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_r VALUE='1' %s></TD>", (user->authfiles&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_m VALUE='1' %s></TD>", (user->authfiles&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_i VALUE='1' %s></TD>", (user->authfiles&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_d VALUE='1' %s></TD>", (user->authfiles&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_a VALUE='1' %s></TD>", (user->authfiles&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Forums&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_r VALUE='1' %s></TD>", (user->authforums&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_i VALUE='1' %s></TD>", (user->authforums&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_a VALUE='1' %s></TD>", (user->authforums&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Messages&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_r VALUE='1' %s></TD>", (user->authmessages&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_i VALUE='1' %s></TD>", (user->authmessages&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_d VALUE='1' %s></TD>", (user->authmessages&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Orders&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_r VALUE='1' %s></TD>", (user->authorders&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_m VALUE='1' %s></TD>", (user->authorders&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_i VALUE='1' %s></TD>", (user->authorders&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_d VALUE='1' %s></TD>", (user->authorders&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_a VALUE='1' %s></TD>", (user->authorders&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Profile&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authprofile_r VALUE='1' %s></TD>", (user->authprofile&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authprofile_m VALUE='1' %s></TD>", (user->authprofile&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;SQL Queries&nbsp;</B></TD>", COLOR_EDITFORM);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authquery_r VALUE='1' %s></TD>", (user->authquery&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authquery_a VALUE='1' %s></TD>", (user->authquery&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PREFERENCE INFORMATION</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaystart style='width:182px'>\n", COLOR_EDITFORM);
	htselect_hour(sid, user->prefdaystart);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Length&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaylength style='width:182px'>\n", COLOR_EDITFORM);
	htselect_number(sid, user->prefdaylength, 0, 24);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Max Results/Page&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefmaxlist style='width:182px'>\n", COLOR_EDITFORM);
	htselect_number(sid, user->prefmaxlist, 5, 50);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Time Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preftimezone style='width:182px'>\n", COLOR_EDITFORM);
	htselect_timezone(sid, user->preftimezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefgeozone style='width:182px'>\n", COLOR_EDITFORM);
	htselect_zone(sid, user->prefgeozone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PERSONAL INFORMATION</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Surname&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->surname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Given Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->givenname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->jobtitle));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Division&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=division      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->division));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Supervisor&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=supervisor  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->supervisor));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->address));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;City&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=locality          value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->locality));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Province&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=region        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->region));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Country&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=country        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->country));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=postalcode value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->postalcode));
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->homenumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->worknumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;FAX&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber          value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->faxnumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Cell Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->cellnumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Pager&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->pagernumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;E-Mail&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email           value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->email));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Date of Birth&nbsp;</B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=birthdate2>\n");
	htselect_month(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate1>\n");
	htselect_day(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate3>\n");
	htselect_year(sid, 1900, user->birthdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Hire Date&nbsp;</B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=hiredate2>\n");
	htselect_month(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate1>\n");
	htselect_day(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate3>\n");
	htselect_year(sid, 1900, user->hiredate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;S.I.N.&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sin         value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->sin));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Active&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=isactive    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user->isactive));
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Notes", COLOR_TH, COLOR_THTEXT);
	prints(sid, " [<A HREF=%s/notes/editnew?table=users&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, user->userid, COLOR_THLINK);
	prints(sid, "</FONT></TH></TR>\n");
	notessublist(sid, "users", user->userid, 2);
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ADMIN)&A_ADMIN)&&(user->userid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (user->userid!=0) {
		prints(sid, "[<A HREF=%s/admin/usertimeedit?userid=%d>Edit Availability</A>]\n", sid->dat->in_ScriptName, user->userid);
	}
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.useredit.username.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminuserlist(CONNECTION *sid)
{
	int i, j;
	int sqr1;
	int sqr2;
	int sqr3;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr1=sqlQuery(sid, "SELECT userid, username, surname, givenname, groupid, prefgeozone FROM gw_users ORDER BY userid ASC"))<0) return;
	if ((sqr2=sqlQuery(sid, "SELECT groupid, groupname FROM gw_groups ORDER BY groupid ASC"))<0) {
		sqlFreeconnect(sqr1);
		return;
	}
	if ((sqr3=sqlQuery(sid, "SELECT zoneid, zonename FROM gw_zones ORDER BY zoneid ASC"))<0) {
		sqlFreeconnect(sqr1);
		sqlFreeconnect(sqr2);
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TH);
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;User Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Real Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Group&nbsp;</FONT></TH>", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
	if (sqlNumtuples(sqr3)>0) {
		prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Zone&nbsp;</FONT></TH>", COLOR_THTEXT);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<sqlNumtuples(sqr1);i++) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", COLOR_FVAL, sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr1, i, 1)));
		prints(sid, "<TD NOWRAP>%s", str2html(sid, sqlGetvalue(sqr1, i, 2)));
		if (strlen(sqlGetvalue(sqr1, i, 2))&&strlen(sqlGetvalue(sqr1, i, 3))) prints(sid, ", ");
		prints(sid, "%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr1, i, 3)));
		for (j=0;j<sqlNumtuples(sqr2);j++) {
			if (atoi(sqlGetvalue(sqr2, j, 0))==atoi(sqlGetvalue(sqr1, i, 4))) {
				prints(sid, "<TD NOWRAP>%s</TD>", str2html(sid, sqlGetvalue(sqr2, j, 1)));
				break;
			}
		}
		if (j==sqlNumtuples(sqr2)) {
			prints(sid, "<TD NOWRAP>&nbsp;</TD>");
		}
		if (sqlNumtuples(sqr3)>0) {
			for (j=0;j<sqlNumtuples(sqr3);j++) {
				if (atoi(sqlGetvalue(sqr3, j, 0))==atoi(sqlGetvalue(sqr1, i, 5))) {
					prints(sid, "<TD NOWRAP>%s</TD>", str2html(sid, sqlGetvalue(sqr3, j, 1)));
					break;
				}
			}
			if (j==sqlNumtuples(sqr3)) {
				prints(sid, "<TD NOWRAP>&nbsp;</TD>");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	sqlFreeconnect(sqr1);
	sqlFreeconnect(sqr2);
	sqlFreeconnect(sqr3);
	prints(sid, "<A HREF=%s/admin/usereditnew>New User</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void adminusersave(CONNECTION *sid)
{
	REC_USER user;
	char query[4096];
	char curdate[40];
	char opassword[50];
	char *ptemp;
	time_t t;
	int userid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (db_read(sid, 2, DB_USERS, userid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", user.password);
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL) snprintf(user.username, sizeof(user.username)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "GROUPID"))!=NULL) user.groupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ENABLED"))!=NULL) user.enabled=atoi(ptemp);
	user.authadmin=0;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_R"))!=NULL) user.authadmin+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_M"))!=NULL) user.authadmin+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_I"))!=NULL) user.authadmin+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_D"))!=NULL) user.authadmin+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_A"))!=NULL) user.authadmin+=A_ADMIN;
	user.authbookmarks=0;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_R"))!=NULL) user.authbookmarks+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_M"))!=NULL) user.authbookmarks+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_I"))!=NULL) user.authbookmarks+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_D"))!=NULL) user.authbookmarks+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_A"))!=NULL) user.authbookmarks+=A_ADMIN;
	user.authcalendar=0;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_R"))!=NULL) user.authcalendar+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_M"))!=NULL) user.authcalendar+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_I"))!=NULL) user.authcalendar+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_D"))!=NULL) user.authcalendar+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_A"))!=NULL) user.authcalendar+=A_ADMIN;
	user.authcalls=0;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_R"))!=NULL) user.authcalls+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_M"))!=NULL) user.authcalls+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_I"))!=NULL) user.authcalls+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_D"))!=NULL) user.authcalls+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_A"))!=NULL) user.authcalls+=A_ADMIN;
	user.authcontacts=0;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_R"))!=NULL) user.authcontacts+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_M"))!=NULL) user.authcontacts+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_I"))!=NULL) user.authcontacts+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_D"))!=NULL) user.authcontacts+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_A"))!=NULL) user.authcontacts+=A_ADMIN;
	user.authfiles=0;
	if ((ptemp=getpostenv(sid, "AUTHFILES_R"))!=NULL) user.authfiles+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHFILES_M"))!=NULL) user.authfiles+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHFILES_I"))!=NULL) user.authfiles+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHFILES_D"))!=NULL) user.authfiles+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHFILES_A"))!=NULL) user.authfiles+=A_ADMIN;
	user.authforums=0;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_R"))!=NULL) user.authforums+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_M"))!=NULL) user.authforums+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_I"))!=NULL) user.authforums+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_D"))!=NULL) user.authforums+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_A"))!=NULL) user.authforums+=A_ADMIN;
	user.authmessages=0;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_R"))!=NULL) user.authmessages+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_M"))!=NULL) user.authmessages+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_I"))!=NULL) user.authmessages+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_D"))!=NULL) user.authmessages+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_A"))!=NULL) user.authmessages+=A_ADMIN;
	user.authorders=0;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_R"))!=NULL) user.authorders+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_M"))!=NULL) user.authorders+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_I"))!=NULL) user.authorders+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_D"))!=NULL) user.authorders+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_A"))!=NULL) user.authorders+=A_ADMIN;
	user.authprofile=0;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_R"))!=NULL) user.authprofile+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_M"))!=NULL) user.authprofile+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_I"))!=NULL) user.authprofile+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_D"))!=NULL) user.authprofile+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_A"))!=NULL) user.authprofile+=A_ADMIN;
	user.authquery=0;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_R"))!=NULL) user.authquery+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_M"))!=NULL) user.authquery+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_I"))!=NULL) user.authquery+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_D"))!=NULL) user.authquery+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_A"))!=NULL) user.authquery+=A_ADMIN;
	user.authwebmail=0;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_R"))!=NULL) user.authwebmail+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_M"))!=NULL) user.authwebmail+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_I"))!=NULL) user.authwebmail+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_D"))!=NULL) user.authwebmail+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_A"))!=NULL) user.authwebmail+=A_ADMIN;
	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL) user.prefdaystart=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL) user.prefdaylength=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL) user.prefmaxlist=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL) user.preftimezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL) user.prefgeozone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SURNAME"))!=NULL) snprintf(user.surname, sizeof(user.surname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "GIVENNAME"))!=NULL) snprintf(user.givenname, sizeof(user.givenname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "JOBTITLE"))!=NULL) snprintf(user.jobtitle, sizeof(user.jobtitle)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DIVISION"))!=NULL) snprintf(user.division, sizeof(user.division)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SUPERVISOR"))!=NULL) snprintf(user.supervisor, sizeof(user.supervisor)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ADDRESS"))!=NULL) snprintf(user.address, sizeof(user.address)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "LOCALITY"))!=NULL) snprintf(user.locality, sizeof(user.locality)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REGION"))!=NULL) snprintf(user.region, sizeof(user.region)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "COUNTRY"))!=NULL) snprintf(user.country, sizeof(user.country)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POSTALCODE"))!=NULL) snprintf(user.postalcode, sizeof(user.postalcode)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMENUMBER"))!=NULL) snprintf(user.homenumber, sizeof(user.homenumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKNUMBER"))!=NULL) snprintf(user.worknumber, sizeof(user.worknumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "FAXNUMBER"))!=NULL) snprintf(user.faxnumber, sizeof(user.faxnumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "CELLNUMBER"))!=NULL) snprintf(user.cellnumber, sizeof(user.cellnumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAGERNUMBER"))!=NULL) snprintf(user.pagernumber, sizeof(user.pagernumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "EMAIL"))!=NULL) snprintf(user.email, sizeof(user.email)-1, "%s", ptemp);
	memset(user.birthdate, 0, sizeof(user.birthdate));
	if ((ptemp=getpostenv(sid, "BIRTHDATE3"))!=NULL) snprintf(user.birthdate, sizeof(user.birthdate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "BIRTHDATE2"))!=NULL) strcatf(user.birthdate, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "BIRTHDATE1"))!=NULL) strcatf(user.birthdate, "%02d", atoi(ptemp));
	if (strlen(user.birthdate)!=10) memset(user.birthdate, 0, sizeof(user.birthdate));
	memset(user.hiredate, 0, sizeof(user.hiredate));
	if ((ptemp=getpostenv(sid, "HIREDATE3"))!=NULL) snprintf(user.hiredate, sizeof(user.hiredate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "HIREDATE2"))!=NULL) strcatf(user.hiredate, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "HIREDATE1"))!=NULL) strcatf(user.hiredate, "%02d", atoi(ptemp));
	if (strlen(user.hiredate)!=10) memset(user.hiredate, 0, sizeof(user.hiredate));
	if ((ptemp=getpostenv(sid, "SIN"))!=NULL) snprintf(user.sin, sizeof(user.sin)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ISACTIVE"))!=NULL) snprintf(user.isactive, sizeof(user.isactive)-1, "%s", ptemp);
	if (user.userid==1) {
		user.enabled=1;
		user.authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	}
	if (strlen(user.birthdate)==0) snprintf(user.birthdate, sizeof(user.birthdate)-1, "1900-01-01");
	if (strlen(user.hiredate)==0) snprintf(user.hiredate, sizeof(user.hiredate)-1, "1900-01-01");
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)||(userid<2)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_users WHERE userid = %d", user.userid)<0) return;
		prints(sid, "<CENTER>User %d deleted successfully</CENTER><BR>\n", userid);
		logaccess(sid, 1, "%s - %s deleted user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else if (user.userid==0) {
		if ((sqr=sqlQueryf(sid, "SELECT username FROM gw_users where username = '%s'", user.username))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\n", user.username);
			sqlFreeconnect(sqr);
			adminuseredit(sid, &user);
			return;
		}
		sqlFreeconnect(sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\n");
			adminuseredit(sid, &user);
			return;
		}
		snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
		if ((sqr=sqlQuery(sid, "SELECT max(userid) FROM gw_users"))<0) return;
		user.userid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (user.userid<1) user.userid=1;
		strcpy(query, "INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, loginip, logintime, logintoken, username, password, groupid, enabled, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authfiles, authforums, authmessages, authorders, authprofile, authquery, authwebmail, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", user.userid, curdate, curdate, user.obj_uid, user.obj_gid, user.obj_gperm, user.obj_operm);
		strcatf(query, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
		strcatf(query, "'%s', ", str2sql(sid, user.username));
		strcatf(query, "'%s', ", user.password);
		strcatf(query, "'%d', ", user.groupid);
		strcatf(query, "'%d', ", user.enabled);
		strcatf(query, "'%d', ", user.authadmin);
		strcatf(query, "'%d', ", user.authbookmarks);
		strcatf(query, "'%d', ", user.authcalendar);
		strcatf(query, "'%d', ", user.authcalls);
		strcatf(query, "'%d', ", user.authcontacts);
		strcatf(query, "'%d', ", user.authfiles);
		strcatf(query, "'%d', ", user.authforums);
		strcatf(query, "'%d', ", user.authmessages);
		strcatf(query, "'%d', ", user.authorders);
		strcatf(query, "'%d', ", user.authprofile);
		strcatf(query, "'%d', ", user.authquery);
		strcatf(query, "'%d', ", user.authwebmail);
		strcatf(query, "'%d', ", user.prefdaystart);
		strcatf(query, "'%d', ", user.prefdaylength);
		strcatf(query, "'%d', ", user.prefmailcurrent);
		strcatf(query, "'%d', ", user.prefmaildefault);
		strcatf(query, "'%d', ", user.prefmaxlist);
		strcatf(query, "'%d', ", user.prefmenustyle);
		strcatf(query, "'%d', ", user.preftimezone);
		strcatf(query, "'%d', ", user.prefgeozone);
		strcatf(query, "'%s', ", user.availability);
		strcatf(query, "'%s', ", str2sql(sid, user.surname));
		strcatf(query, "'%s', ", str2sql(sid, user.givenname));
		strcatf(query, "'%s', ", str2sql(sid, user.jobtitle));
		strcatf(query, "'%s', ", str2sql(sid, user.division));
		strcatf(query, "'%s', ", str2sql(sid, user.supervisor));
		strcatf(query, "'%s', ", str2sql(sid, user.address));
		strcatf(query, "'%s', ", str2sql(sid, user.locality));
		strcatf(query, "'%s', ", str2sql(sid, user.region));
		strcatf(query, "'%s', ", str2sql(sid, user.country));
		strcatf(query, "'%s', ", str2sql(sid, user.postalcode));
		strcatf(query, "'%s', ", str2sql(sid, user.homenumber));
		strcatf(query, "'%s', ", str2sql(sid, user.worknumber));
		strcatf(query, "'%s', ", str2sql(sid, user.faxnumber));
		strcatf(query, "'%s', ", str2sql(sid, user.cellnumber));
		strcatf(query, "'%s', ", str2sql(sid, user.pagernumber));
		strcatf(query, "'%s', ", str2sql(sid, user.email));
		strcatf(query, "'%s', ", user.birthdate);
		strcatf(query, "'%s', ", user.hiredate);
		strcatf(query, "'%s', ", str2sql(sid, user.sin));
		strcatf(query, "'%s')", str2sql(sid, user.isactive));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>User added successfully</CENTER><BR>\n");
		logaccess(sid, 1, "%s - %s added user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sqlQueryf(sid, "SELECT username FROM gw_users where username = '%s' and userid <> %d", user.username, user.userid))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\n", user.username);
			sqlFreeconnect(sqr);
			adminuseredit(sid, &user);
			return;
		}
		sqlFreeconnect(sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\n");
			adminuseredit(sid, &user);
			return;
		}
		if (strcmp(opassword, user.password)!=0) {
			snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_users SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, user.obj_uid, user.obj_gid, user.obj_gperm, user.obj_operm);
		strcatf(query, "username = '%s', ", str2sql(sid, user.username));
		strcatf(query, "password = '%s', ", user.password);
		strcatf(query, "groupid = '%d', ", user.groupid);
		strcatf(query, "enabled = '%d', ", user.enabled);
		strcatf(query, "authadmin = '%d', ", user.authadmin);
		strcatf(query, "authbookmarks = '%d', ", user.authbookmarks);
		strcatf(query, "authcalendar = '%d', ", user.authcalendar);
		strcatf(query, "authcalls = '%d', ", user.authcalls);
		strcatf(query, "authcontacts = '%d', ", user.authcontacts);
		strcatf(query, "authfiles = '%d', ", user.authfiles);
		strcatf(query, "authforums = '%d', ", user.authforums);
		strcatf(query, "authmessages = '%d', ", user.authmessages);
		strcatf(query, "authorders = '%d', ", user.authorders);
		strcatf(query, "authprofile = '%d', ", user.authprofile);
		strcatf(query, "authquery = '%d', ", user.authquery);
		strcatf(query, "authwebmail = '%d', ", user.authwebmail);
		strcatf(query, "prefdaystart = '%d', ", user.prefdaystart);
		strcatf(query, "prefdaylength = '%d', ", user.prefdaylength);
		strcatf(query, "prefmailcurrent = '%d', ", user.prefmailcurrent);
		strcatf(query, "prefmaildefault = '%d', ", user.prefmaildefault);
		strcatf(query, "prefmaxlist = '%d', ", user.prefmaxlist);
		strcatf(query, "prefmenustyle = '%d', ", user.prefmenustyle);
		strcatf(query, "preftimezone = '%d', ", user.preftimezone);
		strcatf(query, "prefgeozone = '%d', ", user.prefgeozone);
		strcatf(query, "availability = '%s', ", user.availability);
		strcatf(query, "surname = '%s', ", str2sql(sid, user.surname));
		strcatf(query, "givenname = '%s', ", str2sql(sid, user.givenname));
		strcatf(query, "jobtitle = '%s', ", str2sql(sid, user.jobtitle));
		strcatf(query, "division = '%s', ", str2sql(sid, user.division));
		strcatf(query, "supervisor = '%s', ", str2sql(sid, user.supervisor));
		strcatf(query, "address = '%s', ", str2sql(sid, user.address));
		strcatf(query, "locality = '%s', ", str2sql(sid, user.locality));
		strcatf(query, "region = '%s', ", str2sql(sid, user.region));
		strcatf(query, "country = '%s', ", str2sql(sid, user.country));
		strcatf(query, "postalcode = '%s', ", str2sql(sid, user.postalcode));
		strcatf(query, "homenumber = '%s', ", str2sql(sid, user.homenumber));
		strcatf(query, "worknumber = '%s', ", str2sql(sid, user.worknumber));
		strcatf(query, "faxnumber = '%s', ", str2sql(sid, user.faxnumber));
		strcatf(query, "cellnumber = '%s', ", str2sql(sid, user.cellnumber));
		strcatf(query, "pagernumber = '%s', ", str2sql(sid, user.pagernumber));
		strcatf(query, "email = '%s', ", str2sql(sid, user.email));
		strcatf(query, "birthdate = '%s', ", user.birthdate);
		strcatf(query, "hiredate = '%s', ", user.hiredate);
		strcatf(query, "sin = '%s', ", str2sql(sid, user.sin));
		strcatf(query, "isactive = '%s'", str2sql(sid, user.isactive));
		strcatf(query, " WHERE userid = %d", user.userid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>User %d modified successfully</CENTER><BR>\n", userid);
		logaccess(sid, 1, "%s - %s modified user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, userid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	return;
}

void adminusertimeedit(CONNECTION *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	REC_USER user;
	int userid;
	int i;
	int j;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "USERID")==NULL) return;
	userid=atoi(getgetenv(sid, "USERID"));
	if (db_read(sid, 2, DB_USERS, userid, &user)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
		return;
	}
	if ((sqr=sqlQueryf(sid, "SELECT availability FROM gw_users WHERE userid = %d", user.userid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\n", user.userid);
		sqlFreeconnect(sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sqlGetvalue(sqr, 0, 0), sizeof(uavailability)-1);
	sqlFreeconnect(sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if ((sqr=sqlQueryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", user.groupid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\n", user.groupid);
		sqlFreeconnect(sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sqlGetvalue(sqr, 0, 0), sizeof(gavailability)-1);
	sqlFreeconnect(sqr);
	if (strlen(gavailability)!=168) {
		for (i=0;i<168;i++) {
			gavailability[i]='0';
		}
	}
	for (i=0;i<168;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='X';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usertimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user.userid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=25><FONT COLOR=%s>Availability for <A HREF=%s/admin/useredit?userid=%d STYLE='color: %s'>%s</A></FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, sid->dat->in_ScriptName, userid, COLOR_THLINK, user.username);
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FVAL);
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", COLOR_FNAME, i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
			} else {
				prints(sid, "<TD><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void adminusertimesave(CONNECTION *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int userid;
	int i;
	int j;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sqlUpdatef(sid, "UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, userid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	logaccess(sid, 1, "%s - %s modified availability for user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, userid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/useredit?userid=%d\");\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/useredit?userid=%d\">\n", sid->dat->in_ScriptName, userid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void adminzoneedit(CONNECTION *sid)
{
	REC_ZONE zone;
	int zoneid;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/zoneeditnew")==0) {
		zoneid=0;
		db_read(sid, 2, DB_ZONES, 0, &zone);
	} else {
		if (getgetenv(sid, "ZONEID")==NULL) return;
		zoneid=atoi(getgetenv(sid, "ZONEID"));
		if (db_read(sid, 2, DB_ZONES, zoneid, &zone)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", zoneid);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/zonesave NAME=zoneedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=zoneid VALUE='%d'>\n", zone.zoneid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (zoneid!=0) {
		prints(sid, "zone %d</FONT></TH></TR>\n", zoneid);
	} else {
		prints(sid, "New Zone</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Zone Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=zonename   value=\"%s\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, str2html(sid, zone.zonename));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ADMIN)&A_ADMIN)&&(zoneid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.zoneedit.zonename.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminzonelist(CONNECTION *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sqlQuery(sid, "SELECT zoneid, zonename FROM gw_zones ORDER BY zonename ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=150><FONT COLOR=%s>&nbsp;Zone Name&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/zoneedit?zoneid=%d'\">", COLOR_FVAL, sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/zoneedit?zoneid=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no zones<BR>\n");
	}
	sqlFreeconnect(sqr);
	prints(sid, "<A HREF=%s/admin/zoneeditnew>New Zone</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void adminzonesave(CONNECTION *sid)
{
	REC_ZONE zone;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int zoneid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ZONEID"))==NULL) return;
	zoneid=atoi(ptemp);
	if (db_read(sid, 2, DB_ZONES, zoneid, &zone)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "ZONENAME"))!=NULL) snprintf(zone.zonename, sizeof(zone.zonename)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_zones WHERE zoneid = %d", zone.zoneid)<0) return;
		prints(sid, "<CENTER>Zone %d deleted successfully</CENTER><BR>\n", zone.zoneid);
		logaccess(sid, 1, "%s - %s deleted zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else if (zone.zoneid==0) {
		if (strlen(zone.zonename)<1) {
			prints(sid, "<CENTER>Zone name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sqlQuery(sid, "SELECT max(zoneid) FROM gw_zones"))<0) return;
		zone.zoneid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (zone.zoneid<1) zone.zoneid=1;
		strcpy(query, "INSERT INTO gw_zones (zoneid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, zonename) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", zone.zoneid, curdate, curdate, zone.obj_uid, zone.obj_gid, zone.obj_gperm, zone.obj_operm);
		strcatf(query, "'%s')", str2sql(sid, zone.zonename));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Zone %d added successfully</CENTER><BR>\n", zone.zoneid);
		logaccess(sid, 1, "%s - %s added zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_zones SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, zone.obj_uid, zone.obj_gid, zone.obj_gperm, zone.obj_operm);
		strcatf(query, "zonename = '%s'", str2sql(sid, zone.zonename));
		strcatf(query, " WHERE zoneid = %d", zone.zoneid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Zone %d modified successfully</CENTER><BR>\n", zone.zoneid);
		logaccess(sid, 1, "%s - %s modified zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/zonelist\">\n", sid->dat->in_ScriptName);
	return;
}

void adminmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ADMIN);
	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/admin/access", 13)==0)
		adminaccess(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/error", 12)==0)
		adminerror(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/configedit", 17)==0)
		adminconfigedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/configsave", 17)==0)
		adminconfigsave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupedit", 16)==0)
		admingroupedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouplist", 16)==0)
		admingrouplist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupsave", 16)==0)
		admingroupsave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouptimeedit", 20)==0)
		admingrouptimeedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouptimesave", 20)==0)
		admingrouptimesave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/useredit", 15)==0)
		adminuseredit(sid, NULL);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/userlist", 15)==0)
		adminuserlist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usersave", 15)==0)
		adminusersave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usertimeedit", 19)==0)
		adminusertimeedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usertimesave", 19)==0)
		adminusertimesave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zoneedit", 15)==0)
		adminzoneedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zonelist", 15)==0)
		adminzonelist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zonesave", 15)==0)
		adminzonesave(sid);
	htpage_footer(sid);
	return;
}
