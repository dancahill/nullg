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
#include "mod_email.h"

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
/*
	striprn(buffer);
	logerror(sid, __FILE__, __LINE__, 4, "> %s", buffer);
*/
	if (sid->dat->wm->showdebug) {
		prints(sid, "<FONT COLOR=green>%s</FONT><BR />\r\n", buffer);
		flushbuffer(sid);
	}
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
			prints(sid, "<BR />%s", lang_gets(sid, "mod_email", "mail_err_connreset"));
			tcp_close(&sid->dat->wm->socket, 1);
			sid->dat->wm->connected=0;
			return -1;
		} else if (rc>0) {
			sid->socket.atime=time(NULL);
			sid->dat->wm->socket.recvbufsize+=rc;
			sid->dat->wm->imapmread+=rc;
		}
	}
	if (time(NULL)-sid->socket.atime>mod_config.sock_maxidle) return -1;
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
	if (n>max-1) {
		if (sid->dat->wm->showdebug) {
			prints(sid, "<FONT COLOR=blue>%s</FONT><BR />\r\n", buffer);
			flushbuffer(sid);
		}
		return n;
	}
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
/*
	logerror(sid, __FILE__, __LINE__, 4, "< %s", buffer);
*/
	if (sid->dat->wm->showdebug) {
		prints(sid, "<FONT COLOR=blue>%s</FONT><BR />\r\n", buffer);
		flushbuffer(sid);
	}
	return n;
}

int wmffgets(char *buffer, int max, FILE **fp)
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

static void wmclose(CONN *sid)
{
	tcp_close(&sid->dat->wm->socket, 1);
	sid->dat->wm->connected=0;
	return;
}

static int wmserver_smtpconnect(CONN *sid)
{
	char inbuffer[1024];
	int rc;

	rc=tcp_connect(&sid->dat->wm->socket, sid->dat->wm->smtpserver, sid->dat->wm->smtpport, sid->dat->wm->smtpssl);
	if (rc==-1) {
		prints(sid, "<BR />%s '%s'\r\n", lang_gets(sid, "mod_email", "mail_err_dns_smtp"), sid->dat->wm->smtpserver);
		return -1;
	} else if (rc==-2) {
		prints(sid, "<BR />%s '%s'\r\n", lang_gets(sid, "mod_email", "mail_err_con_smtp"), sid->dat->wm->smtpserver);
		return -1;
	}
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	return 0;
}

static int wmserver_smtpauth(CONN *sid)
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

static void wmserver_smtpdisconnect(CONN *sid)
{
	char inbuffer[1024];

	wmprints(sid, "QUIT\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) break;
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	wmclose(sid);
}

static int wmserver_connect(CONN *sid, int verbose)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char inbuffer[1024];
	char timebuffer[100];
	char *ptemp;
	int rc;
	int accountid=(int)nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");

	if (sid->dat->wm->connected) return 0;
	if (!(auth_priv(sid, "email")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return -1;
	}
	if (dbread_mailcurrent(sid, accountid)<0) return -1;
	if (verbose==0) sid->dat->wm->showdebug=0;
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")==0) {
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
	rc=tcp_connect(&sid->dat->wm->socket, sid->dat->wm->popserver, sid->dat->wm->popport, sid->dat->wm->popssl);
	if (rc==-1) {
		prints(sid, "<BR />%s '%s'\r\n", lang_gets(sid, "mod_email", "mail_err_dns_pop3"), sid->dat->wm->popserver);
		return -1;
	} else if (rc==-2) {
		prints(sid, "<BR />%s '%s'\r\n", lang_gets(sid, "mod_email", "mail_err_con_pop3"), sid->dat->wm->popserver);
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
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
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
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
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
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
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
				if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
			}
			wmclose(sid);
			if (verbose) {
				wmloginform(sid);
			}
			return -1;
		}
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")==0) {
		time_unix2sql(timebuffer, sizeof(timebuffer)-1, time(NULL));
		sql_updatef(proc->N, "UPDATE gw_email_accounts SET obj_mtime = '%s', popusername = '%s', poppassword = '%s' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->wm->username, EncodeBase64string(sid, sid->dat->wm->password), sid->dat->uid, accountid);
	}
	return 0;
}

static void wmserver_disconnect(CONN *sid)
{
	char inbuffer[1024];

	if (sid==NULL) return;
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

static int wmserver_count(CONN *sid)
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

static int wmserver_msgdele(CONN *sid, int message)
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
			if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
			wmclose(sid);
			wmloginform(sid);
			return -1;
		}
		return 0;
	}
	return -1;
}
/*
static int wmserver_msghead(CONN *sid, int message)
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

static int wmserver_msgretr(CONN *sid, int message)
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

static int wmserver_msgsize(CONN *sid, int message)
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
			if (ptemp) prints(sid, "<CENTER>%s</CENTER>\r\n", ptemp);
			wmclose(sid);
			wmloginform(sid);
			return -1;
		}
		return size;
	}
	return -1;
}

static int wmserver_uidl(CONN *sid, int message, char *uidl)
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
*/
static int is_msg_end(CONN *sid, char *buffer)
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

static int wmserver_msgbodysync(CONN *sid, char *fname, int localid, int accountid, char *uidl)
{
	char inbuffer[1024];
	char curdate[40];
	obj_t *hobj=NULL;
	obj_t *qobj=NULL, *cobj, *tobj;
	struct stat sb;
	FILE *fp;
	int id;
	int rc=0;
	int dstmbox;
	int i;
	time_t t;

	if ((fp=fopen(fname, "wb"))==NULL) {
		log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Could not open message '%s'!", fname);
		return -1;
	}
	for (;;) {
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) {
			prints(sid, "[network error]\r\n");
			fprintf(fp, "[network error]\r\n");
			fclose(fp);
			/* unlink(fname); */
			return -2;
		}
		if (is_msg_end(sid, inbuffer)) break;
		fprintf(fp, "%s\r\n", inbuffer);
	}
	fclose(fp);
	if ((fp=fopen(fname, "r"))!=NULL) {
		webmailheader(sid, &hobj, &fp);
		fclose(fp);
	}
	if ((stat(fname, &sb)!=0)||(!nes_istable(hobj))) { rc=-3; goto cleanup; }
	if (dbread_mailheader(sid, 2, 0, &qobj)<0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		rc=-4; goto cleanup;
	}
	tobj=nes_getiobj(proc->N, nes_settable(proc->N, qobj, "_rows"), 0);
	nes_setnum(sid->N, tobj, "pid", accountid);
	nes_setstr(sid->N, tobj, "name", uidl, -1);
	tobj=nes_settable(proc->N, tobj, "_data");
	nes_setnum(sid->N, tobj, "accountid",       accountid);
	nes_setnum(sid->N, tobj, "folderid",        0);
	cobj=nes_getobj(sid->N, hobj, "from");                      if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_from",        nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "reply-to");                  if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_replyto",     nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "to");                        if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_to",          nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "cc");                        if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_cc",          nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "subject");                   if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_subject",     nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "date");                      if (nes_isstr(cobj)) {
		t=time_wmgetdate(nes_tostr(sid->N, cobj));
		strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
		nes_setstr(sid->N, tobj, "hdr_date",        curdate, -1);
	}
	cobj=nes_getobj(sid->N, hobj, "message-id");                if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_messageid",   nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "in-reply-to");               if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_inreplyto",   nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "boundary");                  if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_boundary",    nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "content-type");              if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_contenttype", nes_tostr(sid->N, cobj), -1);
	cobj=nes_getobj(sid->N, hobj, "content-transfer-encoding"); if (nes_isstr(cobj)) nes_setstr(sid->N, tobj, "hdr_encoding",    nes_tostr(sid->N, cobj), -1);
	nes_setstr(sid->N, tobj, "status",    nes_getstr(sid->N, hobj, "status"), -1);
	nes_setstr(sid->N, tobj, "uidl",      uidl,                               -1);
	if ((id=dbwrite_mailheader(sid, 0, &qobj))<1) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		rc=-5; goto cleanup;
	}
	wmfilter_apply(sid, &hobj, accountid, id);
/*
//	dstmbox=wmfilter_apply(sid, &header, sid->dat->mailcurrent, localid);
//	if (wmfolder_msgmove(sid, accountid, id, folderid, dstmbox)<0) return -1;
*/
	dstmbox=0;
	if ((tobj=ldir_getlist(sid->N, "emailfolder", accountid, sid->dat->did))==NULL) goto cleanup;
	for (i=0;i<ldir_numentries(&tobj);i++) {
		if ((dstmbox==0)||(atoi(ldir_getval(&tobj, i, "id"))<dstmbox)) dstmbox=atoi(ldir_getval(&tobj, i, "id"));
	}
	ldir_freeresult(&tobj);
	if (dstmbox) {
		if (wmfolder_msgmove(sid, accountid, id, 0, dstmbox)<0) return -1;
/*
		sql_updatef("UPDATE gw_email_headers SET size = %d WHERE mailheaderid = %d AND accountid = %d", (int)sb.st_size, localid, sid->dat->mailcurrent);
*/
		if (qobj) sql_freeresult(proc->N, &qobj);
		if ((qobj=ldir_getentry(sid->N, "emailheader", NULL, id, sid->dat->did))!=NULL) {
			tobj=nes_settable(sid->N, nes_getiobj(sid->N, nes_settable(sid->N, qobj, "_rows"), 0), "_data");
			nes_setnum(sid->N, tobj, "folderid", dstmbox);
			nes_setnum(sid->N, tobj, "mailheaderid", id);
			ldir_saveentry(sid, id, "emailheader", &qobj);
			ldir_freeresult(&qobj);
		}
	}
	/* unlink leaves the var intact if there are other references.  hobj still needs to be freed, tho... */
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	if (hobj) {
		nes_unlinkval(sid->N, hobj);
		free(hobj);
	}
	return rc;
}

static int wmserver_msgsync(CONN *sid, int remoteid, int localid, char *uidl, int verbose)
{
	char *dompath=nes_getstr(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "var_domains_path");
	int accountid=(int)nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");
	char inbuffer[1024];
	char msgfilename[512];
	char *ptemp;
	int folderid=0;

	if (wmfolder_testcreate(sid, accountid, folderid)<0) return -1;
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.tmp", dompath, sid->dat->did, accountid, folderid, localid);
	fixslashes(msgfilename);
	if (strncasecmp(sid->dat->wm->servertype, "pop3", 4)==0) {
		wmprints(sid, "RETR %d\r\n", remoteid);
		if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
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
	} else {
		return -1;
	}
	if (verbose) {
		prints(sid, ".");
		flushbuffer(sid);
	}
	if (wmserver_msgbodysync(sid, msgfilename, localid, accountid, uidl)<0) return -1;
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

static int wmserver_mlistsync(CONN *sid, char ***uidl_list)
{
	char inbuffer[1024];
	char lockfile[512];
	char uidltemp[71];
#ifdef WIN32
	FILE *fp;
#else
	int fp;
#endif
	obj_t *qobj1=NULL, *qobj2=NULL;
	char **uidls=NULL;
	char *ptemp;
	int accountid=(int)nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");
	int found=0;
	int headerid;
	int i, j;
	int nummessages=0;
	struct stat sb;

//	DEBUG_IN(sid, "wmserver_mlistsync()");
	if (sid==NULL) {
//		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	if (accountid==0) {
//		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	if (wmfolder_testcreate(sid, accountid, 1)<0) {
		return -1;
	}
	memset(lockfile, 0, sizeof(lockfile));
	snprintf(lockfile, sizeof(lockfile)-1, "%s/%04d/mail/%04d/mbox.lock", nes_getstr(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "var_domains_path"), sid->dat->did, accountid);
	fixslashes(lockfile);
	if (stat(lockfile, &sb)==0) {
		found=1;
		i=time(NULL)-sb.st_mtime;
		if (i<60) {
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "lock file found (%d seconds old) - bailing", i);
			return -2;
		} else {
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "lock file found (%d seconds old) - proceeding", i);
			unlink(lockfile);
		}
	}
#ifdef WIN32
	if ((fp=_fsopen(lockfile, "wb", _SH_DENYWR))==NULL) return -2;
	fprintf(fp, "This mailbox is LOCKED!\r\n");
#else
	unlink(lockfile);
	if ((fp=open(lockfile, O_RDWR|O_CREAT|O_TRUNC))<0) return -2;
	if (flock(fp, LOCK_EX)!=0) return -2;
	write(fp, "This mailbox is LOCKED!\r\n", 25);
#endif
	if (found) {
		log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ignored stale lock file");
	}
	/* purge abandoned headers */
//	sql_updatef(proc->N, "DELETE FROM gw_email_headers WHERE folder = 0 AND accountid = %d", accountid);
	/* checking for drop-in msgs in the tmp dir might be good here */
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
	if ((qobj1=ldir_getlist(sid->N, "emailheader", accountid, sid->dat->did))==NULL) {
//	if (sql_queryf(&sqr1, "SELECT mailheaderid, uidl FROM gw_email_headers WHERE obj_uid = %d AND accountid = %d", sid->dat->uid, sid->dat->mailcurrent)<0) {
		nummessages=-1;
		goto cleanup;
	}
	for (i=0;i<nummessages;i++) {
		found=-1;
		for (j=0;j<ldir_numentries(&qobj1);j++) {
			if (strcmp((char *)uidls[i], ldir_getval(&qobj1, j, "uidl"))==0) {
				found=j;
				break;
			}
		}
		if (found>-1) continue;
//		if (sql_queryf(proc->N, &qobj2, "SELECT max(id) FROM nullgs_entries WHERE accountid = %d", accountid)<0) {
		if (sql_queryf(proc->N, &qobj2, "SELECT max(id) FROM nullgs_entries")<0) {
			ldir_freeresult(&qobj1);
			nummessages=-1;
			goto cleanup;
		}
		headerid=atoi(sql_getvalue(proc->N, &qobj2, 0, 0))+1;
		sql_freeresult(proc->N, &qobj2);
		if (headerid<1) headerid=1;
		if (wmserver_msgsync(sid, i+1, headerid, uidls[i], 0)<0) goto cleanup;
		prints(sid, ".");
		flushbuffer(sid);
	}
	sql_freeresult(proc->N, &qobj1);
cleanup:
#ifdef WIN32
	fclose(fp);
#else
	flock(fp, LOCK_UN);
	close(fp);
#endif
	unlink(lockfile);
//	DEBUG_OUT(sid, "wmserver_mlistsync()");
	return nummessages;
}

void wmserver_purge(CONN *sid)
{
/*
	char **uidls;
	int numlocal;
	int numremote;
	int i, j;
	SQLRES sqr;

	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(EMAIL)))==NULL) return;
	}
	numremote=wmserver_mlistsync(sid, &uidls);
	if (numremote==-2) prints(sid, "<BR /><B>%s</B>\r\n", lang_gets(sid, "mod_email", "mail_err_box_lock"));
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
		if (sql_queryf(&sqr, "SELECT mailheaderid, uidl FROM gw_email_headers WHERE obj_uid = %d AND accountid = %d ORDER BY hdr_date DESC", sid->dat->uid, sid->dat->mailcurrent)<0) goto cleanup;
	} else if (sid->dat->wm->remove==2) {
		if (sql_queryf(&sqr, "SELECT mailheaderid, uidl FROM gw_email_headers WHERE obj_uid = %d AND accountid = %d AND status = 'd' ORDER BY hdr_date DESC", sid->dat->uid, sid->dat->mailcurrent)<0) goto cleanup;
	} else {
		goto cleanup;
	}
	numlocal=sql_numtuples(&sqr);
	for (i=numlocal-1;i>-1;i--) {
		for (j=numremote-1;j>-1;j--) {
			if (strlen(sql_getvalue(&sqr, i, 1))<1) continue;
			if (strcmp(sql_getvalue(&sqr, i, 1), uidls[j])==0) {
				wmserver_msgdele(sid, j+1);
				break;
			}
		}
	}
	sql_freeresult(&sqr);
cleanup:
	wmserver_disconnect(sid);
	for (i=0;i<numremote;i++) free(uidls[i]);
	free(uidls);
*/
	return;
}

static int wmserver_send(CONN *sid, int mailid, int verbose)
{
	char inbuffer[1024];
	char outbuffer[1024];
	char msgfilename[512];
	char msgaddr[128];
	obj_t *confobj=nes_settable(proc->N, &proc->N->g, "CONFIG");
	obj_t *hobj=NULL;
	char *hostname=nes_getstr(proc->N, confobj, "host_name");
	char *ptemp;
	FILE *fp;
	int accountid=(int)nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");
	int outlen;
	int i;
//	SQLRES sqr;

	memset(outbuffer, 0, sizeof(outbuffer));
//	if ((hobj=ldir_getentry(sid->N, "emailheader", NULL, mailid, sid->dat->did))==NULL) {
//		if (verbose) prints(sid, "<BR />%s", lang_gets(sid, "mod_email", "mail_err_nomsg"));
//		return -1;
//		prints(sid, "error reading header"); return;
//	}
//	if (sql_queryf(&sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d and accountid = %d and status != 'd' and folder = '2' and mailheaderid = %d", sid->dat->uid, sid->dat->mailcurrent, mailid)<0) return -1;
//	if (sql_numtuples(&sqr)!=1) {
//		if (verbose) prints(sid, "<BR />%s", lang_gets(sid, "mod_email", "mail_err_nomsg"));
//		return -1;
//	}
	dbread_mailheader(sid, 2, mailid, &hobj);
	if (hobj==NULL) {
		if (verbose) prints(sid, "<BR />%s", lang_gets(sid, "mod_email", "mail_err_nomsg"));
		return -1;
	}
//	sql_freeresult(&sqr);
	if (wmserver_smtpconnect(sid)!=0) return -1;
	snprintf(outbuffer, sizeof(outbuffer)-1, "HELO %s\r\n", hostname);
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
			case 1: ptemp=ldir_getval(&hobj, 0, "cc"); break;
			case 2: ptemp=ldir_getval(&hobj, 0, "bcc"); break;
			default: ptemp=ldir_getval(&hobj, 0, "to"); break;
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
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/domains/%04d/mail/%04d/%04d/%06d.msg", nes_getstr(proc->N, confobj, "var_path"), sid->dat->did, accountid, 2, mailid);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "r");
	if (fp==NULL) {
		if (verbose) prints(sid, "<BR /><B>Message %d is missing!</B>\r\n", mailid);
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
		wmffgets(inbuffer, sizeof(inbuffer)-1, &fp);
		if (fp==NULL) break;
		striprn(inbuffer);
		if (strcmp(inbuffer, ".")==0) {
			strncat(inbuffer, ".", sizeof(inbuffer));
		}
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
		if (verbose) prints(sid, "<BR /><B>%s</B>\r\n", lang_gets(sid, "mod_email", "mail_sent"));
		flushbuffer(sid);
		wmserver_smtpdisconnect(sid);
		sql_updatef(proc->N, "UPDATE gw_email_headers SET folder = '3' WHERE mailheaderid = %d AND accountid = %d", mailid, accountid);
		wmfolder_msgmove(sid, accountid, mailid, 2, 3);
		return 0;
	}
quit:
	if (verbose) prints(sid, "<BR /><B>ERROR: %s</B>\r\n", str2html(sid, outbuffer));
	if (verbose) prints(sid, "<BR /><B>ERROR: %s</B>\r\n", str2html(sid, inbuffer));
	flushbuffer(sid);
	wmserver_smtpdisconnect(sid);
	return -1;
}

int wmsync(CONN *sid, int verbose)
{
	int accountid=(int)nes_getnum(sid->N, nes_settable(sid->N, nes_settable(sid->N, &sid->N->g, "_USER"), "pref"), "mailcurrent");
	char *ScriptName=nes_getstr(sid->N, nes_settable(sid->N, &sid->N->g, "_SERVER"), "SCRIPT_NAME");
	char **uidls=NULL;
	int numremote;
	int numlocal;
	int i, j;
//	SQLRES sqr;
	obj_t *qobj=NULL;
	int smtperror=0;

	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(EMAIL)))==NULL) return -1;
	}
	if (verbose) {
		prints(sid, "<BR /><B><NOBR>%s</NOBR> ", lang_gets(sid, "mod_email", "mail_checking"));
		flushbuffer(sid);
	}
	numremote=wmserver_mlistsync(sid, &uidls);
	if ((verbose)&&(numremote==-2)) prints(sid, "<BR /><B>%s</B>\r\n", lang_gets(sid, "mod_email", "mail_err_box_lock"));
	if (numremote<0) {
		wmserver_disconnect(sid);
		if (sid->dat->wm!=NULL) {
			free(sid->dat->wm);
			sid->dat->wm=NULL;
		}
		return numremote;
	}
//	sql_updatef("UPDATE gw_email_accounts SET lastcount = %d, lastcheck = '%s' WHERE obj_uid = %d AND mailaccountid = %d", numremote, time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, time(NULL)), sid->dat->uid, sid->dat->mailcurrent);
	if (numremote<1) goto cleanup;
	if (uidls!=NULL) {
		if (sql_queryf(proc->N, &qobj, "SELECT mailheaderid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_email_headers WHERE obj_uid = %d and accountid = %d and status != 'd' ORDER BY hdr_date ASC", sid->dat->uid, accountid)<0) goto cleanup;
		numlocal=sql_numtuples(proc->N, &qobj);
		for (i=0;i<numlocal;i++) {
			for (j=0;j<numremote;j++) {
				if (strcmp(uidls[j], sql_getvalue(proc->N, &qobj, i, 4))==0) {
		// see if we actually need to resync this msg, or if we're just dumb
	//				prints(sid, "bogus update attempt? FIXME<BR />");
	//				wmserver_msgsync(sid, j+1, atoi(sql_getvalue(&sqr, i, 0)), uidls[j], verbose);
					break;
				}
			}
		}
		sql_freeresult(proc->N, &qobj);
	}
cleanup:
	if (verbose) {
		prints(sid, " %s.</B>\r\n", lang_gets(sid, "mod_email", "mail_ok"));
		flushbuffer(sid);
	}
	wmserver_disconnect(sid);
	if (uidls!=NULL) {
		for (i=0;i<numremote;i++) free(uidls[i]);
		free(uidls);
	}
	if (verbose) {
		if (sql_queryf(proc->N, &qobj, "SELECT mailheaderid FROM gw_email_headers WHERE obj_uid = %d AND obj_did = %d AND accountid = %d AND status != 'd' and folder = 2", sid->dat->uid, sid->dat->did, accountid)<0) return -1;
		for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
			if (wmserver_send(sid, atoi(sql_getvalue(proc->N, &qobj, i, 0)), verbose)!=0) smtperror=1;
		}
		sql_freeresult(proc->N, &qobj);
		if ((!smtperror)&&(!sid->dat->wm->showdebug)) {
//			if (sid->dat->menustyle>0) {
//				prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\n");
//				prints(sid, "window.setTimeout('location.replace(\"%s/mail/main\")', 2000);\r\n", ScriptName);
//				prints(sid, "// -->\r\n</SCRIPT>\r\n");
//				prints(sid, "<NOSCRIPT>\r\n");
//				prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/main\">\r\n", ScriptName);
//				prints(sid, "</NOSCRIPT>\r\n");
//			} else {
				prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/list\">\r\n", ScriptName);
//			}
		}
	}
	if (sid->dat->wm!=NULL) {
		free(sid->dat->wm);
		sid->dat->wm=NULL;
	}
	return numremote;
}
