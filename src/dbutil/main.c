/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#include "nullsd/config-nt.h"
#else
#include "nullsd/config.h"
#endif
#include <stdio.h>
#include <time.h>
#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <ctype.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/stat.h>
#endif
#include "nullsd/defines.h"
#include "nsp/nsp.h"
#include "nsd.h"

#define SQLBUFSIZE 32768

obj_t *sql_schema=NULL;
obj_t *sql_default_data=NULL;
char sql_type[32];
char rootpass[40];
int loglevel;

int sqlfprintf(nsp_state *N, FILE *fp, const char *format, ...)
{
	char *buffer=calloc(SQLBUFSIZE, sizeof(char));
	int offset=0;
	va_list ap;

	if (buffer==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(buffer, SQLBUFSIZE-1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if (buffer[offset]==13) {
/*			fprintf(fp, "\\r"); */
		} else if (buffer[offset]==10) {
			fprintf(fp, "\\n");
		} else if (buffer[offset]=='\'') {
			fprintf(fp, "''");
		} else {
			fprintf(fp, "%c", buffer[offset]);
		}
		offset++;
	}
	free(buffer);
	return 0;
}

int sequence_sync(nsp_state *N)
{
	obj_t *qobj=NULL;
	obj_t *table_schema;
	int max;

	if (strcasecmp(sql_type, "FBSQL")==0) {
		//printf("fbsql_generator_sync started;\r\n");
		for (table_schema=sql_schema->val->d.table.f;table_schema;table_schema=table_schema->next) {
			char *tablename=nsp_getstr(N, table_schema, "name");
			char *indexname=nsp_getstr(N, table_schema, "index");
			char *sequencename=nsp_getstr(N, table_schema, "sequencename");

			if (!nsp_istable(table_schema)) continue;
			if (strcmp(sequencename, "null")==0) continue;
			if (_sql_queryf(N, &qobj, "SELECT MAX(%s) FROM %s", indexname, tablename)<0) return -1;
			if (sql_numtuples(N, &qobj)!=1) {
				_sql_freeresult(N, &qobj);
				printf("sql error;\r\n");
				return -1;
			}
			max=atoi(sql_getvalue(N, &qobj, 0, 0));
			_sql_freeresult(N, &qobj);
			_sql_updatef(N, "SET GENERATOR gen_%s TO %d;", tablename, max);
		}
		//printf("fbsql_generator_sync done;\r\n");
	} else if (strcasecmp(sql_type, "PGSQL")==0) {
		//for (i=0;;i++) {
		//	if (pgsqldb_tables[i].seqname==NULL) break;
		//	if (_sql_queryf(N, &qobj, "SELECT max(%s) FROM %s", pgsqldb_tables[i].index, pgsqldb_tables[i].name)<0) return -1;
		//	if (sql_numtuples(N, &qobj)!=1) {
		//		_sql_freeresult(N, &qobj);
		//		return -1;
		//	}
		//	max=atoi(sql_getvalue(N, &qobj, 0, 0))+1;
		//	_sql_freeresult(N, &qobj);
		//	if (_sql_queryf(N, &qobj, "SELECT last_value FROM %s", pgsqldb_tables[i].seqname)<0) return -1;
		//	if (sql_numtuples(N, &qobj)!=1) {
		//		_sql_freeresult(N, &qobj);
		//		return -1;
		//	}
		//	seq=atoi(sql_getvalue(N, &qobj, 0, 0));
		//	_sql_freeresult(N, &qobj);
		//	if (seq<max) seq=max;
		//	if (_sql_queryf(N, &qobj, "SELECT setval ('\"%s\"', %d, false);", pgsqldb_tables[i].seqname, seq)<0) return -1;
		//	if (sql_numtuples(N, &qobj)!=1) {
		//		_sql_freeresult(N, &qobj);
		//		return -1;
		//	}
		//	_sql_freeresult(N, &qobj);
		//}
	}
	return 0;
}

int table_check(nsp_state *N)
{
	obj_t *qobj=NULL;
	//int i;
	int x;

	/* CHECK gw_dbinfo TABLE */
	if (_sql_query(N, &qobj, "SELECT count(*) FROM gw_dbinfo")<0) return -1;
	x=atoi(sql_getvalue(N, &qobj, 0, 0));
	_sql_freeresult(N, &qobj);
	if (x==0) {
//		for (i=0;sqldata_new[i]!=NULL;i++) {
//			if (_sql_update(N, sqldata_new[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_new[i]); return -1; }
//		}
	}
	/* CHECK gw_calls_actions TABLE */
	if (_sql_query(N, &qobj, "SELECT count(*) FROM gw_calls_actions")<0) return -1;
	x=atoi(sql_getvalue(N, &qobj, 0, 0));
	_sql_freeresult(N, &qobj);
	if (x==0) {
//		for (i=0;sqldata_callactions[i]!=NULL;i++) {
//			if (_sql_update(N, sqldata_callactions[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_callactions[i]); return -1; }
//		}
	}
	/* CHECK gw_domains TABLE */
	if (_sql_query(N, &qobj, "SELECT count(*) FROM gw_domains")<0) return -1;
	x=atoi(sql_getvalue(N, &qobj, 0, 0));
	_sql_freeresult(N, &qobj);
	if (x==0) {
//		for (i=0;sqldata_domains[i]!=NULL;i++) {
//			if (_sql_update(N, sqldata_domains[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_domains[i]); return -1; }
//		}
	}
	/* CHECK gw_eventclosings TABLE */
	if (_sql_query(N, &qobj, "SELECT count(*) FROM gw_events_closings")<0) return -1;
	x=atoi(sql_getvalue(N, &qobj, 0, 0));
	_sql_freeresult(N, &qobj);
	if (x==0) {
//		for (i=0;sqldata_eventclosings[i]!=NULL;i++) {
//			if (_sql_update(N, sqldata_eventclosings[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventclosings[i]); return -1; }
//		}
	}
	/* CHECK gw_eventtypes TABLE */
	if (_sql_query(N, &qobj, "SELECT count(*) FROM gw_events_types")<0) return -1;
	x=atoi(sql_getvalue(N, &qobj, 0, 0));
	_sql_freeresult(N, &qobj);
	if (x==0) {
//		for (i=0;sqldata_eventtypes[i]!=NULL;i++) {
//			if (_sql_update(N, sqldata_eventtypes[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventtypes[i]); return -1; }
//		}
	}
	/* Assign all orphaned records to domain 1 */
	//_sql_updatef(N, "UPDATE gw_users SET obj_did = 1, domainid = 1 WHERE domainid = 0");
	//_sql_updatef(N, "UPDATE gw_groups SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_contacts SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_activity SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_bookmarks SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_bookmarks_folders SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_calls SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_email_accounts SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_email_folders SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_email_headers SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_events SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_files SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_finance_invoices SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_notes SET obj_did = 1 WHERE obj_did = 0");
	//_sql_updatef(N, "UPDATE gw_tasks SET obj_did = 1 WHERE obj_did = 0");
	/* Update the db version */
	_sql_updatef(N, "UPDATE gw_dbinfo SET dbversion = '%s'", PACKAGE_VERSION);
	return 0;
}

int table_init(nsp_state *N, obj_t *table_schema)
{
	char query[8192];
	char *tablename=nsp_getstr(N, table_schema, "name");
	char *indexname=nsp_getstr(N, table_schema, "index");
	char *sequencename=nsp_getstr(N, table_schema, "sequencename");
	char *primarykey=nsp_getstr(N, table_schema, "primarykey");
	int line=0;
	obj_t *field;
	char *qo=query;
	int bl=sizeof(query)-1;
	int l;

//	printf("\r\nname=[%s]\r\n", tablename);
//	printf("indexname=[%s]\r\n", indexname);
//	printf("sequencename=[%s]\r\n", sequencename);
//	printf("primarykey=[%s]\r\n", primarykey);
	l=snprintf(qo, bl, "CREATE TABLE %s (\r\n\t", tablename);
	qo+=l;
	bl-=l;
	for (field=nsp_getobj(N, table_schema, "fields")->val->d.table.f;field;field=field->next) {
		//obj_t *attribute;
		if (!nsp_istable(field)) continue;
		char *name=nsp_getstr(N, field, "0");
		char *type=nsp_getstr(N, field, "1");
		char *null=nsp_getstr(N, field, "2");
		char *dflt=nsp_getstr(N, field, "3");

		if (strcasecmp(sql_type, "FBSQL")==0) {
			if (strcasecmp(type, "TEXT")==0) {
				type="blob sub_type 1";
			}
		}

		if (line>0) {
			l=snprintf(qo, bl, ",\r\n\t");
			qo+=l;
			bl-=l;
		}
		line++;
		//for (attribute=field->val->d.table.f;attribute;attribute=attribute->next) {
		//	if (!nsp_isstr(attribute)||attribute->val->d.str==NULL) {
		//		printf("	%s=[ N U L L ]\r\n", attribute->name);
		//	} else {
		//		printf("	%s=[%s]\r\n", attribute->name, attribute->val->d.str);
		//	}
		//}
		l=snprintf(qo, bl, "%-20s %-20s", name, type);
		qo+=l;
		bl-=l;
		if (strcmp(dflt, "null")) {
			l=snprintf(qo, bl, "DEFAULT %s", dflt);
			qo+=l;
			bl-=l;
		}
		if (strcmp(dflt, "NOT NULL")!=0) {
			if (strcmp(dflt, "null")!=0) {
				l=snprintf(qo, bl, " ");
				qo+=l;
				bl-=l;
			}
			l=snprintf(qo, bl, "%s", null);
			qo+=l;
			bl-=l;
		}
	}
	if (strcmp(primarykey, "null")!=0) {
		l=snprintf(qo, bl, ",\r\n\tPRIMARY KEY(%s)", primarykey);
		qo+=l;
		bl-=l;
	}
	l=snprintf(qo, bl, "\r\n)\r\n");
	qo+=l;
	bl-=l;
//	printf("%s", query);

	if (strcasecmp(sql_type, "FBSQL")==0) {
		/* drop any old triggers and generators, and then the table */
		if (strcmp(sequencename, "null")!=0) {
			_sql_updatef(N, "DROP TRIGGER %s_bi;", tablename);
			_sql_updatef(N, "DROP GENERATOR gen_%s;", tablename);
		}
		_sql_updatef(N, "DROP TABLE %s;", tablename);
	} else if (strcasecmp(sql_type, "MYSQL")==0) {
		_sql_updatef(N, "DROP TABLE IF EXISTS %s;", tablename);
#ifdef WIN32
	} else if (strcasecmp(sql_type, "ODBC")==0) {
#endif
	} else if (strcasecmp(sql_type, "PGSQL")==0) {
		/* drop and create sequence before creating the table */
		_sql_updatef(N, "DROP SEQUENCE %s;", sequencename);
		_sql_updatef(N, "CREATE SEQUENCE %s start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;", sequencename);
		_sql_updatef(N, "DROP TABLE %s;", tablename);
	} else if (strcasecmp(sql_type, "SQLITE2")==0) {
		_sql_updatef(N, "DROP TABLE %s;", tablename);
	} else if (strncasecmp(sql_type, "SQLITE", 6)==0) {
		_sql_updatef(N, "DROP TABLE IF EXISTS %s;", tablename);
	}
	if (_sql_update(N, query)<0) {
		printf("\r\nError inserting %s\r\n", tablename);
		return -1;
	}
	if (strcasecmp(sql_type, "FBSQL")==0) {
		/* create generators and triggers for this table */
		if (strcmp(sequencename, "null")!=0) {
			_sql_updatef(N, "CREATE GENERATOR gen_%s;", tablename);
			_sql_updatef(N, "SET GENERATOR gen_%s TO 0;", tablename);
			_sql_updatef(N, "CREATE TRIGGER %s_bi FOR %s\n"
				"ACTIVE BEFORE INSERT POSITION 0\n"
				"AS\n"
				"BEGIN\n"
				"IF (NEW.%s IS NULL OR NEW.%s=0) THEN NEW.%s = GEN_ID(gen_%s, 1);\n"
				"END\n"
				, tablename, tablename, indexname, indexname, indexname, tablename
			);
		}
	}
	return 0;
}

int init_db(nsp_state *N)
{
	obj_t *table_schema;

	printf("Initialising %s database tables...", sql_type);
#if defined(linux)
	printf("\r\n");
	for (table_schema=sql_schema->val->d.table.f;table_schema;table_schema=table_schema->next) {
		if (!nsp_istable(sql_schema)) continue;
		printf("-");
	}
	printf("\r");
#endif
	for (table_schema=sql_schema->val->d.table.f;table_schema;table_schema=table_schema->next) {
		if (!nsp_istable(sql_schema)) continue;
		table_init(N, table_schema);
#if defined(linux)
		printf("=");
#endif
	}
#if defined(linux)
		printf("\r\n");
#endif
	table_check(N);
	sequence_sync(N);
	if (strlen(rootpass)>0) {
		if (_sql_updatef(N, "UPDATE gw_users SET password = '%s' WHERE userid = 1", rootpass)<0) {
			printf("\r\nError setting root password\r\n");
			return -1;
		}
	}
	printf("done.\r\n");
	_sql_disconnect(N);
	return 0;
}

int restore_db(nsp_state *N, char *filename)
{
	char line[32768];
	char *pTemp;
	FILE *fp;

	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	init_db(N);
	printf("Restoring data...\r\n");
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) {
			line[strlen(line)-1]='\0';
		}
		if (strncasecmp(line, "INSERT", 6)==0) {
			pTemp=line;
			while (*pTemp) {
				if ((pTemp[0]=='\\')&&(pTemp[1]=='n')) {
					pTemp[0]=13;
					pTemp[1]=10;
				}
				pTemp++;
			}
			if (_sql_update(N, line)<0) return -1;
		}
	}
	sequence_sync(N);
	fclose(fp);
	table_check(N);
	printf("done.\r\n");
	_sql_disconnect(N);
	return 0;
}

int dump_table(nsp_state *N, FILE *fp, char *table, char *index)
{
	char query[100];
	char *ptemp;
	int i;
	int j;
	obj_t *qobj=NULL;

	if (strcmp(table, "gw_dbinfo")==0) {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY %s ASC", table, index);
	} else {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY obj_did ASC, %s ASC", table, index);
	}
	if (_sql_query(N, &qobj, query)<0) {
		printf("\r\nError dumping %s\r\n", table);
		return -1;
	}
	for (i=0;i<sql_numtuples(N, &qobj);i++) {
		fprintf(fp, "INSERT INTO %s (", table);
		for (j=0;j<sql_numfields(N, &qobj);j++) {
			ptemp=sql_getname(N, &qobj, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			fprintf(fp, "%s", sql_getname(N, &qobj, j));
			if (j<sql_numfields(N, &qobj)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ") VALUES (");
		for (j=0;j<sql_numfields(N, &qobj);j++) {
			ptemp=sql_getname(N, &qobj, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			if (j>0) fprintf(fp, ", ");
			fprintf(fp, "'");
			sqlfprintf(N, fp, "%s", sql_getvalue(N, &qobj, i, j));
			fprintf(fp, "'");
		}
		fprintf(fp, ");\n");
	}
	_sql_freeresult(N, &qobj);
	return 0;
}

int dump_db_sql(nsp_state *N, char *filename)
{
	FILE *fp;

	printf("Dumping %s database to %s...", sql_type, filename);
#ifndef WIN32
	umask(077);
#endif
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	dump_table(N, fp, "gw_dbinfo",                 "dbversion");
	dump_table(N, fp, "gw_users",                  "userid");
	dump_table(N, fp, "gw_users_sessions",         "sessionid");
	dump_table(N, fp, "gw_activity",               "activityid");
	dump_table(N, fp, "gw_bookmarks",              "bookmarkid");
	dump_table(N, fp, "gw_bookmarks_folders",      "folderid");
	dump_table(N, fp, "gw_calls",                  "callid");
	dump_table(N, fp, "gw_calls_actions",          "actionid");
	dump_table(N, fp, "gw_contacts",               "contactid");
	dump_table(N, fp, "gw_contacts_folders",       "folderid");
	dump_table(N, fp, "gw_contacts_sessions",      "sessionid");
	dump_table(N, fp, "gw_domains",                "domainid");
	dump_table(N, fp, "gw_domains_aliases",        "domainaliasid");
	dump_table(N, fp, "gw_email_accounts",         "mailaccountid");
	dump_table(N, fp, "gw_email_filters",          "mailfilterid");
	dump_table(N, fp, "gw_email_folders",          "mailfolderid");
	dump_table(N, fp, "gw_email_headers",          "mailheaderid");
	dump_table(N, fp, "gw_events",                 "eventid");
	dump_table(N, fp, "gw_events_closings",        "eventclosingid");
	dump_table(N, fp, "gw_events_types",           "eventtypeid");
	dump_table(N, fp, "gw_files",                  "fileid");
	dump_table(N, fp, "gw_finance_accounts",       "accountid");
	dump_table(N, fp, "gw_finance_inventory",      "inventoryid");
	dump_table(N, fp, "gw_finance_invoices",       "invoiceid");
	dump_table(N, fp, "gw_finance_invoices_items", "invoiceitemid");
	dump_table(N, fp, "gw_finance_journal",        "entryid");
	dump_table(N, fp, "gw_forums",                 "forumid");
	dump_table(N, fp, "gw_forums_groups",          "forumgroupid");
	dump_table(N, fp, "gw_forums_posts",           "messageid");
	dump_table(N, fp, "gw_groups",                 "groupid");
	dump_table(N, fp, "gw_groups_members",         "groupmemberid");
	dump_table(N, fp, "gw_messages",               "messageid");
	dump_table(N, fp, "gw_notes",                  "noteid");
	dump_table(N, fp, "gw_projects",               "projectid");
	dump_table(N, fp, "gw_queries",                "queryid");
	dump_table(N, fp, "gw_smtp_relayrules",        "relayruleid");
	dump_table(N, fp, "gw_tasks",                  "taskid");
	dump_table(N, fp, "gw_weblog_comments",        "commentid");
	dump_table(N, fp, "gw_weblog_entries",         "entryid");
	dump_table(N, fp, "gw_weblog_groups",          "groupid");
	dump_table(N, fp, "gw_zones",                  "zoneid");
	fclose(fp);
	printf("done.\r\n");
	_sql_disconnect(N);
	return -1;
}

void usage(char *arg0)
{
	char *progname;

	printf("\r\nNullLogic GroupServer Database Utility [NullLogic GroupServer %s]\r\n", PACKAGE_VERSION);
#ifdef WIN32
	progname=strrchr(arg0, '\\');
#else
	progname=strrchr(arg0, '/');
#endif
	if (progname==NULL) {
		progname=arg0;
	} else {
		progname++;
	}
	printf("Usage: %s command parameter\r\n\r\n", progname);
	printf("  command  Must be one of the following\r\n\r\n");
	printf("    INIT     Initializes a new database\r\n");
	printf("    DUMP     Dumps the data from the current database to a file\r\n");
	printf("    RESTORE  Restores the current database from a file\r\n");
	printf("\r\nThe INIT command is used to initialize a new NullLogic GroupServer database\r\n");
	printf("(as defined in nullsd.conf).  The parameter for this function\r\n");
	printf("is the password for the administrator account.\r\n");
	printf("\r\nThe DUMP command is used to export the contents of the current database\r\n");
	printf("(as defined in nullsd.conf) to a text file.  The parameter for this function\r\n");
	printf("is the name of the file to which the data is to be saved.\r\n");
	printf("\r\nThe RESTORE command is used to restore a previous database dump to the current\r\n");
	printf("database (as defined in nullsd.conf).  The parameter for this function is the\r\n");
	printf("name of the file from which the data is to be restored.\r\n");
	printf("\r\nNOTE: The INIT and RESTORE commands WILL destroy any current database before\r\n");
	printf("recreating it.  Be sure to use DUMP to make a backup prior to using the INIT\r\n");
	printf("and RESTORE commands.\r\n");
	return;
}

int main(int argc, char *argv[])
{
	nsp_state *N;
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char salt[10];
	char function[16];
	char parameter[64];
	char *pTemp;
	int i;
	obj_t *tobj;
	obj_t *cobj;
	char filename[255];

	setvbuf(stdout, NULL, _IONBF, 0);
	if ((N=nsp_newstate())==NULL) {
		printf("nsp_newstate() error\r\n");
		return -1;
	}
	loglevel=1;
	memset(rootpass, 0, sizeof(rootpass));
	memset(salt, 0, sizeof(salt));
	memset(sql_type, 0, sizeof(sql_type));
	memset(function, 0, sizeof(function));
	memset(parameter, 0, sizeof(parameter));
	if (argc<2) {
		usage(argv[0]);
		return 0;
	}
	strncpy(function, argv[1], sizeof(function)-1);
	pTemp=function;
	while (*pTemp) {
		*pTemp=tolower(*pTemp);
		pTemp++;
	}
	if ((strcmp(function, "init")!=0)&&(argc!=3)) {
		usage(argv[0]);
		return 0;
	}
	if (argc>2) {
		strncpy(parameter, argv[2], sizeof(parameter)-1);
	}
	if (config_read(N, "", NULL)<0) {
		printf("can't read the config file.\r\n");
		return 0;
	}
	tobj=nsp_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type!=NT_TABLE) return 0;
	cobj=nsp_getobj(N, tobj, "sql_server_type");
	if (cobj->val->type==NT_STRING) {
		snprintf(sql_type, sizeof(sql_type)-1, "%s", cobj->val->d.str);
	}

	snprintf(filename, sizeof(filename)-1, "%s/var/share/scripts/dbinit.ns", "..");
	if (strlen(filename)>0) {
		if (nsp_execfile(N, filename)==0);
	}
	sql_schema=nsp_getobj(N, &N->g, "sql_schema");
	if (!nsp_istable(sql_schema)) {
		printf("table 'sql_schema' not found\r\n");
		return 0;
	}

	if (strcmp(function, "init")==0) {
		if (argc<3) {
			printf("Failing to provide a new password for the administrator account may leave\r\n");
			printf("your system vulnerable.  Now would be a REALLY good time to provide one.\r\n");
		} else {
			srand(time(NULL));
			for (i=0;i<8;i++) salt[i]=itoa64[(rand()%64)];
			md5_crypt(rootpass, argv[2], salt);
		}
		return init_db(N);
	}
	if (strcmp(function, "dump")==0) {
		return dump_db_sql(N, parameter);
	}
	if (strcmp(function, "restore")==0) {
		return restore_db(N, parameter);
	}
	if (N->err) {
		printf("errno=%d :: \"%s\"\r\n", N->err, N->errbuf);
	}
	N=nsp_endstate(N);
	return 0;
}
