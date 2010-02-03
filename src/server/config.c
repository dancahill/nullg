/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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

static void addpath(nsp_state *N, obj_t *tobj, char *name, const char *format, ...)
{
	obj_t *cobj;
	char tmpbuf[255];
	va_list ap;
	char *ptemp;

	memset(tmpbuf, 0, sizeof(tmpbuf));
	va_start(ap, format);
	vsnprintf(tmpbuf, sizeof(tmpbuf)-1, format, ap);
	va_end(ap);
	ptemp=tmpbuf;
 	while (*ptemp) {
 		if (*ptemp=='\\') *ptemp='/';
		ptemp++;
	}
	cobj=nsp_getobj(N, tobj, name);
	if (cobj->val->type==NT_NULL) nsp_setstr(N, tobj, name, tmpbuf, strlen(tmpbuf));
	return;
}

int conf_read(nsp_state *N)
{
	obj_t *tobj;
	obj_t *cobj;
	char basepath[255];
	char tmpbuf[255];
	char *ptemp;

	config_read(N, "", NULL);
	tobj=nsp_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type==NT_NULL) tobj=nsp_settable(N, &N->g, "CONFIG");
	/* define default values */
	memset(tmpbuf, 0, sizeof(tmpbuf));
	gethostname(tmpbuf, sizeof(tmpbuf)-1);
	nsp_setstr(N, tobj, "host_name", tmpbuf, -1);
	nsp_setstr(N, tobj, "uid", DEFAULT_SERVER_USERNAME, -1);
	nsp_setstr(N, tobj, "default_language", DEFAULT_SERVER_LANGUAGE, -1);
	/* try to find our way into the program's BIN dir */
	ptemp=nsp_getstr(N, &N->g, "program_name");
	fixslashes(ptemp);
	if (*ptemp=='\"') ptemp++;
	snprintf(tmpbuf, sizeof(tmpbuf)-1, "%s", ptemp);
	if ((ptemp=strrchr(tmpbuf, '/'))!=NULL) {
		*ptemp='\0';
	} else {
		/* no slash means no path.  assume we're already in BIN */
#ifdef WIN32
		GetCurrentDirectory(sizeof(tmpbuf)-1, tmpbuf);
#else
		if (getcwd(tmpbuf, sizeof(tmpbuf)-1)==NULL) return -1;
#endif
	}
	fixslashes(tmpbuf);
	/* either way, change to BIN */
	if (chdir(tmpbuf)!=0) {
		log_error(N, "core", __FILE__, __LINE__, 1, "can't chdir(\"%s\")", tmpbuf);
		return -1;
	}
	memset(basepath, 0, sizeof(basepath));
	snprintf(basepath, sizeof(basepath)-1, "%s", tmpbuf);
	if ((ptemp=strrchr(basepath, '/'))!=NULL) *ptemp='\0';
	addpath(N, tobj, "bin_path", "%s/bin", basepath);
	addpath(N, tobj, "etc_path", "%s/etc", basepath);
	addpath(N, tobj, "lib_path", "%s/lib", basepath);
	addpath(N, tobj, "var_path", "%s/var", basepath);
	cobj=nsp_getobj(N, tobj, "sql_server_type");
	if (cobj->val->type==NT_NULL) nsp_setstr(N, tobj, "sql_server_type", "SQLITE", 6);
	cobj=nsp_getobj(N, tobj, "log_level");
	if (cobj->val->type==NT_NULL) nsp_setnum(N, tobj, "log_level", 1);
	cobj=nsp_getobj(N, tobj, "umask");
	if (cobj->val->type==NT_NULL) nsp_setnum(N, tobj, "umask", 0007);
	strncpy(basepath, nsp_getstr(N, tobj, "var_path"), sizeof(basepath)-1);
	addpath(N, tobj, "var_backup_path",  "%s/backup",        basepath);
	addpath(N, tobj, "var_cgi_path",     "%s/share/cgi-bin", basepath);
	addpath(N, tobj, "var_db_path",      "%s/db",            basepath);
	addpath(N, tobj, "var_domains_path", "%s/domains",       basepath);
	addpath(N, tobj, "var_htdocs_path",  "%s/share/htdocs",  basepath);
	addpath(N, tobj, "var_log_path",     "%s/log",           basepath);
	addpath(N, tobj, "var_spool_path",   "%s/spool",         basepath);
	addpath(N, tobj, "var_tmp_path",     "%s/tmp",           basepath);
	return 0;
}
