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

#ifdef WIN32
int restore_mdb(char *filename)
{
	printf("Restoring ODBC database from %s...", filename);
	if (makemdb("..\\etc\\sentinel.mdb")<0) exit(0);
	if (sqlUpdate(1, MDB_DBINFO)<0) return -1;
//	if (sqlUpdate(1, MDB_CALLS)<0) return -1;
	if (sqlUpdate(1, MDB_COMPANIES)<0) return -1;
	if (sqlUpdate(1, MDB_CONTACTS)<0) return -1;
	if (sqlUpdate(1, MDB_EVENTS)<0) return -1;
	if (sqlUpdate(1, MDB_FORUMS)<0) return -1;
	if (sqlUpdate(1, MDB_FORUMPOSTS)<0) return -1;
	if (sqlUpdate(1, MDB_LINKS)<0) return -1;
	if (sqlUpdate(1, MDB_MESSAGES)<0) return -1;
	if (sqlUpdate(1, MDB_ORDERS)<0) return -1;
	if (sqlUpdate(1, MDB_ORDERDETAILS)<0) return -1;
	if (sqlUpdate(1, MDB_PRODUCTS)<0) return -1;
	if (sqlUpdate(1, MDB_QUERIES)<0) return -1;
	if (sqlUpdate(1, MDB_TASKS)<0) return -1;
	if (sqlUpdate(1, MDB_USERS)<0) return -1;
	return 0;
}
#endif

int restore_mysql(char *filename)
{
	printf("Restoring MySQL database from %s...", filename);
	sqlUpdate(0, "DROP TABLE gw_dbinfo;");
//	sqlUpdate(0, "DROP TABLE gw_calls;");
	sqlUpdate(0, "DROP TABLE gw_companies;");
	sqlUpdate(0, "DROP TABLE gw_contacts;");
	sqlUpdate(0, "DROP TABLE gw_events;");
	sqlUpdate(0, "DROP TABLE gw_forums;");
	sqlUpdate(0, "DROP TABLE gw_forumposts;");
	sqlUpdate(0, "DROP TABLE gw_links;");
	sqlUpdate(0, "DROP TABLE gw_messages;");
	sqlUpdate(0, "DROP TABLE gw_orders;");
	sqlUpdate(0, "DROP TABLE gw_orderdetails;");
	sqlUpdate(0, "DROP TABLE gw_products;");
	sqlUpdate(0, "DROP TABLE gw_queries;");
	sqlUpdate(0, "DROP TABLE gw_tasks;");
	sqlUpdate(0, "DROP TABLE gw_users;");
	if (sqlUpdate(1, MYSQLDB_DBINFO)<0) return -1;
//	if (sqlUpdate(1, MYSQLDB_CALLS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_COMPANIES)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_CONTACTS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_EVENTS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_FORUMS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_FORUMPOSTS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_LINKS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_MESSAGES)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_ORDERS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_ORDERDETAILS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_PRODUCTS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_QUERIES)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_TASKS)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_USERS)<0) return -1;
	return 0;
}

int restore_pgsql(char *filename)
{
	printf("Restoring PostgreSQL database from %s...", filename);
	sqlUpdate(0, "DROP TABLE gw_dbinfo;");
//	sqlUpdate(0, "DROP TABLE gw_calls;");
	sqlUpdate(0, "DROP TABLE gw_companies;");
	sqlUpdate(0, "DROP TABLE gw_contacts;");
	sqlUpdate(0, "DROP TABLE gw_events;");
	sqlUpdate(0, "DROP TABLE gw_forums;");
	sqlUpdate(0, "DROP TABLE gw_forumposts;");
	sqlUpdate(0, "DROP TABLE gw_links;");
	sqlUpdate(0, "DROP TABLE gw_messages;");
	sqlUpdate(0, "DROP TABLE gw_orders;");
	sqlUpdate(0, "DROP TABLE gw_orderdetails;");
	sqlUpdate(0, "DROP TABLE gw_products;");
	sqlUpdate(0, "DROP TABLE gw_queries;");
	sqlUpdate(0, "DROP TABLE gw_tasks;");
	sqlUpdate(0, "DROP TABLE gw_users;");
	if (sqlUpdate(1, PGSQLDB_SEQUENCES)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_DBINFO)<0) return -1;
//	if (sqlUpdate(1, PGSQLDB_CALLS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_COMPANIES)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_CONTACTS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_EVENTS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_FORUMS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_FORUMPOSTS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_LINKS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_MESSAGES)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_ORDERS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_ORDERDETAILS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_PRODUCTS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_QUERIES)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_TASKS)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_USERS)<0) return -1;
	return 0;
}

int restore_db(char *filename)
{
	char line[8192];
	char *pTemp;
	FILE *fp;

	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	if (strcasecmp(sql.sql_type, "MYSQL")==0) {
		if (restore_mysql(filename)<0) return -1;
#ifdef WIN32
	} else if (strcasecmp(sql.sql_type, "ODBC")==0) {
		if (restore_mdb(filename)<0) return -1;
#endif
	} else if (strcasecmp(sql.sql_type, "PGSQL")==0) {
		if (restore_pgsql(filename)<0) return -1;
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
	printf("done.\r\n");
	fclose(fp);
	sqlDisconnect();
	return 0;
}
