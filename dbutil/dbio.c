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
#include "schema/data.h"
#include "schema/mysql.h"
#include "schema/odbc.h"
#include "schema/pgsql.h"
#include "schema/sqlite.h"

int dump_table(FILE *fp, char *table, char *index)
{
	char query[100];
	int i;
	int j;
	int sqr;

	snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY %s ASC", table, index);
	if ((sqr=sqlQuery(query))<0) return -1;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		fprintf(fp, "INSERT INTO %s (", table);
		for (j=0;j<sqlNumfields(sqr);j++) {
			fprintf(fp, "%s", sqlGetfieldname(sqr, j));
			if (j<sqlNumfields(sqr)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ") VALUES (");
		for (j=0;j<sqlNumfields(sqr);j++) {
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

int dump_db(char *filename)
{
	FILE *fp;

	configread();
	printf("Dumping %s database to %s...", config.sql_type, filename);
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '1' WHERE folder = 'Inbox'");
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '2' WHERE folder = 'Outbox'");
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '3' WHERE folder = 'Sent Items'");
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '4' WHERE folder = 'Trash')");
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '5' WHERE folder = 'Drafts')");
	sqlUpdatef(0, "UPDATE gw_mailheaders SET folder = '6' WHERE folder = 'Saved Items')");
	if (dump_table(fp, "gw_dbinfo", "dbversion")<0)			printf("\r\nError dumping gw_dbinfo\r\n");
	if (dump_table(fp, "gw_users", "userid")<0) 			printf("\r\nError dumping gw_users\r\n");
	if (dump_table(fp, "gw_activity", "activityid")<0)		printf("\r\nError dumping gw_activity\r\n");
	if (dump_table(fp, "gw_bookmarkfolders", "folderid")<0)		printf("\r\nError dumping gw_bookmarkfolders\r\n");
	if (dump_table(fp, "gw_bookmarks", "bookmarkid")<0)		printf("\r\nError dumping gw_bookmarks\r\n");
	if (dump_table(fp, "gw_calls", "callid")<0)			printf("\r\nError dumping gw_calls\r\n");
	if (dump_table(fp, "gw_callactions", "callactionid")<0)		printf("\r\nError dumping gw_callactions\r\n");
	if (dump_table(fp, "gw_contacts", "contactid")<0)		printf("\r\nError dumping gw_contacts\r\n");
	if (dump_table(fp, "gw_domains", "domainid")<0)			printf("\r\nError dumping gw_domains\r\n");
	if (dump_table(fp, "gw_events", "eventid")<0)			printf("\r\nError dumping gw_events\r\n");
	if (dump_table(fp, "gw_eventclosings", "eventclosingid")<0)	printf("\r\nError dumping gw_eventclosings\r\n");
	if (dump_table(fp, "gw_eventtypes", "eventtypeid")<0)		printf("\r\nError dumping gw_eventtypes\r\n");
	if (dump_table(fp, "gw_files", "fileid")<0)			printf("\r\nError dumping gw_files\r\n");
	if (dump_table(fp, "gw_forums", "forumid")<0)			printf("\r\nError dumping gw_forums\r\n");
	if (dump_table(fp, "gw_forumgroups", "forumgroupid")<0)		printf("\r\nError dumping gw_forumgroups\r\n");
	if (dump_table(fp, "gw_forumposts", "messageid")<0) 		printf("\r\nError dumping gw_forumposts\r\n");
	if (dump_table(fp, "gw_groups", "groupid")<0) 			printf("\r\nError dumping gw_groups\r\n");
	if (dump_table(fp, "gw_mailaccounts", "mailaccountid")<0) 	printf("\r\nError dumping gw_mailaccounts\r\n");
	if (dump_table(fp, "gw_mailfilters", "mailfilterid")<0) 	printf("\r\nError dumping gw_mailfilters\r\n");
	if (dump_table(fp, "gw_mailfolders", "mailfolderid")<0) 	printf("\r\nError dumping gw_mailfolders\r\n");
	if (dump_table(fp, "gw_mailheaders", "mailheaderid")<0) 	printf("\r\nError dumping gw_mailheaders\r\n");
	if (dump_table(fp, "gw_messages", "messageid")<0) 		printf("\r\nError dumping gw_messages\r\n");
	if (dump_table(fp, "gw_notes", "noteid")<0) 			printf("\r\nError dumping gw_notes\r\n");
	if (dump_table(fp, "gw_orders", "orderid")<0) 			printf("\r\nError dumping gw_orders\r\n");
	if (dump_table(fp, "gw_orderitems", "orderitemid")<0) 		printf("\r\nError dumping gw_orderitems\r\n");
	if (dump_table(fp, "gw_products", "productid")<0) 		printf("\r\nError dumping gw_products\r\n");
	if (dump_table(fp, "gw_queries", "queryid")<0) 			printf("\r\nError dumping gw_queries\r\n");
	if (dump_table(fp, "gw_tasks", "taskid")<0) 			printf("\r\nError dumping gw_tasks\r\n");
	if (dump_table(fp, "gw_zones", "zoneid")<0) 			printf("\r\nError dumping gw_zones\r\n");
	fclose(fp);
	printf("done.\r\n");
	sqlDisconnect();
	return -1;
}

#ifdef WIN32
int init_mdb(void)
{
	if (sqlUpdate(1, MDB_DBINFO)<0)          { printf("\r\nError inserting gw_dbinfo\r\n");          return -1; }
	if (sqlUpdate(1, MDB_ACTIVITY)<0)        { printf("\r\nError inserting gw_activity\r\n");        return -1; }
	if (sqlUpdate(1, MDB_BOOKMARKFOLDERS)<0) { printf("\r\nError inserting gw_bookmarkfolders\r\n"); return -1; }
	if (sqlUpdate(1, MDB_BOOKMARKS)<0)       { printf("\r\nError inserting gw_bookmarks\r\n");       return -1; }
	if (sqlUpdate(1, MDB_CALLS)<0)           { printf("\r\nError inserting gw_calls\r\n");           return -1; }
	if (sqlUpdate(1, MDB_CALLACTIONS)<0)     { printf("\r\nError inserting gw_callactions\r\n");     return -1; }
	if (sqlUpdate(1, MDB_CONTACTS)<0)        { printf("\r\nError inserting gw_contacts\r\n");        return -1; }
	if (sqlUpdate(1, MDB_DOMAINS)<0)         { printf("\r\nError inserting gw_domains\r\n");         return -1; }
	if (sqlUpdate(1, MDB_EVENTS)<0)          { printf("\r\nError inserting gw_events\r\n");          return -1; }
	if (sqlUpdate(1, MDB_EVENTCLOSINGS)<0)   { printf("\r\nError inserting gw_eventclosings\r\n");   return -1; }
	if (sqlUpdate(1, MDB_EVENTTYPES)<0)      { printf("\r\nError inserting gw_eventtypes\r\n");      return -1; }
	if (sqlUpdate(1, MDB_FILES)<0)           { printf("\r\nError inserting gw_files\r\n");           return -1; }
	if (sqlUpdate(1, MDB_FORUMS)<0)          { printf("\r\nError inserting gw_forums\r\n");          return -1; }
	if (sqlUpdate(1, MDB_FORUMGROUPS)<0)     { printf("\r\nError inserting gw_forumgroups\r\n");     return -1; }
	if (sqlUpdate(1, MDB_FORUMPOSTS)<0)      { printf("\r\nError inserting gw_forumposts\r\n");      return -1; }
	if (sqlUpdate(1, MDB_GROUPS)<0)          { printf("\r\nError inserting gw_groups\r\n");          return -1; }
	if (sqlUpdate(1, MDB_MAILACCOUNTS)<0)    { printf("\r\nError inserting gw_mailaccounts\r\n");    return -1; }
	if (sqlUpdate(1, MDB_MAILFILTERS)<0)     { printf("\r\nError inserting gw_mailfilters\r\n");     return -1; }
	if (sqlUpdate(1, MDB_MAILFOLDERS)<0)     { printf("\r\nError inserting gw_mailfolders\r\n");     return -1; }
	if (sqlUpdate(1, MDB_MAILHEADERS)<0)     { printf("\r\nError inserting gw_mailheaders\r\n");     return -1; }
	if (sqlUpdate(1, MDB_MESSAGES)<0)        { printf("\r\nError inserting gw_messages\r\n");        return -1; }
	if (sqlUpdate(1, MDB_NOTES)<0)           { printf("\r\nError inserting gw_notes\r\n");           return -1; }
	if (sqlUpdate(1, MDB_ORDERS)<0)          { printf("\r\nError inserting gw_orders\r\n");          return -1; }
	if (sqlUpdate(1, MDB_ORDERITEMS)<0)      { printf("\r\nError inserting gw_orderitems\r\n");      return -1; }
	if (sqlUpdate(1, MDB_PRODUCTS)<0)        { printf("\r\nError inserting gw_products\r\n");        return -1; }
	if (sqlUpdate(1, MDB_QUERIES)<0)         { printf("\r\nError inserting gw_queries\r\n");         return -1; }
	if (sqlUpdate(1, MDB_TASKS)<0)           { printf("\r\nError inserting gw_tasks\r\n");           return -1; }
	if (sqlUpdate(1, MDB_USERS1 MDB_USERS2)<0) { printf("\r\nError inserting gw_users\r\n");         return -1; }
	if (sqlUpdate(1, MDB_ZONES)<0)           { printf("\r\nError inserting gw_zones\r\n");           return -1; }
	return 0;
}
#endif

int init_mysql(void)
{
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_dbinfo;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_activity;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_bookmarkfolders;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_bookmarks;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_calls;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_callactions;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_contacts;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_domains;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_events;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_eventclosings;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_eventtypes;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_files;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_forums;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_forumgroups;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_forumposts;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_groups;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_mailaccounts;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_mailfilters;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_mailfolders;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_mailheaders;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_messages;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_notes;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_orders;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_orderitems;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_products;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_queries;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_tasks;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_users;");
	sqlUpdate(0, "DROP TABLE IF EXISTS gw_zones;");
	if (sqlUpdate(1, MYSQLDB_DBINFO)<0)          { printf("\r\nError inserting gw_dbinfo\r\n");          return -1; }
	if (sqlUpdate(1, MYSQLDB_ACTIVITY)<0)        { printf("\r\nError inserting gw_activity\r\n");        return -1; }
	if (sqlUpdate(1, MYSQLDB_BOOKMARKFOLDERS)<0) { printf("\r\nError inserting gw_bookmarkfolders\r\n"); return -1; }
	if (sqlUpdate(1, MYSQLDB_BOOKMARKS)<0)       { printf("\r\nError inserting gw_bookmarks\r\n");       return -1; }
	if (sqlUpdate(1, MYSQLDB_CALLS)<0)           { printf("\r\nError inserting gw_calls\r\n");           return -1; }
	if (sqlUpdate(1, MYSQLDB_CALLACTIONS)<0)     { printf("\r\nError inserting gw_callactions\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_CONTACTS)<0)        { printf("\r\nError inserting gw_contacts\r\n");        return -1; }
	if (sqlUpdate(1, MYSQLDB_DOMAINS)<0)         { printf("\r\nError inserting gw_domains\r\n");         return -1; }
	if (sqlUpdate(1, MYSQLDB_EVENTS)<0)          { printf("\r\nError inserting gw_events\r\n");          return -1; }
	if (sqlUpdate(1, MYSQLDB_EVENTCLOSINGS)<0)   { printf("\r\nError inserting gw_eventclosings\r\n");   return -1; }
	if (sqlUpdate(1, MYSQLDB_EVENTTYPES)<0)      { printf("\r\nError inserting gw_eventtypes\r\n");      return -1; }
	if (sqlUpdate(1, MYSQLDB_FILES)<0)           { printf("\r\nError inserting gw_files\r\n");           return -1; }
	if (sqlUpdate(1, MYSQLDB_FORUMS)<0)          { printf("\r\nError inserting gw_forums\r\n");          return -1; }
	if (sqlUpdate(1, MYSQLDB_FORUMGROUPS)<0)     { printf("\r\nError inserting gw_forumgroups\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_FORUMPOSTS)<0)      { printf("\r\nError inserting gw_forumposts\r\n");      return -1; }
	if (sqlUpdate(1, MYSQLDB_GROUPS)<0)          { printf("\r\nError inserting gw_groups\r\n");          return -1; }
	if (sqlUpdate(1, MYSQLDB_MAILACCOUNTS)<0)    { printf("\r\nError inserting gw_mailaccounts\r\n");    return -1; }
	if (sqlUpdate(1, MYSQLDB_MAILFILTERS)<0)     { printf("\r\nError inserting gw_mailfilters\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_MAILFOLDERS)<0)     { printf("\r\nError inserting gw_mailfolders\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_MAILHEADERS)<0)     { printf("\r\nError inserting gw_mailheaders\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_MESSAGES)<0)        { printf("\r\nError inserting gw_messages\r\n");        return -1; }
	if (sqlUpdate(1, MYSQLDB_NOTES)<0)           { printf("\r\nError inserting gw_notes\r\n");           return -1; }
	if (sqlUpdate(1, MYSQLDB_ORDERS)<0)          { printf("\r\nError inserting gw_orders\r\n");          return -1; }
	if (sqlUpdate(1, MYSQLDB_ORDERITEMS)<0)      { printf("\r\nError inserting gw_orderitems\r\n");      return -1; }
	if (sqlUpdate(1, MYSQLDB_PRODUCTS)<0)        { printf("\r\nError inserting gw_products\r\n");        return -1; }
	if (sqlUpdate(1, MYSQLDB_QUERIES)<0)         { printf("\r\nError inserting gw_queries\r\n");         return -1; }
	if (sqlUpdate(1, MYSQLDB_TASKS)<0)           { printf("\r\nError inserting gw_tasks\r\n");           return -1; }
	if (sqlUpdate(1, MYSQLDB_USERS1 MYSQLDB_USERS2)<0) { printf("\r\nError inserting gw_users\r\n");     return -1; }
	if (sqlUpdate(1, MYSQLDB_ZONES)<0)           { printf("\r\nError inserting gw_zones\r\n");           return -1; }
	return 0;
}

int init_pgsql(void)
{
	sqlUpdate(0, "DROP SEQUENCE actiid_seq;");
	sqlUpdate(0, "DROP SEQUENCE bfldid_seq;");
	sqlUpdate(0, "DROP SEQUENCE bkmkid_seq;");
	sqlUpdate(0, "DROP SEQUENCE callid_seq;");
	sqlUpdate(0, "DROP SEQUENCE calaid_seq;");
	sqlUpdate(0, "DROP SEQUENCE contid_seq;");
	sqlUpdate(0, "DROP SEQUENCE domaid_seq;");
	sqlUpdate(0, "DROP SEQUENCE evenid_seq;");
	sqlUpdate(0, "DROP SEQUENCE ecloid_seq;");
	sqlUpdate(0, "DROP SEQUENCE etypid_seq;");
	sqlUpdate(0, "DROP SEQUENCE fileid_seq;");
	sqlUpdate(0, "DROP SEQUENCE foruid_seq;");
	sqlUpdate(0, "DROP SEQUENCE forgid_seq;");
	sqlUpdate(0, "DROP SEQUENCE grouid_seq;");
	sqlUpdate(0, "DROP SEQUENCE mailid_seq;");
	sqlUpdate(0, "DROP SEQUENCE mfilid_seq;");
	sqlUpdate(0, "DROP SEQUENCE mfdrid_seq;");
	sqlUpdate(0, "DROP SEQUENCE mhdrid_seq;");
	sqlUpdate(0, "DROP SEQUENCE messid_seq;");
	sqlUpdate(0, "DROP SEQUENCE noteid_seq;");
	sqlUpdate(0, "DROP SEQUENCE ordeid_seq;");
	sqlUpdate(0, "DROP SEQUENCE ordiid_seq;");
	sqlUpdate(0, "DROP SEQUENCE prodid_seq;");
	sqlUpdate(0, "DROP SEQUENCE querid_seq;");
	sqlUpdate(0, "DROP SEQUENCE taskid_seq;");
	sqlUpdate(0, "DROP SEQUENCE userid_seq;");
	sqlUpdate(0, "DROP SEQUENCE zoneid_seq;");
	sqlUpdate(0, "DROP TABLE gw_dbinfo;");
	sqlUpdate(0, "DROP TABLE gw_activity;");
	sqlUpdate(0, "DROP TABLE gw_bookmarkfolders;");
	sqlUpdate(0, "DROP TABLE gw_bookmarks;");
	sqlUpdate(0, "DROP TABLE gw_calls;");
	sqlUpdate(0, "DROP TABLE gw_callactions;");
	sqlUpdate(0, "DROP TABLE gw_contacts;");
	sqlUpdate(0, "DROP TABLE gw_domains;");
	sqlUpdate(0, "DROP TABLE gw_events;");
	sqlUpdate(0, "DROP TABLE gw_eventclosings;");
	sqlUpdate(0, "DROP TABLE gw_eventtypes;");
	sqlUpdate(0, "DROP TABLE gw_files;");
	sqlUpdate(0, "DROP TABLE gw_forums;");
	sqlUpdate(0, "DROP TABLE gw_forumgroups;");
	sqlUpdate(0, "DROP TABLE gw_forumposts;");
	sqlUpdate(0, "DROP TABLE gw_groups;");
	sqlUpdate(0, "DROP TABLE gw_mailaccounts;");
	sqlUpdate(0, "DROP TABLE gw_mailfilters;");
	sqlUpdate(0, "DROP TABLE gw_mailfolders;");
	sqlUpdate(0, "DROP TABLE gw_mailheaders;");
	sqlUpdate(0, "DROP TABLE gw_messages;");
	sqlUpdate(0, "DROP TABLE gw_notes;");
	sqlUpdate(0, "DROP TABLE gw_orders;");
	sqlUpdate(0, "DROP TABLE gw_orderitems;");
	sqlUpdate(0, "DROP TABLE gw_products;");
	sqlUpdate(0, "DROP TABLE gw_queries;");
	sqlUpdate(0, "DROP TABLE gw_tasks;");
	sqlUpdate(0, "DROP TABLE gw_users;");
	sqlUpdate(0, "DROP TABLE gw_zones;");
	if (sqlUpdate(1, PGSQLDB_SEQUENCES1)<0)      { printf("\r\nError inserting sequences\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_SEQUENCES2)<0)      { printf("\r\nError inserting sequences\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_DBINFO)<0)          { printf("\r\nError inserting gw_dbinfo\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_ACTIVITY)<0)        { printf("\r\nError inserting gw_activity\r\n");        return -1; }
	if (sqlUpdate(1, PGSQLDB_BOOKMARKFOLDERS)<0) { printf("\r\nError inserting gw_bookmarkfolders\r\n"); return -1; }
	if (sqlUpdate(1, PGSQLDB_BOOKMARKS)<0)       { printf("\r\nError inserting gw_bookmarks\r\n");       return -1; }
	if (sqlUpdate(1, PGSQLDB_CALLS)<0)           { printf("\r\nError inserting gw_calls\r\n");           return -1; }
	if (sqlUpdate(1, PGSQLDB_CALLACTIONS)<0)     { printf("\r\nError inserting gw_callactions\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_CONTACTS)<0)        { printf("\r\nError inserting gw_contacts\r\n");        return -1; }
	if (sqlUpdate(1, PGSQLDB_DOMAINS)<0)         { printf("\r\nError inserting gw_domains\r\n");         return -1; }
	if (sqlUpdate(1, PGSQLDB_EVENTS)<0)          { printf("\r\nError inserting gw_events\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_EVENTCLOSINGS)<0)   { printf("\r\nError inserting gw_eventclosings\r\n");   return -1; }
	if (sqlUpdate(1, PGSQLDB_EVENTTYPES)<0)      { printf("\r\nError inserting gw_eventtypes\r\n");      return -1; }
	if (sqlUpdate(1, PGSQLDB_FILES)<0)           { printf("\r\nError inserting gw_files\r\n");           return -1; }
	if (sqlUpdate(1, PGSQLDB_FORUMS)<0)          { printf("\r\nError inserting gw_forums\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_FORUMGROUPS)<0)     { printf("\r\nError inserting gw_forumgroups\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_FORUMPOSTS)<0)      { printf("\r\nError inserting gw_forumposts\r\n");      return -1; }
	if (sqlUpdate(1, PGSQLDB_GROUPS)<0)          { printf("\r\nError inserting gw_groups\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_MAILACCOUNTS)<0)    { printf("\r\nError inserting gw_mailaccounts\r\n");    return -1; }
	if (sqlUpdate(1, PGSQLDB_MAILFILTERS)<0)     { printf("\r\nError inserting gw_mailfilters\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_MAILFOLDERS)<0)     { printf("\r\nError inserting gw_mailfolders\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_MAILHEADERS)<0)     { printf("\r\nError inserting gw_mailheaders\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_MESSAGES)<0)        { printf("\r\nError inserting gw_messages\r\n");        return -1; }
	if (sqlUpdate(1, PGSQLDB_NOTES)<0)           { printf("\r\nError inserting gw_notes\r\n");           return -1; }
	if (sqlUpdate(1, PGSQLDB_ORDERS)<0)          { printf("\r\nError inserting gw_orders\r\n");          return -1; }
	if (sqlUpdate(1, PGSQLDB_ORDERITEMS)<0)      { printf("\r\nError inserting gw_orderitems\r\n");      return -1; }
	if (sqlUpdate(1, PGSQLDB_PRODUCTS)<0)        { printf("\r\nError inserting gw_products\r\n");        return -1; }
	if (sqlUpdate(1, PGSQLDB_QUERIES)<0)         { printf("\r\nError inserting gw_queries\r\n");         return -1; }
	if (sqlUpdate(1, PGSQLDB_TASKS)<0)           { printf("\r\nError inserting gw_tasks\r\n");           return -1; }
	if (sqlUpdate(1, PGSQLDB_USERS1 PGSQLDB_USERS2)<0) { printf("\r\nError inserting gw_users\r\n");     return -1; }
	if (sqlUpdate(1, PGSQLDB_ZONES)<0)           { printf("\r\nError inserting gw_zones\r\n");           return -1; }
	return 0;
}

int pgsql_seqsync(char *tablename, char *indexname, char *seqname)
{
	int max;
	int seq;
	int sqr;

	if ((sqr=sqlQueryf("SELECT max(%s) FROM %s", indexname, tablename))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	max=atoi(sqlGetvalue(sqr, 0, 0))+1;
	sqlFreeconnect(sqr);
	if ((sqr=sqlQueryf("SELECT last_value FROM %s", seqname))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	seq=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	if (seq<max) seq=max;
	if ((sqr=sqlQueryf("SELECT setval ('\"%s\"', %d, false);", seqname, seq))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	sqlFreeconnect(sqr);
	return 0;
}

int init_pgsqlseq(void)
{
	pgsql_seqsync("gw_activity",        "activityid",     "actiid_seq");
	pgsql_seqsync("gw_bookmarkfolders", "folderid",       "bfldid_seq");
	pgsql_seqsync("gw_bookmarks",       "bookmarkid",     "bkmkid_seq");
	pgsql_seqsync("gw_calls",           "callid",         "callid_seq");
	pgsql_seqsync("gw_callactions",     "callactionid",   "calaid_seq");
	pgsql_seqsync("gw_contacts",        "contactid",      "contid_seq");
	pgsql_seqsync("gw_domains",         "domainid",       "domaid_seq");
	pgsql_seqsync("gw_events",          "eventid",        "evenid_seq");
	pgsql_seqsync("gw_eventclosings",   "eventclosingid", "ecloid_seq");
	pgsql_seqsync("gw_eventtypes",      "eventtypeid",    "etypid_seq");
	pgsql_seqsync("gw_files",           "fileid",         "fileid_seq");
	pgsql_seqsync("gw_forums",          "forumid",        "foruid_seq");
	pgsql_seqsync("gw_forumgroups",     "forumgroupid",   "forgid_seq");
	pgsql_seqsync("gw_groups",          "groupid",        "grouid_seq");
	pgsql_seqsync("gw_mailaccounts",    "mailaccountid",  "mailid_seq");
	pgsql_seqsync("gw_mailfilters",     "mailfilterid",   "mfilid_seq");
	pgsql_seqsync("gw_mailfolders",     "mailfolderid",   "mfdrid_seq");
	pgsql_seqsync("gw_mailheaders",     "mailheaderid",   "mhdrid_seq");
	pgsql_seqsync("gw_messages",        "messageid",      "messid_seq");
	pgsql_seqsync("gw_notes",           "noteid",         "noteid_seq");
	pgsql_seqsync("gw_orders",          "orderid",        "ordeid_seq");
	pgsql_seqsync("gw_orderitems",      "orderitemid",    "ordiid_seq");
	pgsql_seqsync("gw_products",        "productid",      "prodid_seq");
	pgsql_seqsync("gw_queries",         "queryid",        "querid_seq");
	pgsql_seqsync("gw_tasks",           "taskid",         "taskid_seq");
	pgsql_seqsync("gw_users",           "userid",         "userid_seq");
	pgsql_seqsync("gw_zones",           "zoneid",         "zoneid_seq");
	return 0;
}

int init_sqlite(void)
{
	sqlUpdate(0, "DROP TABLE gw_dbinfo;");
	sqlUpdate(0, "DROP TABLE gw_activity;");
	sqlUpdate(0, "DROP TABLE gw_bookmarkfolders;");
	sqlUpdate(0, "DROP TABLE gw_bookmarks;");
	sqlUpdate(0, "DROP TABLE gw_calls;");
	sqlUpdate(0, "DROP TABLE gw_callactions;");
	sqlUpdate(0, "DROP TABLE gw_contacts;");
	sqlUpdate(0, "DROP TABLE gw_domains;");
	sqlUpdate(0, "DROP TABLE gw_events;");
	sqlUpdate(0, "DROP TABLE gw_eventclosings;");
	sqlUpdate(0, "DROP TABLE gw_eventtypes;");
	sqlUpdate(0, "DROP TABLE gw_files;");
	sqlUpdate(0, "DROP TABLE gw_forums;");
	sqlUpdate(0, "DROP TABLE gw_forumgroups;");
	sqlUpdate(0, "DROP TABLE gw_forumposts;");
	sqlUpdate(0, "DROP TABLE gw_groups;");
	sqlUpdate(0, "DROP TABLE gw_mailaccounts;");
	sqlUpdate(0, "DROP TABLE gw_mailfilters;");
	sqlUpdate(0, "DROP TABLE gw_mailfolders;");
	sqlUpdate(0, "DROP TABLE gw_mailheaders;");
	sqlUpdate(0, "DROP TABLE gw_messages;");
	sqlUpdate(0, "DROP TABLE gw_notes;");
	sqlUpdate(0, "DROP TABLE gw_orders;");
	sqlUpdate(0, "DROP TABLE gw_orderitems;");
	sqlUpdate(0, "DROP TABLE gw_products;");
	sqlUpdate(0, "DROP TABLE gw_queries;");
	sqlUpdate(0, "DROP TABLE gw_tasks;");
	sqlUpdate(0, "DROP TABLE gw_users;");
	sqlUpdate(0, "DROP TABLE gw_zones;");
	if (sqlUpdate(1, SQLITEDB_DBINFO)<0)          { printf("\r\nError inserting gw_dbinfo\r\n");          return -1; }
	if (sqlUpdate(1, SQLITEDB_ACTIVITY)<0)        { printf("\r\nError inserting gw_activity\r\n");        return -1; }
	if (sqlUpdate(1, SQLITEDB_BOOKMARKFOLDERS)<0) { printf("\r\nError inserting gw_bookmarkfolders\r\n"); return -1; }
	if (sqlUpdate(1, SQLITEDB_BOOKMARKS)<0)       { printf("\r\nError inserting gw_bookmarks\r\n");       return -1; }
	if (sqlUpdate(1, SQLITEDB_CALLS)<0)           { printf("\r\nError inserting gw_calls\r\n");           return -1; }
	if (sqlUpdate(1, SQLITEDB_CALLACTIONS)<0)     { printf("\r\nError inserting gw_callactions\r\n");     return -1; }
	if (sqlUpdate(1, SQLITEDB_CONTACTS)<0)        { printf("\r\nError inserting gw_contacts\r\n");        return -1; }
	if (sqlUpdate(1, SQLITEDB_DOMAINS)<0)         { printf("\r\nError inserting gw_domains\r\n");         return -1; }
	if (sqlUpdate(1, SQLITEDB_EVENTS)<0)          { printf("\r\nError inserting gw_events\r\n");          return -1; }
	if (sqlUpdate(1, SQLITEDB_EVENTCLOSINGS)<0)   { printf("\r\nError inserting gw_eventclosings\r\n");   return -1; }
	if (sqlUpdate(1, SQLITEDB_EVENTTYPES)<0)      { printf("\r\nError inserting gw_eventtypes\r\n");      return -1; }
	if (sqlUpdate(1, SQLITEDB_FILES)<0)           { printf("\r\nError inserting gw_files\r\n");           return -1; }
	if (sqlUpdate(1, SQLITEDB_FORUMS)<0)          { printf("\r\nError inserting gw_forums\r\n");          return -1; }
	if (sqlUpdate(1, SQLITEDB_FORUMGROUPS)<0)     { printf("\r\nError inserting gw_forumgroups\r\n");     return -1; }
	if (sqlUpdate(1, SQLITEDB_FORUMPOSTS)<0)      { printf("\r\nError inserting gw_forumposts\r\n");      return -1; }
	if (sqlUpdate(1, SQLITEDB_GROUPS)<0)          { printf("\r\nError inserting gw_groups\r\n");          return -1; }
	if (sqlUpdate(1, SQLITEDB_MAILACCOUNTS)<0)    { printf("\r\nError inserting gw_mailaccounts\r\n");    return -1; }
	if (sqlUpdate(1, SQLITEDB_MAILFILTERS)<0)     { printf("\r\nError inserting gw_mailfilters\r\n");     return -1; }
	if (sqlUpdate(1, SQLITEDB_MAILFOLDERS)<0)     { printf("\r\nError inserting gw_mailfolders\r\n");     return -1; }
	if (sqlUpdate(1, SQLITEDB_MAILHEADERS)<0)     { printf("\r\nError inserting gw_mailheaders\r\n");     return -1; }
	if (sqlUpdate(1, SQLITEDB_MESSAGES)<0)        { printf("\r\nError inserting gw_messages\r\n");        return -1; }
	if (sqlUpdate(1, SQLITEDB_NOTES)<0)           { printf("\r\nError inserting gw_notes\r\n");           return -1; }
	if (sqlUpdate(1, SQLITEDB_ORDERS)<0)          { printf("\r\nError inserting gw_orders\r\n");          return -1; }
	if (sqlUpdate(1, SQLITEDB_ORDERITEMS)<0)      { printf("\r\nError inserting gw_orderitems\r\n");      return -1; }
	if (sqlUpdate(1, SQLITEDB_PRODUCTS)<0)        { printf("\r\nError inserting gw_products\r\n");        return -1; }
	if (sqlUpdate(1, SQLITEDB_QUERIES)<0)         { printf("\r\nError inserting gw_queries\r\n");         return -1; }
	if (sqlUpdate(1, SQLITEDB_TASKS)<0)           { printf("\r\nError inserting gw_tasks\r\n");           return -1; }
	if (sqlUpdate(1, SQLITEDB_USERS1 SQLITEDB_USERS2)<0) { printf("\r\nError inserting gw_users\r\n");    return -1; }
	if (sqlUpdate(1, SQLITEDB_ZONES)<0)           { printf("\r\nError inserting gw_zones\r\n");           return -1; }
	return 0;
}

int init_db(void)
{
	if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Initialising MySQL database...");
		if (init_mysql()<0) return -1;
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Initialising ODBC *.mdb database...");
		if (init_mdb()<0) return -1;
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Initialising PostgreSQL database...");
		if (init_pgsql()<0) return -1;
	} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
		printf("Initialising SQLite database...");
		if (init_sqlite()<0) return -1;
	}
	if (sqlUpdate(1, DBDATA_01)<0) { printf("\r\nError inserting dbdata_01\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_02)<0) { printf("\r\nError inserting dbdata_02\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_03)<0) { printf("\r\nError inserting dbdata_03\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_04)<0) { printf("\r\nError inserting dbdata_04\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_05)<0) { printf("\r\nError inserting dbdata_05\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_06)<0) { printf("\r\nError inserting dbdata_06\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_07)<0) { printf("\r\nError inserting dbdata_07\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_08)<0) { printf("\r\nError inserting dbdata_08\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_09)<0) { printf("\r\nError inserting dbdata_09\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_10)<0) { printf("\r\nError inserting dbdata_10\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_11)<0) { printf("\r\nError inserting dbdata_11\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_12)<0) { printf("\r\nError inserting dbdata_12\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_13)<0) { printf("\r\nError inserting dbdata_13\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_14)<0) { printf("\r\nError inserting dbdata_14\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_15)<0) { printf("\r\nError inserting dbdata_15\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_16)<0) { printf("\r\nError inserting dbdata_16\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_17)<0) { printf("\r\nError inserting dbdata_17\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_18)<0) { printf("\r\nError inserting dbdata_18\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_19)<0) { printf("\r\nError inserting dbdata_19\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_20)<0) { printf("\r\nError inserting dbdata_20\r\n"); return -1; }
	if (sqlUpdate(1, DBDATA_21)<0) { printf("\r\nError inserting dbdata_21\r\n"); return -1; }
	if (strcasecmp(config.sql_type, "PGSQL")==0) {
		init_pgsqlseq();
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
	char line[12288];
	char *pTemp;
	FILE *fp;

	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Restoring MySQL database from %s...", filename);
		if (init_mysql()<0) return -1;
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Restoring ODBC database from %s...", filename);
		if (init_mdb()<0) return -1;
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Restoring PostgreSQL database from %s...", filename);
		if (init_pgsql()<0) return -1;
	} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
		printf("Restoring SQLite database from %s...", filename);
		if (init_sqlite()<0) return -1;
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
		init_pgsqlseq();
	}
	fclose(fp);
	printf("done.\r\n");
	sqlDisconnect();
	return 0;
}
