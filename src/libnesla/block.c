/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2008 Dan Cahill

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

/* Advance readptr to next specified char */
void n_skipto(nes_state *N, const char *fn, unsigned short c)
{
#define __FUNCTION__ __FILE__ ":n_skipto()"
	settrace();
	while (*N->readptr) {
		if (*N->readptr==c) return;
		N->readptr=n_seekop(N, N->readptr, 1, 1);
	}
	n_error(N, NE_SYNTAX, fn, "expected a '%s'", n_getsym(N, c));
	return;
#undef __FUNCTION__
}

void n_if(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_if()"
	char done=0, t=0;

	DEBUG_IN();
	settrace();
l1:
	if (*N->readptr==OP_POPAREN) {
		N->readptr++;
		if (!done) t=(char)nes_tobool(N, nes_eval(N, (char *)N->readptr));
		n_skipto(N, __FUNCTION__, OP_PCPAREN);
		N->readptr++;
	}
l2:
	if (!t||done) {
		if (*N->readptr==OP_POBRACE) {
			N->readptr++;
			n_skipto(N, __FUNCTION__, OP_PCBRACE);
			N->readptr++;
		} else {
			n_skipto(N, __FUNCTION__, OP_PSEMICOL);
			if (*N->readptr==OP_PSEMICOL) N->readptr++;
		}
	} else {
		done=1;
		if (*N->readptr==OP_POBRACE) {
			nes_exec(N, (char *)N->readptr);
			n_skipto(N, __FUNCTION__, OP_PCBRACE);
			N->readptr++;
		} else {
			N->single=1;
			nes_exec(N, (char *)N->readptr);
			if (*N->readptr==OP_PSEMICOL) N->readptr++;
		}
	}
/*
	if (*N->readptr==OP_POBRACE) {
		if (!t||done) {
			N->readptr++;
		} else {
			done=1;
			nes_exec(N, (char *)N->readptr);
		}
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		N->readptr++;
	} else {
		if (!t||done) {
			n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		} else {
			done=1;
			N->single=1;
			nes_exec(N, (char *)N->readptr);
		}
		if (*N->readptr==OP_PSEMICOL) N->readptr++;
	}
*/

	if (N->ret) { DEBUG_OUT(); return; }
	if (*N->readptr==OP_KELSE) {
		N->readptr++;
		if (*N->readptr==OP_KIF) {
			N->readptr++;
			goto l1;
		}
		t=1;
		goto l2;
	}
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_for(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_for()"
	uchar *arginit, *argcomp, *argexec;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	n_expect(N, __FUNCTION__, OP_POPAREN);
	arginit=++N->readptr;
	n_skipto(N, __FUNCTION__, OP_PSEMICOL);
	argcomp=++N->readptr;
	n_skipto(N, __FUNCTION__, OP_PSEMICOL);
	argexec=++N->readptr;
	n_skipto(N, __FUNCTION__, OP_PCPAREN);
	bs=++N->readptr;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		single=0;
	} else {
		n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		single=1;
	}
	be=++N->readptr;
	N->readptr=arginit;
	n_readvar(N, &N->l, NULL);
	for (;;) {
		if (argcomp[0]!=OP_PSEMICOL) {
			cobj=nes_eval(N, (char *)argcomp);
			n_expect(N, __FUNCTION__, OP_PSEMICOL);
			N->readptr++;
			if (nes_tonum(N, cobj)==0) break;
		}
		N->readptr=bs;
		N->single=single;
		nes_exec(N, (char *)N->readptr);
		if (N->cnt) N->cnt=0;
		if (N->brk) { N->brk--; break; }
		if (N->ret) break;
		N->readptr=argexec;
		n_readvar(N, &N->l, NULL);
	}
	N->readptr=be;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_do(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_do()"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	bs=N->readptr;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		N->readptr++;
		single=0;
	} else {
		n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		N->readptr++;
		single=1;
	}
	be=N->readptr;
	for (;;) {
		N->readptr=bs;
		N->single=single;
		nes_exec(N, (char *)N->readptr);
		if (N->cnt>0) N->cnt=0;
		if (N->brk>0) { N->brk--; break; }
		if (N->ret) break;
		N->readptr=be;
		n_expect(N, __FUNCTION__, OP_KWHILE);
		N->readptr++;
		n_expect(N, __FUNCTION__, OP_POPAREN);
		N->readptr++;
		argcomp=N->readptr;
		cobj=nes_eval(N, (char *)argcomp);
		n_expect(N, __FUNCTION__, OP_PCPAREN);
		if (nes_tonum(N, cobj)==0) break;
	}
	N->readptr=be;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_while(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_while()"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	n_expect(N, __FUNCTION__, OP_POPAREN);
	argcomp=++N->readptr;
	n_skipto(N, __FUNCTION__, OP_PCPAREN);
	bs=++N->readptr;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		N->readptr++;
		single=0;
	} else {
		n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		N->readptr++;
		single=1;
	}
	be=N->readptr;
	for (;;) {
		cobj=nes_eval(N, (char *)argcomp);
		n_expect(N, __FUNCTION__, OP_PCPAREN);
		N->readptr++;
		if (nes_tonum(N, cobj)==0) break;
		N->readptr=bs;
		N->single=single;
		nes_exec(N, (char *)N->readptr);
		if (N->cnt>0) N->cnt=0;
		if (N->brk>0) { N->brk--; break; }
		if (N->ret) break;
	}
	N->readptr=be;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_try(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_try()"
	jmp_buf *savjmp;
	uchar *bs, *be;
	obj_t *tobj;

	DEBUG_IN();
	settrace();
	bs=N->readptr;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
	} else {
		n_skipto(N, __FUNCTION__, OP_PSEMICOL);
	}
	N->readptr++;
	be=N->readptr;
	N->readptr=bs;
	savjmp=N->savjmp;
	N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
	if (setjmp(*N->savjmp)==0) {
		nes_exec(N, (char *)N->readptr);
		tobj=nes_getobj(N, &N->l, "_exception");
		if (!nes_isnull(tobj)) nes_unlinkval(N, tobj);
	} else {
		tobj=nes_settable(N, &N->l, "_exception");
		nes_setnum(N, tobj, "errno", N->err);
		nes_setstr(N, tobj, "errtext", N->errbuf, -1);
		N->err=0;
	}
	n_free(N, (void *)&N->savjmp);
	N->savjmp=savjmp;
	N->readptr=be;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}
