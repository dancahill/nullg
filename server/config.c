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

int config_read(CONFIG *config)
{
	FILE *fp=NULL;
	char line[512];
	struct stat sb;
	char *pVar;
	char *pVal;
	short int founddir=0;
	int i;
#ifdef WIN32
	char slash='\\';
#else
	char slash='/';
#endif

	/* define default values */
	snprintf(config->server_username,      sizeof(config->server_username)-1,      "%s", DEFAULT_SERVER_USERNAME);
	pVal=proc.program_name;
	if (*pVal=='\"') pVal++;
#ifdef WIN32
	snprintf(config->server_dir_base, sizeof(config->server_dir_base)-1, "%s", pVal);
#else
	if (getcwd(config->server_dir_base, sizeof(config->server_dir_base)-1)==NULL) return -1;
	strcat(config->server_dir_base, "/");
#endif
	if (strrchr(config->server_dir_base, slash)!=NULL) {
		pVal=strrchr(config->server_dir_base, slash);
		*pVal='\0';
		chdir(config->server_dir_base);
		if (strrchr(config->server_dir_base, slash)!=NULL) {
			pVal=strrchr(config->server_dir_base, slash);
			*pVal='\0';
			founddir=1;
		}
	}
	if (!founddir) {
		snprintf(config->server_dir_base, sizeof(config->server_dir_base)-1, "%s", DEFAULT_BASE_DIR);
	}
	snprintf(config->server_dir_bin, sizeof(config->server_dir_bin)-1, "%s/bin", config->server_dir_base);
	snprintf(config->server_dir_cgi, sizeof(config->server_dir_cgi)-1, "%s/cgi-bin", config->server_dir_base);
	snprintf(config->server_dir_etc, sizeof(config->server_dir_etc)-1, "%s/etc", config->server_dir_base);
	snprintf(config->server_dir_lib, sizeof(config->server_dir_lib)-1, "%s/lib", config->server_dir_base);
	snprintf(config->server_dir_var, sizeof(config->server_dir_var)-1, "%s/var", config->server_dir_base);
#ifdef HAVE_SQLITE
	strncpy(config->sql_type, "SQLITE", sizeof(config->sql_type)-1);
#endif
	config->server_loglevel=1;
	config->http_port=HTTP_PORT;
	config->http_maxconn=50;
	config->http_maxidle=120;
	config->http_maxpostsize=33554432; /* 32 MB limit for POST request sizes */
	config->pop3_port=POP3_PORT;
	config->pop3_maxconn=50;
	config->pop3_maxidle=120;
	config->smtp_port=SMTP_PORT;
	config->smtp_maxconn=50;
	config->smtp_maxidle=120;
#ifdef HAVE_LIBSSL
	config->http_port_ssl=HTTPS_PORT;
	config->pop3_port_ssl=POP3S_PORT;
	config->smtp_port_ssl=SMTPS_PORT;
#endif
	config->sql_maxconn=config->http_maxconn*2;
	/* try to open the config file */
	/* try the current directory first, then ../etc/, then the default etc/ */
	if (fp==NULL) {
		snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s.cfg", SERVER_BASENAME);
		fp=fopen(proc.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "../etc/%s.cfg", SERVER_BASENAME);
		fixslashes(proc.config_filename);
		fp=fopen(proc.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(proc.config_filename, sizeof(proc.config_filename)-1, "%s/%s.cfg", config->server_dir_etc, SERVER_BASENAME);
		fixslashes(proc.config_filename);
		fp=fopen(proc.config_filename, "r");
	}
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		if (stat(config->server_dir_etc, &sb)!=0) {
			log_error(NULL, __FILE__, __LINE__, 0, CONFIG_NODIR, config->server_dir_etc);
			return -1;
		};
		printf("\r\n%s", CONFIG_MAKE);
		log_error(NULL, __FILE__, __LINE__, 1, "%s", CONFIG_MAKE);
		config_write(config);
		printf("done.\n");
		goto sanity_check;
	}
	/* else if config file does exist, read it */
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if (isalpha(line[0])) {
			pVar=line;
			pVal=line;
			while ((*pVal!='=')&&((char *)&pVal+1!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			while (*pVal=='"') pVal++;
			if (pVal[strlen(pVal)-1]=='"') pVal[strlen(pVal)-1]='\0';
			if (strcmp(pVar, "SERVER.DIR.BASE")==0) {
				strncpy(config->server_dir_base, pVal, sizeof(config->server_dir_base)-1);
			} else if (strcmp(pVar, "SERVER.DIR.BIN")==0) {
				strncpy(config->server_dir_bin, pVal, sizeof(config->server_dir_bin)-1);
			} else if (strcmp(pVar, "SERVER.DIR.CGI")==0) {
				strncpy(config->server_dir_cgi, pVal, sizeof(config->server_dir_cgi)-1);
			} else if (strcmp(pVar, "SERVER.DIR.ETC")==0) {
				strncpy(config->server_dir_etc, pVal, sizeof(config->server_dir_etc)-1);
			} else if (strcmp(pVar, "SERVER.DIR.LIB")==0) {
				strncpy(config->server_dir_lib, pVal, sizeof(config->server_dir_lib)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR")==0) {
				strncpy(config->server_dir_var, pVal, sizeof(config->server_dir_var)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.BACKUP")==0) {
				strncpy(config->server_dir_var_backup, pVal, sizeof(config->server_dir_var_backup)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.DB")==0) {
				strncpy(config->server_dir_var_db, pVal, sizeof(config->server_dir_var_db)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.DOMAINS")==0) {
				strncpy(config->server_dir_var_domains, pVal, sizeof(config->server_dir_var_domains)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.HTDOCS")==0) {
				strncpy(config->server_dir_var_htdocs, pVal, sizeof(config->server_dir_var_htdocs)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.LOG")==0) {
				strncpy(config->server_dir_var_log, pVal, sizeof(config->server_dir_var_log)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.SPOOL")==0) {
				strncpy(config->server_dir_var_spool, pVal, sizeof(config->server_dir_var_spool)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.TMP")==0) {
				strncpy(config->server_dir_var_tmp, pVal, sizeof(config->server_dir_var_tmp)-1);
			} else if (strcmp(pVar, "SERVER.LOGLEVEL")==0) {
				config->server_loglevel=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.USERNAME")==0) {
				strncpy(config->server_username, pVal, sizeof(config->server_username)-1);
			} else if (strcmp(pVar, "HTTP.HOSTNAME")==0) {
				strncpy(config->http_hostname, pVal, sizeof(config->http_hostname)-1);
			} else if (strcmp(pVar, "HTTP.PORT")==0) {
				config->http_port=atoi(pVal);
#ifdef HAVE_LIBSSL
			} else if (strcmp(pVar, "HTTP.SSLPORT")==0) {
				config->http_port_ssl=atoi(pVal);
			} else if (strcmp(pVar, "POP3.SSLPORT")==0) {
				config->pop3_port_ssl=atoi(pVal);
			} else if (strcmp(pVar, "SMTP.SSLPORT")==0) {
				config->smtp_port_ssl=atoi(pVal);
#endif
			} else if (strcmp(pVar, "HTTP.MAXCONN")==0) {
				config->http_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "HTTP.MAXIDLE")==0) {
				config->http_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "HTTP.MAXPOSTSIZE")==0) {
				config->http_maxpostsize=atoi(pVal);
			} else if (strcmp(pVar, "POP3.HOSTNAME")==0) {
				strncpy(config->pop3_hostname, pVal, sizeof(config->pop3_hostname)-1);
			} else if (strcmp(pVar, "POP3.PORT")==0) {
				config->pop3_port=atoi(pVal);
			} else if (strcmp(pVar, "POP3.MAXCONN")==0) {
				config->pop3_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "POP3.MAXIDLE")==0) {
				config->pop3_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "SMTP.HOSTNAME")==0) {
				strncpy(config->smtp_hostname, pVal, sizeof(config->smtp_hostname)-1);
			} else if (strcmp(pVar, "SMTP.RELAYHOST")==0) {
				strncpy(config->smtp_relayhost, pVal, sizeof(config->smtp_relayhost)-1);
			} else if (strcmp(pVar, "SMTP.PORT")==0) {
				config->smtp_port=atoi(pVal);
			} else if (strcmp(pVar, "SMTP.MAXCONN")==0) {
				config->smtp_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "SMTP.MAXIDLE")==0) {
				config->smtp_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "SQL.TYPE")==0) {
				strncpy(config->sql_type, pVal, sizeof(config->sql_type)-1);
			} else if (strcmp(pVar, "SQL.HOSTNAME")==0) {
				strncpy(config->sql_hostname, pVal, sizeof(config->sql_hostname)-1);
			} else if (strcmp(pVar, "SQL.PORT")==0) {
				config->sql_port=atoi(pVal);
			} else if (strcmp(pVar, "SQL.DBNAME")==0) {
				strncpy(config->sql_dbname, pVal, sizeof(config->sql_dbname)-1);
			} else if (strcmp(pVar, "SQL.USERNAME")==0) {
				strncpy(config->sql_username, pVal, sizeof(config->sql_username)-1);
			} else if (strcmp(pVar, "SQL.PASSWORD")==0) {
				strncpy(config->sql_password, pVal, sizeof(config->sql_password)-1);
			} else if (strcmp(pVar, "SQL.ODBC_DSN")==0) {
				strncpy(config->sql_odbc_dsn, pVal, sizeof(config->sql_odbc_dsn)-1);
			} else if (strcmp(pVar, "UTIL.SCANFILE")==0) {
				strncpy(config->util_scanfile, pVal, sizeof(config->util_scanfile)-1);
			} else if (strcmp(pVar, "UTIL.SCANMAIL")==0) {
				strncpy(config->util_scanmail, pVal, sizeof(config->util_scanmail)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
sanity_check:
	if (!strlen(config->server_dir_var_backup)) {
		snprintf(config->server_dir_var_backup, sizeof(config->server_dir_var_backup)-1, "%s/backup", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_db)) {
		snprintf(config->server_dir_var_db,     sizeof(config->server_dir_var_db)-1,     "%s/db", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_domains)) {
		snprintf(config->server_dir_var_domains,  sizeof(config->server_dir_var_domains)-1,  "%s/domains", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_htdocs)) {
		snprintf(config->server_dir_var_htdocs, sizeof(config->server_dir_var_htdocs)-1, "%s/htdocs", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_log)) {
		snprintf(config->server_dir_var_log,    sizeof(config->server_dir_var_log)-1,    "%s/log", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_spool)) {
		snprintf(config->server_dir_var_spool,   sizeof(config->server_dir_var_spool)-1,   "%s/spool", config->server_dir_var);
	}
	if (!strlen(config->server_dir_var_tmp)) {
		snprintf(config->server_dir_var_tmp,    sizeof(config->server_dir_var_tmp)-1,    "%s/tmp", config->server_dir_var);
	}
	fixslashes(config->server_dir_base);
	fixslashes(config->server_dir_bin);
	fixslashes(config->server_dir_cgi);
	fixslashes(config->server_dir_etc);
	fixslashes(config->server_dir_lib);
	fixslashes(config->server_dir_var);
	fixslashes(config->server_dir_var_backup);
	fixslashes(config->server_dir_var_db);
	fixslashes(config->server_dir_var_domains);
	fixslashes(config->server_dir_var_htdocs);
	fixslashes(config->server_dir_var_log);
	fixslashes(config->server_dir_var_spool);
	fixslashes(config->server_dir_var_tmp);
	if (config->http_maxconn==0) config->http_maxconn=50;
	if (config->http_maxidle==0) config->http_maxidle=120;
	if (config->http_maxconn<5) config->http_maxconn=5;
	if (config->http_maxconn>1000) config->http_maxconn=1000;
	if (config->http_maxidle<15) config->http_maxidle=15;
	if (config->http_maxpostsize<1048576) config->http_maxpostsize=1048576;
	if (strlen(config->http_hostname)==0) strncpy(config->http_hostname, "INADDR_ANY", sizeof(config->http_hostname)-1);
	if (config->pop3_maxconn==0) config->pop3_maxconn=50;
	if (config->pop3_maxidle==0) config->pop3_maxidle=120;
	if (config->pop3_maxconn<5) config->pop3_maxconn=5;
	if (config->pop3_maxconn>1000) config->pop3_maxconn=1000;
	if (config->pop3_maxidle<15) config->pop3_maxidle=15;
	if (strlen(config->pop3_hostname)==0) strncpy(config->pop3_hostname, "INADDR_ANY", sizeof(config->pop3_hostname)-1);
	if (config->smtp_maxconn==0) config->smtp_maxconn=50;
	if (config->smtp_maxidle==0) config->smtp_maxidle=120;
	if (config->smtp_maxconn<5) config->smtp_maxconn=5;
	if (config->smtp_maxconn>1000) config->smtp_maxconn=1000;
	if (config->smtp_maxidle<15) config->smtp_maxidle=15;
	if (strlen(config->smtp_hostname)==0) strncpy(config->smtp_hostname, "INADDR_ANY", sizeof(config->smtp_hostname)-1);
	config->sql_maxconn=config->http_maxconn*2;
	return 0;
}

int config_write(CONFIG *config)
{
	FILE *fp=NULL;

	if (config->http_maxconn<1) config->http_maxconn=50;
	if (config->http_maxidle<1) config->http_maxidle=120;
	if (config->http_maxconn<5) config->http_maxconn=5;
	if (config->http_maxidle<15) config->http_maxidle=15;
	if (config->http_maxconn>1000) config->http_maxconn=1000;
	config->http_maxpostsize=33554432; /* 32 MB limit for POST request sizes */
	if (strlen(config->http_hostname)==0) strncpy(config->http_hostname, "INADDR_ANY", sizeof(config->http_hostname)-1);
	if (config->pop3_maxconn<1) config->pop3_maxconn=50;
	if (config->pop3_maxidle<1) config->pop3_maxidle=120;
	if (config->pop3_maxconn<5) config->pop3_maxconn=5;
	if (config->pop3_maxidle<15) config->pop3_maxidle=15;
	if (config->pop3_maxconn>1000) config->pop3_maxconn=1000;
	if (strlen(config->pop3_hostname)==0) strncpy(config->pop3_hostname, "INADDR_ANY", sizeof(config->pop3_hostname)-1);
	if (config->smtp_maxconn<1) config->smtp_maxconn=50;
	if (config->smtp_maxidle<1) config->smtp_maxidle=120;
	if (config->smtp_maxconn<5) config->smtp_maxconn=5;
	if (config->smtp_maxidle<15) config->smtp_maxidle=15;
	if (config->smtp_maxconn>1000) config->smtp_maxconn=1000;
	if (strlen(config->smtp_hostname)==0) strncpy(config->smtp_hostname, "INADDR_ANY", sizeof(config->smtp_hostname)-1);
	config->sql_maxconn=config->http_maxconn*2;
	fixslashes(proc.config_filename);
	if ((fp=fopen(proc.config_filename, "w"))==NULL) return -1;
	fprintf(fp, "# %s\n\n", CONFIG_HEAD);
	fprintf(fp, "SERVER.DIR.BASE        = \"%s\"\n", config->server_dir_base);
	fprintf(fp, "SERVER.DIR.BIN         = \"%s\"\n", config->server_dir_bin);
	fprintf(fp, "SERVER.DIR.CGI         = \"%s\"\n", config->server_dir_cgi);
	fprintf(fp, "SERVER.DIR.ETC         = \"%s\"\n", config->server_dir_etc);
	fprintf(fp, "SERVER.DIR.LIB         = \"%s\"\n", config->server_dir_lib);
	fprintf(fp, "SERVER.DIR.VAR         = \"%s\"\n", config->server_dir_var);
	fprintf(fp, "SERVER.DIR.VAR.BACKUP  = \"%s\"\n", config->server_dir_var_backup);
	fprintf(fp, "SERVER.DIR.VAR.DB      = \"%s\"\n", config->server_dir_var_db);
	fprintf(fp, "SERVER.DIR.VAR.DOMAINS = \"%s\"\n", config->server_dir_var_domains);
	fprintf(fp, "SERVER.DIR.VAR.HTDOCS  = \"%s\"\n", config->server_dir_var_htdocs);
	fprintf(fp, "SERVER.DIR.VAR.LOG     = \"%s\"\n", config->server_dir_var_log);
	fprintf(fp, "SERVER.DIR.VAR.SPOOL   = \"%s\"\n", config->server_dir_var_spool);
	fprintf(fp, "SERVER.DIR.VAR.TMP     = \"%s\"\n", config->server_dir_var_tmp);
	fprintf(fp, "SERVER.LOGLEVEL        = \"%d\"\n", config->server_loglevel);
	fprintf(fp, "SERVER.USERNAME        = \"%s\"\n", config->server_username);
	fprintf(fp, "HTTP.HOSTNAME          = \"%s\"\n", config->http_hostname);
	fprintf(fp, "HTTP.PORT              = \"%d\"\n", config->http_port);
	fprintf(fp, "HTTP.SSLPORT           = \"%d\"\n", config->http_port_ssl);
	fprintf(fp, "HTTP.MAXCONN           = \"%d\"\n", config->http_maxconn);
	fprintf(fp, "HTTP.MAXIDLE           = \"%d\"\n", config->http_maxidle);
	fprintf(fp, "HTTP.MAXPOSTSIZE       = \"%d\"\n", config->http_maxpostsize);
	fprintf(fp, "POP3.HOSTNAME          = \"%s\"\n", config->pop3_hostname);
	fprintf(fp, "POP3.PORT              = \"%d\"\n", config->pop3_port);
	fprintf(fp, "POP3.MAXCONN           = \"%d\"\n", config->pop3_maxconn);
	fprintf(fp, "POP3.MAXIDLE           = \"%d\"\n", config->pop3_maxidle);
	fprintf(fp, "SMTP.HOSTNAME          = \"%s\"\n", config->smtp_hostname);
//	if (strlen(config->smtp_relayhost)) {
		fprintf(fp, "SMTP.RELAYHOST         = \"%s\"\n", config->smtp_relayhost);
//	}
	fprintf(fp, "SMTP.PORT              = \"%d\"\n", config->smtp_port);
	fprintf(fp, "SMTP.MAXCONN           = \"%d\"\n", config->smtp_maxconn);
	fprintf(fp, "SMTP.MAXIDLE           = \"%d\"\n", config->smtp_maxidle);
	fprintf(fp, "SQL.TYPE               = \"%s\"\n", config->sql_type);
	fprintf(fp, "SQL.HOSTNAME           = \"%s\"\n", config->sql_hostname);
	fprintf(fp, "SQL.PORT               = \"%d\"\n", config->sql_port);
	fprintf(fp, "SQL.DBNAME             = \"%s\"\n", config->sql_dbname);
	fprintf(fp, "SQL.USERNAME           = \"%s\"\n", config->sql_username);
	fprintf(fp, "SQL.PASSWORD           = \"%s\"\n", config->sql_password);
#ifdef WIN32
	fprintf(fp, "SQL.ODBC_DSN           = \"%s\"\n", config->sql_odbc_dsn);
#endif
	fprintf(fp, "UTIL.SCANFILE          = \"%s\"\n", config->util_scanfile);
	fprintf(fp, "UTIL.SCANMAIL          = \"%s\"\n", config->util_scanmail);
	fclose(fp);
	return 0;
}
