/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "mod_substub.h"
#include "mod_admin.h"

void adminconfigedit(CONN *sid)
{
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4><FONT COLOR=%s>%s</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, ADM_CFG_TITLE);
	prints(sid, "<TR BGCOLOR=%s><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_base       VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBASE, str2html(sid, config->server_dir_base));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_bin        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBIN,  str2html(sid, config->server_dir_bin));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_cgi        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVCGI,  str2html(sid, config->server_dir_cgi));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_etc        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVETC,  str2html(sid, config->server_dir_etc));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_lib        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVLIB,  str2html(sid, config->server_dir_lib));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVVAR,  str2html(sid, config->server_dir_var));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_backup VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBAK,  str2html(sid, config->server_dir_var_backup));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_db     VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVDB,   str2html(sid, config->server_dir_var_db));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_files  VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVFILE, str2html(sid, config->server_dir_var_files));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_htdocs VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVHDOC, str2html(sid, config->server_dir_var_htdocs));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_log    VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVLOG,  str2html(sid, config->server_dir_var_log));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_dir_var_mail   VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVMAIL, str2html(sid, config->server_dir_var_mail));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><SELECT NAME=server_loglevel style='width:120px'>", config->colour_editform, ADM_CFG_SRVLOG);
	htselect_number(sid, config->server_loglevel, 0, 4);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_hostname   VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVHOST, str2html(sid, config->server_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=server_port       VALUE=\"%d\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SRVPORT, config->server_port);
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>%s</B>&nbsp;</TD><TD>", config->colour_editform, ADM_CFG_SQLTYPE);
	prints(sid, "<SELECT NAME=sql_type onchange=FormUpdate(); style='width:120px'>\n");
#ifdef HAVE_MYSQL
	prints(sid, "<OPTION VALUE='MYSQL'%s>MYSQL\n", strcmp(config->sql_type, "MYSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_ODBC
	prints(sid, "<OPTION VALUE='ODBC'%s>ODBC\n", strcmp(config->sql_type, "ODBC")==0?" SELECTED":"");
#endif
#ifdef HAVE_PGSQL
	prints(sid, "<OPTION VALUE='PGSQL'%s>PGSQL\n", strcmp(config->sql_type, "PGSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_SQLITE
	prints(sid, "<OPTION VALUE='SQLITE'%s>SQLITE\n", strcmp(config->sql_type, "SQLITE")==0?" SELECTED":"");
#endif
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_hostname      VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLHOST, str2html(sid, config->sql_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_port          VALUE=\"%d\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLPORT, config->sql_port);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_dbname        VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLNAME, str2html(sid, config->sql_dbname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_username      VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLUSER, str2html(sid, config->sql_username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=PASSWORD NAME=sql_password  VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLPASS, str2html(sid, config->sql_password));
#ifdef WIN32
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=sql_odbc_dsn      VALUE=\"%s\" SIZE=45></TD></TR>\n", config->colour_editform, ADM_CFG_SQLODBC, str2html(sid, config->sql_odbc_dsn));
#endif
	prints(sid, "</TABLE></TD></TR></TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='%s'>\n", FORM_SAVE);
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='%s'>\n", FORM_RESET);
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.configedit.server_dir_base.focus();\n");
	prints(sid, "SQLTypeUpdate();\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}

void adminconfigsave(CONN *sid)
{
	CONFIG cfg;
	char *ptemp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	memset((char *)&cfg, 0, sizeof(cfg));
	config_read(&cfg);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_BASE"))!=NULL)       strncpy(cfg.server_dir_base,       ptemp, sizeof(cfg.server_dir_base)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_BIN"))!=NULL)        strncpy(cfg.server_dir_bin,        ptemp, sizeof(cfg.server_dir_bin)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_CGI"))!=NULL)        strncpy(cfg.server_dir_cgi,        ptemp, sizeof(cfg.server_dir_cgi)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_ETC"))!=NULL)        strncpy(cfg.server_dir_etc,        ptemp, sizeof(cfg.server_dir_etc)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_LIB"))!=NULL)        strncpy(cfg.server_dir_lib,        ptemp, sizeof(cfg.server_dir_lib)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR"))!=NULL)        strncpy(cfg.server_dir_var,        ptemp, sizeof(cfg.server_dir_var)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_BACKUP"))!=NULL) strncpy(cfg.server_dir_var_backup, ptemp, sizeof(cfg.server_dir_var_backup)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_DB"))!=NULL)     strncpy(cfg.server_dir_var_db,     ptemp, sizeof(cfg.server_dir_var_db)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_FILES"))!=NULL)  strncpy(cfg.server_dir_var_files,  ptemp, sizeof(cfg.server_dir_var_files)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_HTDOCS"))!=NULL) strncpy(cfg.server_dir_var_htdocs, ptemp, sizeof(cfg.server_dir_var_htdocs)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_LOG"))!=NULL)    strncpy(cfg.server_dir_var_log,    ptemp, sizeof(cfg.server_dir_var_log)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_MAIL"))!=NULL)   strncpy(cfg.server_dir_var_mail,   ptemp, sizeof(cfg.server_dir_var_mail)-1);
	if ((ptemp=getpostenv(sid, "SERVER_LOGLEVEL"))!=NULL)       cfg.server_loglevel=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_HOSTNAME"))!=NULL)       strncpy(cfg.server_hostname,       ptemp, sizeof(cfg.server_hostname)-1);
	if ((ptemp=getpostenv(sid, "SERVER_PORT"))!=NULL)           cfg.server_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SQL_TYPE"))!=NULL)              strncpy(cfg.sql_type,              ptemp, sizeof(cfg.sql_type)-1);
	if ((ptemp=getpostenv(sid, "SQL_HOSTNAME"))!=NULL)          strncpy(cfg.sql_hostname,          ptemp, sizeof(cfg.sql_hostname)-1);
	if ((ptemp=getpostenv(sid, "SQL_PORT"))!=NULL)              cfg.sql_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SQL_DBNAME"))!=NULL)            strncpy(cfg.sql_dbname,            ptemp, sizeof(cfg.sql_dbname)-1);
	if ((ptemp=getpostenv(sid, "SQL_USERNAME"))!=NULL)          strncpy(cfg.sql_username,          ptemp, sizeof(cfg.sql_username)-1);
	if ((ptemp=getpostenv(sid, "SQL_PASSWORD"))!=NULL)          strncpy(cfg.sql_password,          ptemp, sizeof(cfg.sql_password)-1);
#ifdef WIN32
	if ((ptemp=getpostenv(sid, "SQL_ODBC_DSN"))!=NULL)          strncpy(cfg.sql_odbc_dsn,          ptemp, sizeof(cfg.sql_odbc_dsn)-1);
#endif
	if (config_write(&cfg)!=0) {
		logerror(sid, __FILE__, __LINE__, ADM_CFG_NOFILE, proc->config_filename);
		prints(sid, ADM_CFG_NOFILE, proc->config_filename);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE><TR><TD><PRE>");
	prints(sid, ADM_CFG_SAVED);
	prints(sid, "</PRE></TD><TR></TABLE>\n</CENTER>\n");
	db_log_activity(sid, 1, "config", 0, "modify", "%s - %s modified config", sid->dat->in_RemoteAddr, sid->dat->user_username);
	flushbuffer(sid);
//	server_restart();
	return;
}
