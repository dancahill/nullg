/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include <windows.h>

#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <process.h>
#include <shellapi.h>
#include <signal.h>
#include <windowsx.h>
#include <io.h>
#include <direct.h>

#include "resource.h"

#define DEFAULT_SERVER_USERNAME		"nullgs"

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp

typedef struct {
	char      server_dir_base[255];
	char      server_dir_bin[255];
	char      server_dir_cgi[255];
	char      server_dir_etc[255];
	char      server_dir_lib[255];
	char      server_dir_var[255];
	char      server_dir_var_backup[255];
	char      server_dir_var_db[255];
	char      server_dir_var_files[255];
	char      server_dir_var_htdocs[255];
	char      server_dir_var_log[255];
	char      server_dir_var_mail[255];
	char      server_dir_var_tmp[255];
	short int server_loglevel;
	char      server_username[33];
	char      http_hostname[128];
	short int http_port;
	short int http_maxconn;
	short int http_maxidle;
	char      pop3_hostname[128];
	short int pop3_port;
	short int pop3_maxconn;
	short int pop3_maxidle;
	char      smtp_hostname[128];
	char      smtp_relayhost[128];
	short int smtp_port;
	short int smtp_maxconn;
	short int smtp_maxidle;
	char      sql_type[32];
	char      sql_username[32];
	char      sql_password[32];
	char      sql_dbname[32];
	char      sql_odbc_dsn[200];
	char      sql_hostname[128];
	short int sql_port;
	char      util_virusscan[255];
} CONFIG;
CONFIG cfg;
HINSTANCE _hInstance;
int choice=0;
char config_filename[255];

BOOL CALLBACK DbutilDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

char *strcatf(char *dest, const char *format, ...)
{
	char catbuffer[1024];
	va_list ap;

	memset(catbuffer, 0, sizeof(catbuffer));
	va_start(ap, format);
	vsnprintf(catbuffer, sizeof(catbuffer)-1, format, ap);
	va_end(ap);
	strcat(dest, catbuffer);
	return dest;
}

void fixslashes(char *string)
{
 	while (*string) {
#ifdef WIN32
 		if (*string=='/') *string='\\';
#else
 		if (*string=='\\') *string='/';
#endif
		string++;
	}
}

int config_read(CONFIG *config)
{
	FILE *fp=NULL;
	char program_name[512];
	char line[512];
	char *pVar;
	char *pVal;
	short int founddir=0;
	int i;
	char slash='\\';

	snprintf(program_name, sizeof(program_name)-1, "%s", GetCommandLine());
	pVal=program_name;
	if (*pVal=='\"') pVal++;
	snprintf(config->server_dir_base, sizeof(config->server_dir_base)-1, "%s", pVal);
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
	/* define default values */
	snprintf(config->server_dir_bin,        sizeof(config->server_dir_bin)-1,        "%s/bin", config->server_dir_base);
	snprintf(config->server_dir_etc,        sizeof(config->server_dir_etc)-1,        "%s/etc", config->server_dir_base);
	snprintf(config->server_dir_lib,        sizeof(config->server_dir_lib)-1,        "%s/lib", config->server_dir_base);
	snprintf(config->server_dir_var,        sizeof(config->server_dir_var)-1,        "%s/var", config->server_dir_base);
	snprintf(config->server_dir_var_backup, sizeof(config->server_dir_var_backup)-1, "%s/backup", config->server_dir_var);
	snprintf(config->server_dir_var_db,     sizeof(config->server_dir_var_db)-1,     "%s/db", config->server_dir_var);
	snprintf(config->server_dir_cgi,        sizeof(config->server_dir_cgi)-1,        "%s/share/cgi-bin", config->server_dir_var);
	snprintf(config->server_dir_var_files,  sizeof(config->server_dir_var_files)-1,  "%s/files", config->server_dir_var);
	snprintf(config->server_dir_var_htdocs, sizeof(config->server_dir_var_htdocs)-1, "%s/share/htdocs", config->server_dir_var);
	snprintf(config->server_dir_var_log,    sizeof(config->server_dir_var_log)-1,    "%s/log", config->server_dir_var);
	snprintf(config->server_dir_var_mail,   sizeof(config->server_dir_var_mail)-1,   "%s/mail", config->server_dir_var);
	snprintf(config->server_dir_var_tmp,    sizeof(config->server_dir_var_tmp)-1,    "%s/tmp", config->server_dir_var);
	fixslashes(config->server_dir_base);
	fixslashes(config->server_dir_bin);
	fixslashes(config->server_dir_cgi);
	fixslashes(config->server_dir_etc);
	fixslashes(config->server_dir_lib);
	fixslashes(config->server_dir_var);
	fixslashes(config->server_dir_var_backup);
	fixslashes(config->server_dir_var_db);
	fixslashes(config->server_dir_var_files);
	fixslashes(config->server_dir_var_htdocs);
	fixslashes(config->server_dir_var_log);
	fixslashes(config->server_dir_var_mail);
	fixslashes(config->server_dir_var_tmp);
#ifdef HAVE_SQLITE3
	strncpy(config->sql_type, "SQLITE", sizeof(config->sql_type)-1);
#endif
	config->server_loglevel=1;
	snprintf(config->server_username, sizeof(config->server_username)-1, "%s", DEFAULT_SERVER_USERNAME);
	config->http_port=4110;
	config->http_maxconn=50;
	config->http_maxidle=120;
	config->pop3_port=110;
	config->pop3_maxconn=50;
	config->pop3_maxidle=120;
	config->smtp_port=25;
	config->smtp_maxconn=50;
	config->smtp_maxidle=120;
	/* try to open the config file */
	/* try the current directory first, then ../etc/, then the default etc/ */
	if (fp==NULL) {
		snprintf(config_filename, sizeof(config_filename)-1, "nullgs.conf");
		fp=fopen(config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config_filename, sizeof(config_filename)-1, "../etc/nullgs.conf");
		fixslashes(config_filename);
		fp=fopen(config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config_filename, sizeof(config_filename)-1, "%s/nullgs.conf", config->server_dir_etc);
		fixslashes(config_filename);
		fp=fopen(config_filename, "r");
	}
	/* if config file couldn't be opened, abort */
	if (fp==NULL) {
		return 0;
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
			} else if (strcmp(pVar, "SERVER.DIR.VAR.FILES")==0) {
				strncpy(config->server_dir_var_files, pVal, sizeof(config->server_dir_var_files)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.HTDOCS")==0) {
				strncpy(config->server_dir_var_htdocs, pVal, sizeof(config->server_dir_var_htdocs)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.LOG")==0) {
				strncpy(config->server_dir_var_log, pVal, sizeof(config->server_dir_var_log)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR.MAIL")==0) {
				strncpy(config->server_dir_var_mail, pVal, sizeof(config->server_dir_var_mail)-1);
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
			} else if (strcmp(pVar, "HTTP.MAXCONN")==0) {
				config->http_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "HTTP.MAXIDLE")==0) {
				config->http_maxidle=atoi(pVal);
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
			} else if (strcmp(pVar, "UTIL.VIRUSSCAN")==0) {
				strncpy(config->util_virusscan, pVal, sizeof(config->util_virusscan)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	if (config->http_maxconn==0) config->http_maxconn=50;
	if (config->http_maxidle==0) config->http_maxidle=120;
	if (config->http_maxconn<5) config->http_maxconn=5;
	if (config->http_maxconn>1000) config->http_maxconn=1000;
	if (config->http_maxidle<15) config->http_maxidle=15;
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
	fixslashes(config_filename);
	if ((fp=fopen(config_filename, "w"))==NULL) return -1;
	fprintf(fp, "# This file contains system settings for NullLogic GroupServer.\n\n");
	fprintf(fp, "SERVER.DIR.BASE       = \"%s\"\n", config->server_dir_base);
	fprintf(fp, "SERVER.DIR.BIN        = \"%s\"\n", config->server_dir_bin);
	fprintf(fp, "SERVER.DIR.CGI        = \"%s\"\n", config->server_dir_cgi);
	fprintf(fp, "SERVER.DIR.ETC        = \"%s\"\n", config->server_dir_etc);
	fprintf(fp, "SERVER.DIR.LIB        = \"%s\"\n", config->server_dir_lib);
	fprintf(fp, "SERVER.DIR.VAR        = \"%s\"\n", config->server_dir_var);
	fprintf(fp, "SERVER.DIR.VAR.BACKUP = \"%s\"\n", config->server_dir_var_backup);
	fprintf(fp, "SERVER.DIR.VAR.DB     = \"%s\"\n", config->server_dir_var_db);
	fprintf(fp, "SERVER.DIR.VAR.FILES  = \"%s\"\n", config->server_dir_var_files);
	fprintf(fp, "SERVER.DIR.VAR.HTDOCS = \"%s\"\n", config->server_dir_var_htdocs);
	fprintf(fp, "SERVER.DIR.VAR.LOG    = \"%s\"\n", config->server_dir_var_log);
	fprintf(fp, "SERVER.DIR.VAR.MAIL   = \"%s\"\n", config->server_dir_var_mail);
	fprintf(fp, "SERVER.DIR.VAR.TMP    = \"%s\"\n", config->server_dir_var_tmp);
	fprintf(fp, "SERVER.LOGLEVEL       = \"%d\"\n", config->server_loglevel);
	fprintf(fp, "SERVER.USERNAME       = \"%s\"\n", config->server_username);
	fprintf(fp, "HTTP.HOSTNAME         = \"%s\"\n", config->http_hostname);
	fprintf(fp, "HTTP.PORT             = \"%d\"\n", config->http_port);
	fprintf(fp, "HTTP.MAXCONN          = \"%d\"\n", config->http_maxconn);
	fprintf(fp, "HTTP.MAXIDLE          = \"%d\"\n", config->http_maxidle);
	fprintf(fp, "POP3.HOSTNAME         = \"%s\"\n", config->pop3_hostname);
	fprintf(fp, "POP3.PORT             = \"%d\"\n", config->pop3_port);
	fprintf(fp, "POP3.MAXCONN          = \"%d\"\n", config->pop3_maxconn);
	fprintf(fp, "POP3.MAXIDLE          = \"%d\"\n", config->pop3_maxidle);
	fprintf(fp, "SMTP.HOSTNAME         = \"%s\"\n", config->smtp_hostname);
	if (strlen(config->smtp_relayhost)) {
		fprintf(fp, "SMTP.RELAYHOST        = \"%s\"\n", config->smtp_relayhost);
	}
	fprintf(fp, "SMTP.PORT             = \"%d\"\n", config->smtp_port);
	fprintf(fp, "SMTP.MAXCONN          = \"%d\"\n", config->smtp_maxconn);
	fprintf(fp, "SMTP.MAXIDLE          = \"%d\"\n", config->smtp_maxidle);
	fprintf(fp, "SQL.TYPE              = \"%s\"\n", config->sql_type);
	fprintf(fp, "SQL.HOSTNAME          = \"%s\"\n", config->sql_hostname);
	fprintf(fp, "SQL.PORT              = \"%d\"\n", config->sql_port);
	fprintf(fp, "SQL.DBNAME            = \"%s\"\n", config->sql_dbname);
	fprintf(fp, "SQL.USERNAME          = \"%s\"\n", config->sql_username);
	fprintf(fp, "SQL.PASSWORD          = \"%s\"\n", config->sql_password);
#ifdef WIN32
	fprintf(fp, "SQL.ODBC_DSN          = \"%s\"\n", config->sql_odbc_dsn);
#endif
	fprintf(fp, "UTIL.VIRUSSCAN        = \"%s\"\n", config->util_virusscan);
	fclose(fp);
	MessageBox(NULL, "Configuration file saved", "", MB_OK);
	return 0;
}

BOOL CALLBACK ConfigDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hZAlist=GetDlgItem(hDlg, IDC_COMBO7);
	HWND hZBlist=GetDlgItem(hDlg, IDC_COMBO10);
	char buffer[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		ComboBox_ResetContent(hZAlist);
		ComboBox_AddString(hZAlist, "0");
		ComboBox_AddString(hZAlist, "1");
		ComboBox_AddString(hZAlist, "2");
		ComboBox_AddString(hZAlist, "3");
		ComboBox_AddString(hZAlist, "4");
		ComboBox_SetCurSel(hZAlist, cfg.server_loglevel);
//		EnableWindow(hZAlist, 1);
		ComboBox_ResetContent(hZBlist);
		ComboBox_AddString(hZBlist, "MYSQL");
		ComboBox_AddString(hZBlist, "ODBC");
		ComboBox_AddString(hZBlist, "PGSQL");
		ComboBox_AddString(hZBlist, "SQLITE");
		if (strcasecmp(cfg.sql_type, "MYSQL")==0) {
			ComboBox_SetCurSel(hZBlist, 0);
		} else if (strcasecmp(cfg.sql_type, "ODBC")==0) {
			ComboBox_SetCurSel(hZBlist, 1);
		} else if (strcasecmp(cfg.sql_type, "PGSQL")==0) {
			ComboBox_SetCurSel(hZBlist, 2);
		} else if (strcasecmp(cfg.sql_type, "SQLITE")==0) {
			ComboBox_SetCurSel(hZBlist, 3);
		}
//		EnableWindow(hZBlist, 1);
		SetDlgItemText(hDlg, IDC_EDIT1,  cfg.server_dir_base);
		SetDlgItemText(hDlg, IDC_EDIT2,  cfg.server_dir_bin);
		SetDlgItemText(hDlg, IDC_EDIT3,  cfg.server_dir_cgi);
		SetDlgItemText(hDlg, IDC_EDIT4,  cfg.server_dir_etc);
		SetDlgItemText(hDlg, IDC_EDIT5,  cfg.server_dir_lib);
		SetDlgItemText(hDlg, IDC_EDIT6,  cfg.server_dir_var);
//		SetDlgItemText(hDlg, IDC_COMBO7, cfg.server_loglevel);
		SetDlgItemText(hDlg, IDC_EDIT8,  cfg.http_hostname);
		snprintf(buffer, sizeof(buffer)-1, "%d", cfg.http_port);
		SetDlgItemText(hDlg, IDC_EDIT9,  buffer);
//		SetDlgItemText(hDlg, IDC_COMBO10, cfg.sql_type);
		SetDlgItemText(hDlg, IDC_EDIT11,  cfg.sql_hostname);
		snprintf(buffer, sizeof(buffer)-1, "%d", cfg.sql_port);
		SetDlgItemText(hDlg, IDC_EDIT12,  buffer);
		SetDlgItemText(hDlg, IDC_EDIT13,  cfg.sql_dbname);
		SetDlgItemText(hDlg, IDC_EDIT14,  cfg.sql_username);
		SetDlgItemText(hDlg, IDC_EDIT15,  cfg.sql_password);
		SetDlgItemText(hDlg, IDC_EDIT16,  cfg.sql_odbc_dsn);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
//		case IDCANCEL:
//			EndDialog(hDlg, 0);
//			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT1,   cfg.server_dir_base,   sizeof(cfg.server_dir_base)-1);
			GetDlgItemText(hDlg, IDC_EDIT2,   cfg.server_dir_bin ,   sizeof(cfg.server_dir_bin)-1);
			GetDlgItemText(hDlg, IDC_EDIT3,   cfg.server_dir_cgi,    sizeof(cfg.server_dir_cgi)-1);
			GetDlgItemText(hDlg, IDC_EDIT4,   cfg.server_dir_etc,    sizeof(cfg.server_dir_etc)-1);
			GetDlgItemText(hDlg, IDC_EDIT5,   cfg.server_dir_lib,    sizeof(cfg.server_dir_lib)-1);
			GetDlgItemText(hDlg, IDC_EDIT6,   cfg.server_dir_var,    sizeof(cfg.server_dir_var)-1);
			GetDlgItemText(hDlg, IDC_COMBO7,  buffer,                   sizeof(buffer)-1);
			cfg.server_loglevel=atoi(buffer);
			GetDlgItemText(hDlg, IDC_EDIT8,   cfg.http_hostname,     sizeof(cfg.http_hostname)-1);
			GetDlgItemText(hDlg, IDC_EDIT9,   buffer,                   sizeof(buffer)-1);
			cfg.http_port=atoi(buffer);
			GetDlgItemText(hDlg, IDC_COMBO10, cfg.sql_type,          sizeof(cfg.sql_type)-1);
			GetDlgItemText(hDlg, IDC_EDIT11,  cfg.sql_hostname,      sizeof(cfg.sql_hostname)-1);
			GetDlgItemText(hDlg, IDC_EDIT12,  buffer,                   sizeof(buffer)-1);
			cfg.sql_port=atoi(buffer);
			GetDlgItemText(hDlg, IDC_EDIT13,  cfg.sql_dbname,        sizeof(cfg.sql_dbname)-1);
			GetDlgItemText(hDlg, IDC_EDIT14,  cfg.sql_username,      sizeof(cfg.sql_username)-1);
			GetDlgItemText(hDlg, IDC_EDIT15,  cfg.sql_password,      sizeof(cfg.sql_password)-1);
			GetDlgItemText(hDlg, IDC_EDIT16,  cfg.sql_odbc_dsn,      sizeof(cfg.sql_odbc_dsn)-1);
			config_write(&cfg);
			return (TRUE);
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			return (TRUE);
		case IDC_BUTTON3:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		exit(0);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}

BOOL CALLBACK DbdumpDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char cmdbuffer[100];
	char buffer[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1,  cfg.sql_type);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT2, buffer, sizeof(buffer)-1);
			snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\nullgs-dbutil dump \"%s\"", buffer);
			system(cmdbuffer);
			return (TRUE);
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}
BOOL CALLBACK DbinitDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char cmdbuffer[100];
	char buffer[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1,  cfg.sql_type);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		case IDC_BUTTON1:
			if (MessageBox(0, "Initialising the database will destroy any previously stored data.  Are you sure you want to do this?", "Init confirmation", MB_YESNO)==IDYES) {
				GetDlgItemText(hDlg, IDC_EDIT2, buffer, sizeof(buffer)-1);
				snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\nullgs-dbutil init \"%s\"", buffer);
				system(cmdbuffer);
			}
			return (TRUE);
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}
BOOL CALLBACK DbrestDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char cmdbuffer[100];
	char buffer[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1,  cfg.sql_type);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT2, buffer, sizeof(buffer)-1);
			snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\nullgs-dbutil restore \"%s\"", buffer);
			system(cmdbuffer);
			return (TRUE);
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}

BOOL CALLBACK DbutilDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hZAlist=GetDlgItem(hDlg, IDC_COMBO1);
	char buffer[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1,  cfg.sql_type);
		ComboBox_ResetContent(hZAlist);
		ComboBox_AddString(hZAlist, "Init");
		ComboBox_AddString(hZAlist, "Dump");
		ComboBox_AddString(hZAlist, "Restore");
		ComboBox_SetCurSel(hZAlist, choice);
//		EnableWindow(hZAlist, 1);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
//		case IDCANCEL:
//			EndDialog(hDlg, 0);
//			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_COMBO1, buffer, sizeof(buffer)-1);
			if (strcasecmp(buffer, "INIT")==0) {
				choice=0;
				EndDialog(hDlg, 0);
				DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL_INIT), NULL, DbinitDlgProc);
			} else if (strcasecmp(buffer, "DUMP")==0) {
				choice=1;
				EndDialog(hDlg, 0);
				DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL_DUMP), NULL, DbdumpDlgProc);
			} else if (strcasecmp(buffer, "RESTORE")==0) {
				choice=2;
				EndDialog(hDlg, 0);
				DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL_REST), NULL, DbrestDlgProc);
			}
			return (TRUE);
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			return (TRUE);
		case IDC_BUTTON3:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL, ConfigDlgProc);
			return (TRUE);
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		exit(0);
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_hInstance=hInstance;
	memset((char *)&cfg, 0, sizeof(cfg));
	config_read(&cfg);
	if (strstr(lpCmdLine, "initdb")!=NULL) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
	} else {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL, ConfigDlgProc);
	}
	return 0;
}
