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
	snprintf(config.server_cgi_dir, sizeof(config.server_cgi_dir)-1, "%s/cgi-bin", config.server_base_dir);
	snprintf(config.server_etc_dir, sizeof(config.server_etc_dir)-1, "%s/etc", config.server_base_dir);
	snprintf(config.server_file_dir, sizeof(config.server_file_dir)-1, "%s/files", config.server_base_dir);
	snprintf(config.server_htdocs_dir, sizeof(config.server_htdocs_dir)-1, "%s/htdocs", config.server_base_dir);
	fixslashes(config.server_base_dir);
	fixslashes(config.server_bin_dir);
	fixslashes(config.server_cgi_dir);
	fixslashes(config.server_etc_dir);
	fixslashes(config.server_file_dir);
	fixslashes(config.server_htdocs_dir);
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
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_etc_dir);
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "r");
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		if (stat(config.server_etc_dir, &sb)!=0) {
			logerror(NULL, __FILE__, __LINE__, ADM_CFG_NODIR, config.server_etc_dir);
			return -1;
		};
		printf("\r\n%s", ADM_CFG_MAKE);
		logaccess(NULL, 1, "%s", ADM_CFG_MAKE);
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_etc_dir);
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "w");
		if (fp==NULL) {
			return -1;
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
			if (strcmp(pVar, "SERVER_BASE_DIR")==0) {
				strncpy(config.server_base_dir, pVal, sizeof(config.server_base_dir)-1);
			} else if (strcmp(pVar, "SERVER_BIN_DIR")==0) {
				strncpy(config.server_bin_dir, pVal, sizeof(config.server_bin_dir)-1);
			} else if (strcmp(pVar, "SERVER_CGI_DIR")==0) {
				strncpy(config.server_cgi_dir, pVal, sizeof(config.server_cgi_dir)-1);
			} else if (strcmp(pVar, "SERVER_ETC_DIR")==0) {
				strncpy(config.server_etc_dir, pVal, sizeof(config.server_etc_dir)-1);
			} else if (strcmp(pVar, "SERVER_FILE_DIR")==0) {
				strncpy(config.server_file_dir, pVal, sizeof(config.server_file_dir)-1);
			} else if (strcmp(pVar, "SERVER_HTDOCS_DIR")==0) {
				strncpy(config.server_htdocs_dir, pVal, sizeof(config.server_htdocs_dir)-1);
			} else if (strcmp(pVar, "SERVER_LOGLEVEL")==0) {
				config.server_loglevel=atoi(pVal);
			} else if (strcmp(pVar, "SERVER_HOSTNAME")==0) {
				strncpy(config.server_hostname, pVal, sizeof(config.server_hostname)-1);
			} else if (strcmp(pVar, "SERVER_MAXCONN")==0) {
				config.server_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "SERVER_PORT")==0) {
				config.server_port=atoi(pVal);
			} else if (strcmp(pVar, "SERVER_MAXIDLE")==0) {
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
sanity_check:
	if (config.server_maxconn==0) config.server_maxconn=50;
	if (config.server_maxidle==0) config.server_maxidle=120;
	if (config.server_maxconn<5) config.server_maxconn=5;
	if (config.server_maxconn>1000) config.server_maxconn=1000;
	if (config.server_maxidle<15) config.server_maxidle=15;
	if (strlen(config.server_hostname)==0) strncpy(config.server_hostname, "INADDR_ANY", sizeof(config.server_hostname)-1);
	config.sql_maxconn=config.server_maxconn*2;
	return 0;
}
