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
#ifndef _NSPLIB_H
#include "nsp/nsplib.h"
#endif
#include "opcodes.h"

    /*
     * calling this a compiler is something of a misnomer. it does not _compile_
     * code from multiple sources.  it tokenizes and optimizes source chunks.  that's all.
     */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef struct cstate {
	uchar *destbuf;
	long destmax;
	obj_t *tobj1;
	obj_t *lobj1;
	long index;
	long lineno;
	long offset;
} cstate;

static long n_unescape(nsp_state *N, char *src, char *dst, long len, cstate *state)
{
#define __FN__ __FILE__ ":n_unescape()"
	long i, n;
	short e = 0;

	settrace();
	if (dst == NULL) return 0;
	for (i = 0, n = 0; i < len; i++) {
		if (!e) {
			if (src[i] == '\\') {
				e = 1;
			}
			else {
				dst[n++] = src[i];
			}
			continue;
		}
		switch (src[i]) {
		case '0': dst[n++] = 0;    break;
		case 'a': dst[n++] = '\a'; break;
		case 't': dst[n++] = '\t'; break;
		case 'f': dst[n++] = '\f'; break;
		case 'e': dst[n++] = 27;   break;
		case 'r': dst[n++] = '\r'; break;
		case 'n': dst[n++] = '\n'; break;
		case '\'': dst[n++] = '\''; break;
		case '\"': dst[n++] = '\"'; break;
		case '`': dst[n++] = '`';  break;
		case '\\': dst[n++] = '\\'; break;
		default:
			n_warn(N, __FN__, "bad escape %d", src[i]);
			break;
		}
		e = 0;
	}
	dst[n] = '\0';
	return n;
#undef __FN__
}

static obj_t *n_newobj(nsp_state *N, cstate *state)
{
#define __FN__ __FILE__ ":n_newobj()"
	settrace();
	if (state->lobj1 == NULL) {
		state->lobj1 = state->tobj1->val->d.table.f = n_newiobj(N, state->index);
	}
	else {
		state->lobj1->next = n_newiobj(N, state->index);
		state->lobj1->next->prev = state->lobj1;
		state->lobj1 = state->lobj1->next;
	}
	state->index++;
	return state->lobj1;
#undef __FN__
}

/* Advance readptr to next non-blank */
static void n_skipblank(nsp_state *N, cstate *state)
{
#define __FN__ __FILE__ ":n_skipblank()"
	uchar *p = N->readptr;

	settrace();
	while (*p) {
		if (p[0] == '#') {
			p++;
			while (*p) {
				if (*p == '\n') {
					n_newobj(N, state);
					nsp_setnum(N, state->lobj1, NULL, ++state->lineno);
					state->lobj1->val->attr = OP_LINENUM;
					break;
				}
				else if (*p == '\r') break;
				p++;
			}
		}
		else if (p[0] == '/'&&p[1] == '/') {
			p += 2;
			while (*p) {
				if (*p == '\n') {
					n_newobj(N, state);
					nsp_setnum(N, state->lobj1, NULL, ++state->lineno);
					state->lobj1->val->attr = OP_LINENUM;
					break;
				}
				else if (*p == '\r') break;
				p++;
			}
		}
		else if (p[0] == '/'&&p[1] == '*') {
			p += 2;
			while (*p) {
				if (*p == '\n') state->lineno++;
				else if (p[0] == '*'&&p[1] == '/') {
					p += 2;
					if (*p == '\n') {
						n_newobj(N, state);
						nsp_setnum(N, state->lobj1, NULL, ++state->lineno);
						state->lobj1->val->attr = OP_LINENUM;
					}
					break;
				}
				p++;
			}
		}
		else {
			if (*p == '\n') {
				n_newobj(N, state);
				nsp_setnum(N, state->lobj1, NULL, ++state->lineno);
				state->lobj1->val->attr = OP_LINENUM;
			}
		}
		if (!nc_isspace(*p)) break;
		p++;
	}
	N->readptr = p;
	return;
#undef __FN__
}

/* Advance readptr to next matching quote */
static void n_skipquote(nsp_state *N, unsigned short c, cstate *state, unsigned short verbatim)
{
#define __FN__ __FILE__ ":n_skipquote()"
	settrace();
	while (*N->readptr) {
		if (*N->readptr == '\\' && !verbatim) {
			N->readptr++;
		}
		else if (*N->readptr == c) {
			N->readptr++;
			break;
		}
		else if (*N->readptr == '\n') {
			++state->lineno;
		}
		N->readptr++;
		if (!*N->readptr) n_error(N, NE_SYNTAX, __FN__, "unterminated string");
	}
	return;
#undef __FN__
}

/* return the next quoted block */
static obj_t *n_extractquote(nsp_state *N, cstate *state, unsigned short verbatim)
{
#define __FN__ __FILE__ ":n_extractquote()"
	obj_t *cobj;
	char q = *N->readptr;
	char *qs, *qe;
	obj_t tobj;
	int n;

	DEBUG_IN();
	settrace();
	sanetest();
	if ((q != '\'') && (q != '\"') && (q != '`')) {
		DEBUG_OUT();
		return NULL;
	}
	N->readptr++;
	qs = (char *)N->readptr;
	n_skipquote(N, q, state, verbatim);
	qe = (char *)N->readptr;

	if (verbatim) {
		cobj = nsp_setstr(N, &N->r, "", qs, qe - qs - 1);
	}
	else {
		nc_memset((void *)&tobj, 0, sizeof(obj_t));
		nsp_setstr(N, &tobj, "", NULL, qe - qs - 1);
		n = n_unescape(N, qs, tobj.val->d.str, qe - qs - 1, state);
		cobj = nsp_setstr(N, &N->r, "", tobj.val->d.str, n);
		nsp_unlinkval(N, &tobj);
	}
	DEBUG_OUT();
	return cobj;
#undef __FN__
}

static void n_decompose_sub(nsp_state *N, cstate *state)
{
#define __FN__ __FILE__ ":n_decompose_sub()"
	char lastname[MAX_OBJNAMELEN + 1];
	char *p;
	short op;
	short prevop = 0;

	settrace();
	while (*N->readptr) {
		n_skipblank(N, state);
		op = n_getop(N, lastname);
		n_skipblank(N, state);
		if (op == OP_UNDEFINED) {
			n_warn(N, __FN__, "bad op? index=%d line=%d op=%d:%d name='%s'", state->index, state->lineno, op, N->readptr[0], lastname);
			return;
		}
		else if (op == OP_LABEL) {
			n_newobj(N, state);
			nsp_setstr(N, state->lobj1, NULL, lastname, -1);
			state->lobj1->val->attr = op;
		}
		else if (OP_ISMATH(op) || OP_ISKEY(op) || OP_ISPUNC(op)) {
			if (OP_ISKEY(op) && prevop == OP_PDOT) {
				// it's NOT a keyword!
				int i;

				for (i = OP_KEXIT; i <= OP_KBREAK; i++) {
					if (nc_strcmp(oplist[i].name, lastname) == 0) {
						n_newobj(N, state);
						nsp_setstr(N, state->lobj1, NULL, lastname, -1);
						state->lobj1->val->attr = OP_LABEL;
						break;
					}
				}
			}
			else {
				n_newobj(N, state);
				nsp_setstr(N, state->lobj1, NULL, n_getsym(N, op), -1);
				state->lobj1->val->attr = op;
			}
		}
		else if (N->readptr[0] == '\"' || N->readptr[0] == '\'' || N->readptr[0] == '`') {
			op = N->readptr[0];
			n_newobj(N, state);
			nsp_linkval(N, state->lobj1, n_extractquote(N, state, 0));
			state->lobj1->val->attr = op == '`' ? OP_ESTRDATA : OP_STRDATA;
		}
		else if (N->readptr[0] == '@' && (N->readptr[1] == '\"' || N->readptr[1] == '\'' || N->readptr[1] == '`')) {
			op = N->readptr[1];
			N->readptr++;
			n_newobj(N, state);
			nsp_linkval(N, state->lobj1, n_extractquote(N, state, 1));
			state->lobj1->val->attr = op == '`' ? OP_ESTRDATA : OP_STRDATA;
		}
		else if (nc_isdigit(*N->readptr)) {
			p = (char *)N->readptr;
			while (nc_isdigit(*N->readptr) || *N->readptr == '.') N->readptr++;
			n_newobj(N, state);
			nsp_setstr(N, state->lobj1, NULL, p, (char *)N->readptr - p);
			state->lobj1->val->attr = OP_NUMDATA;
		}
		prevop = op;
	}
	return;
#undef __FN__
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define testgrow(b) \
if (state.offset+b+1>state.destmax) { \
	state.destbuf=n_realloc(N, (void *)&state.destbuf, state.destmax+b+1024, state.destmax, 0); \
	state.destmax+=b+1024; \
	if (state.destbuf==NULL) n_warn(N, __FN__, "realloc error!!!"); \
}

uchar   *n_decompose(nsp_state *N, char *srcfile, uchar *srctext, uchar **dsttext, int *dstsize)
{
#define __FN__ __FILE__ ":n_decompose()"
	cstate state;
	obj_t *cobj, *tobj;
	unsigned short op;
	uchar *p, *p2;

	settrace();

	*dsttext = NULL;
	*dstsize = 0;

	if ((srctext[0] == 0x0D) && ((srctext[1] == 0xAC))) {
		n_warn(N, __FN__, "already chewed on this");
		return srctext;
	}
	nc_memset((char *)&state, 0, sizeof(state));
	state.lineno = 1;
	state.destmax = 1024;
	state.destbuf = (uchar *)n_alloc(N, state.destmax, 0);
	N->readptr = srctext;
	tobj = nsp_settable(N, &N->g, "decomped_script");
	nsp_freetable(N, tobj);
	state.tobj1 = nsp_settable(N, tobj, "code");
	n_decompose_sub(N, &state);
	/* header - 8 bytes */
	testgrow(8); /* safe portable use of sprintf is still considered dangerous according to openbsd */
	state.destbuf[state.offset++] = 0x0D;
	state.destbuf[state.offset++] = 0xAC;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	/* file size - 4 bytes */
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	/* optab offset  - 4 bytes (little endian) */
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	/* symtab offset - 4 bytes (little endian) */
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	state.destbuf[state.offset++] = 0;
	/* now write the ops */
	/* optab offset */
	writei4(state.offset, (state.destbuf + 12));
	/* set line number to 1 */
	testgrow((long)(5));
	state.destbuf[state.offset++] = OP_LINENUM;
	writei4(1, (state.destbuf + state.offset));
	state.offset += 4;

	for (cobj = state.tobj1->val->d.table.f; cobj; cobj = cobj->next) {
		op = (unsigned short)cobj->val->attr;
		cobj->val->attr = 0;
		if (op == OP_LINENUM) {
			testgrow((long)(5));
			state.destbuf[state.offset++] = op & 255;
			writei4((int)cobj->val->d.num, (state.destbuf + state.offset));
			state.offset += 4;
			continue;
		}
		if (!nsp_isstr(cobj)) break;
		if (op == OP_UNDEFINED) break;
		if (op == OP_POBRACE) {
			testgrow((long)(5));
			state.destbuf[state.offset++] = op & 255;
			writei4(0, (state.destbuf + state.offset));
			state.offset += 4;
			continue;
		}
		else if (op == OP_POPAREN) {
			testgrow((long)(3));
			state.destbuf[state.offset++] = op & 255;
			writei2(0, (state.destbuf + state.offset));
			state.offset += 2;
			continue;
		}
		else if (op == OP_STRDATA || op == OP_ESTRDATA) {
			testgrow((long)(6 + cobj->val->size));
			state.destbuf[state.offset++] = op & 255;
			writei4(cobj->val->size, (state.destbuf + state.offset));
			state.offset += 4;
			nc_memcpy((char *)state.destbuf + state.offset, cobj->val->d.str, cobj->val->size);
			state.offset += cobj->val->size;
			state.destbuf[state.offset++] = 0;
		}
		else if (op == OP_NUMDATA) {
			testgrow((long)(3 + cobj->val->size));
			state.destbuf[state.offset++] = op & 255;
			state.destbuf[state.offset++] = (uchar)(cobj->val->size & 255);
			nc_memcpy((char *)state.destbuf + state.offset, cobj->val->d.str, cobj->val->size);
			state.offset += cobj->val->size;
			state.destbuf[state.offset++] = 0;
		}
		else if (op == OP_LABEL) {
			testgrow((long)(3 + cobj->val->size));
			state.destbuf[state.offset++] = op & 255;
			state.destbuf[state.offset++] = (uchar)(cobj->val->size & 255);
			nc_memcpy((char *)state.destbuf + state.offset, cobj->val->d.str, cobj->val->size);
			state.offset += cobj->val->size;
			state.destbuf[state.offset++] = '\0';
		}
		else if (OP_ISMATH(op) || OP_ISKEY(op) || OP_ISPUNC(op)) {
			testgrow(1);
			state.destbuf[state.offset++] = op & 255;
			if (op == OP_KFUNC) {
				char *p = srcfile;

				//n_warn(N, __FN__, "'%s'", p);
				if (cobj->next->val->attr == OP_LABEL) {
					cobj = cobj->next;
					op = (unsigned short)cobj->val->attr;
					cobj->val->attr = 0;
					testgrow((long)(3 + cobj->val->size));
					state.destbuf[state.offset++] = op & 255;
					state.destbuf[state.offset++] = (uchar)(cobj->val->size & 255);
					nc_memcpy((char *)state.destbuf + state.offset, cobj->val->d.str, cobj->val->size);
					state.offset += cobj->val->size;
					state.destbuf[state.offset++] = '\0';
				}
				if (srcfile == NULL) p = "";
				testgrow((long)(3 + nc_strlen(p)));
				state.destbuf[state.offset++] = OP_LABEL;
				state.destbuf[state.offset++] = (uchar)(nc_strlen(p) & 255);
				nc_memcpy((char *)state.destbuf + state.offset, p, nc_strlen(p));
				state.offset += nc_strlen(p);
				state.destbuf[state.offset++] = '\0';
			}
		}
		else {
			n_warn(N, __FN__, "bad op?");
		}
	}
	/* file size */
	writei4(state.offset, (state.destbuf + 8));
	/* add some trailing nulls for fun... */
	testgrow(4); writei4(0, (state.destbuf + state.offset));
	state.offset += 4;
	/* n_dumpvars(N, &N->g, 0); */
	nsp_freetable(N, tobj);
	*dsttext = state.destbuf;
	*dstsize = state.destmax;
	for (p = *dsttext + 12; p < *dsttext + state.offset - 4;) {
		if (*p == OP_LINENUM) { p += 5; continue; }
		if (*p == OP_POBRACE) {
			p2 = n_seekop(N, p, 1);
			if (p2 <= p) {
				n_warn(N, __FN__, "pointer did not progress");
				break;
			}
			--p2;
			// if (*p2!=OP_PCBRACE) n_warn(N, __FN__, "no OP_PCBRACE? %d", (p+(p2-p)+5)[0]);
			if (*p2 != OP_PCBRACE) {
				/*
				int i;
				for (i=0;i<state.offset;i++) {
				if (i==p2-*dsttext) {
				printf("-----------------------\r\n[%d]\r\n-------------------", state.destbuf[i]);
				} else {
				if (state.destbuf[i]>=32 && state.destbuf[i]<128) {
				printf("'%c' %d\r\n", state.destbuf[i], state.destbuf[i]);
				} else {
				printf("%d\r\n", state.destbuf[i]);
				}
				}
				}
				*/
				n_warn(N, __FN__, "no OP_PCBRACE? %d .. %d %d %d %d [%d] %d %d", OP_PCBRACE, p2[-4], p2[-3], p2[-2], p2[-1], p2[0], p2[1], p2[2]);
				//n_decompile(N, *dsttext+12, *dsttext+state.offset-4, NULL, 0);
			}
			writei4((p2 - p - 5), (p + 1));
		}
		else if (*p == OP_POPAREN) {
			p2 = n_seekop(N, p, 1);
			if (p2 <= p) {
				n_warn(N, __FN__, "pointer did not progress");
				break;
			}
			--p2;
			if (*p2 != OP_PCPAREN) {
				n_warn(N, __FN__, "no OP_PCPAREN? %d .. %d %d %d %d [%d] %d %d", OP_PCPAREN, p2[-4], p2[-3], p2[-2], p2[-1], p2[0], p2[1], p2[2]);
			}
			writei2((p2 - p - 3), (p + 1));
		}
		p = n_seekop(N, p, 0);
	}
	return *dsttext;
#undef __FN__
}
