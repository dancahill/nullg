/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

#define SPOOL_DIR "/usr/local/nullg/var/spool"

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

void striprn(char *string)
{
	int i=strlen(string)-1;

	while (1) {
		if (i<0) break;
		if (string[i]=='\r') { string[i]='\0'; i--; continue; }
		if (string[i]=='\n') { string[i]='\0'; i--; continue; }
		break;
	};
}
/*
void usage(char *arg0)
{
	char *progname;

	printf("\r\nNullLogic GroupServer Database Utility [NullLogic GroupServer vX]\r\n");
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
	printf("\r\nThe INIT command is used to initialize a new NullLogic GroupServer database\r\n");
	printf("(as defined in nullg.conf).  No parameter is required.\r\n");
	printf("\r\nThe DUMP command is used to export the contents of the current database\r\n");
	printf("(as defined in nullg.conf) to a text file.  The parameter for this function\r\n");
	printf("is the name of the file to which the data is to be saved.\r\n");
	printf("\r\nThe RESTORE command is used to restore a previous database dump to the current\r\n");
	printf("database (as defined in nullg.conf).  The parameter for this function is the\r\n");
	printf("name of the file from which the data is to be restored.\r\n");
	printf("\r\nNOTE: The INIT and RESTORE commands WILL destroy any current database before\r\n");
	printf("recreating it.  Be sure to use DUMP to make a backup prior to using the INIT\r\n");
	printf("and RESTORE commands.\r\n");
	return;
}
*/
int main(int argc, char *argv[])
{
	FILE *fp;
	char filename[250];
	char from[250];
	char rcpt[250];
//	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//	char *salt="$1$/Ktr8sET$";
//	char salt[10];
//	char function[16];
//	char parameter[64];
	char inbuf[1024];
	char *ptemp;
	int i;
	time_t t=time(NULL);


/*
	FILE *fp=NULL;
	short int status;

	if (argc<2) return -1;
	if ((fp=fopen(argv[1], "rb"))!=NULL) {
		status=smtp_client(fp);
		fclose(fp);
	}
	return 0;
*/


	setvbuf(stdout, NULL, _IONBF, 0);
//	memset(salt, 0, sizeof(salt));
//	memset(function, 0, sizeof(function));
//	memset(parameter, 0, sizeof(parameter));
/*
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
*/
	memset(from, 0, sizeof(from));
	memset(rcpt, 0, sizeof(rcpt));
//	printf("[%d]\r\n", argc);
	for (i=0;i<argc;i++) {
//		printf("[%d][%s]\r\n", i, argv[i]);
		if (argv[i][0]!='-') continue;
		switch (argv[i][1]) {
		case 'f':
			if ((argv[i][2]=='\0')&&(argc>i+1)) {
				snprintf(from, sizeof(from)-1, "%s", argv[i+1]);
			} else {
				ptemp=argv[i]+2;
				snprintf(from, sizeof(from)-1, "%s", ptemp);
			}
			break;
		case 't':
			if ((argv[i][2]=='\0')&&(argc>i+1)) {
				snprintf(rcpt, sizeof(rcpt)-1, "%s", argv[i+1]);
			} else {
				ptemp=argv[i]+2;
				snprintf(rcpt, sizeof(rcpt)-1, "%s", ptemp);
			}
			break;
		default:
//			printf("unknown option\r\n");
			break;
		}
	}
	printf("MAIL: <%s>\r\n", from);
	printf("RCPT: <%s>\r\n", rcpt);
	snprintf(filename, sizeof(filename)-1, "%s/mqueue/%d.msg", SPOOL_DIR, (int)t);
	fixslashes(filename);
	if ((fp=fopen(filename, "wb"))!=NULL) {
		fprintf(fp, "From: %s\r\n", from);
		fprintf(fp, "To: %s\r\n", rcpt);
		fprintf(fp, "\r\n");
		while (fgets(inbuf, sizeof(inbuf)-1, stdin)!=NULL) {
			striprn(inbuf);
			fprintf(fp, "%s\r\n", inbuf);
		}
		fclose(fp);
	}
	snprintf(filename, sizeof(filename)-1, "%s/mqinfo/%d.dat", SPOOL_DIR, (int)t);
	fixslashes(filename);
	if ((fp=fopen(filename, "wb"))!=NULL) {
		fprintf(fp, "MAIL: %s\r\n", from);
		fprintf(fp, "RCPT: %s\r\n", rcpt);
		fprintf(fp, "DATE: whenever\r\n");
		fclose(fp);
	}
	return 0;
}
