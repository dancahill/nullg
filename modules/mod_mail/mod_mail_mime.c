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
#include "mod_substub.h"
#include "mod_mail.h"

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
			prevheader=header->ReplyTo;
			prevheadersize=sizeof(header->ReplyTo);
			ptemp=inbuffer+9;
			while ((*ptemp==' ')||(*ptemp=='\t')) ptemp++;
			while ((*ptemp)&&(*ptemp!='<')) ptemp++;
			if (*ptemp=='<') ptemp++;
			while ((*ptemp)&&(*ptemp!='>')&&(strlen(header->ReplyTo)<sizeof(header->ReplyTo)-1)) {
				header->ReplyTo[strlen(header->ReplyTo)]=*ptemp;
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
			while ((*ptemp)&&(*ptemp!='>')) ptemp++;
			if (*ptemp=='>') *ptemp='\0';
		}
	}
	if (strlen(header->From)==0) strcpy(header->From, "(No Sender Name)");
	if (strlen(header->Subject)==0) strcpy(header->Subject, "(No Subject)");
	if (strlen(header->Date)==0) strcpy(header->Date, "(No Date)");
	return 0;
}

int webmailfiledl_r(CONNECTION *sid, FILE *fp, char *contenttype, char *encoding, char *boundary, char *filename, short int depth)
{
	char inbuffer[1024];
	char fnamebuf[512];
	char cdisp[128];
	char cencode[100];
	char cfile[256];
	char ctype[200];
	char cbound[100];
	char tbound[100];
	char *ptemp;
	int head=0;
	char *prevheader=NULL;
	int prevheadersize=0;

	depth++;
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (fp==NULL) break;
		if (p_strcasestr(inbuffer, boundary)!=NULL) {
			head=1;
			break;
		}
	}
	memset(cdisp, 0, sizeof(cdisp));
	memset(ctype, 0, sizeof(ctype));
	memset(cbound, 0, sizeof(cbound));
	memset(tbound, 0, sizeof(tbound));
	snprintf(tbound, sizeof(tbound)-1, "%s--", boundary);
	prevheader=NULL;
	for (;;) {
		if (fp==NULL) break;
		if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return 0;
		if (head) {
			memset(cfile, 0, sizeof(cfile));
			memset(cdisp, 0, sizeof(cdisp));
			memset(ctype, 0, sizeof(ctype));
			memset(cencode, 0, sizeof(cencode));
			prevheader=NULL;
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) return 0;
				if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return 0;
				if (strcmp(inbuffer, "")==0) {
					head=0;
					break;
				}
				if (strncasecmp(inbuffer, boundary, strlen(boundary))==0) {
					memset(cfile, 0, sizeof(cfile));
					memset(cdisp, 0, sizeof(cdisp));
					memset(ctype, 0, sizeof(ctype));
					memset(cencode, 0, sizeof(cencode));
					prevheader=NULL;
					continue;
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
				}
				if (strncasecmp(inbuffer, "Content-Type:", 13)==0) {
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
			if (webmailfiledl_r(sid, fp, ctype, cencode, cbound, filename, depth)==1) {
				return 1;
			} else {
				head=1;
				continue;
			}
		}
		if (fp==NULL) break;
		if ((strlen(cencode))&&(strlen(cfile))) {
			ptemp=get_mime_type(cfile);
			if (strncasecmp(ptemp, "image/", 6)!=0) {
				snprintf(sid->dat->out_ContentDisposition, sizeof(sid->dat->out_ContentDisposition)-1, "attachment");
			}
			send_header(sid, 1, 200, "OK", "1", ptemp, -1, -1);
			flushbuffer(sid);
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) return 1;
				if (strncasecmp(inbuffer, boundary, strlen(boundary))==0) break;
				if (strncasecmp(cencode, "base64", 6)==0) {
					DecodeBase64(sid, inbuffer, "");
				} else if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(sid, 0, inbuffer, "");
				} else if (strncasecmp(cencode, "7bit", 4)==0) {
					DecodeText(sid, 0, inbuffer);
				} else if (strncasecmp(cencode, "8bit", 4)==0) {
					DecodeText(sid, 0, inbuffer);
				}
			}
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) return 1;
			}
		} else {
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) break;
				if (strcmp(inbuffer, "")==0) {
					break;
				}
				if (p_strcasestr(inbuffer, boundary)!=NULL) {
					head=1;
					break;
				}
			}
		}
	}
	return 0;
}

void webmailfiledl(CONNECTION *sid)
{
	FILE *fp=NULL;
	wmheader header;
	char *pQueryString;
	char filename[256];
	char inbuffer[1024];
	char msgfilename[256];
	int msgnum=0;
	int sqr;

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
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, msgnum))<0) return;
	if (sql_numtuples(sqr)!=1) {
		send_error(sid, 400, "Bad Request", "No such message.");
		sql_freeresult(sqr);
		return;
	}
	snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", sql_getvalue(sqr, 0, 1));
	snprintf(header.boundary, sizeof(header.boundary)-1, "%s", sql_getvalue(sqr, 0, 2));
	snprintf(header.encoding, sizeof(header.encoding)-1, "%s", sql_getvalue(sqr, 0, 3));
	sql_freeresult(sqr);
	if (p_strcasestr(header.contenttype, "multipart")==NULL) {
		send_error(sid, 400, "Bad Request", "No files are attached to this message.");
		return;
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, msgnum);
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
	if (webmailfiledl_r(sid, fp, header.contenttype, header.encoding, header.boundary, filename, 0)==0) {
		send_error(sid, 400, "Bad Request", "Attachment not found.");
	}
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

int webmailmime(CONNECTION *sid, FILE *fp, char *contenttype, char *encoding, char *boundary, short int nummessage, short int reply, short int depth)
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
	if ((p_strcasestr(contenttype, "multipart")==NULL)&&(p_strcasestr(contenttype, "message/rfc822")==NULL)) {
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "<PRE><FONT FACE=Arial, Verdana>");
		}
		for (;;) {
			wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
			if (fp==NULL) break;
			if (strncasecmp(encoding, "quoted-printable", 16)==0) {
				DecodeQP(sid, reply, inbuffer, contenttype);
			} else if (strncasecmp(encoding, "base64", 6)==0) {
				DecodeBase64(sid, inbuffer, contenttype);
			} else if (strncasecmp(contenttype, "text/html", 9)==0) {
				DecodeHTML(sid, reply, inbuffer, contenttype, 1);
			} else {
				printline(sid, reply, inbuffer);
			}
		}
		if ((!reply)&&(strncasecmp(contenttype, "text/html", 9)!=0)) {
			prints(sid, "</FONT></PRE>");
		}
		return 1;
	}
	if (strncasecmp(contenttype, "message/rfc822", 14)==0) {
		head=1;
	} else {
		for (;;) {
			wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
			if (fp==NULL) break;
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
		if (fp==NULL) break;
		if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
			if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
		}
		if (head) {
			memset(ctype, 0, sizeof(ctype));
			memset(cencode, 0, sizeof(cencode));
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) break;
				if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
					if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
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
						wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
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
					while ((*ptemp)&&(*ptemp!='\"')&&(strlen(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])<sizeof(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])-1)) {
						sid->dat->user_wmfiles[sid->dat->user_wmnumfiles][strlen(sid->dat->user_wmfiles[sid->dat->user_wmnumfiles])]=*ptemp;
						ptemp++;
					}
					file=1;
					sid->dat->user_wmnumfiles++;
				}
			}
		}

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
		if (fp==NULL) break;
		if (file) {
			for (;;) {
				wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
				if (fp==NULL) break;
				if ((p_strcasestr(contenttype, "multipart")!=NULL)||(p_strcasestr(contenttype, "message")!=NULL)) {
					if ((depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
				}
				if (strcmp(inbuffer, "")==0) {
					file=0;
					break;
				}
				if (p_strcasestr(inbuffer, boundary)!=NULL) {
					file=0;
					head=1;
					break;
				}
			}
		}
		if (fp==NULL) break;
		if (head) continue;
		// start reading the part body
		for (;;) {
			if (strcmp(inbuffer, "")==0) break;
			wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
			if (fp==NULL) break;
			if ((p_strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
		}
		if (fp==NULL) break;
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
		for (;;) {
			wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
			if (fp==NULL) break;
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
				if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(sid, reply, inbuffer, ctype);
				} else if (strncasecmp(cencode, "base64", 6)==0) {
					DecodeBase64(sid, inbuffer, ctype);
				} else {
					DecodeHTML(sid, reply, inbuffer, ctype, 1);
				}
			} else if (strncasecmp(contenttype, "multipart/alternative", 21)!=0) {
				if (strncasecmp(cencode, "quoted-printable", 16)==0) {
					DecodeQP(sid, reply, inbuffer, ctype);
				} else if (strncasecmp(cencode, "base64", 6)==0) {
					DecodeBase64(sid, inbuffer, ctype);
				} else {
					printline(sid, reply, inbuffer);
				}
			}
		}
		if (!reply) {
			if ((!msgdone)&&(strncasecmp(ctype, "text/html", 9)!=0)) {
				prints(sid, "</FONT></PRE>\n");
			} else if (strncasecmp(ctype, "message", 7)==0) {
				prints(sid, "</FONT></PRE>\n");
			}
		}
		if (strncasecmp(ctype, "text/html", 9)==0) msgdone=1;
		if ((strncasecmp(ctype, "text/plain", 10)==0)&&(strncasecmp(contenttype, "multipart/alternative", 21)!=0)) {
			msgdone=1;
		}
		if (fp==NULL) break;
		if ((p_strcasestr(contenttype, "multipart")!=NULL)&&(depth>1)&&(p_strcasestr(inbuffer, tbound)!=NULL)) return msgdone;
	}
	if ((!reply)&&(sid->dat->user_wmnumfiles>0)) {
		prints(sid, "<HR>Attachments<BR>\n");
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
