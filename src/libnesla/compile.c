/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2009 Dan Cahill

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
#include "nesla/libnesla.h"
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

static long n_unescape(nes_state *N, char *src, char *dst, long len, cstate *state)
{
#define __FUNCTION__ __FILE__ ":n_unescape()"
	long i, n;
	short e=0;

	settrace();
	if (dst==NULL) return 0;
	for (i=0,n=0;i<len;i++) {
		if (!e) {
			if (src[i]=='\\') {
				e=1;
			} else {
				dst[n++]=src[i];
			}
			continue;
		}
		switch (src[i]) {
			case '0'  : dst[n++]=0;    break;
			case 'a'  : dst[n++]='\a'; break;
			case 't'  : dst[n++]='\t'; break;
			case 'f'  : dst[n++]='\f'; break;
			case 'e'  : dst[n++]=27;   break;
			case 'r'  : dst[n++]='\r'; break;
			case 'n'  : dst[n++]='\n'; break;
			case '\'' : dst[n++]='\''; break;
			case '\"' : dst[n++]='\"'; break;
			case '\\' : dst[n++]='\\'; break;
			default   : break;
		}
		e=0;
	}
	dst[n]='\0';
	return n;
#undef __FUNCTION__
}

static obj_t *n_newobj(nes_state *N, cstate *state)
{
#define __FUNCTION__ __FILE__ ":n_newobj()"
	settrace();
	if (state->lobj1==NULL) {
		state->lobj1=state->tobj1->val->d.table.f=n_newiobj(N, state->index);
	} else {
		state->lobj1->next=n_newiobj(N, state->index);
		state->lobj1->next->prev=state->lobj1;
		state->lobj1=state->lobj1->next;
	}
	state->index++;
	return state->lobj1;
#undef __FUNCTION__
}

/* Advance readptr to next non-blank */
static void n_skipblank(nes_state *N, cstate *state)
{
#define __FUNCTION__ __FILE__ ":n_skipblank()"
	uchar *p=N->readptr;

	settrace();
	while (*p) {
		if (p[0]=='#') {
			p++;
			while (*p) {
				if (*p=='\n') { state->lineno++; break; }
				else if (*p=='\r') break;
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='/') {
			p+=2;
			while (*p) {
				if (*p=='\n') { state->lineno++; break; }
				else if (*p=='\r') break;
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='*') {
			p+=2;
			while (*p) {
				if (*p=='\n') state->lineno++;
				else if (p[0]=='*'&&p[1]=='/') {
					p+=2;
					if (*p=='\n') state->lineno++;
					break;
				}
				p++;
			}
		} else {
			if (*p=='\n') state->lineno++;
		}
		if (!nc_isspace(*p)) break;
		p++;
	}
	N->readptr=p;
	return;
#undef __FUNCTION__
}

/* Advance readptr to next matching quote */
static void n_skipquote(nes_state *N, unsigned short c)
{
#define __FUNCTION__ __FILE__ ":n_skipquote()"
	settrace();
	while (*N->readptr) {
		if (*N->readptr=='\\') {
			N->readptr++;
		} else if (*N->readptr==c) {
			N->readptr++;
			break;
		}
		N->readptr++;
		if (!*N->readptr) n_error(N, NE_SYNTAX, __FUNCTION__, "unterminated string");
	}
	return;
#undef __FUNCTION__
}

/* return the next quoted block */
static obj_t *n_extractquote(nes_state *N, cstate *state)
{
#define __FUNCTION__ __FILE__ ":n_extractquote()"
	obj_t *cobj;
	char q=*N->readptr;
	char *qs, *qe;

	DEBUG_IN();
	settrace();
	sanetest();
	if ((q!='\'')&&(q!='\"')) {
		DEBUG_OUT();
		return NULL;
	}
	N->readptr++;
	qs=(char *)N->readptr;
	n_skipquote(N, q);
	qe=(char *)N->readptr;
	cobj=nes_setstr(N, &N->r, "", NULL, qe-qs-1);
	cobj->val->size=n_unescape(N, qs, cobj->val->d.str, qe-qs-1, state);
	DEBUG_OUT();
	return cobj;
#undef __FUNCTION__
}

static void n_decompose_sub(nes_state *N, cstate *state)
{
#define __FUNCTION__ __FILE__ ":n_decompose_sub()"
	char lastname[MAX_OBJNAMELEN+1];
	char *p;
	short op;

	settrace();
	while (*N->readptr) {
		n_skipblank(N, state);
		op=n_getop(N, lastname);
		n_skipblank(N, state);
		if (op==OP_UNDEFINED) {
			n_warn(N, __FUNCTION__, "bad op? index=%d line=%d op=%d:%d name='%s'", state->index, state->lineno, op, N->readptr[0], lastname);
/*
			obj_t *cobj;

			n_warn(N, __FUNCTION__, "bad op? index=%d op=%d:%d name='%s'", state->index, op, N->readptr[0], lastname);
			for (cobj=state->tobj1->val->d.table.f;cobj;cobj=cobj->next) {
				cobj->val->attr=0;
			}
			n_dumpvars(N, &N->g, 0);
			n_error(N, NE_SYNTAX, __FUNCTION__, "bad op");
*/
			return;
		} else if (op==OP_LABEL) {
			n_newobj(N, state);
			nes_setstr(N, state->lobj1, NULL, lastname, -1);
			state->lobj1->val->attr=op;
		} else if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
/*
			if (op==OP_PCPAREN) return;
			if (op==OP_PCBRACE) return;
*/
			n_newobj(N, state);
			nes_setstr(N, state->lobj1, NULL, n_getsym(N, op), -1);
			state->lobj1->val->attr=op;
/*
			if (op==OP_POBRACE) {
				nobj=nes_settable(N, tobj, n_ntoa(N, x, *index, 10, 0));
				*index+=1;
				while (N->lastop!=OP_PCBRACE) {
					n_decompose_sub(N, rawtext, nobj, index);
				}
				if (N->lastop==OP_PCBRACE) {
					nes_setstr(N, tobj, n_ntoa(N, x, *index, 10, 0), lastname, -1);
					*index+=1;
				}
			}

			if (op==OP_POPAREN) {
				nobj=nes_settable(N, tobj, n_ntoa(N, x, *index, 10, 0));
				*index+=1;
				while (N->lastop!=OP_PCPAREN) {
					n_decompose_sub(N, rawtext, nobj, index);
				}
				if (N->lastop==OP_PCPAREN) {
					nes_setstr(N, tobj, n_ntoa(N, x, *index, 10, 0), lastname, -1);
					*index+=1;
				}
			}
*/
		} else if ((*N->readptr=='\"')||(*N->readptr=='\'')) {
			n_newobj(N, state);
			nes_linkval(N, state->lobj1, n_extractquote(N, state));
			state->lobj1->val->attr=OP_STRDATA;
		} else if (nc_isdigit(*N->readptr)) {
			p=(char *)N->readptr;
			while (nc_isdigit(*N->readptr)||*N->readptr=='.') N->readptr++;
			n_newobj(N, state);
			nes_setstr(N, state->lobj1, NULL, p, (char *)N->readptr-p);
			state->lobj1->val->attr=OP_NUMDATA;
		}
	}
	return;
#undef __FUNCTION__
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define testgrow(b) \
	if (state.offset+b+1>state.destmax) { \
		state.destmax+=b+1024; \
		state.destbuf=n_realloc(N, (void *)&state.destbuf, state.destmax, 0); \
		if (state.destbuf==NULL) n_warn(N, __FUNCTION__, "realloc error!!!"); \
	}

uchar *n_decompose(nes_state *N, uchar *rawtext)
{
#define __FUNCTION__ __FILE__ ":n_decompose()"
	cstate state;
	obj_t *cobj, *tobj;
	unsigned short op;

	settrace();
	if ((rawtext[0]==0x0D)&&((rawtext[1]==0xAC))) {
		n_warn(N, __FUNCTION__, "already chewed on this");
		return rawtext;
	}
	nc_memset((char *)&state, 0, sizeof(state));
	state.lineno=1;
	state.destmax=1024;
	state.destbuf=malloc(state.destmax);
	N->readptr=rawtext;
	tobj=nes_settable(N, &N->g, "decomped_script");
	nes_freetable(N, tobj);
	state.tobj1=nes_settable(N, tobj, "code");
	n_decompose_sub(N, &state);
	/* header - 8 bytes */
	testgrow(8); /* safe portable use of sprintf is still considered dangerous according to openbsd */
	state.destbuf[state.offset++]=0x0D;
	state.destbuf[state.offset++]=0xAC;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* file size - 4 bytes */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* optab offset  - 4 bytes (little endian) */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* symtab offset - 4 bytes (little endian) */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* now write the ops */
	/* optab offset */
	writei4(state.offset, (state.destbuf+12));
	for (cobj=state.tobj1->val->d.table.f;cobj;cobj=cobj->next) {
		op=(unsigned short)cobj->val->attr;
		cobj->val->attr=0;
		if (!nes_isstr(cobj)) break;
		if (op==OP_UNDEFINED) break;
		if (op==OP_STRDATA) {
			testgrow((long)(6+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			writei4(cobj->val->size, (state.destbuf+state.offset));
			state.offset+=4;
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_NUMDATA) {
			testgrow((long)(3+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			state.destbuf[state.offset++]=(uchar)(cobj->val->size&255);
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_LABEL) {
			testgrow((long)(3+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			state.destbuf[state.offset++]=(uchar)(cobj->val->size&255);
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]='\0';
		} else if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
			testgrow(1);
			state.destbuf[state.offset++]=op&255;
		} else {
			n_warn(N, __FUNCTION__, "bad op?");
		}
	}
	/* file size */
	writei4(state.offset, (state.destbuf+8));
	/* add some trailing nulls for fun... */
	testgrow(4); writei4(0, (state.destbuf+state.offset));
	state.offset+=4;
	/* n_dumpvars(N, &N->g, 0); */
	nes_freetable(N, tobj);
	return state.destbuf;
#undef __FUNCTION__
}
