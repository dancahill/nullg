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
#include "main.h"

static int sanity_dircheck(const char *format, ...)
{
	obj_t *tobj=nsp_getobj(proc.N, &proc.N->g, "CONFIG");
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
	if (mkdir(dirname, ~(int)nsp_getnum(proc.N, tobj, "umask")&0777)!=0) {
#endif
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "Error accessing directory '%s'", dirname);
		return -1;
	}
	return 0;
}

static int sanity_dbcheck_table(char *tablename, char *indexname, int numfields)
{
	obj_t *qobj=NULL;
	int nfields;

	if (sql_queryf(proc.N, &qobj, "SELECT * FROM %s WHERE %s = 1", tablename, indexname)<0) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: Could not access %s table!", tablename);
		exit(-2);
	}
	nfields=sql_numfields(proc.N, &qobj);
	sql_freeresult(proc.N, &qobj);
	if (nfields!=numfields) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: %s has %d fields, but it should have %d!", tablename, nfields, numfields);
		return -1;
	}
	return 0;
}

int sanity_checkdb()
{
	obj_t *tobj=nsp_getobj(proc.N, &proc.N->g, "CONFIG");
	char cmdline[200];
	char dbfile[200];
	char msgbuf[200];
	obj_t *qobj=NULL;
	struct stat sb;
	int i;
	int errs=0;

	if (strncmp(nsp_getstr(proc.N, tobj, "sql_server_type"), "SQLITE", 6)==0) {
		if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_path"))!=0) exit(-2);
		if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_db_path"))!=0) exit(-2);
		if (strcmp(nsp_getstr(proc.N, tobj, "sql_server_type"), "SQLITE2")==0) {
			snprintf(dbfile, sizeof(dbfile)-1, "%s/%s.db2", nsp_getstr(proc.N, tobj, "var_db_path"), SERVER_BASENAME);
		} else {
			snprintf(dbfile, sizeof(dbfile)-1, "%s/%s.db3", nsp_getstr(proc.N, tobj, "var_db_path"), SERVER_BASENAME);
		}
		fixslashes(dbfile);
		if ((stat(dbfile, &sb)!=0)||(sb.st_size==0)) {
#ifdef WIN32
			snprintf(cmdline, sizeof(cmdline)-1, "./nullsd-dbutil.exe init");
#else
			snprintf(cmdline, sizeof(cmdline)-1, "./nullsd-dbutil init");
#endif
			fixslashes(cmdline);
#ifndef WIN32
			printf("\r\n");
#endif
			system(cmdline);
		}

	}
	i=0;
	if (proc.debug) printf("Testing DB tables\r\n");
	while (sql_query(proc.N, &qobj, "SELECT COUNT(*) FROM nullsd_entries")<0) {
		if (proc.debug) printf("%s %d blah\r\n", __FILE__, __LINE__);
		sleep(2);
		i++;
		if (i>5) {
			snprintf(msgbuf, sizeof(msgbuf)-1, "%s responded abnormally.", nsp_getstr(proc.N, tobj, "sql_server_type"));
			log_error(proc.N, "core", __FILE__, __LINE__, 0, "%s", msgbuf);
			strcat(msgbuf, "\r\nSee error.log for more information on this error.");
			printf("\r\n%s\r\n", msgbuf);
			exit(-1);
		}
	}
	sql_freeresult(proc.N, &qobj);
	if (proc.debug) printf("Testing more DB tables\r\n");
	if (sanity_dbcheck_table("nullsd_entries",  "id", 8)==-1) errs++;
	if (sanity_dbcheck_table("nullsd_sessions", "id", 8)==-1) errs++;
	if (errs!=0) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "Please use dbutil to dump and restore the database");
		exit(-2);
	}
	if (proc.debug) printf("Done testing DB tables\r\n");
	return 0;
}

int sanity_checkdirs()
{
	obj_t *tobj=nsp_getobj(proc.N, &proc.N->g, "CONFIG");
	obj_t *qobj=NULL;
	int domainid;
	int i;

	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_path"))!=0) exit(-2);
	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_db_path"))!=0) exit(-2);
	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_domains_path"))!=0) exit(-2);
	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_log_path"))!=0) exit(-2);
	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_spool_path"))!=0) exit(-2);
	if (sanity_dircheck("%s/mqueue", nsp_getstr(proc.N, tobj, "var_spool_path"))!=0) exit(-2);
	if (sanity_dircheck("%s/mqinfo", nsp_getstr(proc.N, tobj, "var_spool_path"))!=0) exit(-2);
	if (sanity_dircheck("%s", nsp_getstr(proc.N, tobj, "var_tmp_path"))!=0) exit(-2);
return 0;
	if (sql_query(proc.N, &qobj, "SELECT id, name FROM nullsd_entries WHERE class = 'organization'")<0) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "Could not retrieve domain list");
		exit(-1);
	}
	if (sql_numtuples(proc.N, &qobj)<1) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "no domains exist"); exit(-1);
		sql_update(proc.N, "INSERT INTO nullsd_entries (id, pid, did, name, class, data) VALUES (1, 0, 1, \"default domain\", \"organization\", \"{}\");");
		sql_update(proc.N, "INSERT INTO nullsd_entries (id, pid, did, name, class, data) VALUES (2, 1, 1, \"localhost\", \"associatedDomain\", \"{}\");");
	}
	for (i=0;i<sql_numtuples(proc.N, &qobj);i++) {
		domainid=atoi(sql_getvalue(proc.N, &qobj, i, 0));
		if (sanity_dircheck("%s/%04d",             nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/attachments", nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/cgi-bin",     nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/files",       nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/htdocs",      nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/mail",        nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
		if (sanity_dircheck("%s/%04d/mailspool",   nsp_getstr(proc.N, tobj, "var_domains_path"), domainid)!=0) exit(-2);
	}
	sql_freeresult(proc.N, &qobj);
	return 0;
}
