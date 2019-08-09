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
#include "mod_email.h"

int webmailheader(CONN *sid, wmheader *header, FILE **fp)
{
	char inbuffer[1024];
	char *prevheader=NULL;
	char *ptemp;
	int prevheadersize=0;

	header->status[0]='n';
	memset(header->boundary, 0, sizeof(header->boundary));
	for (;;) {
		if (fp!=NULL) {
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
		} else {
			if (wmfgets(sid, inbuffer, sizeof(inbuffer)-1, &sid->dat->wm->socket)<0) return -1;
		}
		striprn(inbuffer);
		if (strcmp(inbuffer, "")==0) break;
		if (strncasecmp(inbuffer, "Reply-To:", 9)==0) {
			prevheader=header->ReplyTo;
			prevheadersize=sizeof(header->ReplyTo);
			ptemp=inbuffer+9;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->ReplyTo, DecodeRFC2047(sid, ptemp), sizeof(header->ReplyTo)-1);
		} else if (strncasecmp(inbuffer, "From:", 5)==0) {
			prevheader=header->From;
			prevheadersize=sizeof(header->From);
			ptemp=inbuffer+5;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->From, DecodeRFC2047(sid, ptemp), sizeof(header->From)-1);
		} else if (strncasecmp(inbuffer, "To:", 3)==0) {
			prevheader=header->To;
			prevheadersize=sizeof(header->To);
			ptemp=inbuffer+3;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->To, DecodeRFC2047(sid, ptemp), sizeof(header->To)-1);
		} else if (strncasecmp(inbuffer, "CC:", 3)==0) {
			prevheader=header->CC;
			prevheadersize=sizeof(header->CC);
			ptemp=inbuffer+3;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->CC, DecodeRFC2047(sid, ptemp), sizeof(header->CC)-1);
		} else if (strncasecmp(inbuffer, "Subject:", 8)==0) {
			prevheader=header->Subject;
			prevheadersize=sizeof(header->Subject);
			ptemp=inbuffer+8;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->Subject, DecodeRFC2047(sid, ptemp), sizeof(header->Subject)-1);
		} else if (strncasecmp(inbuffer, "Date:", 5)==0) {
			prevheader=header->Date;
			prevheadersize=sizeof(header->Date);
			ptemp=inbuffer+5;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->Date, DecodeRFC2047(sid, ptemp), sizeof(header->Date)-1);
		} else if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
			prevheader=header->contenttype;
			prevheadersize=sizeof(header->contenttype);
			ptemp=inbuffer+13;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->contenttype, DecodeRFC2047(sid, ptemp), sizeof(header->contenttype)-1);
		} else if (strncasecmp(inbuffer, "Content-Transfer-Encoding: ", 26)==0) {
			prevheader=header->encoding;
			prevheadersize=sizeof(header->encoding);
			ptemp=inbuffer+26;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->encoding, DecodeRFC2047(sid, ptemp), sizeof(header->encoding)-1);
		} else if (strncasecmp(inbuffer, "In-Reply-To: ", 13)==0) {
			prevheader=header->InReplyTo;
			prevheadersize=sizeof(header->InReplyTo);
			ptemp=inbuffer+13;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->InReplyTo, DecodeRFC2047(sid, ptemp), sizeof(header->InReplyTo)-1);
		} else if (strncasecmp(inbuffer, "Message-ID: ", 12)==0) {
			prevheader=header->MessageID;
			prevheadersize=sizeof(header->MessageID);
			ptemp=inbuffer+12;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncpy(header->MessageID, DecodeRFC2047(sid, ptemp), sizeof(header->MessageID)-1);
		} else if (strncasecmp(inbuffer, "Status:", 7)==0) {
			prevheader=NULL;
			prevheadersize=sizeof(header->status);
			ptemp=inbuffer+7;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			if (strchr(ptemp, 'R')!=NULL) header->status[0]='r';
		} else if ((inbuffer[0]==' ')||(inbuffer[0]=='\t')) {
			if (prevheader==NULL) continue;
			ptemp=inbuffer;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			strncat(prevheader, DecodeRFC2047(sid, ptemp), prevheadersize-strlen(prevheader)-1);
		} else {
			prevheader=NULL;
			prevheadersize=0;
		}
	}
	if ((ptemp=p_strcasestr(header->contenttype, "boundary="))!=NULL) {
		ptemp+=9;
		if (*ptemp==' ') ptemp++;
		if (*ptemp=='\"') ptemp++;
		snprintf(header->boundary, sizeof(header->boundary)-1, "--");
		while ((*ptemp)&&(*ptemp!='\"')&&(strlen(header->boundary)<sizeof(header->boundary)-1)) {
			header->boundary[strlen(header->boundary)]=*ptemp;
			ptemp++;
		}
	}
	if (strlen(header->ReplyTo)==0) {
		ptemp=header->From;
		while ((*ptemp)&&(*ptemp!='<')) ptemp++;
		if (*ptemp=='<') ptemp++;
		while ((*ptemp)&&(*ptemp!='>')&&(strlen(header->ReplyTo)<sizeof(header->ReplyTo)-1)) {
			header->ReplyTo[strlen(header->ReplyTo)]=*ptemp;
			ptemp++;
		}
		if (strlen(header->ReplyTo)==0) {
			strncpy(header->ReplyTo, header->From, sizeof(header->ReplyTo)-1);
		}
	} else {
		ptemp=header->ReplyTo;
		while ((*ptemp)&&(*ptemp!='<')) ptemp++;
		if (*ptemp=='<') {
			ptemp++;
			memmove(header->ReplyTo, ptemp, strlen(ptemp)+1);
			ptemp=header->ReplyTo;
			while ((*ptemp)&&(*ptemp!='>')) ptemp++;
			if (*ptemp=='>') *ptemp='\0';
		}
	}
//	if (strlen(header->From)==0) strcpy(header->From, "(No Sender Name)");
//	if (strlen(header->Subject)==0) strcpy(header->Subject, "(No Subject)");
//	if (strlen(header->Date)==0) strcpy(header->Date, "(No Date)");
	return 0;
}

static int webmailfiledl_r(CONN *sid, FILE **fp, char *contenttype, char *boundary, char *filename, short int depth)
{
	char inbuffer[1024];
	char outbuffer[1024];
	char b64buffer[1024];
	char fnamebuf[512];
	char cdisp[128];
	char cencode[100];
	char cfile[256];
	char ctype[200];
	char cbound[100];
	char tbound[100];
	char *ptemp;
	int head=0;
	short int b64len;
	char *prevheader=NULL;
	int prevheadersize=0;

	depth++;
	if (strncasecmp(contenttype, "message/rfc822", 14)==0) {
		head=1;
	} else {
		for (;;) {
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
			if (p_strcasestr(inbuffer, boundary)!=NULL) {
				head=1;
				break;
			}
		}
	}
	memset(cdisp, 0, sizeof(cdisp));
	memset(ctype, 0, sizeof(ctype));
	memset(cbound, 0, sizeof(cbound));
	memset(tbound, 0, sizeof(tbound));
	snprintf(tbound, sizeof(tbound)-1, "%s--", boundary);
	prevheader=NULL;
	for (;;) {
		if (*fp==NULL) break;
		if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return 0;
		if (head) {
			memset(cfile, 0, sizeof(cfile));
			memset(cdisp, 0, sizeof(cdisp));
			memset(ctype, 0, sizeof(ctype));
			memset(cencode, 0, sizeof(cencode));
			prevheader=NULL;
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) return 0;
				if (inbuffer[0]=='\0') { head=0; break; }
				if (inbuffer[0]=='-') {
					if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return 0;
					if (strncasecmp(inbuffer, boundary, strlen(boundary))==0) {
						memset(cfile, 0, sizeof(cfile));
						memset(cdisp, 0, sizeof(cdisp));
						memset(ctype, 0, sizeof(ctype));
						memset(cencode, 0, sizeof(cencode));
						prevheader=NULL;
						continue;
					}
				}
				if (strncasecmp(inbuffer, "Content-Transfer-Encoding:", 26)==0) {
					ptemp=inbuffer+26;
					while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
					strncpy(cencode, ptemp, sizeof(cencode)-1);
					if ((strncasecmp(cencode, "base64", 6)!=0)&&(strncasecmp(cencode, "quoted-printable", 16)!=0)&&
					    (strncasecmp(cencode, "7bit", 4)!=0)&&(strncasecmp(cencode, "8bit", 4)!=0)) {
						memset(cencode, 0, sizeof(cencode));
						memset(cfile, 0, sizeof(cfile));
						continue;
					}
				} else if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
					prevheader=ctype;
					prevheadersize=sizeof(ctype);
					ptemp=inbuffer+13;
					while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
					strncat(prevheader, ptemp, prevheadersize-strlen(prevheader)-1);
				} else if (strncasecmp(inbuffer, "Content-Disposition:", 20)==0) {
					prevheader=cdisp;
					prevheadersize=sizeof(cdisp);
					ptemp=inbuffer+20;
					while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
					strncat(prevheader, ptemp, prevheadersize-strlen(prevheader)-1);
				} else if ((inbuffer[0]==' ')||(inbuffer[0]=='\t')) {
					if (prevheader!=NULL) strncat(prevheader, inbuffer, prevheadersize-strlen(prevheader)-1);
				} else {
					prevheader=NULL;
					prevheadersize=0;
				}
				if (strlen(cfile)==0) {
					if ((ptemp=p_strcasestr(ctype, "name="))==NULL) {
						if ((ptemp=p_strcasestr(cdisp, "name="))==NULL) {
							continue;
						}
					}
					ptemp+=5;
					if (*ptemp=='\"') ptemp++;
					memset(fnamebuf, 0, sizeof(fnamebuf));
					while ((*ptemp)&&(*ptemp!='\"')&&(strlen(fnamebuf)<sizeof(fnamebuf)-1)) {
						fnamebuf[strlen(fnamebuf)]=*ptemp;
						ptemp++;
					}
					snprintf(cfile, sizeof(cfile)-1, "%s", DecodeRFC2047(sid, fnamebuf));
					if (strcmp(cfile, filename)!=0) {
						memset(cfile, 0, sizeof(cfile));
						memset(cdisp, 0, sizeof(cdisp));
						memset(ctype, 0, sizeof(ctype));
						memset(cencode, 0, sizeof(cencode));
						prevheader=NULL;
						continue;
					}
				}
				if ((strlen(cfile)>0)&&(strcmp(inbuffer, "")==0)) break;
				if (strlen(cencode)==0) {
					snprintf(cencode, sizeof(cencode)-1, "8bit");
				}
			}
		}
		if (strncasecmp(ctype, "message/rfc822", 14)==0) {
			if (webmailfiledl_r(sid, fp, ctype, boundary, filename, depth)==1) {
				return 1;
			} else {
				head=1;
				continue;
			}
		}
		if ((ptemp=p_strcasestr(ctype, "boundary="))!=NULL) {
			ptemp+=9;
			if (*ptemp=='\"') ptemp++;
			snprintf(cbound, sizeof(cbound)-1, "--");
			while ((*ptemp)&&(*ptemp!='\"')&&(strlen(cbound)<sizeof(cbound)-1)) {
				cbound[strlen(cbound)]=*ptemp;
				ptemp++;
			}
		}
		if (p_strcasestr(ctype, "multipart")!=NULL) {
			if (webmailfiledl_r(sid, fp, ctype, cbound, filename, depth)==1) {
				return 1;
			} else {
				head=1;
				continue;
			}
		}
		if (*fp==NULL) break;
		if ((strlen(cencode))&&(strlen(cfile))) {
			ptemp=get_mime_type(cfile);
			if (strncasecmp(ptemp, "image/", 6)!=0) {
				snprintf(sid->dat->out_ContentDisposition, sizeof(sid->dat->out_ContentDisposition)-1, "attachment");
			}
			send_header(sid, 1, 200, "1", ptemp, -1, -1);
			flushbuffer(sid);
			memset(b64buffer, 0, sizeof(b64buffer));
			b64len=0;
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) return 1;
				if (strncasecmp(inbuffer, boundary, strlen(boundary))==0) break;
				if (strncasecmp(cencode, "base64", 6)==0) {
					if (strlen(inbuffer)+b64len>=sizeof(b64buffer)-1) {
						if (DecodeBase64file(sid, b64buffer)<0) {
							if (*fp!=NULL) { fclose(*fp); *fp=NULL; }
						}
						memset(b64buffer, 0, sizeof(b64buffer));
						b64len=0;
					}
					strcat(b64buffer, inbuffer);
					b64len+=strlen(inbuffer);
				} else if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(outbuffer, sizeof(outbuffer)-1, inbuffer);
					prints(sid, "%s", outbuffer);
				} else if (strncasecmp(cencode, "7bit", 4)==0) {
					DecodeText(outbuffer, sizeof(outbuffer)-1, inbuffer);
					prints(sid, "%s", outbuffer);
				} else if (strncasecmp(cencode, "8bit", 4)==0) {
					DecodeText(outbuffer, sizeof(outbuffer)-1, inbuffer);
					prints(sid, "%s", outbuffer);
				}
			}
			if ((strncasecmp(cencode, "base64", 6)==0)&&(strlen(b64buffer))) {
				DecodeBase64file(sid, b64buffer);
			}
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) return 1;
			}
		} else {
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) break;
				if (inbuffer[0]=='\0') break;
				if (inbuffer[0]!='-') continue;
				if (p_strcasestr(inbuffer, boundary)!=NULL) {
					head=1;
					break;
				}
			}
		}
	}
	return 0;
}

void webmailfiledl(CONN *sid)
{
	FILE *fp=NULL;
	wmheader header;
	char *pQueryString;
	char filename[256];
	char inbuffer[1024];
	char msgfilename[256];
	int msgnum=0;
	int folderid=0;
	SQLRES sqr;

	memset(filename, 0, sizeof(filename));
	pQueryString=strstr(sid->dat->in_RequestURI, "/mail/file/");
	if (pQueryString==NULL) {
		send_error(sid, 400, "Bad Request", "Malformed URI");
		return;
	}
	pQueryString+=11;
	msgnum=atoi(pQueryString);
	while ((isdigit(*pQueryString)!=0)&&(*pQueryString!=0)) pQueryString++;
	while (*pQueryString=='/') pQueryString++;
	strncpy(filename, pQueryString, sizeof(filename)-1);
	decodeurl(filename);
	memset((char *)&header, 0, sizeof(header));
	if (sql_queryf(&sqr, "SELECT mailheaderid, folder, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_email_headers WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, msgnum)<0) return;
	if (sql_numtuples(&sqr)!=1) {
		send_error(sid, 400, "Bad Request", "No such message.");
		sql_freeresult(&sqr);
		return;
	}
	folderid=atoi(sql_getvalue(&sqr, 0, 1));
	snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", sql_getvalue(&sqr, 0, 2));
	snprintf(header.boundary, sizeof(header.boundary)-1, "%s", sql_getvalue(&sqr, 0, 3));
	snprintf(header.encoding, sizeof(header.encoding)-1, "%s", sql_getvalue(&sqr, 0, 4));
	sql_freeresult(&sqr);
	if (p_strcasestr(header.contenttype, "multipart")==NULL) {
		send_error(sid, 400, "Bad Request", "No files are attached to this message.");
		return;
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->user_did, sid->dat->user_mailcurrent, folderid, msgnum);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "r");
	if (fp==NULL) {
		send_error(sid, 400, "Bad Request", "Could not open message.");
		return;
	}
	while (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
		striprn(inbuffer);
		if (strlen(inbuffer)==0) break;
	}
	if (webmailfiledl_r(sid, &fp, header.contenttype, header.boundary, filename, 0)==0) {
		send_error(sid, 400, "Bad Request", "Attachment not found.");
	}
	return;
}

char *webmailfileul(CONN *sid, char *xfilename, char *xfilesize)
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

int webmailmime_line(CONN *sid, char *bodytemp, int szbodytemp, char *inbuffer, char *ctype, char *encoding, short int reply)
{
	char outbuffer1[1024];
	char outbuffer2[1024];
	int bytes=0;

	if (strncasecmp(encoding, "quoted-printable", 16)==0) {
		DecodeQP(outbuffer1, sizeof(outbuffer1)-1, inbuffer);
	} else if (strncasecmp(encoding, "base64", 6)==0) {
		snprintf(outbuffer1, sizeof(outbuffer1)-1, "%s", DecodeBase64string(sid, inbuffer));
	} else {
		snprintf(outbuffer1, sizeof(outbuffer1)-1, "%s", inbuffer);
	}
	if (strncasecmp(ctype, "text/html", 9)==0) {
		DecodeHTML(sid, outbuffer2, sizeof(outbuffer2)-1, outbuffer1, reply);
	} else {
		DecodeText(outbuffer2, sizeof(outbuffer2)-1, outbuffer1);
	}
	if (szbodytemp<0) return 0;
	strncat(bodytemp, outbuffer2, szbodytemp-strlen(bodytemp)-1);
	bytes=strlen(outbuffer2);
	return bytes;
}

int webmailmime(CONN *sid, FILE **fp, char *contenttype, char *encoding, char *boundary, int nummessage, short int reply, short int depth)
{
	char inbuffer[1024];
	char cencode[100];
	char ctype[200];
	char cbound[100];
	char tbound[100];
	char *ptemp;
	char *bodytemp;
	char *sqltemp;
	int szbodytemp=65536;
	int bodysize;
	int file=0;
	int head=0;
	int msgdone=0;
	int i;

	if (depth==0) {
		memset(sid->dat->wm->files, 0, sizeof(sid->dat->wm->files));
		sid->dat->wm->numfiles=0;
	}
	depth++;
	if ((p_strcasestr(contenttype, "multipart")==NULL)&&(p_strcasestr(contenttype, "message/rfc822")==NULL)) {
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
		}
		bodytemp=calloc(szbodytemp, sizeof(char));
		bodysize=0;
		for (;;) {
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
			bodysize+=webmailmime_line(sid, bodytemp+bodysize, szbodytemp-bodysize, inbuffer, contenttype, encoding, reply);
		}
		if (bodysize>0) {
			sqltemp=calloc(bodysize+256, sizeof(char));
			snprintf(sqltemp, bodysize+255, "UPDATE gw_email_headers SET msg_text = '%s' WHERE mailheaderid = %d AND obj_uid = %d AND obj_did = %d AND accountid = %d", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bodytemp), nummessage, sid->dat->user_uid, sid->dat->user_did, sid->dat->user_mailcurrent);
			sql_update(sqltemp);
			free(sqltemp);
		}
		if ((reply)||(strncasecmp(contenttype, "text/html", 9)!=0)) {
			printline(sid, reply, bodytemp);
		} else {
			ptemp=bodytemp;
			while (bodysize>0) {
				if ((i=prints(sid, "%s", ptemp))<1) break;
				ptemp+=i;
			}
		}
		free(bodytemp);
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "</FONT></PRE>");
		}
		return 1;
	}
	if (strncasecmp(contenttype, "message/rfc822", 14)==0) {
		head=1;
	} else {
		for (;;) {
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
			if (p_strcasestr(inbuffer, boundary)!=NULL) {
				head=1;
				break;
			}
		}
	}
	memset(cbound, 0, sizeof(cbound));
	memset(tbound, 0, sizeof(tbound));
	snprintf(tbound, sizeof(tbound)-1, "%s--", boundary);
	for (;;) {
		if (*fp==NULL) break;
		if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
			if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) {
				return msgdone;
			}
		}
		if (head) {
			memset(ctype, 0, sizeof(ctype));
			memset(cencode, 0, sizeof(cencode));
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) break;
				if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
					if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) {
						return msgdone;
					}
				}
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
						wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
						striprn(inbuffer);
						strncat(ctype, inbuffer, sizeof(ctype)-strlen(ctype)-1);
					}
				}
				if ((p_strcasestr(contenttype, "multipart")==NULL)&&(p_strcasestr(contenttype, "message")==NULL)) continue;
				if (file) continue;
				ptemp=p_strcasestr(inbuffer, "name=");
				if (ptemp!=NULL) {
					ptemp+=5;
					if (*ptemp=='\"') ptemp++;
					while ((*ptemp)&&(*ptemp!='\"')&&(strlen(sid->dat->wm->files[sid->dat->wm->numfiles])<sizeof(sid->dat->wm->files[sid->dat->wm->numfiles])-1)) {
						sid->dat->wm->files[sid->dat->wm->numfiles][strlen(sid->dat->wm->files[sid->dat->wm->numfiles])]=*ptemp;
						ptemp++;
					}
					file=1;
					sid->dat->wm->numfiles++;
				}
			}
		}
		if (*fp==NULL) break;
		if (strncasecmp(ctype, "message/rfc822", 14)==0) {
			if (!reply) prints(sid, "<HR>");
			webmailmime(sid, fp, ctype, cencode, boundary, nummessage, reply, depth);
			head=1;
			continue;
		}
		if ((ptemp=p_strcasestr(ctype, "boundary="))!=NULL) {
			ptemp+=9;
			if (*ptemp=='\"') ptemp++;
			snprintf(cbound, sizeof(cbound)-1, "--");
			while ((*ptemp)&&(*ptemp!='\"')&&(strlen(cbound)<sizeof(cbound)-1)) {
				cbound[strlen(cbound)]=*ptemp;
				ptemp++;
			}
		}
		if (p_strcasestr(ctype, "multipart")!=NULL) {
			msgdone=webmailmime(sid, fp, ctype, cencode, cbound, nummessage, reply, depth);
			head=1;
			continue;
		}
		if (*fp==NULL) break;
		if (file) {
			for (;;) {
				wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
				if (*fp==NULL) break;
				if (inbuffer[0]=='\0') { file=0; break; }
				if (inbuffer[0]!='-') continue;
				if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
					if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) {
						return msgdone;
					}
				}
				if (p_strcasestr(inbuffer, boundary)!=NULL) {
					file=0;
					head=1;
					break;
				}
			}
		}
		if (*fp==NULL) break;
		if (head) continue;
		// start reading the part body
		for (;;) {
			if (strcmp(inbuffer, "")==0) break;
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
			if ((p_strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) {
				return msgdone;
			}
		}
		if (*fp==NULL) break;
		if (!reply) {
			if (strncasecmp(contenttype, "multipart/report", 16)==0) {
				prints(sid, "<HR>");
			}
			if ((!msgdone)&&(strncasecmp(ctype, "text/html", 9)!=0)) {
				prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
			} else if (strncasecmp(ctype, "message", 7)==0) {
				prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
			}
		}
		bodytemp=calloc(szbodytemp, sizeof(char));
		bodysize=0;
		for (;;) {
			wmffgets(inbuffer, sizeof(inbuffer)-1, fp);
			if (*fp==NULL) break;
			if ((p_strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) break;
			if (p_strcasestr(inbuffer, boundary)!=NULL) {
				head=1;
				break;
			}
			if ((strncasecmp(contenttype, "multipart/report", 16)!=0)&&(strncasecmp(contenttype, "message/rfc822", 14)!=0)) {
				if (msgdone) continue;
			}
			if ((strncasecmp(ctype, "text/html", 9)!=0)&&(strncasecmp(ctype, "text/plain", 10)!=0)) {
				if (reply) continue;
			}
			if (strncasecmp(ctype, "text/html", 9)==0) {
				bodysize+=webmailmime_line(sid, bodytemp+bodysize, szbodytemp-bodysize, inbuffer, ctype, cencode, reply);
			} else if (strncasecmp(contenttype, "multipart/alternative", 21)!=0) {
				bodysize+=webmailmime_line(sid, bodytemp+bodysize, szbodytemp-bodysize, inbuffer, ctype, cencode, reply);
			}
		}
		if (bodysize>0) {
			sqltemp=calloc(bodysize+256, sizeof(char));
			snprintf(sqltemp, bodysize+255, "UPDATE gw_email_headers SET msg_text = '%s' WHERE mailheaderid = %d AND obj_uid = %d AND obj_did = %d AND accountid = %d", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bodytemp), nummessage, sid->dat->user_uid, sid->dat->user_did, sid->dat->user_mailcurrent);
			sql_update(sqltemp);
			free(sqltemp);
		}
		if ((reply)||(strncasecmp(ctype, "text/html", 9)!=0)) {
			printline(sid, reply, bodytemp);
		} else {
			ptemp=bodytemp;
			while (bodysize>0) {
				if ((i=prints(sid, "%s", ptemp))<1) break;
				ptemp+=i;
			}
		}
		free(bodytemp);
		if (!reply) {
			if ((!msgdone)&&(strncasecmp(ctype, "text/html", 9)!=0)) {
				prints(sid, "</FONT></PRE>\r\n");
			} else if (strncasecmp(ctype, "message", 7)==0) {
				prints(sid, "</FONT></PRE>\r\n");
			}
		}
		if (strncasecmp(ctype, "text/html", 9)==0) msgdone=1;
		if ((strncasecmp(ctype, "text/plain", 10)==0)&&(strncasecmp(contenttype, "multipart/alternative", 21)!=0)) {
			msgdone=1;
		}
		if (*fp==NULL) break;
		if ((p_strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) {
			return msgdone;
		}
	}
	if ((!reply)&&(sid->dat->wm->numfiles>0)&&(depth<2)) {
		prints(sid, "<HR>Attachments<BR>\r\n");
		for (i=0;i<sid->dat->wm->numfiles;i++) {
			prints(sid, "[<A HREF=%s/mail/file/%d/", sid->dat->in_ScriptName, nummessage);
			printhex(sid, "%s", DecodeRFC2047(sid, sid->dat->wm->files[i]));
			ptemp=get_mime_type(sid->dat->wm->files[i]);
			if (strncasecmp(ptemp, "image/", 6)==0) {
				prints(sid, " TARGET=_blank");
			}
			prints(sid, ">");
			printht(sid, "%s", DecodeRFC2047(sid, sid->dat->wm->files[i]));
			prints(sid, "</A>]<BR>\r\n");
		}
	}
	return msgdone;
}