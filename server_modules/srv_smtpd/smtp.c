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
/*
         The following are the SMTP commands:

            HELO <SP> <domain> <CRLF>
            MAIL <SP> FROM:<reverse-path> <CRLF>
            RCPT <SP> TO:<forward-path> <CRLF>
            DATA <CRLF>
            RSET <CRLF>
            SEND <SP> FROM:<reverse-path> <CRLF>
            SOML <SP> FROM:<reverse-path> <CRLF>
            SAML <SP> FROM:<reverse-path> <CRLF>
            VRFY <SP> <string> <CRLF>
            EXPN <SP> <string> <CRLF>
            HELP [<SP> <string>] <CRLF>
            NOOP <CRLF>
            QUIT <CRLF>
            TURN <CRLF>
*/

typedef struct {
	short unsigned int sender_is_local;
	short unsigned int num_rcpts;
	short unsigned int rcptalloc;
	int msgbodysize;
	char helo[1024];
	char from[1024];
	char *msgbody;
	char **rcpt;
} MAILCONN;

static void smtp_accept(CONN *sid, MAILCONN *mconn)
{
	FILE *fp;
	struct stat sb;
	char tmpname[256];
	char tmpaddr[128];
	char *host;
	char *buf_ptr;
	int i;
	int userid;
	int sqr;
	int is_remote;
	struct timeval ttime;
	struct timezone tzone;
	int blocksize;
	int bytes;

	for (i=0;i<mconn->num_rcpts;i++) {
		is_remote=0;
		snprintf(tmpaddr, sizeof(tmpaddr)-1, "%s", mconn->rcpt[i]);
		if ((host=strchr(tmpaddr, '@'))!=NULL) {
			*host++='\0';
		} else {
			host="localhost";
		}
		if (strcasecmp(host, "localhost")==0) {
			userid=0;
			if ((sqr=sql_queryf("SELECT userid FROM gw_users WHERE username = '%s'", tmpaddr))<0) return;
			if (sql_numtuples(sqr)==1) userid=atoi(sql_getvalue(sqr, 0, 0));
			sql_freeresult(sqr);
			if (userid<1) continue;
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/local/%s", config->server_dir_var_mail, tmpaddr);
			if (stat(tmpname, &sb)!=0) {
#ifdef WIN32
				if (mkdir(tmpname)!=0) {
#else
				if (mkdir(tmpname, 0700)!=0) {
#endif
					log_error("smtp", __FILE__, __LINE__, 0, "Error creating directory '%s'", tmpname);
					continue;
				}
			}
retry1:
			gettimeofday(&ttime, &tzone);
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/local/%s/%d%03d.msg", config->server_dir_var_mail, tmpaddr, (int)ttime.tv_sec, (int)(ttime.tv_usec/1000));
			fixslashes(tmpname);
			if (stat(tmpname, &sb)==0) goto retry1;
			log_access("smtp", "local delivery from: '%s', to: '%s'", mconn->from, mconn->rcpt[i]);
		} else if (mconn->sender_is_local) {
			is_remote=1;
retry2:
			gettimeofday(&ttime, &tzone);
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/queue/%d%03d.msg", config->server_dir_var_mail, (int)ttime.tv_sec, (int)(ttime.tv_usec/1000));
			fixslashes(tmpname);
			if (stat(tmpname, &sb)==0) goto retry2;
		} else {
			log_error("smtp", __FILE__, __LINE__, 1, "relaying denied.");
			continue;
		}
		if ((fp=fopen(tmpname, "wb"))!=NULL) {
			if (is_remote) {
				fprintf(fp, "X-SMTP-MAIL: %s\r\n", mconn->from);
				fprintf(fp, "X-SMTP-RCPT: %s\r\n", mconn->rcpt[i]);
			}
			buf_ptr=mconn->msgbody;
			blocksize=mconn->msgbodysize;
			do {
				if ((bytes=fwrite(buf_ptr, sizeof(char), blocksize, fp))<0) return;
				blocksize-=bytes;
				buf_ptr+=bytes;
			} while (blocksize>0);
			fclose(fp);
		} else {
			log_error("smtp", __FILE__, __LINE__, 0, "ERROR: Cannot write to file [%s].", tmpname);
		}
	}
	return;
}

static void smtp_data(CONN *sid, MAILCONN *mconn)
{
	char line[1024];
	char *buf_ptr;
	unsigned int buf_alloc;

	tcp_fprintf(&sid->socket, "354 Send the mail data, end with .\r\n");
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody=NULL; }
	buf_alloc=65536;
	mconn->msgbody=calloc(buf_alloc, sizeof(char));
	buf_ptr=mconn->msgbody;
	mconn->msgbodysize=0;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) return;
		if ((strcmp(line, ".\r\n")==0)||(strcmp(line, ".\n")==0)) break;
		if ((mconn->msgbodysize+strlen(line)+2)>buf_alloc) {
			buf_alloc+=131072;
			mconn->msgbody=realloc(mconn->msgbody, buf_alloc*sizeof(char));
			if (mconn->msgbody==NULL) return;
		}
		buf_ptr=mconn->msgbody+mconn->msgbodysize;
		strcat(buf_ptr, line);
		mconn->msgbodysize+=strlen(line);
	} while (1);
	smtp_accept(sid, mconn);
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody=NULL; }
	tcp_fprintf(&sid->socket, "250 Message accepted for delivery\r\n");
	return;
}

static void smtp_from(CONN *sid, MAILCONN *mconn, char *line)
{
	char address[120];
	char *ptemp;
	char *host;
	int i;

	for (i=0;i<mconn->num_rcpts;i++) {
		if (mconn->rcpt[i]!=NULL) { free(mconn->rcpt[i]); mconn->rcpt[i]=NULL; }
	}
	memset(mconn->from, 0, sizeof(mconn->from));
	mconn->sender_is_local=0;
	mconn->num_rcpts=0;
	ptemp=line;
	while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
	snprintf(address, sizeof(address)-1, "%s", ptemp);
	if ((ptemp=strchr(address, '>'))!=NULL) *ptemp='\0';
	if ((ptemp=strchr(address, '<'))!=NULL) {
		ptemp++;
	} else {
		ptemp=address;
	}
	snprintf(mconn->from, sizeof(mconn->from)-1, "%s", ptemp);
	if ((host=strchr(address, '@'))!=NULL) {
		*host++='\0';
	} else {
		host="localhost";
	}
	if (strcasecmp(host, "localhost")==0) mconn->sender_is_local=1;
	if (strlen(mconn->from)) {
		tcp_fprintf(&sid->socket, "250 Sender '%s' OK\r\n", mconn->from);
	} else {
		tcp_fprintf(&sid->socket, "500 Sender Invalid\r\n");
	}
	return;
}

static void smtp_rcpt(CONN *sid, MAILCONN *mconn, char *line)
{
	char address[120];
	char *ptemp;

	ptemp=line;
	while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
	snprintf(address, sizeof(address)-1, "%s", ptemp);
	if ((ptemp=strchr(address, '>'))!=NULL) *ptemp='\0';
	if ((ptemp=strchr(address, '<'))!=NULL) {
		ptemp++;
	} else {
		ptemp=address;
	}
	mconn->rcpt[mconn->num_rcpts]=calloc(128, sizeof(char));
	snprintf(mconn->rcpt[mconn->num_rcpts], 127, "%s", ptemp);
	mconn->num_rcpts++;
	if (strlen(mconn->rcpt[mconn->num_rcpts-1])) {
		tcp_fprintf(&sid->socket, "250 Recipient '%s' OK\r\n", mconn->rcpt[mconn->num_rcpts-1]);
	} else {
		tcp_fprintf(&sid->socket, "500 Sender Invalid\r\n");
	}
	return;
}

static void smtp_rset(CONN *sid, MAILCONN *mconn)
{
	int i;

	for (i=0;i<mconn->num_rcpts;i++) {
		if (mconn->rcpt[i]!=NULL) { free(mconn->rcpt[i]); mconn->rcpt[i]=NULL; }
	}
	memset(mconn->from, 0, sizeof(mconn->from));
	mconn->sender_is_local=0;
	mconn->num_rcpts=0;
	tcp_fprintf(&sid->socket, "250 Reset OK\r\n");
	return;
}

static void smtp_noop(CONN *sid, MAILCONN *mconn)
{
	tcp_fprintf(&sid->socket, "220 OK\r\n");
	return;
}

static void smtp_turn(CONN *sid, MAILCONN *mconn)
{
	tcp_fprintf(&sid->socket, "502 No\r\n");
	return;
}

void smtp_dorequest(CONN *sid)
{
	MAILCONN mconn;
	char line[128];
	char *ptemp;
	int i;

	memset((char *)&mconn, 0, sizeof(mconn));
	tcp_fprintf(&sid->socket, "250 Welcome to %s SMTPd\r\n", SERVER_NAME);
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) return;
		striprn(line);
		if (strcasecmp(line, "quit")==0) {
			tcp_fprintf(&sid->socket, "221 Goodbye\r\n");
			return;
		} else if ((strncasecmp(line, "helo", 4)==0)||(strncasecmp(line, "ehlo", 4)==0)) {
			ptemp=line+4;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(mconn.helo, sizeof(mconn.helo)-1, "%s", ptemp);
			tcp_fprintf(&sid->socket, "250 Hello, %s\r\n", mconn.helo);
			if (strlen(mconn.helo)) break;
		} else {
			tcp_fprintf(&sid->socket, "500 Some people still say hello\r\n");
		}
	} while (1);
//	strncpy(sid->dat->in_RemoteAddr, inet_ntoa(sid->ClientAddr.sin_addr), sizeof(sid->dat->in_RemoteAddr)-1);
	log_access("smtp", "HELO '%s' (%s)", mconn.helo, inet_ntoa(sid->socket.ClientAddr.sin_addr));
	mconn.rcptalloc=50;
	if ((mconn.rcpt=(char **)calloc(mconn.rcptalloc, sizeof(char *)))==NULL) return;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) goto cleanup;
		striprn(line);
		if (strcasecmp(line, "quit")==0) {
			tcp_fprintf(&sid->socket, "221 Goodbye\r\n");
			break;
		} else if (strncasecmp(line, "mail from:", 10)==0) {
			smtp_from(sid, &mconn, line+10);
		} else if (strncasecmp(line, "rcpt to:", 8)==0) {
			smtp_rcpt(sid, &mconn, line+8);
		} else if (strcasecmp(line, "data")==0) {
			smtp_data(sid, &mconn);
		} else if (strcasecmp(line, "rset")==0) {
			smtp_rset(sid, &mconn);
		} else if (strcasecmp(line, "turn")==0) {
			smtp_turn(sid, &mconn);
		} else if (strcasecmp(line, "noop")==0) {
			smtp_noop(sid, &mconn);
		} else {
			log_error("smtp", __FILE__, __LINE__, 1, "UNKNOWN COMMAND: '%s'", line);
			tcp_fprintf(&sid->socket, "500 Unknown Command\r\n");
		}
	} while (1);
cleanup:
	for (i=0;i<mconn.num_rcpts;i++) {
		if (mconn.rcpt[i]!=NULL) {
			free(mconn.rcpt[i]);
		}
	}
	free(mconn.rcpt);
	return;
}
