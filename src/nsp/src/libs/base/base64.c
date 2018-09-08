/*
    NESLA NullLogic Embedded Scripting Language
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
#include "nsp/nsplib.h"
#include "base.h"

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

NSP_FUNCTION(libnsp_base_base64_decode)
{
#define __FN__ __FILE__ ":libnsp_base_base64_decode()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
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
		}
		else if (nc_islower(ch)) {
			x = ch - 'a' + 26;
		}
		else if (nc_isdigit(ch)) {
			x = ch - '0' + 52;
		}
		else if (ch == '+') {
			x = 62;
		}
		else if (ch == '/') {
			x = 63;
		}
		else {
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

NSP_FUNCTION(libnsp_base_base64_encode)
{
#define __FN__ __FILE__ ":libnsp_base_base64_encode()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
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
	for (i = 0;i < enclen;i++) {
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
	}
	else if (remlen == 2) {
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
