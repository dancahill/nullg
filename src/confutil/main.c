/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "nullgs/config-nt.h"
#else
#include "nullgs/config.h"
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
	#include <dirent.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/stat.h>
#endif
#include "nullgs/defines.h"

#include "nesla/nesla.h"
#include "ngs.h"

FILE *fp=NULL;

void dumpvars(nes_state *N, obj_t *tobj, int depth)
{
	obj_t *cobj=tobj;
	char indent[20];
	int i;
	char b;
	char *g;
	char *l;

	for (i=0;i<depth;i++) indent[i]='\t';
	indent[i]='\0';
	for (;cobj;cobj=cobj->next) {
		if ((cobj->val->attr&NST_HIDDEN)||(cobj->val->attr&NST_SYSTEM)) continue;
		if ((depth==1)&&(strcmp(cobj->name, "filename")==0)) continue;
		g=(depth<1)?"global ":"";
		l=(depth<1)?";":(cobj->next)?",":"";
		if (isdigit(cobj->name[0])) b=1; else b=0;
		if ((cobj->val->type==NT_NULL)||(cobj->val->type==NT_BOOLEAN)||(cobj->val->type==NT_NUMBER)) {
			fprintf(fp, "%s%s%s%s%s = ", indent, g, b?"[":"", cobj->name, b?"]":"");
			fprintf(fp, "%s%s\n", nes_tostr(N, cobj), l);
		} else if (cobj->val->type==NT_STRING) {
			fprintf(fp, "%s%s%s%s%s = ", indent, g, b?"[":"", cobj->name, b?"]":"");
			fprintf(fp, "\"%s\"%s\n", nes_tostr(N, cobj), l);
		} else if (cobj->val->type==NT_TABLE) {
			if (strcmp(cobj->name, "_GLOBALS")==0) continue;
			fprintf(fp, "%s%s%s%s%s = {\n", indent, g, b?"[":"", cobj->name, b?"]":"");
			dumpvars(N, cobj->val->d.table.f, depth+1);
			fprintf(fp, "%s}%s\n", indent, l);
		}
	}
	return;
}

int savefile(nes_state *N)
{
	obj_t *cobj, *tobj;

	tobj=nes_getobj(N, &N->g, "CONFIG");
	if (!nes_istable(tobj)) return 0;
	cobj=nes_getobj(N, tobj, "filename");
	if (nes_isstr(cobj)) {
		if ((fp=fopen(nes_tostr(N, cobj), "w"))==NULL) {
			printf("Error saving %s\r\n", nes_tostr(N, cobj));
			return -1;
		}
		fprintf(fp, "# This file contains config settings for NullLogic GroupServer.\n\n");
		dumpvars(N, N->g.val->d.table.f, 0);
		fclose(fp);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	nes_state *N;
	obj_t *tobj;
	char line[PATH_MAX];
	char sql_type[64];
	char sql_host[64];
	char sql_db[64];
	char sql_user[64];
	char sql_pass[64];
	int sql_port;

	if ((N=nes_newstate())==NULL) {
		printf("nes_newstate() error\r\n");
		return -1;
	}
	memset(sql_type, 0, sizeof(sql_type));
	memset(sql_host, 0, sizeof(sql_host));
	memset(sql_db,   0, sizeof(sql_db));
	memset(sql_user, 0, sizeof(sql_user));
	memset(sql_pass, 0, sizeof(sql_pass));
	if ((argc==2)&&(strcasecmp(argv[1], "upgrade")==0)) {
		return 0;
	}
	if (config_read(N, "", NULL)<0) {
		printf("can't read the config file.\r\n");
		return 0;
	}
	if ((argc>2)&&(strcasecmp(argv[1], "show")==0)) {
		snprintf(line, sizeof(line)-1, "print(%s);", argv[2]);
		nes_exec(N, line);
		return 0;
	}
	tobj=nes_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type!=NT_TABLE) return 0;
	printf("NullLogic GroupServer Configuration\r\n");
	printf("===================================\r\n");
	snprintf(sql_type, sizeof(sql_type)-1, "%s", nes_getstr(N, tobj, "sql_server_type"));
	if (strlen(sql_type)==0) snprintf(sql_type, sizeof(sql_type)-1, "SQLITE");
/*
	fp=stdout;
	dumpvars(N, N->g.d.table, 0);
*/
	printf("Will you be using SQLITE, MYSQL or PGSQL [%s]: ", sql_type);
	fgets(line, sizeof(line)-1, stdin);
	striprn(line);
	if (strlen(line)>0) snprintf(sql_type, sizeof(sql_type)-1, "%s", line);
	nes_setstr(N, tobj, "sql_server_type", sql_type, strlen(sql_type));
	if (strcasecmp(sql_type, "MYSQL")==0) {
		snprintf(sql_host, sizeof(sql_host)-1, "%s", nes_getstr(N, tobj, "sql_host_name"));
		if (strlen(sql_host)==0) snprintf(sql_host, sizeof(sql_host)-1, "localhost");
		sql_port=(int)nes_getnum(N, tobj, "sql_port");
		if (sql_port==0) sql_port=3306;
		snprintf(sql_db,   sizeof(sql_db)-1, "%s", nes_getstr(N, tobj, "sql_database_name"));
		if (strlen(sql_db)==0) snprintf(sql_db,   sizeof(sql_db)-1,   "nullgs");
		snprintf(sql_user, sizeof(sql_user)-1, "%s", nes_getstr(N, tobj, "sql_user_name"));
		if (strlen(sql_user)==0) snprintf(sql_user, sizeof(sql_user)-1, "nobody");
		snprintf(sql_pass, sizeof(sql_pass)-1, "%s", nes_getstr(N, tobj, "sql_password"));
		if (strlen(sql_pass)==0) snprintf(sql_pass, sizeof(sql_pass)-1, "password");
	} else if (strcasecmp(sql_type, "PGSQL")==0) {
		snprintf(sql_host, sizeof(sql_host)-1, "%s", nes_getstr(N, tobj, "sql_host_name"));
		if (strlen(sql_host)==0) snprintf(sql_host, sizeof(sql_host)-1, "localhost");
		sql_port=(int)nes_getnum(N, tobj, "sql_port");
		if (sql_port==0) sql_port=5432;
		snprintf(sql_db,   sizeof(sql_db)-1, "%s", nes_getstr(N, tobj, "sql_database_name"));
		if (strlen(sql_db)==0) snprintf(sql_db,   sizeof(sql_db)-1,   "nullgs");
		snprintf(sql_user, sizeof(sql_user)-1, "%s", nes_getstr(N, tobj, "sql_user_name"));
		if (strlen(sql_user)==0) snprintf(sql_user, sizeof(sql_user)-1, "nobody");
		snprintf(sql_pass, sizeof(sql_pass)-1, "%s", nes_getstr(N, tobj, "sql_password"));
		if (strlen(sql_pass)==0) snprintf(sql_pass, sizeof(sql_pass)-1, "password");
	} else if (strcasecmp(sql_type, "SQLITE")==0) {
		sql_port=0;
	} else {
		printf("'%s' is an unknown SQL server type.\r\n", sql_type);
		return -1;
	}
	if ((strcasecmp(sql_type, "MYSQL")==0)||(strcasecmp(sql_type, "PGSQL")==0)) {
		printf("What is the host name of the %s server? [%s]: ", sql_type, sql_host);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_host, sizeof(sql_host)-1, "%s", line);
		nes_setstr(N, tobj, "sql_host_name", sql_host, strlen(sql_host));
		printf("What TCP/IP port is the %s server listening on? [%d]: ", sql_type, sql_port);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) sql_port=atoi(line);
		snprintf(line, sizeof(line)-1, "%d", sql_port);
		nes_setnum(N, tobj, "sql_port", atoi(line));
		printf("What is the name of the database NullLogic GroupServer should use? [%s]: ", sql_db);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_db, sizeof(sql_db)-1, "%s", line);
		nes_setstr(N, tobj, "sql_database_name", sql_db, strlen(sql_db));
		printf("What username should NullLogic GroupServer use to access the database? [%s]: ", sql_user);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_user, sizeof(sql_user)-1, "%s", line);
		nes_setstr(N, tobj, "sql_user_name", sql_user, strlen(sql_user));
		printf("What is the password for the '%s' account? [%s]: ", sql_user, sql_pass);
		fgets(line, sizeof(line)-1, stdin);
		striprn(line);
		if (strlen(line)>0) snprintf(sql_pass, sizeof(sql_pass)-1, "%s", line);
		nes_setstr(N, tobj, "sql_password", sql_pass, strlen(sql_pass));
		printf("\r\n");
		printf("In order to complete the install, you will need to manually create the\r\n");
		printf("SQL database %s as follows.\r\n", sql_db);
		if (strcasecmp(sql_type, "MYSQL")==0) {
			printf("  mysqladmin create %s\r\n", sql_db);
		} else if (strcasecmp(sql_type, "PGSQL")==0) {
			printf("  createdb %s\r\n", sql_db);
		}
		printf("\r\n");
		printf("After creating the database, please run nullgs-dbutil to\r\n");
		printf("initialise the database.\r\n");
		printf("\r\n");
	}
	if (savefile(N)<0) return -1;
	if (N->err) {
		printf("errno=%d :: \"%s\"\r\n", N->err, N->errbuf);
	}
	N=nes_endstate(N);
	return 0;
}
