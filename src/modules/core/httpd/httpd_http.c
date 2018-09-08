/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

static void read_postdata(CONN *conn)
{
	obj_t *cobj;
	int maxplen = (int)nsp_getnum(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd"), "max_post_size");
	int postlen = (int)nsp_getnum(conn->N, nsp_settable(conn->N, &conn->N->g, "_SERVER"), "CONTENT_LENGTH");
	char *PostData, *p;
	int bytesleft;
	int rc;
	int i;

	cobj = nsp_getobj(conn->N, &conn->N->g, "POSTRAWDATA");
	if (cobj->val->type != NT_NULL) return;
	bytesleft = postlen;
	if (bytesleft < 0) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", postlen);
		postlen = 0;
		closeconnect(conn, 1);
		return;
	}
	else if (bytesleft > maxplen) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length of %d is too large.", postlen);
		postlen = 0;
		closeconnect(conn, 1);
		return;
	}
	if ((PostData = calloc(postlen + 64, sizeof(char))) == NULL) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "Memory allocation error while reading POST data.");
		closeconnect(conn, 1);
	}
	p = PostData;
	/*
	 * If we already snorted part of the POST data while reading
	 * the header, then copy it here.
	 */
	 /* what broke this? */
	 /* if (conn->socket.recvbufsize>bytesleft) { */
	if (conn->socket.recvbufsize - 2 > bytesleft) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length is %d bytes larger than advertised (%d).", conn->socket.recvbufsize > bytesleft, postlen);
		/*
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: %d %d %d %d", postlen, nsp_getnum(conn->N, htobj, "CONTENT_LENGTH"), conn->socket.recvbufsize, bytesleft);
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: [%s]", conn->socket.recvbuf+conn->socket.recvbufoffset);
		*/
		postlen = 0;
		closeconnect(conn, 1);
		return;
	}
	i = conn->socket.recvbufsize;
	memcpy(p, conn->socket.recvbuf + conn->socket.recvbufoffset, conn->socket.recvbufsize);
	memset(conn->socket.recvbuf, 0, sizeof(conn->socket.recvbuf));
	p += conn->socket.recvbufsize;
	bytesleft -= conn->socket.recvbufsize;
	conn->socket.recvbufoffset = 0;
	conn->socket.recvbufsize = 0;
	while (bytesleft > 0) {
		rc = tcp_recv(&conn->socket, p, bytesleft, 0);
		if (rc < 0) {
			closeconnect(conn, 1);
			return;
		}
		else if (rc > 0) {
			conn->socket.atime = time(NULL);
			i += rc;
		}
		bytesleft -= rc;
		p += rc;
	}
	cobj = nsp_setstr(conn->N, &conn->N->g, "POSTRAWDATA", NULL, 0);
	cobj->val->attr |= NST_HIDDEN;
	cobj->val->size = i;
	cobj->val->d.str = PostData;
	/* log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "[%s]", conn->PostData); */
	return;
}

static int read_vardata(CONN *conn)
{
	obj_t *servobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *gobj = nsp_settable(conn->N, &conn->N->g, "_GET");
	obj_t *pobj = nsp_settable(conn->N, &conn->N->g, "_POST");
	obj_t *fobj;
	obj_t *cobj;
	char boundary[100];
	int blen;
	int clen;
	int maxplen = (int)nsp_getnum(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, nsp_settable(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd"), "max_post_size");
	char tmpbuf[MAX_OBJNAMELEN];
	//char tmpbuf2[MAX_OBJNAMELEN + 5];
	char filename[255];
	char *QueryString = nsp_getstr(conn->N, servobj, "QUERY_STRING");
	char *p1, *p2;
	unsigned int len;
	int i;

	if (maxplen < 8192) maxplen = 8192;
	p1 = QueryString;
	while (*p1 != '\0') {
		for (i = 0;i < MAX_OBJNAMELEN;i++) {
			if (*p1 == '\0') { tmpbuf[i] = '\0'; break; }
			if (*p1 == '=') { tmpbuf[i] = '\0'; p1++; break; }
			tmpbuf[i] = toupper(*p1);
			p1++;
		}
		tmpbuf[MAX_OBJNAMELEN - 1] = '\0';
		for (i = 0;;i++) {
			if (p1[i] == '\0') { len = i; break; }
			if (p1[i] == '&') { len = i; i++; break; }
		}
		cobj = nsp_setstr(conn->N, gobj, tmpbuf, p1, len);
		if (cobj->val->d.str) {
			decodeurl(cobj->val->d.str); cobj->val->size = strlen(cobj->val->d.str);
		}
		p1 += i;
	}
	if (strcmp(nsp_getstr(conn->N, servobj, "REQUEST_METHOD"), "POST") == 0) {
		clen = (int)nsp_getnum(conn->N, servobj, "CONTENT_LENGTH");
		if (clen > maxplen) {
			/* try to print an error : note the inbuffer is still
			 * full, so the cgi will probably just puke, and die.
			 * But at least it'll do it quickly. ;-)
			 */
			send_header(conn, 0, 200, "1", "text/html", -1, -1);
			//			prints(conn, "ERROR: Content-Length of %d is too large.", clen);
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: Content-Length of %d is too large.", clen);
			closeconnect(conn, 1);
			return -1;
		}
		read_postdata(conn);
		p1 = nsp_getstr(conn->N, servobj, "CONTENT_TYPE");
		/* CONTENT_TYPE = "multipart/form-data; boundary=---------------------------7d723815a700f2" */
		if (strncmp(p1, "multipart/form-data", strlen("multipart/form-data")) == 0) {
			/* start reading MIME */
			if ((p1 = strstr(p1, "boundary")) == NULL) {
				/* error */
				return -1;
			}
			p1 += 8;
			if (*p1 != '=') {
				/* error */
				return -1;
			}
			p1++;
			if ((*p1 == '\'') || (*p1 == '\"')) p1++;
			for (i = 0;i < sizeof(boundary);i++) {
				if ((*p1 == '\0') || (*p1 == ';') || (*p1 == '\'') || (*p1 == '\"')) { boundary[i] = '\0'; break; }
				boundary[i] = *p1;
				p1++;
			}
			boundary[sizeof(boundary) - 1] = '\0';
			blen = strlen(boundary);
			cobj = nsp_setstr(conn->N, &conn->N->g, "POSTBOUNDARY", boundary, blen);
			cobj->val->attr |= NST_HIDDEN;
			p1 = nsp_getstr(conn->N, &conn->N->g, "POSTRAWDATA");
			p2 = p1;
			do {
				if (p2 - p1 > clen) break;
				if (strncmp("--", p2, 2) != 0) { p2++; continue; }
				p2 += 2;
				if (strncmp(boundary, p2, blen) != 0) { p2++; continue; }
				/* start part reading */
				p2 += blen;
				while ((*p2 == '\r') || (*p2 == '\n')) p2++;
				if (strncasecmp(p2, "Content-Disposition: form-data; ", 32) != 0) { p2++; continue; }
				p2 += 32;
				if (strncasecmp(p2, "name=", 5) != 0) continue;
				p2 += 5;
				if (*p2 == '\"') p2++;
				for (i = 0;i < MAX_OBJNAMELEN;i++) {
					if ((*p2 == '\0') || (*p2 == ';')) { tmpbuf[i] = '\0'; break; }
					if ((*p2 == '\'') || (*p2 == '\"')) { tmpbuf[i] = '\0'; break; }
					if ((*p2 == '\r') || (*p2 == '\n')) { tmpbuf[i] = '\0'; break; }
					tmpbuf[i] = toupper(*p2);
					p2++;
				}
				tmpbuf[MAX_OBJNAMELEN - 1] = '\0';
				if (*p2 == '\"') p2++;
				filename[0] = '\0';
				if (*p2 == ';') {
					p2++;
					while (isspace(*p2)) p2++;
					if (strncasecmp(p2, "filename=", 9) != 0) continue;
					p2 += 9;
					if (*p2 == '\"') p2++;
					for (i = 0;i < sizeof(filename);i++) {
						if ((*p2 == '\0') || (*p2 == ';')) { filename[i] = '\0'; break; }
						if ((*p2 == '\'') || (*p2 == '\"')) { filename[i] = '\0'; break; }
						if ((*p2 == '\r') || (*p2 == '\n')) { filename[i] = '\0'; break; }
						if ((*p2 == '\\') || (*p2 == '/')) { i = -1; p2++; continue; }
						filename[i] = *p2;
						p2++;
					}
					filename[sizeof(filename) - 1] = '\0';
				}
				while ((*p2) && (*p2 != '\r') && (*p2 != '\n')) p2++;
				if (*p2 == '\r') p2++; if (*p2 == '\n') p2++;
				while ((*p2) && (*p2 != '\r') && (*p2 != '\n')) {
					while ((*p2) && (*p2 != '\r') && (*p2 != '\n')) p2++;
					if (*p2 == '\r') p2++; if (*p2 == '\n') p2++;
				}
				if (*p2 == '\r') p2++; if (*p2 == '\n') p2++;
				for (i = 0;;i++) {
					/* be careful.  this may be binary. */
					/* if ((p2[i]=='\0')||(p2+i-p1>clen)) break; */
					if (p2 + i - p1 > clen) break;
					if (strncmp(boundary, p2 + i, blen) == 0) break;
				}
				/* read backward 4 chars */
				if (p2[i - 1] == '-') i--; if (p2[i - 1] == '-') i--;
				if (p2[i - 1] == '\n') i--; if (p2[i - 1] == '\r') i--;
				if (filename[0] != '\0') {
					fobj = nsp_settable(conn->N, pobj, tmpbuf);
					cobj = nsp_setstr(conn->N, fobj, "DATA", NULL, 0);

					//cobj->val->attr |= NST_HIDDEN; /* hide attached files from var dumps */
					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_name", tmpbuf);
					//nsp_setstr(conn->N, fobj, tmpbuf2, filename, strlen(filename));
					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_size", tmpbuf);
					//nsp_setnum(conn->N, fobj, tmpbuf2, i);

					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_name", tmpbuf);
					nsp_setstr(conn->N, fobj, "FILENAME", filename, -1);
					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_size", tmpbuf);
					nsp_setnum(conn->N, fobj, "SIZE", i);

				}
				else {
					cobj = nsp_setstr(conn->N, pobj, tmpbuf, NULL, 0);
				}
				if (i) {
					cobj->val->d.str = malloc(i + 1);
					if (cobj->val->d.str) {
						memcpy(cobj->val->d.str, p2, i);
						cobj->val->d.str[i] = '\0';
					}
				}
				cobj->val->size = cobj->val->d.str ? i : 0;
				/* decodeurl(conn->dat->smallbuf[1]); */
				/* does any of this need to be decoded?  ever?  */
				//if (filename[0] != '\0') {
					//cobj->val->attr |= NST_HIDDEN; /* hide attached files from var dumps */
					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_name", tmpbuf);
					//nsp_setstr(conn->N, fobj, tmpbuf2, filename, strlen(filename));
					//snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_size", tmpbuf);
					//nsp_setnum(conn->N, fobj, tmpbuf2, i);
				//}
				p2 += i;
				/* end part reading */
			} while (1);
			/* done reading MIME */
		}
		else if (strncmp(p1, "text/xml", strlen("text/xml")) == 0) {
			/* XML data isn't handled here yet, but it should be */
		}
		else {
			p1 = nsp_getstr(conn->N, &conn->N->g, "POSTRAWDATA");
			while (*p1 != '\0') {
				for (i = 0;i < MAX_OBJNAMELEN;i++) {
					if (*p1 == '\0') { tmpbuf[i] = '\0'; break; }
					if (*p1 == '=') { tmpbuf[i] = '\0'; p1++; break; }
					tmpbuf[i] = toupper(*p1);
					p1++;
				}
				tmpbuf[MAX_OBJNAMELEN - 1] = '\0';
				for (i = 0;;i++) {
					if (p1[i] == '\0') { len = i; break; }
					if (p1[i] == '&') { len = i; i++; break; }
				}
				cobj = nsp_setstr(conn->N, pobj, tmpbuf, p1, len);
				if (cobj->val->d.str) {
					decodeurl(cobj->val->d.str); cobj->val->size = strlen(cobj->val->d.str);
				}
				p1 += i;
			}
		}
	}
	return 0;
}

char *getgetenv(CONN *conn, char *query)
{
	obj_t *cobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_GET"), query);

	if (cobj->val->type == NT_NULL) return NULL;
	return nsp_tostr(conn->N, cobj);
}

char *getpostenv(CONN *conn, char *query)
{
	obj_t *cobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_POST"), query);

	if (cobj->val->type == NT_NULL) return NULL;
	return nsp_tostr(conn->N, cobj);
}

char *getxmlenv(CONN *conn, char *query)
{
	obj_t *cobj = nsp_getobj(conn->N, &conn->N->g, "POSTRAWDATA");
	char *buffer = getbuffer(conn);
	char *pToken;

	if (cobj->val->type != NT_STRING) return NULL;
	if ((pToken = cobj->val->d.str) == NULL) return NULL;
	while (*pToken) {
		if ((strncasecmp(pToken, query, strlen(query)) != 0)) {
			while ((*pToken) && (*pToken != '<')) pToken++;
			if (*pToken == '<') pToken++;
			continue;
		}
		if ((*pToken) && (pToken[strlen(query)] != ' ') && (pToken[strlen(query)] != '/') && (pToken[strlen(query)] != '>')) { pToken++; continue; }
		while ((*pToken) && (*pToken == ' ')) {
			pToken++;
		}
		if (*pToken == '/') return "";
		pToken = strchr(pToken, '>');
		if (pToken == NULL) break;
		pToken++;
		while ((*pToken) && (*pToken != '<') && (strlen(buffer) < sizeof(conn->dat->smallbuf[0]) - 1)) {
			buffer[strlen(buffer)] = *pToken;
			pToken++;
		}
		decodeurl(buffer);
		return buffer;
	}
	return NULL;
}

char *getxmlparam(CONN *conn, int param, char *reqtype)
{
	obj_t *cobj = nsp_getobj(conn->N, &conn->N->g, "POSTRAWDATA");
	char *buffer = getbuffer(conn);
	char *pToken;
	char type[20];
	int params = 0;

	if (cobj->val->type != NT_STRING) return NULL;
	if ((pToken = cobj->val->d.str) == NULL) return NULL;
	memset(type, 0, sizeof(type));
	snprintf(type, sizeof(type) - 1, "<%s>", reqtype);
	while (*pToken) {
		while ((strncasecmp(pToken, "<param>", strlen("<param>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		params++;
		pToken += strlen("<param>");
		if (param != params) continue;
		while ((strncasecmp(pToken, "<value>", strlen("<value>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen("<value>");
		while ((strncasecmp(pToken, type, strlen(type)) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen(type);
		if (param == params) {
			while ((*pToken) && (*pToken != '<') && (strlen(buffer) < sizeof(conn->dat->smallbuf[0]) - 1)) {
				buffer[strlen(buffer)] = *pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
	}
	return NULL;
}

char *getxmlstruct(CONN *conn, char *reqmember, char *reqtype)
{
	obj_t *cobj = nsp_getobj(conn->N, &conn->N->g, "POSTRAWDATA");
	char *buffer = getbuffer(conn);
	char *pToken;
	char type[32];

	if (cobj->val->type != NT_STRING) return NULL;
	if ((pToken = cobj->val->d.str) == NULL) return NULL;
	memset(type, 0, sizeof(type));
	while ((strncasecmp(pToken, "<struct>", strlen("<struct>")) != 0)) {
		if (*pToken == '\0') return NULL;
		pToken++;
	}
	pToken += strlen("<struct>");
	while (*pToken) {
		while ((strncasecmp(pToken, "<member>", strlen("<member>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen("<member>");
		while ((strncasecmp(pToken, "<name>", strlen("<name>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen("<name>");
		/* if the name matches, we never leave this block, btw */
		if ((strncasecmp(pToken, reqmember, strlen(reqmember)) == 0)) {
			pToken += strlen(reqmember);
			if (*pToken != '<') continue; /* oops..  wrong member.. never mind what i just said */
			while ((strncasecmp(pToken, "</name>", strlen("</name>")) != 0)) {
				if (*pToken == '\0') return NULL;
				pToken++;
			}
			pToken += strlen("</name>");
			while ((strncasecmp(pToken, "<value>", strlen("<value>")) != 0)) {
				if (*pToken == '\0') return NULL;
				pToken++;
			}
			pToken += strlen("<value>");
			while ((strncasecmp(pToken, "<", strlen("<")) != 0)) {
				if (*pToken == '\0') return NULL;
				pToken++;
			}
			pToken += strlen("<");
			while ((*pToken) && (*pToken != '>') && (strlen(type) < sizeof(type) - 1)) {
				type[strlen(type)] = *pToken;
				pToken++;
			}
			if (*pToken == '>') pToken++;
			while ((*pToken) && (*pToken != '<') && (strlen(buffer) < sizeof(conn->dat->smallbuf[0]) - 1)) {
				buffer[strlen(buffer)] = *pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
		while ((strncasecmp(pToken, "</name>", strlen("</name>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen("</name>");
		while ((strncasecmp(pToken, "</member>", strlen("</member>")) != 0)) {
			if (*pToken == '\0') return NULL;
			pToken++;
		}
		pToken += strlen("</member>");
	}
	return NULL;
}

char *get_mime_type(CONN *conn, char *name)
{
	obj_t *cobj, *mobj = nsp_getobj(conn->N, &conn->N->g, "mime_types");
	char *ext = strrchr(name, '.');

	if (ext == NULL) return "text/plain";
	if (mobj->val->type != NT_TABLE) {
		mobj = nsp_settable(conn->N, &conn->N->g, "mime_types");
		htnsp_dotemplate(conn, "", "mime.ns");
	}
	cobj = nsp_getobj(conn->N, mobj, ext + 1);
	if (cobj->val->type == NT_STRING) {
		return cobj->val->d.str ? cobj->val->d.str : "";
	}
	return "application/octet-stream";
}

int read_header(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *newconfobj;
	obj_t *hobj = nsp_settable(proc->N, nsp_getobj(proc->N, confobj, "modules"), "httpd");
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

	x = time(NULL);
	maxtime = x + (time_t)nsp_getnum(proc->N, hobj, "max_keepalive");
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) {
			return -1;
		}
		striprn(line);
		if (strlen(line) > 0) break;
		if (time(NULL) > maxtime) {
			closeconnect(conn, 2);
			return -1;
		}
		if (conn->socket.want_close) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "Closing connection [%s:%d]", conn->socket.RemoteAddr, conn->socket.RemotePort);
			closeconnect(conn, 2);
			return -1;
		}
	} while (1);
	conn->N = nsp_newstate();
	conn->N->warnformat = 'h';
	hcobj = nsp_settable(conn->N, &conn->N->g, "_COOKIE");
	hcobj->val->attr |= NST_AUTOSORT;
	hrobj = nsp_settable(conn->N, &conn->N->g, "_HEADER");
	hrobj->val->attr |= NST_AUTOSORT;
	hsobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	hsobj->val->attr |= NST_AUTOSORT;
	nsp_setstr(conn->N, hsobj, "CONTENT_TYPE", "application/x-www-form-urlencoded", -1);
	nsp_setstr(conn->N, hsobj, "GATEWAY_INTERFACE", "CGI/1.1", -1);
	nsp_setstr(conn->N, hsobj, "REMOTE_ADDR", conn->socket.RemoteAddr, -1);
	nsp_setnum(conn->N, hsobj, "REMOTE_PORT", conn->socket.RemotePort);
	nsp_setstr(conn->N, hsobj, "REMOTE_USER", "nobody", -1);
	nsp_setstr(conn->N, hsobj, "SERVER_ADDR", conn->socket.ServerAddr, -1);
	nsp_setnum(conn->N, hsobj, "SERVER_PORT", conn->socket.ServerPort);
	snprintf(varname, sizeof(varname) - 1, "%s %s", SERVER_NAME, PACKAGE_VERSION);
	nsp_setstr(conn->N, hsobj, "SERVER_SOFTWARE", varname, -1);
	nsp_setstr(conn->N, hsobj, "NSP_VERSION", NSP_VERSION, -1);
	conn->state = 1;
	pt1 = line;
	if ((pt2 = strchr(pt1, ' ')) == NULL) { send_error(conn, 400, "Bad Request", "Method not found in request."); return -1; }
	else *pt2++ = '\0';
	if ((pt3 = strchr(pt2, ' ')) == NULL) { send_error(conn, 400, "Bad Request", "Method not found in request."); return -1; }
	else *pt3++ = '\0';
	ptemp = pt1;
	while (*ptemp) { *ptemp = toupper(*ptemp); ptemp++; };
	if (strlen(pt1) == 0) { send_error(conn, 400, "Bad Request", "Method not found in request."); return -1; }
	if (strlen(pt2) == 0) { send_error(conn, 400, "Bad Request", "URI not found in request."); return -1; }
	if (strlen(pt3) == 0) { send_error(conn, 400, "Bad Request", "Protocol not found in request."); return -1; }
	cobj = nsp_setstr(conn->N, hsobj, "REQUEST_METHOD", pt1, strlen(pt1));
	RequestMethod = cobj->val->d.str ? cobj->val->d.str : "";
	cobj = nsp_setstr(conn->N, hsobj, "REQUEST_URI", pt2, strlen(pt2));

	/* CHEAP HACK */
	// nsp_exec(conn->N, "_SERVER['REQUEST_URI']=string.replace(_SERVER['REQUEST_URI'], '%20', ' ');");
	/* a slightly less cheap hack, but still bad */
	decodeurl(cobj->val->d.str); cobj->val->size = strlen(cobj->val->d.str);

	RequestURI = cobj->val->d.str ? cobj->val->d.str : "";
	cobj = nsp_setstr(conn->N, hsobj, "SERVER_PROTOCOL", pt3, strlen(pt3));
	/* log_error(proc->N, MODSHORTNAME "headers", __FILE__, __LINE__, 1, "%s:%d [%s][%s][%s]", conn->socket.RemoteAddr, conn->socket.RemotePort, pt1, pt2, pt3); */
	while (strlen(line) > 0) {
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) return -1;
		striprn(line);
		if (!strlen(line)) break;
		pt1 = line;
		if ((pt2 = strchr(pt1, ':')) == NULL) { send_error(conn, 400, "Bad Request", "Broken header."); return -1; }
		else *pt2++ = '\0';
		while (*pt2 == ' ') pt2++;
		pt3 = pt1; while (*pt3) { if (*pt3 == '-') *pt3 = '_'; else *pt3 = toupper(*pt3); pt3++; }
		snprintf(varname, sizeof(varname) - 1, "HTTP_%s", pt1);
		/* log_error(proc->N, MODSHORTNAME "headers", __FILE__, __LINE__, 1, "[%s][%s]", varname, pt2); */
		if (strcmp(varname, "HTTP_CONTENT_LENGTH") == 0) {
			length = atoi(pt2);
			if (length < 0) {
				log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", length);
				length = 0;
			}
			nsp_setnum(conn->N, hsobj, "CONTENT_LENGTH", length);
		}
		else if (strcmp(varname, "HTTP_CONTENT_TYPE") == 0) {
			nsp_setstr(conn->N, hsobj, "CONTENT_TYPE", pt2, -1);
		}
		else if (strcmp(varname, "HTTP_COOKIE") == 0) {
			nsp_setstr(conn->N, hsobj, "HTTP_COOKIE", pt2, -1);
			for (;;) {
				pt1 = pt2;
				if ((pt2 = strchr(pt1, '=')) == NULL) { send_error(conn, 400, "Bad Request", "Undefined cookie error."); return -1; }
				else *pt2++ = '\0';
				if ((pt3 = strchr(pt2, ';')) != NULL) { *pt3++ = '\0'; while (*pt3 == ' ') pt3++; }
				nsp_setstr(conn->N, hcobj, pt1, pt2, strlen(pt2));
				if (pt3 == NULL) break;
				if (!strlen(pt3)) break;
				pt2 = pt3;
			}
		}
		else {
			nsp_setstr(conn->N, hsobj, varname, pt2, strlen(pt2));
		}
	}
	if (strcasecmp(nsp_getstr(conn->N, hsobj, "HTTP_CONNECTION"), "Keep-Alive") == 0) {
		nsp_setstr(conn->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
	}
	else {
		if ((strcasecmp(nsp_getstr(conn->N, hrobj, "PROTOCOL"), "HTTP/1.1") == 0) && (strlen(nsp_getstr(conn->N, hsobj, "HTTP_CONNECTION")) == 0)) {
			nsp_setstr(conn->N, hrobj, "CONNECTION", "Keep-Alive", strlen("Keep-Alive"));
		}
		else {
			nsp_setstr(conn->N, hrobj, "CONNECTION", "Close", strlen("Close"));
		}
	}
	if ((strcmp(RequestMethod, "GET") != 0) && (strcmp(RequestMethod, "POST") != 0)) {
		send_error(conn, 501, "Not Implemented", "That method is not implemented.");
		closeconnect(conn, 1);
		return -1;
	}
	if (strcmp(RequestMethod, "POST") == 0) {
		if (nsp_getnum(conn->N, hsobj, "CONTENT_LENGTH") < nsp_getnum(proc->N, hobj, "max_post_size")) {
			read_postdata(conn);
		}
		else {
			/* try to print an error : note the inbuffer being full may block us */
			send_error(conn, 413, "Bad Request", "Request entity too large.");
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s - Large POST (>%d bytes) disallowed", conn->socket.RemoteAddr, nsp_getnum(proc->N, hobj, "max_post_size"));
			closeconnect(conn, 1);
			return -1;
		}
	}
	if (RequestURI[0] != '/') {
		send_error(conn, 400, "Bad Request", "Bad filename.");
	}
	if ((ptemp = strchr(RequestURI, '?')) != NULL) {
		nsp_setstr(conn->N, hsobj, "QUERY_STRING", ptemp + 1, strlen(ptemp + 1));
		//		nsp_setstr(conn->N, hsobj, "SCRIPT_NAME", RequestURI, ptemp-RequestURI);

	}
	else {
		nsp_setstr(conn->N, hsobj, "QUERY_STRING", "", 0);
		//		nsp_setstr(conn->N, hsobj, "SCRIPT_NAME", RequestURI, strlen(RequestURI));
	}
	nsp_setstr(conn->N, hsobj, "SCRIPT_NAME", NULL, 0);
	snprintf(line, sizeof(line) - 1, "%s", nsp_getstr(conn->N, hsobj, "HTTP_HOST"));
	if ((ptemp = strchr(line, ':')) != NULL) *ptemp = 0;
	nsp_setstr(conn->N, hsobj, "SERVER_NAME", line, strlen(line));

	newconfobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	newconfobj->val->attr |= NST_AUTOSORT;
	cobj = nsp_getobj(proc->N, confobj, "default_language");
	if (nsp_isstr(cobj)) nsp_setstr(conn->N, newconfobj, "default_language", cobj->val->d.str, cobj->val->size);
	cobj = nsp_getobj(proc->N, confobj, "sql_server_type");
	if (nsp_isstr(cobj)) nsp_setstr(conn->N, newconfobj, "sql_server_type", cobj->val->d.str, cobj->val->size);
	cobj = nsp_getobj(proc->N, confobj, "host_name");
	if (nsp_isstr(cobj)) nsp_setstr(conn->N, newconfobj, "host_name", cobj->val->d.str, cobj->val->size);

	nsp_setstr(conn->N, nsp_settable(conn->N, newconfobj, "paths"), "lib", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), -1);
	nsp_setstr(conn->N, nsp_settable(conn->N, newconfobj, "paths"), "var", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), -1);

	nsp_linkval(conn->N, nsp_settable(conn->N, newconfobj, "MODULES"), nsp_getobj(proc->N, &proc->N->g, "GWMODULES"));

	/* cheat and peek at the master process's global var table */
	nsp_linkval(conn->N, nsp_settable(conn->N, &conn->N->g, "MASTERGLOBAL"), &proc->N->g);
	//	nsp_linkval(conn->N, nsp_settable(conn->N, &conn->N->g, "HTTPDGLOBAL"), &htproc.N->g);

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
 /*
 static int read_header_scriptinfo(CONN *conn, int domainid)
 {
	 obj_t *confobj=nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	 obj_t *htobj=nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	 char progname[255];
	 char scriptname[255];
	 char scriptfilename[255];
	 char *ptemp;
	 struct stat sb;
	 int length;

	 strncpy(scriptname, nsp_getstr(conn->N, htobj, "REQUEST_URI"), sizeof(scriptname)-1);
	 if ((ptemp=strchr(scriptname, '?'))!=NULL) *ptemp='\0';
	 if (domainid>0) {
		 if (strncmp(scriptname, "/cgi-bin/", 9)==0) {
			 snprintf(progname, sizeof(progname)-1, "%s", scriptname+8);
			 snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s/%04d/cgi-bin%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var_domains"), domainid, progname);
		 } else if (strncmp(scriptname, "/", 1)==0) {
			 snprintf(progname, sizeof(progname)-1, "%s", scriptname);
			 snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s/%04d/htdocs%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var_domains"), domainid, progname);
		 }
	 } else {
		 if (strncmp(scriptname, "/cgi-bin/", 9)==0) {
			 snprintf(progname, sizeof(progname)-1, "%s", scriptname+8);
			 snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var_cgi"), progname);
		 } else if (strncmp(scriptname, "/", 1)==0) {
			 snprintf(progname, sizeof(progname)-1, "%s", scriptname);
			 snprintf(scriptfilename, sizeof(scriptfilename)-1, "%s%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var_htdocs"), progname);
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
 //		nsp_setstr(conn->N, htobj, "SCRIPT_FILENAME", scriptfilename, strlen(scriptfilename));
		 nsp_setstr(conn->N, htobj, "PATH_TRANSLATED", scriptfilename, strlen(scriptfilename));
 //		nsp_setstr(conn->N, htobj, "SCRIPT_NAME", scriptname, strlen(scriptname));
		 nsp_setstr(conn->N, htobj, "PATH_INFO", scriptname, strlen(scriptname));
 //		if (strlen(ptemp)>0) {
 //			nsp_setstr(conn->N, htobj, "PATH_INFO", ptemp, strlen(ptemp));
 //		}
	 } else {
		 if (domainid>0) read_header_scriptinfo(conn, 0);
	 }
	 / * prints(conn, "[[%s]][%s][%s]<BR />", nsp_getstr(conn->N, htobj, "SCRIPT_NAME"), scriptname, scriptfilename); * /
	 return 0;
 }
 */

void send_error(CONN *conn, int status, char* title, char* text)
{
	obj_t *tobj = nsp_settable(conn->N, &conn->N->g, "_TEMP");
	char *p;

	nsp_setnum(conn->N, tobj, "status", status);
	nsp_setstr(conn->N, tobj, "title", title, strlen(title));
	nsp_setstr(conn->N, tobj, "theme", conn->dat->theme, strlen(conn->dat->theme));
	p = str2html(conn, text);
	nsp_setstr(conn->N, tobj, "text", p, strlen(p));
	//send_header(conn, 0, 200, "1", "text/html", -1, -1);
	send_header(conn, 0, status, "1", "text/html", -1, -1);
	if (htnsp_dotemplate(conn, "html", "error.ns") != 0) {
		prints(conn, "<!DOCTYPE html>\r\n");
		prints(conn, "<html>\r\n<head>\r\n");
		prints(conn, "<meta http-equiv=\"content-type\" content=\"text/html; charset=us-ascii\">\r\n");
		prints(conn, "<title>%d %s</title>\r\n</head>\r\n", status, title);
		prints(conn, "<body bgcolor=\"#f0f0f0\" text=\"#000000\" link=\"#0000ff\" alink=\"#0000ff\" vlink=\"#0000ff\">\r\n");
		prints(conn, "<h1>%d %s</h1>\r\n", status, title);
		prints(conn, "%s\r\n", p);
		prints(conn, "<hr>\r\n<address>%s %s</address>\r\n</body></html>\r\n", SERVER_NAME, PACKAGE_VERSION);
	}
	conn->dat->out_bodydone = 1;
	flushbuffer(conn);
	log_htaccess(conn);
	closeconnect(conn, 1);
	return;
}

void send_header(CONN *conn, int cacheable, int status, char *extra_header, char *mime_type, int length, time_t mod)
{
	obj_t *hrobj = nsp_getobj(conn->N, &conn->N->g, "_HEADER");
	char timebuf[100];
	time_t now;

	if (status) {
		conn->dat->out_status = status;
	}
	else if ((status = (int)nsp_getnum(conn->N, hrobj, "STATUS")) != 0) {
		conn->dat->out_status = status;
	}
	else {
		conn->dat->out_status = 200;
	}
	if (length >= 0) {
		conn->dat->out_ContentLength = length;
	}
	if (mod != (time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		nsp_setstr(conn->N, hrobj, "LAST_MODIFIED", timebuf, strlen(timebuf));
	}
	now = time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	nsp_setstr(conn->N, hrobj, "DATE", timebuf, strlen(timebuf));
	if (cacheable) {
		nsp_setstr(conn->N, hrobj, "CACHE_CONTROL", "public", -1);
		nsp_setstr(conn->N, hrobj, "PRAGMA", "public", -1);
	}
	else {
		nsp_setstr(conn->N, hrobj, "CACHE_CONTROL", "no-cache, no-store, must-revalidate", -1);
		nsp_setstr(conn->N, hrobj, "EXPIRES", timebuf, -1);
		nsp_setstr(conn->N, hrobj, "PRAGMA", "no-cache", -1);
	}
	if (extra_header != (char*)0) {
		nsp_setstr(conn->N, hrobj, "CONTENT_TYPE", mime_type, -1);
	}
	else {
		nsp_setstr(conn->N, hrobj, "CONTENT_TYPE", "text/html", -1);
	}
}

int test_add(CONN *conn, char *path, char *name, char *ext)
{
	obj_t *htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	struct stat sb;
	char file[512];
	char *p;

	snprintf(file, sizeof(file) - strlen(ext) - 1, "%s%s", path, name);
	if ((p = strchr(file, '?')) != NULL) *p = '\0';
	strcat(file, ext);
	//	fixslashes(file);
	if (file[0] != '\0') { p = file + strlen(file) - 1; if ((*p == '\\') || (*p == '/')) *p = '\0'; }
	if (stat(file, &sb) == 0) {
		//		nsp_setstr(conn->N, htobj, "SCRIPT_FILENAME", file, -1);
		nsp_setstr(conn->N, htobj, "PATH_TRANSLATED", file, -1);
		//		nsp_setstr(conn->N, htobj, "SCRIPT_NAME", file+strlen(path), -1);
		nsp_setstr(conn->N, htobj, "PATH_INFO", file + strlen(path), -1);
		//		nsp_setstr(conn->N, htobj, "REQUEST_URI", file+strlen(path), -1);
		return 1;
	}
	return 0;
}

void http_dorequest(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj, *hrobj, *tobj;
	//	obj_t *newconfobj;
	char buf[255];
	//char *RequestMethod;
	char *RequestURI;
	char *ptemp;
	int relocate = 0;
	int authenticated = 0;
	//	int rc;

	if (read_header(conn) < 0) {
		closeconnect(conn, 1);
		conn->N = nsp_endstate(conn->N);
		return;
	}
	tobj = nsp_getobj(conn->N, &conn->N->g, "io");
	nsp_setcfunc(conn->N, tobj, "flush", (void *)htnsp_flush);
	gettimeofday(&conn->dat->runtime, NULL);
	htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	hrobj = nsp_getobj(conn->N, &conn->N->g, "_HEADER");
	nsp_setstr(conn->N, hrobj, "CONTENT_TYPE", "text/html", -1);
	//RequestMethod=nsp_getstr(conn->N, htobj, "REQUEST_METHOD");
	RequestURI = nsp_getstr(conn->N, htobj, "REQUEST_URI");
	//	memset(buf, 0, sizeof(buf));
	//	snprintf(buf, sizeof(buf) - 1, "%s%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var_htdocs"), RequestURI);
	proc->stats.http_pages++;

	read_vardata(conn);
	authenticated = !auth_getcookie(conn);
	//	read_header_scriptinfo(conn, conn->dat->did);
	if (authenticated) {
		if (strncmp(RequestURI, "/logout", 7) == 0) {
			auth_logout(conn);
			goto wrap;
		}
	}
	if (!authenticated) {
		if (auth_setcookie(conn) == 0) {
			authenticated = 1;
			//relocate=1;
			db_log_activity(conn, "login", 0, "login", "%s - Login: username=%s", conn->socket.RemoteAddr, conn->dat->username);
		}
		else if ((getgetenv(conn, "USERNAME") != NULL) || (getpostenv(conn, "USERNAME") != NULL)) {
			db_log_activity(conn, "login", 0, "failed login", "%s - Login failed: username=%s", conn->socket.RemoteAddr, conn->dat->username);
		}
	}
	if (authenticated) {
		ptemp = getpostenv(conn, "PAGEURI");
		snprintf(buf, sizeof(buf) - 1, "%s", ptemp ? ptemp : RequestURI);
		if ((ptemp = p_strcasestr(buf, "username")) != NULL) { *ptemp = '\0'; relocate = 1; }
		if ((ptemp = p_strcasestr(buf, "password")) != NULL) { *ptemp = '\0'; relocate = 1; }
		if (buf[strlen(buf) - 1] == '&') buf[strlen(buf) - 1] = '\0';
		if (buf[strlen(buf) - 1] == '?') buf[strlen(buf) - 1] = '\0';
		if (relocate) {
			nsp_setstr(conn->N, hrobj, "LOCATION", buf, -1);
			send_header(conn, 0, 302, "1", "text/html", -1, -1);
			goto wrap;
		}
	}
	htnsp_runinit(conn);
	if (conn->N->err) {
		// conn->N->err=0;
		if (strcmp(conn->N->errbuf, "authentication required") == 0) {
			//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "http_dorequest: htnsp_runinit has an exception");
			send_error(conn, 403, "Forbidden", "authentication required");
			//htpage_login(conn);
			//prints(conn, "<B>[Exception: \"%s\"]</B>\r\n", conn->N->errbuf);
			//prints(conn, "%s", conn->N->errbuf);
			conn->N->err = 0;
			flushbuffer(conn);
			log_htaccess(conn);
			closeconnect(conn, 1);

			goto wrap;
		}
		else {
			snprintf(buf, sizeof(buf) - 1, "%s/domains/%04d%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did, RequestURI);
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "http_dorequest: [%s]", buf);
			if (filesend(conn, buf) == 0) goto wrap;
			snprintf(buf, sizeof(buf) - 1, "%s/htdocs%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), RequestURI);
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "http_dorequest: [%s]", buf);
			if (filesend(conn, buf) == 0) goto wrap;
			snprintf(buf, sizeof(buf) - 1, "%s/htdocs%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), RequestURI);
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "http_dorequest: [%s]", buf);
			if (filesend(conn, buf) == 0) goto wrap;
		}
	}

	//	newconfobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	//	snprintf(buf, sizeof(buf) - 1, "%s/domains/%04d/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	//	nsp_setstr(conn->N, newconfobj, "private_htdocs_path", buf, -1);
	//	snprintf(buf, sizeof(buf) - 1, "%s/domains/%04d/scripts", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	//	nsp_setstr(conn->N, newconfobj, "private_scripts_path", buf, -1);

		//	if (strcmp(RequestURI, "/")==0) {
		//		htpage_motd(conn);
		//		goto wrap;
		//	}
		//	if ((ext=strrchr(ScriptName, '.'))!=NULL) {
		//		if (strcmp(ext, ".ns")==0) {
		//			if (htnsp_doscript(conn)>-1) goto wrap;
		//		}
		//	}


	//	char buf[255];

	snprintf(buf, sizeof(buf) - 1, "%s/domains/%04d/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	//	cobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_CONFIG"), "private_htdocs_path");
	//	if (nsp_isstr(cobj)) {
	if (test_add(conn, buf, RequestURI, "") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".ns") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".nsp") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	//	}
		/* if it's not in the private htdocs, try the public */
	snprintf(buf, sizeof(buf) - 1, "%s/share/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"));
	//	cobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_CONFIG"), "shared_htdocs_path");
	//	if (nsp_isstr(cobj)) {
	if (test_add(conn, buf, RequestURI, "") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".ns") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".nsp") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	//	}
		/* else try the static shared htdocs */
	snprintf(buf, sizeof(buf) - 1, "%s/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"));
	if (test_add(conn, buf, RequestURI, "") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".ns") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}
	else if (test_add(conn, buf, RequestURI, ".nsp") == 1) {
		if (htnsp_doscript(conn) > -1) goto wrap;
	}



	RequestURI = nsp_getstr(conn->N, htobj, "REQUEST_URI");
	if (strncmp(RequestURI, "/logout", 7) == 0) {
		auth_logout(conn);
	}
	else if (module_menucall(conn)) {
		goto wrap;
	}
	else {
		snprintf(buf, sizeof(buf) - 1, "The requested URL '%s' was not found on this server.", RequestURI);
		send_error(conn, 404, "Not Found", buf);
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 2, "%s - Bad request '%s' by %s", conn->socket.RemoteAddr, RequestURI, conn->dat->username);
	}
wrap:
	if (conn->N->err) {
		prints(conn, "<HR><B>[errno=%d :: %s]</B>\r\n", conn->N->err, conn->N->errbuf);
		closeconnect(conn, 1);
	}
	auth_savesession(conn);
	conn->dat->out_bodydone = 1;
	flushbuffer(conn);
	log_htaccess(conn);
	if (conn->dat->wm != NULL) {
		tcp_close(&conn->dat->wm->socket, 1);
		free(conn->dat->wm);
		conn->dat->wm = NULL;
	}
	return;
}
