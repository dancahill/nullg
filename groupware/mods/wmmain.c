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

void wmnotice(CONNECTION *sid)
{
	int i;
	int lastcount;
	int nummessages;
	int sqr;
	int sqr2;
	int totalmessages=0;

	if (!(auth_priv(sid, AUTH_WEBMAIL)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Groupware E-Mail Notice</FONT></B>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=left WIDTH=100%%>Account Name</TH><TH>New</TH><TH>Total</TH></TR>\n", COLOR_TH);
	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname, poppassword FROM gw_mailaccounts where obj_uid = %d", sid->dat->user_uid))<0) return;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		if (strlen(sqlGetvalue(sqr, i, 2))==0) continue;
		sid->dat->user_mailcurrent=atoi(sqlGetvalue(sqr, i, 0));
		if (wmserver_connect(sid, 0)==0) {
			nummessages=wmserver_count(sid);
			wmserver_disconnect(sid);
			if ((sqr2=sqlQueryf(sid, "SELECT lastcount FROM gw_mailaccounts WHERE obj_uid = %d AND mailaccountid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent))>=0) {
				lastcount=atoi(sqlGetvalue(sqr2, 0, 0));
			} else {
				lastcount=0;
			}
			sqlFreeconnect(sqr2);
			if (lastcount>nummessages) {
				lastcount=nummessages;
				sqlUpdatef(sid, "UPDATE gw_mailaccounts SET lastcount = %d WHERE obj_uid = %d AND mailaccountid = %d", nummessages, sid->dat->user_uid, sid->dat->user_mailcurrent);
			}
			prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT NOWRAP>", COLOR_FVAL);
			prints(sid, "<A HREF=%s/mail/main?accountid=%d TARGET=gwmain>%-.25s</A>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), sqlGetvalue(sqr, i, 1));
			prints(sid, "&nbsp;</TD><TD ALIGN=RIGHT NOWRAP>%d</TD><TD ALIGN=RIGHT NOWRAP>%d</TD></TR>\n", nummessages-lastcount, nummessages);
			if (nummessages-lastcount>0) totalmessages+=nummessages-lastcount;
		}
	}
	sqlFreeconnect(sqr);
	prints(sid, "</TABLE></CENTER>\n");
	if (totalmessages>0) {
		prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
	}
	return;
}

void wmloginform(CONNECTION *sid)
{
	char msgto[512];
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	if (getgetenv(sid, "MSGTO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "MSGTO"), sizeof(msgto)-1);
	}
	/* don't ask...  heh..  it was either this or a goto */
	for (;;) {
		if ((sqr=sqlQueryf(sid, "SELECT popusername, poppassword FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) break;
		if (sqlNumtuples(sqr)==1) {
			strncpy(sid->dat->user_wmusername, sqlGetvalue(sqr, 0, 0), sizeof(sid->dat->user_wmusername)-1);
			strncpy(sid->dat->user_wmpassword, DecodeBase64string(sid, sqlGetvalue(sqr, 0, 1)), sizeof(sid->dat->user_wmpassword)-1);
		}
		sqlFreeconnect(sqr);
		break;
	}
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/", sid->dat->in_ScriptName);
	if (strlen(msgto)>0) {
		prints(sid, "write?msgto=%s", msgto);
	} else {
		prints(sid, "list");
	}
	prints(sid, " AUTOCOMPLETE=OFF NAME=wmlogin>\n");
	prints(sid, "<TABLE CELLPADDING=2 CELLSPACING=0 BORDER=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Webmail Login</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD><B>Username</B></TD><TD><INPUT TYPE=TEXT NAME=WMUSERNAME SIZE=25 VALUE='%s'></TD></TR>\n", COLOR_EDITFORM, sid->dat->user_wmusername);
	prints(sid, "<TR BGCOLOR=%s><TD><B>Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=WMPASSWORD SIZE=25 VALUE='%s'></TD></TR>\n", COLOR_EDITFORM, sid->dat->user_wmpassword);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT VALUE='Login'></CENTER></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</TABLE></FORM></CENTER>\n");
	if (strlen(sid->dat->user_wmusername)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.WMUSERNAME.focus();\n// -->\n</SCRIPT>\n");
	} else {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.WMPASSWORD.focus();\n// -->\n</SCRIPT>\n");
	}
}

void wmlogout(CONNECTION *sid)
{
	char timebuffer[32];

	memset(timebuffer, 0, sizeof(timebuffer));
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, time(NULL)));
	sqlUpdatef(sid, "UPDATE gw_mailaccounts SET obj_mtime = '%s',  poppassword = '' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->user_uid, sid->dat->user_mailcurrent);
	return;
}

int webmailheader(CONNECTION *sid, wmheader *header)
{
	char inbuffer[1024];
	char *prevheader=NULL;
	char *ptemp;
	int prevheadersize=0;

	header->status='n';
	for (;;) {
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		striprn(inbuffer);
		if (strcmp(inbuffer, "")==0) break;
		if (strncasecmp(inbuffer, "Reply-To:", 9)==0) {
			prevheader=header->Replyto;
			prevheadersize=sizeof(header->Replyto);
			ptemp=inbuffer+9;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			while ((*ptemp)&&(*ptemp!='<')) ptemp++;
			if (*ptemp=='<') ptemp++;
			while ((*ptemp)&&(*ptemp!='>')&&(strlen(header->Replyto)<sizeof(header->Replyto)-1)) {
				header->Replyto[strlen(header->Replyto)]=*ptemp;
				ptemp++;
			}
		} else if (strncasecmp(inbuffer, "From:", 5)==0) {
			prevheader=header->From;
			prevheadersize=sizeof(header->From);
			ptemp=inbuffer+5;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->From, ptemp, sizeof(header->From)-1);
		} else if (strncasecmp(inbuffer, "To:", 3)==0) {
			prevheader=header->To;
			prevheadersize=sizeof(header->To);
			ptemp=inbuffer+3;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->To, ptemp, sizeof(header->To)-1);
		} else if (strncasecmp(inbuffer, "CC:", 3)==0) {
			prevheader=header->CC;
			prevheadersize=sizeof(header->CC);
			ptemp=inbuffer+3;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->CC, ptemp, sizeof(header->CC)-1);
		} else if (strncasecmp(inbuffer, "Subject:", 8)==0) {
			prevheader=header->Subject;
			prevheadersize=sizeof(header->Subject);
			ptemp=inbuffer+8;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->Subject, ptemp, sizeof(header->Subject)-1);
		} else if (strncasecmp(inbuffer, "Date:", 5)==0) {
			prevheader=header->Date;
			prevheadersize=sizeof(header->Date);
			ptemp=inbuffer+5;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->Date, ptemp, sizeof(header->Date)-1);
		} else if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
			prevheader=header->contenttype;
			prevheadersize=sizeof(header->contenttype);
			ptemp=inbuffer+13;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->contenttype, ptemp, sizeof(header->contenttype)-1);
		} else if (strncasecmp(inbuffer, "Content-Transfer-Encoding: ", 26)==0) {
			prevheader=header->encoding;
			prevheadersize=sizeof(header->encoding);
			ptemp=inbuffer+26;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->encoding, ptemp, sizeof(header->encoding)-1);
		} else if (strncasecmp(inbuffer, "Status:", 7)==0) {
			prevheader=NULL;
			prevheadersize=sizeof(header->status);
			ptemp=inbuffer+7;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			if (strchr(ptemp, 'R')!=NULL) header->status='r';
		} else if ((inbuffer[0]==' ')||(inbuffer[0]=='\t')) {
			if (prevheader==NULL) continue;
			strncat(prevheader, inbuffer, prevheadersize-strlen(prevheader)-1);
		} else {
			prevheader=NULL;
			prevheadersize=0;
		}
	}
	if ((ptemp=strcasestr(header->contenttype, "boundary="))!=NULL) {
		ptemp+=9;
		if (*ptemp==' ') ptemp++;
		if (*ptemp=='\"') ptemp++;
		snprintf(header->boundary, sizeof(header->boundary)-1, "--");
		while ((*ptemp)&&(*ptemp!='\"')&&(strlen(header->boundary)<sizeof(header->boundary)-1)) {
			header->boundary[strlen(header->boundary)]=*ptemp;
			ptemp++;
		}
	}
	if (strlen(header->Replyto)==0) {
		ptemp=header->From;
		while ((*ptemp)&&(*ptemp!='<')) ptemp++;
		if (*ptemp=='<') ptemp++;
		while ((*ptemp)&&(*ptemp!='>')&&(strlen(header->Replyto)<sizeof(header->Replyto)-1)) {
			header->Replyto[strlen(header->Replyto)]=*ptemp;
			ptemp++;
		}
		if (strlen(header->Replyto)==0) {
			strncpy(header->Replyto, header->From, sizeof(header->Replyto)-1);
		}
	}
	if (strlen(header->From)==0) strcpy(header->From, "(No Sender Name)");
	if (strlen(header->Subject)==0) strcpy(header->Subject, "(No Subject)");
	if (strlen(header->Date)==0) strcpy(header->Date, "(No Date)");
	return 0;
}

void webmailraw(CONNECTION *sid)
{
	char inbuffer[1024];
	int nummessages;
	int nummessage;

	send_header(sid, 1, 200, "OK", "1", "text/plain", -1, -1);
	if (wmserver_connect(sid, 1)!=0) return;
	nummessages=wmserver_count(sid);
	nummessage=atoi(getgetenv(sid, "MSG"));
	if ((nummessage>nummessages)||(nummessage<1)) {
		prints(sid, "No such message.<BR>");
		wmserver_disconnect(sid);
		return;
	}
	wmserver_msgretr(sid, nummessage);
	prints(sid, "<PRE>\r\n");
	for (;;) {
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (strcmp(inbuffer, "")==0) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "\r\n");
	for (;;) {
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (is_msg_end(sid, inbuffer)) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "</PRE>\r\n");
	wmserver_disconnect(sid);
	return;
}

void webmailfiledl(CONNECTION *sid)
{
	wmheader header;
	char *pQueryString;
	char *ptemp;
	char contentencoding[512];
	char contentfilename[512];
	char filename[512];
	char fnamebuf[512];
	char inbuffer[1024];
	char msgtype[100];
	int nummessages;
	int nummessage=0;

	memset(filename, 0, sizeof(filename));
	pQueryString=strstr(sid->dat->in_RequestURI, "/mail/file/");
	if (pQueryString==NULL) {
		send_error(sid, 400, "Bad Request", "Malformed URI");
		return;
	}
	pQueryString+=11;
	nummessage=atoi(pQueryString);
	while ((isdigit(*pQueryString)!=0)&&(*pQueryString!=0)) pQueryString++;
	while (*pQueryString=='/') pQueryString++;
	strncpy(filename, pQueryString, sizeof(filename)-1);
	ptemp=filename;
	decodeurl(ptemp);
	if ((nummessage<1)||(strlen(filename)<1)) {
		send_error(sid, 400, "Bad Request", "No such message.");
		return;
	}
	if (wmserver_connect(sid, 1)!=0) return;
	nummessages=wmserver_count(sid);
	if (nummessages<nummessage) {
		send_error(sid, 400, "Bad Request", "No such message.");
		goto quit;
	}
	wmserver_msgretr(sid, nummessage);
	memset((char *)&header, 0, sizeof(header));
	memset(msgtype, 0, sizeof(msgtype));
	if (webmailheader(sid, &header)!=0) return;
	if (strcasestr(header.contenttype, "multipart")==NULL) {
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) break;
		}
		send_error(sid, 400, "Bad Request", "No files are attached to this message.");
		goto quit;
	}
	for (;;) {
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (is_msg_end(sid, inbuffer)) break;
		if (strncasecmp(inbuffer, header.boundary, strlen(header.boundary))==0) {
			memset(contentencoding, 0, sizeof(contentencoding));
			memset(contentfilename, 0, sizeof(contentfilename));
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
		if ((strncasecmp(inbuffer, "Content-Type:", 13)==0)||(strncasecmp(inbuffer, "Content-Disposition:", 20)==0)) {
			while (inbuffer[strlen(inbuffer)-1]==';') {
				wmfgets(sid, inbuffer+strlen(inbuffer), sizeof(inbuffer)-strlen(inbuffer)-1, sid->dat->user_wmsocket);
				striprn(inbuffer);
			}
			ptemp=strcasestr(inbuffer, "name=");
			if ((ptemp!=NULL)&&(strlen(contentfilename)==0)) {
				ptemp+=5;
				if (*ptemp=='\"') ptemp++;
				memset(fnamebuf, 0, sizeof(fnamebuf));
				while ((*ptemp)&&(*ptemp!='\"')&&(strlen(fnamebuf)<sizeof(fnamebuf)-1)) {
					fnamebuf[strlen(fnamebuf)]=*ptemp;
					ptemp++;
				}
				snprintf(contentfilename, sizeof(contentfilename)-1, "%s", DecodeRFC2047(sid, fnamebuf));
				if (strcmp(contentfilename, filename)!=0) {
					memset(contentencoding, 0, sizeof(contentencoding));
					memset(contentfilename, 0, sizeof(contentfilename));
				}
			}
		}
		if ((strlen(contentfilename)>0)&&(strcmp(inbuffer, "")==0)) break;
	}
	if (strlen(contentencoding)==0) {
		snprintf(contentencoding, sizeof(contentencoding)-1, "8bit");
	}
	if ((strlen(contentencoding))&&(strlen(contentfilename))) {
		ptemp=get_mime_type(contentfilename);
		if (strncasecmp(ptemp, "image/", 6)!=0) {
			snprintf(sid->dat->out_ContentDisposition, sizeof(sid->dat->out_ContentDisposition)-1, "attachment");
		}
		send_header(sid, 1, 200, "OK", "1", ptemp, -1, -1);
		flushbuffer(sid);
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) goto quit;
			if (strncasecmp(inbuffer, header.boundary, strlen(header.boundary))==0) break;
			if (strncasecmp(contentencoding, "base64", 6)==0) {
				DecodeBase64(sid, inbuffer, "");
			} else if (strncasecmp(contentencoding, "quoted-printable", 16)==0) {
				DecodeQP(0, inbuffer, "");
			} else if (strncasecmp(contentencoding, "7bit", 4)==0) {
				DecodeText(0, inbuffer);
			} else if (strncasecmp(contentencoding, "8bit", 4)==0) {
				DecodeText(0, inbuffer);
			}
		}
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) goto quit;
		}
	} else {
		send_error(sid, 400, "Bad Request", "Attachment not found.");
		goto quit;
	}
quit:
	wmserver_disconnect(sid);
	return;
}

char *webmailfileul(CONNECTION *sid, char *xfilename, char *xfilesize)
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
	char *ptemp;
	int i;
	unsigned int j;

	pPostData=sid->PostData;
	memset(boundary, 0, sizeof(boundary));
	memset(location, 0, sizeof(location));
	i=0;
	j=0;
	/* duhh..  this should just retrieve the boundary from sid->dat->in_ContentType,
	 * and maybe check same to make sure this really is MIME, while it's at it.
	 */
	/* get the mime boundary */
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(strlen(boundary)<sizeof(boundary)-1)) {
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
	pPostData=sid->PostData;
	while ((strcmp(line, boundary2)!=0)&&(i<sid->dat->in_ContentLength)) {
		memset(line, 0, sizeof(line));
		j=0;
		while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(strlen(line)<sizeof(line)-1)) {
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
		ptemp=line;
		if (strncasecmp(line, "Content-Disposition: form-data; ", 32)!=0) continue;
		ptemp+=32;
		if (strncasecmp(ptemp, "name=\"fattach\"; ", 16)!=0) continue;
		ptemp+=16;
		if (strncasecmp(ptemp, "filename=\"", 10)!=0) continue;
		ptemp+=10;
		if (strrchr(ptemp, '\\')!=NULL) {
			ptemp=strrchr(ptemp, '\\')+1;
		}
		snprintf(lfilename, sizeof(lfilename)-1, "%s", ptemp);
		while (lfilename[strlen(lfilename)-1]=='\"') lfilename[strlen(lfilename)-1]='\0';
		while ((strncmp(pPostData, "\r\n\r\n", 4)!=0)&&(strncmp(pPostData, "\n\n", 2)!=0)&&(i<sid->dat->in_ContentLength)) {
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
		while ((strncmp(pPostData, boundary3, strlen(boundary3))!=0)&&(i<sid->dat->in_ContentLength)) {
			pPostData++;
			filesize++;
		}
	}
	snprintf(xfilesize, 9, "%d", filesize);
	return filebody;
}

int webmailmime(CONNECTION *sid, char *contenttype, char *encoding, char *boundary, short int nummessage, short int reply, short int depth)
{
	char inbuffer[1024];
	char cencode[100];
	char ctype[200];
	char cbound[100];
	char tbound[100];
	char *ptemp;
	int file=0;
	int head=0;
	int msgdone=0;
	int i;

	memset(sid->dat->user_wmfiles, 0, sizeof(sid->dat->user_wmfiles));
	depth++;
	if (strcasestr(contenttype, "multipart")==NULL) {
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
		}
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) break;
			if (strncasecmp(encoding, "quoted-printable", 16)==0) {
				DecodeQP(reply, inbuffer, contenttype);
			} else if (strncasecmp(encoding, "base64", 6)==0) {
				DecodeBase64(sid, inbuffer, contenttype);
			} else if (strncasecmp(contenttype, "text/html", 9)==0) {
				DecodeHTML(reply, inbuffer, contenttype, 1);
			} else {
				printline(sid, reply, inbuffer);
			}
		}
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "</FONT></PRE>");
		}
		return 1;
	}
	for (;;) {
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
		if (is_msg_end(sid, inbuffer)) break;
		if (strcasestr(inbuffer, boundary)!=NULL) {
			head=1;
			break;
		}
	}
	memset(cbound, 0, sizeof(cbound));
	memset(tbound, 0, sizeof(tbound));
	snprintf(tbound, sizeof(tbound)-1, "%s--", boundary);
	for (;;) {
		if (is_msg_end(sid, inbuffer)) break;
		if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
		if (head) {
			memset(ctype, 0, sizeof(ctype));
			memset(cencode, 0, sizeof(cencode));
			for (;;) {
				wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
				if (is_msg_end(sid, inbuffer)) break;
				if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
				if (strcmp(inbuffer, "")==0) {
					head=0;
					break;
				}
				if (strncasecmp(inbuffer, "Content-Transfer-Encoding:", 26)==0) {
					ptemp=inbuffer+26;
					while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
					strncpy(cencode, ptemp, sizeof(cencode)-1);
				}
				if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
					ptemp=inbuffer+13;
					while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
					strncpy(ctype, ptemp, sizeof(ctype)-1);
					while (inbuffer[strlen(inbuffer)-1]==';') {
						wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
						striprn(inbuffer);
						strncat(ctype, inbuffer, sizeof(ctype)-strlen(ctype)-1);
					}
				}
				if (strcasestr(contenttype, "multipart")==NULL) continue;
				if (file) continue;
				ptemp=strcasestr(inbuffer, "name=");
				if (ptemp!=NULL) {
					ptemp+=5;
					if (*ptemp=='\"') ptemp++;
					while ((*ptemp)&&(*ptemp!='\"')&&(strlen(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])<sizeof(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])-1)) {
						sid->dat->user_wmfiles[sid->dat->user_wmnumfiles][strlen(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])]=*ptemp;
						ptemp++;
					}
					file=1;
					sid->dat->user_wmnumfiles++;
				}
			}
		}
		if ((ptemp=strcasestr(ctype, "boundary="))!=NULL) {
			ptemp+=9;
			if (*ptemp=='\"') ptemp++;
			snprintf(cbound, sizeof(cbound)-1, "--");
			while ((*ptemp)&&(*ptemp!='\"')&&(strlen(cbound)<sizeof(cbound)-1)) {
				cbound[strlen(cbound)]=*ptemp;
				ptemp++;
			}
		}
//		if ((strcasestr(ctype, "multipart/alternative")!=NULL)||(strcasestr(ctype, "multipart/mixed")!=NULL)) {
		if (strcasestr(ctype, "multipart/alternative")!=NULL) {
			msgdone=webmailmime(sid, ctype, cencode, cbound, nummessage, reply, depth);
			head=1;
			continue;
		}
		if (is_msg_end(sid, inbuffer)) break;
		if (file) {
			for (;;) {
				wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
				if (is_msg_end(sid, inbuffer)) break;
				if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
				if (strcmp(inbuffer, "")==0) {
					file=0;
					break;
				}
				if (strcasestr(inbuffer, boundary)!=NULL) {
					file=0;
					head=1;
					break;
				}
			}
		}
		if (is_msg_end(sid, inbuffer)) break;
		if (head) continue;
		// start reading the part body
		for (;;) {
			if (strcmp(inbuffer, "")==0) break;
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) break;
			if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
		}
		if (is_msg_end(sid, inbuffer)) break;
		if (!reply) {
			if ((strncasecmp(contenttype, "multipart/report", 16)==0)||(strncasecmp(ctype, "message/rfc822", 14)==0)) {
				prints(sid, "<HR>");
			}
			if ((!msgdone)&&((strncasecmp(ctype, "text", 4)==0)||(strncasecmp(ctype, "message", 7)==0))) {
				if ((strncasecmp(ctype, "text/plain", 10)==0)&&(strncasecmp(contenttype, "multipart/alternative", 21)!=0)) {
					prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
				}
			} else if (strncasecmp(ctype, "message", 7)==0) {
				prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
			}
		}
		for (;;) {
			wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			if (is_msg_end(sid, inbuffer)) break;
			if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) break;
			if (strcasestr(inbuffer, boundary)!=NULL) {
				head=1;
				break;
			}
			if ((msgdone)&&(strncasecmp(contenttype, "multipart/report", 16)!=0)&&(strncasecmp(ctype, "message/rfc822", 14)!=0)) {
				continue;
			}
			if ((reply)&&(strncasecmp(ctype, "text/html", 9)!=0)&&(strncasecmp(ctype, "text/plain", 10)!=0)) {
				continue;
			}
			if (strncasecmp(ctype, "text/html", 9)==0) {
				if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(reply, inbuffer, ctype);
				} else if (strncasecmp(cencode, "base64", 6)==0) {
					DecodeBase64(sid, inbuffer, ctype);
				} else {
					DecodeHTML(reply, inbuffer, ctype, 1);
				}
			} else if (strncasecmp(contenttype, "multipart/alternative", 21)!=0) {
				if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(reply, inbuffer, ctype);
				} else if (strncasecmp(cencode, "base64", 6)==0) {
					DecodeBase64(sid, inbuffer, ctype);
				} else {
					printline(sid, reply, inbuffer);
				}
			}
		}
		if (!reply) {
			if ((!msgdone)&&((strncasecmp(ctype, "text", 4)==0)||(strncasecmp(ctype, "message", 7)==0))) {
				if ((strncasecmp(ctype, "text/plain", 10)==0)&&(strncasecmp(contenttype, "multipart/alternative", 21)!=0)) {
					prints(sid, "</FONT></PRE>\n");
				} else if (strncasecmp(ctype, "message", 7)==0) {
					prints(sid, "</FONT></PRE>\n");
				}
				prints(sid, "<BR>\n");
			} else if (strncasecmp(ctype, "message", 7)==0) {
				prints(sid, "</FONT></PRE>\n<BR>\n");
			}
		}
		if (strncasecmp(ctype, "text/html", 9)==0) msgdone=1;
		if ((strncasecmp(ctype, "text/plain", 10)==0)&&(strncasecmp(contenttype, "multipart/alternative", 21)!=0)) {
			msgdone=1;
		}
		if (is_msg_end(sid, inbuffer)) break;
		if ((strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
	}
	if ((!reply)&&(sid->dat->user_wmnumfiles>0)) {
		prints(sid, "Attachments<BR>\n");
		for (i=0;i<sid->dat->user_wmnumfiles;i++) {
			prints(sid, "[<A HREF=%s/mail/file/%d/", sid->dat->in_ScriptName, nummessage);
			printhex(sid, "%s", DecodeRFC2047(sid, sid->dat->user_wmfiles[i]));
			ptemp=get_mime_type(sid->dat->user_wmfiles[i]);
			if (strncasecmp(ptemp, "image/", 6)==0) {
				prints(sid, " TARGET=_blank");
			}
			prints(sid, ">");
			printht(sid, "%s", DecodeRFC2047(sid, sid->dat->user_wmfiles[i]));
			prints(sid, "</A>]<BR>\n");
		}
	}
	return msgdone;
}

void webmaillist(CONNECTION *sid)
{
	char **uidls;
	char *ptemp;
	char curdate[40];
	char fromtemp[26];
	char msgsize[20];
	int msize;
	int nummessages;
	int offset=0;
	int i, j;
	int sqr;
	int found;
	time_t unixdate;

	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/profile/maileditnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/maileditnew\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
		sqlFreeconnect(sqr);
		return;
	}
	sqlFreeconnect(sqr);
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sqlUpdatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username)==0) {
			sid->dat->user_mailcurrent=atoi(ptemp);
		}
	}
	htselect_mailjump(sid, sid->dat->user_mailcurrent);
	flushbuffer(sid);
	if (wmserver_connect(sid, 1)!=0) return;
	if ((nummessages=wmserver_mlistsync(sid, &uidls))<0) return;
	sqlUpdatef(sid, "UPDATE gw_mailaccounts SET lastcount = %d WHERE obj_uid = %d AND mailaccountid = %d", nummessages, sid->dat->user_uid, sid->dat->user_mailcurrent);
	if (nummessages<1) {
		prints(sid, "<CENTER>You have no messages in your mailbox.</CENTER><BR>\n");
		goto cleanup;
	}
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp);
	}
	if (offset<0) offset=0;
	if ((sqr=sqlQueryf(sid, "SELECT mailheaderid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' ORDER BY hdr_date ASC", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) goto cleanup;
	nummessages=sqlNumtuples(sqr);
	i=nummessages-offset-sid->dat->user_maxlist+1;
	if (i<1) i=1;
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<FORM METHOD=POST NAME=webmail ACTION=%s/mail/delete%s>\n", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=wmread":"");
	prints(sid, "<TR><TD ALIGN=LEFT NOWRAP WIDTH=150><INPUT TYPE=SUBMIT VALUE=\"Delete Selected\"></TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP>");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<nummessages) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Next Page</A>]", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		} else {
			prints(sid, "[Next Page]");
		}
	} else {
		prints(sid, "&nbsp;");
	}
	prints(sid, "</TD><TD ALIGN=RIGHT NOWRAP>Listing %d-%d of %d</TD></TR>\n", nummessages-offset, i, nummessages);
	prints(sid, "<TR><TD COLSPAN=3>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_TH);
	prints(sid, "<TH ALIGN=LEFT>&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT><FONT COLOR=%s>&nbsp;From&nbsp;</FONT></TH>", COLOR_THTEXT);
	prints(sid, "<TH ALIGN=LEFT WIDTH=100%%><FONT COLOR=%s>&nbsp;Subject&nbsp;</FONT></TH>", COLOR_THTEXT);
	prints(sid, "<TH ALIGN=LEFT><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH>", COLOR_THTEXT);
	prints(sid, "<TH ALIGN=LEFT><FONT COLOR=%s>&nbsp;Size&nbsp;</FONT></TH>", COLOR_THTEXT);
	prints(sid, "<TH ALIGN=LEFT>&nbsp;</TH>");
	prints(sid, "</TR>\n");
        for (i=nummessages-offset-1;(i>-1)&&(i>nummessages-offset-sid->dat->user_maxlist-1);i--) {
		msize=atoi(sqlGetvalue(sqr, i, 3));
		if (msize>=1048576) {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f M", (float)msize/1048576.0);
		} else {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f K", (float)msize/1024.0);
		}
		memset(fromtemp, 0, sizeof(fromtemp));
		snprintf(fromtemp, sizeof(fromtemp)-1, "%s", DecodeRFC2047(sid, sqlGetvalue(sqr, i, 5)));
		if ((ptemp=strchr(fromtemp, '<'))!=NULL) *ptemp='\0';
		if (strcmp(sqlGetvalue(sqr, i, 2), "r")!=0) {
			prints(sid, "<TR BGCOLOR=#D0D0FF>");
		} else {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
		}
		found=-1;
		for (j=0;j<sqlNumtuples(sqr);j++) {
			if (strcmp(uidls[j], sqlGetvalue(sqr, i, 4))==0) {
				found=j+1;
				break;
			}
		}
		if (strcmp(sqlGetvalue(sqr, i, 2), "n")==0) {
			sqlUpdatef(sid, "UPDATE gw_mailheaders SET status = 'o' WHERE mailheaderid = %d AND obj_uid = %d", atoi(sqlGetvalue(sqr, i, 0)), sid->dat->user_uid);
		}
		prints(sid, "<TD NOWRAP STYLE='padding:0px'><INPUT TYPE=checkbox NAME=%d VALUE=\"%s\"></TD>", found, sqlGetvalue(sqr, i, 4));
		prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, fromtemp));
		prints(sid, "<TD NOWRAP><A HREF=%s/mail/read?msg=%d%s TITLE=\"%s\">", sid->dat->in_ScriptName, found, sid->dat->user_menustyle>0?" TARGET=wmread":"", DecodeRFC2047(sid, sqlGetvalue(sqr, i, 9)));
		prints(sid, "%.40s</A>&nbsp;</TD>", DecodeRFC2047(sid, sqlGetvalue(sqr, i, 9)));
		unixdate=time_sql2unix(sqlGetvalue(sqr, i, 8));
		unixdate+=time_tzoffset(sid, unixdate);
		if (unixdate<0) unixdate=0;
		strftime(curdate, 30, "%b %d, %Y %I:%M%p", gmtime(&unixdate));
		curdate[18]=tolower(curdate[18]);
		curdate[19]=tolower(curdate[19]);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", curdate);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", msgsize);
		if (strcasestr(sqlGetvalue(sqr, i, 11), "multipart/mixed")!=NULL) {
			prints(sid, "<TD><IMG BORDER=0 SRC=/groupware/images/paperclip.gif HEIGHT=16 WIDTH=11 ALT='File Attachments'></TD>");
		} else {
			prints(sid, "<TD>&nbsp;&nbsp;&nbsp;</TD>");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n</TD></TR>");
	prints(sid, "<TR><TD ALIGN=LEFT NOWRAP WIDTH=150><INPUT TYPE=SUBMIT VALUE=\"Delete Selected\"></TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP>");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<nummessages) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Next Page</A>]", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		} else {
			prints(sid, "[Next Page]");
		}
	} else {
		prints(sid, "&nbsp;");
	}
	prints(sid, "</TD><TD ALIGN=RIGHT NOWRAP WIDTH=150>&nbsp;</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
cleanup:
	for (j=0;j<nummessages;j++) free(uidls[j]);
	free(uidls);
	return;
}

void webmailread(CONNECTION *sid)
{
	wmheader header;
	char uidl[100];
	short int nummessages;
	short int nummessage;

	if (wmserver_connect(sid, 1)!=0) return;
	nummessages=wmserver_count(sid);
	nummessage=atoi(getgetenv(sid, "MSG"));
	if ((nummessage>nummessages)||(nummessage<1)) {
		prints(sid, "No such message.<BR>");
		wmserver_disconnect(sid);
		return;
	}
	memset(uidl, 0, sizeof(uidl));
	wmserver_uidl(sid, nummessage, uidl);

	sqlUpdatef(sid, "UPDATE gw_mailheaders SET status = 'r' WHERE uidl = '%s' AND obj_uid = %d", str2sql(sid, uidl), sid->dat->user_uid);

	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this message?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (nummessage>1) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Previous</A>]\n", sid->dat->in_ScriptName, nummessage-1);
	} else {
		prints(sid, "[Previous]\n");
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo(%d)>Reply</A>]\n", nummessage);
		prints(sid, "[<A HREF=javascript:Forward(%d)>Forward</A>]\n", nummessage);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d>Reply</A>]\n", sid->dat->in_ScriptName, nummessage);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d>Forward</A>]\n", sid->dat->in_ScriptName, nummessage);
	}
	prints(sid, "[<A HREF=%s/mail/delete?%d=%s onClick=\"return ConfirmDelete();\">Delete</A>]\n", sid->dat->in_ScriptName, nummessage, uidl);
	if (nummessage<nummessages) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Next</A>]\n", sid->dat->in_ScriptName, nummessage+1);
	} else {
		prints(sid, "[Next]\n");
	}
	prints(sid, "<BR><TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	wmserver_msgretr(sid, nummessage);
	memset((char *)&header, 0, sizeof(header));
	if (webmailheader(sid, &header)!=0) return;
	prints(sid, "<TR BGCOLOR=%s><TD><TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n", COLOR_FVAL);
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;From   &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;<A HREF=javascript:MsgTo(\"%s\")>", COLOR_TH, COLOR_THTEXT, COLOR_FVAL, header.Replyto);
	printht(sid, "%s", DecodeRFC2047(sid, header.From));
	prints(sid, "</A>&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;To     &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", COLOR_TH, COLOR_THTEXT, COLOR_FVAL);
	printht(sid, "%s", DecodeRFC2047(sid, header.To));
	prints(sid, "&nbsp;</TD></TR>\n");
	if (strlen(header.CC)) {
		prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;CC     &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", COLOR_TH, COLOR_THTEXT, COLOR_FVAL);
		printht(sid, "%s", DecodeRFC2047(sid, header.CC));
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;Subject&nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", COLOR_TH, COLOR_THTEXT, COLOR_FVAL);
	printht(sid, "%s", DecodeRFC2047(sid, header.Subject));
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;Date   &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", COLOR_TH, COLOR_THTEXT, COLOR_FVAL);
	printht(sid, "%s", header.Date);
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD>[<A HREF=%s/mail/raw?msg=%d TARGET=_blank>View Source</A>]</TD></TR>\n", COLOR_FVAL, sid->dat->in_ScriptName, nummessage);
	prints(sid, "<TR BGCOLOR=%s><TD>\n", COLOR_FVAL);
	webmailmime(sid, header.contenttype, header.encoding, header.boundary, nummessage, 0, 0);
	prints(sid, "</TD></TR></TABLE>\n");
	if (nummessage>1) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Previous</A>]\n", sid->dat->in_ScriptName, nummessage-1);
	} else {
		prints(sid, "[Previous]\n");
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo(%d)>Reply</A>]\n", nummessage);
		prints(sid, "[<A HREF=javascript:Forward(%d)>Forward</A>]\n", nummessage);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d>Reply</A>]\n", sid->dat->in_ScriptName, nummessage);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d>Forward</A>]\n", sid->dat->in_ScriptName, nummessage);
	}
	prints(sid, "[<A HREF=%s/mail/delete?%d=%s onClick=\"return ConfirmDelete();\">Delete</A>]\n", sid->dat->in_ScriptName, nummessage, uidl);
	if (nummessage<nummessages) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Next</A>]\n", sid->dat->in_ScriptName, nummessage+1);
	} else {
		prints(sid, "[Next]\n");
	}
	prints(sid, "<BR>\n");
	wmserver_disconnect(sid);
	return;
}

void webmailwrite(CONNECTION *sid)
{
	wmheader header;
	char msgto[512];
	char msgcc[1024];
	char msgbcc[1024];
	char subject[512];
	short int replyto=0;
	short int forward=0;

	if (wmserver_connect(sid, 1)!=0) return;
	memset(msgcc, 0, sizeof(msgcc));
	memset(msgbcc, 0, sizeof(msgbcc));
	memset(subject, 0, sizeof(subject));
	if (getgetenv(sid, "REPLYTO")!=NULL) {
		replyto=atoi(getgetenv(sid, "REPLYTO"));
	}
	if (getgetenv(sid, "FORWARD")!=NULL) {
		forward=atoi(getgetenv(sid, "FORWARD"));
	}
	if (getgetenv(sid, "TO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "TO"), sizeof(msgto)-1);
	} else if (getgetenv(sid, "MSGTO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "MSGTO"), sizeof(msgto)-1);
	}
	if (getgetenv(sid, "CC")!=NULL) {
		strncpy(msgcc, getgetenv(sid, "CC"), sizeof(msgcc)-1);
	}
	if (getgetenv(sid, "BCC")!=NULL) {
		strncpy(msgbcc, getgetenv(sid, "BCC"), sizeof(msgbcc)-1);
	}
	if ((replyto>0)||(forward>0)) {
		if (replyto>0) {
			wmserver_msgretr(sid, replyto);
		} else {
			wmserver_msgretr(sid, forward);
		}
		memset((char *)&header, 0, sizeof(header));
		if (webmailheader(sid, &header)!=0) return;
		snprintf(msgto, sizeof(msgto)-1, "%s", header.Replyto);
		if (replyto>0) {
			if (strncasecmp(DecodeRFC2047(sid, header.Subject), "RE:", 3)!=0) {
				snprintf(subject, sizeof(subject)-1, "Re: %s", DecodeRFC2047(sid, header.Subject));
			} else {
				snprintf(subject, sizeof(subject)-1, "%s", DecodeRFC2047(sid, header.Subject));
			}
		} else if (forward>0) {
			snprintf(subject, sizeof(subject)-1, "Fwd: %s", DecodeRFC2047(sid, header.Subject));
		}
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/send NAME=wmcompose ENCTYPE=multipart/form-data>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>To     </B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=msgto      SIZE=60 VALUE=\"%s\"></TD></TR>\n", COLOR_EDITFORM, str2html(sid, msgto));
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>CC     </B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=msgcc      SIZE=60 VALUE=\"%s\"></TD></TR>\n", COLOR_EDITFORM, str2html(sid, msgcc));
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>BCC    </B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=msgbcc     SIZE=60 VALUE=\"%s\"></TD></TR>\n", COLOR_EDITFORM, str2html(sid, msgbcc));
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>Subject</B>&nbsp;</TD><TD><INPUT TYPE=TEXT NAME=msgsubject SIZE=60 VALUE=\"%s\"></TD></TR>\n", COLOR_EDITFORM, str2html(sid, subject));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA NAME=msgbody COLS=78 ROWS=16 WRAP=HARD>\n", COLOR_EDITFORM);
	if ((replyto<1)&&(forward<1)) {
		prints(sid, "</TEXTAREA></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>File</B>&nbsp;</TD><TD><INPUT TYPE=FILE NAME=fattach SIZE=50></TD></TR>\n", COLOR_EDITFORM);
		prints(sid, "</TABLE>\n");
		prints(sid, "<INPUT TYPE=SUBMIT VALUE='Send Mail'>\n");
		prints(sid, "</FORM>\n</CENTER>\n");
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmcompose.msgto.focus();\n// -->\n</SCRIPT>\n");
		wmserver_disconnect(sid);
		return;
	}
	prints(sid, "--- %s wrote:\n", header.From);
	if (forward>0) {
		prints(sid, "> From:    %s\n", header.From);
		prints(sid, "> Subject: %s\n", header.Subject);
		prints(sid, "> Date:    %s\n", header.Date);
		prints(sid, ">\n");
	}
	if (replyto>0) {
		webmailmime(sid, header.contenttype, header.encoding, header.boundary, replyto, 1, 0);
	} else {
		webmailmime(sid, header.contenttype, header.encoding, header.boundary, forward, 1, 0);
	}
	prints(sid, "</TEXTAREA></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>File</B>&nbsp;</TD><TD><INPUT TYPE=FILE NAME=fattach SIZE=50></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT VALUE='Send Mail'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmcompose.msgto.focus();\n// -->\n</SCRIPT>\n");
	wmserver_disconnect(sid);
	return;
}

void webmailsend(CONNECTION *sid)
{
	char *filebody=NULL;
	char *msgbody=NULL;
	char *pmsgbody;
	char *ptemp;
	char boundary[100];
	char inbuffer[1024];
	char filename[1024];
	char cfilesize[10];
	char msgaddr[128];
	char msgto[1024];
	char msgcc[1024];
	char msgbcc[1024];
	char msgsubject[128];
	char msgdate[100];
	char msgdatetz[100];
	char line[1024];
	int filesize=0;
	struct timeval ttime;
	struct timezone tzone;
	time_t t;
	unsigned int mimesize;
	int i;

	t=time(NULL);
	memset(boundary, 0, sizeof(boundary));
	snprintf(boundary, sizeof(boundary)-1, "------------SGW%d", (int)t);
	if (strcasecmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	if (getmimeenv(sid, "MSGTO", &mimesize)==NULL) {
		prints(sid, "<CENTER>No recipient specified - Message was not sent</CENTER>\n");
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
	msgbody=calloc(sid->dat->in_ContentLength+1024, sizeof(char));
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
		strncpy(msgbody, getmimeenv(sid, "MSGBODY", &mimesize), sid->dat->in_ContentLength+1023);
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
	ttime.tv_sec+=time_tzoffset(sid, ttime.tv_sec);
	strftime(msgdate, sizeof(msgdate), "%a, %d %b %Y %H:%M:%S", gmtime(&ttime.tv_sec));
	snprintf(msgdatetz, sizeof(msgdatetz)-1, " %+.4d", +time_tzoffset(sid, ttime.tv_sec)/36);
	strncat(msgdate, msgdatetz, sizeof(msgdate)-strlen(msgdate)-1);
	if (wmserver_smtpconnect(sid)!=0) return;
	wmprints(sid, "HELO %s\r\n", sid->dat->user_wmsmtpserver);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
//	if (use_auth) {
//		if (wmserver_smtpauth(sid)!=0) goto quit;
//	}
	wmprints(sid, "MAIL From: <%s>\r\n", sid->dat->user_wmreplyto);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
	prints(sid, "Sender '%s' OK.<BR>\n", sid->dat->user_wmreplyto);
	flushbuffer(sid);
	for (i=0;i<3;i++) {
		switch (i) {
			case 1: ptemp=msgcc; break;
			case 2: ptemp=msgbcc; break;
			default: ptemp=msgto; break;
		}
		while (*ptemp) {
			if (strstr(ptemp, "@")==NULL) break;
			memset(msgaddr, 0, sizeof(msgaddr));
			while ((*ptemp)&&(*ptemp!=',')&&(*ptemp!=' ')&&(strlen(ptemp)<sizeof(msgaddr)-1)) {
				msgaddr[strlen(msgaddr)]=*ptemp;
				ptemp++;
			}
			while ((*ptemp==',')||(*ptemp==' ')) {
				ptemp++;
			}
			while (!isalpha(msgaddr[strlen(msgaddr)-1])) {
				msgaddr[strlen(msgaddr)-1]='\0';
			}
			wmprints(sid, "RCPT To: <%s>\r\n", msgaddr);
			do {
				memset(inbuffer, 0, sizeof(inbuffer));
				wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
			} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
			if (strncasecmp(inbuffer, "250", 3)!=0) goto quit;
			prints(sid, "Recipient '%s' OK.<BR>\n", msgaddr);
			flushbuffer(sid);
		}
	}
	wmprints(sid, "DATA\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "354", 3)!=0) goto quit;
	wmprints(sid, "From: %s <%s>\r\n", sid->dat->user_wmrealname, sid->dat->user_wmreplyto);
	wmprints(sid, "Reply-To: <%s>\r\n", sid->dat->user_wmreplyto);
	if (strlen(sid->dat->user_wmorganization)>0) {
		wmprints(sid, "Organization: %s\r\n", sid->dat->user_wmorganization);
	}
	wmprints(sid, "To: %s\r\n", msgto);
	if (strlen(msgcc)) {
		wmprints(sid, "Cc: %s\r\n", msgcc);
	}
	wmprints(sid, "Subject: %s\r\n", msgsubject);
	wmprints(sid, "Date: %s\r\n", msgdate);
	if (filesize>0) {
		wmprints(sid, "MIME-Version: 1.0\r\n");
		wmprints(sid, "Content-Type: multipart/mixed; boundary=\"%s\"\r\n", boundary);
	} else {
		wmprints(sid, "Content-Type: text/plain\r\n");
	}
	wmprints(sid, "X-Mailer: %s\r\n", SERVER_NAME);
	wmprints(sid, "\r\n");
	pmsgbody=msgbody;
	if (filesize>0) {
		wmprints(sid, "This is a multi-part message in MIME format.\r\n");
		wmprints(sid, "--%s\r\n", boundary);
		wmprints(sid, "Content-Type: text/plain\r\n");
		wmprints(sid, "Content-Transfer-Encoding: 8bit\r\n\r\n");
	}
	while (strlen(pmsgbody)>78) {
		memset(line, 0, sizeof(line));
		snprintf(line, 79, "%s", pmsgbody);
		line[78]='\0';
		if (strchr(line, '\r')||strchr(line, '\n')) {
			if ((ptemp=strchr(line, '\r'))!=NULL) *ptemp='\0';
			if ((ptemp=strchr(line, '\n'))!=NULL) *ptemp='\0';
			wmprints(sid, "%s\r\n", line);
			pmsgbody+=strlen(line);
			if (*pmsgbody=='\r') pmsgbody++;
			if (*pmsgbody=='\n') pmsgbody++;
		} else if ((ptemp=strrchr(line, ' '))!=NULL) {
			*ptemp='\0';
			wmprints(sid, "%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else {
			wmprints(sid, "%s", line);
			pmsgbody+=strlen(line);
		}
	}
	memset(line, 0, sizeof(line));
	snprintf(line, 78, "%s", pmsgbody);
	wmprints(sid, "%s\r\n", line);
	free(msgbody);
	if (strlen(sid->dat->user_wmsignature)>0) {
		wmprints(sid, "\r\n");
		wmprints(sid, "%s\r\n", sid->dat->user_wmsignature);
	}
	if (filesize>0) {
		prints(sid, "Sending file '%s' (%d bytes)<BR>\n", filename, filesize);
		flushbuffer(sid);
		wmprints(sid, "\r\n--%s\r\n", boundary);
		wmprints(sid, "Content-Type: application/octet-stream; name=\"%s\"\r\n", filename);
		wmprints(sid, "Content-Transfer-Encoding: base64\r\n");
		wmprints(sid, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", filename);
		EncodeBase64(sid, filebody, filesize);
		wmprints(sid, "\r\n--%s--\r\n", boundary);
	}
	wmprints(sid, "\r\n.\r\n");
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		wmfgets(sid, inbuffer, sizeof(inbuffer)-1, sid->dat->user_wmsocket);
	} while ((inbuffer[3]!=' ')&&(inbuffer[3]!='\0'));
	if (strncasecmp(inbuffer, "250", 3)==0) {
		memset(inbuffer, 0, sizeof(inbuffer));
		prints(sid, "<BR>Message Successfully Sent.<BR>\n", inbuffer);
		if (sid->dat->user_menustyle==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/list\">\n", sid->dat->in_ScriptName);
		} else {
			prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]<BR>\n", sid->dat->in_ScriptName);
		}
		flushbuffer(sid);
		wmserver_smtpdisconnect(sid);
		return;
	}
quit:
	prints(sid, "%s<BR>\n", inbuffer);
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]<BR>\n", sid->dat->in_ScriptName);
	}
	flushbuffer(sid);
	wmserver_smtpdisconnect(sid);
	return;
}

void webmaildelete(CONNECTION *sid)
{
	char msgnum[8];
	char uidl[100];
	int nummessages;
	int deleted=0;
	int i;

	if (wmserver_connect(sid, 1)!=0) return;
	nummessages=wmserver_count(sid);
	if (nummessages>0) {
		for (i=nummessages;i>0;i--) {
			snprintf(msgnum, sizeof(msgnum)-1, "%d", i);
			if ((getpostenv(sid, msgnum)==NULL)&&(getgetenv(sid, msgnum)==NULL)) continue;
			memset(uidl, 0, sizeof(uidl));
			wmserver_uidl(sid, i, uidl);
			if ((getpostenv(sid, msgnum)!=NULL)&&(strcmp(getpostenv(sid, msgnum), uidl)!=0)) continue;
			if ((getgetenv(sid,msgnum)!=NULL)&&(strcmp(getgetenv(sid, msgnum), uidl)!=0)) continue;
			prints(sid, "Deleting message %d...", i);
			deleted=i;
			if (wmserver_msgdele(sid, i)==0) {
				prints(sid, "success.<BR>\n");
			} else {
				prints(sid, "failure.<BR>\n");
			}
			flushbuffer(sid);
		}
	} else {
		prints(sid, "<CENTER>You have no messages in your mailbox.</CENTER><BR>\n");
	}
	wmserver_disconnect(sid);
	snprintf(msgnum, sizeof(msgnum)-1, "%d", deleted);
	if (deleted>=nummessages) deleted=nummessages-1;
	if (sid->dat->user_menustyle>0) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "parent.wmlist.location=\"%s/mail/list\";\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/null\">\n", sid->dat->in_ScriptName);
	} else {
		if ((getgetenv(sid, msgnum)==NULL)||(nummessages<2)) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/list\">\n", sid->dat->in_ScriptName);
		} else {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/read?msg=%d\">\n", sid->dat->in_ScriptName, deleted);
		}
	}
	return;
}

void webmailframeset(CONNECTION *sid)
{
	char *ptemp;
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<HTML>\n<HEAD>\n<TITLE>NullLogic Groupware Webmail</TITLE>\n</HEAD>\n");
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sqlUpdatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username)==0) {
			sid->dat->user_mailcurrent=atoi(ptemp);
		}
	}
	if ((sqr=sqlQueryf(sid, "SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/profile/maileditnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/maileditnew\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
		prints(sid, "</HTML>\n");
	} else {
		prints(sid, "<FRAMESET ROWS=\"50%%,50%%\" BORDER=0 FRAMEBORDER=1 FRAMESPACING=1>\n");
		prints(sid, "<FRAME BORDER=0 NAME=\"wmlist\" SRC=%s/mail/list MARGINHEIGHT=1 MARGINWIDTH=1 SCROLLING=YES>\n", sid->dat->in_ScriptName);
		prints(sid, "<FRAME BORDER=0 NAME=\"wmread\" SRC=%s/mail/null MARGINHEIGHT=1 MARGINWIDTH=1 SCROLLING=YES>\n", sid->dat->in_ScriptName);
		prints(sid, "</FRAMESET>\n");
		prints(sid, "To view this page, you need a web browser capable of displaying frames.\n");
		prints(sid, "</HTML>\n");
	}
	sqlFreeconnect(sqr);
	return;
}

void webmailmain(CONNECTION *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/mail/main", 10)==0) {
		webmailframeset(sid);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/file", 10)==0) {
		webmailfiledl(sid);
		return;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/mail/notice", 12)==0) {
		htpage_header(sid, "E-Mail Notice");
		wmnotice(sid);
		htpage_footer(sid);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/quit", 10)==0) {
		wmlogout(sid);
	}
	htpage_topmenu(sid, MENU_WEBMAIL);
	flushbuffer(sid);
	if (strncmp(sid->dat->in_RequestURI, "/mail/quit", 10)==0) {
		wmloginform(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/read", 10)==0) {
		webmailread(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/write", 11)==0) {
		webmailwrite(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/list", 10)==0) {
		webmaillist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/send", 10)==0) {
		webmailsend(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/delete", 12)==0) {
		webmaildelete(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/raw", 9)==0) {
		webmailraw(sid);
	}
	htpage_footer(sid);
	return;
}
