/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
 */
static int regfunctions()
{
	static char loaded=0;
	obj_t *tobj;

	if (loaded) return 0;
	loaded=1;
	tobj=nes_settable(proc.N, &proc.N->g, "_exports");
	tobj->val->attr|=NST_HIDDEN;
	tobj=nes_settable(proc.N, tobj, "core");
	tobj->val->attr|=NST_HIDDEN;
	nes_setcfunc(proc.N, tobj, "config_read",		(void *)config_read);
	nes_setcfunc(proc.N, tobj, "config_write",		(void *)config_write);
	nes_setcfunc(proc.N, tobj, "decode_base64",		(void *)decode_base64);
	nes_setcfunc(proc.N, tobj, "dir_list",			(void *)dir_list);
	nes_setcfunc(proc.N, tobj, "domain_getname",		(void *)domain_getname);
	nes_setcfunc(proc.N, tobj, "domain_getid",		(void *)domain_getid);
	nes_setcfunc(proc.N, tobj, "language_gets",		(void *)language_gets);
	nes_setcfunc(proc.N, tobj, "lib_open",			(void *)lib_open);
	nes_setcfunc(proc.N, tobj, "lib_sym",			(void *)lib_sym);
	nes_setcfunc(proc.N, tobj, "lib_error",			(void *)lib_error);
	nes_setcfunc(proc.N, tobj, "lib_close",			(void *)lib_close);
	nes_setcfunc(proc.N, tobj, "log_access",		(void *)log_access);
	nes_setcfunc(proc.N, tobj, "log_error",			(void *)log_error);
	nes_setcfunc(proc.N, tobj, "md5_init",			(void *)md5_init);
	nes_setcfunc(proc.N, tobj, "md5_update",		(void *)md5_update);
	nes_setcfunc(proc.N, tobj, "md5_final",			(void *)md5_final);
	nes_setcfunc(proc.N, tobj, "md5_crypt",			(void *)md5_crypt);

	nes_setcfunc(proc.N, tobj, "nes_newstate",		(void *)nes_newstate);
	nes_setcfunc(proc.N, tobj, "nes_endstate",		(void *)nes_endstate);
	nes_setcfunc(proc.N, tobj, "nes_exec",			(void *)nes_exec);
	nes_setcfunc(proc.N, tobj, "nes_execfile",		(void *)nes_execfile);
	nes_setcfunc(proc.N, tobj, "nes_linkval",		(void *)nes_linkval);
	nes_setcfunc(proc.N, tobj, "nes_unlinkval",		(void *)nes_unlinkval);
	nes_setcfunc(proc.N, tobj, "nes_freetable",		(void *)nes_freetable);
	nes_setcfunc(proc.N, tobj, "nes_getobj",		(void *)nes_getobj);
	nes_setcfunc(proc.N, tobj, "nes_getiobj",		(void *)nes_getiobj);
	nes_setcfunc(proc.N, tobj, "nes_setobj",		(void *)nes_setobj);
	nes_setcfunc(proc.N, tobj, "nes_strcat",		(void *)nes_strcat);
	nes_setcfunc(proc.N, tobj, "nes_tonum",			(void *)nes_tonum);
	nes_setcfunc(proc.N, tobj, "nes_tostr",			(void *)nes_tostr);
	nes_setcfunc(proc.N, tobj, "nes_eval",			(void *)nes_eval);
	nes_setcfunc(proc.N, tobj, "nes_evalf",			(void *)nes_evalf);

	nes_setcfunc(proc.N, tobj, "sql_freeresult",		(void *)sql_freeresult);
	nes_setcfunc(proc.N, tobj, "sql_update",		(void *)sql_update);
	nes_setcfunc(proc.N, tobj, "sql_updatef",		(void *)sql_updatef);
	nes_setcfunc(proc.N, tobj, "sql_query",			(void *)sql_query);
	nes_setcfunc(proc.N, tobj, "sql_queryf",		(void *)sql_queryf);
	nes_setcfunc(proc.N, tobj, "sql_getname",		(void *)sql_getname);
	nes_setcfunc(proc.N, tobj, "sql_getvalue",		(void *)sql_getvalue);
	nes_setcfunc(proc.N, tobj, "sql_getvaluebyname",	(void *)sql_getvaluebyname);
	nes_setcfunc(proc.N, tobj, "sql_numfields",		(void *)sql_numfields);
	nes_setcfunc(proc.N, tobj, "sql_numtuples",		(void *)sql_numtuples);
	nes_setcfunc(proc.N, tobj, "sys_system",		(void *)sys_system);
	nes_setcfunc(proc.N, tobj, "tcp_bind",			(void *)tcp_bind);
	nes_setcfunc(proc.N, tobj, "tcp_accept",		(void *)tcp_accept);
	nes_setcfunc(proc.N, tobj, "tcp_connect",		(void *)tcp_connect);
	nes_setcfunc(proc.N, tobj, "tcp_fgets",			(void *)tcp_fgets);
	nes_setcfunc(proc.N, tobj, "tcp_fprintf",		(void *)tcp_fprintf);
	nes_setcfunc(proc.N, tobj, "tcp_recv",			(void *)tcp_recv);
	nes_setcfunc(proc.N, tobj, "tcp_send",			(void *)tcp_send);
	nes_setcfunc(proc.N, tobj, "tcp_close",			(void *)tcp_close);
	nes_setcfunc(proc.N, tobj, "time_sql2unix",		(void *)time_sql2unix);
	nes_setcfunc(proc.N, tobj, "time_unix2sql",		(void *)time_unix2sql);
	nes_setcfunc(proc.N, tobj, "strncatf",			(void *)strncatf);
	nes_setcfunc(proc.N, tobj, "striprn",			(void *)striprn);
	nes_setcfunc(proc.N, tobj, "fixslashes",		(void *)fixslashes);
	nes_setcfunc(proc.N, tobj, "str2sql",			(void *)str2sql);
	nes_setcfunc(proc.N, tobj, "p_strcasestr",		(void *)p_strcasestr);
	nes_setcfunc(proc.N, tobj, "addlistener",		(void *)addlistener);
#ifdef HAVE_SSL
	nes_setcfunc(proc.N, tobj, "ssl_accept",		(void *)ssl_accept);
	nes_setcfunc(proc.N, tobj, "ssl_close",			(void *)ssl_close);
	nes_setcfunc(proc.N, tobj, "ssl_shutdown",		(void *)ssl_shutdown);
#endif
#ifdef WIN32
	nes_setcfunc(proc.N, tobj, "gettimeofday",		(void *)gettimeofday);
#endif
	return 0;
}

#ifdef WIN32
unsigned _stdcall cronloop(void *x)
#else
void *cronloop(void *x)
#endif
{
	int i;

	log_error(proc.N, "core", __FILE__, __LINE__, 2, "Starting cronloop() thread");
	for (;;) {
		sleep(1);
		for (i=0;i<MAX_MOD_FUNCTIONS;i++) {
			if (strlen(proc.srvmod[i].mod_name)<1) break;
			if (proc.srvmod[i].mod_cron==NULL) continue;
			if ((mod_cron=(SRVMOD_EXEC)proc.srvmod[i].mod_cron)==NULL) break;
			if (mod_cron()!=0) break;
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
	if (pthread_create(&CronThread, &thr_attr, cronloop, NULL)==-1) {
		log_error(proc.N, NULL, __FILE__, __LINE__, 0, "cronloop() failed...");
		return -2;
	}
	return 0;
}

int modules_exec()
{
	int i;

	for (i=0;i<MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exec==NULL) break;
		if (strlen(proc.srvmod[i].mod_name)<1) break;
		if ((mod_exec=(SRVMOD_EXEC)proc.srvmod[i].mod_exec)==NULL) return -1;
		if (mod_exec()!=0) return -1;
	}
	return 0;
}

int modules_exit()
{
	int i;

	for (i=0;i<MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exit==NULL) break;
		if (strlen(proc.srvmod[i].mod_name)<1) break;
		if ((mod_exit=(SRVMOD_EXEC)proc.srvmod[i].mod_exit)==NULL) return -1;
		if (mod_exit()!=0) return -1;
	}
	return 0;
}

int module_load(char *modname)
{
	obj_t *confobj=nes_getobj(proc.N, &proc.N->g, "CONFIG");
#ifdef WIN32
	HINSTANCE hinstLib=NULL;
	char *ext="dll";
#else
	void *hinstLib=NULL;
	char *ext="so";
#endif
	char libname[255];
	int i;
	void *srv_init;
	void *srv_exec;
	void *srv_cron;
	void *srv_exit;

	regfunctions();
	for (i=0;i<MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exec==NULL) break;
		if (strlen(proc.srvmod[i].mod_name)<1) continue;
	}
	if (i==MAX_MOD_FUNCTIONS) return -1;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/core/%s.%s", nes_getstr(proc.N, confobj, "lib_path"), modname, ext);
	fixslashes(libname);
	snprintf(proc.srvmod[i].mod_name, sizeof(proc.srvmod[i].mod_name)-1, "%s", modname);
	if ((hinstLib=lib_open(libname))==NULL) goto fail;
	if ((srv_init=(void *)lib_sym(hinstLib, "mod_init"))==NULL) goto fail;
	if ((srv_exec=(void *)lib_sym(hinstLib, "mod_exec"))==NULL) goto fail;
	if ((srv_exit=(void *)lib_sym(hinstLib, "mod_exit"))==NULL) goto fail;
	srv_cron=(void *)lib_sym(hinstLib, "mod_cron");
	proc.srvmod[i].mod_init=srv_init;
	proc.srvmod[i].mod_exec=srv_exec;
	proc.srvmod[i].mod_exit=srv_exit;
	proc.srvmod[i].mod_cron=srv_cron;
	proc.srvmod[i].hinstLib=hinstLib;
	mod_init=(SRVMOD_INIT)srv_init;
	if (mod_init(&proc)!=0) return -1;
	return 0;
fail:
	log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s '%s'", modname, ext, lib_error());
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

int modules_init(nes_state *N)
{
	obj_t *cobj, *tobj;

	tobj=nes_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type!=NT_TABLE) return 0;
	tobj=nes_getobj(N, tobj, "modules");
	if (tobj->val->type!=NT_TABLE) return 0;
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->name[0]=='_') continue;
		if (cobj->val->type==NT_STRING) module_load(cobj->val->d.str);
	}
	return 0;
}
