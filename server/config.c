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

int config_read()
{
	FILE *fp=NULL;
	char line[512];
	struct stat sb;
	char *pVar;
	char *pVal;
	short int founddir=0;
	int i;
#ifdef WIN32
	char slash='\\';
#else
	char slash='/';
#endif

	/* define default values */
	memset((char *)&config, 0, sizeof(config));
	snprintf(config.colour_topmenu,       sizeof(config.colour_topmenu)-1,       "%s", DEFAULT_COLOUR_TOPMENU);
	snprintf(config.colour_editform,      sizeof(config.colour_editform)-1,      "%s", DEFAULT_COLOUR_EDITFORM);
	snprintf(config.colour_fieldname,     sizeof(config.colour_fieldname)-1,     "%s", DEFAULT_COLOUR_FIELDNAME);
	snprintf(config.colour_fieldnametext, sizeof(config.colour_fieldnametext)-1, "%s", DEFAULT_COLOUR_FIELDNAMETEXT);
	snprintf(config.colour_fieldval,      sizeof(config.colour_fieldval)-1,      "%s", DEFAULT_COLOUR_FIELDVAL);
	snprintf(config.colour_fieldvaltext,  sizeof(config.colour_fieldvaltext)-1,  "%s", DEFAULT_COLOUR_FIELDVALTEXT);
	snprintf(config.colour_th,            sizeof(config.colour_th)-1,            "%s", DEFAULT_COLOUR_TH);
	snprintf(config.colour_thtext,        sizeof(config.colour_thtext)-1,        "%s", DEFAULT_COLOUR_THTEXT);
	snprintf(config.colour_thlink,        sizeof(config.colour_thlink)-1,        "%s", DEFAULT_COLOUR_THLINK);
	snprintf(config.colour_links,         sizeof(config.colour_links)-1,         "%s", DEFAULT_COLOUR_LINKS);
	pVal=program_name;
	if (*pVal=='\"') pVal++;
#ifdef WIN32
	snprintf(config.server_dir_base, sizeof(config.server_dir_base)-1, "%s", pVal);
#else
	if (getcwd(config.server_dir_base, sizeof(config.server_dir_base)-1)==NULL) return -1;
	strcat(config.server_dir_base, "/");
#endif
	if (strrchr(config.server_dir_base, slash)!=NULL) {
		pVal=strrchr(config.server_dir_base, slash);
		*pVal='\0';
		chdir(config.server_dir_base);
		if (strrchr(config.server_dir_base, slash)!=NULL) {
			pVal=strrchr(config.server_dir_base, slash);
			*pVal='\0';
			founddir=1;
		}
	}
	if (!founddir) {
		snprintf(config.server_dir_base, sizeof(config.server_dir_base)-1, "%s", DEFAULT_BASE_DIR);
	}
	snprintf(config.server_dir_bin, sizeof(config.server_dir_bin)-1, "%s/bin", config.server_dir_base);
	snprintf(config.server_dir_cgi, sizeof(config.server_dir_cgi)-1, "%s/cgi-bin", config.server_dir_base);
	snprintf(config.server_dir_etc, sizeof(config.server_dir_etc)-1, "%s/etc", config.server_dir_base);
	snprintf(config.server_dir_lib, sizeof(config.server_dir_lib)-1, "%s/lib", config.server_dir_base);
	snprintf(config.server_dir_var, sizeof(config.server_dir_var)-1, "%s/var", config.server_dir_base);
	snprintf(config.server_dir_var_backup, sizeof(config.server_dir_var_backup)-1, "%s/backup", config.server_dir_var);
	snprintf(config.server_dir_var_db,     sizeof(config.server_dir_var_db)-1, "%s/db", config.server_dir_var);
	snprintf(config.server_dir_var_files,  sizeof(config.server_dir_var_files)-1, "%s/files", config.server_dir_var);
	snprintf(config.server_dir_var_htdocs, sizeof(config.server_dir_var_htdocs)-1, "%s/htdocs", config.server_dir_var);
	snprintf(config.server_dir_var_log,    sizeof(config.server_dir_var_log)-1, "%s/log", config.server_dir_var);
	snprintf(config.server_dir_var_mail,   sizeof(config.server_dir_var_mail)-1, "%s/mail", config.server_dir_var);
	fixslashes(config.server_dir_base);
	fixslashes(config.server_dir_bin);
	fixslashes(config.server_dir_cgi);
	fixslashes(config.server_dir_etc);
	fixslashes(config.server_dir_lib);
	fixslashes(config.server_dir_var);
	fixslashes(config.server_dir_var_backup);
	fixslashes(config.server_dir_var_db);
	fixslashes(config.server_dir_var_files);
	fixslashes(config.server_dir_var_htdocs);
	fixslashes(config.server_dir_var_log);
	fixslashes(config.server_dir_var_mail);
#ifdef HAVE_SQLITE
	strncpy(config.sql_type, "SQLITE", sizeof(config.sql_type)-1);
#endif
	config.server_loglevel=1;
	config.server_port=4110;
	config.server_maxconn=50;
	config.server_maxidle=120;
	config.sql_maxconn=config.server_maxconn*2;
	/* try to open the config file */
	/* try the current directory first, then ../etc/, then the default etc/ */
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "groupware.cfg");
		fp=fopen(config.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "../etc/groupware.cfg");
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_dir_etc);
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "r");
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		if (stat(config.server_dir_etc, &sb)!=0) {
			logerror(NULL, __FILE__, __LINE__, ADM_CFG_NODIR, config.server_dir_etc);
			return -1;
		};
		printf("\r\n%s", ADM_CFG_MAKE);
		logaccess(NULL, 1, "%s", ADM_CFG_MAKE);
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_dir_etc);
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "w");
		if (fp==NULL) {
			return -1;
		}
		fprintf(fp, "# %s\n\n", ADM_CFG_HEAD);
		fprintf(fp, "SERVER.DIR.BASE       = \"%s\"\n", config.server_dir_base);
		fprintf(fp, "SERVER.DIR.BIN        = \"%s\"\n", config.server_dir_bin);
		fprintf(fp, "SERVER.DIR.CGI        = \"%s\"\n", config.server_dir_cgi);
		fprintf(fp, "SERVER.DIR.ETC        = \"%s\"\n", config.server_dir_etc);
		fprintf(fp, "SERVER.DIR.LIB        = \"%s\"\n", config.server_dir_lib);
		fprintf(fp, "SERVER.DIR.VAR        = \"%s\"\n", config.server_dir_var);
		fprintf(fp, "SERVER.DIR.VAR.BACKUP = \"%s\"\n", config.server_dir_var_backup);
		fprintf(fp, "SERVER.DIR.VAR.DB     = \"%s\"\n", config.server_dir_var_db);
		fprintf(fp, "SERVER.DIR.VAR.FILES  = \"%s\"\n", config.server_dir_var_files);
		fprintf(fp, "SERVER.DIR.VAR.HTDOCS = \"%s\"\n", config.server_dir_var_htdocs);
		fprintf(fp, "SERVER.DIR.VAR.LOG    = \"%s\"\n", config.server_dir_var_log);
		fprintf(fp, "SERVER.DIR.VAR.MAIL   = \"%s\"\n", config.server_dir_var_mail);
		fprintf(fp, "SERVER.LOGLEVEL       = \"%d\"\n", config.server_loglevel);
		fprintf(fp, "SERVER.HOSTNAME       = \"%s\"\n", config.server_hostname);
		fprintf(fp, "SERVER.PORT           = \"%d\"\n", config.server_port);
		fprintf(fp, "SERVER.MAXCONN        = \"%d\"\n", config.server_maxconn);
		fprintf(fp, "SERVER.MAXIDLE        = \"%d\"\n", config.server_maxidle);
		fprintf(fp, "SQL.TYPE              = \"%s\"\n", config.sql_type);
		fprintf(fp, "SQL.HOSTNAME          = \"%s\"\n", config.sql_hostname);
		fprintf(fp, "SQL.PORT              = \"%d\"\n", config.sql_port);
		fprintf(fp, "SQL.DBNAME            = \"%s\"\n", config.sql_dbname);
		fprintf(fp, "SQL.USERNAME          = \"%s\"\n", config.sql_username);
		fprintf(fp, "SQL.PASSWORD          = \"%s\"\n", config.sql_password);
#ifdef WIN32
		fprintf(fp, "SQL_ODBC_DSN          = \"%s\"\n", config.sql_odbc_dsn);
#endif
		fclose(fp);
		printf("done.\n");
		goto sanity_check;
	}
	/* else if config file does exist, read it */
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
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
			if (strcmp(pVar, "COLOUR.TOPMENU")==0) {
				snprintf(config.colour_topmenu,       sizeof(config.colour_topmenu)-1,       "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.EDITFORM")==0) {
				snprintf(config.colour_editform,      sizeof(config.colour_editform)-1,      "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.FIELDNAME")==0) {
				snprintf(config.colour_fieldname,     sizeof(config.colour_fieldname)-1,     "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.FIELDNAMETEXT")==0) {
				snprintf(config.colour_fieldnametext, sizeof(config.colour_fieldnametext)-1, "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.FIELDVAL")==0) {
				snprintf(config.colour_fieldval,      sizeof(config.colour_fieldval)-1,      "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.FIELDVALTEXT")==0) {
				snprintf(config.colour_fieldvaltext,  sizeof(config.colour_fieldvaltext)-1,  "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.TH")==0) {
				snprintf(config.colour_th,            sizeof(config.colour_th)-1,            "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.THTEXT")==0) {
				snprintf(config.colour_thtext,        sizeof(config.colour_thtext)-1,        "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.THLINK")==0) {
				snprintf(config.colour_thlink,        sizeof(config.colour_thlink)-1,        "%s", pVal);
			} else if (strcmp(pVar, "COLOUR.LINKS")==0) {
				snprintf(config.colour_links,         sizeof(config.colour_links)-1,         "%s", pVal);
			} else if (strcmp(pVar, "SERVER.DIR.BASE")==0) {
				strncpy(config.server_dir_base, pVal, sizeof(config.server_dir_base)-1);
			} else if (strcmp(pVar, "SERVER.DIR.BIN")==0) {
				strncpy(config.server_dir_bin, pVal, sizeof(config.server_dir_bin)-1);
			} else if (strcmp(pVar, "SERVER.DIR.CGI")==0) {
				strncpy(config.server_dir_cgi, pVal, sizeof(config.server_dir_cgi)-1);
			} else if (strcmp(pVar, "SERVER.DIR.ETC")==0) {
				strncpy(config.server_dir_etc, pVal, sizeof(config.server_dir_etc)-1);
			} else if (strcmp(pVar, "SERVER.DIR.LIB")==0) {
				strncpy(config.server_dir_lib, pVal, sizeof(config.server_dir_lib)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR")==0) {
				strncpy(config.server_dir_var, pVal, sizeof(config.server_dir_var)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.BACKUP")==0) {
				strncpy(config.server_dir_var_backup, pVal, sizeof(config.server_dir_var_backup)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.DB")==0) {
				strncpy(config.server_dir_var_db, pVal, sizeof(config.server_dir_var_db)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.FILES")==0) {
				strncpy(config.server_dir_var_files, pVal, sizeof(config.server_dir_var_files)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.HTDOCS")==0) {
				strncpy(config.server_dir_var_htdocs, pVal, sizeof(config.server_dir_var_htdocs)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.LOG")==0) {
				strncpy(config.server_dir_var_log, pVal, sizeof(config.server_dir_var_log)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.MAIL")==0) {
				strncpy(config.server_dir_var_mail, pVal, sizeof(config.server_dir_var_mail)-1);
			} else if (strcmp(pVar, "SERVER.LOGLEVEL")==0) {
				config.server_loglevel=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.HOSTNAME")==0) {
				strncpy(config.server_hostname, pVal, sizeof(config.server_hostname)-1);
			} else if (strcmp(pVar, "SERVER.MAXCONN")==0) {
				config.server_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.PORT")==0) {
				config.server_port=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.MAXIDLE")==0) {
				config.server_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "SQL.TYPE")==0) {
				strncpy(config.sql_type, pVal, sizeof(config.sql_type)-1);
			} else if (strcmp(pVar, "SQL.HOSTNAME")==0) {
				strncpy(config.sql_hostname, pVal, sizeof(config.sql_hostname)-1);
			} else if (strcmp(pVar, "SQL.PORT")==0) {
				config.sql_port=atoi(pVal);
			} else if (strcmp(pVar, "SQL.DBNAME")==0) {
				strncpy(config.sql_dbname, pVal, sizeof(config.sql_dbname)-1);
			} else if (strcmp(pVar, "SQL.USERNAME")==0) {
				strncpy(config.sql_username, pVal, sizeof(config.sql_username)-1);
			} else if (strcmp(pVar, "SQL.PASSWORD")==0) {
				strncpy(config.sql_password, pVal, sizeof(config.sql_password)-1);
			} else if (strcmp(pVar, "SQL.ODBC_DSN")==0) {
				strncpy(config.sql_odbc_dsn, pVal, sizeof(config.sql_odbc_dsn)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
sanity_check:
	if (config.server_maxconn==0) config.server_maxconn=50;
	if (config.server_maxidle==0) config.server_maxidle=120;
	if (config.server_maxconn<5) config.server_maxconn=5;
	if (config.server_maxconn>1000) config.server_maxconn=1000;
	if (config.server_maxidle<15) config.server_maxidle=15;
	if (strlen(config.server_hostname)==0) strncpy(config.server_hostname, "INADDR_ANY", sizeof(config.server_hostname)-1);
	config.sql_maxconn=config.server_maxconn*2;
	config.RunAsCGI=RunAsCGI;
	return 0;
}
