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
#include "smtp_main.h"

static int sanity_dircheck(const char *format, ...)
{
	struct stat sb;
	char dirname[512];
	va_list ap;

	memset(dirname, 0, sizeof(dirname));
	va_start(ap, format);
	vsnprintf(dirname, sizeof(dirname)-1, format, ap);
	va_end(ap);
	fixslashes(dirname);
	if (stat(dirname, &sb)==0) return 0;
#ifdef WIN32
	if (mkdir(dirname)!=0) {
#else
	if (mkdir(dirname, 0700)!=0) {
#endif
		logerror(NULL, __FILE__, __LINE__, "Error accessing directory '%s'", dirname);
		return -1;
	}
	return 0;
}

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
	char msgbuffer[200];
	int i;
	int sqr;
	int checkerror=0;

	if (sanity_dircheck("%s", proc.config.server_dir_var)!=0) exit(0);
	if (sanity_dircheck("%s", proc.config.server_dir_var_db)!=0) exit(0);
	if (sanity_dircheck("%s", proc.config.server_dir_var_files)!=0) exit(0);
	if (sanity_dircheck("%s", proc.config.server_dir_var_log)!=0) exit(0);
	if (sanity_dircheck("%s", proc.config.server_dir_var_mail)!=0) exit(0);
	if (sanity_dircheck("%s/local", proc.config.server_dir_var_mail)!=0) exit(0);
	if (sanity_dircheck("%s/queue", proc.config.server_dir_var_mail)!=0) exit(0);
	if (sanity_dircheck("%s", proc.config.server_dir_var_tmp)!=0) exit(0);
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
				printf("\r\n%s\r\n", msgbuffer);
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
		if (sanity_dbcheck_table("gw_mailfolders",	"mailfolderid",		MAILFOLDERFIELDS)==-1) checkerror++;
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
	return 0;
}
