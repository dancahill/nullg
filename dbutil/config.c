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
#include "main.h"

typedef	void  (*CONF_CALLBACK)(char *, char *);

int config_read(char *section, void *callback)
{
	CONF_CALLBACK conf_callback=callback;
	FILE *fp=NULL;
	char configfile[200];
	char line[512];
	char *pVar;
	char *pVal;
	int i;
	short int match;

	/* try to open the config file */
	/* try the current directory first, then ../etc/ */
	if (fp==NULL) {
		snprintf(configfile, sizeof(configfile)-1, "groupware.conf");
		fp=fopen(configfile, "r");
	}
	if (fp==NULL) {
#ifdef WIN32
		snprintf(configfile, sizeof(configfile)-1, "..\\etc\\groupware.conf");
#else
		snprintf(configfile, sizeof(configfile)-1, "../etc/groupware.conf");
#endif
		fp=fopen(configfile, "r");
	}
	if (fp==NULL) {
		printf("Error opening groupware.conf\r\n");
		exit(0);
	}
	/* else if config file does exist, read it */
	match=0;
	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if ((pVar=strchr(line, '#'))!=NULL) *pVar='\0';
		pVar=line;
		pVal=strchr(line, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			match=0;
			pVar++;
			*pVal='\0';
			if (strcasecmp(pVar, section)==0) match=1;
			continue;
		}
		if (match) {
			pVar=line;
			pVal=line;
			while ((pVal[0]!='=')&&(pVal[1]!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			if ((pVal[0]=='"')&&(pVal[strlen(pVal)-1]=='"')) {
				pVal++;
				pVal[strlen(pVal)-1]='\0';
			}
			if (pVar[0]!='\0') {
				conf_callback(pVar, pVal);
			}
		}
	}
	fclose(fp);
	return 0;
}

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "base path")==0) {
		strncpy(config.dir_base, val, sizeof(config.dir_base)-1);
	} else if (strcmp(var, "bin path")==0) {
		strncpy(config.dir_bin, val, sizeof(config.dir_bin)-1);
	} else if (strcmp(var, "etc path")==0) {
		strncpy(config.dir_etc, val, sizeof(config.dir_etc)-1);
	} else if (strcmp(var, "lib path")==0) {
		strncpy(config.dir_lib, val, sizeof(config.dir_lib)-1);
	} else if (strcmp(var, "var path")==0) {
		strncpy(config.dir_var, val, sizeof(config.dir_var)-1);
	} else if (strcmp(var, "var_backup path")==0) {
		strncpy(config.dir_var_backup, val, sizeof(config.dir_var_backup)-1);
	} else if (strcmp(var, "var_cgi path")==0) {
		strncpy(config.dir_var_cgi, val, sizeof(config.dir_var_cgi)-1);
	} else if (strcmp(var, "var_db path")==0) {
		strncpy(config.dir_var_db, val, sizeof(config.dir_var_db)-1);
	} else if (strcmp(var, "var_domains path")==0) {
		strncpy(config.dir_var_domains, val, sizeof(config.dir_var_domains)-1);
	} else if (strcmp(var, "var_htdocs path")==0) {
		strncpy(config.dir_var_htdocs, val, sizeof(config.dir_var_htdocs)-1);
	} else if (strcmp(var, "var_log path")==0) {
		strncpy(config.dir_var_log, val, sizeof(config.dir_var_log)-1);
	} else if (strcmp(var, "spool path")==0) {
		strncpy(config.dir_var_spool, val, sizeof(config.dir_var_spool)-1);
	} else if (strcmp(var, "tmp path")==0) {
		strncpy(config.dir_var_tmp, val, sizeof(config.dir_var_tmp)-1);
	} else if (strcmp(var, "sql server type")==0) {
		strncpy(config.sql_type, val, sizeof(config.sql_type)-1);
	} else if (strcmp(var, "sql host name")==0) {
		strncpy(config.sql_hostname, val, sizeof(config.sql_hostname)-1);
	} else if (strcmp(var, "sql port")==0) {
		config.sql_port=atoi(val);
	} else if (strcmp(var, "sql database name")==0) {
		strncpy(config.sql_dbname, val, sizeof(config.sql_dbname)-1);
	} else if (strcmp(var, "sql user name")==0) {
		strncpy(config.sql_username, val, sizeof(config.sql_username)-1);
	} else if (strcmp(var, "sql password")==0) {
		strncpy(config.sql_password, val, sizeof(config.sql_password)-1);
	} else if (strcmp(var, "sql odbc dsn")==0) {
		strncpy(config.sql_odbc_dsn, val, sizeof(config.sql_odbc_dsn)-1);
	}
	return;
}

int conf_read()
{
	/* define default values */
	memset((char *)&config, 0, sizeof(config));

	config_read("global", conf_callback);

	if (strlen(config.dir_var_db)==0) snprintf(config.dir_var_db, sizeof(config.dir_var_db)-1, "%s/db", config.dir_var);

	return 0;
}
