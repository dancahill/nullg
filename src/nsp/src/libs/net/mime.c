/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#include "nsp/nsplib.h"
#include "net.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#define snprintf _snprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int hex2int(char *p)
{
	int h = tolower(p[0]);
	int l = tolower(p[1]);

	if ('0' <= h && h <= '9') h -= '0'; else if ('a' <= h && h <= 'f') h -= ('a' - 10);
	if ('0' <= l && l <= '9') l -= '0'; else if ('a' <= l && l <= 'f') l -= ('a' - 10);
	return 16 * h + l;
}

static time_t time_wmgetdate(char *src)
{
	char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	int dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	time_t unixdate = -1;
	int year, month, day, hours, minutes, seconds;
	int i, tzoffset;
	char *p = src;

	while ((*p) && (!isdigit(*p))) p++;
	if (!isdigit(*p)) return 0;
	if (*p == '0') p++;
	day = atoi(p);
	while ((*p) && (!isalpha(*p))) p++;
	month = -1;
	for (i = 0; i < 12; i++) {
		if (strncasecmp(p, months[i], 3) == 0) month = i;
	}
	if (month == -1) return 0;
	while ((*p) && (!isdigit(*p))) p++;
	year = atoi(p);
	for (i = 1970; i < year; i++) {
		unixdate += 365;
		if ((i / 4.0f) == (int)(i / 4)) {
			if ((i / 400.0f) == (int)(i / 400)) {
				unixdate++;
			} else if ((i / 100.0f) != (int)(i / 100)) {
				unixdate++;
			}
		}
	}
	for (i = 0; i < month; i++) {
		unixdate += dim[i];
		if (i != 1) continue;
		if ((year / 4.0f) == (int)(year / 4)) {
			if ((year / 400.0f) == (int)(year / 400)) {
				unixdate++;
			} else if ((year / 100.0f) != (int)(year / 100)) {
				unixdate++;
			}
		}
	}
	unixdate += day;
	if (unixdate < 1) return 0;
	unixdate *= 86400;
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	hours = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	minutes = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	seconds = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p)) && (*p != '-')) p++;
	tzoffset = atoi(p);
	unixdate += hours * 3600;
	unixdate += minutes * 60;
	unixdate += seconds;
	unixdate -= (int)(tzoffset / 100) * 3600;
	return unixdate;
}

static void parseval(nsp_state *N, obj_t *cobj, obj_t *tobj, char *name)
{
	char *p1, *p2;

	//cobj = nsp_getobj(N, ctobj, "content-type");
	//parseval(N, cobj, cvobj, "boundary");

	if (!nsp_isstr(cobj) || (cobj->val->size == 0)) return;
	/* if ((p1=strcasestr(cobj->val->d.str, "boundary="))!=NULL) { */
	if ((p1 = strstr(cobj->val->d.str, name)) != NULL) {
		p1 += strlen(name);
		if (*p1 != '=') return;
		p1++;
		if (*p1 == ' ') p1++;
		if (*p1 == '\"') {
			p1++;
			p2 = p1;
			while (*p2 && *p2 != '\"') p2++;
		} else {
			p2 = p1;
			while (*p2 && *p2 != ';' && !isspace(*p2)) p2++;
		}
		nsp_setstr(N, tobj, name, p1, p2 - p1);
	}
}

static char *mime_read_head(nsp_state *N, obj_t *tobj, char *inptr)
{
	char namebuf[MAX_OBJNAMELEN + 1];
	obj_t *cobj = NULL;
	obj_t *iobj;
	obj_t *ctobj = NULL;
	obj_t *cvobj = NULL;
	char *p;
	char *p1;
	unsigned int p1s;
	int size;

	ctobj = nsp_settable(N, tobj, "head");
	cvobj = nsp_settable(N, tobj, "headvalues");
	if (inptr == NULL) return NULL;
	while (1) {
		p = namebuf;
		while (*inptr && (isalnum(*inptr) || *inptr == '-' || *inptr == '_')) *p++ = tolower(*inptr++);
		*p = '\0';
		if (!*inptr) break;
		if (*inptr == ':') {
			inptr++;
			if (*inptr == ' ') inptr++;
			p = inptr;
			while (*inptr && *inptr != '\r' && *inptr != '\n') inptr++;
			cobj = nsp_getobj(N, ctobj, namebuf);
			if (nsp_isstr(cobj)) {
				p1s = cobj->val->size;
				p1 = cobj->val->d.str;
				cobj->val->size = 0;
				cobj->val->d.str = NULL;
				iobj = nsp_settable(N, cobj, namebuf);
				cobj = nsp_setstr(N, iobj, "0", NULL, 0);
				cobj->val->size = p1s;
				cobj->val->d.str = p1;
				cobj = nsp_setstr(N, iobj, n_ntoa(N, namebuf, 1, 10, 0), p, inptr - p);
			} else if (nsp_istable(cobj)) {
				size = 0;
				for (iobj = cobj->val->d.table.f; iobj; iobj = iobj->next) {
					if (iobj->val->attr & NST_SYSTEM) { size--; continue; }
					if (!nsp_isnull(iobj)) size++;
				}
				cobj = nsp_setstr(N, cobj, n_ntoa(N, namebuf, size, 10, 0), p, inptr - p);
			} else {
				cobj = nsp_setstr(N, ctobj, namebuf, p, inptr - p);
			}
			if (*inptr == '\r') inptr++;
			if (*inptr == '\n') inptr++;
		} else if (*inptr == ' ' || *inptr == '\t') {
			while (*inptr == ' ' || *inptr == '\t') inptr++;
			p = inptr;
			while (*inptr && *inptr != '\r' && *inptr != '\n') inptr++;
			if (cobj) nsp_strcat(N, cobj, " ", 1);
			if (cobj) nsp_strcat(N, cobj, p, (unsigned long)(inptr - p));
			if (*inptr == '\r') inptr++;
			if (*inptr == '\n') inptr++;
		} else if (*inptr != '\r' && *inptr != '\n' && *inptr != '\0') {
			//char *x = inptr;
			while (*inptr != '\r' && *inptr != '\n' && *inptr != '\0') inptr++;
			if (*inptr == '\r') inptr++;
			if (*inptr == '\n') inptr++;
			//nsp_setstr(N, &N->g, "mime_bug", x, inptr - x);
			continue;
		} else {
			break;
		}
		if (nc_strcmp(namebuf, "date") == 0) {
			nsp_setnum(N, cvobj, "date_numeric", (num_t)time_wmgetdate(nsp_tostr(N, cobj)));
		}
	}
	if (*inptr == '\r') inptr++;
	if (*inptr == '\n') inptr++;
	parseval(N, nsp_getobj(N, ctobj, "content-type"), cvobj, "boundary");
	parseval(N, nsp_getobj(N, ctobj, "content-type"), cvobj, "name");
	parseval(N, nsp_getobj(N, ctobj, "content-disposition"), cvobj, "filename");



/*
	if (nsp_isstr(cobj) && (cobj->val->size > 0)) {
		if ((p1 = strstr(cobj->val->d.str, "boundary=")) != NULL) {
			p1 += 9;
			if (*p1 == ' ') p1++;
			if (*p1 == '\"') {
				p1++;
				p2 = p1;
				while (*p2&&*p2 != '\"') p2++;
			}
			else {
				p2 = p1;
				while (*p2&&*p2 != ';' && !isspace(*p2)) p2++;
			}
			nsp_setstr(N, cvobj, "boundary", p1, p2 - p1);
		}
	}
*/
	return inptr;
}

static char *mime_read_body(nsp_state *N, obj_t *tobj, char *inptr, char *boundary)
{
#define __FN__ __FILE__ ":mime_read_body()"
	char name[8];
	char *p;
	obj_t *bobj, *cobj, *ctobj;
	obj_t *iobj;
	short b, blen, i;

	if (boundary == NULL) {
		p = inptr;
		while (*inptr) inptr++;
		nsp_setstr(N, tobj, "body", p, inptr - p);
		return inptr;
	}
	ctobj = nsp_settable(N, tobj, "body");
	blen = (short)strlen(boundary);
	b = 1;
	while (*inptr) {
		if (inptr[0] == '-' && inptr[1] == '-' && strncmp(inptr + 2, boundary, blen) == 0) {
			inptr += blen + 2;
			if (inptr[0] == '-' && inptr[1] == '-') {
				inptr += 2;
				while (*inptr == '\r' || *inptr == '\n') inptr++;
				return inptr;
			}
			if (*inptr == '\r') inptr++;
			if (*inptr == '\n') inptr++;
			break;
		} else inptr++;
	}
	for (i = 0;; i++) {
		if (!*inptr) break;
		cobj = nsp_settable(NULL, ctobj, n_ntoa(N, name, i, 10, 0));
		cobj->val->attr &= ~NST_AUTOSORT;
		inptr = mime_read_head(N, cobj, inptr);
		bobj = nsp_getobj(N, nsp_getobj(N, cobj, "headvalues"), "boundary");
		b = nsp_isstr(bobj) && (bobj->val->size > 0) ? 1 : 0;
		if (b) inptr = mime_read_body(N, cobj, inptr, bobj->val->d.str);
		p = inptr;
		while (*inptr) {
			if (inptr[0] == '-' && inptr[1] == '-' && strncmp(inptr + 2, boundary, blen) == 0) {
				/* n_warn(N, __FN__, "bobj=%s, bound=%s, %s", nsp_tostr(N, bobj), boundary, inptr); */
				if (!b) {
					iobj = nsp_setstr(N, cobj, "body", p, inptr - p);
					/* delete the \r\n pair that comes just before the boundary */
					if (iobj->val->size > 0 && iobj->val->d.str[iobj->val->size - 1] == '\n') {
						iobj->val->d.str[--iobj->val->size] = '\0';
						if (iobj->val->size > 0 && iobj->val->d.str[iobj->val->size - 1] == '\r') {
							iobj->val->d.str[--iobj->val->size] = '\0';
						}
					}
				}
				inptr += blen + 2;
				if (inptr[0] == '-' && inptr[1] == '-') {
					inptr += 2;
					while (*inptr == '\r' || *inptr == '\n') inptr++;
					return inptr;
				}
				if (*inptr == '\r') inptr++;
				if (*inptr == '\n') inptr++;
				break;
			} else inptr++;
		}
	}
	return inptr;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_read)
{
#define __FN__ __FILE__ ":libnsp_net_mime_read()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj;
	obj_t tobj;
	char *inptr = NULL;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	/* tobj.val->attr|=NST_AUTOSORT; */
	tobj.val->attr &= ~NST_AUTOSORT;
	if ((inptr = cobj1->val->d.str) != NULL) {
		inptr = mime_read_head(N, &tobj, inptr);
		cobj = nsp_getobj(N, nsp_getobj(N, &tobj, "headvalues"), "boundary");
		if (nsp_isstr(cobj) && (cobj->val->size > 0)) {
			inptr = mime_read_body(N, &tobj, inptr, cobj->val->d.str);
		} else {
			inptr = mime_read_body(N, &tobj, inptr, NULL);
		}
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_write)
{
#define __FN__ __FILE__ ":libnsp_net_mime_write()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	if (cobj1->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_base64_decode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_base64_decode()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;
	char *src;
	char *dest;
	int destidx = 0;
	int state = 0;
	int ch;
	int x;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->size < 1) return 0;
	/* should actually be about 3/4 the size of cobj1->val->size */
	if ((robj->val->d.str = n_alloc(N, cobj1->val->size + 1, 0)) == NULL) return 0;
	src = cobj1->val->d.str;
	dest = robj->val->d.str;
	while ((ch = *src++) != '\0') {
		if (nc_isspace(ch)) continue;
		if (ch == '=') break;
		if (nc_isupper(ch)) {
			x = ch - 'A';
		} else if (nc_islower(ch)) {
			x = ch - 'a' + 26;
		} else if (nc_isdigit(ch)) {
			x = ch - '0' + 52;
		} else if (ch == '+') {
			x = 62;
		} else if (ch == '/') {
			x = 63;
		} else {
			return 0;
		}
		switch (state) {
		case 0:
			dest[destidx] = x << 2;
			state = 1;
			break;
		case 1:
			dest[destidx] |= x >> 4;
			dest[destidx + 1] = (x & 0x0f) << 4;
			destidx++;
			state = 2;
			break;
		case 2:
			dest[destidx] |= x >> 2;
			dest[destidx + 1] = (x & 0x03) << 6;
			destidx++;
			state = 3;
			break;
		case 3:
			dest[destidx] |= x;
			destidx++;
			state = 0;
			break;
		}
	}
	dest[destidx] = '\0';
	robj->val->size = destidx;
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_base64_encode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_base64_encode()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *robj;
	char *dest;
	uchar a, b, c, d, *cp;
	int dst, i, enclen, remlen, linelen, maxline = 0;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	cp = (uchar *)cobj1->val->d.str;
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->size < 1) return 0;
	dst = 0;
	linelen = 0;
	if (cobj2->val->type == NT_NUMBER) maxline = (int)cobj2->val->d.num;
	enclen = cobj1->val->size / 3;
	remlen = cobj1->val->size - 3 * enclen;
	/* should actually be about 4/3 the size of cobj1->val->size */
	i = maxline ? (((cobj1->val->size + 3) / 3 * 4) / maxline) : 5;
	i = i * 2 + 5;
	if ((dest = n_alloc(N, enclen * 4 + i, 0)) == NULL) return 0;
	robj->val->d.str = dest;
	for (i = 0; i < enclen; i++) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		c |= (cp[2] >> 6);
		d = cp[2] & 0x3f;
		cp += 3;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = b64chars[c];
		dest[dst + 3] = b64chars[d];
		dst += 4;
		linelen += 4;
		if ((maxline > 0) && (linelen >= maxline)) {
			dest[dst + 0] = '\r';
			dest[dst + 1] = '\n';
			dst += 2;
			linelen = 0;
		}
	}
	if (remlen == 1) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = '=';
		dest[dst + 3] = '=';
		dst += 4;
	} else if (remlen == 2) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = b64chars[c];
		dest[dst + 3] = '=';
		dst += 4;
	}
	dest[dst] = '\0';
	robj->val->size = dst;
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_qp_decode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_qp_decode()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj;
	obj_t tobj;
	char *s, *d;
	int len = 0;

	n_expect_argtype(N, NT_STRING, 1, cobj1, 1);
	//if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	cobj = nsp_setstr(N, &tobj, "", cobj1->val->d.str, cobj1->val->size);
	if (nsp_isstr(cobj) && (cobj->val->size > 0)) {
		d = s = cobj->val->d.str;
		while (*s) {
			if (*s != '=') {
				*d++ = *s++;
				len++;
			} else {
				s++;
				if (isxdigit(s[0]) && isxdigit(s[1])) {
					*d++ = (uchar)hex2int(s);
					s += 2;
					len++;
				} else if (s[0] == '\n') {
					s += 1;
				} else if (s[0] == '\r' && s[1] == '\n') {
					s += 2;
				}
			}
		}
		cobj->val->size = len;
		*d = '\0';
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_qp_encode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_qp_encode()"
	/* RFC 2045 - 6.7.  Quoted-Printable Content-Transfer-Encoding
	(2)   (Literal representation) Octets with decimal values of
	33 through 60 inclusive, and 62 through 126, inclusive,
	MAY be represented as the US-ASCII characters which
	correspond to those octets (EXCLAMATION POINT through
	LESS THAN, and GREATER THAN through TILDE,
	respectively).
	*/
	const char *hex = "0123456789ABCDEF";
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;
	uchar *cp;
	char *dest;
	unsigned int i;
	int di;
	int outlen = 0;
	int linelen = 0;
	int maxlinelen = 76;

	n_expect_argtype(N, NT_STRING, 1, cobj1, 1);
	if (cobj1->val->size == 0) {
		nsp_setstr(N, &N->r, "", NULL, 0);
		return 0;
	}
	cp = (uchar *)cobj1->val->d.str;
	// calculate output size
	for (i = 0; i < cobj1->val->size; i++) {
	breakline1:
		if (linelen >= maxlinelen) {
			outlen += 3;
			linelen = 0;
		}
		if (cp[i] != ' ' && (cp[i] < 33 || cp[i] == 61 || cp[i] > 127)) {
			linelen += 3;
			if (linelen >= maxlinelen) goto breakline1;
			outlen += 3;
		} else {
			linelen += 1;
			if (linelen >= maxlinelen) goto breakline1;
			outlen += 1;
		}
	}
	if ((dest = n_alloc(N, outlen + 1, 0)) == NULL) return 0;
	dest[outlen] = '\0';
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	robj->val->size = outlen;
	robj->val->d.str = dest;
	di = 0;
	linelen = 0;
	// create output string
	for (i = 0; i < cobj1->val->size; i++) {
	breakline2:
		if (linelen >= maxlinelen) {
			dest[di++] = '=';
			dest[di++] = '\r';
			dest[di++] = '\n';
			linelen = 0;
		}
		if (cp[i] != ' ' && (cp[i] < 33 || cp[i] == 61 || cp[i] > 127)) {
			linelen += 3;
			if (linelen >= maxlinelen) goto breakline2;
			dest[di++] = '=';
			dest[di++] = hex[(unsigned int)cp[i] / 16];
			dest[di++] = hex[(unsigned int)cp[i] & 15];
		} else {
			linelen += 1;
			if (linelen >= maxlinelen) goto breakline2;
			dest[di++] = cp[i];
		}
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_rfc2047_decode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_rfc2047_decode()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj;
	obj_t tobj;
	char *s, *d;
	int len = 0;

	char *pos;
	int state;
	int ch = 0;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	cobj = nsp_setstr(N, &tobj, "", cobj1->val->d.str, cobj1->val->size);
	if (nsp_isstr(cobj) && (cobj->val->size > 0)) {
		d = s = cobj->val->d.str;
		while (*s) {
			if (strncmp(s, "=?", 2) != 0) {
				*d++ = *s++;
				len++;
				continue;
			}
			while (*s && *s != '?') s++;
			if (!*s) break;
			s++;
			/* we don't try to make sense of the charset */
			while (*s && *s != '?') s++;
			if (!*s) break;
			s++;
			if (*s == 'q' || *s == 'Q') {
				while (*s && *s != '?') s++;
				if (!*s) break;
				s++;
				while ((ch = *s++) != '\0') {
					if (ch == '?') break;
					if (ch != '=') {
						*d++ = (uchar)ch;
						len++;
					} else {
						if (isxdigit(s[0]) && isxdigit(s[1])) {
							*d++ = (uchar)hex2int(s);
							len++;
							s += 2;
						}
					}
				}
			} else if (*s == 'b' || *s == 'B') {
				while (*s && *s != '?') s++;
				if (!*s) break;
				s++;
				state = 0;
				while ((ch = *s++) != '\0') {
					if (isspace(ch)) continue;
					if (ch == '=') break;
					pos = strchr(b64chars, ch);
					if (pos == 0) break;
					switch (state) {
					case 0:
						*d = (pos - b64chars) << 2;
						state = 1;
						break;
					case 1:
						*d |= (pos - b64chars) >> 4;
						*++d = ((pos - b64chars) & 0x0f) << 4;
						len++;
						state = 2;
						break;
					case 2:
						*d |= (pos - b64chars) >> 2;
						*++d = ((pos - b64chars) & 0x03) << 6;
						len++;
						state = 3;
						break;
					case 3:
						*d++ |= (pos - b64chars);
						len++;
						state = 0;
						break;
					}
				}
			}
			if (ch == '=') {
				while (*s && *s != '?') s++;
			}
			if (!*s) break;
			s++;
			if (*s == '=') s++;
			ch = 0;
		}
		cobj->val->size = len;
		*d = '\0';
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_mime_rfc2047_encode)
{
#define __FN__ __FILE__ ":libnsp_net_mime_rfc2047_encode()"
	const char *hex = "0123456789ABCDEF";
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;
	uchar *cp;
	char *dest;
	unsigned int i;
	int di;
	int outlen = 0;
	int linelen = 0;
	int maxlinelen = 76;

	n_expect_argtype(N, NT_STRING, 1, cobj1, 1);
	if (cobj1->val->size == 0) {
		nsp_setstr(N, &N->r, "", NULL, 0);
		return 0;
	}
	cp = (uchar *)cobj1->val->d.str;
	// calculate output size
	for (i = 0; i < cobj1->val->size; i++) {
	breakline1:
		if (linelen >= maxlinelen) {
			outlen += 3;
			linelen = 0;
		}
		if (cp[i] != ' ' && (cp[i] < 33 || cp[i] == 61 || cp[i] > 127)) {
			linelen += 3;
			if (linelen >= maxlinelen) goto breakline1;
			outlen += 3;
		} else {
			linelen += 1;
			if (linelen >= maxlinelen) goto breakline1;
			outlen += 1;
		}
	}
	if (outlen == cobj1->val->size) {
		nsp_setstr(N, &N->r, "", cobj1->val->d.str, cobj1->val->size);
		return 0;
	}
	if ((dest = n_alloc(N, outlen + 1, 0)) == NULL) return 0;
	dest[outlen] = '\0';
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	di = 0;
	linelen = 0;
	// create output string
	nsp_strcat(N, robj, "=?UTF-8?Q?", -1);
	for (i = 0; i < cobj1->val->size; i++) {
	breakline2:
		if (linelen >= maxlinelen) {
			dest[di++] = '=';
			dest[di++] = '\r';
			dest[di++] = '\n';
			linelen = 0;
		}
		if (cp[i] != ' ' && (cp[i] < 33 || cp[i] == 61 || cp[i] > 127)) {
			linelen += 3;
			if (linelen >= maxlinelen) goto breakline2;
			dest[di++] = '=';
			dest[di++] = hex[(unsigned int)cp[i] / 16];
			dest[di++] = hex[(unsigned int)cp[i] & 15];
		} else {
			linelen += 1;
			if (linelen >= maxlinelen) goto breakline2;
			dest[di++] = cp[i];
		}
	}
	nsp_strcat(N, robj, dest, outlen);
	nsp_strcat(N, robj, "?=", -1);
	n_free(N, (void *)&dest, outlen + 1);
	return 0;
#undef __FN__
}
