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

/*
void adminconfigedit(CONN *sid)
{
	CONFIG cfg;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset((char *)&cfg, 0, sizeof(cfg));
	config_read(&cfg);
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
	htscript_showpage(sid, 3);
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<B>%s</B>\n", ADM_CFG_TITLE);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/configsave NAME=configedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TR><TD ALIGN=LEFT COLSPAN=2>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SERVER</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>PATHS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>SQL</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=http_hostname   VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVHOST, str2html(sid, cfg.http_hostname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=http_port       VALUE=\"%d\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVPORT, cfg.http_port);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=server_loglevel STYLE='width:255px'>", ADM_CFG_SRVLOG);
	htselect_number(sid, cfg.server_loglevel, 0, 4, 1);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>Max Connections</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=http_maxconn STYLE='width:255px'>");
	htselect_number(sid, cfg.http_maxconn, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>Max Idle Time</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=http_maxidle STYLE='width:255px'>");
	htselect_number(sid, cfg.http_maxidle, 5, 300, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>File Scanner</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=util_scanfile VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", str2html(sid, cfg.util_scanfile));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>Mail Scanner</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=util_scanmail VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", str2html(sid, cfg.util_scanmail));
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_base        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVBASE,  str2html(sid, cfg.server_dir_base));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_bin         VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVBIN,   str2html(sid, cfg.server_dir_bin));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_etc         VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVETC,   str2html(sid, cfg.server_dir_etc));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_lib         VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVLIB,   str2html(sid, cfg.server_dir_lib));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var         VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVVAR,   str2html(sid, cfg.server_dir_var));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_backup  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVBAK,   str2html(sid, cfg.server_dir_var_backup));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_cgi     VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVCGI,   str2html(sid, cfg.server_dir_var_cgi));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_db      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVDB,    str2html(sid, cfg.server_dir_var_db));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_domains VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVDOM,   str2html(sid, cfg.server_dir_var_domains));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_htdocs  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVHDOC,  str2html(sid, cfg.server_dir_var_htdocs));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_log     VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVLOG,   str2html(sid, cfg.server_dir_var_log));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_spool   VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SRVSPOOL, str2html(sid, cfg.server_dir_var_spool));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>Server Temp Directory</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_tmp    VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", str2html(sid, cfg.server_dir_var_tmp));
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT>", ADM_CFG_SQLTYPE);
	prints(sid, "<SELECT NAME=sql_type onchange=FormUpdate(); STYLE='width:255px'>\n");
#ifdef HAVE_MYSQL
	prints(sid, "<OPTION VALUE='MYSQL'%s>MYSQL\n", strcmp(cfg.sql_type, "MYSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_ODBC
	prints(sid, "<OPTION VALUE='ODBC'%s>ODBC\n", strcmp(cfg.sql_type, "ODBC")==0?" SELECTED":"");
#endif
#ifdef HAVE_PGSQL
	prints(sid, "<OPTION VALUE='PGSQL'%s>PGSQL\n", strcmp(cfg.sql_type, "PGSQL")==0?" SELECTED":"");
#endif
#ifdef HAVE_SQLITE3
	prints(sid, "<OPTION VALUE='SQLITE'%s>SQLITE\n", strcmp(cfg.sql_type, "SQLITE")==0?" SELECTED":"");
#endif
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_hostname      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLHOST, str2html(sid, cfg.sql_hostname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_port          VALUE=\"%d\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLPORT, cfg.sql_port);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_dbname        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLNAME, str2html(sid, cfg.sql_dbname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_username      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLUSER, str2html(sid, cfg.sql_username));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=sql_password  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLPASS, str2html(sid, cfg.sql_password));
#ifdef WIN32
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_odbc_dsn      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", ADM_CFG_SQLODBC, str2html(sid, cfg.sql_odbc_dsn));
#endif
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='%s'>\n", FORM_SAVE);
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='%s'>\n", FORM_RESET);
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "document.configedit.http_hostname.focus();\n");
	prints(sid, "showpage(1);\n");
	prints(sid, "SQLTypeUpdate();\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}
*/

/*
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

	if ((ptemp=getpostenv(sid, "HTTP_HOSTNAME"))!=NULL)          strncpy(cfg.http_hostname,          ptemp, sizeof(cfg.http_hostname)-1);
	if ((ptemp=getpostenv(sid, "HTTP_PORT"))!=NULL)              cfg.http_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_LOGLEVEL"))!=NULL)        cfg.server_loglevel=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "HTTP_MAXCONN"))!=NULL)           cfg.http_maxconn=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "HTTP_MAXIDLE"))!=NULL)           cfg.http_maxidle=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_BASE"))!=NULL)        strncpy(cfg.server_dir_base,        ptemp, sizeof(cfg.server_dir_base)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_BIN"))!=NULL)         strncpy(cfg.server_dir_bin,         ptemp, sizeof(cfg.server_dir_bin)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_ETC"))!=NULL)         strncpy(cfg.server_dir_etc,         ptemp, sizeof(cfg.server_dir_etc)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_LIB"))!=NULL)         strncpy(cfg.server_dir_lib,         ptemp, sizeof(cfg.server_dir_lib)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR"))!=NULL)         strncpy(cfg.server_dir_var,         ptemp, sizeof(cfg.server_dir_var)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_BACKUP"))!=NULL)  strncpy(cfg.server_dir_var_backup,  ptemp, sizeof(cfg.server_dir_var_backup)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_CGI"))!=NULL)     strncpy(cfg.server_dir_var_cgi,     ptemp, sizeof(cfg.server_dir_var_cgi)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_DB"))!=NULL)      strncpy(cfg.server_dir_var_db,      ptemp, sizeof(cfg.server_dir_var_db)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_DOMAINS"))!=NULL) strncpy(cfg.server_dir_var_domains, ptemp, sizeof(cfg.server_dir_var_domains)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_HTDOCS"))!=NULL)  strncpy(cfg.server_dir_var_htdocs,  ptemp, sizeof(cfg.server_dir_var_htdocs)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_LOG"))!=NULL)     strncpy(cfg.server_dir_var_log,     ptemp, sizeof(cfg.server_dir_var_log)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_SPOOL"))!=NULL)   strncpy(cfg.server_dir_var_spool,   ptemp, sizeof(cfg.server_dir_var_spool)-1);
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_TMP"))!=NULL)     strncpy(cfg.server_dir_var_tmp,     ptemp, sizeof(cfg.server_dir_var_tmp)-1);
	if ((ptemp=getpostenv(sid, "SQL_TYPE"))!=NULL)               strncpy(cfg.sql_type,               ptemp, sizeof(cfg.sql_type)-1);
	if ((ptemp=getpostenv(sid, "SQL_HOSTNAME"))!=NULL)           strncpy(cfg.sql_hostname,           ptemp, sizeof(cfg.sql_hostname)-1);
	if ((ptemp=getpostenv(sid, "SQL_PORT"))!=NULL)               cfg.sql_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SQL_DBNAME"))!=NULL)             strncpy(cfg.sql_dbname,             ptemp, sizeof(cfg.sql_dbname)-1);
	if ((ptemp=getpostenv(sid, "SQL_USERNAME"))!=NULL)           strncpy(cfg.sql_username,           ptemp, sizeof(cfg.sql_username)-1);
	if ((ptemp=getpostenv(sid, "SQL_PASSWORD"))!=NULL)           strncpy(cfg.sql_password,           ptemp, sizeof(cfg.sql_password)-1);
#ifdef WIN32
	if ((ptemp=getpostenv(sid, "SQL_ODBC_DSN"))!=NULL)           strncpy(cfg.sql_odbc_dsn,           ptemp, sizeof(cfg.sql_odbc_dsn)-1);
#endif
	if ((ptemp=getpostenv(sid, "UTIL_SCANFILE"))!=NULL)          strncpy(cfg.util_scanfile,          ptemp, sizeof(cfg.util_scanfile)-1);
	if ((ptemp=getpostenv(sid, "UTIL_SCANMAIL"))!=NULL)          strncpy(cfg.util_scanmail,          ptemp, sizeof(cfg.util_scanmail)-1);
	if (config_write(&cfg)!=0) {
		log_error("mod_admin", __FILE__, __LINE__, 1, ADM_CFG_NOFILE, proc->config_filename);
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
*/
