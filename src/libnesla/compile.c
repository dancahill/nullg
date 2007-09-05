/*
    NESLA NullLogic Embedded Scripting Language - Copyright (C) 2007 Dan Cahill

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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef struct cstate {
	uchar *destbuf;
	int destmax;
	obj_t *tobj1;
	obj_t *lobj1;
	int index;
	int offset;
} cstate;

static int n_unescape(nes_state *N, char *src, char *dst, int len)
{
	int i, n;
	short e=0;

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
}

static obj_t *n_newobj(nes_state *N, cstate *state)
{
	if (state->lobj1==NULL) {
		state->lobj1=state->tobj1->val->d.table=n_newiobj(N, state->index);
	} else {
		state->lobj1->next=n_newiobj(N, state->index);
		state->lobj1->next->prev=state->lobj1;
		state->lobj1=state->lobj1->next;
	}
	state->index++;
	return state->lobj1;
}

/* Advance readptr to next non-blank */
static void n_skipblank(nes_state *N)
{
	uchar *p=N->readptr;

	while (*p) {
		if (p[0]=='#') {
			p++;
			while (*p) {
				if (*p=='\r'||*p=='\n') { break; }
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='/') {
			p+=2;
			while (*p) {
				if (*p=='\r'||*p=='\n') { break; }
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='*') {
			p+=2;
			while (*p) {
				if (p[0]=='*'&&p[1]=='/') { p+=2; break; }
				p++;
			}
		}
		if (!nc_isspace(*p)) break;
		p++;
	}
	N->readptr=p;
	return;
}

/* Advance readptr to next matching quote */
static void n_skipquote(nes_state *N, unsigned short c)
{
	while (*N->readptr) {
		if (*N->readptr=='\\') {
			N->readptr++;
		} else if (*N->readptr==c) {
			N->readptr++;
			break;
		}
		N->readptr++;
		if (!*N->readptr) n_error(N, NE_SYNTAX, "n_skipquote", "unterminated string");
	}
	return;
}

/* return the next quoted block */
static obj_t *n_extractquote(nes_state *N)
{
	obj_t *cobj;
	char q=*N->readptr;
	char *qs, *qe;

	DEBUG_IN();
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
	cobj->val->size=n_unescape(N, qs, cobj->val->d.str, qe-qs-1);
	DEBUG_OUT();
	return cobj;
}

static void n_decompose_sub(nes_state *N, cstate *state)
{
	char lastname[MAX_OBJNAMELEN+1];
	char *p;
	short op;
	int len;

	while (*N->readptr) {
		n_skipblank(N);
		op=n_getop(N, lastname);
		if (op==OP_UNDEFINED) {
			n_warn(N, "x", "bad op? index=%d op=%d:%d name='%s'", state->index, op, N->readptr[0], lastname);
/*
			obj_t *cobj;

			n_warn(N, "x", "bad op? index=%d op=%d:%d name='%s'", state->index, op, N->readptr[0], lastname);
			for (cobj=state->tobj1->val->d.table;cobj;cobj=cobj->next) {
				cobj->val->attr=0;
			}
			n_dumpvars(N, &N->g, 0);
			n_error(N, NE_SYNTAX, "n_decompose_sub", "bad op");
*/
			return;
		}

		if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
/*
			if (op==OP_PCPAREN) return;
			if (op==OP_PCBRACE) return;
*/
			n_newobj(N, state);
			p=n_getsym(N, op);
			len=nc_strlen(p);
			nes_setstr(N, state->lobj1, NULL, p, len);
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
		} else if (op==OP_LABEL) {
			n_newobj(N, state);
			len=nc_strlen(lastname);
			nes_setstr(N, state->lobj1, NULL, lastname, len);
			state->lobj1->val->attr=op;
		} else if (nc_isdigit(*N->readptr)) {
			p=(char *)N->readptr;
			while (nc_isdigit(*N->readptr)||*N->readptr=='.') N->readptr++;
			n_newobj(N, state);
			len=(char *)N->readptr-p;
			nes_setstr(N, state->lobj1, NULL, p, len);
			state->lobj1->val->attr=OP_NUMDATA;
		}
x:
		if (!*N->readptr) return;
		n_skipblank(N);
		if ((*N->readptr=='\"')||(*N->readptr=='\'')) {
			n_newobj(N, state);
			nes_linkval(N, state->lobj1, n_extractquote(N));
			state->lobj1->val->attr=OP_STRDATA;
			len=state->lobj1->val->size;
			goto x;
		} else if (nc_isdigit(*N->readptr)) {
			p=(char *)N->readptr;
			while (nc_isdigit(*N->readptr)||*N->readptr=='.') N->readptr++;
			n_newobj(N, state);
			len=(char *)N->readptr-p;
			nes_setstr(N, state->lobj1, NULL, p, len);
			state->lobj1->val->attr=OP_NUMDATA;
			goto x;
		}
	}
	return;
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
#define __FUNCTION__ "n_decompose"
	cstate state;
	obj_t *cobj, *tobj;
	unsigned short op;

	if ((rawtext[0]==0x0D)&&((rawtext[1]==0xAC))) {
		n_warn(N, __FUNCTION__, "already chewed on this");
		return rawtext;
	}
	nc_memset((char *)&state, 0, sizeof(state));
	state.destmax=1024;
	state.destbuf=malloc(state.destmax);
	N->readptr=rawtext;
	tobj=nes_settable(N, &N->g, "decomped_script");
	nes_freetable(N, tobj);
	state.tobj1=nes_settable(N, tobj, "code");
	n_decompose_sub(N, &state);
	/* header - 8 bytes */
	testgrow(8); state.offset+=sprintf((char *)state.destbuf+state.offset, "%c%c%c%c%c%c%c%c", 0x0D, 0xAC, 0, 0, 0, 0, 0, 0);
	/* file size - 4 bytes */
	testgrow(4); state.offset+=sprintf((char *)state.destbuf+state.offset, "%c%c%c%c", 0, 0, 0, 0);
	/* optab offset  - 4 bytes (little endian) */
	testgrow(4); state.offset+=sprintf((char *)state.destbuf+state.offset, "%c%c%c%c", 0, 0, 0, 0);
	/* symtab offset - 4 bytes (little endian) */
	testgrow(4); state.offset+=sprintf((char *)state.destbuf+state.offset, "%c%c%c%c", 0, 0, 0, 0);
	/* now write the ops */
	/* optab offset */
	writei4(state.offset, (state.destbuf+12));
	for (cobj=state.tobj1->val->d.table;cobj;cobj=cobj->next) {
		op=(unsigned short)cobj->val->attr;
		cobj->val->attr=0;
		if (!nes_isstr(cobj)) break;
		if (op==OP_UNDEFINED) break;
		if (op==OP_STRDATA) {
			testgrow((int)(6+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			writei4(cobj->val->size, (state.destbuf+state.offset));
			state.offset+=4;
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_NUMDATA) {
			testgrow((int)(3+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			state.offset+=sprintf((char *)state.destbuf+state.offset, "%c", cobj->val->size&255);
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_LABEL) {
			testgrow((int)(3+cobj->val->size));
			state.offset+=sprintf((char *)state.destbuf+state.offset, "%c%c%s%c", op&255, cobj->val->size&255, cobj->val->d.str, '\0');
		} else if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
			testgrow(1);
			state.offset+=sprintf((char *)state.destbuf+state.offset, "%c", op&255);
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
