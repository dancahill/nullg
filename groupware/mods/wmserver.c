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
#include "main.h"

static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int wmprints(CONNECTION *sid, const char *format, ...)
{
	char buffer[1024];
	va_list ap;

	sid->atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	send(sid->dat->user_wmsocket, buffer, strlen(buffer), 0);

//	striprn(buffer);
//	logerror(sid, __FILE__, __LINE__, "> %s", buffer);
	return 0;
}

int wmfgets(CONNECTION *sid, char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char temp[2];
	int n=0;
	int rc;

	sid->atime=time(NULL);
	memset(temp, 0, sizeof(temp));
	while (n<max) {
		if ((rc=recv(fd, temp, 1, 0))<1) {
			prints(sid, "<CENTER>Connection reset by server.</CENTER>");
			closeconnect(sid, 1);
			return -1;
		}
		sid->dat->user_wmimapmread++;
		n++;
		if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
			if ((temp[0]=='.')&&(pbuffer[0]=='.')&&(n==2)) continue;
		}
		if (temp[0]==13) continue;
		*buffer=temp[0];
		buffer++;
		if (temp[0]==10) break;
	}
	*buffer=0;
	striprn(pbuffer);

//	logerror(sid, __FILE__, __LINE__, "< %s", pbuffer);
	return n;
}

void wmclose(CONNECTION *sid)
{
	/* shutdown(x,0=recv, 1=send, 2=both) */
	shutdown(sid->dat->user_wmsocket, 2);
	closesocket(sid->dat->user_wmsocket);
	fflush(stdout);
	return;
}

int wmserver_smtpconnect(CONNECTION *sid)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char inbuffer[1024];

	/* some smtp servers like pop auth before smtp */
	if (wmserver_connect(sid, 1)!=0) return -1;
	wmserver_disconnect(sid);
	if ((hp=gethostbyname(sid->dat->user_wmsmtpserver))==NULL) {
		prints(sid, "<CENTER>Could not find SMTP server '%s'</CENTER>\n", sid->dat->user_wmsmtpserver);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=(unsigned short)htons(sid->dat->user_wmsmtpport);
	if ((sid->dat->user_wmsocket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
//	setsockopt(wmsocket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(sid->dat->user_wmsocket, (struct sockaddr *)&server, sizeof(server))<0) {
		prints(sid, "<CENTER>Could not connect to SMTP server '%s'</CENTER>\n", sid->dat->user_wmsmtpserver);
		return -1;
	}
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	return 0;
}

int wmserver_smtpauth(CONNECTION *sid)
{
	char authstring[200];
	char inbuffer[1024];

	wmprints(sid, "AUTH PLAIN ");
	snprintf(authstring, sizeof(authstring)-1, "%s%c%s%c%s", sid->dat->user_wmusername, '\0', sid->dat->user_wmusername, '\0', sid->dat->user_wmpassword);
	EncodeBase64(sid, authstring, (strlen(sid->dat->user_wmusername)*2)+strlen(sid->dat->user_wmpassword)+2);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "502", 3)==0) return 0;
	if (strncasecmp(inbuffer, "235", 3)!=0) return -1;
	return 0;
}

void wmserver_smtpdisconnect(CONNECTION *sid)
{
	char inbuffer[1024];

	wmprints(sid, "QUIT\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	wmclose(sid);
}

int wmserver_connect(CONNECTION *sid, int verbose)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char inbuffer[1024];
	char timebuffer[100];
	char *ptemp;
	int sqr;

	if (sid->dat->user_wmconnected) return 0;
	if (!(auth_priv(sid, AUTH_WEBMAIL)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	if ((sqr=sqlQueryf(sid, "SELECT realname, organization, popusername, poppassword, hosttype, pophost, popport, smtphost, smtpport, address, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		if ((sqr=sqlQueryf(sid, "SELECT mailaccountid FROM gw_mailaccounts where obj_uid = %d", sid->dat->user_uid))<0) return -1;
		if (sqlNumtuples(sqr)<1) {
			sqlFreeconnect(sqr);
			return -1;
		}
		sid->dat->user_mailcurrent=atoi(sqlGetvalue(sqr, 0, 0));
		if (sqlUpdatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", sid->dat->user_mailcurrent, sid->dat->user_username)<0) return -1;
		sqlFreeconnect(sqr);
		if ((sqr=sqlQueryf(sid, "SELECT realname, organization, popusername, poppassword, hosttype, pophost, popport, smtphost, smtpport, address, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return -1;
	}
	if (sqlNumtuples(sqr)==1) {
		strncpy(sid->dat->user_wmrealname, sqlGetvalue(sqr, 0, 0), sizeof(sid->dat->user_wmrealname)-1);
		strncpy(sid->dat->user_wmorganization, sqlGetvalue(sqr, 0, 1), sizeof(sid->dat->user_wmorganization)-1);
		strncpy(sid->dat->user_wmusername, sqlGetvalue(sqr, 0, 2), sizeof(sid->dat->user_wmusername)-1);
		strncpy(sid->dat->user_wmpassword, DecodeBase64string(sid, sqlGetvalue(sqr, 0, 3)), sizeof(sid->dat->user_wmpassword)-1);
		strncpy(sid->dat->user_wmservertype, sqlGetvalue(sqr, 0, 4), sizeof(sid->dat->user_wmservertype)-1);
		strncpy(sid->dat->user_wmpopserver, sqlGetvalue(sqr, 0, 5), sizeof(sid->dat->user_wmpopserver)-1);
		sid->dat->user_wmpopport=atoi(sqlGetvalue(sqr, 0, 6));
		strncpy(sid->dat->user_wmsmtpserver, sqlGetvalue(sqr, 0, 7), sizeof(sid->dat->user_wmsmtpserver)-1);
		sid->dat->user_wmsmtpport=atoi(sqlGetvalue(sqr, 0, 8));
		strncpy(sid->dat->user_wmreplyto, sqlGetvalue(sqr, 0, 9), sizeof(sid->dat->user_wmreplyto)-1);
		strncpy(sid->dat->user_wmsignature, sqlGetvalue(sqr, 0, 10), sizeof(sid->dat->user_wmsignature)-1);
	}
	sqlFreeconnect(sqr);
	if (strcmp(sid->dat->in_RequestMethod,"POST")==0) {
		if (getpostenv(sid, "WMUSERNAME")!=NULL) {
			strncpy(sid->dat->user_wmusername, getpostenv(sid, "WMUSERNAME"), sizeof(sid->dat->user_wmusername)-1);
		}
		if (getpostenv(sid, "WMPASSWORD")!=NULL) {
			strncpy(sid->dat->user_wmpassword, getpostenv(sid, "WMPASSWORD"), sizeof(sid->dat->user_wmpassword)-1);
		}
	}
	if ((strlen(sid->dat->user_wmusername)==0)||(strlen(sid->dat->user_wmpassword)==0)||(strlen(sid->dat->user_wmpopserver)==0)||(strlen(sid->dat->user_wmsmtpserver)==0)) {
		if (verbose) {
			wmloginform(sid);
		}
		return -1;
	}
	if ((hp=gethostbyname(sid->dat->user_wmpopserver))==NULL) {
		if (verbose) {
			prints(sid, "<CENTER>Could not find POP3 server '%s'</CENTER>\n", sid->dat->user_wmpopserver);
		}
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=htons(sid->dat->user_wmpopport);
	if ((sid->dat->user_wmsocket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
//	setsockopt(sid->dat->user_wmsocket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(sid->dat->user_wmsocket, (struct sockaddr *)&server, sizeof(server))<0) {
		if (verbose) {
			prints(sid, "<CENTER>Could not connect to POP3 server '%s'</CENTER>\n", sid->dat->user_wmpopserver);
		}
		return -1;
	}
	sid->dat->user_wmconnected=1;
	/* Check current status */
	wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		/* Send user name */
		wmprints(sid, "USER %s\r\n", sid->dat->user_wmusername);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
		wmprints(sid, "PASS %s\r\n", sid->dat->user_wmpassword);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		/* Send user name */
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d LOGIN %s %s\r\n", sid->dat->user_wmimapidx, sid->dat->user_wmusername, sid->dat->user_wmpassword);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
//		sid->dat->user_wmimapidx++;
//		wmprints(sid, "%d LIST \"\" \"*\"\r\n", sid->dat->user_wmimapidx);
//		for (;;) {
//			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
//			if (isdigit(inbuffer[0])) break;
//		}
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d SELECT INBOX\r\n", sid->dat->user_wmimapidx);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
		snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, time(NULL)));
		sqlUpdatef(sid, "UPDATE gw_mailaccounts SET obj_mtime = '%s', popusername = '%s', poppassword = '%s' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->user_wmusername, EncodeBase64string(sid, sid->dat->user_wmpassword), sid->dat->user_uid, sid->dat->user_mailcurrent);
	}
	return 0;
}

void wmserver_disconnect(CONNECTION *sid)
{
	char inbuffer[1024];

	if (sid<0) return;
	if (!sid->dat->user_wmconnected) return;
	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "QUIT\r\n");
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		wmclose(sid);
		sid->dat->user_wmconnected=0;
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d LOGOUT\r\n", sid->dat->user_wmimapidx);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (isdigit(inbuffer[0])) break;
		}
		wmclose(sid);
		sid->dat->user_wmconnected=0;
	}
	return;
}

int wmserver_count(CONNECTION *sid)
{
	char inbuffer[1024];
	char *ptemp;
	int count=-1;

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "STAT\r\n");
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		return atoi(ptemp);
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d STATUS INBOX (MESSAGES)\r\n", sid->dat->user_wmimapidx);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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

int wmserver_msgdele(CONNECTION *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "DELE %d\r\n", message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d STORE %d +FLAGS (\\Deleted)\r\n", sid->dat->user_wmimapidx, message);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (isdigit(inbuffer[0])) break;
		}
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d EXPUNGE\r\n", sid->dat->user_wmimapidx);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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

int wmserver_msghead(CONNECTION *sid, int message)
{
	char inbuffer[1024];

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "TOP %d 0\r\n", message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d FETCH %d BODY[HEADER]\r\n", sid->dat->user_wmimapidx, message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		return 0;
	}
	return -1;
}

int wmserver_msgretr(CONNECTION *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "RETR %d\r\n", message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		return 0;
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d FETCH %d RFC822\r\n", sid->dat->user_wmimapidx, message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		ptemp=strchr(inbuffer, '{');
		if (*ptemp) ptemp++;
		if (*ptemp) {
			sid->dat->user_wmimapmsize=atoi(ptemp);
		} else {
			sid->dat->user_wmimapmsize=-1;
		}
		sid->dat->user_wmimapmread=0;
		return 0;
	}
	return -1;
}

int wmserver_msgsize(CONNECTION *sid, int message)
{
	char inbuffer[1024];
	char *ptemp;
	int size=-1;

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "LIST %d\r\n", message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		while ((ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		return atoi(ptemp);
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d FETCH %d RFC822.SIZE\r\n", sid->dat->user_wmimapidx, message);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (isdigit(inbuffer[0])) break;
			if (strcasestr(inbuffer, "FETCH")!=NULL) {
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

int wmserver_uidl(CONNECTION *sid, int message, char *uidl)
{
	char uidltemp[71];
	unsigned char a, b, c, d;
	int dst, i, enclen, remlen;
	char inbuffer[1024];
	char *ptemp;

	memset(uidltemp, 0, sizeof(uidltemp));
	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "UIDL %d\r\n", message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) return -1;
		ptemp=inbuffer;
		while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
		ptemp++;
		snprintf(uidltemp, 70, "%s", ptemp);
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d FETCH %d UID\r\n", sid->dat->user_wmimapidx, message);
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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

int is_msg_end(CONNECTION *sid, char *buffer)
{
	char inbuffer[1024];

	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		if (strcmp(buffer, ".")==0) return 1;
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		if ((sid->dat->user_wmimapmsize>0)&&(sid->dat->user_wmimapmread<sid->dat->user_wmimapmsize)) return 0;
		if (strcmp(buffer, ")")==0) {
			for (;;) {
				if (isdigit(inbuffer[0])) {
					if (atoi(inbuffer)==sid->dat->user_wmimapidx) break;
				}
				wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			}
			return 1;
		}
	}
	return 0;
}

int wmserver_mlistsync(CONNECTION *sid, char ***uidl_list)
{
	wmheader header;
	char curdate[40];
	char inbuffer[1024];
	char query[2048];
	char uidltemp[71];
	char **uidls;
	char *ptemp;
	int i, j;
	int found;
	int msize;
	int nummessages;
	int sqr1=-1;
	int sqr2=-1;
	int headerid;
	int showsync=1;
	time_t t;

	DEBUG_IN(sid, "wmserver_mlistsync()");
	if (sid==NULL) {
		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	if (wmserver_connect(sid, 0)!=0) {
		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	nummessages=wmserver_count(sid);
	uidls=(char **)calloc(nummessages, sizeof(char *));
	*uidl_list=uidls;
	for (i=0;i<nummessages;i++) uidls[i]=calloc(120, sizeof(char));
	if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
		wmprints(sid, "UIDL\r\n");
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strncasecmp(inbuffer, "+OK", 3)!=0) goto cleanup;
		for (i=0;;i++) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
	} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
		sid->dat->user_wmimapidx++;
		wmprints(sid, "%d FETCH 1:* UID\r\n", sid->dat->user_wmimapidx);
		for (;;) {
			memset(uidltemp, 0, sizeof(uidltemp));
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
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
		DEBUG_OUT(sid, "wmserver_mlistsync()");
		return -1;
	}
	if (!RunAsCGI) pthread_mutex_lock(&Lock.DB_mheader);
	if ((sqr1=sqlQueryf(sid, "SELECT mailheaderid, uidl FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	for (i=0;i<nummessages;i++) {
		found=-1;
		for (j=0;j<sqlNumtuples(sqr1);j++) {
			if (strcmp((char *)uidls[i], sqlGetvalue(sqr1, j, 1))==0) {
				found=j;
				break;
			}
		}
		if (found==-1) {
			if (showsync) {
				prints(sid, "Syncing headers");
				flushbuffer(sid);
				showsync=0;
			}
			if ((sqr2=sqlQuery(sid, "SELECT max(mailheaderid) FROM gw_mailheaders"))<0) goto cleanup;
			headerid=atoi(sqlGetvalue(sqr2, 0, 0))+1;
			sqlFreeconnect(sqr2);
			sqr2=-1;
			if (headerid<1) headerid=1;
//			if ((msize=wmserver_msgsize(sid, i+1))<0) continue;
			msize=wmserver_msgsize(sid, i+1);
			wmserver_msghead(sid, i+1);
			memset((char *)&header, 0, sizeof(header));
			if (webmailheader(sid, &header)!=0) goto cleanup;
			if (strncasecmp(sid->dat->user_wmservertype, "pop3", 4)==0) {
				for (;;) {
					wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
					if (strcmp(inbuffer, ".")==0) break;
				}
			} else if (strncasecmp(sid->dat->user_wmservertype, "imap", 4)==0) {
				for (;;) {
					wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
					if (isdigit(inbuffer[0])) break;
				}
			}
			t=time(NULL);
			strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
			memset(query, 0, sizeof(query));
			strcpy(query, "INSERT INTO gw_mailheaders (mailheaderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", headerid, curdate, curdate, sid->dat->user_uid, 0, 0, 0);
			strcatf(query, "'%d', ", sid->dat->user_mailcurrent);
			strcatf(query, "'Inbox', "); /* folder */
			strcatf(query, "'%c', ", header.status);
			strcatf(query, "'%d', ", msize);
			strcatf(query, "'%s', ", str2sql(sid, (char *)uidls[i]));
			strcatf(query, "'%s', ", str2sql(sid, header.From));
			strcatf(query, "'%s', ", str2sql(sid, header.Replyto));
			strcatf(query, "'%s', ", str2sql(sid, header.To));
			t=wmgetdate(header.Date);
			strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
			strcatf(query, "'%s', ", str2sql(sid, curdate));
			strcatf(query, "'%s', ", str2sql(sid, header.Subject));
			strcatf(query, "'%s', ", str2sql(sid, header.CC));
			strcatf(query, "'%s', ", str2sql(sid, header.contenttype));
			strcatf(query, "'%s', ", str2sql(sid, header.boundary));
			strcatf(query, "'%s')", str2sql(sid, header.encoding));
			if (sqlUpdate(sid, query)<0) goto cleanup;
			prints(sid, ".");
			flushbuffer(sid);
		}
	}
	wmserver_disconnect(sid);
	sqlFreeconnect(sqr1);
	sqr1=-1;
	if ((sqr1=sqlQueryf(sid, "SELECT mailheaderid, uidl FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	for (i=0;i<sqlNumtuples(sqr1);i++) {
		found=-1;
		for (j=0;j<nummessages;j++) {
			if (strcmp(sqlGetvalue(sqr1, i, 1), uidls[j])==0) {
				found=j;
				break;
			}
		}
		if (found==-1) {
			if (sqlUpdatef(sid, "DELETE FROM gw_mailheaders WHERE mailheaderid = %d AND obj_uid = %d", atoi(sqlGetvalue(sqr1, i, 0)), sid->dat->user_uid)<0) goto cleanup;
		}
	}
	sqlFreeconnect(sqr1);
	sqr1=-1;
	if (sqr1>=0) sqlFreeconnect(sqr1);
	if (sqr2>=0) sqlFreeconnect(sqr2);
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.DB_mheader);
	DEBUG_OUT(sid, "wmserver_mlistsync()");
	return nummessages;
cleanup:
	if (sqr1>=0) sqlFreeconnect(sqr1);
	if (sqr2>=0) sqlFreeconnect(sqr2);
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.DB_mheader);
	DEBUG_OUT(sid, "wmserver_mlistsync()");
	return -1;
}
