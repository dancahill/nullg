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
	htscript_showpage(sid, 4);
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<B>%s</B>\n", ADM_CFG_TITLE);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/configsave NAME=configedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TR><TD ALIGN=LEFT COLSPAN=2>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR BGCOLOR=%s>\n", config->colour_fieldname);
	prints(sid, "<TD ID=page1tab STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(1)>SERVER</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(2)>PATHS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(3)>COLOURS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page4tab STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(4)>SQL</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:3px'>", config->colour_editform);
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_hostname   VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVHOST, str2html(sid, cfg.server_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_port       VALUE=\"%d\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVPORT, cfg.server_port);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=server_loglevel STYLE='width:255px'>", config->colour_editform, ADM_CFG_SRVLOG);
	htselect_number(sid, cfg.server_loglevel, 0, 4, 1);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Max Connections</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=server_maxconn STYLE='width:255px'>", config->colour_editform);
	htselect_number(sid, cfg.server_maxconn, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Max Idle Time</B>&nbsp;</TD><TD ALIGN=RIGHT><SELECT NAME=server_maxidle STYLE='width:255px'>", config->colour_editform);
	htselect_number(sid, cfg.server_maxidle, 5, 300, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Virus Scanner</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=util_virusscan   VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, str2html(sid, cfg.util_virusscan));
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_base       VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBASE, str2html(sid, cfg.server_dir_base));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_bin        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBIN,  str2html(sid, cfg.server_dir_bin));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_cgi        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVCGI,  str2html(sid, cfg.server_dir_cgi));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_etc        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVETC,  str2html(sid, cfg.server_dir_etc));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_lib        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVLIB,  str2html(sid, cfg.server_dir_lib));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVVAR,  str2html(sid, cfg.server_dir_var));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_backup VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVBAK,  str2html(sid, cfg.server_dir_var_backup));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_db     VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVDB,   str2html(sid, cfg.server_dir_var_db));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_files  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVFILE, str2html(sid, cfg.server_dir_var_files));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_htdocs VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVHDOC, str2html(sid, cfg.server_dir_var_htdocs));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_log    VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVLOG,  str2html(sid, cfg.server_dir_var_log));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_mail   VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SRVMAIL, str2html(sid, cfg.server_dir_var_mail));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Server Temp Directory</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=server_dir_var_tmp    VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, str2html(sid, cfg.server_dir_var_tmp));
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Edit Form Background</B>&nbsp;</TD>   <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_editform      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Field Name Background</B>&nbsp;</TD>  <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_fieldname     VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_fieldname);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Field Name Text</B>&nbsp;</TD>        <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_fieldnametext VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_fieldnametext);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Field Value Background</B>&nbsp;</TD> <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_fieldval      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_fieldval);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Field Value Text</B>&nbsp;</TD>       <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_fieldvaltext  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_fieldvaltext);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Hyperlinks</B>&nbsp;</TD>             <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_links         VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_links);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Table Trim</B>&nbsp;</TD>             <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_tabletrim     VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Table Header Background</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_th            VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_th);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Table Header Hyperlinks</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_thlink        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_thlink);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Table Header Text</B>&nbsp;</TD>      <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_thtext        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>Top Menu</B>&nbsp;</TD>               <TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=colour_topmenu       VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, cfg.colour_topmenu);
	prints(sid, "</TABLE>");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT>", config->colour_editform, ADM_CFG_SQLTYPE);
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
#ifdef HAVE_SQLITE
	prints(sid, "<OPTION VALUE='SQLITE'%s>SQLITE\n", strcmp(cfg.sql_type, "SQLITE")==0?" SELECTED":"");
#endif
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_hostname      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLHOST, str2html(sid, cfg.sql_hostname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_port          VALUE=\"%d\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLPORT, cfg.sql_port);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_dbname        VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLNAME, str2html(sid, cfg.sql_dbname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_username      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLUSER, str2html(sid, cfg.sql_username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=sql_password  VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLPASS, str2html(sid, cfg.sql_password));
#ifdef WIN32
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>%s</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sql_odbc_dsn      VALUE=\"%s\" SIZE=45 STYLE='width:255px'></TD></TR>\n", config->colour_editform, ADM_CFG_SQLODBC, str2html(sid, cfg.sql_odbc_dsn));
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
	prints(sid, "document.configedit.server_hostname.focus();\n");
	prints(sid, "showpage(1);\n");
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

	if ((ptemp=getpostenv(sid, "SERVER_HOSTNAME"))!=NULL)       strncpy(cfg.server_hostname,       ptemp, sizeof(cfg.server_hostname)-1);
	if ((ptemp=getpostenv(sid, "SERVER_PORT"))!=NULL)           cfg.server_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_LOGLEVEL"))!=NULL)       cfg.server_loglevel=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_MAXCONN"))!=NULL)        cfg.server_maxconn=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SERVER_MAXIDLE"))!=NULL)        cfg.server_maxidle=atoi(ptemp);
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
	if ((ptemp=getpostenv(sid, "SERVER_DIR_VAR_TMP"))!=NULL)    strncpy(cfg.server_dir_var_tmp,    ptemp, sizeof(cfg.server_dir_var_tmp)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_EDITFORM"))!=NULL)       strncpy(cfg.colour_editform,       ptemp, sizeof(cfg.colour_editform)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_FIELDNAME"))!=NULL)      strncpy(cfg.colour_fieldname,      ptemp, sizeof(cfg.colour_fieldname)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_FIELDNAMETEXT"))!=NULL)  strncpy(cfg.colour_fieldnametext,  ptemp, sizeof(cfg.colour_fieldnametext)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_FIELDVAL"))!=NULL)       strncpy(cfg.colour_fieldval,       ptemp, sizeof(cfg.colour_fieldval)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_FIELDVALTEXT"))!=NULL)   strncpy(cfg.colour_fieldvaltext,   ptemp, sizeof(cfg.colour_fieldvaltext)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_LINKS"))!=NULL)          strncpy(cfg.colour_links,          ptemp, sizeof(cfg.colour_links)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_TABLETRIM"))!=NULL)      strncpy(cfg.colour_tabletrim,      ptemp, sizeof(cfg.colour_tabletrim)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_TH"))!=NULL)             strncpy(cfg.colour_th,             ptemp, sizeof(cfg.colour_th)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_THLINK"))!=NULL)         strncpy(cfg.colour_thlink,         ptemp, sizeof(cfg.colour_thlink)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_THTEXT"))!=NULL)         strncpy(cfg.colour_thtext,         ptemp, sizeof(cfg.colour_thtext)-1);
	if ((ptemp=getpostenv(sid, "COLOUR_TOPMENU"))!=NULL)        strncpy(cfg.colour_topmenu,        ptemp, sizeof(cfg.colour_topmenu)-1);
	if ((ptemp=getpostenv(sid, "SQL_TYPE"))!=NULL)              strncpy(cfg.sql_type,              ptemp, sizeof(cfg.sql_type)-1);
	if ((ptemp=getpostenv(sid, "SQL_HOSTNAME"))!=NULL)          strncpy(cfg.sql_hostname,          ptemp, sizeof(cfg.sql_hostname)-1);
	if ((ptemp=getpostenv(sid, "SQL_PORT"))!=NULL)              cfg.sql_port=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SQL_DBNAME"))!=NULL)            strncpy(cfg.sql_dbname,            ptemp, sizeof(cfg.sql_dbname)-1);
	if ((ptemp=getpostenv(sid, "SQL_USERNAME"))!=NULL)          strncpy(cfg.sql_username,          ptemp, sizeof(cfg.sql_username)-1);
	if ((ptemp=getpostenv(sid, "SQL_PASSWORD"))!=NULL)          strncpy(cfg.sql_password,          ptemp, sizeof(cfg.sql_password)-1);
#ifdef WIN32
	if ((ptemp=getpostenv(sid, "SQL_ODBC_DSN"))!=NULL)          strncpy(cfg.sql_odbc_dsn,          ptemp, sizeof(cfg.sql_odbc_dsn)-1);
#endif
	if ((ptemp=getpostenv(sid, "UTIL_VIRUSSCAN"))!=NULL)        strncpy(cfg.util_virusscan,        ptemp, sizeof(cfg.util_virusscan)-1);
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
