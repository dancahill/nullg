/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp strnicmp
#else
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include "nullgw/common/defines.h"

#define LINELEN 128
#define CONFLEN 512

char *rawconfig[CONFLEN];
char basedir[1024];
char filename[256];
char tmpline[LINELEN];
int linecount;

void striprn(char *string)
{
	int i=strlen(string)-1;

	while (1) {
		if (i<0) break;
		if (string[i]=='\r') { string[i]='\0'; i--; continue; }
		if (string[i]=='\n') { string[i]='\0'; i--; continue; }
		break;
	};
}

int newfile()
{
	FILE *fp=NULL;

	if ((fp=fopen(filename, "w"))==NULL) {
		printf("Error opening %s\r\n", filename);
		return -1;
	}
	printf("Writing %s\r\n", filename);
	fprintf(fp, "# This file contains system settings for NullLogic Groupware.\n\n");
	fprintf(fp, "[global]\n");
	fprintf(fp, "   uid               = nullgw\n");
	fprintf(fp, "   gid               = nullgw\n");
#ifndef WIN32
	fprintf(fp, "   umask             = 007\n");
#endif
	fprintf(fp, "   log level         = 1\n");
	fprintf(fp, "   default language  = en\n");
	fprintf(fp, "   host name         = \"localhost\"\n");
	fprintf(fp, "   bin path          = \"%s/bin\"\n", basedir);
	fprintf(fp, "   etc path          = \"%s/etc\"\n", basedir);
	fprintf(fp, "   lib path          = \"%s/lib\"\n", basedir);
	fprintf(fp, "   var path          = \"%s/var\"\n", basedir);
	fprintf(fp, "   sql server type   = \"SQLITE\"\n");
	fprintf(fp, ";  ssl cert file     = \"%s/etc/ssl-cert.pem\"\n", basedir);
	fprintf(fp, ";  ssl key file      = \"%s/etc/ssl-priv.pem\"\n", basedir);
	fprintf(fp, "\n");
	fprintf(fp, "   load module       = ghttpd\n");
	fprintf(fp, ";  load module       = httpd\n");
	fprintf(fp, "   load module       = pop3d\n");
	fprintf(fp, "   load module       = smtpd\n");
	fprintf(fp, "   load module       = smtpq\n");
	fprintf(fp, "\n");
	fprintf(fp, "[ghttpd]\n");
	fprintf(fp, "   interface         = INADDR_ANY\n");
	fprintf(fp, "   port              = %d\n", GHTTP_PORT);
	fprintf(fp, "   ssl port          = %d\n", GHTTPS_PORT);
	fprintf(fp, "   max connections   = 50\n");
	fprintf(fp, "   max keepalive     = 15\n");
	fprintf(fp, "   max idle          = 120\n");
	fprintf(fp, "   max post size     = 33554432\n");
	fprintf(fp, "   session limit     = 1\n");
	fprintf(fp, "\n");
	fprintf(fp, "   load module       = mod_html\n");
	fprintf(fp, "   load module       = mod_admin\n");
	fprintf(fp, "   load module       = mod_bookmarks\n");
	fprintf(fp, "   load module       = mod_calendar\n");
	fprintf(fp, "   load module       = mod_calls\n");
	fprintf(fp, "   load module       = mod_cgi\n");
	fprintf(fp, "   load module       = mod_contacts\n");
	fprintf(fp, "   load module       = mod_email\n");
	fprintf(fp, "   load module       = mod_files\n");
	fprintf(fp, ";  load module       = mod_finance\n");
	fprintf(fp, "   load module       = mod_forums\n");
	fprintf(fp, "   load module       = mod_messages\n");
	fprintf(fp, "   load module       = mod_notes\n");
	fprintf(fp, "   load module       = mod_profile\n");
	fprintf(fp, "   load module       = mod_projects\n");
	fprintf(fp, "   load module       = mod_searches\n");
	fprintf(fp, ";  load module       = mod_spellcheck\n");
	fprintf(fp, "   load module       = mod_tasks\n");
	fprintf(fp, "   load module       = mod_xmlrpc\n");
	fprintf(fp, "\n");
	fprintf(fp, "[httpd]\n");
	fprintf(fp, "   interface         = INADDR_ANY\n");
	fprintf(fp, "   port              = %d\n", HTTP_PORT);
	fprintf(fp, ";  ssl port          = %d\n", HTTPS_PORT);
	fprintf(fp, "   max connections   = 50\n");
	fprintf(fp, "   max keepalive     = 15\n");
	fprintf(fp, "   max idle          = 120\n");
	fprintf(fp, "   max post size     = 33554432\n");
	fprintf(fp, "\n");
	fprintf(fp, "   load module       = mod_cgi\n");
	fprintf(fp, ";  load module       = mod_php\n");
	fprintf(fp, "\n");
	fprintf(fp, "[pop3d]\n");
	fprintf(fp, "   interface         = localhost\n");
	fprintf(fp, "   port              = %d\n", POP3_PORT);
	fprintf(fp, ";  ssl port          = %d\n", POP3S_PORT);
	fprintf(fp, "   max connections   = 50\n");
	fprintf(fp, "   max idle          = 120\n");
	fprintf(fp, "\n");
	fprintf(fp, "[smtpd]\n");
	fprintf(fp, "   interface         = localhost\n");
	fprintf(fp, ";  relay host        = \"\"\n");
	fprintf(fp, "   port              = %d\n", SMTP_PORT);
	fprintf(fp, ";  ssl port          = %d\n", SMTPS_PORT);
	fprintf(fp, "   max connections   = 50\n");
	fprintf(fp, "   max idle          = 120\n");
	fprintf(fp, "   retry delay       = 300\n");
	fprintf(fp, ";  filter program    = \"%s/bin/nullgw-scansmtp.sh\"\n", basedir);
	fprintf(fp, "\n");
	fprintf(fp, "[mod_email]\n");
	fprintf(fp, ";  filter program    = \"%s/bin/nullgw-scanmail.sh\"\n", basedir);
	fprintf(fp, "   max idle          = 120\n");
	fprintf(fp, "\n");
	fprintf(fp, "[mod_files]\n");
	fprintf(fp, ";  filter program    = \"%s/bin/nullgw-scanfile.sh\"\n", basedir);
	fclose(fp);
	return 0;
}

int loadfile()
{
	FILE *fp=NULL;
	char *ptemp;

	if ((fp=fopen(filename, "r"))==NULL) return -1;
	linecount=0;
	ptemp=calloc(LINELEN, sizeof(char));
	while (fgets(ptemp, LINELEN-1, fp)!=NULL) {
		striprn(ptemp);
		rawconfig[linecount]=ptemp;
		linecount++;
		ptemp=calloc(LINELEN, sizeof(char));
	}
	fclose(fp);
	free(ptemp);
	return 0;
}

int swapvar(char *sec, char *var, char *val)
{
	char section[32];
	char *pVar;
	char *pVal;
	int line;

	memset(section, 0, sizeof(section));
	snprintf(section, sizeof(section)-1, "global");
	for (line=0;line<linecount;line++) {
		strncpy(tmpline, rawconfig[line], sizeof(tmpline)-1);
		if ((pVar=strchr(tmpline, '#'))!=NULL) *pVar='\0';
		pVar=tmpline;
		pVal=strchr(tmpline, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			pVar++;
			*pVal='\0';
			snprintf(section, sizeof(section)-1, "%s", pVar);
			continue;
		}
		pVar=tmpline;
		pVal=tmpline;
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
			if (strcasecmp(section, sec)==0) {
				if (strcasecmp(pVar, var)==0) {
					snprintf(rawconfig[line], LINELEN-1, "   %-17s = %s", pVar, val);
					return 0;
				}
			}
		}
	}
	return -1;
}

int insertvar(char *sec, char *var, char *val)
{
	char section[32];
	char *pVar;
	char *pVal;
	int line;
	int i;

	memset(section, 0, sizeof(section));
	snprintf(section, sizeof(section)-1, "global");
	for (line=0;line<linecount;line++) {
		strncpy(tmpline, rawconfig[line], sizeof(tmpline)-1);
		if ((pVar=strchr(tmpline, '#'))!=NULL) *pVar='\0';
		pVar=tmpline;
		pVal=strchr(tmpline, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			if (strcasecmp(section, sec)==0) {
				if (linecount<CONFLEN) linecount++;
				for (i=linecount-1;i>line;i--) {
					rawconfig[i]=rawconfig[i-1];
				}
				for (;line>0;line--) {
					if (strlen(rawconfig[line-1])>0) break;
					rawconfig[line]=rawconfig[line-1];
				}
				rawconfig[line]=calloc(LINELEN, sizeof(char));
				snprintf(rawconfig[line], LINELEN-1, "   %-17s = %s", var, val);
				return 0;
			}
			pVar++;
			*pVal='\0';
			snprintf(section, sizeof(section)-1, "%s", pVar);
			continue;
		}
	}
	return -1;
}

char *getvar(char *sec, char *var)
{
	char section[32];
	char *pVar;
	char *pVal;
	int line;

	memset(section, 0, sizeof(section));
	snprintf(section, sizeof(section)-1, "global");
	for (line=0;line<linecount;line++) {
		strncpy(tmpline, rawconfig[line], sizeof(tmpline)-1);
		if ((pVar=strchr(tmpline, '#'))!=NULL) *pVar='\0';
		pVar=tmpline;
		pVal=strchr(tmpline, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			pVar++;
			*pVal='\0';
			snprintf(section, sizeof(section)-1, "%s", pVar);
			continue;
		}
		pVar=tmpline;
		pVal=tmpline;
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
			if (strcasecmp(section, sec)==0) {
				if (strcasecmp(pVar, var)==0) {
					return pVal;
				}
			}
		}
	}
	return "";
}

int savefile()
{
	FILE *fp=NULL;
	int line=0;

	if ((fp=fopen(filename, "w"))==NULL) {
		printf("Error saving %s\r\n", filename);
		return -1;
	}
	for (line=0;line<linecount;line++) {
		fprintf(fp, "%s\n", rawconfig[line]);
	}
	fclose(fp);
	return 0;
}


int swaporinsert(char *sec, char *var, char *val)
{
	if (swapvar(sec, var, val)<0) {
		if (insertvar(sec, var, val)<0) {
			return -1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char line[40];
	int i;

	char sql_type[64];
	char sql_host[64];
	char sql_db[64];
	char sql_user[64];
	char sql_pass[64];
	int sql_port;

	memset(basedir, 0, sizeof(basedir));
	memset(filename, 0, sizeof(filename));
	memset(rawconfig, 0, sizeof(rawconfig));
	memset(sql_type, 0, sizeof(sql_type));
	memset(sql_host, 0, sizeof(sql_host));
	memset(sql_db,   0, sizeof(sql_db));
	memset(sql_user, 0, sizeof(sql_user));
	memset(sql_pass, 0, sizeof(sql_pass));
#ifdef WIN32
	GetCurrentDirectory(sizeof(basedir)-1, basedir);
#else
	if (getcwd(basedir, sizeof(basedir)-1)==NULL) return -1;
#endif
	i=strlen(basedir)-1;
	while (1) {
		if (i<0) break;
		if ((basedir[i]!='\\')&&(basedir[i]!='/')) { basedir[i]='\0'; i--; continue; }
		break;
	}
	while (1) {
		if (i<0) break;
		if ((basedir[i]=='\\')||(basedir[i]=='/')) { basedir[i]='\0'; i--; continue; }
		break;
	};
	if ((argc==4)&&(strcasecmp(argv[1], "show")==0)) {
		snprintf(filename, sizeof(filename)-1, "groupware.conf");
		if (loadfile()<0) {
			snprintf(filename, sizeof(filename)-1, "../etc/groupware.conf");
#ifndef WIN32
			if (loadfile()<0) {
				snprintf(filename, sizeof(filename)-1, ETCDIR "/groupware.conf");
#endif
				if (loadfile()<0) {
				}
#ifndef WIN32
			}
#endif
		}
		printf("%s", getvar(argv[2], argv[3]));
		return 0;
	}
	if ((argc==2)&&(strcasecmp(argv[1], "upgrade")==0)) {
		return 0;
	}
/*	snprintf(filename, sizeof(filename)-1, "%s/etc/groupware.conf", basedir); */
	snprintf(filename, sizeof(filename)-1, "groupware.conf");
	if (loadfile()<0) {
		snprintf(filename, sizeof(filename)-1, "../etc/groupware.conf");
		if (loadfile()<0) {
#ifndef WIN32
			snprintf(filename, sizeof(filename)-1, "%s/groupware.conf", ETCDIR);
			if (loadfile()<0) {
#endif
				if (newfile()<0) {
					printf("can't read or create conf file\n");
					return -1;
				} else {
					loadfile();
				}
#ifndef WIN32
			}
#endif
		}
	}
	printf("NullLogic Groupware Configuration\r\n");
	printf("=================================\r\n");
	snprintf(sql_type, sizeof(sql_type)-1, "%s", getvar("global", "sql server type"));
	if (strlen(sql_type)==0) snprintf(sql_type, sizeof(sql_type)-1, "SQLITE");
	printf("Will you be using SQLITE, MYSQL or PGSQL [%s]: ", sql_type);
	fgets(line, sizeof(line)-1, stdin);
	striprn(line);
	if (strlen(line)>0) snprintf(sql_type, sizeof(sql_type)-1, "%s", line);
	if (swaporinsert("global", "sql server type", sql_type)<0) {
		printf("couldn't insert [%s] [%s] [%s]\n", "global", "sql server type", sql_type);
	}
	if (strcasecmp(sql_type, "FBSQL")==0) {
		snprintf(sql_host, sizeof(sql_host)-1, "%s", getvar("global", "sql host name"));
		if (strlen(sql_host)==0) snprintf(sql_host, sizeof(sql_host)-1, "localhost");
		sql_port=atoi(getvar("global", "sql port"));
		if (sql_port==0) sql_port=3050;
		snprintf(sql_db,   sizeof(sql_db)-1, "%s", getvar("global", "sql database name"));
		if (strlen(sql_db)==0) snprintf(sql_db,   sizeof(sql_db)-1,   "/var/lib/nullgw/db/nullgw.fdb");
		snprintf(sql_user, sizeof(sql_user)-1, "%s", getvar("global", "sql user name"));
		if (strlen(sql_user)==0) snprintf(sql_user, sizeof(sql_user)-1, "sysdba");
		snprintf(sql_pass, sizeof(sql_pass)-1, "%s", getvar("global", "sql password"));
		if (strlen(sql_pass)==0) snprintf(sql_pass, sizeof(sql_pass)-1, "masterkey");
	} else if (strcasecmp(sql_type, "MYSQL")==0) {
		snprintf(sql_host, sizeof(sql_host)-1, "%s", getvar("global", "sql host name"));
		if (strlen(sql_host)==0) snprintf(sql_host, sizeof(sql_host)-1, "localhost");
		sql_port=atoi(getvar("global", "sql port"));
		if (sql_port==0) sql_port=3306;
		snprintf(sql_db,   sizeof(sql_db)-1, "%s", getvar("global", "sql database name"));
		if (strlen(sql_db)==0) snprintf(sql_db,   sizeof(sql_db)-1,   "nullgroupware");
		snprintf(sql_user, sizeof(sql_user)-1, "%s", getvar("global", "sql user name"));
		if (strlen(sql_user)==0) snprintf(sql_user, sizeof(sql_user)-1, "nobody");
		snprintf(sql_pass, sizeof(sql_pass)-1, "%s", getvar("global", "sql password"));
		if (strlen(sql_pass)==0) snprintf(sql_pass, sizeof(sql_pass)-1, "password");
	} else if (strcasecmp(sql_type, "PGSQL")==0) {
		snprintf(sql_host, sizeof(sql_host)-1, "%s", getvar("global", "sql host name"));
		if (strlen(sql_host)==0) snprintf(sql_host, sizeof(sql_host)-1, "localhost");
		sql_port=atoi(getvar("global", "sql port"));
		if (sql_port==0) sql_port=5432;
		snprintf(sql_db,   sizeof(sql_db)-1, "%s", getvar("global", "sql database name"));
		if (strlen(sql_db)==0) snprintf(sql_db,   sizeof(sql_db)-1,   "nullgroupware");
		snprintf(sql_user, sizeof(sql_user)-1, "%s", getvar("global", "sql user name"));
		if (strlen(sql_user)==0) snprintf(sql_user, sizeof(sql_user)-1, "nobody");
		snprintf(sql_pass, sizeof(sql_pass)-1, "%s", getvar("global", "sql password"));
		if (strlen(sql_pass)==0) snprintf(sql_pass, sizeof(sql_pass)-1, "password");
	} else if (strcasecmp(sql_type, "SQLITE")==0) {
		sql_port=0;
	} else {
		printf("'%s' is an unknown SQL server type.\r\n", sql_type);
		return -1;
	}
	if ((strcasecmp(sql_type, "FBSQL")==0)||(strcasecmp(sql_type, "MYSQL")==0)||(strcasecmp(sql_type, "PGSQL")==0)) {
		printf("What is the host name of the %s server? [%s]: ", sql_type, sql_host);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_host, sizeof(sql_host)-1, "%s", line);
		swaporinsert("global", "sql host name", sql_host);
		printf("What TCP/IP port is the %s server listening on? [%d]: ", sql_type, sql_port);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) sql_port=atoi(line);
		snprintf(line, sizeof(line)-1, "%d", sql_port);
		swaporinsert("global", "sql port", line);
		printf("What is the name of the database NullLogic Groupware should use? [%s]: ", sql_db);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_db, sizeof(sql_db)-1, "%s", line);
		swaporinsert("global", "sql database name", sql_db);
		printf("What username should NullLogic Groupware use to access the database? [%s]: ", sql_user);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_user, sizeof(sql_user)-1, "%s", line);
		swaporinsert("global", "sql user name", sql_user);
		printf("What is the password for the '%s' account? [%s]: ", sql_user, sql_pass);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_pass, sizeof(sql_pass)-1, "%s", line);
		swaporinsert("global", "sql password", sql_pass);
	}
	if (strcasecmp(sql_type, "FBSQL")==0) {
		printf("\r\n");
		printf("In order to complete the install, you will need to manually create the\r\n");
		printf("SQL database %s as follows.\r\n", sql_db);
		printf("isql-fb\r\n");
		printf("SQL>CREATE DATABASE '%s'\r\n", sql_db);
		printf("CON>user '%s' password '%s';\r\n", sql_user, sql_pass);
		printf("\r\n");
		printf("After creating the database, please run nullgw-dbutil to\r\n");
		printf("initialise the database.\r\n");
		printf("\r\n");
	} else if (strcasecmp(sql_type, "MYSQL")==0) {
		printf("\r\n");
		printf("In order to complete the install, you will need to manually create the\r\n");
		printf("SQL database %s as follows.\r\n", sql_db);
		printf("  mysqladmin create %s\r\n", sql_db);
		printf("\r\n");
		printf("After creating the database, please run nullgw-dbutil to\r\n");
		printf("initialise the database.\r\n");
		printf("\r\n");
	} else if (strcasecmp(sql_type, "PGSQL")==0) {
		printf("\r\n");
		printf("In order to complete the install, you will need to manually create the\r\n");
		printf("SQL database %s as follows.\r\n", sql_db);
		printf("  createdb %s\r\n", sql_db);
		printf("\r\n");
		printf("After creating the database, please run nullgw-dbutil to\r\n");
		printf("initialise the database.\r\n");
		printf("\r\n");
	}
	if (savefile()<0) return -1;
	return 0;
}
