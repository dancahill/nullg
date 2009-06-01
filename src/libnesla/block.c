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
	if (single) --N->readptr;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_foreach(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_foreach()"
	char itemnamebuf[MAX_OBJNAMELEN+1];
	char setnamebuf[MAX_OBJNAMELEN+1];
	obj_t *iobj, *sobj, *xobj;
	uchar *bs, *be;
	short single;

	DEBUG_IN();
	settrace();
	n_expect(N, __FUNCTION__, OP_POPAREN);
	++N->readptr;
	n_expect(N, __FUNCTION__, OP_LABEL);
	n_getlabel(N, itemnamebuf);
	n_expect(N, __FUNCTION__, OP_LABEL);
	if (nc_strcmp(n_getlabel(N, NULL), "in")!=0) {
		n_error(N, NE_INTERNAL, __FUNCTION__, "expected 'in'");
	}
	n_expect(N, __FUNCTION__, OP_LABEL);
	n_getlabel(N, setnamebuf);
	sobj=nes_getobj(N, NULL, setnamebuf);
	if (!nes_istable(sobj)) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a table for set obj");
	n_expect(N, __FUNCTION__, OP_PCPAREN);
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
	xobj=nes_setnum(N, &N->l, itemnamebuf, 0);
	for (iobj=sobj->val->d.table.f; iobj; iobj=iobj->next) {
		if (nes_isnull(iobj) || iobj->val->attr&NST_SYSTEM) continue;
		N->readptr=bs;
		N->single=single;
		nes_linkval(N, xobj, iobj);
		nes_exec(N, (char *)N->readptr);
		nes_unlinkval(N, xobj);
		if (N->cnt) N->cnt=0;
		if (N->brk) { N->brk--; break; }
		if (N->ret) break;
	}
	N->readptr=be;
	if (single) --N->readptr;
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
	if (single) --N->readptr;
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
	if (single) --N->readptr;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}

void n_try(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_try()"
	char exnamebuf[MAX_OBJNAMELEN+1];
	jmp_buf *savjmp;
	uchar *bs, *be;
	obj_t *tobj;
	short single, except;

	DEBUG_IN();
	settrace();
	bs=N->readptr;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		single=0;
	} else {
		n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		single=1;
	}
	N->readptr++;
	be=N->readptr;
	N->readptr=bs;
	savjmp=N->savjmp;
	N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
	if (setjmp(*N->savjmp)==0) {
		nes_exec(N, (char *)N->readptr);
		except=0;
	} else {
		except=1;
	}
	n_free(N, (void *)&N->savjmp);
	N->savjmp=savjmp;
	N->readptr=be;
	if (*N->readptr==OP_KCATCH) {
		++N->readptr;
		n_expect(N, __FUNCTION__, OP_POPAREN);
		++N->readptr;
		n_expect(N, __FUNCTION__, OP_LABEL);
		n_getlabel(N, exnamebuf);
		n_expect(N, __FUNCTION__, OP_PCPAREN);
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
		if (except) {
			N->readptr=bs;
			tobj=nes_settable(N, &N->l, exnamebuf);
			/* needing setvaltype is a bug */
			nes_setvaltype(N, tobj, NT_TABLE);
			nes_setnum(N, tobj, "errno", N->err);
			nes_setstr(N, tobj, "errtext", N->errbuf, -1);
			N->err=0;
			savjmp=N->savjmp;
			N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
			if (setjmp(*N->savjmp)==0) {
				nes_exec(N, (char *)N->readptr);
				except=0;
			} else {
				except=1;
			}
			n_free(N, (void *)&N->savjmp);
			N->savjmp=savjmp;
			n_freeval(N, nes_getobj(N, &N->l, exnamebuf));
		}
		N->readptr=be;
	}
	if (*N->readptr==OP_KFINALLY) {
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
		N->readptr=bs;
		nes_exec(N, (char *)N->readptr);
		N->readptr=be;
	}
	if (except) {
		char errbuf[sizeof(N->errbuf)];
		nc_strncpy(errbuf, N->errbuf, sizeof(N->errbuf)-1);
		n_error(N, NE_SYNTAX, NULL, "%s", errbuf);
	}
	if (single) --N->readptr;
	DEBUG_OUT();
	return;
#undef __FUNCTION__
}
