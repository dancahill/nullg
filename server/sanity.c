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
#include "main.h"

static int sanity_dbcheck_table(char *tablename, char *indexname, int numfields)
{
	int sqr;

	if ((sqr=sql_queryf(NULL, "SELECT * FROM %s WHERE %s = 1", tablename, indexname))<0) {
		logerror(NULL, __FILE__, __LINE__, "ERROR: Could not access %s table!", tablename);
		exit(0);
	}
	if (sql_numfields(sqr)!=numfields) {
		logerror(NULL, __FILE__, __LINE__, "ERROR: %s has %d fields, but it should have %d!", tablename, sql_numfields(sqr), numfields);
		sql_freeresult(sqr);
		return -1;
	}
	sql_freeresult(sqr);
	return 0;
}

int sanity_dbcheck()
{
	char commandline[200];
	char file[100];
	struct stat sb;
	char msgbuffer[200];
	int i;
	int sqr;
	int checkerror=0;

//	struct timeval ttime;
//	struct timezone tzone;
//	int x, y;

#ifdef WIN32
	if (strcmp(proc.config.sql_type, "ODBC")==0) {
		snprintf(file, sizeof(file)-1, "%s/%s.mdb", proc.config.server_dir_var_db, SERVER_BASENAME);
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
	if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		snprintf(file, sizeof(file)-1, "%s/%s.db", proc.config.server_dir_var_db, SERVER_BASENAME);
		fixslashes(file);
		if ((stat(file, &sb)!=0)||(sb.st_size==0)) {
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
	while ((sqr=sql_query(NULL, "SELECT count(username) FROM gw_users"))<0) {
		sleep(5);
		i++;
		if (i>6) {
			snprintf(msgbuffer, sizeof(msgbuffer)-1, "%s responded abnormally.", proc.config.sql_type);
			logerror(NULL, __FILE__, __LINE__, "%s", msgbuffer);
			if (strcmp(proc.config.sql_type, "ODBC")==0) {
				strcat(msgbuffer, "\n\nPlease verify the integrity of your data source, ");
				strcat(msgbuffer, "and make sure your MDAC and JET drivers are up to date.\n");
			} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
				strcat(msgbuffer, "\n\nPlease verify that the MYSQL server is running and properly configured.\n");
			} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
				strcat(msgbuffer, "\n\nPlease verify that the PGSQL server is running and properly configured.\n");
			}
			strcat(msgbuffer, "\nSee error.log for more information on this error.");
			if (proc.RunAsCGI) {
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
	sql_freeresult(sqr);
	if ((sqr=sql_query(NULL, "SELECT dbversion, tax1name, tax2name, tax1percent, tax2percent FROM gw_dbinfo"))<0) {
		logerror(NULL, __FILE__, __LINE__, "Could not read dbinfo from database");
		exit(0);
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		logerror(NULL, __FILE__, __LINE__, "Missing dbinfo data");
		exit(0);
	}
	snprintf(proc.info.version, sizeof(proc.info.version)-1, "%s", sql_getvalue(sqr, 0, 0));
	snprintf(proc.info.tax1name, sizeof(proc.info.tax1name)-1, "%s", sql_getvalue(sqr, 0, 1));
	snprintf(proc.info.tax2name, sizeof(proc.info.tax2name)-1, "%s", sql_getvalue(sqr, 0, 2));
	proc.info.tax1percent=(float)atof(sql_getvalue(sqr, 0, 3));
	proc.info.tax2percent=(float)atof(sql_getvalue(sqr, 0, 4));
	sql_freeresult(sqr);
	if ((!proc.RunAsCGI)&&(strcasecmp(proc.config.sql_type, "SQLITE")!=0)) {
		if (sanity_dbcheck_table("gw_activity",		"activityid",		ACTIVITYFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_bookmarkfolders",	"folderid",		BOOKMARKFOLDERFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_bookmarks",	"bookmarkid",		BOOKMARKFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_callactions",	"callactionid",		CALLACTIONFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_calls",		"callid",		CALLFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_contacts",		"contactid",		CONTACTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_eventclosings",	"eventclosingid",	EVENTCLOSINGFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_events",		"eventid",		EVENTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_eventtypes",	"eventtypeid",		EVENTTYPEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_files",		"fileid",		FILEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_forumgroups",	"forumgroupid",		FORUMGROUPFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_forumposts",	"messageid",		FORUMPOSTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_forums",		"forumid",		FORUMFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_groups",		"groupid",		GROUPFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_mailaccounts",	"mailaccountid",	MAILACCTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_mailheaders",	"mailheaderid",		MAILHEADFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_messages",		"messageid",		MESSAGEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_notes",		"noteid",		NOTEFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_orderitems",	"orderitemid",		ORDERITEMFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_orders",		"orderid",		ORDERFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_products",		"productid",		PRODUCTFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_queries",		"queryid",		QUERYFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_tasks",		"taskid",		TASKFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_users",		"userid",		USERFIELDS)==-1) checkerror++;
		if (sanity_dbcheck_table("gw_zones",		"zoneid",		ZONEFIELDS)==-1) checkerror++;
		if (checkerror!=0) {
			logerror(NULL, __FILE__, __LINE__, "Please use dbutil to dump and restore the database");
			exit(0);
		}
	}
/*
	gettimeofday(&ttime, &tzone);
	x=ttime.tv_sec; y=ttime.tv_usec;
	for (i=0;i<100;i++) {
		if ((sqr=sql_query(NULL, "SELECT * from gw_users"))<0) exit(0);
		sql_freeresult(sqr);
	}
	gettimeofday(&ttime, &tzone);
	x=((ttime.tv_sec-x)*1000000)-y+ttime.tv_usec;
	logerror(NULL, __FILE__, __LINE__, "Query speed test finished [%s][queries=100][time=%1.3f seconds]", proc.config.sql_type, (float)x/(float)1000000);
*/
	return 0;
}
