/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

static FUNCTION functions[]={
	{ "auth_setpass",		auth_setpass			},
	{ "auth_priv",			auth_priv			},
	{ "db_read",			db_read				},
	{ "db_write",			db_write			},
	{ "db_log_activity",		db_log_activity			},
	{ "htpage_header",		htpage_header			},
	{ "htpage_footer",		htpage_footer			},
	{ "htpage_topmenu",		htpage_topmenu			},
	{ "htselect_contact",		htselect_contact		},
	{ "htselect_day",		htselect_day			},
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
	{ "htview_eventclosingstatus",	htview_eventclosingstatus	},
	{ "htview_eventstatus",		htview_eventstatus		},
	{ "htview_eventtype",		htview_eventtype		},
	{ "htview_holiday",		htview_holiday			},
	{ "htview_reminder",		htview_reminder			},
	{ "htview_user",		htview_user			},
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
	{ "time_sql2datetext",		time_sql2datetext		},
	{ "time_sql2timetext",		time_sql2timetext		},
	{ "time_sql2unix",		time_sql2unix			},
	{ "time_tzoffset",		time_tzoffset			},
	{ "time_tzoffset2",		time_tzoffset2			},
	{ "time_unix2sql",		time_unix2sql			},
	{ "time_unix2text",		time_unix2text			},
	{ "time_unix2lotimetext",	time_unix2lotimetext		},
	{ "time_unix2timetext",		time_unix2timetext		},
	{ "time_unix2datetext",		time_unix2datetext		},
	{ "time_wmgetdate",		time_wmgetdate			},
	{ "getmimeenv",			getmimeenv			},
	{ "get_mime_type",		get_mime_type			},
	{ "getgetenv",			getgetenv			},
	{ "getpostenv",			getpostenv			},
	{ "logaccess",			logaccess			},
	{ "logerror",			logerror			},
	{ "decodeurl",			decodeurl			},
	{ "filesend",			filesend			},
	{ "str2html",			str2html			},
	{ "strncatf",			strncatf			},
	{ "send_header",		send_header			},
	{ "send_error",			send_error			},
	{ "printline",			printline			},
	{ "striprn",			striprn				},
	{ "prints",			prints				},
	{ "printhex",			printhex			},
	{ "printht",			printht				},
	{ "printline2",			printline2			},
	{ "fixslashes",			fixslashes			},
	{ "getbuffer",			getbuffer			},
	{ "flushbuffer",		flushbuffer			},
	{ "str2sql",			str2sql				},
	{ "str2sqlbuf",			str2sqlbuf			},
	{ "hex2int",			hex2int				},
	{ "closeconnect",		closeconnect			},
	{ "p_strcasestr",		strcasestr			},
	{ "srv_restart",		server_restart			},
	{ "module_call",		module_call			},
#ifdef WIN32
	{ "gettimeofday",		gettimeofday			},
	{ "opendir",			opendir				},
	{ "readdir",			readdir				},
	{ "closedir",			closedir			},
#endif
	{ NULL, NULL }
};

MOD_INIT mod_init;
MOD_MAIN mod_main;

#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif

void *module_call(CONNECTION *sid, char *fn_name)
{
	int i;

	for (i=0;;i++) {
		if ((mod_functions[i].fn_name==NULL)||(mod_functions[i].fn_ptr==NULL)) break;
		if (strcmp(mod_functions[i].fn_name, fn_name)==0) {
			return mod_functions[i].fn_ptr;
		}
	}
	return NULL;
}

int module_menucall(CONNECTION *sid)
{
	int i;

	for (i=0;;i++) {
		if ((mod_menuitems[i].fn_name==NULL)||(mod_menuitems[i].fn_ptr==NULL)) break;
		if (strlen(mod_menuitems[i].fn_uri)<1) continue;
		if (strncmp(sid->dat->in_RequestURI, mod_menuitems[i].fn_uri, strlen(mod_menuitems[i].fn_uri))==0) {
			mod_main=mod_menuitems[i].fn_ptr;
			mod_main(sid);
			return 1;
		}
	}
	return 0;
}

int module_load(char *modname)
{
#ifdef WIN32
	char *ext="dll";
#else
	char *ext="so";
#endif
	char libname[255];

	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/%s.%s", config.server_dir_lib, modname, ext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))==NULL) goto fail;
	if ((mod_init=(MOD_INIT)dlsym(hinstLib, "mod_init"))==NULL) goto fail;
	if (mod_init(&config, functions, mod_menuitems, mod_functions)!=0) return -1;
	return 0;
fail:
	logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to load %s.%s", modname, ext);
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

int modules_init()
{
	memset((char *)&mod_menuitems, 0, sizeof(mod_menuitems));
	memset((char *)&mod_functions, 0, sizeof(mod_functions));
	module_load("mod_admin");
	module_load("mod_bookmarks");
	module_load("mod_calendar");
	module_load("mod_calls");
	module_load("mod_contacts");
	module_load("mod_files");
	module_load("mod_forums");
	module_load("mod_mail");
	module_load("mod_messages");
	module_load("mod_notes");
	module_load("mod_orders");
	module_load("mod_profile");
	module_load("mod_searches");
	module_load("mod_tasks");
	return 0;
}
