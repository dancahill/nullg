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

typedef	void  (*CONF_CALLBACK)(char *, char *);

int config_read(char *section, void *callback)
{
	CONF_CALLBACK conf_callback=callback;
	FILE *fp=NULL;
	char line[512];
	struct stat sb;
	char *pVar;
	char *pVal;
	int i;
	short int match;

	if (strlen(proc.config_filename)>0) {
		fp=fopen(proc.config_filename, "r");
	} else {
		/* try to open the config file */
		/* try the current directory first, then ../etc/, then the default etc/ */
		if (fp==NULL) {
			snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s.conf", SERVER_BASENAME);
			fp=fopen(proc.config_filename, "r");
		}
		if (fp==NULL) {
			snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "../etc/%s.conf", SERVER_BASENAME);
			fixslashes(proc.config_filename);
			fp=fopen(proc.config_filename, "r");
		}
		if (fp==NULL) {
			snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s/%s.conf", proc.config.dir_etc, SERVER_BASENAME);
			fixslashes(proc.config_filename);
			fp=fopen(proc.config_filename, "r");
		}
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		memset(proc.config_filename, 0, sizeof(proc.config_filename));
		if (stat(proc.config.dir_etc, &sb)!=0) {
			log_error("core", __FILE__, __LINE__, 0, CONFIG_NODIR, proc.config.dir_etc);
			/* return -1; */
		};
		snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "../etc/%s.conf", SERVER_BASENAME);
		config_write(&proc.config);
		/* goto sanity_check; */
		return 0;
	}
	/* else if config file does exist, read it */
	match=0;
	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if ((pVar=strchr(line, '#'))!=NULL) *pVar='\0';
		if ((pVar=strchr(line, ';'))!=NULL) *pVar='\0';
		pVar=line;
		pVal=strchr(line, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			match=0;
			pVar++;
			*pVal='\0';
			if (strcasecmp(pVar, section)==0) match=1;
			continue;
		}
		if (match) {
			pVar=line;
			pVal=line;
			while ((pVal[0]!='=')&&(pVal[1]!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			if ((pVal[0]=='"')&&(pVal[strlen(pVal)-1]=='"')) {
				pVal++;
				pVal[strlen(pVal)-1]='\0';
			}
			if (pVar[0]!='\0') {
				conf_callback(pVar, pVal);
			}
		}
	}
	fclose(fp);
	return 0;
}

int config_write(GLOBAL_CONFIG *config)
{
	FILE *fp=NULL;

	fixslashes(proc.config_filename);
	if ((fp=fopen(proc.config_filename, "w"))==NULL) return -1;
	fprintf(fp, "# %s\n\n", CONFIG_HEAD);
	fprintf(fp, "[global]\n");
	fprintf(fp, "   log level         = %d\n", config->loglevel);
	fprintf(fp, "   host name         = \"%s\"\n", config->hostname);
	fprintf(fp, "   base path         = \"%s\"\n", config->dir_base);
	fprintf(fp, "   bin path          = \"%s\"\n", config->dir_bin);
	fprintf(fp, "   etc path          = \"%s\"\n", config->dir_etc);
	fprintf(fp, "   lib path          = \"%s\"\n", config->dir_lib);
	fprintf(fp, "   var path          = \"%s\"\n", config->dir_var);
	fprintf(fp, "   sql server type   = \"%s\"\n", config->sql_type);
	fprintf(fp, "\n");
	fprintf(fp, "   load module       = srv_httpd\n");
	fprintf(fp, "   load module       = srv_pop3d\n");
	fprintf(fp, "   load module       = srv_smtpd\n");
	fprintf(fp, "   load module       = srv_smtpq\n");
	fprintf(fp, "\n[srv_httpd]\n");
	fprintf(fp, "\n");
	fprintf(fp, "   load module       = mod_html\n");
	fprintf(fp, "   load module       = mod_admin\n");
	fprintf(fp, "   load module       = mod_bookmarks\n");
	fprintf(fp, "   load module       = mod_calendar\n");
	fprintf(fp, "   load module       = mod_calls\n");
	fprintf(fp, "   load module       = mod_cgi\n");
	fprintf(fp, "   load module       = mod_contacts\n");
	fprintf(fp, "   load module       = mod_mail\n");
	fprintf(fp, "   load module       = mod_files\n");
	fprintf(fp, "   load module       = mod_forums\n");
	fprintf(fp, "   load module       = mod_messages\n");
	fprintf(fp, "   load module       = mod_notes\n");
	fprintf(fp, ";  load module       = mod_orders\n");
	fprintf(fp, "   load module       = mod_profile\n");
	fprintf(fp, ";  load module       = mod_projects\n");
	fprintf(fp, "   load module       = mod_searches\n");
	fprintf(fp, "   load module       = mod_tasks\n");
	fprintf(fp, "   load module       = mod_xmlrpc\n");
	fclose(fp);
	return 0;
}

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "uid")==0) {
		strncpy(proc.config.uid, val, sizeof(proc.config.uid)-1);
	} else if (strcmp(var, "gid")==0) {
		strncpy(proc.config.gid, val, sizeof(proc.config.gid)-1);
	} else if (strcmp(var, "log level")==0) {
		proc.config.loglevel=atoi(val);
	} else if (strcmp(var, "host name")==0) {
		strncpy(proc.config.hostname, val, sizeof(proc.config.hostname)-1);
	} else if (strcmp(var, "base path")==0) {
		strncpy(proc.config.dir_base, val, sizeof(proc.config.dir_base)-1);
	} else if (strcmp(var, "bin path")==0) {
		strncpy(proc.config.dir_bin, val, sizeof(proc.config.dir_bin)-1);
	} else if (strcmp(var, "etc path")==0) {
		strncpy(proc.config.dir_etc, val, sizeof(proc.config.dir_etc)-1);
	} else if (strcmp(var, "lib path")==0) {
		strncpy(proc.config.dir_lib, val, sizeof(proc.config.dir_lib)-1);
	} else if (strcmp(var, "var path")==0) {
		strncpy(proc.config.dir_var, val, sizeof(proc.config.dir_var)-1);
	} else if (strcmp(var, "var_backup path")==0) {
		strncpy(proc.config.dir_var_backup, val, sizeof(proc.config.dir_var_backup)-1);
	} else if (strcmp(var, "var_cgi path")==0) {
		strncpy(proc.config.dir_var_cgi, val, sizeof(proc.config.dir_var_cgi)-1);
	} else if (strcmp(var, "var_db path")==0) {
		strncpy(proc.config.dir_var_db, val, sizeof(proc.config.dir_var_db)-1);
	} else if (strcmp(var, "var_domains path")==0) {
		strncpy(proc.config.dir_var_domains, val, sizeof(proc.config.dir_var_domains)-1);
	} else if (strcmp(var, "var_htdocs path")==0) {
		strncpy(proc.config.dir_var_htdocs, val, sizeof(proc.config.dir_var_htdocs)-1);
	} else if (strcmp(var, "var_log path")==0) {
		strncpy(proc.config.dir_var_log, val, sizeof(proc.config.dir_var_log)-1);
	} else if (strcmp(var, "var_spool path")==0) {
		strncpy(proc.config.dir_var_spool, val, sizeof(proc.config.dir_var_spool)-1);
	} else if (strcmp(var, "var_tmp path")==0) {
		strncpy(proc.config.dir_var_tmp, val, sizeof(proc.config.dir_var_tmp)-1);
	} else if (strcmp(var, "sql server type")==0) {
		strncpy(proc.config.sql_type, val, sizeof(proc.config.sql_type)-1);
	} else if (strcmp(var, "sql host name")==0) {
		strncpy(proc.config.sql_hostname, val, sizeof(proc.config.sql_hostname)-1);
	} else if (strcmp(var, "sql port")==0) {
		proc.config.sql_port=atoi(val);
	} else if (strcmp(var, "sql database name")==0) {
		strncpy(proc.config.sql_dbname, val, sizeof(proc.config.sql_dbname)-1);
	} else if (strcmp(var, "sql user name")==0) {
		strncpy(proc.config.sql_username, val, sizeof(proc.config.sql_username)-1);
	} else if (strcmp(var, "sql password")==0) {
		strncpy(proc.config.sql_password, val, sizeof(proc.config.sql_password)-1);
	} else if (strcmp(var, "sql odbc dsn")==0) {
		strncpy(proc.config.sql_odbc_dsn, val, sizeof(proc.config.sql_odbc_dsn)-1);
	} else if (strcmp(var, "ssl cert file")==0) {
		strncpy(proc.config.ssl_cert, val, sizeof(proc.config.ssl_cert)-1);
	} else if (strcmp(var, "ssl key file")==0) {
		strncpy(proc.config.ssl_key, val, sizeof(proc.config.ssl_key)-1);
	} else if (strcmp(var, "load module")==0) {
	} else {
		log_error("http", __FILE__, __LINE__, 1, "unknown configuration directive '%s'", var);
	}
	return;
}

static void conf_callback_modules(char *var, char *val)
{
	if (strcmp(var, "load module")==0) {
		module_load(val);
	}
	return;
}

int conf_read()
{
#ifdef WIN32
	char slash='\\';
#else
	char slash='/';
#endif
	char *ptemp;
	short int founddir=0;

	/* define default values */
	gethostname(proc.config.hostname, sizeof(proc.config.hostname)-1);
	snprintf(proc.config.uid, sizeof(proc.config.uid)-1, "%s", DEFAULT_SERVER_USERNAME);
	ptemp=proc.program_name;
	if (*ptemp=='\"') ptemp++;
#ifdef WIN32
	snprintf(proc.config.dir_base, sizeof(proc.config.dir_base)-1, "%s", ptemp);
#else
	if (getcwd(proc.config.dir_base, sizeof(proc.config.dir_base)-1)==NULL) return -1;
	strcat(proc.config.dir_base, "/");
#endif
	if (strrchr(proc.config.dir_base, slash)!=NULL) {
		ptemp=strrchr(proc.config.dir_base, slash);
		*ptemp='\0';
		chdir(proc.config.dir_base);
		if (strrchr(proc.config.dir_base, slash)!=NULL) {
			ptemp=strrchr(proc.config.dir_base, slash);
			*ptemp='\0';
			founddir=1;
		}
	}
	if (!founddir) {
		snprintf(proc.config.dir_base, sizeof(proc.config.dir_base)-1, "%s", DEFAULT_BASE_DIR);
	}
	snprintf(proc.config.dir_bin, sizeof(proc.config.dir_bin)-1, "%s/bin", proc.config.dir_base);
	snprintf(proc.config.dir_etc, sizeof(proc.config.dir_etc)-1, "%s/etc", proc.config.dir_base);
	snprintf(proc.config.dir_lib, sizeof(proc.config.dir_lib)-1, "%s/lib", proc.config.dir_base);
	snprintf(proc.config.dir_var, sizeof(proc.config.dir_var)-1, "%s/var", proc.config.dir_base);
#ifdef HAVE_SQLITE3
	strncpy(proc.config.sql_type, "SQLITE", sizeof(proc.config.sql_type)-1);
#endif
	proc.config.loglevel=1;
	proc.config.sql_maxconn=50;

	config_read("global", conf_callback);


	if (!strlen(proc.config.dir_var_backup)) {
		snprintf(proc.config.dir_var_backup,  sizeof(proc.config.dir_var_backup)-1,  "%s/backup", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_cgi)) {
		snprintf(proc.config.dir_var_cgi,     sizeof(proc.config.dir_var_cgi)-1,     "%s/cgi-bin", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_db)) {
		snprintf(proc.config.dir_var_db,      sizeof(proc.config.dir_var_db)-1,      "%s/db", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_domains)) {
		snprintf(proc.config.dir_var_domains, sizeof(proc.config.dir_var_domains)-1, "%s/domains", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_htdocs)) {
		snprintf(proc.config.dir_var_htdocs,  sizeof(proc.config.dir_var_htdocs)-1,  "%s/htdocs", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_log)) {
		snprintf(proc.config.dir_var_log,     sizeof(proc.config.dir_var_log)-1,     "%s/log", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_spool)) {
		snprintf(proc.config.dir_var_spool,   sizeof(proc.config.dir_var_spool)-1,   "%s/spool", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_tmp)) {
		snprintf(proc.config.dir_var_tmp,     sizeof(proc.config.dir_var_tmp)-1,     "%s/tmp", proc.config.dir_var);
	}
	fixslashes(proc.config.dir_base);
	fixslashes(proc.config.dir_bin);
	fixslashes(proc.config.dir_etc);
	fixslashes(proc.config.dir_lib);
	fixslashes(proc.config.dir_var);
	fixslashes(proc.config.dir_var_backup);
	fixslashes(proc.config.dir_var_cgi);
	fixslashes(proc.config.dir_var_db);
	fixslashes(proc.config.dir_var_domains);
	fixslashes(proc.config.dir_var_htdocs);
	fixslashes(proc.config.dir_var_log);
	fixslashes(proc.config.dir_var_spool);
	fixslashes(proc.config.dir_var_tmp);

	return 0;
}

int conf_read_modules()
{
	config_read("global", conf_callback_modules);
	return 0;
}
