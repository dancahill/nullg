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

#ifdef WIN32
#define dlopen(x, y) LoadLibrary(x)
#define dlsym(x, y) GetProcAddress(x, y)
#define dlclose(x) FreeLibrary(x)
#endif

HTMOD_INIT htmod_init;
HTMOD_MAIN htmod_main;

#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif

int module_exists(CONN *sid, char *mod_name)
{
	int i;

	for (i=0;;i++) {
		if ((http_proc.mod_menuitems[i].fn_name==NULL)||(http_proc.mod_menuitems[i].fn_ptr==NULL)) break;
		if (strcmp(http_proc.mod_menuitems[i].mod_name, mod_name)==0) return 1;
	}
	return 0;
}

void *module_call(CONN *sid, char *fn_name)
{
	int i;

	for (i=0;;i++) {
		if ((http_proc.mod_functions[i].fn_name==NULL)||(http_proc.mod_functions[i].fn_ptr==NULL)) break;
		if (strcmp(http_proc.mod_functions[i].fn_name, fn_name)==0) {
			return http_proc.mod_functions[i].fn_ptr;
		}
	}
	return NULL;
}

int module_menucall(CONN *sid)
{
	int i;

	for (i=0;;i++) {
		if ((http_proc.mod_menuitems[i].fn_name==NULL)||(http_proc.mod_menuitems[i].fn_ptr==NULL)) break;
		if (strlen(http_proc.mod_menuitems[i].fn_uri)<1) continue;
		if (strncmp(sid->dat->in_RequestURI, http_proc.mod_menuitems[i].fn_uri, strlen(http_proc.mod_menuitems[i].fn_uri))==0) {
			htmod_main=http_proc.mod_menuitems[i].fn_ptr;
			htmod_main(sid);
			return 1;
		}
	}
	return 0;
}

int module_load(char *modname)
{
	FUNCTION export_list[]={
		{ "config_read",		config_read			},
		{ "config_write",		config_write			},
		{ "decode_base64",		decode_base64			},
		{ "domain_getname",		domain_getname			},
		{ "domain_getid",		domain_getid			},
		{ "log_access",			log_access			},
		{ "log_error",			log_error			},
		{ "md5_init",			md5_init			},
		{ "md5_update",			md5_update			},
		{ "md5_final",			md5_final			},
		{ "md5_crypt",			md5_crypt			},
		{ "sql_freeresult",		sql_freeresult			},
		{ "sql_update",			sql_update			},
		{ "sql_updatef",		sql_updatef			},
		{ "sql_query",			sql_query			},
		{ "sql_queryf",			sql_queryf			},
		{ "sql_getname",		sql_getname			},
		{ "sql_getvalue",		sql_getvalue			},
		{ "sql_getvaluebyname",		sql_getvaluebyname		},
		{ "sql_numfields",		sql_numfields			},
		{ "sql_numtuples",		sql_numtuples			},
		{ "sys_system",			sys_system			},
//		{ "tcp_bind",			tcp_bind			},
//		{ "tcp_accept",			tcp_accept			},
		{ "tcp_fgets",			tcp_fgets			},
		{ "tcp_fprintf",		tcp_fprintf			},
		{ "tcp_recv",			tcp_recv			},
		{ "tcp_send",			tcp_send			},
		{ "tcp_close",			tcp_close			},
		{ "time_sql2unix",		time_sql2unix			},
		{ "time_unix2sql",		time_unix2sql			},
		{ "strncatf",			strncatf			},
		{ "striprn",			striprn				},
		{ "fixslashes",			fixslashes			},
		{ "str2sql",			str2sql				},
		{ "p_strcasestr",		p_strcasestr			},

		{ "auth_setcookie",		auth_setcookie			},
		{ "auth_setpass",		auth_setpass			},
		{ "auth_priv",			auth_priv			},
		{ "db_log_activity",		db_log_activity			},
		{ "htpage_header",		htpage_header			},
		{ "htpage_footer",		htpage_footer			},
		{ "htpage_topmenu",		htpage_topmenu			},
		{ "htselect_contact",		htselect_contact		},
		{ "htselect_day",		htselect_day			},
		{ "htselect_domain",		htselect_domain			},
		{ "htselect_eventstatus",	htselect_eventstatus		},
		{ "htselect_group",		htselect_group			},
		{ "htselect_hour",		htselect_hour			},
		{ "htselect_minutes",		htselect_minutes		},
		{ "htselect_month",		htselect_month			},
		{ "htselect_number",		htselect_number			},
		{ "htselect_priority",		htselect_priority		},
		{ "htselect_qhours",		htselect_qhours			},
		{ "htselect_qminutes",		htselect_qminutes		},
		{ "htselect_reminder",		htselect_reminder		},
		{ "htselect_time",		htselect_time			},
		{ "htselect_timezone",		htselect_timezone		},
		{ "htselect_user",		htselect_user			},
		{ "htselect_year",		htselect_year			},
		{ "htselect_zone",		htselect_zone			},
		{ "htview_callaction",		htview_callaction		},
		{ "htview_contact",		htview_contact			},
		{ "htview_domain",		htview_domain			},
		{ "htview_eventclosingstatus",	htview_eventclosingstatus	},
		{ "htview_eventstatus",		htview_eventstatus		},
		{ "htview_eventtype",		htview_eventtype		},
		{ "htview_holiday",		htview_holiday			},
		{ "htview_reminder",		htview_reminder			},
		{ "htview_user",		htview_user			},
		{ "htscript_showpage",		htscript_showpage		},
		{ "time_sql2datetext",		time_sql2datetext		},
		{ "time_sql2timetext",		time_sql2timetext		},
		{ "time_tzoffset",		time_tzoffset			},
		{ "time_tzoffset2",		time_tzoffset2			},
		{ "time_tzoffsetcon",		time_tzoffsetcon		},
		{ "time_unix2text",		time_unix2text			},
		{ "time_unix2lotimetext",	time_unix2lotimetext		},
		{ "time_unix2timetext",		time_unix2timetext		},
		{ "time_unix2datetext",		time_unix2datetext		},
		{ "time_wmgetdate",		time_wmgetdate			},
		{ "getmimeenv",			getmimeenv			},
		{ "getxmlenv",			getxmlenv			},
		{ "getxmlparam",		getxmlparam			},
		{ "getxmlstruct",		getxmlstruct			},
		{ "get_mime_type",		get_mime_type			},
		{ "getgetenv",			getgetenv			},
		{ "getpostenv",			getpostenv			},
		{ "decodeurl",			decodeurl			},
		{ "encodeurl",			encodeurl			},
		{ "filesend",			filesend			},
		{ "str2html",			str2html			},
		{ "send_header",		send_header			},
		{ "send_error",			send_error			},
		{ "printline",			printline			},
		{ "prints",			prints				},
		{ "raw_prints",			raw_prints			},
		{ "printhex",			printhex			},
		{ "printht",			printht				},
		{ "printline2",			printline2			},
		{ "getbuffer",			getbuffer			},
		{ "flushbuffer",		flushbuffer			},
		{ "hex2int",			hex2int				},
		{ "closeconnect",		closeconnect			},
//		{ "srv_restart",		server_restart			},
		{ "module_exists",		module_exists			},
		{ "module_call",		module_call			},
#ifdef WIN32
		{ "gettimeofday",		gettimeofday			},
		{ "opendir",			opendir				},
		{ "readdir",			readdir				},
		{ "closedir",			closedir			},
#endif
		{ NULL, NULL }
	};
#ifdef WIN32
	char *ext="dll";
#else
	char *ext="so";
#endif
	char libname[255];

	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/%s.%s", config->dir_lib, modname, ext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))==NULL) goto fail;
	if ((htmod_init=(HTMOD_INIT)dlsym(hinstLib, "mod_init"))==NULL) goto fail;
	if (htmod_init(proc, &http_proc, export_list)!=0) return -1;
	return 0;
fail:
	log_error("http", __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s", modname, ext);
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

int modules_init()
{
	memset((char *)&http_proc.mod_menuitems, 0, sizeof(MODULE_MENU));
	memset((char *)&http_proc.mod_functions, 0, sizeof(MODULE_FUNC));
	conf_read_modules();
	return 0;
}
