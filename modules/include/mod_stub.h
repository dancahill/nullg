/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "mod_substub.h"

void *_get_func(char *fn_name)
{
	int i;

	for (i=0;;i++) {
		if ((functions[i].fn_name==NULL)||(functions[i].fn_ptr==NULL)) break;
		if (strcmp(functions[i].fn_name, fn_name)==0) {
//			if (logerror!=NULL) logerror(NULL, "mod_stub.h", __LINE__, "[%s]=[%d]", functions[i].fn_name, functions[i].fn_ptr);
			return functions[i].fn_ptr;
		}
	}
	if (logerror!=NULL) logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to find function %s", fn_name);
	return NULL;
}

int mod_import()
{
	logerror=NULL;
	if ((logerror                   = _get_func("logerror"))==NULL) return -1;
	if ((logaccess                  = _get_func("logaccess"))==NULL) return -1;
	if ((auth_priv                  = _get_func("auth_priv"))==NULL) return -1;
	if ((auth_setcookie             = _get_func("auth_setcookie"))==NULL) return -1;
	if ((auth_setpass               = _get_func("auth_setpass"))==NULL) return -1;
	if ((config_read                = _get_func("config_read"))==NULL) return -1;
	if ((config_write               = _get_func("config_write"))==NULL) return -1;
	if ((db_log_activity            = _get_func("db_log_activity"))==NULL) return -1;
	if ((htpage_header              = _get_func("htpage_header"))==NULL) return -1;
	if ((htpage_footer              = _get_func("htpage_footer"))==NULL) return -1;
	if ((htpage_topmenu             = _get_func("htpage_topmenu"))==NULL) return -1;
	if ((htselect_contact           = _get_func("htselect_contact"))==NULL) return -1;
	if ((htselect_day               = _get_func("htselect_day"))==NULL) return -1;
	if ((htselect_eventstatus       = _get_func("htselect_eventstatus"))==NULL) return -1;
	if ((htselect_group             = _get_func("htselect_group"))==NULL) return -1;
	if ((htselect_hour              = _get_func("htselect_hour"))==NULL) return -1;
	if ((htselect_minutes           = _get_func("htselect_minutes"))==NULL) return -1;
	if ((htselect_month             = _get_func("htselect_month"))==NULL) return -1;
	if ((htselect_number            = _get_func("htselect_number"))==NULL) return -1;
	if ((htselect_priority          = _get_func("htselect_priority"))==NULL) return -1;
	if ((htselect_qhours            = _get_func("htselect_qhours"))==NULL) return -1;
	if ((htselect_qminutes          = _get_func("htselect_qminutes"))==NULL) return -1;
	if ((htselect_reminder          = _get_func("htselect_reminder"))==NULL) return -1;
	if ((htselect_time              = _get_func("htselect_time"))==NULL) return -1;
	if ((htselect_timezone          = _get_func("htselect_timezone"))==NULL) return -1;
	if ((htselect_user              = _get_func("htselect_user"))==NULL) return -1;
	if ((htselect_year              = _get_func("htselect_year"))==NULL) return -1;
	if ((htselect_zone              = _get_func("htselect_zone"))==NULL) return -1;
	if ((htview_callaction          = _get_func("htview_callaction"))==NULL) return -1;
	if ((htview_contact             = _get_func("htview_contact"))==NULL) return -1;
	if ((htview_eventclosingstatus  = _get_func("htview_eventclosingstatus"))==NULL) return -1;
	if ((htview_eventstatus         = _get_func("htview_eventstatus"))==NULL) return -1;
	if ((htview_eventtype           = _get_func("htview_eventtype"))==NULL) return -1;
	if ((htview_holiday             = _get_func("htview_holiday"))==NULL) return -1;
	if ((htview_reminder            = _get_func("htview_reminder"))==NULL) return -1;
	if ((htview_user                = _get_func("htview_user"))==NULL) return -1;
	if ((md5_init                   = _get_func("md5_init"))==NULL) return -1;
	if ((md5_update                 = _get_func("md5_update"))==NULL) return -1;
	if ((md5_final                  = _get_func("md5_final"))==NULL) return -1;
	if ((md5_crypt                  = _get_func("md5_crypt"))==NULL) return -1;
	if ((sql_freeresult             = _get_func("sql_freeresult"))==NULL) return -1;
	if ((sql_getname                = _get_func("sql_getname"))==NULL) return -1;
	if ((sql_getvalue               = _get_func("sql_getvalue"))==NULL) return -1;
	if ((sql_getvaluebyname         = _get_func("sql_getvaluebyname"))==NULL) return -1;
	if ((sql_numfields              = _get_func("sql_numfields"))==NULL) return -1;
	if ((sql_numtuples              = _get_func("sql_numtuples"))==NULL) return -1;
	if ((sql_query                  = _get_func("sql_query"))==NULL) return -1;
	if ((sql_queryf                 = _get_func("sql_queryf"))==NULL) return -1;
	if ((sql_update                 = _get_func("sql_update"))==NULL) return -1;
	if ((sql_updatef                = _get_func("sql_updatef"))==NULL) return -1;
	if ((time_sql2datetext          = _get_func("time_sql2datetext"))==NULL) return -1;
	if ((time_sql2timetext          = _get_func("time_sql2timetext"))==NULL) return -1;
	if ((time_sql2unix              = _get_func("time_sql2unix"))==NULL) return -1;
	if ((time_tzoffset              = _get_func("time_tzoffset"))==NULL) return -1;
	if ((time_tzoffset2             = _get_func("time_tzoffset2"))==NULL) return -1;
	if ((time_unix2sql              = _get_func("time_unix2sql"))==NULL) return -1;
	if ((time_unix2text             = _get_func("time_unix2text"))==NULL) return -1;
	if ((time_unix2lotimetext       = _get_func("time_unix2lotimetext"))==NULL) return -1;
	if ((time_unix2timetext         = _get_func("time_unix2timetext"))==NULL) return -1;
	if ((time_unix2datetext         = _get_func("time_unix2datetext"))==NULL) return -1;
	if ((time_wmgetdate             = _get_func("time_wmgetdate"))==NULL) return -1;
	if ((decodeurl                  = _get_func("decodeurl"))==NULL) return -1;
	if ((decode_b64s                = _get_func("decode_b64s"))==NULL) return -1;
	if ((get_mime_type              = _get_func("get_mime_type"))==NULL) return -1;
	if ((getgetenv                  = _get_func("getgetenv"))==NULL) return -1;
	if ((getmimeenv                 = _get_func("getmimeenv"))==NULL) return -1;
	if ((getpostenv                 = _get_func("getpostenv"))==NULL) return -1;
	if ((getxmlenv                  = _get_func("getxmlenv"))==NULL) return -1;
	if ((getxmlparam                = _get_func("getxmlparam"))==NULL) return -1;
	if ((getxmlstruct               = _get_func("getxmlstruct"))==NULL) return -1;
	if ((prints                     = _get_func("prints"))==NULL) return -1;
	if ((printhex                   = _get_func("printhex"))==NULL) return -1;
	if ((printht                    = _get_func("printht"))==NULL) return -1;
	if ((printline                  = _get_func("printline"))==NULL) return -1;
	if ((printline2                 = _get_func("printline2"))==NULL) return -1;
	if ((str2html                   = _get_func("str2html"))==NULL) return -1;
	if ((strncatf                   = _get_func("strncatf"))==NULL) return -1;
	if ((send_error                 = _get_func("send_error"))==NULL) return -1;
	if ((send_header                = _get_func("send_header"))==NULL) return -1;
	if ((fixslashes                 = _get_func("fixslashes"))==NULL) return -1;
	if ((striprn                    = _get_func("striprn"))==NULL) return -1;
	if ((flushbuffer                = _get_func("flushbuffer"))==NULL) return -1;
	if ((str2sql                    = _get_func("str2sql"))==NULL) return -1;
	if ((str2sqlbuf                 = _get_func("str2sqlbuf"))==NULL) return -1;
	if ((getbuffer                  = _get_func("getbuffer"))==NULL) return -1;
	if ((filesend                   = _get_func("filesend"))==NULL) return -1;
	if ((tcp_send                   = _get_func("tcp_send"))==NULL) return -1;
	if ((hex2int                    = _get_func("hex2int"))==NULL) return -1;
	if ((closeconnect               = _get_func("closeconnect"))==NULL) return -1;
	if ((p_strcasestr               = _get_func("p_strcasestr"))==NULL) return -1;
	if ((srv_restart                = _get_func("srv_restart"))==NULL) return -1;
	if ((module_exists              = _get_func("module_exists"))==NULL) return -1;
	if ((module_call                = _get_func("module_call"))==NULL) return -1;
#ifdef WIN32
	if ((gettimeofday               = _get_func("gettimeofday"))==NULL) return -1;
	if ((opendir                    = _get_func("opendir"))==NULL) return -1;
	if ((readdir                    = _get_func("readdir"))==NULL) return -1;
	if ((closedir                   = _get_func("closedir"))==NULL) return -1;
#endif
//	if ((notessublist               = _get_func("notessublist"))==NULL) return -1;
//	logaccess(NULL, 1, "HEY FROM STUBBY!");
	return 0;
}

int mod_export_main(char *mod_name, char *mod_menuname, char *mod_menuuri, char *fn_name, char *fn_uri, void *fn_ptr)
{
	int i;

	for (i=0;i<50;i++) {
		if (strlen(proc->mod_menuitems[i].mod_name)>0) continue;
		if (strlen(proc->mod_menuitems[i].mod_menuname)>0) continue;
		if (proc->mod_menuitems[i].fn_ptr!=NULL) continue;
		snprintf(proc->mod_menuitems[i].mod_name, sizeof(proc->mod_menuitems[i].mod_name)-1, "%s", mod_name);
		snprintf(proc->mod_menuitems[i].mod_menuname, sizeof(proc->mod_menuitems[i].mod_menuname)-1, "%s", mod_menuname);
		snprintf(proc->mod_menuitems[i].mod_menuuri, sizeof(proc->mod_menuitems[i].mod_menuuri)-1, "%s", mod_menuuri);
		snprintf(proc->mod_menuitems[i].fn_name, sizeof(proc->mod_menuitems[i].fn_name)-1, "%s", fn_name);
		snprintf(proc->mod_menuitems[i].fn_uri, sizeof(proc->mod_menuitems[i].fn_uri)-1, "%s", fn_uri);
		proc->mod_menuitems[i].fn_ptr=fn_ptr;
		return 0;
	}
	if (logerror!=NULL) logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to find function %s", fn_name);
	return -1;
}

int mod_export_function(char *mod_name, char *fn_name, void *fn_ptr)
{
	int i;

	for (i=0;i<50;i++) {
		if (strlen(proc->mod_functions[i].mod_name)>0) continue;
		if (strlen(proc->mod_functions[i].fn_name)>0) continue;
		if (proc->mod_functions[i].fn_ptr!=NULL) continue;
		snprintf(proc->mod_functions[i].mod_name, sizeof(proc->mod_functions[i].mod_name)-1, "%s", mod_name);
		snprintf(proc->mod_functions[i].fn_name, sizeof(proc->mod_functions[i].fn_name)-1, "%s", fn_name);
		proc->mod_functions[i].fn_ptr=fn_ptr;
		return 0;
	}
	if (logerror!=NULL) logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to find function %s", fn_name);
	return -1;
}

#ifdef WIN32
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpReserved)
{
	switch (dwFunction) {
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
#else
/*
void _init()
{
	return;
} 
*/
#endif
