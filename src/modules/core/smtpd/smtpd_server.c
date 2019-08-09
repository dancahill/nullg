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
#include "smtpd_main.h"
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
	short unsigned int sender_can_relay;
	short unsigned int sender_is_local;
	short unsigned int sender_is_blank;
	short unsigned int num_rcpts;
	short unsigned int rcptalloc;
	int msgbodysize;
	char helo[1024];
	char ehlo[1024];
	char from[1024];
	char *msgbody;
	char **rcpt;
} MAILCONN;

typedef struct {
	char From[80];
	char Date[80];
	char MessageID[80];
} MSGHEADER;

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

static int allow_relay(CONN *sid)
{
	char curdate[32];
	SQLRES sqr;
	int allowed;

	allowed=0;
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL)-mod_config.popauth_window);
	sql_updatef("DELETE FROM gw_smtp_relayrules WHERE persistence <> 'perm' AND obj_mtime < '%s'", curdate);
	if (sql_queryf(&sqr, "SELECT * FROM gw_smtp_relayrules WHERE ipaddress = '%s' GROUP BY ipaddress", sid->dat->user_RemoteAddr)<0) return 0;
	if (sql_numtuples(&sqr)>0) {
		allowed=1;
		log_error(MODSHORTNAME, __FILE__, __LINE__, 2, "%s is allowed to relay", sid->dat->user_RemoteAddr);
	}
	sql_freeresult(&sqr);
	return allowed;
}

static int smtp_accept(CONN *sid, MAILCONN *mconn)
{
	FILE *fp;
	struct stat sb;
	char tmpname1[256];
	char tmpname2[256];
	char tmpaddr[128];
	char *host;
	char *buf_ptr;
	int i;
	int localdomainid;
	int userid;
	SQLRES sqr;
	int is_remote;
	int err;
	struct timeval ttime;
	struct timezone tzone;
	int blocksize;
	int bytes;
	char curdate1[40];
	char curdate2[40];
	time_t t=time(NULL);

	memset(curdate1, 0, sizeof(curdate1));
	memset(curdate2, 0, sizeof(curdate2));
	strftime(curdate1, sizeof(curdate1), "%a, %d %b %Y %H:%M:%S +0000", gmtime(&t));
	time_unix2sql(curdate2, sizeof(curdate2)-1, t);
	for (i=0;i<mconn->num_rcpts;i++) {
		is_remote=0;
		snprintf(tmpaddr, sizeof(tmpaddr)-1, "%s", mconn->rcpt[i]);
		if ((host=strchr(tmpaddr, '@'))!=NULL) {
			*host++='\0';
		} else {
			host="localhost";
		}
		localdomainid=domain_getid(host);
		memset(tmpname1, 0, sizeof(tmpname1));
		memset(tmpname2, 0, sizeof(tmpname2));
		if (localdomainid>0) {
			userid=0;
			if (sql_queryf(&sqr, "SELECT userid FROM gw_users WHERE username = '%s' AND domainid = %d", tmpaddr, localdomainid)<0) return -1;
			if (sql_numtuples(&sqr)==1) userid=atoi(sql_getvalue(&sqr, 0, 0));
			sql_freeresult(&sqr);
			if (userid<1) {
				bounce_send(mconn->from, mconn->rcpt[i], NULL, "554 no such mailbox");
				log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "%s - no such mailbox: '%s', sent from '%s'", sid->dat->user_RemoteAddr, mconn->rcpt[i], mconn->from);
				continue;
			}
			snprintf(tmpname1, sizeof(tmpname1)-1, "%s/%04d/mailspool/%s", config->dir_var_domains, localdomainid, tmpaddr);
			if (stat(tmpname1, &sb)!=0) {
#ifdef WIN32
				if (mkdir(tmpname1)!=0) {
#else
				if (mkdir(tmpname1, ~config->umask&0777)!=0) {
#endif
					log_error(MODSHORTNAME, __FILE__, __LINE__, 0, "Error creating directory '%s'", tmpname1);
					continue;
				}
			}
retry1:
			gettimeofday(&ttime, &tzone);
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			snprintf(tmpname1, sizeof(tmpname1)-1, "%s/%04d/mailspool/%s/%d%03d.msg", config->dir_var_domains, localdomainid, tmpaddr, (int)ttime.tv_sec, (int)(ttime.tv_usec/1000));
			fixslashes(tmpname1);
			if (stat(tmpname1, &sb)==0) goto retry1;
			log_access("smtpd", "%s:%d - local delivery from:'%s', for:'%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn->from, mconn->rcpt[i]);
		} else if (mconn->sender_can_relay) {
			is_remote=1;
retry2:
			gettimeofday(&ttime, &tzone);
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			snprintf(tmpname1, sizeof(tmpname1)-1, "%s/mqueue/%d%03d.msg", config->dir_var_spool, (int)ttime.tv_sec, (int)(ttime.tv_usec/1000));
 			snprintf(tmpname2, sizeof(tmpname2)-1, "%s/mqinfo/%d%03d.dat", config->dir_var_spool, (int)ttime.tv_sec, (int)(ttime.tv_usec/1000));
 			fixslashes(tmpname1);
 			fixslashes(tmpname2);
			if (stat(tmpname1, &sb)==0) goto retry2;
			if (stat(tmpname2, &sb)==0) goto retry2;
			log_access("smtpd", "%s:%d - queued mail from:'%s', for:'%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn->from, mconn->rcpt[i]);
		} else {
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "%s - relaying denied from:'%s', for:'%s'", sid->dat->user_RemoteAddr, mconn->from, mconn->rcpt[i]);
			continue;
		}
		if ((fp=fopen(tmpname1, "wb"))!=NULL) {
			fprintf(fp, "Received: from %s ([%s])\r\n", mconn->helo, sid->dat->user_RemoteAddr);
			fprintf(fp, "	by %s for %s;\r\n", proc->config.hostname, mconn->rcpt[i]);
			fprintf(fp, "	%s\r\n", curdate1);
			buf_ptr=mconn->msgbody;
			blocksize=mconn->msgbodysize;
			do {
				if ((bytes=fwrite(buf_ptr, sizeof(char), blocksize, fp))<0) return -1;
				blocksize-=bytes;
				buf_ptr+=bytes;
			} while (blocksize>0);
			fclose(fp);
			err=filter_scan(sid, tmpname1);
			if (err>0) {
				unlink(tmpname1);
				return err;
			}
			if (is_remote) {
				fp=fopen(tmpname2, "wb");
				if (fp==NULL) {
					unlink(tmpname1);
					continue;
				}
				fprintf(fp, "MAIL: %s\r\n", mconn->from);
				fprintf(fp, "RCPT: %s\r\n", mconn->rcpt[i]);
				fprintf(fp, "DATE: %s\r\n", curdate2);
				fclose(fp);
			}
		} else {
			log_error(MODSHORTNAME, __FILE__, __LINE__, 0, "ERROR: Cannot write to file [%s].", tmpname1);
			return -1;
		}
	}
	return 0;
}

static void smtp_data(CONN *sid, MAILCONN *mconn)
{
	MSGHEADER header;
	char curdate[40];
	char line[1024];
	char outbuf[1024];
	char *buf_ptr;
	unsigned int buf_alloc;
	short int header_done;
	int rc;
	time_t t=time(NULL);

	if ((strlen(mconn->from)==0)&&(mconn->sender_is_blank==0)) {
		tcp_fprintf(&sid->socket, "503 No sender\r\n");
		return;
	}
	if (mconn->num_rcpts==0) {
		tcp_fprintf(&sid->socket, "503 No recipient\r\n");
		return;
	}
	tcp_fprintf(&sid->socket, "354 Send the mail data, end with .\r\n");
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody=NULL; }
	buf_alloc=65536;
	mconn->msgbody=calloc(buf_alloc, sizeof(char));
	buf_ptr=mconn->msgbody;
	mconn->msgbodysize=0;
	header_done=0;
	memset((char *)&header, 0, sizeof(header));
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) return;
		if (!header_done) {
			memset(outbuf, 0, sizeof(outbuf));
			if (strncasecmp(line, "From: ", 6)==0) {
				strncpy(header.From, (char *)&line+6, sizeof(header.From)-1);
				striprn(header.From);
			} else if (strncasecmp(line, "Date: ", 6)==0) {
				strncpy(header.Date, (char *)&line+6, sizeof(header.Date)-1);
				striprn(header.Date);
			} else if (strncasecmp(line, "Message-ID: ", 12)==0) {
				strncpy(header.MessageID, (char *)&line+12, sizeof(header.MessageID)-1);
				striprn(header.MessageID);
			} else if ((strcmp(line, "\r\n")==0)||(strcmp(line, "\n")==0)||(strcmp(line, ".\r\n")==0)||(strcmp(line, ".\n")==0)) {
				header_done=1;
				if (!strlen(header.From)) {
					strncatf(outbuf, sizeof(outbuf), "From: %s\r\n", mconn->from);
				}
				if (!strlen(header.Date)) {
					strftime(curdate, sizeof(curdate), RFC1123FMT, gmtime(&t));
					strncatf(outbuf, sizeof(outbuf), "Date: %s\r\n", curdate);
					strncatf(outbuf, sizeof(outbuf), "Date-warning: Date header was inserted by %s\r\n", proc->config.hostname);
				}
				if (!strlen(header.MessageID)) {
					strncatf(outbuf, sizeof(outbuf), "Message-id: <%d@%s>\r\n", t, proc->config.hostname);
				}
				if (strlen(outbuf)) {
					if ((mconn->msgbodysize+strlen(outbuf)+2)>buf_alloc) {
						buf_alloc*=2;
						mconn->msgbody=realloc(mconn->msgbody, buf_alloc*sizeof(char));
						if (mconn->msgbody==NULL) {
							tcp_fprintf(&sid->socket, "451 Temporary failure - try again later\r\n");
							return;
						}
					}
					buf_ptr=mconn->msgbody+mconn->msgbodysize;
					strcat(buf_ptr, outbuf);
					mconn->msgbodysize+=strlen(outbuf);
				}
			}
		}
		if ((strcmp(line, ".\r\n")==0)||(strcmp(line, ".\n")==0)) {
			break;
		}
		if ((mconn->msgbodysize+strlen(line)+2)>buf_alloc) {
			buf_alloc*=2;
			mconn->msgbody=realloc(mconn->msgbody, buf_alloc*sizeof(char));
			if (mconn->msgbody==NULL) {
				tcp_fprintf(&sid->socket, "451 Temporary failure - try again later\r\n");
				return;
			}
		}
		buf_ptr=mconn->msgbody+mconn->msgbodysize;
		strcat(buf_ptr, line);
		mconn->msgbodysize+=strlen(line);
	} while (1);
	rc=smtp_accept(sid, mconn);
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody=NULL; }
	switch (rc) {
	case 0:
		tcp_fprintf(&sid->socket, "250 Message accepted for delivery\r\n");
		break;
	case 1:
		tcp_fprintf(&sid->socket, "554 Message is permanently rejected (spam)\r\n");
		break;
	case 2:
	case 3:
		tcp_fprintf(&sid->socket, "554 Message is permanently rejected (virus)\r\n");
		break;
	default:
		tcp_fprintf(&sid->socket, "451 Temporary failure - try again later\r\n");
		break;
	}
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
	if (strlen(mconn->from)) {
		tcp_fprintf(&sid->socket, "250 Sender '%s' OK\r\n", mconn->from);
	} else {
		mconn->sender_is_blank=1;
		tcp_fprintf(&sid->socket, "250 Sender 'NULL' OK\r\n");
	}
	return;
}

static void smtp_rcpt(CONN *sid, MAILCONN *mconn, char *line)
{
	char address[120];
	char *ptemp1;
	char *ptemp2;
	int domainid;

	if ((strlen(mconn->from)==0)&&(mconn->sender_is_blank==0)) {
		tcp_fprintf(&sid->socket, "503 No sender\r\n");
		return;
	}
	ptemp1=line;
	while ((*ptemp1==' ')||(*ptemp1=='\t')) ptemp1++;
	snprintf(address, sizeof(address)-1, "%s", ptemp1);
	if ((ptemp1=strchr(address, '>'))!=NULL) *ptemp1='\0';
	if ((ptemp1=strchr(address, '<'))!=NULL) {
		ptemp1++;
	} else {
		ptemp1=address;
	}
	ptemp2=strchr(ptemp1, '@');
	if (ptemp2==NULL) {
		log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "%s - recipient invalid from:'%s', to:'%s'", sid->dat->user_RemoteAddr, mconn->from, ptemp1);
		tcp_fprintf(&sid->socket, "553 Recipient Invalid\r\n");
		return;
	}
	ptemp2++;
	domainid=domain_getid(ptemp2);
	if (domainid<1) {
		/* rcpt domain is not local.  can sender relay? */
		if (!mconn->sender_can_relay) {
			log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "%s - relaying denied from:'%s', to:'%s'", sid->dat->user_RemoteAddr, mconn->from, ptemp1);
			log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "%s - sender is not local, and neither is the recipient domain '%s'", sid->dat->user_RemoteAddr, ptemp2);
			tcp_fprintf(&sid->socket, "553 Relaying denied\r\n");
			return;
		}
	}
	mconn->rcpt[mconn->num_rcpts]=calloc(128, sizeof(char));
	snprintf(mconn->rcpt[mconn->num_rcpts], 127, "%s", ptemp1);
	mconn->num_rcpts++;
	tcp_fprintf(&sid->socket, "250 Recipient '%s' OK\r\n", mconn->rcpt[mconn->num_rcpts-1]);
	return;
}

static void smtp_rset(CONN *sid, MAILCONN *mconn)
{
	int i;

	for (i=0;i<mconn->num_rcpts;i++) {
		if (mconn->rcpt[i]!=NULL) { free(mconn->rcpt[i]); mconn->rcpt[i]=NULL; }
	}
	memset(mconn->from, 0, sizeof(mconn->from));
	mconn->sender_can_relay=allow_relay(sid);
	mconn->sender_is_blank=0;
	mconn->num_rcpts=0;
	tcp_fprintf(&sid->socket, "250 Reset OK\r\n");
	return;
}

static void smtp_noop(CONN *sid)
{
	tcp_fprintf(&sid->socket, "220 OK\r\n");
	return;
}

static void smtp_turn(CONN *sid)
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
	mconn.sender_can_relay=allow_relay(sid);
	log_access("smtpd", "%s:%d - NEW CONNECTION", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort);
/*
	tcp_fprintf(&sid->socket, "220 %s - %s SMTPd%s\r\n", proc->config.hostname, SERVER_NAME, mconn.sender_can_relay?" - relaying allowed":" - relaying denied");
*/
	tcp_fprintf(&sid->socket, "220 %s - %s SMTPd\r\n", proc->config.hostname, SERVER_NAME);
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) return;
		striprn(line);
		if (strcasecmp(line, "quit")==0) {
			tcp_fprintf(&sid->socket, "221 Goodbye\r\n");
			return;
		} else if (strncasecmp(line, "helo", 4)==0) {
			ptemp=line+4;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(mconn.helo, sizeof(mconn.helo)-1, "%s", ptemp);
			tcp_fprintf(&sid->socket, "250 %s Hello, %s [%s]\r\n", proc->config.hostname, mconn.helo, sid->dat->user_RemoteAddr);
			log_access("smtpd", "%s:%d - HELO '%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn.helo);
			if (strlen(mconn.helo)) {
				sid->state=1;
				break;
			}
		} else if (strncasecmp(line, "ehlo", 4)==0) {
			ptemp=line+4;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(mconn.ehlo, sizeof(mconn.ehlo)-1, "%s", ptemp);
			tcp_fprintf(&sid->socket, "250 %s Hello, %s [%s]\r\n", proc->config.hostname, mconn.ehlo, sid->dat->user_RemoteAddr);
			log_access("smtpd", "%s:%d - EHLO '%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn.ehlo);
			if (strlen(mconn.ehlo)) {
				sid->state=1;
				break;
			}
		} else {
			tcp_fprintf(&sid->socket, "500 Some people still say hello\r\n");
		}
	} while (1);
	mconn.rcptalloc=50;
	if ((mconn.rcpt=(char **)calloc(mconn.rcptalloc, sizeof(char *)))==NULL) return;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) goto cleanup;
		striprn(line);
		if (strcasecmp(line, "quit")==0) {
			tcp_fprintf(&sid->socket, "221 Goodbye\r\n");
			break;

		} else if (strncasecmp(line, "helo", 4)==0) {
			ptemp=line+4;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(mconn.helo, sizeof(mconn.helo)-1, "%s", ptemp);
			tcp_fprintf(&sid->socket, "250 %s Hello, %s [%s]\r\n", proc->config.hostname, mconn.helo, sid->dat->user_RemoteAddr);
			log_access("smtpd", "%s:%d - HELO '%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn.helo);
		} else if (strncasecmp(line, "ehlo", 4)==0) {
			ptemp=line+4;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			snprintf(mconn.ehlo, sizeof(mconn.ehlo)-1, "%s", ptemp);
			tcp_fprintf(&sid->socket, "250 %s Hello, %s [%s]\r\n", proc->config.hostname, mconn.ehlo, sid->dat->user_RemoteAddr);
			log_access("smtpd", "%s:%d - EHLO '%s'", sid->dat->user_RemoteAddr, sid->dat->user_RemotePort, mconn.ehlo);

		} else if (strncasecmp(line, "mail from:", 10)==0) {
			smtp_from(sid, &mconn, line+10);
		} else if (strncasecmp(line, "rcpt to:", 8)==0) {
			smtp_rcpt(sid, &mconn, line+8);
		} else if (strcasecmp(line, "data")==0) {
			smtp_data(sid, &mconn);
		} else if (strcasecmp(line, "rset")==0) {
			smtp_rset(sid, &mconn);
		} else if (strcasecmp(line, "turn")==0) {
			smtp_turn(sid);
		} else if (strcasecmp(line, "noop")==0) {
			smtp_noop(sid);
		} else {
			log_error(MODSHORTNAME, __FILE__, __LINE__, 1, "UNKNOWN COMMAND: '%s'", line);
			tcp_fprintf(&sid->socket, "502 Unknown Command\r\n");
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
