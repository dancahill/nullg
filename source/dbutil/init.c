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
int init_mdb(void)
{
	printf("Initialising ODBC *.mdb database...");
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
	if (sqlUpdate(1, MDB_DATA0)<0) return -1;
	if (sqlUpdate(1, MDB_DATA1)<0) return -1;
	if (sqlUpdate(1, MDB_DATA2)<0) return -1;
	sqlDisconnect();
	printf("done.\r\n");
	return 0;
}
#endif

int init_mysql(void)
{
	printf("Initialising MySQL database...");
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
	if (sqlUpdate(1, MYSQLDB_DATA0)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_DATA1)<0) return -1;
	if (sqlUpdate(1, MYSQLDB_DATA2)<0) return -1;
	sqlDisconnect();
	printf("done.\r\n");
	return 0;
}

int init_pgsql(void)
{
	printf("Initialising PostgreSQL database...");
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
	if (sqlUpdate(1, PGSQLDB_DATA0)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_DATA1)<0) return -1;
	if (sqlUpdate(1, PGSQLDB_DATA2)<0) return -1;
	sqlDisconnect();
	printf("done.\r\n");
	return 0;
}

int init_db(void)
{
	if (strcasecmp(sql.sql_type, "MYSQL")==0) {
		if (init_mysql()<0) return -1;
#ifdef WIN32
	} else if (strcasecmp(sql.sql_type, "ODBC")==0) {
		if (init_mdb()<0) return -1;
#endif
	} else if (strcasecmp(sql.sql_type, "PGSQL")==0) {
		if (init_pgsql()<0) return -1;
	}
	return 0;
}
