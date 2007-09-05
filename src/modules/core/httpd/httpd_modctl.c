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
	tobj=nes_settable(proc->N, &proc->N->g, "_exports");
	tobj->val->attr|=NST_HIDDEN;
	tobj=nes_settable(proc->N, tobj, "core");
	tobj->val->attr|=NST_HIDDEN;
/*
	nes_setcfunc(proc->N, tobj, "config_read",		(void *)config_read);
	nes_setcfunc(proc->N, tobj, "config_write",		(void *)config_write);
	nes_setcfunc(proc->N, tobj, "decode_base64",		(void *)decode_base64);
	nes_setcfunc(proc->N, tobj, "dir_list",			(void *)dir_list);
	nes_setcfunc(proc->N, tobj, "domain_getname",		(void *)domain_getname);
	nes_setcfunc(proc->N, tobj, "domain_getid",		(void *)domain_getid);
*/
	nes_setcfunc(proc->N, tobj, "lang_gets",		(void *)lang_gets);
/*
	nes_setcfunc(proc->N, tobj, "log_access",		(void *)log_access);
	nes_setcfunc(proc->N, tobj, "log_error",		(void *)log_error);
	nes_setcfunc(proc->N, tobj, "md5_init",			(void *)md5_init);
	nes_setcfunc(proc->N, tobj, "md5_update",		(void *)md5_update);
	nes_setcfunc(proc->N, tobj, "md5_final",		(void *)md5_final);
	nes_setcfunc(proc->N, tobj, "md5_crypt",		(void *)md5_crypt);
*/
	nes_setcfunc(proc->N, tobj, "ldir_freeresult",		(void *)ldir_freeresult);
	nes_setcfunc(proc->N, tobj, "ldir_deleteentry",		(void *)ldir_deleteentry);
	nes_setcfunc(proc->N, tobj, "ldir_saveentry",		(void *)ldir_saveentry);
	nes_setcfunc(proc->N, tobj, "ldir_getlist",		(void *)ldir_getlist);
	nes_setcfunc(proc->N, tobj, "ldir_getentry",		(void *)ldir_getentry);
	nes_setcfunc(proc->N, tobj, "ldir_numentries",		(void *)ldir_numentries);
	nes_setcfunc(proc->N, tobj, "ldir_getval",		(void *)ldir_getval);
/*
	nes_setcfunc(proc->N, tobj, "nes_newstate",		(void *)nes_newstate);
	nes_setcfunc(proc->N, tobj, "nes_endstate",		(void *)nes_endstate);
	nes_setcfunc(proc->N, tobj, "nes_exec",			(void *)nes_exec);
	nes_setcfunc(proc->N, tobj, "nes_execfile",		(void *)nes_execfile);
	nes_setcfunc(proc->N, tobj, "nes_freetable",		(void *)nes_freetable);
	nes_setcfunc(proc->N, tobj, "nes_getobj",		(void *)nes_getobj);
	nes_setcfunc(proc->N, tobj, "nes_getiobj",		(void *)nes_getiobj);
	nes_setcfunc(proc->N, tobj, "nes_setobj",		(void *)nes_setobj);
	nes_setcfunc(proc->N, tobj, "nes_tonum",		(void *)nes_tonum);
	nes_setcfunc(proc->N, tobj, "nes_tostr",		(void *)nes_tostr);
	nes_setcfunc(proc->N, tobj, "nes_eval",			(void *)nes_eval);
	nes_setcfunc(proc->N, tobj, "nes_evalf",		(void *)nes_evalf);
	nes_setcfunc(proc->N, tobj, "nes_linkval",		(void *)nes_linkval);
	nes_setcfunc(proc->N, tobj, "nes_unlinkval",		(void *)nes_unlinkval);
*/
	nes_setcfunc(proc->N, tobj, "htnes_dotemplate",		(void *)htnes_dotemplate);
/*
	nes_setcfunc(proc->N, tobj, "sql_freeresult",		(void *)sql_freeresult);
	nes_setcfunc(proc->N, tobj, "sql_update",		(void *)sql_update);
	nes_setcfunc(proc->N, tobj, "sql_updatef",		(void *)sql_updatef);
	nes_setcfunc(proc->N, tobj, "sql_query",		(void *)sql_query);
	nes_setcfunc(proc->N, tobj, "sql_queryf",		(void *)sql_queryf);
	nes_setcfunc(proc->N, tobj, "sql_getname",		(void *)sql_getname);
	nes_setcfunc(proc->N, tobj, "sql_getvalue",		(void *)sql_getvalue);
	nes_setcfunc(proc->N, tobj, "sql_getvaluebyname",	(void *)sql_getvaluebyname);
	nes_setcfunc(proc->N, tobj, "sql_numfields",		(void *)sql_numfields);
	nes_setcfunc(proc->N, tobj, "sql_numtuples",		(void *)sql_numtuples);
*/
	nes_setcfunc(proc->N, tobj, "sql_permprune",		(void *)sql_permprune);
/*
	nes_setcfunc(proc->N, tobj, "sys_system",		(void *)sys_system);

	nes_setcfunc(proc->N, tobj, "tcp_bind",			(void *)tcp_bind);
	nes_setcfunc(proc->N, tobj, "tcp_accept",		(void *)tcp_accept);
	nes_setcfunc(proc->N, tobj, "tcp_connect",		(void *)tcp_connect);
	nes_setcfunc(proc->N, tobj, "tcp_fgets",		(void *)tcp_fgets);
	nes_setcfunc(proc->N, tobj, "tcp_fprintf",		(void *)tcp_fprintf);
	nes_setcfunc(proc->N, tobj, "tcp_recv",			(void *)tcp_recv);
	nes_setcfunc(proc->N, tobj, "tcp_send",			(void *)tcp_send);
	nes_setcfunc(proc->N, tobj, "tcp_close",		(void *)tcp_close);
	nes_setcfunc(proc->N, tobj, "time_sql2unix",		(void *)time_sql2unix);
	nes_setcfunc(proc->N, tobj, "time_unix2sql",		(void *)time_unix2sql);
	nes_setcfunc(proc->N, tobj, "strncatf",			(void *)strncatf);
	nes_setcfunc(proc->N, tobj, "striprn",			(void *)striprn);
	nes_setcfunc(proc->N, tobj, "fixslashes",		(void *)fixslashes);
	nes_setcfunc(proc->N, tobj, "str2sql",			(void *)str2sql);
	nes_setcfunc(proc->N, tobj, "p_strcasestr",		(void *)p_strcasestr);
*/
	nes_setcfunc(proc->N, tobj, "auth_setcookie",		(void *)auth_setcookie);
	nes_setcfunc(proc->N, tobj, "auth_setpass",		(void *)auth_setpass);
	nes_setcfunc(proc->N, tobj, "auth_priv",		(void *)auth_priv);
	nes_setcfunc(proc->N, tobj, "db_log_activity",		(void *)db_log_activity);
	nes_setcfunc(proc->N, tobj, "htpage_header",		(void *)htpage_header);
	nes_setcfunc(proc->N, tobj, "htpage_footer",		(void *)htpage_footer);
	nes_setcfunc(proc->N, tobj, "htpage_topmenu",		(void *)htpage_topmenu);
	nes_setcfunc(proc->N, tobj, "htpage_dirlist",		(void *)htpage_dirlist);
	nes_setcfunc(proc->N, tobj, "htselect_contact",		(void *)htselect_contact);
	nes_setcfunc(proc->N, tobj, "htselect_day",		(void *)htselect_day);
	nes_setcfunc(proc->N, tobj, "htselect_domain",		(void *)htselect_domain);
	nes_setcfunc(proc->N, tobj, "htselect_eventstatus",	(void *)htselect_eventstatus);
	nes_setcfunc(proc->N, tobj, "htselect_group",		(void *)htselect_group);
	nes_setcfunc(proc->N, tobj, "htselect_hour",		(void *)htselect_hour);
	nes_setcfunc(proc->N, tobj, "htselect_minutes",		(void *)htselect_minutes);
	nes_setcfunc(proc->N, tobj, "htselect_month",		(void *)htselect_month);
	nes_setcfunc(proc->N, tobj, "htselect_number",		(void *)htselect_number);
	nes_setcfunc(proc->N, tobj, "htselect_priority",	(void *)htselect_priority);
	nes_setcfunc(proc->N, tobj, "htselect_qhours",		(void *)htselect_qhours);
	nes_setcfunc(proc->N, tobj, "htselect_qminutes",	(void *)htselect_qminutes);
	nes_setcfunc(proc->N, tobj, "htselect_reminder",	(void *)htselect_reminder);
	nes_setcfunc(proc->N, tobj, "htselect_time",		(void *)htselect_time);
	nes_setcfunc(proc->N, tobj, "htselect_timezone",	(void *)htselect_timezone);
	nes_setcfunc(proc->N, tobj, "htselect_user",		(void *)htselect_user);
	nes_setcfunc(proc->N, tobj, "htselect_year",		(void *)htselect_year);
	nes_setcfunc(proc->N, tobj, "htselect_zone",		(void *)htselect_zone);
	nes_setcfunc(proc->N, tobj, "htview_callaction",	(void *)htview_callaction);
	nes_setcfunc(proc->N, tobj, "htview_contact",		(void *)htview_contact);
	nes_setcfunc(proc->N, tobj, "htview_domain",		(void *)htview_domain);
	nes_setcfunc(proc->N, tobj, "htview_eventclosingstatus",(void *)htview_eventclosingstatus);
	nes_setcfunc(proc->N, tobj, "htview_eventstatus",	(void *)htview_eventstatus);
	nes_setcfunc(proc->N, tobj, "htview_eventtype",		(void *)htview_eventtype);
	nes_setcfunc(proc->N, tobj, "htview_holiday",		(void *)htview_holiday);
	nes_setcfunc(proc->N, tobj, "htview_reminder",		(void *)htview_reminder);
	nes_setcfunc(proc->N, tobj, "htview_user",		(void *)htview_user);
	nes_setcfunc(proc->N, tobj, "htscript_showpage",	(void *)htscript_showpage);
	nes_setcfunc(proc->N, tobj, "time_sql2datetext",	(void *)time_sql2datetext);
	nes_setcfunc(proc->N, tobj, "time_sql2timetext",	(void *)time_sql2timetext);
	nes_setcfunc(proc->N, tobj, "time_tzoffset",		(void *)time_tzoffset);
	nes_setcfunc(proc->N, tobj, "time_tzoffset2",		(void *)time_tzoffset2);
	nes_setcfunc(proc->N, tobj, "time_tzoffsetcon",		(void *)time_tzoffsetcon);
	nes_setcfunc(proc->N, tobj, "time_unix2text",		(void *)time_unix2text);
	nes_setcfunc(proc->N, tobj, "time_unix2lotimetext",	(void *)time_unix2lotimetext);
	nes_setcfunc(proc->N, tobj, "time_unix2timetext",	(void *)time_unix2timetext);
	nes_setcfunc(proc->N, tobj, "time_unix2datetext",	(void *)time_unix2datetext);
	nes_setcfunc(proc->N, tobj, "time_wmgetdate",		(void *)time_wmgetdate);
	nes_setcfunc(proc->N, tobj, "getxmlenv",		(void *)getxmlenv);
	nes_setcfunc(proc->N, tobj, "getxmlparam",		(void *)getxmlparam);
	nes_setcfunc(proc->N, tobj, "getxmlstruct",		(void *)getxmlstruct);
	nes_setcfunc(proc->N, tobj, "get_mime_type",		(void *)get_mime_type);
	nes_setcfunc(proc->N, tobj, "getgetenv",		(void *)getgetenv);
	nes_setcfunc(proc->N, tobj, "getpostenv",		(void *)getpostenv);
	nes_setcfunc(proc->N, tobj, "decodeurl",		(void *)decodeurl);
	nes_setcfunc(proc->N, tobj, "encodeurl",		(void *)encodeurl);
	nes_setcfunc(proc->N, tobj, "filesend",			(void *)filesend);
	nes_setcfunc(proc->N, tobj, "str2html",			(void *)str2html);
	nes_setcfunc(proc->N, tobj, "send_header",		(void *)send_header);
	nes_setcfunc(proc->N, tobj, "send_error",		(void *)send_error);
	nes_setcfunc(proc->N, tobj, "printline",		(void *)printline);
	nes_setcfunc(proc->N, tobj, "prints",			(void *)prints);
	nes_setcfunc(proc->N, tobj, "raw_prints",		(void *)raw_prints);
	nes_setcfunc(proc->N, tobj, "printhex",			(void *)printhex);
	nes_setcfunc(proc->N, tobj, "printht",			(void *)printht);
	nes_setcfunc(proc->N, tobj, "printline2",		(void *)printline2);
	nes_setcfunc(proc->N, tobj, "getbuffer",		(void *)getbuffer);
	nes_setcfunc(proc->N, tobj, "flushbuffer",		(void *)flushbuffer);
	nes_setcfunc(proc->N, tobj, "hex2int",			(void *)hex2int);

	nes_setcfunc(proc->N, tobj, "closeconnect",		(void *)closeconnect);
/*	nes_setcfunc(proc->N, tobj, "srv_restart",		(void *)server_restart); */
	nes_setcfunc(proc->N, tobj, "module_exists",		(void *)module_exists);
	nes_setcfunc(proc->N, tobj, "module_call",		(void *)module_call);
#ifdef WIN32
/*	nes_setcfunc(proc->N, tobj, "gettimeofday",		(void *)gettimeofday); */
#endif
	return 0;
}

int module_exists(char *mod_name)
{
	obj_t *modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *tobj;
	obj_t *cobj;

	tobj=nes_getobj(proc->N, modobj, mod_name);
	if (tobj->val->type!=NT_TABLE) return 0;
	cobj=nes_getobj(proc->N, tobj, "fn_name");
	if (cobj->val->type!=NT_STRING) return 0;
	cobj=nes_getobj(proc->N, tobj, "mod_main");
	if (cobj->val->type!=NT_CFUNC) return 0;
	return 1;
}

int module_menucall(CONN *sid)
{
	obj_t *modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	obj_t *cobj, *tobj;
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	HTMOD_MAIN htmod_main;

	for (tobj=modobj->val->d.table;tobj;tobj=tobj->next) {
		if (tobj->val->type!=NT_TABLE) break;
		if (tobj->name[0]=='_') continue;
		cobj=nes_getobj(proc->N, tobj, "fn_uri");
		if (cobj->val->type!=NT_STRING||cobj->val->d.str==NULL) continue;
		if (strncmp(RequestURI, cobj->val->d.str, strlen(cobj->val->d.str))!=0) continue;
		/* looks like a match to me */
		cobj=nes_getobj(proc->N, tobj, "mod_main");
		if ((cobj->val->type!=NT_CFUNC)||(cobj->val->d.cfunc==NULL)) return 0;
		htmod_main=(HTMOD_MAIN)cobj->val->d.cfunc;
		if (htmod_main(sid)==-1) continue;
		return 1;
	}
	return 0;
}

void *module_call(char *fn_name)
{
	obj_t *modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	obj_t *tobj=nes_settable(proc->N, modobj, "_exports");
	obj_t *cobj;

	cobj=nes_getobj(proc->N, tobj, fn_name);
	if (cobj->val->type!=NT_CFUNC) return NULL;
	return cobj->val->d.cfunc;
}

int module_load(char *modname)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	HTMOD_INIT htmod_init;
#ifdef WIN32
	char *ext="dll";
#else
	char *ext="so";
#endif
	char libname[255];

	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/" MODSHORTNAME "/%s.%s", nes_getstr(proc->N, confobj, "lib_path"), modname, ext);
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
	obj_t *tobj=nes_getobj(proc->N, nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd"), "modules");
	obj_t *cobj;

	if (!nes_istable(tobj)) return 0;
	regfunctions();
	for (cobj=tobj->val->d.table;cobj;cobj=cobj->next) {
		if (cobj->name[0]=='_') continue;
		if (nes_isstr(cobj)) module_load(cobj->val->d.str);
	}
	return 0;
}
