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

static FUNCTION functions[]={
	{ "config_read",		config_read			},
	{ "config_write",		config_write			},
	{ "decode_base64",		decode_base64			},
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
	{ "tcp_bind",			tcp_bind			},
	{ "tcp_accept",			tcp_accept			},
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
#ifdef HAVE_LIBSSL
	{ "ssl_accept",			ssl_accept			},
	{ "ssl_close",			ssl_close			},
	{ "ssl_shutdown",		ssl_shutdown			},
#endif
#ifdef WIN32
	{ "gettimeofday",		gettimeofday			},
	{ "opendir",			opendir				},
	{ "readdir",			readdir				},
	{ "closedir",			closedir			},
#endif
	{ NULL, NULL }
};

SRVMOD_INIT mod_init;
SRVMOD_EXEC mod_exec;
SRVMOD_EXEC mod_cron;

#ifdef WIN32
unsigned _stdcall cronloop(void *x)
#else
void *cronloop(void *x)
#endif
{
	int i;

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
#ifndef OLDLINUX
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) return -2;
#endif
	if (pthread_create(&CronThread, &thr_attr, cronloop, NULL)==-1) {
		log_error(NULL, __FILE__, __LINE__, 0, "cronloop() failed...");
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

int module_load(char *modname)
{
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

	for (i=0;i<MAX_MOD_FUNCTIONS;i++) {
		if (proc.srvmod[i].mod_exec==NULL) break;
		if (strlen(proc.srvmod[i].mod_name)<1) continue;
	}
	if (i==MAX_MOD_FUNCTIONS) return -1;
	memset(libname, 0, sizeof(libname));
	if (strncmp(modname, "srv_", 4)==0) {
		snprintf(libname, sizeof(libname)-1, "%s/%s.%s", proc.config.server_dir_lib, modname, ext);
	} else {
		snprintf(libname, sizeof(libname)-1, "%s/srv_%s.%s", proc.config.server_dir_lib, modname, ext);
	}
	fixslashes(libname);
	snprintf(proc.srvmod[i].mod_name, sizeof(proc.srvmod[i].mod_name)-1, "%s", modname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))==NULL) goto fail;
	if ((srv_init=(void *)dlsym(hinstLib, "mod_init"))==NULL) goto fail;
	if ((srv_exec=(void *)dlsym(hinstLib, "mod_exec"))==NULL) goto fail;
	if ((srv_exit=(void *)dlsym(hinstLib, "mod_exit"))==NULL) goto fail;
	srv_cron=(void *)dlsym(hinstLib, "mod_cron");
	proc.srvmod[i].mod_init=srv_init;
	proc.srvmod[i].mod_exec=srv_exec;
	proc.srvmod[i].mod_exit=srv_exit;
	proc.srvmod[i].mod_cron=srv_cron;
	proc.srvmod[i].hinstLib=hinstLib;
	mod_init=srv_init;
	if (mod_init(&proc, functions)!=0) return -1;
	return 0;
fail:
	log_error("core", __FILE__, __LINE__, 0, "ERROR: Failed to load %s.%s", modname, ext);
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

	snprintf(file, sizeof(file)-1, "%s/servers.cfg", proc.config.server_dir_etc);
	fixslashes(file);
	fp=fopen(file, "r");
	if (fp==NULL) {
		fp=fopen(file, "w");
		if (fp==NULL) {
			log_error("core", __FILE__, __LINE__, 0, "ERROR: Failed to create servers.cfg");
			return -1;
		}
		fprintf(fp, "# This file specifies which servers NullLogic Groupware should load.\n\n");
		fprintf(fp, "srv_httpd\n");
		fprintf(fp, "#srv_pop3d\n");
		fprintf(fp, "#srv_smtpd\n");
		fclose(fp);
		fp=fopen(file, "r");
	}
	if (fp==NULL) {
		log_error("core", __FILE__, __LINE__, 0, "ERROR: Failed to read servers.cfg");
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
