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
#include "httpd_main.h"

#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif

static int regfunctions()
{
	static char loaded=0;
	obj_t *tobj;

	if (loaded) return 0;
	loaded=1;
	tobj=nsp_settable(proc->N, &proc->N->g, "_exports");
	tobj->val->attr|=NST_HIDDEN;
	tobj=nsp_settable(proc->N, tobj, "core");
	tobj->val->attr|=NST_HIDDEN;
/*
	nsp_setcfunc(proc->N, tobj, "config_read",		(void *)config_read);
	nsp_setcfunc(proc->N, tobj, "config_write",		(void *)config_write);
	nsp_setcfunc(proc->N, tobj, "decode_base64",		(void *)decode_base64);
	nsp_setcfunc(proc->N, tobj, "dir_list",			(void *)dir_list);
	nsp_setcfunc(proc->N, tobj, "domain_getname",		(void *)domain_getname);
	nsp_setcfunc(proc->N, tobj, "domain_getid",		(void *)domain_getid);
*/
	nsp_setcfunc(proc->N, tobj, "lang_gets",		(void *)lang_gets);
/*
	nsp_setcfunc(proc->N, tobj, "log_access",		(void *)log_access);
	nsp_setcfunc(proc->N, tobj, "log_error",		(void *)log_error);
	nsp_setcfunc(proc->N, tobj, "md5_init",			(void *)md5_init);
	nsp_setcfunc(proc->N, tobj, "md5_update",		(void *)md5_update);
	nsp_setcfunc(proc->N, tobj, "md5_final",		(void *)md5_final);
	nsp_setcfunc(proc->N, tobj, "md5_crypt",		(void *)md5_crypt);
*/
	nsp_setcfunc(proc->N, tobj, "ldir_freeresult",		(void *)ldir_freeresult);
	nsp_setcfunc(proc->N, tobj, "ldir_deleteentry",	(void *)ldir_deleteentry);
	nsp_setcfunc(proc->N, tobj, "ldir_saveentry",		(void *)ldir_saveentry);
	nsp_setcfunc(proc->N, tobj, "ldir_getlist",		(void *)ldir_getlist);
	nsp_setcfunc(proc->N, tobj, "ldir_getentry",		(void *)ldir_getentry);
	nsp_setcfunc(proc->N, tobj, "ldir_numentries",		(void *)ldir_numentries);
	nsp_setcfunc(proc->N, tobj, "ldir_getval",		(void *)ldir_getval);
/*
	nsp_setcfunc(proc->N, tobj, "nsp_newstate",		(void *)nsp_newstate);
	nsp_setcfunc(proc->N, tobj, "nsp_endstate",		(void *)nsp_endstate);
	nsp_setcfunc(proc->N, tobj, "nsp_exec",			(void *)nsp_exec);
	nsp_setcfunc(proc->N, tobj, "nsp_execfile",		(void *)nsp_execfile);
	nsp_setcfunc(proc->N, tobj, "nsp_freetable",		(void *)nsp_freetable);
	nsp_setcfunc(proc->N, tobj, "nsp_getobj",		(void *)nsp_getobj);
	nsp_setcfunc(proc->N, tobj, "nsp_getiobj",		(void *)nsp_getiobj);
	nsp_setcfunc(proc->N, tobj, "nsp_setobj",		(void *)nsp_setobj);
	nsp_setcfunc(proc->N, tobj, "nsp_tonum",		(void *)nsp_tonum);
	nsp_setcfunc(proc->N, tobj, "nsp_tostr",		(void *)nsp_tostr);
	nsp_setcfunc(proc->N, tobj, "nsp_eval",			(void *)nsp_eval);
	nsp_setcfunc(proc->N, tobj, "nsp_evalf",		(void *)nsp_evalf);
	nsp_setcfunc(proc->N, tobj, "nsp_linkval",		(void *)nsp_linkval);
	nsp_setcfunc(proc->N, tobj, "nsp_unlinkval",		(void *)nsp_unlinkval);
*/
	nsp_setcfunc(proc->N, tobj, "htnsp_dotemplate",	(void *)htnsp_dotemplate);
/*
	nsp_setcfunc(proc->N, tobj, "sql_freeresult",		(void *)sql_freeresult);
	nsp_setcfunc(proc->N, tobj, "sql_update",		(void *)sql_update);
	nsp_setcfunc(proc->N, tobj, "sql_updatef",		(void *)sql_updatef);
	nsp_setcfunc(proc->N, tobj, "sql_query",		(void *)sql_query);
	nsp_setcfunc(proc->N, tobj, "sql_queryf",		(void *)sql_queryf);
	nsp_setcfunc(proc->N, tobj, "sql_getname",		(void *)sql_getname);
	nsp_setcfunc(proc->N, tobj, "sql_getvalue",		(void *)sql_getvalue);
	nsp_setcfunc(proc->N, tobj, "sql_getvaluebyname",	(void *)sql_getvaluebyname);
	nsp_setcfunc(proc->N, tobj, "sql_numfields",		(void *)sql_numfields);
	nsp_setcfunc(proc->N, tobj, "sql_numtuples",		(void *)sql_numtuples);
*/
	nsp_setcfunc(proc->N, tobj, "sql_permprune",		(void *)sql_permprune);
/*
	nsp_setcfunc(proc->N, tobj, "sys_system",		(void *)sys_system);

	nsp_setcfunc(proc->N, tobj, "tcp_bind",			(void *)tcp_bind);
	nsp_setcfunc(proc->N, tobj, "tcp_accept",		(void *)tcp_accept);
	nsp_setcfunc(proc->N, tobj, "tcp_connect",		(void *)tcp_connect);
	nsp_setcfunc(proc->N, tobj, "tcp_fgets",		(void *)tcp_fgets);
	nsp_setcfunc(proc->N, tobj, "tcp_fprintf",		(void *)tcp_fprintf);
	nsp_setcfunc(proc->N, tobj, "tcp_recv",			(void *)tcp_recv);
	nsp_setcfunc(proc->N, tobj, "tcp_send",			(void *)tcp_send);
	nsp_setcfunc(proc->N, tobj, "tcp_close",		(void *)tcp_close);
	nsp_setcfunc(proc->N, tobj, "time_sql2unix",		(void *)time_sql2unix);
	nsp_setcfunc(proc->N, tobj, "time_unix2sql",		(void *)time_unix2sql);
	nsp_setcfunc(proc->N, tobj, "strncatf",			(void *)strncatf);
	nsp_setcfunc(proc->N, tobj, "striprn",			(void *)striprn);
	nsp_setcfunc(proc->N, tobj, "fixslashes",		(void *)fixslashes);
	nsp_setcfunc(proc->N, tobj, "str2sql",			(void *)str2sql);
	nsp_setcfunc(proc->N, tobj, "p_strcasestr",		(void *)p_strcasestr);
*/
	nsp_setcfunc(proc->N, tobj, "auth_setcookie",		(void *)auth_setcookie);
	nsp_setcfunc(proc->N, tobj, "auth_setpass",		(void *)auth_setpass);
	nsp_setcfunc(proc->N, tobj, "auth_priv",		(void *)auth_priv);
	nsp_setcfunc(proc->N, tobj, "db_log_activity",		(void *)db_log_activity);
	nsp_setcfunc(proc->N, tobj, "htpage_header",		(void *)htpage_header);
	nsp_setcfunc(proc->N, tobj, "htpage_footer",		(void *)htpage_footer);
	nsp_setcfunc(proc->N, tobj, "htpage_topmenu",		(void *)htpage_topmenu);
	nsp_setcfunc(proc->N, tobj, "htpage_dirlist",		(void *)htpage_dirlist);
/*
	nsp_setcfunc(proc->N, tobj, "htselect_contact",		(void *)htselect_contact);
	nsp_setcfunc(proc->N, tobj, "htselect_day",		(void *)htselect_day);
	nsp_setcfunc(proc->N, tobj, "htselect_domain",		(void *)htselect_domain);
	nsp_setcfunc(proc->N, tobj, "htselect_eventstatus",	(void *)htselect_eventstatus);
	nsp_setcfunc(proc->N, tobj, "htselect_group",		(void *)htselect_group);
	nsp_setcfunc(proc->N, tobj, "htselect_hour",		(void *)htselect_hour);
	nsp_setcfunc(proc->N, tobj, "htselect_minutes",		(void *)htselect_minutes);
	nsp_setcfunc(proc->N, tobj, "htselect_month",		(void *)htselect_month);
	nsp_setcfunc(proc->N, tobj, "htselect_number",		(void *)htselect_number);
	nsp_setcfunc(proc->N, tobj, "htselect_priority",	(void *)htselect_priority);
	nsp_setcfunc(proc->N, tobj, "htselect_qhours",		(void *)htselect_qhours);
	nsp_setcfunc(proc->N, tobj, "htselect_qminutes",	(void *)htselect_qminutes);
	nsp_setcfunc(proc->N, tobj, "htselect_reminder",	(void *)htselect_reminder);
	nsp_setcfunc(proc->N, tobj, "htselect_time",		(void *)htselect_time);
	nsp_setcfunc(proc->N, tobj, "htselect_timezone",	(void *)htselect_timezone);
	nsp_setcfunc(proc->N, tobj, "htselect_user",		(void *)htselect_user);
	nsp_setcfunc(proc->N, tobj, "htselect_year",		(void *)htselect_year);
	nsp_setcfunc(proc->N, tobj, "htselect_zone",		(void *)htselect_zone);
	nsp_setcfunc(proc->N, tobj, "htview_callaction",	(void *)htview_callaction);
	nsp_setcfunc(proc->N, tobj, "htview_contact",		(void *)htview_contact);
	nsp_setcfunc(proc->N, tobj, "htview_domain",		(void *)htview_domain);
	nsp_setcfunc(proc->N, tobj, "htview_eventclosingstatus",(void *)htview_eventclosingstatus);
	nsp_setcfunc(proc->N, tobj, "htview_eventstatus",	(void *)htview_eventstatus);
	nsp_setcfunc(proc->N, tobj, "htview_eventtype",		(void *)htview_eventtype);
	nsp_setcfunc(proc->N, tobj, "htview_holiday",		(void *)htview_holiday);
	nsp_setcfunc(proc->N, tobj, "htview_reminder",		(void *)htview_reminder);
	nsp_setcfunc(proc->N, tobj, "htview_user",		(void *)htview_user);
*/
	nsp_setcfunc(proc->N, tobj, "htscript_showpage",	(void *)htscript_showpage);
	nsp_setcfunc(proc->N, tobj, "time_sql2datetext",	(void *)time_sql2datetext);
	nsp_setcfunc(proc->N, tobj, "time_sql2timetext",	(void *)time_sql2timetext);
	nsp_setcfunc(proc->N, tobj, "time_tzoffset",		(void *)time_tzoffset);
	nsp_setcfunc(proc->N, tobj, "time_tzoffset2",		(void *)time_tzoffset2);
	nsp_setcfunc(proc->N, tobj, "time_tzoffsetcon",	(void *)time_tzoffsetcon);
	nsp_setcfunc(proc->N, tobj, "time_unix2text",		(void *)time_unix2text);
	nsp_setcfunc(proc->N, tobj, "time_unix2lotimetext",	(void *)time_unix2lotimetext);
	nsp_setcfunc(proc->N, tobj, "time_unix2timetext",	(void *)time_unix2timetext);
	nsp_setcfunc(proc->N, tobj, "time_unix2datetext",	(void *)time_unix2datetext);
	nsp_setcfunc(proc->N, tobj, "time_wmgetdate",		(void *)time_wmgetdate);
	nsp_setcfunc(proc->N, tobj, "getxmlenv",		(void *)getxmlenv);
	nsp_setcfunc(proc->N, tobj, "getxmlparam",		(void *)getxmlparam);
	nsp_setcfunc(proc->N, tobj, "getxmlstruct",		(void *)getxmlstruct);
	nsp_setcfunc(proc->N, tobj, "get_mime_type",		(void *)get_mime_type);
	nsp_setcfunc(proc->N, tobj, "getgetenv",		(void *)getgetenv);
	nsp_setcfunc(proc->N, tobj, "getpostenv",		(void *)getpostenv);
	nsp_setcfunc(proc->N, tobj, "decodeurl",		(void *)decodeurl);
	nsp_setcfunc(proc->N, tobj, "encodeurl",		(void *)encodeurl);
	nsp_setcfunc(proc->N, tobj, "filesend",		(void *)filesend);
	nsp_setcfunc(proc->N, tobj, "str2html",		(void *)str2html);
	nsp_setcfunc(proc->N, tobj, "send_header",		(void *)send_header);
	nsp_setcfunc(proc->N, tobj, "send_error",		(void *)send_error);
	nsp_setcfunc(proc->N, tobj, "printline",		(void *)printline);
	nsp_setcfunc(proc->N, tobj, "prints",			(void *)prints);
	nsp_setcfunc(proc->N, tobj, "raw_prints",		(void *)raw_prints);
	nsp_setcfunc(proc->N, tobj, "printhex",		(void *)printhex);
	nsp_setcfunc(proc->N, tobj, "printht",			(void *)printht);
	nsp_setcfunc(proc->N, tobj, "printline2",		(void *)printline2);
	nsp_setcfunc(proc->N, tobj, "getbuffer",		(void *)getbuffer);
	nsp_setcfunc(proc->N, tobj, "flushbuffer",		(void *)flushbuffer);
	nsp_setcfunc(proc->N, tobj, "hex2int",			(void *)hex2int);

	nsp_setcfunc(proc->N, tobj, "closeconnect",		(void *)closeconnect);
/*	nsp_setcfunc(proc->N, tobj, "srv_restart",		(void *)server_restart); */
	nsp_setcfunc(proc->N, tobj, "module_exists",		(void *)module_exists);
	nsp_setcfunc(proc->N, tobj, "module_call",		(void *)module_call);
#ifdef WIN32
/*	nsp_setcfunc(proc->N, tobj, "gettimeofday",		(void *)gettimeofday); */
#endif
	return 0;
}

int module_exists(char *mod_name)
{
	obj_t *modobj=nsp_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *tobj;
	obj_t *cobj;

	tobj=nsp_getobj(proc->N, modobj, mod_name);
	if (tobj->val->type!=NT_TABLE) return 0;
	cobj=nsp_getobj(proc->N, tobj, "fn_name");
	if (cobj->val->type!=NT_STRING) return 0;
	cobj=nsp_getobj(proc->N, tobj, "mod_main");
	if (cobj->val->type!=NT_CFUNC) return 0;
	return 1;
}

int module_menucall(CONN *conn)
{
	obj_t *modobj=nsp_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *htobj=nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj, *tobj;
	char *RequestURI=nsp_getstr(conn->N, htobj, "REQUEST_URI");
	HTMOD_MAIN htmod_main;

	for (tobj=modobj->val->d.table.f;tobj;tobj=tobj->next) {
		if (tobj->val->type!=NT_TABLE) break;
		if (tobj->name[0]=='_') continue;
		cobj=nsp_getobj(proc->N, tobj, "fn_uri");
		if (cobj->val->type!=NT_STRING||cobj->val->d.str==NULL) continue;
		if (strncmp(RequestURI, cobj->val->d.str, strlen(cobj->val->d.str))!=0) continue;
		/* looks like a match to me */
		cobj=nsp_getobj(proc->N, tobj, "mod_main");
		if ((cobj->val->type!=NT_CFUNC)||(cobj->val->d.cfunc==NULL)) return 0;
		htmod_main=(HTMOD_MAIN)cobj->val->d.cfunc;
		if (htmod_main(conn)==-1) continue;
		return 1;
	}
	return 0;
}

void *module_call(char *fn_name)
{
	obj_t *modobj=nsp_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *tobj=nsp_settable(proc->N, modobj, "_exports");
	obj_t *cobj;

	cobj=nsp_getobj(proc->N, tobj, fn_name);
	if (cobj->val->type!=NT_CFUNC) return NULL;
	return cobj->val->d.cfunc;
}

int module_load(char *modname)
{
	obj_t *confobj=nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	HTMOD_INIT htmod_init;
#ifdef WIN32
	char *ext="dll";
#else
	char *ext="so";
#endif
	char libname[255];

	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/" MODSHORTNAME "/%s.%s", nsp_getstr(proc->N, confobj, "lib_path"), modname, ext);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))==NULL) goto fail;
	if ((htmod_init=(HTMOD_INIT)lib_sym(hinstLib, "mod_init"))==NULL) goto fail;
	if (htmod_init(proc)!=0) return -1;
	return 0;
fail:
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s '%s'", modname, ext, lib_error());
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

int modules_init()
{
	obj_t *tobj=nsp_getobj(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "httpd"), "modules");
	obj_t *cobj;

	if (!nsp_istable(tobj)) return 0;
	regfunctions();
	for (cobj=tobj->val->d.table.f;cobj;cobj=cobj->next) {
		if (cobj->name[0]=='_') continue;
		if (nsp_isstr(cobj)) module_load(cobj->val->d.str);
	}
	return 0;
}
