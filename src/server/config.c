/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
/*	struct stat sb; */
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
#ifdef WIN32
		if (fp==NULL) {
			snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s/%s.conf", proc.config.dir_etc, SERVER_BASENAME);
			fixslashes(proc.config_filename);
			fp=fopen(proc.config_filename, "r");
		}
#else
		if (fp==NULL) {
			snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s/%s.conf", ETCDIR, SERVER_BASENAME);
			fixslashes(proc.config_filename);
			fp=fopen(proc.config_filename, "r");
		}
#endif
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		printf("ERROR: Failed to read configuration file.");
		log_error("core", __FILE__, __LINE__, 0, "Failed to read configuration file.");
		memset(proc.config_filename, 0, sizeof(proc.config_filename));
		/* return -1; */
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
	return 0;
}

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "uid")==0) {
		strncpy(proc.config.uid, val, sizeof(proc.config.uid)-1);
	} else if (strcmp(var, "gid")==0) {
		strncpy(proc.config.gid, val, sizeof(proc.config.gid)-1);
	} else if (strcmp(var, "umask")==0) {
		sscanf(val, "%o", &proc.config.umask);
		proc.config.umask=proc.config.umask&0777;
	} else if (strcmp(var, "log level")==0) {
		proc.config.loglevel=atoi(val);
	} else if (strcmp(var, "default language")==0) {
		strncpy(proc.config.langcode, val, sizeof(proc.config.langcode)-1);
	} else if (strcmp(var, "host name")==0) {
		strncpy(proc.config.hostname, val, sizeof(proc.config.hostname)-1);
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
		log_error("core", __FILE__, __LINE__, 1, "unknown configuration directive '%s'", var);
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
	char basepath[255];
#ifdef WIN32
	char slash='\\';
#else
	char slash='/';
#endif
	char *ptemp;

	/* define default values */
	gethostname(proc.config.hostname, sizeof(proc.config.hostname)-1);
	snprintf(proc.config.uid, sizeof(proc.config.uid)-1, "%s", DEFAULT_SERVER_USERNAME);
	snprintf(proc.config.langcode, sizeof(proc.config.langcode)-1, "%s", DEFAULT_SERVER_LANGUAGE);

	/* try to find our way into the program's BIN dir */
	fixslashes(proc.program_name);
	ptemp=proc.program_name;
	if (*ptemp=='\"') ptemp++;
	snprintf(proc.config.dir_bin, sizeof(proc.config.dir_bin)-1, "%s", ptemp);
	if ((ptemp=strrchr(proc.config.dir_bin, slash))!=NULL) {
		*ptemp='\0';
	} else {
		/* no slash means no path.  assume we're already in BIN */
#ifdef WIN32
		GetCurrentDirectory(sizeof(proc.config.dir_bin)-1, proc.config.dir_bin);
#else
		if (getcwd(proc.config.dir_bin, sizeof(proc.config.dir_bin)-1)==NULL) return -1;
#endif
	}
	/* either way, change to BIN */
	if (chdir(proc.config.dir_bin)!=0) {
		log_error("core", __FILE__, __LINE__, 1, "can't chdir(\"%s\")", proc.config.dir_bin);
		return -1;
	}
	memset(basepath, 0, sizeof(basepath));
	snprintf(basepath, sizeof(basepath)-1, "%s", proc.config.dir_bin);
	if ((ptemp=strrchr(basepath, slash))!=NULL) *ptemp='\0';
	snprintf(proc.config.dir_bin, sizeof(proc.config.dir_bin)-1, "%s/bin", basepath);
	snprintf(proc.config.dir_etc, sizeof(proc.config.dir_etc)-1, "%s/etc", basepath);
	snprintf(proc.config.dir_lib, sizeof(proc.config.dir_lib)-1, "%s/lib", basepath);
	snprintf(proc.config.dir_var, sizeof(proc.config.dir_var)-1, "%s/var", basepath);
#ifdef HAVE_SQLITE3
	strncpy(proc.config.sql_type, "SQLITE", sizeof(proc.config.sql_type)-1);
#endif
	proc.config.loglevel=1;
	proc.config.umask=0007;

	config_read("global", conf_callback);

	if (!strlen(proc.config.dir_var_backup)) {
		snprintf(proc.config.dir_var_backup,  sizeof(proc.config.dir_var_backup)-1,  "%s/backup", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_cgi)) {
		snprintf(proc.config.dir_var_cgi,     sizeof(proc.config.dir_var_cgi)-1,     "%s/share/cgi-bin", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_db)) {
		snprintf(proc.config.dir_var_db,      sizeof(proc.config.dir_var_db)-1,      "%s/db", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_domains)) {
		snprintf(proc.config.dir_var_domains, sizeof(proc.config.dir_var_domains)-1, "%s/domains", proc.config.dir_var);
	}
	if (!strlen(proc.config.dir_var_htdocs)) {
		snprintf(proc.config.dir_var_htdocs,  sizeof(proc.config.dir_var_htdocs)-1,  "%s/share/htdocs", proc.config.dir_var);
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
