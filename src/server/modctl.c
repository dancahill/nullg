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

SRVMOD_EXEC mod_cron;
SRVMOD_EXEC mod_exec;
SRVMOD_EXEC mod_exit;
SRVMOD_INIT mod_init;
/*
 * using -rdynamic to link makes this semi-pointless, but we still need some
 * kind of reference to these functions, or they'll get optimized away...
 * oh..  and -rdynamic + openbsd = hardcore breakage
 */
static int regfunctions()
{
	static char loaded = 0;
	obj_t *tobj;

	if (loaded) return 0;
	loaded = 1;
	tobj = nsp_settable(proc.N, &proc.N->g, "_exports");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(proc.N, tobj, "core");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(proc.N, tobj, "config_read", (void *)config_read);
	nsp_setcfunc(proc.N, tobj, "config_write", (void *)config_write);
	nsp_setcfunc(proc.N, tobj, "decode_base64", (void *)decode_base64);
	nsp_setcfunc(proc.N, tobj, "dir_list", (void *)dir_list);
	nsp_setcfunc(proc.N, tobj, "domain_getname", (void *)domain_getname);
	nsp_setcfunc(proc.N, tobj, "domain_getid", (void *)domain_getid);
	nsp_setcfunc(proc.N, tobj, "language_gets", (void *)language_gets);
	nsp_setcfunc(proc.N, tobj, "lib_open", (void *)lib_open);
	nsp_setcfunc(proc.N, tobj, "lib_sym", (void *)lib_sym);
	nsp_setcfunc(proc.N, tobj, "lib_error", (void *)lib_error);
	nsp_setcfunc(proc.N, tobj, "lib_close", (void *)lib_close);
	nsp_setcfunc(proc.N, tobj, "log_access", (void *)log_access);
	nsp_setcfunc(proc.N, tobj, "log_error", (void *)log_error);
	nsp_setcfunc(proc.N, tobj, "md5_init", (void *)md5_init);
	nsp_setcfunc(proc.N, tobj, "md5_update", (void *)md5_update);
	nsp_setcfunc(proc.N, tobj, "md5_final", (void *)md5_final);
	nsp_setcfunc(proc.N, tobj, "md5_crypt", (void *)md5_crypt);

	nsp_setcfunc(proc.N, tobj, "nsp_newstate", (void *)nsp_newstate);
	nsp_setcfunc(proc.N, tobj, "nsp_endstate", (void *)nsp_endstate);
	nsp_setcfunc(proc.N, tobj, "nsp_exec", (void *)nsp_exec);
	nsp_setcfunc(proc.N, tobj, "nsp_execfile", (void *)nsp_execfile);

	nsp_setcfunc(proc.N, tobj, "nsp_setvaltype", (void *)nsp_setvaltype);
	nsp_setcfunc(proc.N, tobj, "nsp_linkval", (void *)nsp_linkval);
	nsp_setcfunc(proc.N, tobj, "nsp_unlinkval", (void *)nsp_unlinkval);
	nsp_setcfunc(proc.N, tobj, "nsp_freetable", (void *)nsp_freetable);
	nsp_setcfunc(proc.N, tobj, "nsp_getobj", (void *)nsp_getobj);
	nsp_setcfunc(proc.N, tobj, "nsp_getiobj", (void *)nsp_getiobj);
	nsp_setcfunc(proc.N, tobj, "nsp_setobj", (void *)nsp_setobj);
	nsp_setcfunc(proc.N, tobj, "nsp_strcat", (void *)nsp_strcat);
	nsp_setcfunc(proc.N, tobj, "nsp_strmul", (void *)nsp_strmul);
	nsp_setcfunc(proc.N, tobj, "nsp_tobool", (void *)nsp_tobool);
	nsp_setcfunc(proc.N, tobj, "nsp_tonum", (void *)nsp_tonum);
	nsp_setcfunc(proc.N, tobj, "nsp_tostr", (void *)nsp_tostr);
	nsp_setcfunc(proc.N, tobj, "nsp_eval", (void *)nsp_eval);
	nsp_setcfunc(proc.N, tobj, "nsp_evalf", (void *)nsp_evalf);

	nsp_setcfunc(proc.N, tobj, "sql_freeresult", (void *)sql_freeresult);
	nsp_setcfunc(proc.N, tobj, "sql_update", (void *)sql_update);
	nsp_setcfunc(proc.N, tobj, "sql_updatef", (void *)sql_updatef);
	nsp_setcfunc(proc.N, tobj, "sql_query", (void *)sql_query);
	nsp_setcfunc(proc.N, tobj, "sql_queryf", (void *)sql_queryf);
	nsp_setcfunc(proc.N, tobj, "sql_getsequence", (void *)sql_getsequence);
	nsp_setcfunc(proc.N, tobj, "sql_getname", (void *)sql_getname);
	nsp_setcfunc(proc.N, tobj, "sql_getvalue", (void *)sql_getvalue);
	nsp_setcfunc(proc.N, tobj, "sql_getvaluebyname", (void *)sql_getvaluebyname);
	nsp_setcfunc(proc.N, tobj, "sql_numfields", (void *)sql_numfields);
	nsp_setcfunc(proc.N, tobj, "sql_numtuples", (void *)sql_numtuples);
	nsp_setcfunc(proc.N, tobj, "sys_system", (void *)sys_system);

	nsp_setcfunc(proc.N, tobj, "ssl_connect", (void *)ssl_connect);

	nsp_setcfunc(proc.N, tobj, "tcp_bind", (void *)tcp_bind);
	nsp_setcfunc(proc.N, tobj, "tcp_accept", (void *)tcp_accept);
	nsp_setcfunc(proc.N, tobj, "tcp_connect", (void *)tcp_connect);
	nsp_setcfunc(proc.N, tobj, "tcp_fgets", (void *)tcp_fgets);
	nsp_setcfunc(proc.N, tobj, "tcp_fprintf", (void *)tcp_fprintf);
	nsp_setcfunc(proc.N, tobj, "tcp_recv", (void *)tcp_recv);
	nsp_setcfunc(proc.N, tobj, "tcp_send", (void *)tcp_send);
	nsp_setcfunc(proc.N, tobj, "tcp_close", (void *)tcp_close);
	nsp_setcfunc(proc.N, tobj, "time_sql2unix", (void *)time_sql2unix);
	nsp_setcfunc(proc.N, tobj, "time_unix2sql", (void *)time_unix2sql);
	nsp_setcfunc(proc.N, tobj, "strncatf", (void *)strncatf);
	nsp_setcfunc(proc.N, tobj, "striprn", (void *)striprn);
	nsp_setcfunc(proc.N, tobj, "fixslashes", (void *)fixslashes);
	nsp_setcfunc(proc.N, tobj, "str2sql", (void *)str2sql);
	nsp_setcfunc(proc.N, tobj, "p_strcasestr", (void *)p_strcasestr);
	nsp_setcfunc(proc.N, tobj, "addlistener", (void *)addlistener);
#ifdef HAVE_SSL
	nsp_setcfunc(proc.N, tobj, "ssl_accept", (void *)ssl_accept);
	nsp_setcfunc(proc.N, tobj, "ssl_close", (void *)ssl_close);
	nsp_setcfunc(proc.N, tobj, "ssl_shutdown", (void *)ssl_shutdown);
#endif
#ifdef WIN32
	nsp_setcfunc(proc.N, tobj, "gettimeofday", (void *)gettimeofday);
#endif
	return 0;
}

#ifdef WIN32
unsigned _stdcall cronloop(void *x)
#else
void *cronloop(void *x)
#endif
{
	time_t oldt, newt;
	int i;

	log_error(proc.N, "core", __FILE__, __LINE__, 2, "Starting cronloop() thread");
	oldt = (time(NULL) / 60 - 1) * 60;
	for (;;) {
		newt = (time(NULL) / 60) * 60;
		if (newt > oldt) {
			oldt = newt;
			nsdevents_timer();
		}
		sleep(1);
		for (i = 0;i < MAX_MOD_FUNCTIONS;i++) {
			if (strlen(proc.srvmod[i].mod_name) < 1) break;
			if (proc.srvmod[i].mod_cron == NULL) continue;
			if ((mod_cron = (SRVMOD_EXEC)proc.srvmod[i].mod_cron) == NULL) break;
			if (mod_cron() != 0) break;
		}
	}
	return 0;
}

int modules_cron()
{
	pthread_attr_t thr_attr;

	if (pthread_attr_init(&thr_attr)) return -2;
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	if (pthread_attr_setstacksize(&thr_attr, PTHREAD_STACK_SIZE)) return -2;
#endif
	if (pthread_create(&CronThread, &thr_attr, cronloop, NULL) == -1) {
		log_error(proc.N, NULL, __FILE__, __LINE__, 0, "cronloop() failed...");
		return -2;
	}
	return 0;
}

int modules_exec()
{
	int i;

	for (i = 0;i < MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exec == NULL) break;
		if (strlen(proc.srvmod[i].mod_name) < 1) break;
		if ((mod_exec = (SRVMOD_EXEC)proc.srvmod[i].mod_exec) == NULL) return -1;
		if (mod_exec() != 0) return -1;
	}
	return 0;
}

int modules_exit()
{
	int i;

	for (i = 0;i < MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exit == NULL) break;
		if (strlen(proc.srvmod[i].mod_name) < 1) break;
		if ((mod_exit = (SRVMOD_EXEC)proc.srvmod[i].mod_exit) == NULL) return -1;
		if (mod_exit() != 0) return -1;
	}
	return 0;
}

int module_load(char *modname)
{
	obj_t *confobj = nsp_getobj(proc.N, &proc.N->g, "CONFIG");
#ifdef WIN32
	HINSTANCE hinstLib = NULL;
	char *ext = "dll";
#else
	void *hinstLib = NULL;
	char *ext = "so";
#endif
	char libname[255];
	int i;
	void *srv_init;
	void *srv_exec;
	void *srv_cron;
	void *srv_exit;

	regfunctions();
	for (i = 0;i < MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exec == NULL) break;
		if (strlen(proc.srvmod[i].mod_name) < 1) continue;
	}
	if (i == MAX_MOD_FUNCTIONS) return -1;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname) - 1, "%s/core/%s.%s", nsp_getstr(proc.N, nsp_getobj(proc.N, confobj, "paths"), "lib"), modname, ext);
	fixslashes(libname);
	snprintf(proc.srvmod[i].mod_name, sizeof(proc.srvmod[i].mod_name) - 1, "%s", modname);
	if ((hinstLib = lib_open(libname)) == NULL) {
#ifdef WIN32
		printf("%s(%d)(%d) [%s]\r\n", __FILE__, __LINE__, GetLastError(), libname);
#endif
		goto fail;
	}
	if ((srv_init = (void *)lib_sym(hinstLib, "mod_init")) == NULL) goto fail;
	if ((srv_exec = (void *)lib_sym(hinstLib, "mod_exec")) == NULL) goto fail;
	if ((srv_exit = (void *)lib_sym(hinstLib, "mod_exit")) == NULL) goto fail;
	srv_cron = (void *)lib_sym(hinstLib, "mod_cron");
	proc.srvmod[i].mod_init = srv_init;
	proc.srvmod[i].mod_exec = srv_exec;
	proc.srvmod[i].mod_exit = srv_exit;
	proc.srvmod[i].mod_cron = srv_cron;
	proc.srvmod[i].hinstLib = hinstLib;
	mod_init = (SRVMOD_INIT)srv_init;
	if (mod_init(&proc) != 0) return -1;
	return 0;
fail:
	log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s '%s'", modname, ext, lib_error());
	if (hinstLib != NULL) lib_close(hinstLib);
	hinstLib = NULL;
	return -1;
}

int modules_init(nsp_state *N)
{
	obj_t *cobj, *tobj;

	tobj = nsp_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type != NT_TABLE) return 0;
	tobj = nsp_getobj(N, tobj, "modules");
	if (tobj->val->type != NT_TABLE) return 0;
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (cobj->name[0] == '_') continue;
		if (nsp_isstr(cobj)) module_load(cobj->val->d.str);
		else if (nsp_istable(cobj)) module_load(cobj->name);
	}
	return 0;
}
