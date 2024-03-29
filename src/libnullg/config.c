/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "libnullg.h"

typedef	void  (*CONF_CALLBACK)(char *, char *);
/*
char *preppath(const char *format, ...)
{
	va_list ap;
	char *ptemp;

	va_start(ap, format);
	vsnprintf(tmpline, sizeof(tmpline)-1, format, ap);
	va_end(ap);
	ptemp=tmpline;
 	while (*ptemp) {
#ifdef WIN32
 		if (*ptemp=='/') *ptemp='\\';
#else
 		if (*ptemp=='\\') *ptemp='/';
#endif
		ptemp++;
	}
	return tmpline;
}
*/
int config_new(nsp_state *N)
{
	char basedir[PATH_MAX];
	unsigned int i;
	obj_t *tobj;

#ifdef WIN32
	GetCurrentDirectory(sizeof(basedir)-1, basedir);
#else
	if (getcwd(basedir, sizeof(basedir)-1)==NULL) return -1;
#endif
	fixslashes(basedir);
	i=strlen(basedir)-1;
	while (i>-1) {
		if ((basedir[i]!='\\')&&(basedir[i]!='/')) { basedir[i]='\0'; i--; continue; }
		break;
	}
	while (i>-1) {
		if ((basedir[i]=='\\')||(basedir[i]=='/')) { basedir[i]='\0'; i--; continue; }
		break;
	};
	tobj=nsp_getobj(N, &N->g, "CONFIG");
	if (nsp_isnull(tobj)) tobj=nsp_settable(N, &N->g, "CONFIG");
	if (tobj->val->type!=NT_TABLE) return 0;
	nsp_setstr(N, tobj, "uid", "nullg", 5);
	nsp_setstr(N, tobj, "gid", "nullg", 5);
#ifndef WIN32
	nsp_setstr(N, tobj, "umask", "007", 3);
#endif
	nsp_setnum(N, tobj, "log_level", 1);
	nsp_setstr(N, tobj, "default_language", "en", 2);
	nsp_setstr(N, tobj, "host_name", "localhost", 9);
/*	nsp_setstr(N, tobj, "bin_path", preppath("%s/bin", basedir)); */
/*	nsp_setstr(N, tobj, "etc_path", preppath("%s/etc", basedir)); */
/*	nsp_setstr(N, tobj, "lib_path", preppath("%s/lib", basedir)); */
/*	nsp_setstr(N, tobj, "var_path", preppath("%s/var", basedir)); */
//	nsp_setstr(N, tobj, "sql_server_type", "SQLITE", 6);
	nsp_exec(N, "CONFIG['sql']={sql_server_type='SQLITE'}");
/*	nsp_setstr(N, tobj, "ssl_cert_file", preppath("%s/etc/ssl-cert.pem", basedir)); */
/*	nsp_setstr(N, tobj, "ssl_key_file", preppath("%s/etc/ssl-priv.pem", basedir)); */
	nsp_exec(N, "CONFIG['modules']={}");
	nsp_exec(N, "CONFIG['modules']['httpd']={interface='INADDR_ANY',port=80,max_connections=50,max_keepalive=15,max_idle=120,max_post_size=32*1024*1024,session_limit=1}");
	//nsp_exec(N, "CONFIG['modules']['httpd']['modules']={'mod_admin','mod_bookmarks','mod_calendar','mod_calls','mod_cgi','mod_contacts','mod_email','mod_files','mod_finance','mod_forums','mod_messages','mod_notes','mod_profile','mod_projects','mod_searches','mod_spellcheck','mod_tasks','mod_weblog','mod_xmlrpc'}");
	nsp_exec(N, "CONFIG['modules']['pop3d']={interface='localhost',port=110,max_connections=50,max_idle=120}");
	nsp_exec(N, "CONFIG['modules']['smtpd']={interface='localhost',port=25, max_connections=50,max_idle=120,retry_delay=300}");
//	nsp_exec(N, "CONFIG['mod_email'] = { max_idle=120 }");
/*	nsp_exec(N, "CONFIG['mod_files'] = { filter_program='/usr/bin/nullg-scanfile.sh' }"); */
	return 0;
}

int config_read(nsp_state *N, char *section, void *callback)
{
	CONF_CALLBACK conf_callback=(CONF_CALLBACK)callback;
	char config_filename[PATH_MAX];
	obj_t *tobj;
	obj_t *cobj;

	memset(config_filename, 0, sizeof(config_filename));
	tobj=nsp_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type==NT_TABLE) goto _haveconf; /* try skipping the re-read... */

	if (tobj->val->type==NT_TABLE) {
		cobj=nsp_getobj(N, tobj, "filename");
		if (cobj->val->type==NT_STRING) {
			snprintf(config_filename, PATH_MAX-1, "%s", cobj->val->d.str);
		}
	}
	if (strlen(config_filename)>0) {
		if (nsp_execfile(N, config_filename)==0) goto _readconf;
	} else {
		snprintf(config_filename, PATH_MAX-1, "%s.conf", SERVER_BASENAME);
		if (nsp_execfile(N, config_filename)==0) goto _readconf;
		snprintf(config_filename, PATH_MAX-1, "../etc/%s.conf", SERVER_BASENAME);
		if (nsp_execfile(N, config_filename)==0) goto _readconf;
#ifndef WIN32
/*		snprintf(config_filename, PATH_MAX-1, "%s/%s.conf", proc.config.dir_etc, SERVER_BASENAME); */
		snprintf(config_filename, PATH_MAX-1, "%s/%s.conf", ETCDIR, SERVER_BASENAME);
		if (nsp_execfile(N, config_filename)==0) goto _readconf;
#endif
	}
	/* i gave up looking for the config file and just made one up. */
	if (config_new(N)==0) goto _readconf;
	printf("ERROR: Failed to read configuration file.");
	log_error(N, "core", __FILE__, __LINE__, 0, "Failed to read configuration file.");
	memset(config_filename, 0, PATH_MAX);
	/* return -1; */
	return 0;
_readconf:
	tobj=nsp_getobj(N, &N->g, "CONFIG");
	if (tobj->val->type!=NT_TABLE) return 0;
	nsp_setstr(N, tobj, "filename", config_filename, strlen(config_filename));
_haveconf:
	if (conf_callback==NULL) return 0;
	if (strlen(section)>0) {
		if (strcmp(section, "sql") == 0 || strcmp(section, "tls") == 0) {
			tobj = nsp_getobj(N, tobj, section);
		} else {
			tobj = nsp_getobj(N, nsp_getobj(N, tobj, "modules"), section);
		}
		if (tobj->val->type!=NT_TABLE) return 0;
	}
	for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
		if (strcmp(cobj->name, "_retval")==0) continue;
		if (cobj->val->type==NT_NULL) {
		} else if (cobj->val->type==NT_BOOLEAN) {
			conf_callback(cobj->name, nsp_tostr(N, cobj));
		} else if (cobj->val->type==NT_NUMBER) {
			conf_callback(cobj->name, nsp_tostr(N, cobj));
		} else if (cobj->val->type==NT_STRING) {
			conf_callback(cobj->name, nsp_tostr(N, cobj));
		} else if (cobj->val->type==NT_TABLE) {
		}
	}
	return 0;
}

int config_write(nsp_state *N)
{
	return 0;
}
