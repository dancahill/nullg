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

int sanity_dbcheck_table(char *tablename, char *indexname, int numfields)
{
	int sqr;

	if ((sqr=sqlQueryf(NULL, "SELECT * FROM %s WHERE %s = 1", tablename, indexname))<0) {
		logerror(NULL, __FILE__, __LINE__, "ERROR: Could not access %s table!", tablename);
		exit(0);
	}
	if (sqlNumfields(sqr)!=numfields) {
		logerror(NULL, __FILE__, __LINE__, "ERROR: %s has %d fields, but it should have %d!", tablename, sqlNumfields(sqr), numfields);
		sqlFreeconnect(sqr);
		return -1;
	}
	sqlFreeconnect(sqr);
	return 0;
}

/****************************************************************************
 *	sanity_dbcheck()
 *
 *	Purpose	: Test database version, structure and integrity
 *	Args	: None
 *	Returns	: 0 on success
 *	Notes	: None
 ***************************************************************************/
int sanity_dbcheck()
{
	char commandline[200];
	char file[100];
	struct stat sb;
	char msgbuffer[200];
	int i;
	int sqr;

#ifdef WIN32
	if (strcmp(config.sql_type, "ODBC")==0) {
		snprintf(file, sizeof(file)-1, "%s/groupware.mdb", config.server_etc_dir);
		fixslashes(file);
		if (stat(file, &sb)!=0) {
#ifdef WIN32
			snprintf(commandline, sizeof(commandline)-1, "./dbutil.exe init");
#else
			snprintf(commandline, sizeof(commandline)-1, "./dbutil init");
#endif
			fixslashes(commandline);
			system(commandline);
		}
	}
#endif
	if (strcmp(config.sql_type, "SQLITE")==0) {
		snprintf(file, sizeof(file)-1, "%s/groupware.db", config.server_etc_dir);
		fixslashes(file);
		if (stat(file, &sb)!=0) {
#ifdef WIN32
			snprintf(commandline, sizeof(commandline)-1, "./dbutil.exe init");
#else
			snprintf(commandline, sizeof(commandline)-1, "./dbutil init");
#endif
			fixslashes(commandline);
			system(commandline);
		} else if (sb.st_size==0) {
#ifdef WIN32
			snprintf(commandline, sizeof(commandline)-1, "./dbutil.exe init");
#else
			snprintf(commandline, sizeof(commandline)-1, "./dbutil init");
#endif
			fixslashes(commandline);
			system(commandline);
		}

	}
	i=0;
	while ((sqr=sqlQuery(NULL, "SELECT count(username) FROM gw_users"))<0) {
		sleep(5);
		i++;
		if (i>6) {
			snprintf(msgbuffer, sizeof(msgbuffer)-1, "%s responded abnormally.", config.sql_type);
			if (strcmp(config.sql_type, "ODBC")==0) {
				strcat(msgbuffer, "\n\nPlease verify the integrity of your data source, ");
				strcat(msgbuffer, "and make sure your MDAC and JET drivers are up to date.\n");
			} else if (strcmp(config.sql_type, "MYSQL")==0) {
				strcat(msgbuffer, "\n\nPlease verify that the MYSQL server is running and properly configured.\n");
			} else if (strcmp(config.sql_type, "PGSQL")==0) {
				strcat(msgbuffer, "\n\nPlease verify that the PGSQL server is running and properly configured.\n");
			}
			strcat(msgbuffer, "\nSee error.log for more information on this error.");
			if (RunAsCGI) {
				printf("Content-type: text/html\n\n");
				printf("<HTML><CENTER>\r\n%s\r\n</CENTER></HTML>\n", msgbuffer);
				closeconnect(0, 1);
			} else {
#ifdef WIN32
				MessageBox(0, msgbuffer, APPTITLE, MB_ICONERROR);
#else
				printf("\r\n%s\r\n", msgbuffer);
#endif
			}
			exit(0);
		}
	}
	sqlFreeconnect(sqr);
	if ((sqr=sqlQuery(NULL, "SELECT dbversion, tax1name, tax2name, tax1percent, tax2percent FROM gw_dbinfo"))<0) {
		logerror(NULL, __FILE__, __LINE__, "Could not read dbinfo from database");
		exit(0);
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		logerror(NULL, __FILE__, __LINE__, "Missing dbinfo data");
		exit(0);
	}
	snprintf(config.info.version, sizeof(config.info.version)-1, "%s", sqlGetvalue(sqr, 0, 0));
	snprintf(config.info.tax1name, sizeof(config.info.tax1name)-1, "%s", sqlGetvalue(sqr, 0, 1));
	snprintf(config.info.tax2name, sizeof(config.info.tax2name)-1, "%s", sqlGetvalue(sqr, 0, 2));
	config.info.tax1percent=(float)atof(sqlGetvalue(sqr, 0, 3));
	config.info.tax2percent=(float)atof(sqlGetvalue(sqr, 0, 4));
	sqlFreeconnect(sqr);
	if (!RunAsCGI) {
/*		if (sanity_dbcheck_table("gw_bookmarkfolders",	"folderid",		BOOKMARKFOLDERFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_bookmarks",	"bookmarkid",		BOOKMARKFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_calls",		"callid",		CALLFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_contacts",		"contactid",		CONTACTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_events",		"eventid",		EVENTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_files",		"fileid",		FILEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_forums",		"forumid",		FORUMFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_forumposts",	"messageid",		FORUMPOSTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_groups",		"groupid",		GROUPFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_mailaccounts",	"mailaccountid",	MAILACCTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_mailheaders",	"mailheaderid",		MAILHEADFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_messages",		"messageid",		MESSAGEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_notes",		"noteid",		NOTEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_orders",		"orderid",		ORDERFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_orderitems",	"orderitemid",		ORDERITEMFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_products",		"productid",		PRODUCTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_queries",		"queryid",		QUERYFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_tasks",		"taskid",		TASKFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_users",		"userid",		USERFIELDS)==-1) checkerror++;
		if (checkerror!=0) exit(0);
*/
	}
	return 0;
}
