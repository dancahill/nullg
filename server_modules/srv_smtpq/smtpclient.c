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

int smtp_printf(int smtp_sock, const char *format, ...)
{
	char buffer[1024];
	va_list ap;
	int len;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	send(smtp_sock, buffer, strlen(buffer), 0);
	len=strlen(buffer);
	log_error("smtpq_debug", __FILE__, __LINE__, 1, ">> %s", buffer);
	return len;
}

int smtp_fgets(char *buffer, int max, int smtp_sock)
{
	char *pbuffer=buffer;
	char temp[2];
	int n=0;

	memset(temp, 0, sizeof(temp));
	while (n<max) {
		if (recv(smtp_sock, temp, 1, 0)<1) {
			return -1;
		}
		n++;
		*pbuffer=temp[0];
		pbuffer++;
		if (temp[0]==10) break;
	}
	*pbuffer=0;
	log_error("smtpq_debug", __FILE__, __LINE__, 1, "<< %s", buffer);
	return n;
}

static int smtp_connect(char *smtpserver)
{
	struct sockaddr_in server;
	struct hostent *hp;
	int smtp_sock;

	if ((hp=gethostbyname(smtpserver))==NULL) return -1;
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=(unsigned short)htons(25);
	if ((smtp_sock=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
	if (connect(smtp_sock, (struct sockaddr *)&server, sizeof(server))<0) return -1;
	return smtp_sock;
}

static void smtp_disconnect(int smtp_sock)
{
	char inbuffer[1024];

	smtp_printf(smtp_sock, "QUIT\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	shutdown(smtp_sock, 2);
	closesocket(smtp_sock);
	return;
}

int smtp_client(FILE *fp)
{
	char inbuffer[1024];
	char host[128];
	char from[128];
	char rcpt[128];
	char *ptemp;
	int smtp_sock;

	memset(from, 0, sizeof(from));
	memset(host, 0, sizeof(host));
	gethostname(host, sizeof(host)-1);
	if (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
		if (strncasecmp(inbuffer, "X-SMTP-MAIL:", 12)==0) {
			striprn(inbuffer);
			ptemp=inbuffer+12;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(from, sizeof(from)-1, "%s", ptemp);
		}
	}
	memset(rcpt, 0, sizeof(rcpt));
	if (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
		if (strncasecmp(inbuffer, "X-SMTP-RCPT:", 12)==0) {
			striprn(inbuffer);
			ptemp=inbuffer+12;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(rcpt, sizeof(rcpt)-1, "%s", ptemp);
		}
	}
	if (strlen(from)==0) return -1;
	if (strlen(rcpt)==0) return -1;
	memset(inbuffer, 0, sizeof(inbuffer));
	if ((smtp_sock=smtp_connect(config->smtp_relayhost))<0) goto quit;
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	smtp_printf(smtp_sock, "HELO %s\r\n", host);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	smtp_printf(smtp_sock, "MAIL FROM: <%s>\r\n", from);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	smtp_printf(smtp_sock, "RCPT TO: <%s>\r\n", rcpt);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	smtp_printf(smtp_sock, "DATA\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "354", 3)!=0) goto quit;
	while (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
		smtp_printf(smtp_sock, "%s", inbuffer);
	}
	smtp_printf(smtp_sock, ".\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (smtp_fgets(inbuffer, sizeof(inbuffer)-1, smtp_sock)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	log_access("smtpd", "mail sent from: '%s', to: '%s'", from, rcpt);
	smtp_disconnect(smtp_sock);
	return 0;
quit:
	log_error("smtpd", __FILE__, __LINE__, 1, "remote server response: %s", inbuffer);
	smtp_disconnect(smtp_sock);
	return -1;
}

#ifdef WIN32
unsigned _stdcall smtp_spool(void *x)
#else
void *smtp_spool(void *x)
#endif
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	struct stat sb;
	DIR  *handle;
	FILE *fp=NULL;
	char dirname[256];
	char tmpname[256];
	short int status;

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/mqueue", config->server_dir_var_spool);
	fixslashes(dirname);
	while (1) {
		handle=opendir(dirname);
		while ((dentry=readdir(handle))!=NULL) {
			if (strcmp(".", dentry->d_name)==0) continue;
			if (strcmp("..", dentry->d_name)==0) continue;
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/%s", dirname, dentry->d_name);
			fixslashes(tmpname);
			if (stat(tmpname, &sb)!=0) continue;
			if (sb.st_mode&S_IFDIR) continue;
			if ((fp=fopen(tmpname, "rb"))!=NULL) {
				status=smtp_client(fp);
				fclose(fp);
				if (status==0) {
					unlink(tmpname);
				}
			}
		}
		closedir(handle);
		sleep(60);
	}
	return 0;
}
