/*
    nsp.cgi -- simple Nesla CGI host
    Copyright (C) 2007-2018 Dan Cahill

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

static const char Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int printhex(const char *format, ...)
{
	char *hex = "0123456789ABCDEF";
	unsigned char buffer[1024];
	int offset = 0;
	va_list ap;

	va_start(ap, format);
	vsnprintf((char *)buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if ((buffer[offset] > 32) && (buffer[offset] < 128) && (buffer[offset] != '<') && (buffer[offset] != '>')) {
			printf("%c", buffer[offset]);
		}
		else {
			printf("%%%c%c", hex[(unsigned int)buffer[offset] / 16], hex[(unsigned int)buffer[offset] & 15]);
		}
		offset++;
	}
	return 0;
}

int printht(const char *format, ...)
{
	unsigned char buffer[1024];
	int offset = 0;
	va_list ap;

	va_start(ap, format);
	vsnprintf((char *)buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if (buffer[offset] == '<') {
			printf("&lt;");
			/*
					} else if (buffer[offset]=='>') {
						printf("&gt;");
			*/
		}
		else if (buffer[offset] == '&') {
			printf("&amp;");
		}
		else {
			printf("%c", buffer[offset]);
		}
		offset++;
	}
	return 0;
}

char *b64decode(char *src)
{
	static char dest[1024];
	int destidx, state, ch;
	int szdest;
	char *pos;

	memset(dest, 0, sizeof(dest));
	szdest = sizeof(dest) - 1;
	state = 0;
	destidx = 0;
	while ((ch = *src++) != '\0') {
		if (isspace(ch)) continue;
		if (ch == '=') break;
		pos = strchr(Base64, ch);
		if (pos == 0) return "";
		switch (state) {
		case 0:
			//if (dest) {
			if (destidx >= szdest) return dest;
			dest[destidx] = (pos - Base64) << 2;
			//}
			state = 1;
			break;
		case 1:
			//if (dest) {
			if (destidx + 1 >= szdest) return dest;
			dest[destidx] |= (pos - Base64) >> 4;
			dest[destidx + 1] = ((pos - Base64) & 0x0f) << 4;
			//}
			destidx++;
			state = 2;
			break;
		case 2:
			//if (dest) {
			if (destidx + 1 >= szdest) return dest;
			dest[destidx] |= (pos - Base64) >> 2;
			dest[destidx + 1] = ((pos - Base64) & 0x03) << 6;
			//}
			destidx++;
			state = 3;
			break;
		case 3:
			//if (dest) {
			if (destidx >= szdest) return dest;
			dest[destidx] |= (pos - Base64);
			//}
			destidx++;
			state = 0;
			break;
		}
	}
	return dest;
}

char *b64encode(char *src)
{
	unsigned char a, b, c, d, *cp;
	static char dest[1024];
	int dst, i, enclen, remlen;
	int srclen = strlen(src);

	memset(dest, 0, sizeof(dest));
	cp = (unsigned char *)src;
	dst = 0;
	enclen = srclen / 3;
	remlen = srclen - 3 * enclen;
	for (i = 0;i < enclen;i++) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		c |= (cp[2] >> 6);
		d = cp[2] & 0x3f;
		cp += 3;
		dest[dst + 0] = Base64[a];
		dest[dst + 1] = Base64[b];
		dest[dst + 2] = Base64[c];
		dest[dst + 3] = Base64[d];
		dst += 4;
		if (dst >= 1020) return dest;
	}
	if (remlen == 1) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		dest[dst + 0] = Base64[a];
		dest[dst + 1] = Base64[b];
		dest[dst + 2] = '=';
		dest[dst + 3] = '=';
	}
	else if (remlen == 2) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		dest[dst + 0] = Base64[a];
		dest[dst + 1] = Base64[b];
		dest[dst + 2] = Base64[c];
		dest[dst + 3] = '=';
	}
	return dest;
}

int IntFromHex(char *pChars)
{
	int Hi;
	int Lo;
	int Result;

	Hi = pChars[0];
	if ('0' <= Hi&&Hi <= '9') {
		Hi -= '0';
	}
	else if ('a' <= Hi&&Hi <= 'f') {
		Hi -= ('a' - 10);
	}
	else if ('A' <= Hi&&Hi <= 'F') {
		Hi -= ('A' - 10);
	}
	Lo = pChars[1];
	if ('0' <= Lo&&Lo <= '9') {
		Lo -= '0';
	}
	else if ('a' <= Lo&&Lo <= 'f') {
		Lo -= ('a' - 10);
	}
	else if ('A' <= Lo&&Lo <= 'F') {
		Lo -= ('A' - 10);
	}
	Result = Lo + (16 * Hi);
	return (Result);
}

void URLDecode(char *pEncoded)
{
	char *pDecoded;

	pDecoded = pEncoded;
	while (*pDecoded) {
		if (*pDecoded == '+') *pDecoded = ' ';
		pDecoded++;
	};
	pDecoded = pEncoded;
	while (*pEncoded) {
		if (*pEncoded == '%') {
			pEncoded++;
			if (isxdigit(pEncoded[0]) && isxdigit(pEncoded[1])) {
				*pDecoded++ = (char)IntFromHex(pEncoded);
				pEncoded += 2;
			}
		}
		else {
			*pDecoded++ = *pEncoded++;
		}
	}
	*pDecoded = '\0';
}

char *str2html(char *instring)
{
	static unsigned char buffer[8192];
	unsigned char ch;
	int blen = 0;
	int i = 0;

	memset(buffer, 0, sizeof(buffer));
	while ((instring[i]) && (i < sizeof(buffer) - 1)) {
		ch = instring[i];
		if (ch == 0) break;
		if ((ch != 10) && (ch != 13)) {
			if (ch < 32) { i++; continue; }
		}
		if (ch == '\"') {
			buffer[blen++] = '&';
			buffer[blen++] = 'q';
			buffer[blen++] = 'u';
			buffer[blen++] = 'o';
			buffer[blen++] = 't';
			buffer[blen++] = ';';
			i++;
			continue;
		}
		if (ch == '&') {
			buffer[blen++] = '&';
			buffer[blen++] = 'a';
			buffer[blen++] = 'm';
			buffer[blen++] = 'p';
			buffer[blen++] = ';';
			i++;
			continue;
		}
		if (ch == '<') {
			buffer[blen++] = '&';
			buffer[blen++] = 'l';
			buffer[blen++] = 't';
			buffer[blen++] = ';';
			i++;
			continue;
		}
		if (ch == '>') {
			buffer[blen++] = '&';
			buffer[blen++] = 'g';
			buffer[blen++] = 't';
			buffer[blen++] = ';';
			i++;
			continue;
		}
		buffer[blen++] = ch;
		i++;
	}
	return (char *)buffer;
}

char *get_mime_type(char *name)
{
	char *mime_types[40][2] = {
		{ ".html", "text/html" },
		{ ".htm",  "text/html" },
		{ ".shtml","text/html" },
		{ ".css",  "text/css" },
		{ ".txt",  "text/plain" },
		{ ".ns",   "text/plain" },
		{ ".php",  "text/plain" },
		{ ".mdb",  "application/msaccess" },
		{ ".xls",  "application/msexcel" },
		{ ".doc",  "application/msword" },
		{ ".exe",  "application/octet-stream" },
		{ ".pdf",  "application/pdf" },
		{ ".rtf",  "application/rtf" },
		{ ".tgz",  "application/x-compressed" },
		{ ".gz",   "application/x-compressed" },
		{ ".z",    "application/x-compress" },
		{ ".swf",  "application/x-shockwave-flash" },
		{ ".tar",  "application/x-tar" },
		{ ".rar",  "application/x-rar-compressed" },
		{ ".zip",  "application/x-zip-compressed" },
		{ ".ra",   "audio/x-pn-realaudio" },
		{ ".ram",  "audio/x-pn-realaudio" },
		{ ".wav",  "audio/x-wav" },
		{ ".gif",  "image/gif" },
		{ ".jpeg", "image/jpeg" },
		{ ".jpe",  "image/jpeg" },
		{ ".jpg",  "image/jpeg" },
		{ ".png",  "image/png" },
		{ ".avi",  "video/avi" },
		{ ".mp3",  "video/mpeg" },
		{ ".mpeg", "video/mpeg" },
		{ ".mpg",  "video/mpeg" },
		{ ".qt",   "video/quicktime" },
		{ ".mov",  "video/quicktime" },
		{ "",      "" }
	};
	char *extension;
	int i;

	extension = strrchr(name, '.');
	if (extension == NULL) {
		return "text/plain";
	}
	i = 0;
	while (strlen(mime_types[i][0]) > 0) {
		if (strcasecmp(extension, mime_types[i][0]) == 0) {
			return mime_types[i][1];
		}
		i++;
	}
	return "application/octet-stream";
}

char *strcasestr(const char *src, const char *query)
{
	char *pToken;
	char Buffer[8192];
	char Query[64];
	int loop;

	if (strlen(src) == 0) return NULL;
	memset(Buffer, 0, sizeof(Buffer));
	strncpy(Buffer, src, sizeof(Buffer) - 1);
	strncpy(Query, query, sizeof(Query) - 1);
	loop = 0;
	while (Buffer[loop]) {
		Buffer[loop] = toupper(Buffer[loop]);
		loop++;
	}
	loop = 0;
	while (Query[loop]) {
		Query[loop] = toupper(Query[loop]);
		loop++;
	}
	pToken = strstr(Buffer, Query);
	if (pToken != NULL) {
		return (char *)src + (pToken - (char *)&Buffer);
	}
	return NULL;
}

void cgi_readenv()
{
	obj_t *confobj = nsp_settable(N, &N->g, "_CONFIG");
	obj_t *servobj = nsp_settable(N, &N->g, "_SERVER");
	obj_t *cookieobj = nsp_settable(N, &N->g, "_COOKIE");
	obj_t *gobj = nsp_settable(N, &N->g, "_GET");
	obj_t *pobj = nsp_settable(N, &N->g, "_POST");
	obj_t *cobj;
	char boundary[100];
	int blen;
	int clen;
	char tmpbuf[MAX_OBJNAMELEN];
	char tmpbuf2[MAX_OBJNAMELEN + 5];
	char filename[255];
	char *QueryString = nsp_getstr(N, servobj, "QUERY_STRING");
	char *p1, *p2;
	unsigned int len;
	int i;
	int x;

	nsp_setstr(N, servobj, "NSP_VERSION", NSP_VERSION, -1);
	/* strip trailing slashes for thttpd */
	strncpy(boundary, nsp_getstr(N, servobj, "SCRIPT_NAME"), sizeof(boundary) - 1);
	while ((boundary[strlen(boundary) - 1] == '/')) {
		boundary[strlen(boundary) - 1] = '\0';
		nsp_setstr(N, servobj, "SCRIPT_NAME", boundary, -1);
	}
	cobj = nsp_getobj(N, servobj, "REQUEST_URI");
	if (cobj->val->type == NT_STRING) {
		p1 = nsp_tostr(N, cobj);
		len = strlen(nsp_getstr(N, servobj, "SCRIPT_NAME"));
		if (len < strlen(p1)) {
			nsp_setstr(N, servobj, "CGI_URI", p1 + len, strlen(p1) - len);
		}
		else {
			nsp_setstr(N, servobj, "CGI_URI", "/", 1);
		}
	}
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
		cobj = nsp_setstr(N, gobj, tmpbuf, p1, len);
		if (cobj->val->d.str) {
			URLDecode(cobj->val->d.str); cobj->val->size = strlen(cobj->val->d.str);
		}
		p1 += i;
	}
	if (strcmp(nsp_getstr(N, servobj, "REQUEST_METHOD"), "POST") == 0) {
		clen = (int)nsp_getnum(N, servobj, "CONTENT_LENGTH");
		if (clen > nsp_getnum(N, confobj, "max_postsize")) {
			/* try to print an error : note the inbuffer is still
			 * full, so the cgi will probably just puke, and die.
			 * But at least it'll do it quickly. ;-)
			 */
			send_header(0, "text/html", -1, -1);
			printf("POST data is too large");
			exit(0);
		}
		cobj = nsp_getobj(N, &N->g, "POSTRAWDATA");
		if (cobj->val->type == NT_NULL) {
			i = 0;
			if ((p1 = calloc(clen + 64, sizeof(char))) == NULL) {
				send_header(0, "text/html", -1, -1);
				printf("malloc error");
				exit(0);
			}
			/* fgets doesn't exactly tell us if the input is complete... */
			/* fgets(p1, clen+1, stdin); */
			while (i < clen) {
				x = fgetc(stdin);
				if (x == EOF) {
					send_header(0, "text/html", -1, -1);
					printf("premature end to POST data. %d bytes read", i);
					exit(0);
					//					break;
				}
				p1[i++] = x;
			}
			cobj = nsp_setstr(N, &N->g, "POSTRAWDATA", NULL, 0);
			cobj->val->attr |= NST_HIDDEN;
			cobj->val->size = i;
			cobj->val->d.str = p1;
		}
		/* CONTENT_TYPE = "multipart/form-data; boundary=---------------------------7d723815a700f2" */
		p1 = nsp_getstr(N, servobj, "CONTENT_TYPE");
		if (strncmp(p1, "multipart/form-data", strlen("multipart/form-data")) == 0) {
			if ((p1 = strstr(p1, "boundary")) == NULL) {
				/* error */
				return;
			}
			p1 += 8;
			if (*p1 != '=') {
				/* error */
				return;
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
			cobj = nsp_setstr(N, &N->g, "POSTBOUNDARY", boundary, blen);
			cobj->val->attr |= NST_HIDDEN;
			p1 = nsp_getstr(N, &N->g, "POSTRAWDATA");
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
				cobj = nsp_setstr(N, pobj, tmpbuf, NULL, 0);
				if (i) {
					cobj->val->d.str = malloc(i + 1);
					if (cobj->val->d.str) {
						memcpy(cobj->val->d.str, p2, i);
						cobj->val->d.str[i] = '\0';
					}
				}
				cobj->val->size = cobj->val->d.str ? i : 0;
				/* decodeurl(sid->dat->smallbuf[1]); */
				if (filename[0] != '\0') {
					cobj->val->attr |= NST_HIDDEN; /* hide attached files from var dumps */
					snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_NAME", tmpbuf);
					nsp_setstr(N, pobj, tmpbuf2, filename, -1);
					snprintf(tmpbuf2, sizeof(tmpbuf2) - 1, "%s_SIZE", tmpbuf);
					nsp_setnum(N, pobj, tmpbuf2, i);
				}
				p2 += i;
				/* end part reading */
			} while (1);
		}
		else {
			p1 = nsp_getstr(N, &N->g, "POSTRAWDATA");
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
				cobj = nsp_setstr(N, pobj, tmpbuf, p1, len);
				if (cobj->val->d.str) {
					URLDecode(cobj->val->d.str); cobj->val->size = strlen(cobj->val->d.str);
				}
				p1 += i;
			}
		}
	}
	p1 = nsp_getstr(N, servobj, "HTTP_COOKIE");
	if (p1) {
		for (;;) {
			if (strlen(p1) == 0) break;
			for (i = 0;i < MAX_OBJNAMELEN;i++) {
				if (*p1 == '\0') { tmpbuf[i] = '\0'; break; }
				if (*p1 == '=') { tmpbuf[i] = '\0'; p1++; break; }
				tmpbuf[i] = toupper(*p1);
				p1++;
			}
			p2 = strchr(p1, ';');
			if (p2 == NULL) p2 = p1 + strlen(p1);
			nsp_setstr(N, cookieobj, tmpbuf, p1, p2 - p1);
			if (!strlen(p2)) break;
			p2++;
			while (isspace(*p2)) p2++;
			p1 = p2;
		}
	}
	return;
}

char *getgetenv(char *query)
{
	obj_t *cobj = nsp_getobj(N, nsp_settable(N, &N->g, "_GET"), query);

	if (cobj->val->type == NT_NULL) return NULL;
	return nsp_tostr(N, cobj);
}

char *getpostenv(char *query)
{
	obj_t *cobj = nsp_getobj(N, nsp_settable(N, &N->g, "_POST"), query);

	if (cobj->val->type == NT_NULL) return NULL;
	return nsp_tostr(N, cobj);
}

char *fileul(char *xfilename, char *xfilesize)
{
	obj_t *pobj = nsp_settable(N, &N->g, "_POST");
	obj_t *cobj = nsp_getobj(N, pobj, "ATTACHMENT1");

	if (cobj->val->type == NT_STRING) {
		snprintf(xfilename, 1024, "%s", nsp_getstr(N, pobj, "ATTACHMENT1_NAME"));
		snprintf(xfilesize, 9, "%u", (unsigned int)cobj->val->size);
		if ((xfilename[0] == '\0') || (cobj->val->size == 0)) return NULL;
		return cobj->val->d.str;
	}
	xfilename[0] = '\0';
	xfilesize[0] = '\0';
	return NULL;
}

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

void send_header(int cacheable, char *mime_type, int length, time_t mod)
{
	obj_t *servobj = nsp_settable(N, &N->g, "_SERVER");
	obj_t *headobj = nsp_settable(N, &N->g, "_HEADER");
	obj_t *ctobj = NULL;
	obj_t *cobj;
	char tmpbuf[65];
	char timebuf[100];
	char *p;
	static short sent = 0;
	time_t now;

	if (sent) return;
	if (mod != (time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		nsp_setstr(N, headobj, "LAST_MODIFIED", timebuf, -1);
	}
	now = time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	nsp_setstr(N, headobj, "DATE", timebuf, -1);
	if (cacheable) {
		cobj = nsp_getobj(N, headobj, "CACHE_CONTROL");
		if (cobj->val->type == NT_NULL) nsp_setstr(N, headobj, "CACHE_CONTROL", "public", -1);
		cobj = nsp_getobj(N, headobj, "PRAGMA");
		if (cobj->val->type == NT_NULL) nsp_setstr(N, headobj, "PRAGMA", "public", -1);
	}
	else {
		cobj = nsp_getobj(N, headobj, "CACHE_CONTROL");
		if (cobj->val->type == NT_NULL) nsp_setstr(N, headobj, "CACHE_CONTROL", "no-cache, no-store, must-revalidate", -1);
		cobj = nsp_getobj(N, headobj, "EXPIRES");
		if (cobj->val->type == NT_NULL) nsp_setstr(N, headobj, "EXPIRES", timebuf, -1);
		cobj = nsp_getobj(N, headobj, "PRAGMA");
		if (cobj->val->type == NT_NULL) nsp_setstr(N, headobj, "PRAGMA", "no-cache", -1);
	}
	/* IIS bursts into flames when we do a redirect, so use nph- */
	if (strstr(nsp_getstr(N, servobj, "SCRIPT_NAME"), "nph-") != NULL) {
		printf("HTTP/1.0 200 OK\r\n");
	}
	for (cobj = headobj->val->d.table.f; cobj; cobj = cobj->next) {
		if ((cobj->val->type != NT_STRING) && (cobj->val->type != NT_NUMBER)) continue;
		if (strcmp(cobj->name, "CONTENT_TYPE") == 0) { ctobj = cobj; continue; }
		strncpy(tmpbuf, cobj->name, sizeof(tmpbuf) - 1);
		for (p = tmpbuf;p[0];p++) {
			if (p[0] == '_') {
				p[0] = '-';
				if (isalpha(p[1])) { p++; continue; }
			}
			else {
				*p = tolower(*p);
			}
		}
		tmpbuf[0] = toupper(tmpbuf[0]);
		printf("%s: %s\r\n", tmpbuf, nsp_tostr(N, cobj));
	}
	if (ctobj != NULL) {
		printf("Content-Type: %s\r\n", nsp_tostr(N, ctobj));
	}
	else if (mime_type != NULL) {
		printf("Content-Type: %s\r\n", mime_type);
	}
	else {
		printf("Content-Type: text/html\r\n");
	}
	printf("\r\n");
	sent = 1;
	return;
}
