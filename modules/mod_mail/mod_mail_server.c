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
#include "http_mod.h"
#include "mod_mail.h"

static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int wmprints(CONN *sid, const char *format, ...)
{
	char buffer[1024];
	va_list ap;
	int ob;

	sid->socket.atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	ob=tcp_send(&sid->dat->wm->socket, buffer, strlen(buffer), 0);
//	striprn(buffer);
//	logerror(sid, __FILE__, __LINE__, 4, "> %s", buffer);
	return ob;
}

int wmfgets(CONN *sid, char *buffer, int max, TCP_SOCKET *sock)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int pf=-1;
	short int rc;
	short int x;

	if (sid==NULL) return -1;
	if (sid->dat->wm==NULL) return -1;
	if (sid->dat->wm->socket.socket==-1) return -1;
	memset(buffer, 0, 2);
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) pf=0;
retry:
	if (!sid->dat->wm->socket.recvbufsize) {
		x=sizeof(sid->dat->wm->socket.recvbuf)-sid->dat->wm->socket.recvbufoffset-sid->dat->wm->socket.recvbufsize-2;
		obuffer=sid->dat->wm->socket.recvbuf+sid->dat->wm->socket.recvbufoffset+sid->dat->wm->socket.recvbufsize;
		if ((rc=tcp_recv(sock, obuffer, x, 0))<0) {
			msleep(1);
			prints(sid, "<BR>" MOD_MAIL_ERR_CONNRESET);
			shutdown(sid->dat->wm->socket.socket, 2);
			closesocket(sid->dat->wm->socket.socket);
			sid->dat->wm->socket.socket=-1;
			sid->dat->wm->connected=0;
			return -1;
		} else if (rc<1) {
			msleep(1);
		} else {
			sid->socket.atime=time(NULL);
		}
		sid->dat->wm->socket.recvbufsize+=rc;
		sid->dat->wm->imapmread+=rc;
	}
	if (time(NULL)-sid->socket.atime>config->http_maxidle) return -1;
	obuffer=sid->dat->wm->socket.recvbuf+sid->dat->wm->socket.recvbufoffset;
	if ((pf==0)&&(buffer[0]=='.')&&(buffer[1]=='.')) {
		pf=1;
		sid->dat->wm->socket.recvbufoffset++;
		sid->dat->wm->socket.recvbufsize--;
		obuffer++;
	}
	while ((n<max)&&(sid->dat->wm->socket.recvbufsize>0)) {
		sid->dat->wm->socket.recvbufoffset++;
		sid->dat->wm->socket.recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) return n;
	if (!lf) {
		if (sid->dat->wm->socket.recvbufsize>0) {
			memmove(sid->dat->wm->socket.recvbuf, sid->dat->wm->socket.recvbuf+sid->dat->wm->socket.recvbufoffset, sid->dat->wm->socket.recvbufsize);
			memset(sid->dat->wm->socket.recvbuf+sid->dat->wm->socket.recvbufsize, 0, sizeof(sid->dat->wm->socket.recvbuf)-sid->dat->wm->socket.recvbufsize);
			sid->dat->wm->socket.recvbufoffset=0;
		} else {
			memset(sid->dat->wm->socket.recvbuf, 0, sizeof(sid->dat->wm->socket.recvbuf));
			sid->dat->wm->socket.recvbufoffset=0;
			sid->dat->wm->socket.recvbufsize=0;
		}
		goto retry;
	}
	striprn(buffer);
	if ((pf==1)&&(buffer[0]=='.')&&(buffer[1]=='\0')) {
		buffer[1]=' ';
		buffer[2]='\0';
	}

//	logerror(sid, __FILE__, __LINE__, 4, "< %s", buffer);
	return n;
}

int wmffgets(CONN *sid, char *buffer, int max, FILE **fp)
{
	int n=0;

	if (*fp==NULL) {
		*buffer='\0';
		return 0;
	}
	if (fgets(buffer, max, *fp)==NULL) {
		fclose(*fp);
		*fp=NULL;
		*buffer='\0';
		return 0;
	}
	striprn(buffer);
	return n;
}

void wmclose(CONN *sid)
{
	/* shutdown(x,0=recv, 1=send, 2=both) */
	shutdown(sid->dat->wm->socket.socket, 2);
	closesocket(sid->dat->wm->socket.socket);
	sid->dat->wm->socket.socket=-1;
	sid->dat->wm->connected=0;
	return;
}

int wmserver_smtpconnect(CONN *sid)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char inbuffer[1024];

	if ((hp=gethostbyname(sid->dat->wm->smtpserver))==NULL) {
		prints(sid, "<BR>" MOD_MAIL_ERR_DNS_SMTP " '%s'\n", sid->dat->wm->smtpserver);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=(unsigned short)htons(sid->dat->wm->smtpport);
	if ((sid->dat->wm->socket.socket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
//	setsockopt(wmsocket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(sid->dat->wm->socket.socket, (struct sockaddr *)&server, sizeof(server))<0) {
		prints(sid, "<BR>" MOD_MAIL_ERR_CON_SMTP " '%s'\n", sid->dat->wm->smtpserver);
		return -1;
	}
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	return 0;
}

int wmserver_smtpauth(CONN *sid)
{
	char authstring[200];
	char inbuffer[1024];

	wmprints(sid, "AUTH PLAIN ");
	snprintf(authstring, sizeof(authstring)-1, "%s%c%s%c%s", sid->dat->wm->username, '\0', sid->dat->wm->username, '\0', sid->dat->wm->password);
	EncodeBase64(sid, authstring, (strlen(sid->dat->wm->username)*2)+strlen(sid->dat->wm->password)+2);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "502", 3)==0) return 0;
	if (strncasecmp(inbuffer, "235", 3)!=0) return -1;
	return 0;
}

void wmserver_smtpdisconnect(CONN *sid)
{
	char inbuffer[1024];

	wmprints(sid, "QUIT\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) break;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	wmclose(sid);
}

int wmserver_connect(CONN *sid, int verbose)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char inbuffer[1024];
	char timebuffer[100];
	char *ptemp;
	int sqr;

	if (sid->dat->wm->connected) return 0;
	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	if ((sqr=sql_queryf("SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, smtphost, smtpport, address, replyto, remove, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		if ((sqr=sql_queryf("SELECT mailaccountid FROM gw_mailaccounts where obj_uid = %d", sid->dat->user_uid))<0) return -1;
		if (sql_numtuples(sqr)<1) {
			sql_freeresult(sqr);
			return -1;
		}
		sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr, 0, 0));
		if (sql_updatef("UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", sid->dat->user_mailcurrent, sid->dat->user_username)<0) return -1;
		sql_freeresult(sqr);
		if ((sqr=sql_queryf("SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, smtphost, smtpport, address, replyto, remove, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return -1;
	}
	if (sql_numtuples(sqr)==1) {
		strncpy(sid->dat->wm->realname,     sql_getvalue(sqr, 0, 0), sizeof(sid->dat->wm->realname)-1);
		strncpy(sid->dat->wm->organization, sql_getvalue(sqr, 0, 1), sizeof(sid->dat->wm->organization)-1);
		strncpy(sid->dat->wm->username,     sql_getvalue(sqr, 0, 2), sizeof(sid->dat->wm->username)-1);
		strncpy(sid->dat->wm->password,     DecodeBase64string(sid, sql_getvalue(sqr, 0, 3)), sizeof(sid->dat->wm->password)-1);
		strncpy(sid->dat->wm->smtpauth,     sql_getvalue(sqr, 0, 4), sizeof(sid->dat->wm->smtpauth)-1);
		strncpy(sid->dat->wm->servertype,   sql_getvalue(sqr, 0, 5), sizeof(sid->dat->wm->servertype)-1);
		strncpy(sid->dat->wm->popserver,    sql_getvalue(sqr, 0, 6), sizeof(sid->dat->wm->popserver)-1);
		sid->dat->wm->popport=atoi(sql_getvalue(sqr, 0, 7));
		strncpy(sid->dat->wm->smtpserver,   sql_getvalue(sqr, 0, 8), sizeof(sid->dat->wm->smtpserver)-1);
		sid->dat->wm->smtpport=atoi(sql_getvalue(sqr, 0, 9));
		strncpy(sid->dat->wm->address,      sql_getvalue(sqr, 0, 10), sizeof(sid->dat->wm->address)-1);
		strncpy(sid->dat->wm->replyto,      sql_getvalue(sqr, 0, 11), sizeof(sid->dat->wm->replyto)-1);
		sid->dat->wm->remove=atoi(sql_getvalue(sqr, 0, 12));
		strncpy(sid->dat->wm->signature,    sql_getvalue(sqr, 0, 13), sizeof(sid->dat->wm->signature)-1);
	}
	sql_freeresult(sqr);
	if (strcmp(sid->dat->in_RequestMethod,"POST")==0) {
		if ((ptemp=getpostenv(sid, "WMUSERNAME"))!=NULL) {
			strncpy(sid->dat->wm->username, ptemp, sizeof(sid->dat->wm->username)-1);
		}
		if ((ptemp=getpostenv(sid, "WMPASSWORD"))!=NULL) {
			strncpy(sid->dat->wm->password, ptemp, sizeof(sid->dat->wm->password)-1);
		}
	}
	if ((strlen(sid->dat->wm->username)==0)||(strlen(sid->dat->wm->password)==0)||(strlen(sid->dat->wm->popserver)==0)||(strlen(sid->dat->wm->smtpserver)==0)) {
		if (verbose) {
			wmloginform(sid);
		}
		return -1;
	}
	if ((hp=gethostbyname(sid->dat->wm->popserver))==NULL) {
		if (verbose) {
			prints(sid, "<BR>" MOD_MAIL_ERR_DNS_POP3 " '%s'\n", sid->dat->wm->popserver);
		}
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=htons(sid->dat->wm->popport);
	if ((sid->dat->wm->socket.socket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
//	setsockopt(sid->dat->wm->socket.socket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(sid->dat->wm->socket.socket, (struct sockaddr *)&server, sizeof(server))<0) {
		if (verbose) {
			prints(sid, "<BR>" MOD_MAIL_ERR_CON_POP3 " '%s'\n", sid->dat->wm->popserver);
		}
		return -1;
	}
	sid->dat->wm->connected=1;
	/* Check current status */
	if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		/* Send user name */
		wmprints(sid, "USER %s\r\n", sid->dat->wm->username);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) {
			if (verbose) {
				ptemp=strchr(inbuffer, ' ');
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			}
			wmprints(sid, "QUIT\r\n");
			wmclose(sid);
			if (verbose) {
				wmloginform(sid);
			}
			return -1;
		}
		/* Send password */
		wmprints(sid, "PASS %s\r\n", sid->dat->wm->password);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) {
			if (verbose) {
				ptemp=strchr(inbuffer, ' ');
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			}
			wmprints(sid, "QUIT\r\n");
			wmclose(sid);
			if (verbose) {
				wmloginform(sid);
			}
			return -1;
		}
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		/* Send user name & password */
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d LOGIN %s %s\r\n", sid->dat->wm->imapidx, sid->dat->wm->username, sid->dat->wm->password);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
		}
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		if (strncasecmp(ptemp, "OK", 2)!=0) {
			if (verbose) {
				ptemp=strchr(inbuffer, ' ');
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			}
			wmclose(sid);
			if (verbose) {
				wmloginform(sid);
			}
			return -1;
		}
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d SELECT INBOX\r\n", sid->dat->wm->imapidx);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
		}
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		if (strncasecmp(ptemp, "OK", 2)!=0) {
			if (verbose) {
				ptemp=strchr(inbuffer, ' ');
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			}
			wmclose(sid);
			if (verbose) {
				wmloginform(sid);
			}
			return -1;
		}
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")==0) {
		time_unix2sql(timebuffer, sizeof(timebuffer)-1, time(NULL));
		sql_updatef("UPDATE gw_mailaccounts SET obj_mtime = '%s', popusername = '%s', poppassword = '%s' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->wm->username, EncodeBase64string(sid, sid->dat->wm->password), sid->dat->user_uid, sid->dat->user_mailcurrent);
	}
	return 0;
}

void wmserver_disconnect(CONN *sid)
{
	char inbuffer[1024];

	if (sid<0) return;
	if (!sid->dat->wm->connected) return;
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "QUIT\r\n");
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket);
		wmclose(sid);
		sid->dat->wm->connected=0;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d LOGOUT\r\n", sid->dat->wm->imapidx);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) break;
			if (isdigit(inbuffer[0])) break;
		}
		wmclose(sid);
		sid->dat->wm->connected=0;
	}
	return;
}

int wmserver_count(CONN *sid)
{
	char inbuffer[1024];
	char *ptemp;
	int count=-1;

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "STAT\r\n");
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		return atoi(ptemp);
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d STATUS INBOX (MESSAGES)\r\n", sid->dat->wm->imapidx);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
			if (strncasecmp(inbuffer, "* STATUS", 8)==0) {
				ptemp=inbuffer;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				count=atoi(ptemp);
			}
		}
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		if (strncasecmp(ptemp, "OK", 2)!=0) return -1;
		return count;
	}
	return -1;
}

int wmserver_msgdele(CONN *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "DELE %d\r\n", message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d STORE %d +FLAGS (\\Deleted)\r\n", sid->dat->wm->imapidx, message);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
		}
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d EXPUNGE\r\n", sid->dat->wm->imapidx);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
		}
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		if (strncasecmp(ptemp, "OK", 2)!=0) {
			ptemp=strchr(inbuffer, ' ');
			if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			wmclose(sid);
			wmloginform(sid);
			return -1;
		}
		return 0;
	}
	return -1;
}

int wmserver_msghead(CONN *sid, int message)
{
	char inbuffer[1024];

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "TOP %d 0\r\n", message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH %d BODY[HEADER]\r\n", sid->dat->wm->imapidx, message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket);
		return 0;
	}
	return -1;
}

int wmserver_msgretr(CONN *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "RETR %d\r\n", message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH %d RFC822\r\n", sid->dat->wm->imapidx, message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		ptemp=strchr(inbuffer, '{');
		if (*ptemp) ptemp++;
		if (*ptemp) {
			sid->dat->wm->imapmsize=atoi(ptemp);
		} else {
			sid->dat->wm->imapmsize=-1;
		}
		sid->dat->wm->imapmread=0;
		return 0;
	}
	return -1;
}

int wmserver_msgsize(CONN *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;
	int size=-1;

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "LIST %d\r\n", message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		return atoi(ptemp);
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH %d RFC822.SIZE\r\n", sid->dat->wm->imapidx, message);
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (isdigit(inbuffer[0])) break;
			if (p_strcasestr(inbuffer, "FETCH")!=NULL) {
				ptemp=inbuffer;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				while ((ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp) ptemp++;
				size=atoi(ptemp);
			}
		}
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		if (strncasecmp(ptemp, "OK", 2)!=0) {
			ptemp=strchr(inbuffer, ' ');
			if (ptemp) prints(sid, "<CENTER>%s</CENTER>\n", ptemp);
			wmclose(sid);
			wmloginform(sid);
			return -1;
		}
		return size;
	}
	return -1;
}

int wmserver_uidl(CONN *sid, int message, char *uidl)
{
	char uidltemp[71];
	unsigned char a, b, c, d;
	int dst, i, enclen, remlen;
	char inbuffer[1024];
	char *ptemp;

	memset(uidltemp, 0, sizeof(uidltemp));
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "UIDL %d\r\n", message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		snprintf(uidltemp, 70, "%s", ptemp);
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH %d UID\r\n", sid->dat->wm->imapidx, message);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		if (*ptemp) ptemp++;
		snprintf(uidltemp, 70, "%s", ptemp);
		ptemp=uidltemp;
		while ((ptemp)&&(*ptemp!=')')) ptemp++;
		if (*ptemp) *ptemp='\0';
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket);
	}
	ptemp=uidltemp;
	dst=0;
	enclen=strlen(uidltemp)/3;
	remlen=strlen(uidltemp)-3*enclen;
	for (i=0;i<enclen;i++) {
		a=(ptemp[0]>>2);
	        b=(ptemp[0]<<4)&0x30;
		b|=(ptemp[1]>>4);
		c=(ptemp[1]<<2)&0x3c;
		c|=(ptemp[2]>>6);
		d=ptemp[2]&0x3f;
		ptemp+=3;
		uidl[dst++]=Base64[a];
		uidl[dst++]=Base64[b];
		uidl[dst++]=Base64[c];
		uidl[dst++]=Base64[d];
	}
	if (remlen==1) {
		a=(ptemp[0]>>2);
		b=(ptemp[0]<<4)&0x30;
		uidl[dst++]=Base64[a];
		uidl[dst]=Base64[b];
	} else if (remlen==2) {
		a=(ptemp[0]>>2);
		b=(ptemp[0]<<4)&0x30;
		b|=(ptemp[1]>>4);
		c=(ptemp[1]<<2)&0x3c;
		uidl[dst++]=Base64[a];
		uidl[dst++]=Base64[b];
		uidl[dst]=Base64[c];
	}
	return 0;
}

int is_msg_end(CONN *sid, char *buffer)
{
	char inbuffer[1024];

	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		if (strcmp(buffer, ".")==0) return 1;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		if ((sid->dat->wm->imapmsize>0)&&(sid->dat->wm->imapmread<sid->dat->wm->imapmsize)) return 0;
		if (strcmp(buffer, ")")==0) {
			for (;;) {
				if (isdigit(inbuffer[0])) {
					if (atoi(inbuffer)==sid->dat->wm->imapidx) break;
				}
				if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return 1;
			}
			return 1;
		}
	}
	return 0;
}

int wmserver_msgsync(CONN *sid, int remoteid, int localid, int verbose)
{
	char inbuffer[1024];
	char msgfilename[512];
	char *ptemp;
	FILE *fp;
	struct stat sb;
	int sqr;

	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d", config->server_dir_var_mail, sid->dat->user_mailcurrent);
	fixslashes(msgfilename);
	if ((stat(msgfilename, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(msgfilename)!=0) {
#else
		if (mkdir(msgfilename, 0700)!=0) {
#endif
			log_error("mod_mail", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", msgfilename);
			return -1;
		}
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, localid);
	fixslashes(msgfilename);
	if ((sqr=sql_queryf("SELECT mailheaderid, size FROM gw_mailheaders where accountid = %d and obj_uid = %d and mailheaderid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid, localid))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -1;
	}
	if (stat(msgfilename, &sb)==0) {
		if (atoi(sql_getvalue(sqr, 0, 1))==sb.st_size) {
			sql_freeresult(sqr);
			return 0;
		}
	}
	sql_freeresult(sqr);
	if (verbose) {
		prints(sid, ".");
		flushbuffer(sid);
		verbose=0;
	}
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "RETR %d\r\n", remoteid);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		goto sync;
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH %d RFC822\r\n", sid->dat->wm->imapidx, remoteid);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		ptemp=strchr(inbuffer, '{');
		if (*ptemp) ptemp++;
		if (*ptemp) {
			sid->dat->wm->imapmsize=atoi(ptemp);
		} else {
			sid->dat->wm->imapmsize=-1;
		}
		sid->dat->wm->imapmread=0;
		goto sync;
	}
	return -1;
sync:
	fp=fopen(msgfilename, "wb");
	if (fp==NULL) {
		log_error("mod_mail", __FILE__, __LINE__, 1, "ERROR: Could not open message '%s'!", msgfilename);
		return -1;
	} else {
		for (;;) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {
				fclose(fp);
				goto err;
			}
			if (is_msg_end(sid, inbuffer)) break;
			fprintf(fp, "%s\r\n", inbuffer);
		}
		fclose(fp);
		if ((stat(msgfilename, &sb)!=0)||(sb.st_mode&S_IFDIR)) return 0;
		sql_updatef("UPDATE gw_mailheaders SET size = %d WHERE mailheaderid = %d AND accountid = %d", (int)sb.st_size, localid, sid->dat->user_mailcurrent);
	}
err:
	if (sid->dat->wm->remove==1) {
		if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
			wmprints(sid, "NOOP\r\n");
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
			if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
			sid->dat->wm->imapidx++;
			wmprints(sid, "%d NOOP\r\n", sid->dat->wm->imapidx);
			for (;;) {
				if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) break;
				if (isdigit(inbuffer[0])) break;
			}
			ptemp=inbuffer;
			while ((ptemp)&&(*ptemp!=' ')) ptemp++;
			if (*ptemp) ptemp++;
			if (strncasecmp(ptemp, "OK", 2)!=0) return -1;
		}
		wmserver_msgdele(sid, remoteid);
	}
	return 0;
}

int wmserver_mlistsync(CONN *sid, char ***uidl_list)
{
	wmheader header;
	char lockfile[512];
#ifdef WIN32
	FILE *fp;
#else
	int fp;
#endif
	struct stat sb;
	char curdate[40];
	char inbuffer[1024];
	char query[8192];
	char uidltemp[71];
	char **uidls=NULL;
	char *ptemp;
	int i, j;
	int found;
	int nummessages;
	int sqr1=-1;
	int sqr2=-1;
	int headerid;
	int dstmbox;
	time_t t;

	DEBUG_IN(sid, "wmserver_mlistsync()");
	if (sid==NULL) {
		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	if (sid->dat->user_mailcurrent==0) {
		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	memset(lockfile, 0, sizeof(lockfile));
	snprintf(lockfile, sizeof(lockfile)-1, "%s/%04d", config->server_dir_var_mail, sid->dat->user_mailcurrent);
	fixslashes(lockfile);
	if ((stat(lockfile, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(lockfile)!=0) {
#else
		if (mkdir(lockfile, 0700)!=0) {
#endif
			log_error("mod_mail", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", lockfile);
			return -1;
		}
	}
	memset(lockfile, 0, sizeof(lockfile));
	snprintf(lockfile, sizeof(lockfile)-1, "%s/%04d/mbox.lock", config->server_dir_var_mail, sid->dat->user_mailcurrent);
	fixslashes(lockfile);
#ifdef WIN32
	if ((fp=_fsopen(lockfile, "wb", _SH_DENYWR))==NULL) return -2;
	fprintf(fp, "This mailbox is LOCKED!\r\n");
#else
	unlink(lockfile);
	if ((fp=open(lockfile, O_RDWR|O_CREAT|O_TRUNC))<0) return -2;
	if (flock(fp, LOCK_EX)!=0) return -2;
	write(fp, "This mailbox is LOCKED!\r\n", 25);
#endif
	if (wmserver_connect(sid, 1)!=0) {
		nummessages=-1;
		goto cleanup;
	}
	nummessages=wmserver_count(sid);
	if (nummessages<1) {
		nummessages=0;
		goto cleanup;
	}
	uidls=(char **)calloc(nummessages, sizeof(char *));
	*uidl_list=uidls;
	for (i=0;i<nummessages;i++) uidls[i]=calloc(120, sizeof(char));
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "UIDL\r\n");
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {
			nummessages=-1;
			goto cleanup;
		}
		if (strncasecmp(inbuffer, "+OK", 3)!=0) {
			nummessages=-1;
			goto cleanup;
		}
		for (i=0;;i++) {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {;
				nummessages=-1;
				goto cleanup;
			}
			if (strcmp(inbuffer, ".")==0) break;
			if (atoi(inbuffer)<1) break;
			if (i<nummessages) {
				ptemp=inbuffer;
				while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
				if (*ptemp==' ') ptemp++;
				snprintf((char *)uidls[i], 119, "%s", EncodeBase64string(sid, ptemp));
				ptemp=(char *)uidls[i];
				while (1) {
					j=strlen(ptemp)-1;
					if (j<0) break;
					if (ptemp[j]=='=') { ptemp[j]='\0'; continue; }
					break;
				};
			}
		}
	} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
		sid->dat->wm->imapidx++;
		wmprints(sid, "%d FETCH 1:* UID\r\n", sid->dat->wm->imapidx);
		for (;;) {
			memset(uidltemp, 0, sizeof(uidltemp));
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {;
				nummessages=-1;
				goto cleanup;
			}
			if (isdigit(inbuffer[0])) break;
			ptemp=inbuffer;
			while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
			if (*ptemp) ptemp++;
			i=atoi(ptemp)-1;
			while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
			if (*ptemp) ptemp++;
			while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
			if (*ptemp) ptemp++;
			while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
			if (*ptemp) ptemp++;
			snprintf(uidltemp, 70, "%s", ptemp);
			ptemp=uidltemp;
			while ((ptemp)&&(*ptemp!=')')) ptemp++;
			if (*ptemp) *ptemp='\0';
			if ((i<nummessages)&&(i>=0)) {
				snprintf((char *)uidls[i], 119, "%s", EncodeBase64string(sid, uidltemp));
				ptemp=(char *)uidls[i];
				while (1) {
					j=strlen(ptemp)-1;
					if (j<0) break;
					if (ptemp[j]=='=') { ptemp[j]='\0'; continue; }
					break;
				};
			}
		}
	} else {
		nummessages=-1;
		goto cleanup;
	}
	if ((sqr1=sql_queryf("SELECT mailheaderid, uidl FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) {
		nummessages=-1;
		goto cleanup;
	}
	for (i=0;i<nummessages;i++) {
		found=-1;
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (strcmp((char *)uidls[i], sql_getvalue(sqr1, j, 1))==0) {
				found=j;
				break;
			}
		}
		if (found>-1) continue;
		if ((sqr2=sql_queryf("SELECT max(mailheaderid) FROM gw_mailheaders where accountid = %d", sid->dat->user_mailcurrent))<0) {
			nummessages=-1;
			goto cleanup;
		}
		headerid=atoi(sql_getvalue(sqr2, 0, 0))+1;
		sql_freeresult(sqr2);
		sqr2=-1;
		if (headerid<1) headerid=1;
		wmserver_msghead(sid, i+1);
		memset((char *)&header, 0, sizeof(header));
		if (webmailheader(sid, &header)!=0) {
			nummessages=-1;
			goto cleanup;
		}
		dstmbox=1;
		if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
			for (;;) {
				if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {
					nummessages=-1;
					goto cleanup;
				}
				if (strcmp(inbuffer, ".")==0) break;
			}
		} else if (strncasecmp(sid->dat->wm->servertype, "imap", 4)==0) {
			for (;;) {
				if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {
					nummessages=-1;
					goto cleanup;
				}
				if (isdigit(inbuffer[0])) break;
			}
		}
		t=time(NULL);
		strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
		memset(query, 0, sizeof(query));
		strcpy(query, "INSERT INTO gw_mailheaders (mailheaderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_messageid, hdr_inreplyto, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", headerid, curdate, curdate, sid->dat->user_uid, 0, 0, 0);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d", sid->dat->user_mailcurrent);
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%d", dstmbox);
		strncatf(query, sizeof(query)-strlen(query)-1, "', 'n");
		strncatf(query, sizeof(query)-strlen(query)-1, "', '0");
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, (char *)uidls[i]));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.From));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.ReplyTo));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.To));
		t=time_wmgetdate(header.Date);
		strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, curdate));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.MessageID));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.InReplyTo));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.Subject));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.CC));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.contenttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.boundary));
		strncatf(query, sizeof(query)-strlen(query)-1, "', '%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.encoding));
		if (sql_update(query)<0) {
			nummessages=-1;
			goto cleanup;
		}
		prints(sid, ".");
		wmserver_msgsync(sid, i+1, headerid, 0);
		dstmbox=wmfilter_apply(sid, &header, sid->dat->user_mailcurrent, headerid);
		if (dstmbox<1) dstmbox=1;
		flushbuffer(sid);
	}
	sql_freeresult(sqr1);
	sqr1=-1;
cleanup:
#ifdef WIN32
	fclose(fp);
#else
	flock(fp, LOCK_UN);
	close(fp);
#endif
	unlink(lockfile);
	if (sqr1>=0) sql_freeresult(sqr1);
	if (sqr2>=0) sql_freeresult(sqr2);
	DEBUG_OUT(sid, "wmserver_mlistsync()");
	return nummessages;
}

void wmserver_purge(CONN *sid, int remove)
{
	char **uidls;
	int numlocal;
	int numremote;
	int i, j;
	int sqr;

	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(WEBMAIL)))==NULL) return;
	}
	numremote=wmserver_mlistsync(sid, &uidls);
	if (numremote==-2) prints(sid, "<BR><B>" MOD_MAIL_ERR_BOX_LOCK "</B>\n");
	if (numremote<0) {
		wmserver_disconnect(sid);
		if (sid->dat->wm!=NULL) {
			free(sid->dat->wm);
			sid->dat->wm=NULL;
		}
		return;
	}
	if ((sid->dat->wm->remove<1)||(numremote<1)) {
		wmserver_disconnect(sid);
		return;
	}
	if (sid->dat->wm->remove==1) {
		if ((sqr=sql_queryf("SELECT mailheaderid, uidl FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d ORDER BY hdr_date DESC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	} else if (sid->dat->wm->remove==2) {
		if ((sqr=sql_queryf("SELECT mailheaderid, uidl FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND status = 'd' ORDER BY hdr_date DESC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	} else {
		goto cleanup;
	}
	numlocal=sql_numtuples(sqr);
	for (i=numlocal-1;i>-1;i--) {
		for (j=numremote-1;j>-1;j--) {
			if (strlen(sql_getvalue(sqr, i, 1))<1) continue;
			if (strcmp(sql_getvalue(sqr, i, 1), uidls[j])==0) {
				wmserver_msgdele(sid, j+1);
				break;
			}
		}
	}
	sql_freeresult(sqr);
cleanup:
	wmserver_disconnect(sid);
	for (i=0;i<numremote;i++) free(uidls[i]);
	free(uidls);
	return;
}

int wmserver_send(CONN *sid, int mailid, int verbose)
{
	wmheader header;
	FILE *fp;
	char *ptemp;
	char inbuffer[1024];
	char outbuffer[1024];
	char msgfilename[512];
	char msgaddr[128];
	int outlen;
	int i;
	int sqr;

	memset((char *)&header, 0, sizeof(header));
	memset(outbuffer, 0, sizeof(outbuffer));
	if ((sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' and folder = '2' and mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, mailid))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		if (verbose) prints(sid, "<BR>" MOD_MAIL_ERR_NOMSG);
		return -1;
	}
	dbread_getheader(sid, sqr, 0, &header);
	sql_freeresult(sqr);
	if (wmserver_smtpconnect(sid)!=0) return -1;
	snprintf(outbuffer, sizeof(outbuffer)-1, "HELO %s\r\n", sid->dat->wm->smtpserver);
	wmprints(sid, "%s", outbuffer);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) goto quit;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	if (sid->dat->wm->smtpauth[0]=='y') if (wmserver_smtpauth(sid)!=0) goto quit;
	snprintf(outbuffer, sizeof(outbuffer)-1, "MAIL From: <%s>\r\n", sid->dat->wm->address);
	wmprints(sid, "%s", outbuffer);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) goto quit;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	for (i=0;i<3;i++) {
		switch (i) {
			case 1: ptemp=header.CC; break;
			case 2: ptemp=header.BCC; break;
			default: ptemp=header.To; break;
		}
		while (*ptemp) {
			if (strstr(ptemp, "@")==NULL) break;
			memset(msgaddr, 0, sizeof(msgaddr));
			if (*ptemp==',') ptemp++;
			while ((*ptemp)&&(*ptemp!=',')) {
				if (strlen(msgaddr)>sizeof(msgaddr)-1) break;
				if (*ptemp=='\"') {
					ptemp++;
					while ((*ptemp)&&(*ptemp!='\"')) ptemp++;
					if (*ptemp=='\"') ptemp++;
				} else if (*ptemp==' ') {
					ptemp++;
				} else if (*ptemp=='<') {
					memset(msgaddr, 0, sizeof(msgaddr));
					ptemp++;
					do {
						if (strlen(msgaddr)<sizeof(msgaddr)-1) {
							msgaddr[strlen(msgaddr)]=*ptemp;
						}
						ptemp++;
					} while ((*ptemp)&&(*ptemp!='>'));
					if (*ptemp=='>') ptemp++;
				} else {
					if (strlen(msgaddr)<sizeof(msgaddr)-1) {
						msgaddr[strlen(msgaddr)]=*ptemp;
					}
					ptemp++;
				}
			}
			snprintf(outbuffer, sizeof(outbuffer)-1, "RCPT To: <%s>\r\n", msgaddr);
			wmprints(sid, "%s", outbuffer);
			do {
				memset(inbuffer, 0, sizeof(inbuffer));
				if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) goto quit;
			} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
			if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
		}
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, mailid);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "r");
	if (fp==NULL) {
		if (verbose) prints(sid, "<BR><B>Message %d is missing!</B>\r\n", mailid);
		wmserver_smtpdisconnect(sid);
		return -1;
	}
	snprintf(outbuffer, sizeof(outbuffer)-1, "DATA\r\n");
	wmprints(sid, "%s", outbuffer);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) goto quit;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "354", 3)!=0) goto quit;
	memset(outbuffer, 0, sizeof(outbuffer));
	outlen=0;
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (fp==NULL) break;
		striprn(inbuffer);
		if (strlen(inbuffer)+outlen>=sizeof(outbuffer)-3) {
			if (wmprints(sid, "%s", outbuffer)<0) {
				if (fp!=NULL) { fclose(fp); fp=NULL; }
			}
			memset(outbuffer, 0, sizeof(outbuffer));
			outlen=0;
		}
		strcat(outbuffer, inbuffer);
		strcat(outbuffer, "\r\n");
		outlen+=strlen(inbuffer)+2;
	}
	wmprints(sid, "%s", outbuffer);

	snprintf(outbuffer, sizeof(outbuffer)-1, "\r\n.\r\n");
	wmprints(sid, "%s", outbuffer);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) break;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)==0) {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (verbose) prints(sid, "<BR><B>" MOD_MAIL_SENT "</B>\n", mailid);
		flushbuffer(sid);
		wmserver_smtpdisconnect(sid);
		sql_updatef("UPDATE gw_mailheaders SET folder = '3' WHERE mailheaderid = %d AND accountid = %d", mailid, sid->dat->user_mailcurrent);
		return 0;
	}
quit:
	if (verbose) prints(sid, "<BR><B>ERROR: %s</B>\r\n", str2html(sid, outbuffer));
	if (verbose) prints(sid, "<BR><B>ERROR: %s</B>\r\n", str2html(sid, inbuffer));
	flushbuffer(sid);
	wmserver_smtpdisconnect(sid);
	return -1;
}

int wmsync(CONN *sid, int verbose)
{
	char **uidls=NULL;
	int numremote;
	int numlocal;
	int i, j;
	int sqr;
	int smtperror=0;

	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(WEBMAIL)))==NULL) return -1;
	}
	if (verbose) {
		prints(sid, "<BR><B><NOBR>" MOD_MAIL_CHECKING "</NOBR> ");
		flushbuffer(sid);
	}
	numremote=wmserver_mlistsync(sid, &uidls);
	if ((verbose)&&(numremote==-2)) prints(sid, "<BR><B>" MOD_MAIL_ERR_BOX_LOCK "</B>\n");
	if (numremote<0) {
		wmserver_disconnect(sid);
		if (sid->dat->wm!=NULL) {
			free(sid->dat->wm);
			sid->dat->wm=NULL;
		}
		return numremote;
	}
	sql_updatef("UPDATE gw_mailaccounts SET lastcount = %d, lastcheck = '%s' WHERE obj_uid = %d AND mailaccountid = %d", numremote, time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, time(NULL)), sid->dat->user_uid, sid->dat->user_mailcurrent);
	if (numremote<1) goto cleanup;
	if ((sqr=sql_queryf("SELECT mailheaderid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' ORDER BY hdr_date ASC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	numlocal=sql_numtuples(sqr);
	for (i=0;i<numlocal;i++) {
		for (j=0;j<numremote;j++) {
			if (strcmp(uidls[j], sql_getvalue(sqr, i, 4))==0) {
				wmserver_msgsync(sid, j+1, atoi(sql_getvalue(sqr, i, 0)), verbose);
				break;
			}
		}
	}
	sql_freeresult(sqr);
cleanup:
	if (verbose) {
		prints(sid, " " MOD_MAIL_OK ".</B>\r\n");
		flushbuffer(sid);
	}
	wmserver_disconnect(sid);
	for (i=0;i<numremote;i++) free(uidls[i]);
	if (uidls!=NULL) free(uidls);
	if (verbose) {
		if ((sqr=sql_queryf("SELECT mailheaderid FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' and folder = '2'", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) return -1;
		for (i=0;i<sql_numtuples(sqr);i++) {
			if (wmserver_send(sid, atoi(sql_getvalue(sqr, i, 0)), verbose)!=0) smtperror=1;
		}
		sql_freeresult(sqr);
		if (!smtperror) {
			if (sid->dat->user_menustyle>0) {
				prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/main\">\r\n", sid->dat->in_ScriptName);
			} else {
				prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/list\">\r\n", sid->dat->in_ScriptName);
			}
		}
	}
	if (sid->dat->wm!=NULL) {
		free(sid->dat->wm);
		sid->dat->wm=NULL;
	}
	return numremote;
}
