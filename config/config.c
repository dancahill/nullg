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

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp

typedef struct {
	char config_filename[255];
	char server_dir_base[255];
	char server_dir_bin[255];
	char server_dir_cgi[255];
	char server_dir_etc[255];
	char server_dir_lib[255];
	char server_dir_var[255];
	char server_hostname[64];
	short int server_port;
	short int server_loglevel;
	short int server_maxconn;
	short int server_maxidle;
	char sql_type[32];
	char sql_username[32];
	char sql_password[32];
	char sql_dbname[32];
	char sql_odbc_dsn[200];
	char sql_hostname[64];
	short int sql_port;
	short int sql_maxconn;
} CONFIG;
CONFIG config;
HINSTANCE _hInstance;
int choice=0;

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

int config_read()
{
	FILE *fp=NULL;
	char program_name[512];
	char line[512];
//	struct stat sb;
	char *pVar;
	char *pVal;
	short int founddir=0;
	int i;
	char slash='\\';

	/* define default values */
	snprintf(program_name, sizeof(program_name)-1, "%s", GetCommandLine());
	memset((char *)&config, 0, sizeof(config));
	pVal=program_name;
	if (*pVal=='\"') pVal++;
	snprintf(config.server_dir_base, sizeof(config.server_dir_base)-1, "%s", pVal);
	if (strrchr(config.server_dir_base, slash)!=NULL) {
		pVal=strrchr(config.server_dir_base, slash);
		*pVal='\0';
		chdir(config.server_dir_base);
		if (strrchr(config.server_dir_base, slash)!=NULL) {
			pVal=strrchr(config.server_dir_base, slash);
			*pVal='\0';
			founddir=1;
		}
	}
//	if (!founddir) {
//		snprintf(config.server_dir_base, sizeof(config.server_dir_base)-1, "%s", DEFAULT_BASE_DIR);
//	}
	snprintf(config.server_dir_bin, sizeof(config.server_dir_bin)-1, "%s/bin", config.server_dir_base);
	snprintf(config.server_dir_cgi, sizeof(config.server_dir_cgi)-1, "%s/cgi-bin", config.server_dir_base);
	snprintf(config.server_dir_etc, sizeof(config.server_dir_etc)-1, "%s/etc", config.server_dir_base);
	snprintf(config.server_dir_lib, sizeof(config.server_dir_lib)-1, "%s/lib", config.server_dir_base);
	snprintf(config.server_dir_var, sizeof(config.server_dir_var)-1, "%s/var", config.server_dir_base);
	fixslashes(config.server_dir_base);
	fixslashes(config.server_dir_bin);
	fixslashes(config.server_dir_cgi);
	fixslashes(config.server_dir_etc);
	fixslashes(config.server_dir_lib);
	fixslashes(config.server_dir_var);
	strncpy(config.sql_type, "SQLITE", sizeof(config.sql_type)-1);
	config.server_loglevel=1;
	config.server_port=4110;
	config.server_maxconn=50;
	config.server_maxidle=120;
	config.sql_maxconn=config.server_maxconn*2;
	/* try to open the config file */
	/* try the current directory first, then ../etc/, then the default etc/ */
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "groupware.cfg");
		fp=fopen(config.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "../etc/groupware.cfg");
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "r");
	}
	if (fp==NULL) {
		snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_dir_etc);
		fixslashes(config.config_filename);
		fp=fopen(config.config_filename, "r");
	}
	/* if config file couldn't be opened, try to write one */
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
			while (pVal[strlen(pVal)-1]=='"') pVal[strlen(pVal)-1]='\0';
			if (strcmp(pVar, "SERVER.DIR.BASE")==0) {
				strncpy(config.server_dir_base, pVal, sizeof(config.server_dir_base)-1);
			} else if (strcmp(pVar, "SERVER.DIR.BIN")==0) {
				strncpy(config.server_dir_bin, pVal, sizeof(config.server_dir_bin)-1);
			} else if (strcmp(pVar, "SERVER.DIR.CGI")==0) {
				strncpy(config.server_dir_cgi, pVal, sizeof(config.server_dir_cgi)-1);
			} else if (strcmp(pVar, "SERVER.DIR.ETC")==0) {
				strncpy(config.server_dir_etc, pVal, sizeof(config.server_dir_etc)-1);
			} else if (strcmp(pVar, "SERVER.DIR.LIB")==0) {
				strncpy(config.server_dir_lib, pVal, sizeof(config.server_dir_lib)-1);
			} else if (strcmp(pVar, "SERVER.DIR.VAR")==0) {
				strncpy(config.server_dir_var, pVal, sizeof(config.server_dir_var)-1);
			} else if (strcmp(pVar, "SERVER.LOGLEVEL")==0) {
				config.server_loglevel=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.HOSTNAME")==0) {
				strncpy(config.server_hostname, pVal, sizeof(config.server_hostname)-1);
			} else if (strcmp(pVar, "SERVER.MAXCONN")==0) {
				config.server_maxconn=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.PORT")==0) {
				config.server_port=atoi(pVal);
			} else if (strcmp(pVar, "SERVER.MAXIDLE")==0) {
				config.server_maxidle=atoi(pVal);
			} else if (strcmp(pVar, "SQL.TYPE")==0) {
				strncpy(config.sql_type, pVal, sizeof(config.sql_type)-1);
			} else if (strcmp(pVar, "SQL.HOSTNAME")==0) {
				strncpy(config.sql_hostname, pVal, sizeof(config.sql_hostname)-1);
			} else if (strcmp(pVar, "SQL.PORT")==0) {
				config.sql_port=atoi(pVal);
			} else if (strcmp(pVar, "SQL.DBNAME")==0) {
				strncpy(config.sql_dbname, pVal, sizeof(config.sql_dbname)-1);
			} else if (strcmp(pVar, "SQL.USERNAME")==0) {
				strncpy(config.sql_username, pVal, sizeof(config.sql_username)-1);
			} else if (strcmp(pVar, "SQL.PASSWORD")==0) {
				strncpy(config.sql_password, pVal, sizeof(config.sql_password)-1);
			} else if (strcmp(pVar, "SQL.ODBC_DSN")==0) {
				strncpy(config.sql_odbc_dsn, pVal, sizeof(config.sql_odbc_dsn)-1);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	if (config.server_maxconn==0) config.server_maxconn=50;
	if (config.server_maxidle==0) config.server_maxidle=120;
	if (config.server_maxconn<5) config.server_maxconn=5;
	if (config.server_maxconn>1000) config.server_maxconn=1000;
	if (config.server_maxidle<15) config.server_maxidle=15;
	if (strlen(config.server_hostname)==0) strncpy(config.server_hostname, "INADDR_ANY", sizeof(config.server_hostname)-1);
	config.sql_maxconn=config.server_maxconn*2;
	return 0;
}

int config_write()
{
	FILE *fp=NULL;
	struct stat sb;
	char slash='\\';

	fixslashes(config.server_dir_base);
	fixslashes(config.server_dir_bin);
	fixslashes(config.server_dir_cgi);
	fixslashes(config.server_dir_etc);
	fixslashes(config.server_dir_lib);
	fixslashes(config.server_dir_var);
	if (stat(config.server_dir_etc, &sb)!=0) return -1;
	snprintf(config.config_filename, sizeof(config.config_filename)-1, "%s/groupware.cfg", config.server_dir_etc);
	fixslashes(config.config_filename);
	fp=fopen(config.config_filename, "w");
	if (fp==NULL) return -1;
	fprintf(fp, "# This file contains system settings for NullLogic Groupware.\n\n");
	fprintf(fp, "SERVER.DIR.BASE   = \"%s\"\n", config.server_dir_base);
	fprintf(fp, "SERVER.DIR.BIN    = \"%s\"\n", config.server_dir_bin);
	fprintf(fp, "SERVER.DIR.CGI    = \"%s\"\n", config.server_dir_cgi);
	fprintf(fp, "SERVER.DIR.ETC    = \"%s\"\n", config.server_dir_etc);
	fprintf(fp, "SERVER.DIR.LIB    = \"%s\"\n", config.server_dir_lib);
	fprintf(fp, "SERVER.DIR.VAR    = \"%s\"\n", config.server_dir_var);
	fprintf(fp, "SERVER.LOGLEVEL   = \"%d\"\n", config.server_loglevel);
	fprintf(fp, "SERVER.HOSTNAME   = \"%s\"\n", config.server_hostname);
	fprintf(fp, "SERVER.PORT       = \"%d\"\n", config.server_port);
	fprintf(fp, "SERVER.MAXCONN    = \"%d\"\n", config.server_maxconn);
	fprintf(fp, "SERVER.MAXIDLE    = \"%d\"\n", config.server_maxidle);
	fprintf(fp, "SQL.TYPE          = \"%s\"\n", config.sql_type);
	fprintf(fp, "SQL.HOSTNAME      = \"%s\"\n", config.sql_hostname);
	fprintf(fp, "SQL.PORT          = \"%d\"\n", config.sql_port);
	fprintf(fp, "SQL.DBNAME        = \"%s\"\n", config.sql_dbname);
	fprintf(fp, "SQL.USERNAME      = \"%s\"\n", config.sql_username);
	fprintf(fp, "SQL.PASSWORD      = \"%s\"\n", config.sql_password);
	fprintf(fp, "SQL.ODBC_DSN      = \"%s\"\n", config.sql_odbc_dsn);
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
		ComboBox_SetCurSel(hZAlist, config.server_loglevel);
//		EnableWindow(hZAlist, 1);
		ComboBox_ResetContent(hZBlist);
		ComboBox_AddString(hZBlist, "MYSQL");
		ComboBox_AddString(hZBlist, "ODBC");
		ComboBox_AddString(hZBlist, "PGSQL");
		ComboBox_AddString(hZBlist, "SQLITE");
		if (strcasecmp(config.sql_type, "MYSQL")==0) {
			ComboBox_SetCurSel(hZBlist, 0);
		} else if (strcasecmp(config.sql_type, "ODBC")==0) {
			ComboBox_SetCurSel(hZBlist, 1);
		} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
			ComboBox_SetCurSel(hZBlist, 2);
		} else if (strcasecmp(config.sql_type, "SQLITE")==0) {
			ComboBox_SetCurSel(hZBlist, 3);
		}
//		EnableWindow(hZBlist, 1);
		SetDlgItemText(hDlg, IDC_EDIT1,  config.server_dir_base);
		SetDlgItemText(hDlg, IDC_EDIT2,  config.server_dir_bin);
		SetDlgItemText(hDlg, IDC_EDIT3,  config.server_dir_cgi);
		SetDlgItemText(hDlg, IDC_EDIT4,  config.server_dir_etc);
		SetDlgItemText(hDlg, IDC_EDIT5,  config.server_dir_lib);
		SetDlgItemText(hDlg, IDC_EDIT6,  config.server_dir_var);
//		SetDlgItemText(hDlg, IDC_COMBO7, config.server_loglevel);
		SetDlgItemText(hDlg, IDC_EDIT8,  config.server_hostname);
		snprintf(buffer, sizeof(buffer)-1, "%d", config.server_port);
		SetDlgItemText(hDlg, IDC_EDIT9,  buffer);
//		SetDlgItemText(hDlg, IDC_COMBO10, config.sql_type);
		SetDlgItemText(hDlg, IDC_EDIT11,  config.sql_hostname);
		snprintf(buffer, sizeof(buffer)-1, "%d", config.sql_port);
		SetDlgItemText(hDlg, IDC_EDIT12,  buffer);
		SetDlgItemText(hDlg, IDC_EDIT13,  config.sql_dbname);
		SetDlgItemText(hDlg, IDC_EDIT14,  config.sql_username);
		SetDlgItemText(hDlg, IDC_EDIT15,  config.sql_password);
		SetDlgItemText(hDlg, IDC_EDIT16,  config.sql_odbc_dsn);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
//		case IDCANCEL:
//			EndDialog(hDlg, 0);
//			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT1,   config.server_dir_base,   sizeof(config.server_dir_base)-1);
			GetDlgItemText(hDlg, IDC_EDIT2,   config.server_dir_bin ,   sizeof(config.server_dir_bin)-1);
			GetDlgItemText(hDlg, IDC_EDIT3,   config.server_dir_cgi,    sizeof(config.server_dir_cgi)-1);
			GetDlgItemText(hDlg, IDC_EDIT4,   config.server_dir_etc,    sizeof(config.server_dir_etc)-1);
			GetDlgItemText(hDlg, IDC_EDIT5,   config.server_dir_lib,    sizeof(config.server_dir_lib)-1);
			GetDlgItemText(hDlg, IDC_EDIT6,   config.server_dir_var,    sizeof(config.server_dir_var)-1);
			GetDlgItemText(hDlg, IDC_COMBO7,  buffer,                   sizeof(buffer)-1);
			config.server_loglevel=atoi(buffer);
			GetDlgItemText(hDlg, IDC_EDIT8,   config.server_hostname,   sizeof(config.server_hostname)-1);
			GetDlgItemText(hDlg, IDC_EDIT9,   buffer,                   sizeof(buffer)-1);
			config.server_port=atoi(buffer);
			GetDlgItemText(hDlg, IDC_COMBO10, config.sql_type,          sizeof(config.sql_type)-1);
			GetDlgItemText(hDlg, IDC_EDIT11,  config.sql_hostname,      sizeof(config.sql_hostname)-1);
			GetDlgItemText(hDlg, IDC_EDIT12,  buffer,                   sizeof(buffer)-1);
			config.sql_port=atoi(buffer);
			GetDlgItemText(hDlg, IDC_EDIT13,  config.sql_dbname,        sizeof(config.sql_dbname)-1);
			GetDlgItemText(hDlg, IDC_EDIT14,  config.sql_username,      sizeof(config.sql_username)-1);
			GetDlgItemText(hDlg, IDC_EDIT15,  config.sql_password,      sizeof(config.sql_password)-1);
			GetDlgItemText(hDlg, IDC_EDIT16,  config.sql_odbc_dsn,      sizeof(config.sql_odbc_dsn)-1);
			config_write();
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
		SetDlgItemText(hDlg, IDC_EDIT1,  config.sql_type);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT2, buffer, sizeof(buffer)-1);
			snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\dbutil dump \"%s\"", buffer);
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
		SetDlgItemText(hDlg, IDC_EDIT1,  config.sql_type);
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
				snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\dbutil init \"%s\"", buffer);
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
		SetDlgItemText(hDlg, IDC_EDIT1,  config.sql_type);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
			return (TRUE);
		case IDC_BUTTON1:
			GetDlgItemText(hDlg, IDC_EDIT2, buffer, sizeof(buffer)-1);
			snprintf(cmdbuffer, sizeof(cmdbuffer)-1, ".\\dbutil restore \"%s\"", buffer);
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
		SetDlgItemText(hDlg, IDC_EDIT1,  config.sql_type);
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
	config_read();
	if (strstr(lpCmdLine, "initdb")!=NULL) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DBUTIL), NULL, DbutilDlgProc);
	} else {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL, ConfigDlgProc);
	}
	return 0;
}
