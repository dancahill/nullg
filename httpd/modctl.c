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
#include "http_main.h"

static FUNCTION functions[]={
	{ "auth_setcookie",		auth_setcookie			},
	{ "auth_setpass",		auth_setpass			},
	{ "auth_priv",			auth_priv			},
	{ "config_read",		config_read			},
	{ "config_write",		config_write			},
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
	{ "htscript_showpage",		htscript_showpage		},
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
	{ "time_sql2datetext",		time_sql2datetext		},
	{ "time_sql2timetext",		time_sql2timetext		},
	{ "time_sql2unix",		time_sql2unix			},
	{ "time_tzoffset",		time_tzoffset			},
	{ "time_tzoffset2",		time_tzoffset2			},
	{ "time_tzoffsetcon",		time_tzoffsetcon		},
	{ "time_unix2sql",		time_unix2sql			},
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
	{ "logaccess",			logaccess			},
	{ "logerror",			logerror			},
	{ "decodeurl",			decodeurl			},
	{ "encodeurl",			encodeurl			},
	{ "decode_b64s",		DecodeBase64string		},
	{ "filesend",			filesend			},
	{ "str2html",			str2html			},
	{ "strncatf",			strncatf			},
	{ "send_header",		send_header			},
	{ "send_error",			send_error			},
	{ "printline",			printline			},
	{ "striprn",			striprn				},
	{ "prints",			prints				},
	{ "raw_prints",			raw_prints			},
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
	{ "tcp_send",			tcp_send			},
	{ "p_strcasestr",		p_strcasestr			},
	{ "srv_restart",		server_restart			},
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

MOD_INIT mod_init;
MOD_MAIN mod_main;

#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif

int module_exists(CONN *sid, char *mod_name)
{
	int i;

	for (i=0;;i++) {
		if ((proc.mod_menuitems[i].fn_name==NULL)||(proc.mod_menuitems[i].fn_ptr==NULL)) break;
		if (strcmp(proc.mod_menuitems[i].mod_name, mod_name)==0) return 1;
	}
	return 0;
}

void *module_call(CONN *sid, char *fn_name)
{
	int i;

	for (i=0;;i++) {
		if ((proc.mod_functions[i].fn_name==NULL)||(proc.mod_functions[i].fn_ptr==NULL)) break;
		if (strcmp(proc.mod_functions[i].fn_name, fn_name)==0) {
			return proc.mod_functions[i].fn_ptr;
		}
	}
	return NULL;
}

int module_menucall(CONN *sid)
{
	int i;

	for (i=0;;i++) {
		if ((proc.mod_menuitems[i].fn_name==NULL)||(proc.mod_menuitems[i].fn_ptr==NULL)) break;
		if (strlen(proc.mod_menuitems[i].fn_uri)<1) continue;
		if (strncmp(sid->dat->in_RequestURI, proc.mod_menuitems[i].fn_uri, strlen(proc.mod_menuitems[i].fn_uri))==0) {
			mod_main=proc.mod_menuitems[i].fn_ptr;
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
	snprintf(libname, sizeof(libname)-1, "%s/%s.%s", proc.config.server_dir_lib, modname, ext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))==NULL) goto fail;
	if ((mod_init=(MOD_INIT)dlsym(hinstLib, "mod_init"))==NULL) goto fail;
	if (mod_init(&proc, functions)!=0) return -1;
	return 0;
fail:
	logerror(NULL, __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s", modname, ext);
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

int modules_init()
{
	FILE *fp=NULL;
	char line[512];
	char file[256];
	int i;

	memset((char *)&proc.mod_menuitems, 0, sizeof(MODULE_MENU));
	memset((char *)&proc.mod_functions, 0, sizeof(MODULE_FUNC));
	snprintf(file, sizeof(file)-1, "%s/modules.cfg", proc.config.server_dir_etc);
	fixslashes(file);
	fp=fopen(file, "r");
	if (fp==NULL) {
		fp=fopen(file, "w");
		if (fp==NULL) {
			logerror(NULL, __FILE__, __LINE__, 0, "ERROR: Failed to create modules.cfg");
			return -1;
		}
		fprintf(fp, "# This file specifies which modules NullLogic Groupware should load.\n\n");
		fprintf(fp, "mod_html\n");
		fprintf(fp, "mod_admin\n");
		fprintf(fp, "mod_bookmarks\n");
		fprintf(fp, "mod_calendar\n");
		fprintf(fp, "mod_calls\n");
		fprintf(fp, "#mod_cgi\n");
		fprintf(fp, "mod_contacts\n");
		fprintf(fp, "mod_mail\n");
		fprintf(fp, "mod_files\n");
		fprintf(fp, "mod_forums\n");
		fprintf(fp, "mod_messages\n");
		fprintf(fp, "mod_notes\n");
		fprintf(fp, "#mod_orders\n");
		fprintf(fp, "mod_profile\n");
		fprintf(fp, "mod_searches\n");
		fprintf(fp, "mod_tasks\n");
		fprintf(fp, "#mod_xmlrpc\n");
		fclose(fp);
		fp=fopen(file, "r");
	}
	if (fp==NULL) {
		logerror(NULL, __FILE__, __LINE__, 0, "ERROR: Failed to read modules.cfg");
		return -1;
	}
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if (isalpha(line[0])) {
			module_load(line);
		}
	}
	fclose(fp);
	return 0;
}
