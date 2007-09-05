/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "httpd_main.h"

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

static void read_postdata(CONN *sid)
{
	obj_t *cobj;
	int maxplen=(int)nes_getnum(proc->N, nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd"), "max_post_size");
	int postlen=(int)nes_getnum(sid->N, nes_settable(sid->N, &sid->N->g, "_SERVER"), "CONTENT_LENGTH");
	char *PostData, *p;
	int bytesleft;
	int rc;
	int i;

	cobj=nes_getobj(sid->N, &sid->N->g, "POSTRAWDATA");
	if (cobj->val->type!=NT_NULL) return;
	bytesleft=postlen;
	if (bytesleft<0) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", postlen);
		postlen=0;
		closeconnect(sid, 1);
		return;
	} else if (bytesleft>maxplen) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length of %d is too large.", postlen);
		postlen=0;
		closeconnect(sid, 1);
		return;
	}
	if ((PostData=calloc(postlen+64, sizeof(char)))==NULL) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "Memory allocation error while reading POST data.");
		closeconnect(sid, 1);
	}
	p=PostData;
	/*
	 * If we already snorted part of the POST data while reading
	 * the header, then copy it here.
	 */
	/* what broke this? */
	/* if (sid->socket.recvbufsize>bytesleft) { */
	if (sid->socket.recvbufsize-2>bytesleft) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length is %d bytes larger than advertised (%d).", sid->socket.recvbufsize>bytesleft, postlen);
/*
//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: %d %d %d %d", postlen, nes_getnum(sid->N, htobj, "CONTENT_LENGTH"), sid->socket.recvbufsize, bytesleft);
//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: [%s]", sid->socket.recvbuf+sid->socket.recvbufoffset);
*/
		postlen=0;
		closeconnect(sid, 1);
		return;
	}
	i=sid->socket.recvbufsize;
	memcpy(p, sid->socket.recvbuf+sid->socket.recvbufoffset, sid->socket.recvbufsize);
	memset(sid->socket.recvbuf, 0, sizeof(sid->socket.recvbuf));
	p+=sid->socket.recvbufsize;
	bytesleft-=sid->socket.recvbufsize;
	sid->socket.recvbufoffset=0;
	sid->socket.recvbufsize=0;
	while (bytesleft>0) {
		rc=tcp_recv(&sid->socket, p, bytesleft, 0);
		if (rc<0) {
			closeconnect(sid, 1);
			return;
		} else if (rc>0) {
			sid->socket.atime=time(NULL);
			i+=rc;
		}
		bytesleft-=rc;
		p+=rc;
	}
	cobj=nes_setstr(sid->N, &sid->N->g, "POSTRAWDATA", NULL, 0);
	cobj->val->attr|=NST_HIDDEN;
	cobj->val->size=i;
	cobj->val->d.str=PostData;
	/* log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "[%s]", sid->PostData); */
	return;
}

static int read_vardata(CONN *sid)
{
	obj_t *servobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	obj_t *gobj=nes_settable(sid->N, &sid->N->g, "_GET");
	obj_t *pobj=nes_settable(sid->N, &sid->N->g, "_POST");
	obj_t *cobj;
	char boundary[100];
	int blen;
	int clen;
	int maxplen=(int)nes_getnum(proc->N, nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "CONFIG"), "httpd"), "max_post_size");
	char tmpbuf[MAX_OBJNAMELEN];
	char tmpbuf2[MAX_OBJNAMELEN+5];
	char filename[255];
	char *QueryString=nes_getstr(sid->N, servobj, "QUERY_STRING");
	char *p1, *p2;
	unsigned int len;
	int i;

	if (maxplen<8192) maxplen=8192;
	p1=QueryString;
	while (*p1!='\0') {
		for (i=0;i<MAX_OBJNAMELEN;i++) {
			if (*p1=='\0') { tmpbuf[i]='\0'; break; }
			if (*p1=='=') { tmpbuf[i]='\0'; p1++; break; }
			tmpbuf[i]=toupper(*p1);
			p1++;
		}
		tmpbuf[MAX_OBJNAMELEN-1]='\0';
		for (i=0;;i++) {
			if (p1[i]=='\0') { len=i; break; }
			if (p1[i]=='&') { len=i; i++; break; }
		}
		cobj=nes_setstr(sid->N, gobj, tmpbuf, p1, len);
		if (cobj->val->d.str) {
			decodeurl(cobj->val->d.str); cobj->val->size=strlen(cobj->val->d.str);
		}
		p1+=i;
	}
	if (strcmp(nes_getstr(sid->N, servobj, "REQUEST_METHOD"), "POST")==0) {
		clen=(int)nes_getnum(sid->N, servobj, "CONTENT_LENGTH");
		if (clen>maxplen) {
			/* try to print an error : note the inbuffer is still
			 * full, so the cgi will probably just puke, and die.
			 * But at least it'll do it quickly. ;-)
			 */
			send_header(sid, 0, 200, "1", "text/html", -1, -1);
//			prints(sid, "ERROR: Content-Length of %d is too large.", clen);
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length of %d is too large.", clen);
			closeconnect(sid, 1);
			return -1;
		}
		read_postdata(sid);
		p1=nes_getstr(sid->N, servobj, "CONTENT_TYPE");
		/* CONTENT_TYPE = "multipart/form-data; boundary=---------------------------7d723815a700f2" */
		if (strncmp(p1, "multipart/form-data", strlen("multipart/form-data"))==0) {
			/* start reading MIME */
			if ((p1=strstr(p1, "boundary"))==NULL) {
				/* error */
				return -1;
			}
			p1+=8;
			if (*p1!='=') {
				/* error */
				return -1;
			}
			p1++;
			if ((*p1=='\'')||(*p1=='\"')) p1++;
			for (i=0;i<sizeof(boundary);i++) {
				if ((*p1=='\0')||(*p1==';')||(*p1=='\'')||(*p1=='\"'))  { boundary[i]='\0'; break; }
				boundary[i]=*p1;
				p1++;
			}
			boundary[sizeof(boundary)-1]='\0';
			blen=strlen(boundary);
			cobj=nes_setstr(sid->N, &sid->N->g, "POSTBOUNDARY", boundary, blen);
			cobj->val->attr|=NST_HIDDEN;
			p1=nes_getstr(sid->N, &sid->N->g, "POSTRAWDATA");
			p2=p1;
			do {
				if (p2-p1>clen) break;
				if (strncmp("--", p2, 2)!=0) { p2++; continue; }
				p2+=2;
				if (strncmp(boundary, p2, blen)!=0) { p2++; continue; }
				/* start part reading */
				p2+=blen;
				while ((*p2=='\r')||(*p2=='\n')) p2++;
				if (strncasecmp(p2, "Content-Disposition: form-data; ", 32)!=0) { p2++; continue; }
				p2+=32;
				if (strncasecmp(p2, "name=", 5)!=0) continue;
				p2+=5;
				if (*p2=='\"') p2++;
				for (i=0;i<MAX_OBJNAMELEN;i++) {
					if ((*p2=='\0')||(*p2==';')) { tmpbuf[i]='\0'; break; }
					if ((*p2=='\'')||(*p2=='\"')) { tmpbuf[i]='\0'; break; }
					if ((*p2=='\r')||(*p2=='\n')) { tmpbuf[i]='\0'; break; }
					tmpbuf[i]=toupper(*p2);
					p2++;
				}
				tmpbuf[MAX_OBJNAMELEN-1]='\0';
				if (*p2=='\"') p2++;
				filename[0]='\0';
				if (*p2==';') {
					p2++;
					while (isspace(*p2)) p2++;
					if (strncasecmp(p2, "filename=", 9)!=0) continue;
					p2+=9;
					if (*p2=='\"') p2++;
					for (i=0;i<sizeof(filename);i++) {
						if ((*p2=='\0')||(*p2==';')) { filename[i]='\0'; break; }
						if ((*p2=='\'')||(*p2=='\"')) { filename[i]='\0'; break; }
						if ((*p2=='\r')||(*p2=='\n')) { filename[i]='\0'; break; }
						if ((*p2=='\\')||(*p2=='/')) { i=-1; p2++; continue; }
						filename[i]=*p2;
						p2++;
					}
					filename[sizeof(filename)-1]='\0';
				}
				while ((*p2)&&(*p2!='\r')&&(*p2!='\n')) p2++;
				if (*p2=='\r') p2++; if (*p2=='\n') p2++;
				while ((*p2)&&(*p2!='\r')&&(*p2!='\n')) {
					while ((*p2)&&(*p2!='\r')&&(*p2!='\n')) p2++;
					if (*p2=='\r') p2++; if (*p2=='\n') p2++;
				}
				if (*p2=='\r') p2++; if (*p2=='\n') p2++;
				for (i=0;;i++) {
					/* be careful.  this may be binary. */
					/* if ((p2[i]=='\0')||(p2+i-p1>clen)) break; */
					if (p2+i-p1>clen) break;
					if (strncmp(boundary, p2+i, blen)==0) break;
				}
				/* read backward 4 chars */
				if (p2[i-1]=='-') i--; if (p2[i-1]=='-') i--;
				if (p2[i-1]=='\n') i--; if (p2[i-1]=='\r') i--;
				cobj=nes_setstr(sid->N, pobj, tmpbuf, NULL, 0);
				if (i) {
					cobj->val->d.str=malloc(i+1);
					if (cobj->val->d.str) {
						memcpy(cobj->val->d.str, p2, i);
						cobj->val->d.str[i]='\0';
					}
				}
				cobj->val->size=cobj->val->d.str?i:0;
				/* decodeurl(sid->dat->smallbuf[1]); */
				/* does any of this need to be decoded?  ever?  */
				if (filename[0]!='\0') {
					cobj->val->attr|=NST_HIDDEN; /* hide attached files from var dumps */
					snprintf(tmpbuf2, sizeof(tmpbuf2)-1, "%s_name", tmpbuf);
					nes_setstr(sid->N, pobj, tmpbuf2, filename, strlen(filename));
					snprintf(tmpbuf2, sizeof(tmpbuf2)-1, "%s_size", tmpbuf);
					nes_setnum(sid->N, pobj, tmpbuf2, i);
				}
				p2+=i;
				/* end part reading */
			} while (1);
			/* done reading MIME */
		} else if (strncmp(p1, "text/xml", strlen("text/xml"))==0) {
			/* XML data isn't handled here yet, but it should be */
		} else {
			p1=nes_getstr(sid->N, &sid->N->g, "POSTRAWDATA");
			while (*p1!='\0') {
				for (i=0;i<MAX_OBJNAMELEN;i++) {
					if (*p1=='\0') { tmpbuf[i]='\0'; break; }
					if (*p1=='=') { tmpbuf[i]='\0'; p1++; break; }
					tmpbuf[i]=toupper(*p1);
					p1++;
				}
				tmpbuf[MAX_OBJNAMELEN-1]='\0';
				for (i=0;;i++) {
					if (p1[i]=='\0') { len=i; break; }
					if (p1[i]=='&') { len=i; i++; break; }
				}
				cobj=nes_setstr(sid->N, pobj, tmpbuf, p1, len);
				if (cobj->val->d.str) {
					decodeurl(cobj->val->d.str); cobj->val->size=strlen(cobj->val->d.str);
				}
				p1+=i;
			}
		}
	}
	return 0;
}

char *getgetenv(CONN *sid, char *query)
{
	obj_t *cobj=nes_getobj(sid->N, nes_settable(sid->N, &sid->N->g, "_GET"), query);

	if (cobj->val->type==NT_NULL) return NULL;
	return nes_tostr(sid->N, cobj);
}

char *getpostenv(CONN *sid, char *query)
{
	obj_t *cobj=nes_getobj(sid->N, nes_settable(sid->N, &sid->N->g, "_POST"), query);

	if (cobj->val->type==NT_NULL) return NULL;
	return nes_tostr(sid->N, cobj);
}

char *getxmlenv(CONN *sid, char *query)
{
	obj_t *cobj=nes_getobj(sid->N, &sid->N->g, "POSTRAWDATA");
	char *buffer=getbuffer(sid);
	char *pToken;

	if (cobj->val->type!=NT_STRING) return NULL;
	if ((pToken=cobj->val->d.str)==NULL) return NULL;
	while (*pToken) {
		if ((strncasecmp(pToken, query, strlen(query))!=0)) {
			while ((*pToken)&&(*pToken!='<')) pToken++;
			if (*pToken=='<') pToken++;
			continue;
		}
		if ((*pToken)&&(pToken[strlen(query)]!=' ')&&(pToken[strlen(query)]!='/')&&(pToken[strlen(query)]!='>')) { pToken++; continue; }
		while ((*pToken)&&(*pToken==' ')) {
			pToken++;
		}
		if (*pToken=='/') return "";
		pToken=strchr(pToken, '>');
		if (pToken==NULL) break;
		pToken++;
		while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
			buffer[strlen(buffer)]=*pToken;
			pToken++;
		}
		decodeurl(buffer);
		return buffer;
	}
	return NULL;
}

char *getxmlparam(CONN *sid, int param, char *reqtype)
{
	obj_t *cobj=nes_getobj(sid->N, &sid->N->g, "POSTRAWDATA");
	char *buffer=getbuffer(sid);
	char *pToken;
	char type[20];
	int params=0;

	if (cobj->val->type!=NT_STRING) return NULL;
	if ((pToken=cobj->val->d.str)==NULL) return NULL;
	memset(type, 0, sizeof(type));
	snprintf(type, sizeof(type)-1, "<%s>", reqtype);
	while (*pToken) {
		while ((strncasecmp(pToken, "<param>", strlen("<param>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		params++;
		pToken+=strlen("<param>");
		if (param!=params) continue;
		while ((strncasecmp(pToken, "<value>", strlen("<value>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<value>");
		while ((strncasecmp(pToken, type, strlen(type))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen(type);
		if (param==params) {
			while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
				buffer[strlen(buffer)]=*pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
	}
	return NULL;
}

char *getxmlstruct(CONN *sid, char *reqmember, char *reqtype)
{
	obj_t *cobj=nes_getobj(sid->N, &sid->N->g, "POSTRAWDATA");
	char *buffer=getbuffer(sid);
	char *pToken;
	char type[32];

	if (cobj->val->type!=NT_STRING) return NULL;
	if ((pToken=cobj->val->d.str)==NULL) return NULL;
	memset(type, 0, sizeof(type));
	while ((strncasecmp(pToken, "<struct>", strlen("<struct>"))!=0)) {
		if (*pToken=='\0') return NULL;
		pToken++;
	}
	pToken+=strlen("<struct>");
	while (*pToken) {
		while ((strncasecmp(pToken, "<member>", strlen("<member>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<member>");
		while ((strncasecmp(pToken, "<name>", strlen("<name>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<name>");
		/* if the name matches, we never leave this block, btw */
		if ((strncasecmp(pToken, reqmember, strlen(reqmember))==0)) {
			pToken+=strlen(reqmember);
			if (*pToken!='<') continue; /* oops..  wrong member.. never mind what i just said */
			while ((strncasecmp(pToken, "</name>", strlen("</name>"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("</name>");
			while ((strncasecmp(pToken, "<value>", strlen("<value>"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("<value>");
			while ((strncasecmp(pToken, "<", strlen("<"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("<");
			while ((*pToken)&&(*pToken!='>')&&(strlen(type)<sizeof(type)-1)) {
				type[strlen(type)]=*pToken;
				pToken++;
			}
			if (*pToken=='>') pToken++;
			while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
				buffer[strlen(buffer)]=*pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
		while ((strncasecmp(pToken, "</name>", strlen("</name>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("</name>");
		while ((strncasecmp(pToken, "</member>", strlen("</member>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("</member>");
	}
	return NULL;
}

char *get_mime_type(CONN *sid, char *name)
{
	obj_t *cobj, *mobj=nes_getobj(sid->N, &sid->N->g, "mime_types");
	char *ext=strrchr(name, '.');

	if (ext==NULL) return "text/plain";
	if (mobj->val->type!=NT_TABLE) {
		mobj=nes_settable(sid->N, &sid->N->g, "mime_types");
		htnes_dotemplate(sid, "", "mime.ns");
	}
	cobj=nes_getobj(sid->N, mobj, ext+1);
	if (cobj->val->type==NT_STRING) {
		return cobj->val->d.str?cobj->val->d.str:"";
	}
	return "application/octet-stream";
}

int read_header(CONN *sid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *newconfobj;
	obj_t *hobj=nes_settable(proc->N, confobj, "httpd");
	obj_t *hcobj, *hrobj, *hsobj, *cobj;
	char varname[64];
	char line[2048];
	char *RequestMethod;
	char *RequestURI;
	char *pt1, *pt2, *pt3;
	char *ptemp;
	int length;
	time_t maxtime;
	time_t x;

	x=time(NULL);
	maxtime=x+(time_t)nes_getnum(proc->N, hobj, "max_keepalive");
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) {
			return -1;
		}
		striprn(line);
		if (strlen(line)>0) break;
		if (time(NULL)>maxtime) {
			closeconnect(sid, 2);
			return -1;
		}
	} while (1);
	sid->N=nes_newstate();
	hcobj=nes_settable(sid->N, &sid->N->g, "_COOKIE");
	hcobj->val->attr|=NST_AUTOSORT;
	hrobj=nes_settable(sid->N, &sid->N->g, "_HEADER");
	hrobj->val->attr|=NST_AUTOSORT;
	hsobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	hsobj->val->attr|=NST_AUTOSORT;
	nes_setstr(sid->N, hsobj, "CONTENT_TYPE",      "application/x-www-form-urlencoded", strlen("application/x-www-form-urlencoded"));
	nes_setstr(sid->N, hsobj, "GATEWAY_INTERFACE", "CGI/1.1", strlen("CGI/1.1"));
	nes_setstr(sid->N, hsobj, "REMOTE_ADDR",       sid->socket.RemoteAddr, strlen(sid->socket.RemoteAddr));
	nes_setnum(sid->N, hsobj, "REMOTE_PORT",       sid->socket.RemotePort);
	nes_setstr(sid->N, hsobj, "REMOTE_USER",       "nobody", strlen("nobody"));
	nes_setstr(sid->N, hsobj, "SERVER_ADDR",       sid->socket.ServerAddr, strlen(sid->socket.ServerAddr));
	nes_setnum(sid->N, hsobj, "SERVER_PORT",       sid->socket.ServerPort);
	snprintf(varname, sizeof(varname)-1, "%s %s", SERVER_NAME, PACKAGE_VERSION);
	nes_setstr(sid->N, hsobj, "SERVER_SOFTWARE",   varname, strlen(varname));
	nes_setstr(sid->N, hsobj, "NESLA_VERSION",     NESLA_VERSION, strlen(NESLA_VERSION));
	sid->state=1;
	pt1=line;
	if ((pt2=strchr(pt1, ' '))==NULL) { send_error(sid, 400, "Bad Request", "Method not found in request."); return -1; } else *pt2++='\0';
	if ((pt3=strchr(pt2, ' '))==NULL) { send_error(sid, 400, "Bad Request", "Method not found in request."); return -1; } else *pt3++='\0';
	ptemp=pt1;
	while (*ptemp) { *ptemp=toupper(*ptemp); ptemp++; };
	if (strlen(pt1)==0) { send_error(sid, 400, "Bad Request", "Method not found in request."); return -1; }
	if (strlen(pt2)==0) { send_error(sid, 400, "Bad Request", "URI not found in request."); return -1; }
	if (strlen(pt3)==0) { send_error(sid, 400, "Bad Request", "Protocol not found in request."); return -1; }
	cobj=nes_setstr(sid->N, hsobj, "REQUEST_METHOD",  pt1, strlen(pt1));
	RequestMethod=cobj->val->d.str?cobj->val->d.str:"";
	cobj=nes_setstr(sid->N, hsobj, "REQUEST_URI",     pt2, strlen(pt2));
	RequestURI=cobj->val->d.str?cobj->val->d.str:"";
	cobj=nes_setstr(sid->N, hsobj, "SERVER_PROTOCOL", pt3, strlen(pt3));
	/* log_error(proc->N, MODSHORTNAME "headers", __FILE__, __LINE__, 1, "%s:%d [%s][%s][%s]", sid->socket.RemoteAddr, sid->socket.RemotePort, pt1, pt2, pt3); */
	while (strlen(line)>0) {
		if (tcp_fgets(line, sizeof(line)-1, &sid->socket)<0) return -1;
		striprn(line);
		if (!strlen(line)) break;
		pt1=line;
		if ((pt2=strchr(pt1, ':'))==NULL) { send_error(sid, 400, "Bad Request", "Broken header."); return -1; } else *pt2++='\0';
		while (*pt2==' ') pt2++;
		pt3=pt1; while (*pt3) { if (*pt3=='-') *pt3='_'; else *pt3=toupper(*pt3); pt3++; }
		snprintf(varname, sizeof(varname)-1, "HTTP_%s", pt1);
		/* log_error(proc->N, MODSHORTNAME "headers", __FILE__, __LINE__, 1, "[%s][%s]", varname, pt2); */
		if (strcmp(varname, "HTTP_CONTENT_LENGTH")==0) {
			length=atoi(pt2);
			if (length<0) {
				log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", length);
				length=0;
			}
			nes_setnum(sid->N, hsobj, "CONTENT_LENGTH", length);
		} else if (strcmp(varname, "HTTP_CONTENT_TYPE")==0) {
			nes_setstr(sid->N, hsobj, "CONTENT_TYPE", pt2, strlen(pt2));
		} else if (strcmp(varname, "HTTP_COOKIE")==0) {
			nes_setstr(sid->N, hsobj, "HTTP_COOKIE", pt2, strlen(pt2));
			for (;;) {
				pt1=pt2;
				if ((pt2=strchr(pt1, '='))==NULL) { send_error(sid, 400, "Bad Request", "Undefined cookie error."); return -1; } else *pt2++='\0';
				if ((pt3=strchr(pt2, ';'))!=NULL) { *pt3++='\0'; while (*pt3==' ') pt3++; }
				nes_setstr(sid->N, hcobj, pt1, pt2, strlen(pt2));
				if (pt3==NULL) break;
				if (!strlen(pt3)) break;
				pt2=pt3;
			}
		} else {
			nes_setstr(sid->N, hsobj, varname, pt2, strlen(pt2));
		}
	}
	if (strcasecmp(nes_getstr(sid->N, hsobj, "HTTP_CONNECTION"), "Keep-Alive")==0) {
		nes_setstr(sid->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	} else {
		if ((strcasecmp(nes_getstr(sid->N, hrobj, "PROTOCOL"), "HTTP/1.1")==0)&&(strlen(nes_getstr(sid->N, hsobj, "HTTP_CONNECTION"))==0)) {
			nes_setstr(sid->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
		} else {
			nes_setstr(sid->N, hrobj, "CONNECTION", "Close", strlen("Close"));
		}
	}
	if ((strcmp(RequestMethod, "GET")!=0)&&(strcmp(RequestMethod, "POST")!=0)) {
		send_error(sid, 501, "Not Implemented", "That method is not implemented.");
		closeconnect(sid, 1);
		return -1;
	}
	if (strcmp(RequestMethod, "POST")==0) {
		if (nes_getnum(sid->N, hsobj, "CONTENT_LENGTH")<nes_getnum(proc->N, hobj, "max_post_size")) {
			read_postdata(sid);
		} else {
			/* try to print an error : note the inbuffer being full may block us */
			send_error(sid, 413, "Bad Request", "Request entity too large.");
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s - Large POST (>%d bytes) disallowed", sid->socket.RemoteAddr, nes_getnum(proc->N, hobj, "max_post_size"));
			closeconnect(sid, 1);
			return -1;
		}
	}
	if (RequestURI[0]!='/') {
		send_error(sid, 400, "Bad Request", "Bad filename.");
	}
	if ((ptemp=strchr(RequestURI, '?'))!=NULL) {
		nes_setstr(sid->N, hsobj, "QUERY_STRING", ptemp+1, strlen(ptemp+1));
//		nes_setstr(sid->N, hsobj, "SCRIPT_NAME", RequestURI, ptemp-RequestURI);

	} else {
		nes_setstr(sid->N, hsobj, "QUERY_STRING", "", 0);
//		nes_setstr(sid->N, hsobj, "SCRIPT_NAME", RequestURI, strlen(RequestURI));
	}
	nes_setstr(sid->N, hsobj, "SCRIPT_NAME", NULL, 0);
	snprintf(line, sizeof(line)-1, "%s", nes_getstr(sid->N, hsobj, "HTTP_HOST"));
	if ((ptemp=strchr(line, ':'))!=NULL) *ptemp=0;
	nes_setstr(sid->N, hsobj, "SERVER_NAME", line, strlen(line));

	newconfobj=nes_settable(sid->N, &sid->N->g, "_CONFIG");
	newconfobj->val->attr|=NST_AUTOSORT;
//	cobj=nes_getobj(proc->N, confobj, "var_path");
//	if (nes_isstr(cobj)) nes_setstr(sid->N, newconfobj, "var_path", cobj->val->d.str, cobj->val->size);
	cobj=nes_getobj(proc->N, confobj, "default_language");
	if (nes_isstr(cobj)) nes_setstr(sid->N, newconfobj, "default_language", cobj->val->d.str, cobj->val->size);
	cobj=nes_getobj(proc->N, confobj, "sql_server_type");
	if (nes_isstr(cobj)) nes_setstr(sid->N, newconfobj, "sql_server_type", cobj->val->d.str, cobj->val->size);
	cobj=nes_getobj(proc->N, confobj, "host_name");
	if (nes_isstr(cobj)) nes_setstr(sid->N, newconfobj, "host_name", cobj->val->d.str, cobj->val->size);
/*
	snprintf(line, sizeof(line)-1, "%s/%04d/htdocs", nes_getstr(proc->N, confobj, "var_domains_path"), sid->dat->did);
	nes_setstr(sid->N, newconfobj, "private_htdocs_path", line, -1);
	snprintf(line, sizeof(line)-1, "%s/%04d/template", nes_getstr(proc->N, confobj, "var_domains_path"), sid->dat->did);
	nes_setstr(sid->N, newconfobj, "private_template_path", line, -1);
*/
	snprintf(line, sizeof(line)-1, "%s/share/htdocs", nes_getstr(proc->N, confobj, "var_path"));
	nes_setstr(sid->N, newconfobj, "shared_htdocs_path", line, -1);
	snprintf(line, sizeof(line)-1, "%s/share/template", nes_getstr(proc->N, confobj, "var_path"));
	nes_setstr(sid->N, newconfobj, "shared_template_path", line, -1);

	nes_linkval(sid->N, nes_settable(sid->N, newconfobj, "MODULES"), nes_getobj(proc->N, &proc->N->g, "GWMODULES"));

	/* cheat and peek at the master process's global var table */
//	nes_linkval(sid->N, nes_settable(sid->N, &sid->N->g, "MASTERGLOBAL"), &proc->N->g);
//	nes_linkval(sid->N, nes_settable(sid->N, &sid->N->g, "HTTPDGLOBAL"), &htproc.N->g);

	return 0;
}

/*
 * this needs a real rewrite...
 * path_info is the script name as seen in the uri
 * path_translated is the fully qualified fs path to the real script
 * script_name and script_filename are only used for C G I !
 * script_name is the cgi name as seen from the uri
 * script_filename is the fully qualified fs path to the real cgi
 */
static int read_header_scriptinfo(CONN *sid, int domainid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char progname[255];
	char scriptname[255];
	char scriptfilename[255];
	char *ptemp;
	struct stat sb;
	int length;

	strncpy(scriptname, nes_getstr(sid->N, htobj, "REQUEST_URI"), sizeof(scriptname)-1);
	if ((ptemp=strchr(scriptname, '?'))!=NULL) *ptemp='\0';
	if (domainid>0) {
		if (strncmp(scriptname, "/cgi-bin/", 9)==0) {
			snprintf(progname, sizeof(progname)-1, "%s", scriptname+8);
			snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s/%04d/cgi-bin%s", nes_getstr(proc->N, confobj, "var_domains_path"), domainid, progname);
		} else if (strncmp(scriptname, "/", 1)==0) {
			snprintf(progname, sizeof(progname)-1, "%s", scriptname);
			snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s/%04d/htdocs%s", nes_getstr(proc->N, confobj, "var_domains_path"), domainid, progname);
		}
	} else {
		if (strncmp(scriptname, "/cgi-bin/", 9)==0) {
			snprintf(progname, sizeof(progname)-1, "%s", scriptname+8);
			snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s%s", nes_getstr(proc->N, confobj, "var_cgi_path"), progname);
		} else if (strncmp(scriptname, "/", 1)==0) {
			snprintf(progname, sizeof(progname)-1, "%s", scriptname);
			snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s%s", nes_getstr(proc->N, confobj, "var_htdocs_path"), progname);
		}
	}
	fixslashes(scriptfilename);
	length=strlen(scriptfilename);
	do {
		if ((stat(scriptfilename, &sb)==0)&&(!(sb.st_mode&S_IFDIR))) break;
		if ((ptemp=strrchr(scriptfilename, '/'))!=NULL) {
			*ptemp='\0';
#ifdef WIN32
			if ((strlen(scriptfilename)<3)&&((ptemp=strrchr(scriptfilename, ':'))!=NULL)) scriptfilename[0]='\0';
#endif
		} else {
			break;
		}
	} while (strlen(scriptfilename)>0);
	fixslashes(scriptfilename);
	length=length-strlen(scriptfilename);
	if (length>0) scriptname[strlen(scriptname)-length]='\0';
	ptemp=progname+strlen(scriptname);
	if (strlen(scriptfilename)>0) {
//		nes_setstr(sid->N, htobj, "SCRIPT_FILENAME", scriptfilename, strlen(scriptfilename));
		nes_setstr(sid->N, htobj, "PATH_TRANSLATED", scriptfilename, strlen(scriptfilename));
//		nes_setstr(sid->N, htobj, "SCRIPT_NAME", scriptname, strlen(scriptname));
		nes_setstr(sid->N, htobj, "PATH_INFO", scriptname, strlen(scriptname));
//		if (strlen(ptemp)>0) {
//			nes_setstr(sid->N, htobj, "PATH_INFO", ptemp, strlen(ptemp));
//		}
	} else {
		if (domainid>0) read_header_scriptinfo(sid, 0);
	}
	/* prints(sid, "[[%s]][%s][%s]<BR />", nes_getstr(sid->N, htobj, "SCRIPT_NAME"), scriptname, scriptfilename); */
	return 0;
}

void send_error(CONN *sid, int status, char* title, char* text)
{
	obj_t *tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");
	char *p;

	nes_setnum(sid->N, tobj, "status", status);
	nes_setstr(sid->N, tobj, "title", title, strlen(title));
	nes_setstr(sid->N, tobj, "theme", sid->dat->theme, strlen(sid->dat->theme));
	p=str2html(sid, text);
	nes_setstr(sid->N, tobj, "text", p, strlen(p));
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (htnes_dotemplate(sid, "html", "error.ns")!=0) {
		prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
		prints(sid, "<HTML>\r\n<HEAD>\r\n");
		prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=us-ascii\">\r\n");
		prints(sid, "<TITLE>%d %s</TITLE>\r\n</HEAD>\r\n", status, title);
		prints(sid, "<BODY BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\">\r\n");
		prints(sid, "<H1>%d %s</H1>\r\n", status, title);
		prints(sid, "%s\r\n", p);
		prints(sid, "<HR>\r\n<ADDRESS>%s %s</ADDRESS>\r\n</BODY></HTML>\r\n", SERVER_NAME, PACKAGE_VERSION);
	}
	sid->dat->out_bodydone=1;
	flushbuffer(sid);
	log_htaccess(sid);
	closeconnect(sid, 1);
	return;
}

void send_header(CONN *sid, int cacheable, int status, char *extra_header, char *mime_type, int length, time_t mod)
{
	obj_t *hrobj=nes_getobj(sid->N, &sid->N->g, "_HEADER");
	char timebuf[100];
	time_t now;

	if (status) {
		sid->dat->out_status=status;
	} else {
		sid->dat->out_status=200;
	}
	if (length>=0) {
		sid->dat->out_ContentLength=length;
	}
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		nes_setstr(sid->N, hrobj, "LAST_MODIFIED", timebuf, strlen(timebuf));
	}
	now=time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	nes_setstr(sid->N, hrobj, "DATE", timebuf, strlen(timebuf));
	if (cacheable) {
		nes_setstr(sid->N, hrobj, "CACHE_CONTROL", "public", strlen("public"));
		nes_setstr(sid->N, hrobj, "PRAGMA", "public", strlen("public"));
	} else {
		nes_setstr(sid->N, hrobj, "CACHE_CONTROL", "no-cache, no-store, must-revalidate", strlen("no-cache, no-store, must-revalidate"));
		nes_setstr(sid->N, hrobj, "EXPIRES", timebuf, strlen(timebuf));
		nes_setstr(sid->N, hrobj, "PRAGMA", "no-cache", strlen("no-cache"));
	}
	if (extra_header!=(char*)0) {
		nes_setstr(sid->N, hrobj, "CONTENT_TYPE", mime_type, strlen(mime_type));
	} else {
		nes_setstr(sid->N, hrobj, "CONTENT_TYPE", "text/html", strlen("text/html"));
	}
}

int test_add(CONN *sid, char *path, char *name, char *ext)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	struct stat sb;
	char file[512];
	char *p;

	snprintf(file, sizeof(file)-strlen(ext)-1, "%s%s", path, name);
	if ((p=strchr(file, '?'))!=NULL) *p='\0';
	strcat(file, ext);
//	fixslashes(file);
	if (file[0]!='\0') { p=file+strlen(file)-1; if ((*p=='\\')||(*p=='/')) *p='\0'; }
	if (stat(file, &sb)==0) {
//		nes_setstr(sid->N, htobj, "SCRIPT_FILENAME", file, -1);
		nes_setstr(sid->N, htobj, "PATH_TRANSLATED", file, -1);
//		nes_setstr(sid->N, htobj, "SCRIPT_NAME", file+strlen(path), -1);
		nes_setstr(sid->N, htobj, "PATH_INFO", file+strlen(path), -1);
//		nes_setstr(sid->N, htobj, "REQUEST_URI", file+strlen(path), -1);
		return 1;
	}
	return 0;
}

void http_dorequest(CONN *sid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj, *htobj, *hrobj, *tobj;
	char buf[255];
	char *RequestMethod;
	char *RequestURI;
	char *ptemp;
	int relocate=0;
	int rc;

	if (read_header(sid)<0) {
		closeconnect(sid, 1);
		sid->N=nes_endstate(sid->N);
		return;
	}
	tobj=nes_getobj(sid->N, &sid->N->g, "io");
	nes_setcfunc(sid->N, tobj, "flush", (void *)htnes_flush);
	gettimeofday(&sid->dat->runtime, NULL);

	htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	hrobj=nes_getobj(sid->N, &sid->N->g, "_HEADER");
	RequestMethod=nes_getstr(sid->N, htobj, "REQUEST_METHOD");
	RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	proc->stats.http_pages++;

	/* log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 3, "%s - HTTP Request: %s:%d %s", sid->socket.RemoteAddr, sid->socket.RemotePort, RequestMethod, RequestURI); */
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf)-1, "%s%s", nes_getstr(proc->N, confobj, "var_htdocs_path"), RequestURI);
	if (strncmp(RequestURI, "/icons/", strlen("/icons/"))==0) {
		if (filesend(sid, buf)==0) goto wrap;
	}
	if (strncmp(RequestURI, "/"SERVER_BASENAME"/", strlen("/"SERVER_BASENAME"/"))==0) {
		if (filesend(sid, buf)==0) goto wrap;
	}
	if (strncmp(RequestURI, "/xml-rpc/", 9)==0) {
		if (module_menucall(sid)) goto wrap;
	}
	nes_setstr(sid->N, hrobj, "CONTENT_TYPE", "text/html", strlen("text/html"));
	rc=auth_getcookie(sid);
//	read_header_scriptinfo(sid, sid->dat->did);
//	ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	if ((rc==0)&&(strncmp(RequestURI, "/logout", 7)==0)) {
		auth_logout(sid);
		goto wrap;
	}
	read_vardata(sid);

	if (rc!=0) {
		if (auth_setcookie(sid)==0) {
			db_log_activity(sid, "login", 0, "login", "%s - Login: username=%s", sid->socket.RemoteAddr, sid->dat->username);
			relocate=1;
		} else if ((getgetenv(sid, "USERNAME")!=NULL)||(getpostenv(sid, "USERNAME")!=NULL)) {
			db_log_activity(sid, "login", 0, "failed login", "%s - Login failed: username=%s", sid->socket.RemoteAddr, sid->dat->username);
			htpage_login(sid);
			goto wrap;
		} else {
			htpage_login(sid);
			goto wrap;
		}
	}

	if ((ptemp=getpostenv(sid, "PAGEURI"))!=NULL) {
		snprintf(buf, sizeof(buf)-1, "%s", ptemp);
	} else {
		snprintf(buf, sizeof(buf)-1, "%s", RequestURI);
	}
	if ((ptemp=p_strcasestr(buf, "username"))!=NULL) { *ptemp='\0'; relocate=1; }
	if ((ptemp=p_strcasestr(buf, "password"))!=NULL) { *ptemp='\0'; relocate=1; }
	if (buf[strlen(buf)-1]=='&') buf[strlen(buf)-1]='\0';
	if (buf[strlen(buf)-1]=='?') buf[strlen(buf)-1]='\0';

	if (relocate) {
		nes_setstr(sid->N, hrobj, "LOCATION", buf, strlen(buf));
		send_header(sid, 0, 302, "1", "text/html", -1, -1);
		goto wrap;
	}

	if (1) {
		obj_t *newconfobj=nes_settable(sid->N, &sid->N->g, "_CONFIG");

		snprintf(buf, sizeof(buf)-1, "%s/%04d/htdocs", nes_getstr(proc->N, confobj, "var_domains_path"), sid->dat->did);
		nes_setstr(sid->N, newconfobj, "private_htdocs_path", buf, -1);
		snprintf(buf, sizeof(buf)-1, "%s/%04d/template", nes_getstr(proc->N, confobj, "var_domains_path"), sid->dat->did);
		nes_setstr(sid->N, newconfobj, "private_template_path", buf, -1);
	}

	if (strcmp(RequestURI, "/")==0) {
		htpage_motd(sid);
		goto wrap;
	}
//	if ((ext=strrchr(ScriptName, '.'))!=NULL) {
//		if ((strcmp(ext, ".ns")==0)||(strcmp(ext, ".nes")==0)) {
//			if (htnes_doscript(sid)>-1) goto wrap;
//		}
//	}
	cobj=nes_getobj(sid->N, nes_settable(sid->N, &sid->N->g, "_CONFIG"), "private_htdocs_path");
	if (nes_isstr(cobj)) {
		if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, "")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		} else if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, ".ns")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		} else if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, ".nes")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		}
	}
	/* if it's not in the private htdocs, try the public */
	cobj=nes_getobj(sid->N, nes_settable(sid->N, &sid->N->g, "_CONFIG"), "shared_htdocs_path");
	if (nes_isstr(cobj)) {
		if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, "")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		} else if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, ".ns")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		} else if (test_add(sid, nes_tostr(sid->N, cobj), RequestURI, ".nes")==1) {
			if (htnes_doscript(sid)>-1) goto wrap;
		}
	}
	RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
	if (strncmp(RequestURI, "/logout", 7)==0) {
		auth_logout(sid);
	} else if (module_menucall(sid)) {
		goto wrap;
	} else {
		snprintf(buf, sizeof(buf)-1, "The requested URL '%s' was not found on this server.", RequestURI);
		send_error(sid, 404, "Not Found", buf);
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 2, "%s - Bad request '%s' by %s", sid->socket.RemoteAddr, RequestURI, sid->dat->username);
	}
wrap:
	if (sid->N->err) {
		prints(sid, "<HR><B>[errno=%d :: %s]</B>\r\n", sid->N->err, sid->N->errbuf);
	}
	auth_savesession(sid);
	sid->dat->out_bodydone=1;
	flushbuffer(sid);
	log_htaccess(sid);
	if (sid->dat->wm!=NULL) {
		tcp_close(&sid->dat->wm->socket, 1);
		free(sid->dat->wm);
		sid->dat->wm=NULL;
	}
	return;
}
