/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

#define MAX_LIST_SIZE 25

typedef struct {
	char From[128];
	char Replyto[128];
	char To[128];
	char Date[128];
	char Subject[128];
	char CC[1024];
	char contenttype[128];
	char boundary[128];
	char encoding[128];
} wmheader;

static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static short int pop3port=110;
static short int smtpport=25;
#ifdef WIN32
static WSADATA wsaData;
#endif

void wmloginform(int sid)
{
	char msgto[512];
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	if (getgetenv(sid, "MSGTO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "MSGTO"), sizeof(msgto)-1);
	}
	/* don't ask...  heh..  it was either this or a goto */
	for (;;) {
		if ((sqr=sqlQueryf("SELECT emailusername, emailpassword FROM gw_users where username = '%s'", conn[sid].dat->in_username))<0) break;
		if (sqlNumtuples(sqr)==1) {
			strncpy(conn[sid].dat->wm_username, sqlGetvalue(sqr, 0, 0), sizeof(conn[sid].dat->wm_username)-1);
			strncpy(conn[sid].dat->wm_password, sqlGetvalue(sqr, 0, 1), sizeof(conn[sid].dat->wm_password)-1);
		}
		sqlFreeconnect(sqr);
		break;
	}
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/mail", conn[sid].dat->in_ScriptName);
	if (strlen(msgto)>0) {
		prints("write?msgto=%s", msgto);
	} else {
		prints("list");
	}
	prints(" AUTOCOMPLETE=OFF NAME=wmlogin>\n");
	prints("<TABLE CELLPADDING=2 CELLSPACING=0 BORDER=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>Webmail Login</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><B>Username</B></TD><TD><INPUT TYPE=TEXT NAME=WMUSERNAME SIZE=25 VALUE='%s'></TD></TR>\n", COLOR_EDITFORM, conn[sid].dat->wm_username);
	prints("<TR BGCOLOR=%s><TD><B>Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=WMPASSWORD SIZE=25 VALUE='%s'></TD></TR>\n", COLOR_EDITFORM, conn[sid].dat->wm_password);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\n", COLOR_EDITFORM);
	prints("</TABLE></FORM></CENTER>\n");
	if (strlen(conn[sid].dat->wm_username)<1) {
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.WMUSERNAME.focus();\n// -->\n</SCRIPT>\n");
	} else {
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.WMPASSWORD.focus();\n// -->\n</SCRIPT>\n");
	}
}

void wmlogout(int sid)
{
	char timebuffer[100];
	time_t t;

	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	sqlUpdatef("UPDATE gw_users SET mdatetime = '%s',  emailpassword = '' where username = '%s'", timebuffer, conn[sid].dat->in_username);
	return;
}

void wmcloseconn()
{
	char junk[10];
	int sid=getsid();

	if (sid<0) return;
#ifdef WIN32
	shutdown(conn[sid].dat->wm_socket, SD_BOTH);
#endif
	while (recv(conn[sid].dat->wm_socket, junk, sizeof(junk), 0)>0) {
	};
#ifdef WIN32
	closesocket(conn[sid].dat->wm_socket);
#else
	close(conn[sid].dat->wm_socket);
#endif
	fflush(stdout);
	return;
}

int wmfgets(char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char temp[2];
	int n=0;
	int rc=0;
	int sid=getsid();

	conn[sid].atime=time((time_t*)0);
	memset(temp, 0, sizeof(temp));
	while (n<max) {
		rc=recv(fd, temp, 1, 0);
		if (rc<0) {
			conn[sid].dat->wm_connected=0;
			wmcloseconn();
		} else if (rc!=1) {
			n=-n;
			break;
		}
		n++;
		if (temp[0]==13) continue;
		*buffer=temp[0];
		buffer++;
		if (temp[0]==10) break;
	}
	*buffer=0;
	striprn(pbuffer);
	return n;
}

int wmprints(const char *format, ...)
{
	char buffer[1024];
	va_list ap;
	int sid=getsid();

	conn[sid].atime=time((time_t*)0);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	send(conn[sid].dat->wm_socket, buffer, strlen(buffer), 0);
	return 0;
}

/* IS THIS B64 CODE LICENCED?  SOURCE = http://www.jti.net/brad/base64.htm */
int EncodeBase64(char *src, int srclen)
{
	unsigned char a, b, c, d, *cp;
	int dst, i, enclen, remlen, linelen;
	int progress=0;

	cp=src;
	dst=0;
	linelen=0;
	enclen=srclen/3;
	remlen=srclen-3*enclen;
	progress=enclen/10;
	for (i=0;i<enclen;i++) {
		a=(cp[0]>>2);
	        b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		c|=(cp[2]>>6);
		d=cp[2]&0x3f;
		cp+=3;
		wmprints("%c%c%c%c", Base64[a], Base64[b], Base64[c], Base64[d]);
		dst+=4;
		linelen+=4;
		if (linelen>=72) {
			wmprints("\r\n");
			linelen=0;
		}
		if (i>progress) {
			prints("*");
			progress+=enclen/10;
			flushbuffer(getsid());
		}
	}
	if (remlen==1) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		wmprints("%c%c==\r\n",Base64[a],Base64[b]);
		dst+=4;
	} else if (remlen==2) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		wmprints("%c%c%c=\r\n",Base64[a],Base64[b],Base64[c]);
		dst+=4;
	}
	return dst;
}

int DecodeBase64(int sid, char *src)
{
	char dest[1024];
	int destidx, state, ch;
	int szdest;
	char *pos;

	state=0;
	destidx=0;
	szdest=sizeof(dest);
	while ((ch=*src++)!='\0') {
		if (isspace(ch)) continue;
		if (ch=='=') break;
		pos=strchr(Base64, ch);
		if (pos==0) return (-1);
		switch (state) {
			case 0:
				if (dest) {
					if (destidx>=szdest) return (-1);
					dest[destidx]=(pos-Base64)<<2;
				}
				state=1;
				break;
			case 1:
				if (dest) {
					if (destidx+1>=szdest) return (-1);
					dest[destidx]|=(pos-Base64)>>4;
					dest[destidx+1]=((pos-Base64)&0x0f)<<4;
				}
				destidx++;
				state=2;
				break;
			case 2:
				if (dest) {
					if (destidx+1>=szdest) return (-1);
					dest[destidx]|=(pos-Base64)>>2;
					dest[destidx+1]=((pos-Base64)&0x03)<<6;
				}
				destidx++;
				state=3;
				break;
			case 3:
				if (dest) {
					if (destidx>=szdest) return (-1);
					dest[destidx]|=(pos-Base64);
				}
				destidx++;
				state=0;
				break;
		}
	}
	if (RunAsCGI) {
		fwrite(dest, sizeof(char), destidx, stdout);
	} else {
		send(conn[sid].socket, dest, destidx, 0);
	}
	return (destidx);
}

int DecodeQP(char *src)
{
	char dest[1024];
	char *destidx;

	memset(dest, 0, sizeof(dest));
	destidx=dest;
	while ((*src)&&(strlen(dest)<sizeof(dest))) {
		if (*src=='=') {
			src++;
			if (isxdigit(src[0])&&isxdigit(src[1])) {
				*destidx++=(char)IntFromHex(src);
				src+=2;
			}
		} else if (*src=='\r') {
			*destidx++=*src++;
			if (*src!='\n') *destidx++='\n';
		} else {
			*destidx++=*src++;
		}
	}
	prints("%s\r\n", dest);
	return (strlen(dest));
}

int Decode7bit(char *src)
{
	char dest[1024];
	char *destidx;

	memset(dest, 0, sizeof(dest));
	destidx=dest;
	while ((*src)&&(strlen(dest)<sizeof(dest))) {
		if (*src=='\r') {
			*destidx++=*src++;
			if (*src!='\n') *destidx++='\n';
		} else {
			*destidx++=*src++;
		}
	}
	prints("%s\r\n", dest);
	return (strlen(dest));
}

int Decode8bit(char *src)
{
	return Decode7bit(src);
}

int webmailconnect(int sid)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char inbuffer[1024];
	char timebuffer[100];
	char *verbose=NULL;
	time_t t;
	int sqr;

	if (conn[sid].dat->wm_connected) return 0;
	if ((sqr=sqlQueryf("SELECT emailusername, emailpassword, pop3server, smtpserver FROM gw_users where username = '%s'", conn[sid].dat->in_username))<0) return -1;
	if (sqlNumtuples(sqr)==1) {
		strncpy(conn[sid].dat->wm_username, sqlGetvalue(sqr, 0, 0), sizeof(conn[sid].dat->wm_username)-1);
		strncpy(conn[sid].dat->wm_password, sqlGetvalue(sqr, 0, 1), sizeof(conn[sid].dat->wm_password)-1);
		strncpy(conn[sid].dat->wm_pop3server, sqlGetvalue(sqr, 0, 2), sizeof(conn[sid].dat->wm_pop3server)-1);
		strncpy(conn[sid].dat->wm_smtpserver, sqlGetvalue(sqr, 0, 3), sizeof(conn[sid].dat->wm_smtpserver)-1);
	}
	sqlFreeconnect(sqr);
	if (getpostenv(sid, "WMUSERNAME")!=NULL)
		strncpy(conn[sid].dat->wm_username, getpostenv(sid, "WMUSERNAME"), sizeof(conn[sid].dat->wm_username)-1);
	if (getpostenv(sid, "WMPASSWORD")!=NULL)
		strncpy(conn[sid].dat->wm_password, getpostenv(sid, "WMPASSWORD"), sizeof(conn[sid].dat->wm_password)-1);
	if ((strlen(conn[sid].dat->wm_username)==0)||(strlen(conn[sid].dat->wm_password)==0)||(strlen(conn[sid].dat->wm_pop3server)==0)||(strlen(conn[sid].dat->wm_smtpserver)==0)) {
		wmloginform(sid);
		return -1;
	}
	if (!(hp=gethostbyname(conn[sid].dat->wm_pop3server))) {
		prints("<CENTER>Could not find POP3 server '%s'</CENTER>\n", conn[sid].dat->wm_pop3server);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=htons(pop3port);
	if ((conn[sid].dat->wm_socket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
	setsockopt(conn[sid].dat->wm_socket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(conn[sid].dat->wm_socket, (struct sockaddr *)&server, sizeof(server))<0) {
		prints("<CENTER>Could not connect to POP3 server '%s'</CENTER>\n", conn[sid].dat->wm_pop3server);
		return -1;
	}
	conn[sid].dat->wm_connected=1;
	/* Check current status */
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	/* Send user name */
	wmprints("USER %s\r\n", conn[sid].dat->wm_username);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "+OK", 3)!=0) {
		verbose=strchr(inbuffer, ' ');
		if (verbose) prints("<CENTER>%s</CENTER><BR>\n", verbose);
		wmprints("QUIT\r\n");
		wmcloseconn();
		wmloginform(sid);
		return -1;
	}
	/* Send password */
	wmprints("PASS %s\r\n", conn[sid].dat->wm_password);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "+OK", 3)!=0) {
		verbose=strchr(inbuffer, ' ');
		if (verbose) prints("<CENTER>%s</CENTER><BR>\n", verbose);
		wmprints("QUIT\r\n");
		wmcloseconn();
		wmloginform(sid);
		return -1;
	}
	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	sqlUpdatef("UPDATE gw_users SET mdatetime = '%s', emailusername = '%s', emailpassword = '%s', pop3server = '%s', smtpserver = '%s' where username = '%s'", timebuffer, conn[sid].dat->wm_username, conn[sid].dat->wm_password, conn[sid].dat->wm_pop3server, conn[sid].dat->wm_smtpserver, conn[sid].dat->in_username);
	return 0;
}

void webmaildisconnect()
{
	int sid=getsid();

	if (sid<0) return;
	if (conn[sid].dat->wm_connected) {
		wmprints("QUIT\r\n");
		wmcloseconn();
		conn[sid].dat->wm_connected=0;
	}
	return;
}

int webmailheader(int sid, wmheader *header)
{
	char inbuffer[1024];
	char *pTemp;

	if (webmailconnect(sid)!=0) return -1;
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		striprn(inbuffer);
		if (strcmp(inbuffer, "")==0) break;
		if (strncasecmp(inbuffer, "From:", 5)==0) {
			pTemp=inbuffer+5;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->From, pTemp, sizeof(header->From)-1);
		}
		if (strncasecmp(inbuffer, "Replyto:", 8)==0) {
			pTemp=inbuffer+8;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->Replyto, pTemp, sizeof(header->Replyto)-1);
		}
		if (strncasecmp(inbuffer, "To:", 3)==0) {
			pTemp=inbuffer+3;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->To, pTemp, sizeof(header->To)-1);
		}
		if (strncasecmp(inbuffer, "Subject:", 8)==0) {
			pTemp=inbuffer+8;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->Subject, pTemp, sizeof(header->Subject)-1);
		}
		if (strncasecmp(inbuffer, "Date:", 5)==0) {
			pTemp=inbuffer+5;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->Date, pTemp, sizeof(header->Date)-1);
		}
		if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
			pTemp=inbuffer+13;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->contenttype, pTemp, sizeof(header->contenttype)-1);
			if (strcasestr(header->contenttype, "multipart")==NULL) continue;
			if (strcasestr(header->contenttype, "boundary=")==NULL) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				striprn(inbuffer);
				if (strcasestr(inbuffer, "boundary=")!=NULL) {
					strncat(header->contenttype, inbuffer, sizeof(header->contenttype)-strlen(header->contenttype)-1);
				} else {
					continue;
				}
			}
		}
		if (strncasecmp(inbuffer, "Content-Transfer-Encoding: ", 26)==0) {
			pTemp=inbuffer+26;
			while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
			strncpy(header->encoding, pTemp, sizeof(header->encoding)-1);
		}
	}
	if ((pTemp=strcasestr(header->contenttype, "boundary="))!=NULL) {
		pTemp+=9;
		if (*pTemp=='\"') pTemp++;
		while ((*pTemp)&&(*pTemp!='\"')&&(strlen(header->boundary)<sizeof(header->boundary)-1)) {
			header->boundary[strlen(header->boundary)]=*pTemp;
			pTemp++;
		}
	}
	if (strlen(header->Replyto)==0) {
		pTemp=header->From;
		while ((*pTemp)&&(*pTemp!='<')) pTemp++;
		if (*pTemp=='<') pTemp++;
		while ((*pTemp)&&(*pTemp!='>')&&(strlen(header->Replyto)<sizeof(header->Replyto))) {
			header->Replyto[strlen(header->Replyto)]=*pTemp;
			pTemp++;
		}
		if (strlen(header->Replyto)==0) {
			strncpy(header->Replyto, header->From, sizeof(header->Replyto)-1);
		}
	}
	if (strlen(header->From)==0) strcpy(header->From, "(No Sender)");
	if (strlen(header->Subject)==0) strcpy(header->Subject, "(No Subject)");
	if (strlen(header->Date)==0) strcpy(header->Date, "(No Date)");
	return 0;
}

void webmailraw(int sid)
{
	char *pTemp;
	char inbuffer[1024];
	char status[8];
	int nummessages;
	int nummessage;

	send_header(sid, 1, 200, "OK", "1", "text/plain", -1, -1);
	if (webmailconnect(sid)!=0) return;
	wmprints("STAT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset(status, 0, sizeof(status));
	pTemp=inbuffer;
	while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
		status[strlen(status)]=*pTemp;
		pTemp++;
	}
	nummessages=atoi(pTemp);
	nummessage=atoi(getgetenv(sid, "MSG"));
	if ((nummessage>nummessages)||(nummessage<1)) {
		prints("No such message.<BR>");
		webmaildisconnect();
		return;
	}
	wmprints("RETR %d\r\n", nummessage);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	prints("<PRE>\r\n");
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		if (strcmp(inbuffer, "")==0) break;
		printht("%s\r\n", inbuffer);
	}
	prints("\r\n");
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		if (strcmp(inbuffer, ".")==0) break;
		printht("%s\r\n", inbuffer);
	}
	prints("</PRE>\r\n");
	webmaildisconnect();
	return;
}

void webmailfiledl(int sid)
{
	wmheader header;
	char *pQueryString;
	char *pTemp;
	char contentencoding[512];
	char contentfilename[512];
	char filename[512];
	char inbuffer[1024];
	char msgtype[100];
	char status[8];
	int nummessages;
	int nummessage=0;

	memset(filename, 0, sizeof(filename));
	pQueryString=strstr(conn[sid].dat->in_RequestURI, "/mailfile/");
	if (pQueryString==NULL) {
		printerror(sid, 400, "Bad Request", "Malformed URI");
		return;
	}
	pQueryString+=10;
	nummessage=atoi(pQueryString);
	while ((isdigit(*pQueryString)!=0)&&(*pQueryString!=0)) pQueryString++;
	while (*pQueryString=='/') pQueryString++;
	strncpy(filename, pQueryString, sizeof(filename)-1);
	pTemp=filename;
	URLDecode(pTemp);
	if ((nummessage<1)||(strlen(filename)<1)) {
		printerror(sid, 400, "Bad Request", "Attachment not found.");
		return;
	}
	if (webmailconnect(sid)!=0) return;
	wmprints("STAT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset(status, 0, sizeof(status));
	pTemp=inbuffer;
	while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
		status[strlen(status)]=*pTemp;
		pTemp++;
	}
	nummessages=atoi(pTemp);
	if (nummessages<nummessage) {
		printerror(sid, 400, "Bad Request", "No such message.");
		goto quit;
	}
	memset(msgtype, 0, sizeof(msgtype));
	wmprints("RETR %d\r\n", nummessage);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset((char *)&header, 0, sizeof(header));
	if (webmailheader(sid, &header)!=0) return;
	if (strcasestr(header.contenttype, "multipart")==NULL) {
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcmp(inbuffer, ".")==0) break;
		}
		printerror(sid, 400, "Bad Request", "No files are attached to this message.");
		goto quit;
	}
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		if (strcmp(inbuffer, ".")==0) break;
		if (strcasestr(inbuffer, header.boundary)!=NULL) {
			memset(contentencoding, 0, sizeof(contentencoding));
			memset(contentfilename, 0, sizeof(contentfilename));
		}
		pTemp=strcasestr(inbuffer, "name=");
		if (pTemp!=NULL) {
			pTemp+=5;
			if (*pTemp=='\"') pTemp++;
			while ((*pTemp)&&(*pTemp!='\"')&&(strlen(contentfilename)<sizeof(contentfilename)-1)) {
				contentfilename[strlen(contentfilename)]=*pTemp;
				pTemp++;
			}
		}
		if (strncasecmp(inbuffer, "Content-Transfer-Encoding:", 26)==0) {
			strncpy(contentencoding, (char *)&inbuffer+27, sizeof(contentencoding)-1);
			if ((strncasecmp(contentencoding, "base64", 6)!=0)&&(strncasecmp(contentencoding, "quoted-printable", 16)!=0)&&
			    (strncasecmp(contentencoding, "7bit", 4)!=0)&&(strncasecmp(contentencoding, "8bit", 4)!=0)) {
				memset(contentencoding, 0, sizeof(contentencoding));
				memset(contentfilename, 0, sizeof(contentfilename));
				continue;
			}
		}
		if ((strlen(contentencoding))&&(strlen(contentfilename))) {
			if (strcmp(contentfilename, filename)!=0) {
				memset(contentencoding, 0, sizeof(contentencoding));
				memset(contentfilename, 0, sizeof(contentfilename));
				continue;
			}
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) goto quit;
				if (strcmp(inbuffer, "")==0) break;
			}
			if (strcmp(inbuffer, "")==0) break;
		}
	}
	if ((strlen(contentencoding))&&(strlen(contentfilename))) {
		send_header(sid, 1, 200, "OK", "1", get_mime_type(contentfilename), -1, -1);
		flushbuffer(sid);
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcmp(inbuffer, ".")==0) goto quit;
			if (strcasestr(inbuffer, header.boundary)!=NULL) break;
			if (strncasecmp(contentencoding, "base64", 6)==0) {
				DecodeBase64(sid, inbuffer);
			} else if (strncasecmp(contentencoding, "quoted-printable", 16)==0) {
				DecodeQP(inbuffer);
			} else if (strncasecmp(contentencoding, "7bit", 4)==0) {
				Decode7bit(inbuffer);
			} else if (strncasecmp(contentencoding, "8bit", 4)==0) {
				Decode8bit(inbuffer);
			}
		}
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcmp(inbuffer, ".")==0) goto quit;
		}
	} else {
		printerror(sid, 400, "Bad Request", "Attachment not found.");
		goto quit;
	}
quit:
	webmaildisconnect();
	return;
}

char *webmailfileul(int sid, char *xfilename, char *xfilesize)
{
	char *filebody=NULL;
	char lfilename[1024];
	int filesize=0;
	char line[1024];
	char location[1024];
	char boundary[1024];
	char boundary2[1024];
	char boundary3[1024];
	char *pPostData;
	char *pTemp;
	int i;
	unsigned int j;

	pPostData=conn[sid].PostData;
	memset(boundary, 0, sizeof(boundary));
	memset(location, 0, sizeof(location));
	i=0;
	j=0;
	/* duhh..  this should just retrieve the boundary from conn[sid].dat->in_ContentType,
	 * and maybe check same to make sure this really is MIME, while it's at it.
	 */
	/* get the mime boundary */
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(strlen(boundary)<sizeof(boundary)-1)) {
		boundary[j]=*pPostData;
		pPostData++;
		i++;
		j++;
	}
	/* eat newline garbage */
	while ((*pPostData=='\n')||(*pPostData=='\r')) {
		pPostData++;
		i++;
	}
	snprintf(boundary2, sizeof(boundary2)-1, "%s--", boundary);
	snprintf(boundary3, sizeof(boundary3)-1, "\r\n%s", boundary);
	pPostData=conn[sid].PostData;
	while ((strcmp(line, boundary2)!=0)&&(i<conn[sid].dat->in_ContentLength)) {
		memset(line, 0, sizeof(line));
		j=0;
		while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(strlen(line)<sizeof(line)-1)) {
			line[j]=*pPostData;
			pPostData++;
			i++;
			j++;
		}
		/* eat newline garbage */
		while ((*pPostData=='\n')||(*pPostData=='\r')) {
			pPostData++;
			i++;
		}
		pTemp=line;
		if (strncasecmp(line, "Content-Disposition: form-data; ", 32)!=0) continue;
		pTemp+=32;
		if (strncasecmp(pTemp, "name=\"fattach\"; ", 16)!=0) continue;
		pTemp+=16;
		if (strncasecmp(pTemp, "filename=\"", 10)!=0) continue;
		pTemp+=10;
		if (strrchr(pTemp, '\\')!=NULL) {
			pTemp=strrchr(pTemp, '\\')+1;
		}
		snprintf(lfilename, sizeof(lfilename)-1, "%s", pTemp);
		while (lfilename[strlen(lfilename)-1]=='\"') lfilename[strlen(lfilename)-1]='\0';
		while ((strncmp(pPostData, "\r\n\r\n", 4)!=0)&&(strncmp(pPostData, "\n\n", 2)!=0)&&(i<conn[sid].dat->in_ContentLength)) {
			pPostData++;
			i++;
		}
		if (strncmp(pPostData, "\r\n\r\n", 4)==0) {
			pPostData+=4;
			i+=4;
		} else if (strncmp(pPostData, "\n\n", 2)==0) {
			pPostData+=2;
			i+=2;
		} else {
			continue;
		}
		snprintf(xfilename, 1024, "%s", lfilename);
		filebody=pPostData;
		filesize=0;
		while ((strncmp(pPostData, boundary3, strlen(boundary3))!=0)&&(i<conn[sid].dat->in_ContentLength)) {
			pPostData++;
			filesize++;
		}
	}
	snprintf(xfilesize, 9, "%d", filesize);
	return filebody;
}

int retardedOEmime(int sid, int reply, char *ctype)
{
	char *pTemp;
	char boundary[100];
	char inbuffer[1024];
	char msgencoding[100];
	char msgtype[100];
	int msgdone=0;

	memset(boundary, 0, sizeof(boundary));
	memset(msgtype, 0, sizeof(msgtype));
	memset(msgencoding, 0, sizeof(msgencoding));
	for (;;) {
		if (*ctype) {
			memcpy(inbuffer, ctype, strlen(ctype));
			*ctype='\0';
		} else {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			striprn(inbuffer);
		}
		if (strcmp(inbuffer, "")==0) break;
		if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
			strncpy(msgtype, (char *)&inbuffer+14, sizeof(msgtype)-1);
			if (strcasestr(msgtype, "multipart")==NULL) continue;
			if (strcasestr(msgtype, "boundary=")==NULL) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				striprn(inbuffer);
				if (strcasestr(inbuffer, "boundary=")!=NULL) {
					strncat(msgtype, inbuffer, sizeof(msgtype)-strlen(msgtype)-1);
				} else {
					continue;
				}
			}
		}
	}
	pTemp=strcasestr(msgtype, "boundary=");
	if (pTemp!=NULL) {
		pTemp+=9;
		if (*pTemp=='\"') pTemp++;
		while ((*pTemp)&&(*pTemp!='\"')&&(strlen(boundary)<sizeof(boundary)-1)) {
			boundary[strlen(boundary)]=*pTemp;
			pTemp++;
		}
	}
	memset(msgtype, 0, sizeof(msgtype));
	memset(msgencoding, 0, sizeof(msgencoding));
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		striprn(inbuffer);
		if (strlen(msgtype)) {
			if (strcmp(inbuffer, "")==0) break;
		}
		if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
			strncpy(msgtype, (char *)&inbuffer+14, sizeof(msgtype)-1);
			if (strcasestr(msgtype, "multipart")==NULL) continue;
		}
		if (strncasecmp(inbuffer, "Content-Transfer-Encoding:", 26)==0) {
			strncpy(msgencoding, (char *)&inbuffer+27, sizeof(msgencoding)-1);
		}
	}
	if (strncasecmp(msgtype, "text/plain", 10)==0||strncasecmp(msgtype, "text/html", 9)==0) {
		for (;;) {
			msgdone=1;
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcasestr(inbuffer, boundary)!=NULL) return msgdone;
			if (reply) prints("> ");
			if (strncasecmp(msgencoding, "quoted-printable", 16)==0) {
				DecodeQP(inbuffer);
			} else {
				printline(inbuffer);
			}
		}
	}
	return msgdone;
}

void webmailmime(wmheader *header, int nummessage, int reply)
{
	char filename[10][100];
	char inbuffer[1024];
	char ctype[100];
	char *pTemp;
	int sid=getsid();
	int file=0;
	int head=0;
	int msgdone=0;
	int numfiles=0;
	int i;

	if (strcasestr(header->contenttype, "multipart")==NULL) return;
	if (!reply) {
		prints("<FONT FACE=Arial, Helvetica>\n<PRE>");
	}
	for (;;) {
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		if (strcmp(inbuffer, ".")==0) break;
		if (strcasestr(inbuffer, header->boundary)!=NULL) {
			head=1;
			break;
		}
	}
	for (;;) {
		if (strcmp(inbuffer, ".")==0) break;
		if (head) {
			memset(ctype, 0, sizeof(ctype));
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
				if (strcmp(inbuffer, "")==0) {
					head=0;
					break;
				}
				if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
					pTemp=inbuffer+13;
					while ((*pTemp==' ')||(*pTemp=='\t')) pTemp++;
					strncpy(ctype, pTemp, sizeof(ctype)-1);
				}
				if (strcasestr(header->contenttype, "multipart")==NULL) continue;
				if (strcasestr(ctype, "multipart/alternative")!=NULL) {
					msgdone=retardedOEmime(sid, 0, inbuffer);
					head=1;
					break;
				}
				if (file) continue;
				pTemp=strcasestr(inbuffer, "name=");
				if (pTemp!=NULL) {
					pTemp+=5;
					if (*pTemp=='\"') pTemp++;
					while ((*pTemp)&&(*pTemp!='\"')&&(strlen(filename[numfiles])<sizeof(filename[numfiles])-1)) {
						filename[numfiles][strlen(filename[numfiles])]=*pTemp;
						pTemp++;
					}
					file=1;
					numfiles++;
				}
			}
		}
		if (strcmp(inbuffer, ".")==0) break;
		if (file) {
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
				if (strcmp(inbuffer, "")==0) {
					file=0;
					break;
				}
				if (strcasestr(inbuffer, header->boundary)!=NULL) {
					file=0;
					head=1;
					break;
				}
			}
		}
		if (strcmp(inbuffer, ".")==0) break;
		if (head) continue;
		if (strcasestr(ctype, "text/plain")!=NULL||strcasestr(ctype, "text/html")!=NULL) {
			for (;;) {
				if (strcmp(inbuffer, "")==0) break;
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
			}
			if (strcmp(inbuffer, ".")==0) break;
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
				if (strcasestr(inbuffer, header->boundary)!=NULL) {
					head=1;
					break;
				}
				if (msgdone) continue;
				if (reply) prints("> ");
				if (strcasestr(header->encoding, "quoted-printable")!=NULL) {
					DecodeQP(inbuffer);
				} else {
					printline(inbuffer);
				}
			}
			msgdone=1;
		} else if ((strcasestr(header->contenttype, "multipart/report")!=NULL)||strcasestr(ctype, "message/rfc822")!=NULL) {
			for (;;) {
				if (strcmp(inbuffer, "")==0) break;
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
			}
			if (strcmp(inbuffer, ".")==0) break;
			if (!reply) prints("<HR>\r\n");
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
				if (strcasestr(inbuffer, header->boundary)!=NULL) {
					head=1;
					break;
				}
				if (reply) continue;
				if (strcasestr(header->encoding, "quoted-printable")!=NULL) {
					DecodeQP(inbuffer);
				} else {
					printline(inbuffer);
				}
			}
		} else {
			for (;;) {
				if (strcmp(inbuffer, "")==0) break;
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
			}
			if (strcmp(inbuffer, ".")==0) break;
			for (;;) {
				wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
				if (strcmp(inbuffer, ".")==0) break;
				if (strcasestr(inbuffer, header->boundary)!=NULL) {
					head=1;
					break;
				}
				if (reply) continue;
				if (strcasestr(header->encoding, "quoted-printable")!=NULL) {
					DecodeQP(inbuffer);
				} else {
					printline(inbuffer);
				}
			}
		}
		if (strcmp(inbuffer, ".")==0) break;
	}
	if (!reply) {
		prints("</PRE></FONT><BR>");
		if (numfiles>0) {
			prints("Attachments<BR>\n");
			for (i=0;i<numfiles;i++) {
				prints("[<A HREF=%s/mailfile/%d/", conn[sid].dat->in_ScriptName, nummessage);
				hexprints("%s", filename[i]);
				prints(">");
				printht("%s", filename[i]);
				prints("</A>]<BR>\n");
			}
		}
	}
	return;
}

void webmaillist(int sid)
{
	wmheader header;
	char *pTemp;
	char inbuffer[1024];
	char status[8];
	char msgsize[100];
	int msize;
	int nummessages;
	int offset=0;
	int i;

	if (webmailconnect(sid)!=0) return;
	wmprints("STAT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset(status, 0, sizeof(status));
	pTemp=inbuffer;
	while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
		status[strlen(status)]=*pTemp;
		pTemp++;
	}
	nummessages=atoi(pTemp);
	if (nummessages<1) {
		prints("<CENTER>You have no messages in your mailbox.</CENTER><BR>\n");
		webmaildisconnect();
		return;
	}
	if (getgetenv(sid, "OFFSET")!=NULL) {
		offset=atoi(getgetenv(sid, "OFFSET"));
	}
	if (offset<0) offset=0;
	prints("<script language='JavaScript'>\n<!--\n");
	prints("function CheckAll()\n{\n");
	prints("for (var i=0;i<document.webmail.elements.length;i++) {\n");
	prints("var e = document.webmail.elements[i];\n");
	prints("if (e.name != 'allbox')\n");
	prints("e.checked = !e.checked;\n");
	prints("}\n}\n//-->\n</script>\n");
	if (nummessages>MAX_LIST_SIZE) {
		prints("<CENTER>\n");
		if (offset>0) {
			prints("[<A HREF=%s/maillist?offset=%d>Previous</A>]\n", conn[sid].dat->in_ScriptName, offset-MAX_LIST_SIZE);
		} else {
			prints("[Previous]\n");
		}
		if (offset+MAX_LIST_SIZE<nummessages) {
			prints("[<A HREF=%s/maillist?offset=%d>Next</A>]\n", conn[sid].dat->in_ScriptName, offset+MAX_LIST_SIZE);
		} else {
			prints("[Next]\n");
		}
		prints("</CENTER>\n");
	}
	prints("<FORM METHOD=POST NAME=webmail ACTION=%s/maildelete>\n", conn[sid].dat->in_ScriptName);
	prints("<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s>\n", COLOR_TRIM);
	prints("<TD>&nbsp;</TD>");
	prints("<TD><B>From</B></TD>");
	prints("<TD WIDTH=100%%><B>Subject</B></TD>");
	prints("<TD><B>Date</B></TD>");
	prints("<TD><B>Size</B></TD>");
	prints("<TD>&nbsp;</TD>");
	prints("</TR>\n");
        for (i=nummessages-offset-1;(i>-1)&&(i>nummessages-offset-MAX_LIST_SIZE-1);i--) {
		wmprints("LIST %d\r\n", i+1);
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		memset(status, 0, sizeof(status));
		pTemp=inbuffer;
		while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
			status[strlen(status)]=*pTemp;
			pTemp++;
		}
		pTemp++;
		while ((pTemp)&&(*pTemp!=' ')) {
			pTemp++;
		}
		msize=atoi(pTemp);
		if (strncasecmp(status, "+OK", 3)!=0) continue;
		if (msize>1048576) {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f M", (float)msize/1048576.0);
		} else {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f K", (float)msize/1024.0);
		}
		wmprints("TOP %d 0\r\n", i+1);
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		memset((char *)&header, 0, sizeof(header));
		if (webmailheader(sid, &header)!=0) return;
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			striprn(inbuffer);
			if (strcmp(inbuffer, ".")==0) break;
		}
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD NOWRAP><INPUT TYPE=checkbox NAME=%d VALUE=%d></TD>", i+1, i+1);
		if ((pTemp=strchr(header.From, '<'))!=NULL) *pTemp='\0';
		prints("<TD NOWRAP>%.25s&nbsp;</TD>", header.From);
		prints("<TD NOWRAP><A HREF=%s/mailread?msg=%d>%.40s&nbsp;</A></TD>", conn[sid].dat->in_ScriptName, i+1, header.Subject);
		prints("<TD NOWRAP>%s&nbsp;</TD>", getdate(header.Date));
		prints("<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", msgsize);
		if (strcasestr(header.contenttype, "multipart/mixed")!=NULL) {
			if (verifyimage("paperclip.gif")) {
				prints("<TD><IMG BORDER=0 SRC=/sentinel/images/paperclip.gif ALT='File Attachments'></TD>");
			} else {
				prints("<TD>*</TD>");
			}
		} else {
			prints("<TD>&nbsp;</TD>");
		}
		prints("</TR>\n");
	}
	prints("</TABLE>");
	prints("<INPUT TYPE=checkbox NAME=allbox VALUE=check_all onclick='CheckAll();'>\n");
	prints("<B>Select all messages</B><BR>\n");
	prints("<INPUT TYPE=SUBMIT VALUE='Delete Selected Messages'></FORM>\n</CENTER>\n");
	if (nummessages>MAX_LIST_SIZE) {
		prints("<CENTER>\n");
		if (offset>0) {
			prints("[<A HREF=%s/maillist?offset=%d>Previous</A>]\n", conn[sid].dat->in_ScriptName, offset-MAX_LIST_SIZE);
		} else {
			prints("[Previous]\n");
		}
		if (offset+MAX_LIST_SIZE<nummessages) {
			prints("[<A HREF=%s/maillist?offset=%d>Next</A>]\n", conn[sid].dat->in_ScriptName, offset+MAX_LIST_SIZE);
		} else {
			prints("[Next]\n");
		}
		prints("</CENTER>\n");
	}
	webmaildisconnect();
	return;
}

void webmailread(int sid)
{
	wmheader header;
	char inbuffer[1024];
	char status[8];
	char *pTemp;
	int nummessages;
	int nummessage;

	if (webmailconnect(sid)!=0) return;
	wmprints("STAT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset(status, 0, sizeof(status));
	pTemp=inbuffer;
	while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
		status[strlen(status)]=*pTemp;
		pTemp++;
	}
	nummessages=atoi(pTemp);
	nummessage=atoi(getgetenv(sid, "MSG"));
	if ((nummessage>nummessages)||(nummessage<1)) {
		prints("No such message.<BR>");
		webmaildisconnect();
		return;
	}
	prints("<CENTER>\n");
	if (nummessage>1) {
		prints("[<A HREF=%s/mailread?msg=%d>Previous</A>]\n", conn[sid].dat->in_ScriptName, nummessage-1);
	} else {
		prints("[Previous]\n");
	}
	prints("[<A HREF=%s/mailwrite?replyto=%d>Reply</A>]\n", conn[sid].dat->in_ScriptName, nummessage);
	prints("[<A HREF=%s/mailwrite?forward=%d>Forward</A>]\n", conn[sid].dat->in_ScriptName, nummessage);
	prints("[<A HREF=%s/maildelete?%d=%d>Delete</A>]\n", conn[sid].dat->in_ScriptName, nummessage, nummessage);
	if (nummessage<nummessages) {
		prints("[<A HREF=%s/mailread?msg=%d>Next</A>]\n", conn[sid].dat->in_ScriptName, nummessage+1);
	} else {
		prints("[Next]\n");
	}
	prints("<BR><TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	wmprints("RETR %d\r\n", nummessage);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset((char *)&header, 0, sizeof(header));
	if (webmailheader(sid, &header)!=0) return;
	prints("<TR><TD BGCOLOR=%s><B>From    </B></TD><TD BGCOLOR=%s WIDTH=100%%><A HREF=%s/mailwrite?msg=%d>", COLOR_TRIM, COLOR_FTEXT, conn[sid].dat->in_ScriptName, nummessage);
	printht("%s", header.From);
	prints("</A></TD></TR>\n");
	prints("<TR><TD BGCOLOR=%s><B>Subject </B></TD><TD BGCOLOR=%s WIDTH=100%%>", COLOR_TRIM, COLOR_FTEXT);
	printht("%s", header.Subject);
	prints("</TD></TR>\n");
	prints("<TR><TD BGCOLOR=%s><B>Date    </B></TD><TD BGCOLOR=%s WIDTH=100%%>", COLOR_TRIM, COLOR_FTEXT);
	printht("%s", header.Date);
	prints("</TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD COLSPAN=2>[<A HREF=%s/mailraw?msg=%d>View Source</A>]</TD></TR>\n", COLOR_FTEXT, conn[sid].dat->in_ScriptName, nummessage);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2>\n", COLOR_FTEXT);
	if (strcasestr(header.contenttype, "multipart")==NULL) {
		prints("<FONT FACE=Arial, Helvetica>\n<PRE>");
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcmp(inbuffer, ".")==0) break;
			if (strncasecmp(header.encoding, "quoted-printable", 16)==0) {
				DecodeQP(inbuffer);
			} else {
				printline(inbuffer);
			}
		}
		prints("</PRE></FONT><BR>");
	} else {
		webmailmime(&header, nummessage, 0);
	}
	prints("</TD></TR></TABLE>\n");
	if (nummessage>1) {
		prints("[<A HREF=%s/mailread?msg=%d>Previous</A>]\n", conn[sid].dat->in_ScriptName, nummessage-1);
	} else {
		prints("[Previous]\n");
	}
	prints("[<A HREF=%s/mailwrite?replyto=%d>Reply</A>]\n", conn[sid].dat->in_ScriptName, nummessage);
	prints("[<A HREF=%s/mailwrite?forward=%d>Forward</A>]\n", conn[sid].dat->in_ScriptName, nummessage);
	prints("[<A HREF=%s/maildelete?%d=%d>Delete</A>]\n", conn[sid].dat->in_ScriptName, nummessage, nummessage);
	if (nummessage<nummessages) {
		prints("[<A HREF=%s/mailread?msg=%d>Next</A>]\n", conn[sid].dat->in_ScriptName, nummessage+1);
	} else {
		prints("[Next]\n");
	}
	prints("<BR><BR>\n");
	webmaildisconnect();
	return;
}

void webmailwrite(int sid)
{
	wmheader header;
	char inbuffer[1024];
	char msgto[512];
	char subject[512];
	int replyto=0;
	int forward=0;

	if (webmailconnect(sid)!=0) return;
	memset(msgto, 0, sizeof(msgto));
	memset(subject, 0, sizeof(subject));
	if (getgetenv(sid, "REPLYTO")!=NULL) {
		replyto=atoi(getgetenv(sid, "REPLYTO"));
	}
	if (getgetenv(sid, "FORWARD")!=NULL) {
		forward=atoi(getgetenv(sid, "FORWARD"));
	}
	if (getgetenv(sid, "MSGTO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "MSGTO"), sizeof(msgto)-1);
	}
	if ((replyto>0)||(forward>0)) {
		if (replyto>0) {
			wmprints("RETR %d\r\n", replyto);
		} else {
			wmprints("RETR %d\r\n", forward);
		}
		wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
		memset((char *)&header, 0, sizeof(header));
		if (webmailheader(sid, &header)!=0) return;
		snprintf(msgto, sizeof(msgto)-1, "%s", header.Replyto);
		if (replyto>0) {
			if (strncasecmp(header.Subject, "RE:", 3)!=0) {
				snprintf(subject, sizeof(subject)-1, "Re: %s", header.Subject);
			} else {
				snprintf(subject, sizeof(subject)-1, "%s", header.Subject);
			}
		} else if (forward>0) {
			snprintf(subject, sizeof(subject)-1, "Fwd: %s", header.Subject);
		}
	}
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/mailsend NAME=wmcompose ENCTYPE=multipart/form-data>\n", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TD><B>To</B></TD><TD><INPUT TYPE=TEXT NAME=msgto VALUE=\"%s\" SIZE=50></TD></TR>\n", COLOR_EDITFORM, str2html(msgto));
	prints("<TR BGCOLOR=%s><TD><B>Subject</B></TD><TD><INPUT TYPE=TEXT NAME=msgsubject VALUE=\"%s\" SIZE=50></TD></TR>\n", COLOR_EDITFORM, str2html(subject));
	prints("<TR BGCOLOR=%s><TD><B>CC</B></TD><TD><INPUT TYPE=TEXT NAME=msgcc VALUE=\"\" SIZE=50></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD><B>BCC</B></TD><TD><INPUT TYPE=TEXT NAME=msgbcc VALUE=\"\" SIZE=50></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD><B>File</B></TD><TD><INPUT TYPE=FILE NAME=fattach SIZE=50></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA NAME=msgbody COLS=70 ROWS=20 WRAP=hard>\n", COLOR_EDITFORM);
	if ((replyto<1)&&(forward<1)) {
		prints("</TEXTAREA></TD></TR>\n");
		prints("</TABLE>\n");
		prints("<INPUT TYPE=SUBMIT VALUE='Send Mail'>\n");
		prints("</FORM>\n</CENTER>\n");
		prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmcompose.msgto.focus();\n// -->\n</SCRIPT>\n");
		webmaildisconnect();
		return;
	}
	prints("--- %s wrote:\n", header.From);
	if (forward>0) {
		prints("> From:    %s\n", header.From);
		prints("> Subject: %s\n", header.Subject);
		prints("> Date:    %s\n", header.Date);
		prints(">\n");
	}
	if (strcasestr(header.contenttype, "multipart")==NULL) {
		for (;;) {
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strcmp(inbuffer, ".")==0) break;
			prints("> ");
			if (strncasecmp(header.encoding, "quoted-printable", 16)==0) {
				DecodeQP(inbuffer);
			} else {
				prints("%s\r\n", inbuffer);
			}
		}
	} else {
		if (replyto>0) {
			webmailmime(&header, replyto, 1);
		} else {
			webmailmime(&header, forward, 1);
		}
	}
	prints("</TEXTAREA></TD></TR>\n");
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT VALUE='Send Mail'>\n");
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmcompose.msgto.focus();\n// -->\n</SCRIPT>\n");
	webmaildisconnect();
	return;
}

void webmailsend(int sid)
{
	struct hostent *hp;
	struct sockaddr_in server;
	char *filebody=NULL;
	char *msgbody=NULL;
	char *pmsgbody;
	char *pmsgcc;
	char boundary[100];
	char inbuffer[1024];
	char filename[1024];
	char cfilesize[10];
	char msgto[64];
	char msgcc[1024];
	char msgbcc[1024];
	char msgtocc[64];
	char msgsubject[64];
	char msgdate[100];
	char msgdatetz[100];
	char line[1024];
	char wmaddress[1024];
	char wmrealname[1024];
	int filesize=0;
	struct timeval ttime;
	struct timezone tzone;
	time_t t;
	int sqr;
	unsigned int mimesize;

	t=time((time_t*)0);
	memset(boundary, 0, sizeof(boundary));
	snprintf(boundary, sizeof(boundary)-1, "------------SGW%d", (int)t);
	if (strcasecmp(conn[sid].dat->in_RequestMethod, "POST")!=0) return;
	if (getmimeenv(sid, "MSGTO", &mimesize)==NULL) {
		prints("<CENTER>No recipient specified - Message was not sent</CENTER>\n");
		return;
	}
	memset(msgto, 0, sizeof(msgto));
	memset(msgcc, 0, sizeof(msgcc));
	memset(msgbcc, 0, sizeof(msgbcc));
	memset(msgsubject, 0, sizeof(msgsubject));
	if (msgbody!=NULL) {
		free(msgbody);
		msgbody=NULL;
	}
	msgbody=calloc(conn[sid].dat->in_ContentLength+1024, sizeof(char));
	if (getmimeenv(sid, "MSGTO", &mimesize)!=NULL) {
		strncpy(msgto, getmimeenv(sid, "MSGTO", &mimesize), sizeof(msgto)-1);
		if (mimesize<strlen(msgto)) msgto[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGCC", &mimesize)!=NULL) {
		strncpy(msgcc, getmimeenv(sid, "MSGCC", &mimesize), sizeof(msgcc)-1);
		if (mimesize<strlen(msgcc)) msgcc[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGBCC", &mimesize)!=NULL) {
		strncpy(msgbcc, getmimeenv(sid, "MSGBCC", &mimesize), sizeof(msgbcc)-1);
		if (mimesize<strlen(msgbcc)) msgbcc[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGSUBJECT", &mimesize)!=NULL) {
		strncpy(msgsubject, getmimeenv(sid, "MSGSUBJECT", &mimesize), sizeof(msgsubject)-1);
		if (mimesize<strlen(msgsubject)) msgsubject[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGBODY", &mimesize)!=NULL) {
		strncpy(msgbody, getmimeenv(sid, "MSGBODY", &mimesize), conn[sid].dat->in_ContentLength+1023);
		if (mimesize<strlen(msgbody)) msgbody[mimesize]='\0';
	}
	memset(filename, 0, sizeof(filename));
	memset(cfilesize, 0, sizeof(cfilesize));
	if (getmimeenv(sid, "FATTACH", &mimesize)!=NULL) {
		filebody=webmailfileul(sid, filename, cfilesize);
		filesize=atoi(cfilesize);
		if (strlen(filename)==0) filesize=0;
	}
	gettimeofday(&ttime, &tzone);
	strftime(msgdate, sizeof(msgdate), "%a, %d %b %Y %H:%M:%S", localtime(&ttime.tv_sec));
	snprintf(msgdatetz, sizeof(msgdatetz)-1, " %+.4d", -tzone.tz_minuteswest/60*100);
	strncat(msgdate, msgdatetz, sizeof(msgdate)-strlen(msgdate)-1);
	/* some smtp servers like pop auth before smtp */
	if (webmailconnect(sid)!=0) return;
	webmaildisconnect();
	memset(wmaddress, 0, sizeof(wmaddress));
	memset(wmrealname, 0, sizeof(wmrealname));
	if ((sqr=sqlQueryf("SELECT firstname, lastname, emailusername, emailpassword, pop3server, smtpserver, emailaddress FROM gw_users where username = '%s'", conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)==1) {
		strncpy(wmrealname, sqlGetvalue(sqr, 0, 0), sizeof(wmrealname)-1);
		if ((strlen(sqlGetvalue(sqr, 0, 0))>0)&&(strlen(sqlGetvalue(sqr, 0, 1))>0)) {
			strcat(wmrealname, " ");
			strncat(wmrealname, sqlGetvalue(sqr, 0, 1), sizeof(wmrealname)-sizeof(sqlGetvalue(sqr, 0, 0)));
			strcat(wmrealname, " ");
		}
		strncpy(conn[sid].dat->wm_username, sqlGetvalue(sqr, 0, 2), sizeof(conn[sid].dat->wm_username)-1);
		strncpy(conn[sid].dat->wm_password, sqlGetvalue(sqr, 0, 3), sizeof(conn[sid].dat->wm_password)-1);
		strncpy(conn[sid].dat->wm_pop3server, sqlGetvalue(sqr, 0, 4), sizeof(conn[sid].dat->wm_pop3server)-1);
		strncpy(conn[sid].dat->wm_smtpserver, sqlGetvalue(sqr, 0, 5), sizeof(conn[sid].dat->wm_smtpserver)-1);
		strncpy(wmaddress, sqlGetvalue(sqr, 0, 6), sizeof(wmaddress)-1);
		if (strlen(wmrealname)==0) {
			strncpy(wmrealname, sqlGetvalue(sqr, 0, 6), sizeof(wmrealname)-1);
		}
	}
	sqlFreeconnect(sqr);
	if ((strlen(conn[sid].dat->wm_username)==0)||(strlen(conn[sid].dat->wm_password)==0)||(strlen(conn[sid].dat->wm_pop3server)==0)||(strlen(conn[sid].dat->wm_smtpserver)==0)) {
		wmloginform(sid);
		return;
	}
	if (!(hp=gethostbyname(conn[sid].dat->wm_smtpserver))) {
		prints("<CENTER>Could not find SMTP server '%s'</CENTER>\n", conn[sid].dat->wm_smtpserver);
		return;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=(unsigned short)htons(smtpport);
	if ((conn[sid].dat->wm_socket=socket(AF_INET, SOCK_STREAM, 0))<0) return;
	setsockopt(conn[sid].dat->wm_socket, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if (connect(conn[sid].dat->wm_socket, (struct sockaddr *)&server, sizeof(server))<0) {
		prints("<CENTER>Could not connect to SMTP server '%s'</CENTER>\n", conn[sid].dat->wm_smtpserver);
		return;
	}
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	wmprints("HELO %s\r\n", conn[sid].dat->wm_smtpserver);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	wmprints("MAIL From: %s\r\n", wmaddress);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	prints("Sender '%s' OK.<BR>\n", wmaddress);
	flushbuffer(sid);
	wmprints("RCPT To: %s\r\n", msgto);
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	prints("Recipient '%s' OK.<BR>\n", msgto);
	flushbuffer(sid);
	if (strlen(msgcc)) {
		pmsgcc=msgcc;
		while (*pmsgcc) {
			if (strstr(pmsgcc, "@")==NULL) break;
			memset(msgtocc, 0, sizeof(msgtocc));
			while ((*pmsgcc)&&(*pmsgcc!=',')&&(*pmsgcc!=' ')&&(strlen(pmsgcc)<sizeof(msgtocc))) {
				msgtocc[strlen(msgtocc)]=*pmsgcc;
				pmsgcc++;
			}
			while ((*pmsgcc==',')||(*pmsgcc==' ')) {
				pmsgcc++;
			}
			while (!isalpha(msgtocc[strlen(msgtocc)-1])) {
				msgtocc[strlen(msgtocc)-1]='\0';
			}
			wmprints("RCPT To: %s\r\n", msgtocc);
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
			prints("Recipient '%s' OK.<BR>\n", msgtocc);
			flushbuffer(sid);
		}
	}
	if (strlen(msgbcc)) {
		pmsgcc=msgbcc;
		while (*pmsgcc) {
			if (strstr(pmsgcc, "@")==NULL) break;
			memset(msgtocc, 0, sizeof(msgtocc));
			while ((*pmsgcc)&&(*pmsgcc!=',')&&(*pmsgcc!=' ')&&(strlen(pmsgcc)<sizeof(msgtocc))) {
				msgtocc[strlen(msgtocc)]=*pmsgcc;
				pmsgcc++;
			}
			while ((*pmsgcc==',')||(*pmsgcc==' ')) {
				pmsgcc++;
			}
			while (!isalpha(msgtocc[strlen(msgtocc)-1])) {
				msgtocc[strlen(msgtocc)-1]='\0';
			}
			wmprints("RCPT To: %s\r\n", msgtocc);
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
			prints("Recipient '%s' OK.<BR>\n", msgtocc);
			flushbuffer(sid);
		}
	}
	wmprints("DATA\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "354", 3)!=0) goto quit;
	wmprints("From: %s <%s>\r\n", wmrealname, wmaddress);
	wmprints("To: <%s>\r\n", msgto);
	if (strlen(msgcc)) {
		wmprints("Cc: %s\r\n", msgcc);
	}
	wmprints("Subject: %s\r\n", msgsubject);
	wmprints("Date: %s\r\n", msgdate);
	if (filesize>0) {
		wmprints("MIME-Version: 1.0\r\n");
		wmprints("Content-Type: multipart/mixed; boundary=\"%s\"\r\n", boundary);
	}
	wmprints("X-Mailer: %s\r\n", SERVER_NAME);
	wmprints("\r\n");
	pmsgbody=msgbody;
	if (filesize>0) {
		wmprints("This is a multi-part message in MIME format.\r\n");
		wmprints("--%s\r\n", boundary);
		wmprints("Content-Type: text/plain; charset=us-ascii\r\n");
		wmprints("Content-Transfer-Encoding: 8bit\r\n\r\n");
	}
	while (strlen(pmsgbody)>80) {
		memset(line, 0, sizeof(line));
		snprintf(line, 80, "%s", pmsgbody);
		if (strstr(line, "\r\n")) {
			while (line[strlen(line)-1]!='\r') {
				line[strlen(line)-1]='\0';
			}
			wmprints("%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else if (strchr(line, ' ')!=NULL) {
			while ((line[strlen(line)-1]!=' ')&&(strlen(line)>0)) {
				line[strlen(line)-1]='\0';
			}
			wmprints("%s\r\n", line);
			pmsgbody+=strlen(line);
		} else {
			memset(line, 0, sizeof(line));
			while (strlen(line)<sizeof(line)-1) {
				if (line[strlen(line)-1]==' ') {
					line[strlen(line)-1]='\0';
					break;
				}
				if (line[strlen(line)-1]=='\r') {
					line[strlen(line)-1]='\0';
					break;
				}
				line[strlen(line)]=pmsgbody[strlen(line)];
			}
			wmprints("%s\r\n", line);
			pmsgbody+=strlen(line);
		}
	}
	memset(line, 0, sizeof(line));
	snprintf(line, 80, "%s", pmsgbody);
	wmprints("%s\r\n", line);
	pmsgbody+=strlen(line);
	free(msgbody);
	msgbody=NULL;
	if (filesize>0) {
		prints("Sending file '%s' (%d bytes)<BR>\n", filename, filesize);
		flushbuffer(sid);
		wmprints("\r\n--%s\r\n", boundary);
		wmprints("Content-Type: application/octet-stream; name=\"%s\"\r\n", filename);
		wmprints("Content-Transfer-Encoding: base64\r\n");
		wmprints("Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", filename);
		EncodeBase64(filebody, filesize);
		prints("<BR>\r\n");
		wmprints("\r\n--%s--\r\n", boundary);
	}
	wmprints("\r\n.\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	if (strncasecmp(inbuffer, "250", 3)==0) {
		prints("<BR>Message Successfully Sent.<BR>\n", inbuffer);
		flushbuffer(sid);
	}
	memset(inbuffer, 0, sizeof(inbuffer));
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/maillist\">\n", conn[sid].dat->in_ScriptName);
quit:
	prints("%s<BR>\n", inbuffer);
	wmprints("QUIT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	flushbuffer(sid);
	wmcloseconn();
	return;
}

void webmaildelete(int sid)
{
	char *pTemp;
	char inbuffer[1024];
	char status[8];
	char msgnum[8];
	int nummessages;
	int deleted=0;
	int i;

	if (webmailconnect(sid)!=0) return;
	wmprints("STAT\r\n");
	wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
	memset(status, 0, sizeof(status));
	pTemp=inbuffer;
	while ((pTemp)&&(*pTemp!=' ')&&(strlen(status)<sizeof(status)-1)) {
		status[strlen(status)]=*pTemp;
		pTemp++;
	}
	nummessages=atoi(pTemp);
	if (nummessages>0) {
		for (i=nummessages;i>0;i--) {
			snprintf(msgnum, sizeof(msgnum)-1, "%d", i);
			if ((getpostenv(sid, msgnum)==NULL)&&(getgetenv(sid, msgnum)==NULL)) continue;
			if ((getpostenv(sid, msgnum)!=NULL)&&(atoi(getpostenv(sid, msgnum))!=i)) continue;
			if ((getgetenv(sid, msgnum)!=NULL)&&(atoi(getgetenv(sid, msgnum))!=i)) continue;
			deleted=i;
			prints("Deleting message %d...", i);
			wmprints("DELE %d\r\n", i);
			wmfgets(inbuffer, sizeof(inbuffer)-1, conn[sid].dat->wm_socket);
			if (strncasecmp(inbuffer, "+OK", 3)==0) {
				prints("success.<BR>\n");
			} else {
				prints("failure.<BR>\n");
			}
			flushbuffer(sid);
		}
	} else {
		prints("<CENTER>You have no messages in your mailbox.</CENTER><BR>\n");
	}
	webmaildisconnect();
	snprintf(msgnum, sizeof(msgnum)-1, "%d", deleted);
	if (deleted>=nummessages) deleted=nummessages-1;
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("location.replace(\"");
	if ((getgetenv(sid, msgnum)==NULL)||(nummessages<2)) {
		prints("%s/maillist\");\n", conn[sid].dat->in_ScriptName);
	} else {
		prints("%s/mailread?msg=%d\");\n", conn[sid].dat->in_ScriptName, deleted);
	}
	prints("// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; ");
	if ((getgetenv(sid, msgnum)==NULL)||(nummessages<2)) {
		prints("URL=%s/maillist\">\n", conn[sid].dat->in_ScriptName);
	} else {
		prints("URL=%s/mailread?msg=%d\">\n", conn[sid].dat->in_ScriptName, deleted);
	}
	prints("</NOSCRIPT>\n");
	return;
}

void webmailmain(int sid)
{
	if (strncmp(conn[sid].dat->in_RequestURI, "/mailfile", 9)==0) {
		webmailfiledl(sid);
		return;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(conn[sid].dat->in_RequestURI, "/mailquit", 9)==0) {
		wmlogout(sid);
	}
	printheader(sid, "Sentinel Groupware Webmail");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch11-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' HREF=%s/maillist>INBOX</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' HREF=%s/mailwrite>COMPOSE</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' HREF=%s/mailquit>QUIT</a>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/mailquit", 9)==0) {
		wmloginform(sid);
		printfooter(sid);
		return;
	}
	if (strncmp(conn[sid].dat->in_RequestURI, "/mailread", 9)==0)
		webmailread(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mailwrite", 10)==0)
		webmailwrite(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/maillist", 9)==0)
		webmaillist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mailsend", 9)==0)
		webmailsend(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/maildelete", 11)==0)
		webmaildelete(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mailraw", 8)==0)
		webmailraw(sid);
	printfooter(sid);
	return;
}
