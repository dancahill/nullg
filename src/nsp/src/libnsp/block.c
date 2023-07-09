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
#ifndef _NSPLIB_H
#include "nsp/nsplib.h"
#endif
#include "opcodes.h"

/* Advance readptr to next specified char */
void n_skipto(nsp_state *N, const char *fn, unsigned short c)
{
#define __FN__ __FILE__ ":n_skipto()"
	settrace();
	while (*n_context_readptr) {
		if (*n_context_readptr == OP_LINENUM) n_skip_ws(N);
		if (*n_context_readptr == c) return;
		n_context_readptr = n_seekop(N, n_context_readptr, 1);
	}
	n_error(N, NE_SYNTAX, fn, "expected a '%s'", n_getsym(N, c));
	return;
#undef __FN__
}

void n_if(nsp_state *N)
{
#define __FN__ __FILE__ ":n_if()"
	char done = 0, t = 0;
	uchar *ce;

	DEBUG_IN();
	settrace();
l1:
	if (n_peekop(N) == OP_POPAREN) {
		ce = n_context_readptr + readi2((n_context_readptr + 1)) + 3;
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		if (!done) {
			t = (char)nsp_tobool(N, nsp_eval(N, (char *)n_context_readptr));
			if (N->r.val->refs > 1) nsp_unlinkval(N, &N->r);
		}
		n_context_readptr = ce;
		n_expect(N, __FN__, OP_PCPAREN);
		n_context_readptr++;
	}
l2:
	if (!t || done) {
		if (n_peekop(N) == OP_POBRACE) {
			n_context_readptr += 5 + readi4((n_context_readptr + 1));
			n_expect(N, __FN__, OP_PCBRACE);
			n_context_readptr++;
		} else {
			n_skipto(N, __FN__, OP_PSEMICOL);
			if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
		}
	} else {
		done = 1;
		if (n_peekop(N) == OP_POBRACE) {
			uchar *be = n_context_readptr + 5 + readi4((n_context_readptr + 1));
			nsp_exec(N, (char *)n_context_readptr);
			n_context_readptr = be;
			n_expect(N, __FN__, OP_PCBRACE);
			n_context_readptr++;
		} else {
			N->single = 1;
			nsp_exec(N, (char *)n_context_readptr);
			if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
		}
	}
	if (!N->ret && n_peekop(N) == OP_KELSE) {
		n_context_readptr++;
		if (n_peekop(N) == OP_KIF) {
			n_context_readptr++;
			goto l1;
		}
		t = 1;
		goto l2;
	}
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_for(nsp_state *N)
{
#define __FN__ __FILE__ ":n_for()"
	uchar *arginit, *argcomp, *argexec;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	n_expect(N, __FN__, OP_POPAREN);
	arginit = n_context_readptr = n_seekop(N, n_context_readptr, 0);
	n_skipto(N, __FN__, OP_PSEMICOL);
	argcomp = ++n_context_readptr;
	n_skipto(N, __FN__, OP_PSEMICOL);
	argexec = ++n_context_readptr;
	n_skipto(N, __FN__, OP_PCPAREN);
	bs = ++n_context_readptr;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr += 5 + readi4((n_context_readptr + 1));
		n_expect(N, __FN__, OP_PCBRACE);
		single = 0;
	} else {
		n_skipto(N, __FN__, OP_PSEMICOL);
		single = 1;
	}
	be = ++n_context_readptr;
	n_context_readptr = arginit;
	n_readvar(N, &N->context->l, NULL);
	for (;;) {
		if (argcomp[0] != OP_PSEMICOL) {
			cobj = nsp_eval(N, (char *)argcomp);
			n_expect(N, __FN__, OP_PSEMICOL);
			n_context_readptr++;
			if (nsp_tonum(N, cobj) == 0) break;
		}
		n_context_readptr = bs;
		N->single = single;
		nsp_exec(N, (char *)n_context_readptr);
		if (N->cnt) N->cnt = 0;
		if (N->brk) { N->brk--; break; }
		if (N->ret) break;
		n_context_readptr = argexec;
		n_readvar(N, &N->context->l, NULL);
	}
	n_context_readptr = be;
	if (single) --n_context_readptr;
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_foreach(nsp_state *N)
{
#define __FN__ __FILE__ ":n_foreach()"
//	char itemnamebuf1[MAX_OBJNAMELEN+1];
//	char itemnamebuf2[MAX_OBJNAMELEN+1];
	obj_t *iobj, *sobj, *xobj;
	obj_t tobj;
	obj_t *tsobj;
	uchar *bs, *be;
	short single;
	char *namep;
	char *valp;

	DEBUG_IN();
	settrace();
	n_expect(N, __FN__, OP_POPAREN);
	n_context_readptr = n_seekop(N, n_context_readptr, 0);
//	++N->readptr;
	n_expect(N, __FN__, OP_LABEL);
//	n_getlabel(N, itemnamebuf1);
	namep = NULL;
	valp = n_getlabel(N, NULL);
	if (n_peekop(N) == OP_PCOMMA) {
		++n_context_readptr;
		n_expect(N, __FN__, OP_LABEL);
//		n_getlabel(N, itemnamebuf2);
		namep = valp;
		valp = n_getlabel(N, NULL);
//		namep=itemnamebuf1;
//		valp=itemnamebuf2;
//	} else {
//		namep=NULL;
//		valp=itemnamebuf1;
	}
	n_expect(N, __FN__, OP_LABEL);
	if (nc_strcmp(n_getlabel(N, NULL), "in") != 0) {
		n_error(N, NE_INTERNAL, __FN__, "expected 'in'");
	}

	sobj = nsp_eval(N, (char *)n_context_readptr);
	if (!nsp_istable(sobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for set obj");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, sobj);
	sobj = &tobj;
	tsobj = sobj->val->d.table.f;

	n_expect(N, __FN__, OP_PCPAREN);
	bs = ++n_context_readptr;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		n_skipto(N, __FN__, OP_PCBRACE);
		single = 0;
	} else {
		n_skipto(N, __FN__, OP_PSEMICOL);
		single = 1;
	}
	be = ++n_context_readptr;
	for (iobj = sobj->val->d.table.f; iobj; iobj = iobj->next) {
		if (nsp_isnull(iobj) || iobj->val->attr & NST_SYSTEM) continue;
		if (namep) nsp_setstr(N, &N->context->l, namep, iobj->name, -1);
		n_context_readptr = bs;
		N->single = single;
		xobj = nsp_setnum(N, &N->context->l, valp, 0);
		nsp_linkval(N, xobj, iobj);
		nsp_exec(N, (char *)n_context_readptr);
		nsp_unlinkval(N, xobj);
		if (sobj->val->type != NT_TABLE) {
			nsp_unlinkval(N, &tobj);
			n_error(N, NE_SYNTAX, __FN__, "expected a table for set obj");
			break;
		} else 	if (sobj->val->d.table.f != tsobj) {
			n_warn(N, __FN__, "sobj->val!=sval");
		}

		if (N->cnt) N->cnt = 0;
		if (N->brk) { N->brk--; break; }
		if (N->ret) break;
	}
	nsp_unlinkval(N, &tobj);
	n_context_readptr = be;
	if (single) --n_context_readptr;
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_switch(nsp_state *N)
{
#define __FN__ __FILE__ ":n_switch()"
	obj_t cobj, tobj;
	uchar *bs, *be;
	char match = 0;

	DEBUG_IN();
	settrace();
	n_expect(N, __FN__, OP_POPAREN);
	n_context_readptr = n_seekop(N, n_context_readptr, 0);
//	N->readptr++;
	nc_memset((void *)&cobj, 0, sizeof(obj_t));
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, nsp_eval(N, (char *)n_context_readptr));
	n_expect(N, __FN__, OP_PCPAREN);
	n_context_readptr++;
	n_expect(N, __FN__, OP_POBRACE);
	bs = n_context_readptr = n_seekop(N, n_context_readptr, 0);
	n_skipto(N, __FN__, OP_PCBRACE);
	be = ++n_context_readptr;
	n_context_readptr = bs;
	while (*n_context_readptr != OP_PCBRACE) {
		switch (*n_context_readptr) {
		case OP_KCASE:
			n_context_readptr++;
//			switch (nsp_typeof((&tobj))) {
			switch (tobj.val == NULL ? NT_NULL : tobj.val->type) {
			case NT_STRING:
				if (*n_context_readptr != OP_STRDATA) {
					n_error(N, NE_SYNTAX, __FN__, "invalid comparison");
				}
				n_getstring(N, &cobj);
				if (nc_strcmp(tobj.val->d.str, cobj.val->d.str) == 0) match = 1;
				nsp_unlinkval(N, &cobj);
				n_expect(N, __FN__, OP_MCOLON);
				n_context_readptr++;
				break;
			case NT_NUMBER:
				if (*n_context_readptr != OP_NUMDATA) {
					n_error(N, NE_SYNTAX, __FN__, "invalid comparison");
				}
				nsp_setnum(N, &cobj, NULL, n_getnumber(N));
				if (tobj.val->d.num == cobj.val->d.num) match = 1;
				nsp_unlinkval(N, &cobj);
				n_expect(N, __FN__, OP_MCOLON);
				n_context_readptr++;
				break;
			default:
				n_error(N, NE_SYNTAX, __FN__, "invalid switch type");
				break;
			}
			if (!match) {
				while (*n_context_readptr != OP_KCASE && *n_context_readptr != OP_KDEFAULT && *n_context_readptr != OP_PCBRACE) {
					n_context_readptr = n_seekop(N, n_context_readptr, 1);
				}
				continue;
			}
			while (*n_context_readptr != OP_KCASE && *n_context_readptr != OP_KDEFAULT && *n_context_readptr != OP_PCBRACE) {
				N->single = 1;
				nsp_exec(N, (char *)n_context_readptr);
				if (N->brk) break;
				if (N->ret) break;
			}
			if (N->brk) { N->brk--; goto _done; }
			if (N->ret) goto _done;
			break;
		case OP_KDEFAULT:
			n_context_readptr++;
			n_expect(N, __FN__, OP_MCOLON);
			n_context_readptr++;
			while (*n_context_readptr != OP_PCBRACE) {
				N->single = 1;
				nsp_exec(N, (char *)n_context_readptr);
				if (N->brk) break;
				if (N->ret) break;
			}
			if (N->brk) { N->brk--; goto _done; }
			if (N->ret) goto _done;
			break;
		default:
			n_expect(N, __FN__, OP_KCASE);
			goto _done;
		}
	}
_done:
	n_context_readptr = be;
	nsp_unlinkval(N, &tobj);
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_do(nsp_state *N)
{
#define __FN__ __FILE__ ":n_do()"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	bs = n_context_readptr;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		n_skipto(N, __FN__, OP_PCBRACE);
		n_context_readptr++;
		single = 0;
	} else {
		n_skipto(N, __FN__, OP_PSEMICOL);
		n_context_readptr++;
		single = 1;
	}
	be = n_context_readptr;
	for (;;) {
		n_context_readptr = bs;
		N->single = single;
		nsp_exec(N, (char *)n_context_readptr);
		if (N->cnt > 0) N->cnt = 0;
		if (N->brk > 0) { N->brk--; break; }
		if (N->ret) break;
		n_context_readptr = be;
		n_expect(N, __FN__, OP_KWHILE);
		n_context_readptr++;
		n_expect(N, __FN__, OP_POPAREN);
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
//		N->readptr++;
		argcomp = n_context_readptr;
		cobj = nsp_eval(N, (char *)argcomp);
		n_expect(N, __FN__, OP_PCPAREN);
		if (nsp_tonum(N, cobj) == 0) break;
	}
	n_context_readptr = be;

//	n_decompile(N, N->readptr, be, NULL, 0);
	n_expect(N, __FN__, OP_KWHILE);
	n_skipto(N, __FN__, OP_PSEMICOL);
	n_expect(N, __FN__, OP_PSEMICOL);

	if (single) --n_context_readptr;
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_while(nsp_state *N)
{
#define __FN__ __FILE__ ":n_while()"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short single;

	DEBUG_IN();
	settrace();
	n_expect(N, __FN__, OP_POPAREN);
	n_context_readptr = n_seekop(N, n_context_readptr, 0);
	argcomp = n_context_readptr;
	n_skipto(N, __FN__, OP_PCPAREN);
	bs = ++n_context_readptr;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		n_skipto(N, __FN__, OP_PCBRACE);
		n_context_readptr++;
		single = 0;
	} else {
		n_skipto(N, __FN__, OP_PSEMICOL);
		n_context_readptr++;
		single = 1;
	}
	be = n_context_readptr;
	for (;;) {
		cobj = nsp_eval(N, (char *)argcomp);
		n_expect(N, __FN__, OP_PCPAREN);
		n_context_readptr++;
		if (nsp_tonum(N, cobj) == 0) break;
		n_context_readptr = bs;
		N->single = single;
		nsp_exec(N, (char *)n_context_readptr);
		if (N->cnt > 0) N->cnt = 0;
		if (N->brk > 0) { N->brk--; break; }
		if (N->ret) break;
	}
	n_context_readptr = be;
	if (single) --n_context_readptr;
	DEBUG_OUT();
	return;
#undef __FN__
}

void n_try(nsp_state *N)
{
#define __FN__ __FILE__ ":n_try()"
	char exnamebuf[MAX_OBJNAMELEN + 1];
	jmp_buf *savjmp;
	uchar *bs, *be;
	obj_t *tobj;
	short single, except;

	DEBUG_IN();
	settrace();
	bs = n_context_readptr;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		n_skipto(N, __FN__, OP_PCBRACE);
		single = 0;
	} else {
		n_skipto(N, __FN__, OP_PSEMICOL);
		single = 1;
	}
	n_context_readptr++;
	be = n_context_readptr;
	n_context_readptr = bs;
	savjmp = n_context_savjmp;
	n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
	if (setjmp(*n_context_savjmp) == 0) {
		nsp_exec(N, (char *)n_context_readptr);
		except = 0;
	} else {
		except = 1;
	}
	n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
	n_context_savjmp = savjmp;
	n_context_readptr = be;
	if (n_peekop(N) == OP_KCATCH) {
		++n_context_readptr;
		n_expect(N, __FN__, OP_POPAREN);
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
//		++N->readptr;
		n_expect(N, __FN__, OP_LABEL);
		n_getlabel(N, exnamebuf);
		n_expect(N, __FN__, OP_PCPAREN);
		bs = ++n_context_readptr;
		if (n_peekop(N) == OP_POBRACE) {
			n_context_readptr = n_seekop(N, n_context_readptr, 0);
			n_skipto(N, __FN__, OP_PCBRACE);
			single = 0;
		} else {
			n_skipto(N, __FN__, OP_PSEMICOL);
			single = 1;
		}
		be = ++n_context_readptr;
		if (except) {
			n_context_readptr = bs;
			tobj = nsp_settable(N, &N->context->l, exnamebuf);
			/* needing setvaltype is a bug */
			nsp_setvaltype(N, tobj, NT_TABLE);
			nsp_setnum(N, tobj, "errno", N->err);
			nsp_setstr(N, tobj, "errtext", N->errbuf, -1); // this should be deprecated
			nsp_setstr(N, tobj, "description", N->errbuf, -1);
			N->err = 0;
			savjmp = n_context_savjmp;
			n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
			if (setjmp(*n_context_savjmp) == 0) {
				nsp_exec(N, (char *)n_context_readptr);
				except = 0;
			} else {
				except = 1;
			}
			n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
			n_context_savjmp = savjmp;
			n_freeval(N, nsp_getobj(N, &N->context->l, exnamebuf));
		}
		n_context_readptr = be;
	}
	if (n_peekop(N) == OP_KFINALLY) {
		bs = ++n_context_readptr;
		if (n_peekop(N) == OP_POBRACE) {
			n_context_readptr = n_seekop(N, n_context_readptr, 0);
			n_skipto(N, __FN__, OP_PCBRACE);
			single = 0;
		} else {
			n_skipto(N, __FN__, OP_PSEMICOL);
			single = 1;
		}
		be = ++n_context_readptr;
		n_context_readptr = bs;
		nsp_exec(N, (char *)n_context_readptr);
		n_context_readptr = be;
	}
	if (except) {
		char errbuf[sizeof(N->errbuf)];
		nc_strncpy(errbuf, N->errbuf, sizeof(N->errbuf) - 1);
		n_error(N, NE_SYNTAX, NULL, "%s", errbuf);
	}
	if (single) --n_context_readptr;
	DEBUG_OUT();
	return;
#undef __FN__
}
