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

int configread()
{
	FILE *fp=NULL;
	char configfile[200];
	char line[512];
	char *pVar;
	char *pVal;

	/* define default values */
	memset((char *)&sql, 0, sizeof(sql));
	/* try to open the config file */
	/* try the current directory first, then ../etc/ */
	if (fp==NULL) {
		snprintf(configfile, sizeof(configfile)-1, "sentinel.cfg");
		fp=fopen(configfile, "r");
	}
	if (fp==NULL) {
#ifdef WIN32
		snprintf(configfile, sizeof(configfile)-1, "..\\etc\\sentinel.cfg");
#else
		snprintf(configfile, sizeof(configfile)-1, "../etc/sentinel.cfg");
#endif
		fp=fopen(configfile, "r");
	}
	if (fp==NULL) {
		printf("missing config file\r\n");
		exit(0);
	}
	/* if config file does exist, read it */
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
			if (strcmp(pVar, "SQL_TYPE")==0) {
				strncpy(sql.sql_type, pVal, sizeof(sql.sql_type)-1);
			} else if (strcmp(pVar, "SQL_HOSTNAME")==0) {
				strncpy(sql.sql_hostname, pVal, sizeof(sql.sql_hostname)-1);
			} else if (strcmp(pVar, "SQL_PORT")==0) {
				strncpy(sql.sql_port, pVal, sizeof(sql.sql_port)-1);
			} else if (strcmp(pVar, "SQL_DBNAME")==0) {
				strncpy(sql.sql_dbname, pVal, sizeof(sql.sql_dbname)-1);
			} else if (strcmp(pVar, "SQL_USERNAME")==0) {
				strncpy(sql.sql_username, pVal, sizeof(sql.sql_username)-1);
			} else if (strcmp(pVar, "SQL_PASSWORD")==0) {
				strncpy(sql.sql_password, pVal, sizeof(sql.sql_password)-1);
			} else if (strcmp(pVar, "SQL_ODBC_DSN")==0) {
				strncpy(sql.sql_odbc_dsn, pVal, sizeof(sql.sql_odbc_dsn)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	return 0;
}
