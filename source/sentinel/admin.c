/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

void adminaccess(int sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	snprintf(file, sizeof(file)-1, "%s/access.log", config.server_etc_dir);
	FixSlashes(file);
	prints("<CENTER><HR><TABLE><TR><TD><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			prints("%s", line);
		}
		fclose(fp);
	}
	prints("</PRE></TD></TR></TABLE><HR></CENTER>\n");
}

void adminerror(int sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	snprintf(file, sizeof(file)-1, "%s/error.log", config.server_etc_dir);
	FixSlashes(file);
	prints("<CENTER><HR><TABLE><TR><TD><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			prints("%s", line);
		}
		fclose(fp);
	}
	prints("</PRE></TD></TR></TABLE><HR></CENTER>\n");
}

void adminkeyinfo(int sid)
{
	struct timeval ttime;
	struct timezone tzone;
	int unixdate;
	int expirydate;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	prints("<CENTER><TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR><TH BGCOLOR=%s COLSPAN=2 NOWRAP>LICENSE INFORMATION</TH></TR>\n", COLOR_TRIM);
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Name         </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>", COLOR_FNAME, COLOR_FTEXT, config.key.Name);
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>E-Mail       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>", COLOR_FNAME, COLOR_FTEXT, config.key.Email);
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>License Type </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>", COLOR_FNAME, COLOR_FTEXT, config.key.LicenseType);
	if (strcmp(config.key.LicenseType, "Evaluation")==0) {
		gettimeofday(&ttime, &tzone);
		unixdate=(int)((ttime.tv_sec-tzone.tz_minuteswest*60)/86400);
		expirydate=sqldate2unix(config.key.Expiration)/86400;
		prints("<TR><TD BGCOLOR=%s NOWRAP><B>Expiration   </B></TD><TD BGCOLOR=%s NOWRAP>%s (%d days left)&nbsp;</TD></TR>", COLOR_FNAME, COLOR_FTEXT, sqldate2text(config.key.Expiration), expirydate-unixdate);
	} else {
		prints("<TR><TD BGCOLOR=%s NOWRAP><B>License Class</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>", COLOR_FNAME, COLOR_FTEXT, config.key.LicenseClass);
	}
	prints("</TABLE></CENTER>\n");
}

int configread()
{
	FILE *fp=NULL;
	char configfile[200];
	char line[512];
	struct stat sb;
	char *pVar;
	char *pVal;
	short int founddir=0;
#ifdef WIN32
	char slash='\\';
#else
	char slash='/';
#endif

	/* define default values */
	memset((char *)&config, 0, sizeof(config));
	pVal=program_name;
	if (*pVal=='\"') pVal++;
#ifdef WIN32
	snprintf(config.server_base_dir, sizeof(config.server_base_dir)-1, "%s", pVal);
#else
	if (getcwd(config.server_base_dir, sizeof(config.server_base_dir)-1)==NULL) return -1;
	strcat(config.server_base_dir, "/");
#endif
	if (strrchr(config.server_base_dir, slash)!=NULL) {
		pVal=strrchr(config.server_base_dir, slash);
		*pVal='\0';
		chdir(config.server_base_dir);
		if (strrchr(config.server_base_dir, slash)!=NULL) {
			pVal=strrchr(config.server_base_dir, slash);
			*pVal='\0';
			founddir=1;
		}
	}
	if (!founddir) {
		snprintf(config.server_base_dir, sizeof(config.server_base_dir)-1, "%s", DEFAULT_BASE_DIR);
	}
	snprintf(config.server_bin_dir, sizeof(config.server_bin_dir)-1, "%s/bin", config.server_base_dir);
	snprintf(config.server_etc_dir, sizeof(config.server_etc_dir)-1, "%s/etc", config.server_base_dir);
	snprintf(config.server_file_dir, sizeof(config.server_file_dir)-1, "%s/files", config.server_base_dir);
	snprintf(config.server_http_dir, sizeof(config.server_http_dir)-1, "%s/http", config.server_base_dir);
	FixSlashes(config.server_base_dir);
	FixSlashes(config.server_bin_dir);
	FixSlashes(config.server_etc_dir);
	FixSlashes(config.server_file_dir);
	FixSlashes(config.server_http_dir);
#ifdef HAVE_ODBC
	strncpy(config.sql_type, "ODBC", sizeof(config.sql_type)-1);
	snprintf(config.sql_odbc_dsn, sizeof(config.sql_odbc_dsn)-1, "DRIVER={Microsoft Access Driver (*.mdb)};DBQ=%s\\sentinel.mdb", config.server_etc_dir);
#else
#ifdef HAVE_MYSQL
	strncpy(config.sql_type, "MYSQL", sizeof(config.sql_type)-1);
	config.sql_port=3306;
	strncpy(config.sql_username, "root", sizeof(config.sql_username)-1);
	strncpy(config.sql_password, "password", sizeof(config.sql_password)-1);
	strncpy(config.sql_hostname, "localhost", sizeof(config.sql_hostname)-1);
	strncpy(config.sql_dbname, "sentinel", sizeof(config.sql_dbname)-1);
#else
	strncpy(config.sql_type, "PGSQL", sizeof(config.sql_type)-1);
	config.sql_port=5432;
	strncpy(config.sql_username, "postgres", sizeof(config.sql_username)-1);
	strncpy(config.sql_password, "password", sizeof(config.sql_password)-1);
	strncpy(config.sql_hostname, "localhost", sizeof(config.sql_hostname)-1);
	strncpy(config.sql_dbname, "sentinel", sizeof(config.sql_dbname)-1);
#endif
#endif
	config.server_loglevel=1;
	strncpy(config.server_hostname, "localhost", sizeof(config.server_hostname)-1);
	config.server_port=4110;
	config.server_maxconn=50;
	config.server_maxidle=120;
	config.sql_maxconn=config.server_maxconn*2;
	/* try to open the config file */
	/* try the current directory first, then ../etc/, then the default etc/ */
	if (fp==NULL) {
		snprintf(configfile, sizeof(configfile)-1, "sentinel.cfg");
		fp=fopen(configfile, "r");
	}
	if (fp==NULL) {
		snprintf(configfile, sizeof(configfile)-1, "%s/sentinel.cfg", config.server_etc_dir);
		FixSlashes(configfile);
		fp=fopen(configfile, "r");
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		if (stat(config.server_etc_dir, &sb)!=0) {
			logerror("Directory '%s' does not exist.  Failed to create configuration file.", config.server_etc_dir);
			return -1;
		};
		printf("Creating configuration file...");
		logaccess(1, "Creating configuration file...");
		snprintf(configfile, sizeof(configfile)-1, "%s/sentinel.cfg", config.server_etc_dir);
		FixSlashes(configfile);
		fp=fopen(configfile, "w");
		if (fp==NULL) {
			return -1;
		}
		fprintf(fp, "# This file contains system settings for Sentinel Groupware.\n\n");
		fprintf(fp, "SQL_TYPE           = \"%s\"\n", config.sql_type);
		fprintf(fp, "SQL_HOSTNAME       = \"%s\"\n", config.sql_hostname);
		fprintf(fp, "SQL_PORT           = \"%d\"\n", config.sql_port);
		fprintf(fp, "SQL_DBNAME         = \"%s\"\n", config.sql_dbname);
		fprintf(fp, "SQL_USERNAME       = \"%s\"\n", config.sql_username);
		fprintf(fp, "SQL_PASSWORD       = \"%s\"\n", config.sql_password);
#ifdef WIN32
		fprintf(fp, "SQL_ODBC_DSN       = \"%s\"\n", config.sql_odbc_dsn);
#endif
		fprintf(fp, "SENTINEL_BASE_DIR  = \"%s\"\n", config.server_base_dir);
		fprintf(fp, "SENTINEL_BIN_DIR   = \"%s\"\n", config.server_bin_dir);
		fprintf(fp, "SENTINEL_ETC_DIR   = \"%s\"\n", config.server_etc_dir);
		fprintf(fp, "SENTINEL_FILE_DIR  = \"%s\"\n", config.server_file_dir);
		fprintf(fp, "SENTINEL_HTTP_DIR  = \"%s\"\n", config.server_http_dir);
		fprintf(fp, "SENTINEL_LOGLEVEL  = \"%d\"\n", config.server_loglevel);
		fprintf(fp, "SENTINEL_HOSTNAME  = \"%s\"\n", config.server_hostname);
		fprintf(fp, "SENTINEL_PORT      = \"%d\"\n", config.server_port);
		fprintf(fp, "SENTINEL_MAXCONN   = \"%d\"\n", config.server_maxconn);
		fprintf(fp, "SENTINEL_MAXIDLE   = \"%d\"\n", config.server_maxidle);
		fclose(fp);
		printf("done.\n");
		return 0;
	}
	/* else if config file does exist, read it */
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) {
			line[strlen(line)-1]='\0';
		}
		if (isalpha(line[0])) {
			pVar=line;
			pVal=line;
			while ((*pVal!='=')&&((char *)&pVal+1!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			while (*pVal=='"') pVal++;
			while (pVal[strlen(pVal)-1]=='"') pVal[strlen(pVal)-1]='\0';
			if (strcmp(pVar, "SENTINEL_BASE_DIR")==0) {
				strncpy(config.server_base_dir, pVal, sizeof(config.server_base_dir)-1);
			} else if (strcmp(pVar, "SENTINEL_BIN_DIR")==0) {
				strncpy(config.server_bin_dir, pVal, sizeof(config.server_bin_dir)-1);
			} else if (strcmp(pVar, "SENTINEL_ETC_DIR")==0) {
				strncpy(config.server_etc_dir, pVal, sizeof(config.server_etc_dir)-1);
			} else if (strcmp(pVar, "SENTINEL_FILE_DIR")==0) {
				strncpy(config.server_file_dir, pVal, sizeof(config.server_file_dir)-1);
			} else if (strcmp(pVar, "SENTINEL_HTTP_DIR")==0) {
				strncpy(config.server_http_dir, pVal, sizeof(config.server_http_dir)-1);
			} else if (strcmp(pVar, "SENTINEL_LOGLEVEL")==0) {
				config.server_loglevel=atoi(pVal);
			} else if (strcmp(pVar, "SENTINEL_HOSTNAME")==0) {
				strncpy(config.server_hostname, pVal, sizeof(config.server_hostname)-1);
			} else if (strcmp(pVar, "SENTINEL_MAXCONN")==0) {
				config.server_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "SENTINEL_PORT")==0) {
				config.server_port=atoi(pVal);
			} else if (strcmp(pVar, "SENTINEL_MAXIDLE")==0) {
				config.server_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "SQL_TYPE")==0) {
				strncpy(config.sql_type, pVal, sizeof(config.sql_type)-1);
			} else if (strcmp(pVar, "SQL_HOSTNAME")==0) {
				strncpy(config.sql_hostname, pVal, sizeof(config.sql_hostname)-1);
			} else if (strcmp(pVar, "SQL_PORT")==0) {
				config.sql_port=atoi(pVal);
			} else if (strcmp(pVar, "SQL_DBNAME")==0) {
				strncpy(config.sql_dbname, pVal, sizeof(config.sql_dbname)-1);
			} else if (strcmp(pVar, "SQL_USERNAME")==0) {
				strncpy(config.sql_username, pVal, sizeof(config.sql_username)-1);
			} else if (strcmp(pVar, "SQL_PASSWORD")==0) {
				strncpy(config.sql_password, pVal, sizeof(config.sql_password)-1);
			} else if (strcmp(pVar, "SQL_ODBC_DSN")==0) {
				strncpy(config.sql_odbc_dsn, pVal, sizeof(config.sql_odbc_dsn)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	if (config.server_maxconn<1) config.server_maxconn=1;
	if (config.server_maxconn>1000) config.server_maxconn=1000;
	config.sql_maxconn=config.server_maxconn*2;
	return 0;
}

void adminconfigedit(int sid)
{
	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/adminconfigsave NAME=configedit>\n", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4>Sentinel Groupware Configuration</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><B>SQL Server Type           </B></TD><TD>", COLOR_EDITFORM);
	prints("<SELECT NAME=SQL_TYPE>\n");
#ifdef HAVE_MYSQL
	prints("<OPTION VALUE='MYSQL'");
	if (strcmp(config.sql_type, "MYSQL")==0) prints(" SELECTED");
	prints(">MYSQL\n");
#endif
#ifdef HAVE_ODBC
	prints("<OPTION VALUE='ODBC'");
	if (strcmp(config.sql_type, "ODBC")==0) prints(" SELECTED");
	prints(">ODBC\n");
#endif
#ifdef HAVE_PGSQL
	prints("<OPTION VALUE='PGSQL'");
	if (strcmp(config.sql_type, "PGSQL")==0) prints(" SELECTED");
	prints(">PGSQL\n");
#endif
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL Server Host Name   </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_HOSTNAME VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_hostname);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL Server Port        </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_PORT     VALUE='%d' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_port);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL Database Name      </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_DBNAME   VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_dbname);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL Database Username  </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_USERNAME VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_username);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL Database Password  </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_PASSWORD VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_password);
#ifdef WIN32
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SQL ODBC Data Source   </B></TD><TD><INPUT TYPE=TEXT NAME=SQL_ODBC_DSN VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.sql_odbc_dsn);
#endif
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel Base Directory</B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_BASE_DIR VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_base_dir);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel BIN Directory </B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_BIN_DIR  VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_bin_dir);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel ETC Directory </B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_ETC_DIR  VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_etc_dir);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel File Directory</B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_FILE_DIR VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_file_dir);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel HTTP Directory</B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_HTTP_DIR VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_http_dir);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel Log Level     </B></TD><TD><SELECT NAME=SENTINEL_LOGLEVEL>", COLOR_EDITFORM);
	numberselect(sid, config.server_loglevel, 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel Host Name     </B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_HOSTNAME VALUE='%s' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_hostname);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Sentinel Port          </B></TD><TD><INPUT TYPE=TEXT NAME=SENTINEL_PORT     VALUE='%d' SIZE=35></TD></TR>\n", COLOR_EDITFORM, config.server_port);
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.configedit.SQL_TYPE.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminconfigsave(int sid)
{
	char configfile[100];
	char *pval;
	FILE *fp;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0) return;
	if ((pval=getpostenv(sid, "SENTINEL_BASE_DIR"))!=NULL)
		strncpy(config.server_base_dir, pval, sizeof(config.server_base_dir)-1);
	if ((pval=getpostenv(sid, "SENTINEL_BIN_DIR"))!=NULL)
		strncpy(config.server_bin_dir,  pval, sizeof(config.server_bin_dir)-1);
	if ((pval=getpostenv(sid, "SENTINEL_ETC_DIR"))!=NULL)
		strncpy(config.server_etc_dir,  pval, sizeof(config.server_etc_dir)-1);
	if ((pval=getpostenv(sid, "SENTINEL_FILE_DIR"))!=NULL)
		strncpy(config.server_file_dir, pval, sizeof(config.server_file_dir)-1);
	if ((pval=getpostenv(sid, "SENTINEL_HTTP_DIR"))!=NULL)
		strncpy(config.server_http_dir, pval, sizeof(config.server_http_dir)-1);
	if ((pval=getpostenv(sid, "SENTINEL_LOGLEVEL"))!=NULL)
		config.server_loglevel=atoi(pval);
	if ((pval=getpostenv(sid, "SENTINEL_HOSTNAME"))!=NULL)
		strncpy(config.server_hostname, pval, sizeof(config.server_hostname)-1);
	if ((pval=getpostenv(sid, "SENTINEL_PORT"))!=NULL)
		config.server_port=atoi(pval);
	if ((pval=getpostenv(sid, "SQL_TYPE"))!=NULL)
		strncpy(config.sql_type, pval, sizeof(config.sql_type)-1);
	if ((pval=getpostenv(sid, "SQL_USERNAME"))!=NULL)
		strncpy(config.sql_username, pval, sizeof(config.sql_username)-1);
	if ((pval=getpostenv(sid, "SQL_PASSWORD"))!=NULL)
		strncpy(config.sql_password, pval, sizeof(config.sql_password)-1);
	if ((pval=getpostenv(sid, "SQL_HOSTNAME"))!=NULL)
		strncpy(config.sql_hostname, pval, sizeof(config.sql_hostname)-1);
	if ((pval=getpostenv(sid, "SQL_DBNAME"))!=NULL)
		strncpy(config.sql_dbname, pval, sizeof(config.sql_dbname)-1);
	if ((pval=getpostenv(sid, "SQL_PORT"))!=NULL)
		config.sql_port=atoi(pval);
#ifdef WIN32
	strncpy(config.sql_odbc_dsn, getpostenv(sid, "SQL_ODBC_DSN"), sizeof(config.sql_odbc_dsn)-1);
#endif
	snprintf(configfile, sizeof(configfile)-1, "%s/sentinel.cfg", config.server_etc_dir);
	FixSlashes(configfile);
	fp=fopen(configfile, "w");
	if (fp==NULL) {
		logerror("ERROR: Cannot write to configuration file %s.", configfile);
		prints("ERROR: Cannot create configuration file %s.\n", configfile);
		return;
	}
	fprintf(fp, "# This file contains system settings for Sentinel Groupware.\n\n");
	fprintf(fp, "SQL_TYPE           = \"%s\"\n", config.sql_type);
	fprintf(fp, "SQL_HOSTNAME       = \"%s\"\n", config.sql_hostname);
	fprintf(fp, "SQL_PORT           = \"%d\"\n", config.sql_port);
	fprintf(fp, "SQL_DBNAME         = \"%s\"\n", config.sql_dbname);
	fprintf(fp, "SQL_USERNAME       = \"%s\"\n", config.sql_username);
	fprintf(fp, "SQL_PASSWORD       = \"%s\"\n", config.sql_password);
#ifdef WIN32
	fprintf(fp, "SQL_ODBC_DSN       = \"%s\"\n", config.sql_odbc_dsn);
#endif
	fprintf(fp, "SENTINEL_BASE_DIR  = \"%s\"\n", config.server_base_dir);
	fprintf(fp, "SENTINEL_BIN_DIR   = \"%s\"\n", config.server_bin_dir);
	fprintf(fp, "SENTINEL_ETC_DIR   = \"%s\"\n", config.server_etc_dir);
	fprintf(fp, "SENTINEL_FILE_DIR  = \"%s\"\n", config.server_file_dir);
	fprintf(fp, "SENTINEL_HTTP_DIR  = \"%s\"\n", config.server_http_dir);
	fprintf(fp, "SENTINEL_LOGLEVEL  = \"%d\"\n", config.server_loglevel);
	fprintf(fp, "SENTINEL_HOSTNAME  = \"%s\"\n", config.server_hostname);
	fprintf(fp, "SENTINEL_PORT      = \"%d\"\n", config.server_port);
	fprintf(fp, "SENTINEL_MAXCONN   = \"%d\"\n", config.server_maxconn);
	fprintf(fp, "SENTINEL_MAXIDLE   = \"%d\"\n", config.server_maxidle);
	fclose(fp);
	prints("<CENTER>\n<TABLE><TR><TD><PRE>");
	prints("Sentinel configuration modified successfully.\n");
	prints("In order to make sure all settings take effect, you\n");
	prints("may need to restart the Sentinel Groupware server.\n");
	prints("</PRE></TD><TR></TABLE>\n</CENTER>\n");
	logaccess(1, "%s - %s modified configuration", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
	return;
}

void adminmotdedit(int sid)
{
	FILE *fp;
	char motdfile[100];
	char line[512];

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	snprintf(motdfile, sizeof(motdfile)-1, "%s/motd.txt", config.server_etc_dir);
	FixSlashes(motdfile);
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/adminmotdsave NAME=motdedit>\n", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH>Message of the Day</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><FONT SIZE=3><TEXTAREA WRAP=virtual NAME=MOTDTEXT ROWS=15 COLS=65>", COLOR_EDITFORM);
	fp=fopen(motdfile, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			prints("%s", line);
		}
		fclose(fp);
	}
	prints("</TEXTAREA></TD></TR>\n</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.motdedit.MOTDTEXT.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminmotdsave(int sid)
{
	char motdfile[200];
	FILE *fp;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0) return;
	if (getpostenv(sid, "MOTDTEXT")==NULL) return;
	snprintf(motdfile, sizeof(motdfile)-1, "%s/motd.txt", config.server_etc_dir);
	FixSlashes(motdfile);
	fp=fopen(motdfile, "w");
	if (fp==NULL) {
		logerror("ERROR: Cannot create MOTD file %s.\n", motdfile);
		return;
	}
	fprintf(fp, "%s", getpostenv(sid, "MOTDTEXT"));
	fclose(fp);
	prints("<CENTER>Message of the Day modified successfully</CENTER>\n");
	logaccess(1, "%s - %s modified MOTD", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
	return;
}

void adminmain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Administration");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch02-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/adminaccess>ACCESS LOG</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/adminerror>ERROR LOG</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/adminconfigedit>CONFIGURATION</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/adminmotdedit>M.O.T.D.</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/adminuserlist>USERS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/admin")==0)
		adminkeyinfo(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminaccess", 12)==0)
		adminaccess(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminerror", 11)==0)
		adminerror(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminconfigedit", 16)==0)
		adminconfigedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminconfigsave", 16)==0)
		adminconfigsave(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminmotdedit", 14)==0)
		adminmotdedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminmotdsave", 14)==0)
		adminmotdsave(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminuseredit", 14)==0)
		useredit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminuserlist", 14)==0)
		userlist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/adminusersave", 14)==0)
		usersave(sid);
	printfooter(sid);
	return;
}
