/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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

void usage(char *arg0)
{
	char *progname;

	printf("\r\nNullLogic Groupware Database Utility [NullLogic Groupware %s]\r\n", PACKAGE_VERSION);
#ifdef WIN32
	progname=strrchr(arg0, '\\');
#else
	progname=strrchr(arg0, '/');
#endif
	if (progname==NULL) {
		progname=arg0;
	} else {
		progname++;
	}
	printf("Usage: %s command parameter\r\n\r\n", progname);
	printf("  command  Must be one of the following\r\n\r\n");
	printf("    INIT     Initializes a new database\r\n");
	printf("    DUMP     Dumps the data from the current database to a file\r\n");
	printf("    RESTORE  Restores the current database from a file\r\n");
	printf("\r\nThe INIT command is used to initialize a new NullLogic Groupware database\r\n");
	printf("(as defined in groupware.cfg).  No parameter is required.\r\n");
	printf("\r\nThe DUMP command is used to export the contents of the current database\r\n");
	printf("(as defined in groupware.cfg) to a text file.  The parameter for this function\r\n");
	printf("is the name of the file to which the data is to be saved.\r\n");
	printf("\r\nThe RESTORE command is used to restore a previous database dump to the current\r\n");
	printf("database (as defined in groupware.cfg).  The parameter for this function is the\r\n");
	printf("name of the file from which the data is to be restored.\r\n");
	printf("\r\nNOTE: The INIT and RESTORE commands WILL destroy any current database before\r\n");
	printf("recreating it.  Be sure to use DUMP to make a backup prior to using the INIT\r\n");
	printf("and RESTORE commands.\r\n");
	return;
}

int main(int argc, char *argv[])
{
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//	char *salt="$1$/Ktr8sET$";
	char salt[10];
	char function[16];
	char parameter[64];
	char *pTemp;
	int i;

	setvbuf(stdout, NULL, _IONBF, 0);
	memset(rootpass, 0, sizeof(rootpass));
	memset(salt, 0, sizeof(salt));
	memset((char *)&config, 0, sizeof(config));
	memset(function, 0, sizeof(function));
	memset(parameter, 0, sizeof(parameter));
	if (argc<2) {
		usage(argv[0]);
		return 0;
	}
	strncpy(function, argv[1], sizeof(function)-1);
	pTemp=function;
	while (*pTemp) {
		*pTemp=tolower(*pTemp);
		pTemp++;
	}
	if ((strcmp(function, "init")!=0)&&(argc!=3)) {
		usage(argv[0]);
		return 0;
	}
	if (argc>2) {
		strncpy(parameter, argv[2], sizeof(parameter)-1);
	}
	conf_read();
	if (strcmp(function, "init")==0) {
		if (argc==3) {
			srand(time(NULL));
			for (i=0;i<8;i++) salt[i]=itoa64[(rand()%64)];
			strncpy(rootpass, MD5Crypt(argv[2], salt), sizeof(rootpass)-1);
		}
		return init_db();
	}
	if (strcmp(function, "dump")==0) {
		return dump_db(parameter);
	}
	if (strcmp(function, "restore")==0) {
		return restore_db(parameter);
	}
	return 0;
}
