/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "sql/schema/data.h"
#include "sql/schema/mysql.h"
#include "sql/schema/odbc.h"
#include "sql/schema/pgsql.h"
#include "sql/schema/sqlite.h"

int dump_table(FILE *fp, char *table, char *index)
{
	char query[100];
	char *ptemp;
	int i;
	int j;
	int sqr;

	snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY %s ASC", table, index);
	if ((sqr=sqlQuery(query))<0) {
		printf("\r\nError dumping %s\r\n", table);
		return -1;
	}
	for (i=0;i<sqlNumtuples(sqr);i++) {
		fprintf(fp, "INSERT INTO %s (", table);
		for (j=0;j<sqlNumfields(sqr);j++) {
			ptemp=sqlGetfieldname(sqr, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			fprintf(fp, "%s", sqlGetfieldname(sqr, j));
			if (j<sqlNumfields(sqr)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ") VALUES (");
		for (j=0;j<sqlNumfields(sqr);j++) {
			ptemp=sqlGetfieldname(sqr, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			fprintf(fp, "'");
			sqlfprintf(fp, "%s", sqlGetvalue(sqr, i, j));
			fprintf(fp, "'");
			if (j<sqlNumfields(sqr)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ");\n");
	}
	sqlFreeconnect(sqr);
	return 0;
}

int init_table(char *query, char *tablename)
{
	if (strcasecmp(config.sql_type, "MYSQL")==0) {
		sqlUpdatef(0, "DROP TABLE IF EXISTS %s;", tablename);
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		sqlUpdatef(0, "DROP TABLE %s;", tablename);
	} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
		sqlUpdatef(0, "DROP TABLE %s;", tablename);
	}
	if (sqlUpdate(1, query)<0) {
		printf("\r\nError inserting %s\r\n", tablename);
		return -1;
	}
	return 0;
}

int pgsql_sequence_sync(void)
{
	int max;
	int seq;
	int sqr;
	int i;

	for (i=0;;i++) {
		if (pgsqldb_tables[i].seqname==NULL) break;
		if ((sqr=sqlQueryf("SELECT max(%s) FROM %s", pgsqldb_tables[i].index, pgsqldb_tables[i].name))<0) return -1;
		if (sqlNumtuples(sqr)!=1) {
			sqlFreeconnect(sqr);
			return -1;
		}
		max=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if ((sqr=sqlQueryf("SELECT last_value FROM %s", pgsqldb_tables[i].seqname))<0) return -1;
		if (sqlNumtuples(sqr)!=1) {
			sqlFreeconnect(sqr);
			return -1;
		}
		seq=atoi(sqlGetvalue(sqr, 0, 0));
		sqlFreeconnect(sqr);
		if (seq<max) seq=max;
		if ((sqr=sqlQueryf("SELECT setval ('\"%s\"', %d, false);", pgsqldb_tables[i].seqname, seq))<0) return -1;
		if (sqlNumtuples(sqr)!=1) {
			sqlFreeconnect(sqr);
			return -1;
		}
		sqlFreeconnect(sqr);
	}
	return 0;
}

int table_check()
{
	int sqr;
	int i;
	int x;

	/* CHECK gw_dbinfo TABLE */
	if ((sqr=sqlQuery("SELECT count(*) FROM gw_dbinfo"))<0) return -1;
	x=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (x==0) {
		for (i=0;sqldata_new[i]!=NULL;i++) {
			if (sqlUpdate(1, sqldata_new[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_new[i]); return -1; }
		}
	}
	/* CHECK gw_callactions TABLE */
	if ((sqr=sqlQuery("SELECT count(*) FROM gw_callactions"))<0) return -1;
	x=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (x==0) {
		for (i=0;sqldata_callactions[i]!=NULL;i++) {
			if (sqlUpdate(1, sqldata_callactions[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_callactions[i]); return -1; }
		}
	}
	/* CHECK gw_domains TABLE */
	if ((sqr=sqlQuery("SELECT count(*) FROM gw_domains"))<0) return -1;
	x=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (x==0) {
		for (i=0;sqldata_domains[i]!=NULL;i++) {
			if (sqlUpdate(1, sqldata_domains[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_domains[i]); return -1; }
		}
	}
	/* CHECK gw_eventclosings TABLE */
	if ((sqr=sqlQuery("SELECT count(*) FROM gw_eventclosings"))<0) return -1;
	x=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (x==0) {
		for (i=0;sqldata_eventclosings[i]!=NULL;i++) {
			if (sqlUpdate(1, sqldata_eventclosings[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventclosings[i]); return -1; }
		}
	}
	/* CHECK gw_eventtypes TABLE */
	if ((sqr=sqlQuery("SELECT count(*) FROM gw_eventtypes"))<0) return -1;
	x=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (x==0) {
		for (i=0;sqldata_eventtypes[i]!=NULL;i++) {
			if (sqlUpdate(1, sqldata_eventtypes[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventtypes[i]); return -1; }
		}
	}
	/* Assign all orphaned records to domain 1 */
	sqlUpdatef(1, "UPDATE gw_users SET obj_did = 1, domainid = 1 WHERE domainid = 0");
	sqlUpdatef(1, "UPDATE gw_groups SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_contacts SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_activity SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_bookmarks SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_bookmarkfolders SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_calls SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_events SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_files SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_mailaccounts SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_mailfolders SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_mailheaders SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_notes SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_orders SET obj_did = 1 WHERE obj_did = 0");
	sqlUpdatef(1, "UPDATE gw_tasks SET obj_did = 1 WHERE obj_did = 0");
	/* Update the db version */
	sqlUpdatef(1, "UPDATE gw_dbinfo SET dbversion = '%s'", PACKAGE_VERSION);
	return 0;
}

int dump_db(char *filename)
{
	FILE *fp;

	printf("Dumping %s database to %s...", config.sql_type, filename);
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	dump_table(fp, "gw_dbinfo",              "dbversion");
	dump_table(fp, "gw_users",               "userid");
	dump_table(fp, "gw_usersessions",        "sessionid");
	dump_table(fp, "gw_accounting_accounts", "accountid");
	dump_table(fp, "gw_accounting_journal",  "journalentryid");
	dump_table(fp, "gw_activity",            "activityid");
	dump_table(fp, "gw_bookmarkfolders",     "folderid");
	dump_table(fp, "gw_bookmarks",           "bookmarkid");
	dump_table(fp, "gw_calls",               "callid");
	dump_table(fp, "gw_callactions",         "callactionid");
	dump_table(fp, "gw_contacts",            "contactid");
	dump_table(fp, "gw_contactsessions",     "sessionid");
	dump_table(fp, "gw_domains",             "domainid");
	dump_table(fp, "gw_domainaliases",       "domainaliasid");
	dump_table(fp, "gw_events",              "eventid");
	dump_table(fp, "gw_eventclosings",       "eventclosingid");
	dump_table(fp, "gw_eventtypes",          "eventtypeid");
	dump_table(fp, "gw_files",               "fileid");
	dump_table(fp, "gw_forums",              "forumid");
	dump_table(fp, "gw_forumgroups",         "forumgroupid");
	dump_table(fp, "gw_forumposts",          "messageid");
	dump_table(fp, "gw_groups",              "groupid");
	dump_table(fp, "gw_groupmembers",        "groupmemberid");
	dump_table(fp, "gw_mailaccounts",        "mailaccountid");
	dump_table(fp, "gw_mailfilters",         "mailfilterid");
	dump_table(fp, "gw_mailfolders",         "mailfolderid");
	dump_table(fp, "gw_mailheaders",         "mailheaderid");
	dump_table(fp, "gw_messages",            "messageid");
	dump_table(fp, "gw_notes",               "noteid");
	dump_table(fp, "gw_orders",              "orderid");
	dump_table(fp, "gw_orderitems",          "orderitemid");
	dump_table(fp, "gw_products",            "productid");
	dump_table(fp, "gw_projects",            "projectid");
	dump_table(fp, "gw_queries",             "queryid");
	dump_table(fp, "gw_smtp_relayrules",     "relayruleid");
	dump_table(fp, "gw_tasks",               "taskid");
	dump_table(fp, "gw_zones",               "zoneid");
	fclose(fp);
	printf("done.\r\n");
	sqlDisconnect();
	return -1;
}

int init_db(void)
{
	int i;

	if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Initialising MySQL database...");
		for (i=0;;i++) {
			if (mysqldb_tables[i].name==NULL) break;
			if (init_table(mysqldb_tables[i].schema, mysqldb_tables[i].name)!=0) return -1;
		}
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Initialising ODBC *.mdb database...");
		for (i=0;;i++) {
			if (mdb_tables[i].name==NULL) break;
			if (init_table(mdb_tables[i].schema, mdb_tables[i].name)!=0) return -1;
		}
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Initialising PostgreSQL database...");
		for (i=0;;i++) {
			if (pgsqldb_tables[i].seqname==NULL) break;
			sqlUpdatef(0, "DROP SEQUENCE %s;", pgsqldb_tables[i].seqname);
			sqlUpdatef(0, "CREATE SEQUENCE %s start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;", pgsqldb_tables[i].seqname);
		}
		for (i=0;;i++) {
			if (pgsqldb_tables[i].name==NULL) break;
			if (init_table(pgsqldb_tables[i].schema, pgsqldb_tables[i].name)!=0) return -1;
		}
	} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
		printf("Initialising SQLite database...");
		for (i=0;;i++) {
			if (sqlitedb_tables[i].name==NULL) break;
			if (init_table(sqlitedb_tables[i].schema, sqlitedb_tables[i].name)!=0) return -1;
		}
	}
	table_check();
	if (strcasecmp(config.sql_type, "PGSQL")==0) {
		pgsql_sequence_sync();
	}
	if (strlen(rootpass)>0) {
		if (sqlUpdatef(1, "UPDATE gw_users SET password = '%s' WHERE userid = 1", rootpass)<0) {
			printf("\r\nError setting root password\r\n");
			return -1;
		}
	}
	printf("done.\r\n");
	sqlDisconnect();
	return 0;
}

int restore_db(char *filename)
{
	char line[32768];
	char *pTemp;
	FILE *fp;
	int i;

	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Restoring MySQL database from %s...", filename);
		for (i=0;;i++) {
			if (mysqldb_tables[i].name==NULL) break;
			if (init_table(mysqldb_tables[i].schema, mysqldb_tables[i].name)!=0) return -1;
		}
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Restoring ODBC database from %s...", filename);
		for (i=0;;i++) {
			if (mdb_tables[i].name==NULL) break;
			if (init_table(mdb_tables[i].schema, mdb_tables[i].name)!=0) return -1;
		}
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Restoring PostgreSQL database from %s...", filename);
		for (i=0;;i++) {
			if (pgsqldb_tables[i].seqname==NULL) break;
			sqlUpdatef(0, "DROP SEQUENCE %s;", pgsqldb_tables[i].seqname);
			sqlUpdatef(0, "CREATE SEQUENCE %s start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;", pgsqldb_tables[i].seqname);
		}
		for (i=0;;i++) {
			if (pgsqldb_tables[i].name==NULL) break;
			if (init_table(pgsqldb_tables[i].schema, pgsqldb_tables[i].name)!=0) return -1;
		}
	} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
		printf("Restoring SQLite database from %s...", filename);
		for (i=0;;i++) {
			if (sqlitedb_tables[i].name==NULL) break;
			if (init_table(sqlitedb_tables[i].schema, sqlitedb_tables[i].name)!=0) return -1;
		}
	}
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
			if (sqlUpdate(1, line)<0) return -1;
		}
	}
	if (strcasecmp(config.sql_type, "PGSQL")==0) {
		pgsql_sequence_sync();
	}
	fclose(fp);
	table_check();
	printf("done.\r\n");
	sqlDisconnect();
	return 0;
}
