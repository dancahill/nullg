/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2019 Dan Cahill

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

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
#include <Shlobj.h>
#elif !defined( __TURBOC__)
#include <unistd.h>
#endif

    /*
     * generate a value for global _ostype_ var
     * ostype/compilertype
     */
#if defined(WIN32) || defined(WINDOWS)
#  define _OS_ "Windows"
#elif defined(linux)
#  define _OS_ "Linux"
#elif defined(__FreeBSD__)
#  define _OS_ "FreeBSD"
#elif defined(__NetBSD__)
#  define _OS_ "NetBSD"
#elif defined(__OpenBSD__)
#  define _OS_ "OpenBSD"
#elif defined(SOLARIS)
#  define _OS_ "Solaris"
#else
#  define _OS_ "unknown"
#endif
#if defined(__GNUC__)
#  if defined(__CYGWIN__)
#    undef _OS_
#    define _OS_ "Windows"
#    define _COMPILER_ "gcc (cygwin)"
#  else
#    define _COMPILER_ "gcc"
#  endif
#elif defined(_MSC_VER)
#  define _COMPILER_ "msvc"
#elif defined( __BORLANDC__)
#  define _COMPILER_ "borlandc"
#elif defined( __TURBOC__)
#  undef _OS_
#  define _OS_ "DOS"
#  define _COMPILER_ "turboc"
#else
#  define _COMPILER_ "unknown"
#endif
#define _OSTYPE_ _OS_"/"_COMPILER_

nsp_execcontext *n_newexeccontext(nsp_state *N)
{
	nsp_execcontext *newcontext = n_alloc(N, sizeof(nsp_execcontext), 1);
	//newcontext->savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);

	if (N && N->context) {
		newcontext->blockptr = N->context->blockptr;
		newcontext->blockend = N->context->blockend;
		newcontext->readptr = N->context->readptr;
		newcontext->funcname = N->context->funcname;
		newcontext->filename = N->context->filename;
		newcontext->linenum = N->context->linenum;
	}

	nsp_setvaltype(N, &newcontext->l, NT_TABLE);
	newcontext->l.val->attr |= NST_AUTOSORT;
	n_setname(N, &newcontext->l, "!LOCALS!");

	return newcontext;
}

void n_freeexeccontext(nsp_state *N, nsp_execcontext **context)
{
#define __FN__ __FILE__ ":n_freeexeccontext()"
	if (*context) {
		nsp_execcontext *ctx = *context;

		//obj_t *thisobj = nsp_getobj(N, &ctx->l, "this");
		//if (thisobj) nsp_unlinkval(N, thisobj);

		nsp_unlinkval(N, &ctx->t);
		nsp_unlinkval(N, &ctx->l);
		//nsp_freetable(N, &(*context)->l);
		//if ((*context)->l.val) n_free(N, (void *)&(*context)->l.val, sizeof(val_t));
		n_free(N, (void *)context, sizeof(nsp_execcontext));
	}
#undef __FN__
}

static void n_execfunction_setargs(nsp_state *N, obj_t *listobj)
{
#define __FN__ __FILE__ ":n_execfunction_setargs()"
	obj_t *cobj;
	int i;

	if (n_peekop(N) == OP_POPAREN) {
		for (i = 1;; i++) {
			n_context_readptr = n_seekop(N, n_context_readptr, 0);
			if (n_peekop(N) == OP_PCPAREN) break;
			cobj = nsp_appendobj(N, listobj, NULL);
			n_setnamei(N, cobj, i);
			if (n_peekop(N) == OP_MAND) {
				n_context_readptr++;
				n_expect(N, __FN__, OP_LABEL);
				nsp_linkval(N, cobj, nsp_getobj(N, NULL, n_getlabel(N, NULL)));
			}
			else {
				nsp_linkval(N, cobj, nsp_eval(N, (char *)n_context_readptr));
			}
			if (n_peekop(N) == OP_PCOMMA) continue;
			n_expect(N, __FN__, OP_PCPAREN);
			break;
		}
	}
	n_context_readptr++;
#undef __FN__
}

// take function parameter names and map them to the arguments we've collected
static void n_execfunction_setargnames(nsp_state *N, obj_t *fobj)
{
#define __FN__ __FILE__ ":n_execfunction_getargs()"
	char filenamebuf[MAX_OBJNAMELEN + 1];
	unsigned short i;
	obj_t *cobj;

	N->context->blockptr = (uchar *)fobj->val->d.str;
	N->context->blockend = (uchar *)fobj->val->d.str;
	N->context->readptr = (uchar *)fobj->val->d.str;
	/* get the name of the source file */
	n_expect(N, __FN__, OP_LABEL);
	filenamebuf[0] = 0;
	n_getlabel(N, filenamebuf);
	/* get the arg names */
	n_expect(N, __FN__, OP_POPAREN);
	for (i = 1;; i++) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		cobj = nsp_getobj(N, &N->context->l, n_ntoa(N, N->numbuf, i, 10, 0));
		if (n_peekop(N) == OP_LABEL) n_setname(N, cobj, n_getlabel(N, NULL));
		if (n_peekop(N) == OP_PCOMMA) continue;
		n_expect(N, __FN__, OP_PCPAREN);
		n_context_readptr++;
		break;
	}
	n_expect(N, __FN__, OP_POBRACE);
#undef __FN__
}

/// <summary>
/// execute c or script function
/// </summary>
/// <param name="N">NSP global struct</param>
/// <param name="fobj">function object (c or script)</param>
/// <param name="pobj">parent object/table containing fobj</param>
/// <param name="ftype">type of function (function, classmethod, coroutine)</param>
obj_t *n_execfunction(nsp_state *N, obj_t *fobj, obj_t *pobj, enum n_execfunctiontype ftype)
{
#define __FN__ __FILE__ ":n_execfunction()"
	//char *oldfunc;
	jmp_buf *savjmp;
	obj_t listobj, *cobj;
	obj_t *thisobj = NULL;
	unsigned short fobjtype;
	int e = 0;
	short include = 0;
	//short noscopechange = 0;
	uchar *p = NULL;
	int psize = 0;
	int n = 0;

	DEBUG_IN();
	settrace();
	fobjtype = nsp_typeof(fobj);
	if (fobjtype != NT_CFUNC && fobjtype != NT_NFUNC) {
		n_error(N, NE_SYNTAX, __FN__, "'%s' is not a function", fobj->name);
	}
	listobj.val = n_newval(N, NT_TABLE);
	/* disable autosort or 'this' will be hard to find... */
	listobj.val->attr &= ~NST_AUTOSORT;
	/* set this */
	if (pobj || ftype == constructor) {
		cobj = nsp_appendobj(N, &listobj, "this");
		thisobj = pobj ? pobj : &listobj;
		nsp_linkval(N, cobj, thisobj);
		//if (!pobj && cobj->val) cobj->val->attr |= NST_HIDDEN;
	}
	/* set fn name */
	cobj = nsp_appendobj(N, &listobj, "0");
	nsp_setstr(N, cobj, "0", fobj->name, -1);

	if (fobjtype == NT_CFUNC && (NSP_CFUNC)(fobj->val->d.cfunc) == (NSP_CFUNC)nl_coroutine_constructor) {
		short fnfound = 0;

		//n_warn(N, __FN__, "n_execfunction coroutine constructor");
		n_expect(N, __FN__, OP_POPAREN);
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		if (n_peekop(N) == OP_LABEL) {
			char *l = n_getlabel(N, NULL);
			obj_t *nobj = nsp_getobj(N, NULL, l);
			if ((nobj->val->type == NT_NFUNC || nobj->val->type == NT_CFUNC) && *n_context_readptr == OP_POPAREN) {
				fobj = nobj;
				fobjtype = nsp_typeof(fobj);
				fnfound = 1;
			}
		}
		if (!fnfound) n_error(N, NE_SYNTAX, __FN__, "expected a function as coroutine parameter");
		//n_warn(N, __FN__, "n_execfunction found a function in parameters - good");
		nsp_execcontext *oldctx = N->context;
		nsp_execcontext *newctx = n_newexeccontext(N);
		N->context = newctx;
		nsp_linkval(N, &N->context->l, &listobj);
		nl_coroutine_constructor(N);

		n_execfunction_setargs(N, &listobj);
		if (fobjtype == NT_NFUNC) n_execfunction_setargnames(N, fobj);
		nsp_unlinkval(N, &N->r);

		//n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "coroutine constructor: &N->context->l='%s'", N->r.val->d.str);
		//nsp_unlinkval(N, &N->r);

		N->context->funcname = fobj->name;
		if (thisobj) {
			//nsp_linkval(N, nsp_appendobj(N, &newctx->l, "this"), thisobj);
			nsp_linkval(N, &N->context->t, thisobj);
		}
		nsp_unlinkval(N, &listobj);
		N->context = oldctx;
		n_expect(N, __FN__, OP_POPAREN);
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		n_skipto(N, __FN__, OP_PCPAREN);
		n_expect(N, __FN__, OP_PCPAREN);
		n_context_readptr++;
		n_expect(N, __FN__, OP_PCPAREN);
		n_context_readptr++;
		//n_warn(N, __FN__, "n_execfunction coroutine done");
		DEBUG_OUT();
		return &N->r;
	}

	/* set args */
	n_execfunction_setargs(N, &listobj);
	nsp_unlinkval(N, &N->r);

	if (fobjtype == NT_CFUNC && (NSP_CFUNC)(fobj->val->d.cfunc) == (NSP_CFUNC)nl_include) {
		if (nsp_isstr(nsp_getobj(N, &listobj, "2"))) {
			include = 2;
		}
		else if (nsp_isstr(nsp_getobj(N, &listobj, "1"))) {
			include = 1;
		}
		else {
			DEBUG_OUT();
			return &N->r;
		}
	}
	nsp_execcontext *oldctx = N->context;
	N->context = n_newexeccontext(N);
	if (include == 2) {
		obj_t *cobj2 = nsp_getobj(N, &listobj, "2");
		n_decompose(N, NULL, (uchar *)cobj2->val->d.str, &p, &psize);
		if (p) {
			N->context->blockptr = p;
			N->context->blockend = p + readi4((p + 8));
			N->context->readptr = p + readi4((p + 12));
		}
	}
	/* separate local var setup to prevent include() from creating its own local context */
	if (include) {
		N->context->funcname = "";
		if (include == 1) N->context->filename = nsp_getstr(N, &listobj, "1");
		nsp_linkval(N, &N->context->l, &oldctx->l);
	}
	else {
		N->context->funcname = fobj->name;
		nsp_linkval(N, &N->context->l, &listobj);
		if (fobjtype == NT_NFUNC) n_execfunction_setargnames(N, fobj);
	}
	savjmp = n_context_savjmp;
	n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
	if ((e = setjmp(*n_context_savjmp)) == 0) {
		if (include == 2) nsp_linkval(N, &N->r, nsp_exec(N, (char *)n_context_readptr));
		else if (include == 1) n = nsp_execfile(N, N->context->filename);
		else if (fobjtype == NT_CFUNC) fobj->val->d.cfunc(N);
		else nsp_exec(N, (char *)n_context_readptr);
	}
	n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
	n_context_savjmp = savjmp;
	if (include) {
		if (n < 0) n_warn(N, __FN__, "failed to include '%s'", N->context->filename);
		nsp_setbool(N, &N->r, "", n ? 0 : 1);
	}
	else {
		if (ftype == constructor) nsp_linkval(N, &N->r, &N->context->l);
	}
	n_freeexeccontext(N, &N->context);
	N->context = oldctx;
	if (include == 2 && p) n_free(N, (void *)&p, psize);
	nsp_unlinkval(N, &listobj);
	if (N->ret) N->ret = 0;
	if (e && n_context_savjmp != NULL) longjmp(*n_context_savjmp, 1);
	DEBUG_OUT();
	return &N->r;
#undef __FN__
}

/* base class method hack */
obj_t *n_execbasemethod(nsp_state *N, char *name, obj_t *pobj)
{
#define __FN__ __FILE__ ":n_execbasemethod()"
	obj_t *nobj;
	obj_t tobj;
	jmp_buf *savjmp;
	int e;

	n_expect(N, __FN__, OP_POPAREN);
	//if (N->yielded) {
	//	int x = 42;
	//}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_setcfunc(N, &tobj, "base_method", (NSP_CFUNC)nl_base_method);
	nc_strncpy(tobj.name, name, MAX_OBJNAMELEN);
	savjmp = n_context_savjmp;
	n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
	if ((e = setjmp(*n_context_savjmp)) == 0) {
		nobj = n_execfunction(N, &tobj, pobj, function);
	}
	else {
		nobj = NULL;
	}
	n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
	n_context_savjmp = savjmp;
	nsp_unlinkval(N, &tobj);
	if (e) {
		char errbuf[sizeof(N->errbuf)];
		nc_strncpy(errbuf, N->errbuf, sizeof(N->errbuf) - 1);
		n_error(N, NE_SYNTAX, NULL, "%s", errbuf);
	}
	return nobj;
#undef __FN__
}

void n_execconstructor(nsp_state *N, obj_t *cobj, obj_t *pobj)
{
#define __FN__ __FILE__ ":n_execconstrutor()"
	obj_t *xobj;

	nsp_setvaltype(N, cobj, NT_TABLE);
	nsp_zlink(N, cobj, pobj);
	xobj = nsp_getobj(N, cobj, pobj->name);
	if (!nsp_isnull(xobj)) {
		n_execfunction(N, xobj, cobj, constructor);
	}
	else {
		xobj = nsp_getobj(N, cobj, "_constructor");
		if (!nsp_isnull(xobj)) {
			n_execfunction(N, xobj, cobj, constructor);
		}
		else if (n_peekop(N) == OP_POPAREN) {
			n_context_readptr += readi2((n_context_readptr + 1)) + 3;
			n_expect(N, __FN__, OP_PCPAREN);
			n_context_readptr++;
		}
	}
#undef __FN__
}

void n_execdestructor(nsp_state *N, obj_t *cobj, char *cname)
{
#define __FN__ __FILE__ ":n_execdestrutor()"
	obj_t *xobj;

	xobj = nsp_getobj(N, cobj, "_destructor");
	if (!nsp_isnull(xobj)) n_execfunction(N, xobj, cobj, function);
#undef __FN__
}


/*
 * the following functions are public API functions
 */

obj_t *nsp_exec(nsp_state *N, const char *string)
{
#define __FN__ __FILE__ ":nsp_exec()"
	obj_t *cobj, *tobj;
	uchar block, ctype, op;
	uchar jmp = n_context_savjmp ? 1 : 0, single = (uchar)N->single;
	uchar *p;
	int psize;

	DEBUG_IN();
	settrace();
	N->single = 0;

	if (N->yielded) {
		//n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "nsp_exec coroutine locals='%s'", N->r.val->d.str);
		//n_warn(N, __FN__, "yielded coroutine resuming");
		return NULL;
	}
	else if (jmp == 0) {
		nsp_unlinkval(N, &N->r);
		if (string == NULL || string[0] == 0) goto end;
		n_decompose(N, NULL, (uchar *)string, &p, &psize);
		if (p) n_context_blockptr = p;
		n_context_blockend = n_context_blockptr + readi4((n_context_blockptr + 8));
		n_context_readptr = n_context_blockptr + readi4((n_context_blockptr + 12));
		n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*n_context_savjmp) != 0) goto end;
	}
	else {
		n_context_readptr = (uchar *)string;
	}
	if (n_context_readptr == NULL) goto end;
	if (n_peekop(N) == OP_POBRACE) {
		n_context_readptr = n_seekop(N, n_context_readptr, 0);
		block = 1;
	}
	else block = 0;
	while (n_peekop(N)) {
		if (N->signal) {
			n_error(N, NE_INTERNAL, __FN__, "killed by external request");
		}
		if (block && (N->brk > 0 || N->cnt > 0 || n_peekop(N) == OP_PCBRACE)) goto end;
		if (n_peekop(N) == OP_PCBRACE) {
			n_context_readptr++;
			goto endstmt;
		}
		else if (OP_ISMATH(*n_context_readptr)) {
			op = *n_context_readptr++;
			switch (op) {
			case OP_MQUESTION:
				n_storeval(N, &N->r);
				if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
				nc_printf(N, "%s", nsp_tostr(N, &N->r));
				goto endstmt;
			default:
				n_warn(N, __FN__, "unexpected math op '%s'", n_getsym(N, *n_context_readptr));
				n_context_readptr++;
			}
		}
		else if (OP_ISPUNC(*n_context_readptr)) {
			n_warn(N, __FN__, "unexpected punctuation '%s'", n_getsym(N, *n_context_readptr));
			n_context_readptr++;
		}
		else if (OP_ISKEY(*n_context_readptr)) {
			op = *n_context_readptr++;
			switch (op) {
			case OP_KLOCAL:
			case OP_KVAR:     n_readvar(N, &N->context->l, NULL); goto endstmt;
			case OP_KGLOB:    n_readvar(N, &N->g, NULL); goto endstmt;
			case OP_KNAMESPACE: {
				char namebuf[MAX_OBJNAMELEN + 1];
				obj_t *cobj, *tobj = &N->g;

				do {
					namebuf[0] = '\0';
					n_expect(N, __FN__, OP_LABEL);
					n_getlabel(N, namebuf);
					cobj = nsp_getobj(N, tobj, namebuf);
					if (!nsp_istable(cobj) != NT_TABLE) {
						cobj = nsp_settable(N, tobj, namebuf);
					}
					tobj = cobj;
					if (n_peekop(N) != OP_PDOT) break;
					n_context_readptr++;
				} while (1);
				n_expect(N, __FN__, OP_POBRACE);
				n_readtable(N, tobj);
				n_expect(N, __FN__, OP_PCBRACE);
				n_context_readptr++;
				goto endstmt;
			}
			case OP_KIF:      n_if(N);      if (N->ret) goto end; else goto endstmt;
			case OP_KFOR:     n_for(N);     if (N->ret) goto end; else goto endstmt;
			case OP_KFOREACH: n_foreach(N); if (N->ret) goto end; else goto endstmt;
			case OP_KSWITCH:  n_switch(N);  if (N->ret) goto end; else goto endstmt;
			case OP_KDO:      n_do(N);      if (N->ret) goto end; else goto endstmt;
			case OP_KWHILE:   n_while(N);   if (N->ret) goto end; else goto endstmt;
			case OP_KTRY:     n_try(N);     if (N->ret) goto end; else goto endstmt;
			case OP_KCLASS:
				n_expect(N, __FN__ "1", OP_LABEL);
				cobj = nsp_settable(N, &N->g, n_getlabel(N, NULL));
				n_expect(N, __FN__ "1", OP_POBRACE);
				n_readtable(N, cobj);
				n_expect(N, __FN__, OP_PCBRACE);
				n_context_readptr++;

				goto endstmt;
			case OP_KFUNC:
				n_expect(N, __FN__, OP_LABEL);
				cobj = nsp_setnfunc(N, &N->g, n_getlabel(N, NULL), NULL, 0);
				n_getfunction(N, cobj);
				goto endstmt;
			case OP_KBREAK:
				if ((!block) && (!single)) n_error(N, NE_SYNTAX, __FN__, "break without block");
				N->brk = (short)(n_peekop(N) == OP_NUMDATA ? n_getnumber(N) : 1);
				if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
				goto end;
			case OP_KCONT:
				if ((!block) && (!single)) n_error(N, NE_SYNTAX, __FN__, "continue without block");
				N->cnt = 1;
				if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
				goto end;
			case OP_KRET:
				n_storeval(N, &N->r);
				if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
				N->ret = 1;
				goto end;
			case OP_KTHROW:
				n_storeval(N, &N->r);
				n_error(N, NE_EXCEPTION, NULL, "%s", nsp_tostr(N, &N->r));
				if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
				goto end;
			case OP_KEXIT:
				N->err = (short)(n_peekop(N) == OP_NUMDATA ? n_getnumber(N) : 0);
				n_error(N, N->err, __FN__, "exiting normally");
			case OP_KELSE:
				n_error(N, NE_SYNTAX, __FN__, "stray else");
			case OP_KDELETE:
				n_expect(N, __FN__, OP_LABEL);
				cobj = nsp_getobj(N, NULL, n_getlabel(N, NULL));
				n_execdestructor(N, cobj, cobj->name);
				nsp_linkval(N, cobj, NULL);
				goto endstmt;
			default:
				n_warn(N, __FN__, "? %d %s", op, n_getsym(N, op));
			}
		}
		else {
			char namebuf[MAX_OBJNAMELEN + 1];
			obj_t *pobj = NULL;
			unsigned short z;
			//uchar *p = N->readptr;
			//uchar *e;

			if (n_peekop(N) != OP_LABEL) n_error(N, NE_SYNTAX, __FN__, "expected a label [%d][%s]", *n_context_readptr, n_getsym(N, *n_context_readptr));
			tobj = &N->context->l;
			n_getlabel(N, namebuf);
			cobj = nsp_getobj(N, NULL, namebuf);
		x:
			while (n_peekop(N) == OP_POBRACKET || n_peekop(N) == OP_PDOT) {
				pobj = tobj = cobj;
				cobj = n_readindex(N, tobj, namebuf, &z);
				if (namebuf[0] && (*n_context_readptr != OP_POPAREN) && (z || nsp_isnull(cobj))) {
					if (nsp_isnull(tobj)) goto endstmt;
					cobj = nsp_setnum(N, tobj, namebuf, 0);
				}
			}
			ctype = nsp_typeof(cobj);
			if (n_peekop(N) == OP_POPAREN) {
				if (ctype == NT_NFUNC || ctype == NT_CFUNC) {
					n_execfunction(N, cobj, pobj, function);

					if (N->yielded) {
						goto endstmt;
					}

					if (n_peekop(N) == OP_PDOT) goto x;
					goto endstmt;
				}
				else {
					n_execbasemethod(N, namebuf, pobj);
					if (n_peekop(N) == OP_PDOT) goto x;
					goto endstmt;
					//e = N->readptr - 1;
					//nc_memset(errbuf, 0, sizeof(errbuf));
					//n_decompile(N, p, e, errbuf, sizeof(errbuf) - 1);
					//n_error(N, NE_SYNTAX, __FN__, "'%s' is not a function", errbuf);
				}
			}
			if (ctype == NT_NULL) {
				if (namebuf[0] == 0) n_error(N, NE_SYNTAX, __FN__, "expected a label");
				cobj = nsp_setbool(N, tobj, namebuf, 0);
			}
			n_readvar(N, tobj, cobj);
		}
	endstmt:
		if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
		if (N->yielded) {
			//n_warn(N, __FN__, "nsp_exec coroutine yielding at context=0x%08x readptr=0x%08x", N->context, n_context_readptr);
			break;
		}
		if (single) break;
	}
end:
	if (jmp == 0) {
		n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
		if (p) n_free(N, (void *)&p, psize);
		n_context_blockend = NULL;
		n_context_readptr = NULL;
	}
	DEBUG_OUT();
	return NULL;
#undef __FN__
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <string.h>
int nsp_writefile(nsp_state *N, char *file, uchar *dat)
{
#define __FN__ __FILE__ ":nsp_writefile()"
	char outfile[512];
	int fd;
	int sz;

	nc_snprintf(N, outfile, sizeof(outfile), "/tmp/%s", file);
	n_warn(N, __FN__, "writing '%s' from [%s]", outfile, file);
	if ((fd = open(outfile, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) != -1) {
		sz = readi4((dat + 8));

		if (write(fd, dat, sz) != sz) {
			n_warn(N, __FN__, "write() wrote less bytes than expected");
		}
		close(fd);
		return 1;
	}
	return 0;
#undef __FN__
}

int nsp_execfile(nsp_state *N, char *file)
{
#define __FN__ __FILE__ ":nsp_execfile()"
	obj_t *cobj = nsp_getobj(N, &N->g, "_filepath");
	char buf[512];
	char *pfile;
	struct stat sb;
	uchar *p;
	int psize = 0;
	uchar *oldbptr = n_context_blockptr;
	uchar *oldbend = n_context_blockend;
	uchar *oldrptr = n_context_readptr;
	short int jmp = n_context_savjmp ? 1 : 0;
	int bl;
	int fd;
	int r;
	int rc;
	char *o;
	char *oldfname;
	char namebuf[MAX_OBJNAMELEN + 1];

	settrace();
	oldfname = N->context->filename;
	{
		char *p = file + strlen(file) - 1;

		while (*p && p > file) { if (*p == '\\' || *p == '/') { p++; break; } p--; }
		nc_strncpy(namebuf, p, MAX_OBJNAMELEN);
	}
	//	N->file = namebuf;
	//	n_warn(N, __FN__, "[%s][%s]", oldfname, namebuf);

	if (jmp == 0) {
		n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*n_context_savjmp) == 0) {
		}
		else {
			rc = 0;
			goto end1;
		}
	}
	pfile = file;
	if ((o = strrchr(pfile, '/')) == NULL) o = pfile; else o++;
	if ((stat(pfile, &sb) != 0) && (cobj->val->type == NT_STRING)) {
		nc_snprintf(N, buf, sizeof(buf), "%s/%s", cobj->val->d.str, pfile);
		if (stat(buf, &sb) != 0) { rc = -1; goto end2; }
		pfile = buf;
	}
	if ((fd = open(pfile, O_RDONLY | O_BINARY)) == -1) { rc = -1; goto end2; }
	n_context_blockptr = (uchar *)n_alloc(N, sb.st_size + 2, 0);
	p = n_context_blockptr;
	bl = sb.st_size;
	for (;;) {
		r = read(fd, p, bl);
		p += r;
		bl -= r;
		if (bl < 1) break;
	}
	close(fd);
	rc = sb.st_size;
	n_context_blockptr[sb.st_size] = '\0';
	n_decompose(N, namebuf, n_context_blockptr, &p, &psize);
	if (p) {
		/* nsp_writefile(N, o, p); */
		n_free(N, (void *)&n_context_blockptr, sb.st_size + 2);
		n_context_blockptr = p;
		p = NULL;
	}
	n_context_blockend = n_context_blockptr + readi4((n_context_blockptr + 8));
	n_context_readptr = n_context_blockptr + readi4((n_context_blockptr + 12));


#if defined(WIN32) && defined(_DEBUG)
	_RPT1(_CRT_WARN, "execing '%s'\r\n", pfile);
#endif
	N->context->filename = pfile;
	nsp_exec(N, (char *)n_context_readptr);
	if (N->outbuflen) nl_flush(N);
	rc = 0;
end1:
	n_free(N, (void *)&n_context_blockptr, psize);
	n_context_blockptr = oldbptr;
	n_context_blockend = oldbend;
	n_context_readptr = oldrptr;
end2:
	if (jmp == 0) {
		n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
	}
#if defined(WIN32) && defined(_DEBUG)
	_RPT1(_CRT_WARN, "done '%s'\r\n", pfile);
#endif
	N->context->filename = oldfname;
	return rc;
#undef __FN__
}

typedef struct FUNCTION {
	char *fn_name;
	NSP_CFUNC fn_ptr;
} FUNCTION;

nsp_state *nsp_newstate()
{
	FUNCTION list[] = {
		{ "copy", (NSP_CFUNC)nl_copy },
		{ "eval", (NSP_CFUNC)nl_eval },
		{ "exec", (NSP_CFUNC)nl_exec },
		{ "include", (NSP_CFUNC)nl_include },
		{ "print", (NSP_CFUNC)nl_print },
		{ "printf", (NSP_CFUNC)nl_printf },
		{ "runtime", (NSP_CFUNC)nl_runtime },
		{ "serialize", (NSP_CFUNC)nl_serialize },
		{ "sizeof", (NSP_CFUNC)nl_sizeof },
		{ "sleep", (NSP_CFUNC)nl_sleep },
		{ "sprintf", (NSP_CFUNC)nl_printf },
		{ "system", (NSP_CFUNC)nl_system },
		{ "tonumber", (NSP_CFUNC)nl_tonumber },
		{ "typeof", (NSP_CFUNC)nl_typeof },
		{ "write", (NSP_CFUNC)nl_write },
		{ NULL, NULL }
	};
	FUNCTION list_debug[] = {
		{ "break", (NSP_CFUNC)nl_break },
		{ NULL, NULL }
	};
	FUNCTION list_coroutine[] = {
		{ "coroutine", (NSP_CFUNC)nl_coroutine_constructor },
		{ "resume", (NSP_CFUNC)nl_coroutine },
		{ "yield", (NSP_CFUNC)nl_coroutine },
		{ "status", (NSP_CFUNC)nl_coroutine },
		{ NULL, NULL }
	};
	FUNCTION list_dl[] = {
		{ "load", (NSP_CFUNC)nl_dl_load },
		//{ "loadlib", (NSP_CFUNC)nl_dl_load },// deprecated
		{ NULL, NULL }
	};
	FUNCTION list_file[] = {
		{ "append", (NSP_CFUNC)nl_filewriteall },
		{ "chdir", (NSP_CFUNC)nl_filechdir },
		{ "exists", (NSP_CFUNC)nl_fileexists },
		{ "mkdir", (NSP_CFUNC)nl_filemkdir },
		{ "readall", (NSP_CFUNC)nl_filereadall },
		{ "rename", (NSP_CFUNC)nl_filerename },
		{ "stat", (NSP_CFUNC)nl_filestat },
		{ "unlink", (NSP_CFUNC)nl_fileunlink },
		{ "writeall", (NSP_CFUNC)nl_filewriteall },
		{ NULL, NULL }
	};
	FUNCTION list_io[] = {
		{ "print", (NSP_CFUNC)nl_print },
		{ "write", (NSP_CFUNC)nl_write },
		{ "flush", (NSP_CFUNC)nl_flush },
		{ NULL, NULL }
	};
	FUNCTION list_math[] = {
		{ "abs", (NSP_CFUNC)nl_math },
		{ "ceil", (NSP_CFUNC)nl_math },
		{ "floor", (NSP_CFUNC)nl_math },
		{ "rand", (NSP_CFUNC)nl_math },

		{"acos",  (NSP_CFUNC)nl_math },
		{"asin",  (NSP_CFUNC)nl_math },
		{"atan",  (NSP_CFUNC)nl_math },
		{"atan2", (NSP_CFUNC)nl_math },
		{"cos",   (NSP_CFUNC)nl_math },
		{"sin",   (NSP_CFUNC)nl_math },
		{"tan",   (NSP_CFUNC)nl_math },
		{"exp",   (NSP_CFUNC)nl_math },
		{"log",   (NSP_CFUNC)nl_math },
		{"log10", (NSP_CFUNC)nl_math },
		{"cosh",  (NSP_CFUNC)nl_math },
		{"sinh",  (NSP_CFUNC)nl_math },
		{"tanh",  (NSP_CFUNC)nl_math },
		{"sqrt",  (NSP_CFUNC)nl_math },

		{ NULL, NULL }
	};
	FUNCTION list_string[] = {
		{ "atoi", (NSP_CFUNC)nl_atoi },
		{ "itoa", (NSP_CFUNC)nl_itoa },
		{ "cat", (NSP_CFUNC)nl_strcat },
		{ "cmp", (NSP_CFUNC)nl_strcmp },
		{ "icmp", (NSP_CFUNC)nl_strcmp },
		{ "ncmp", (NSP_CFUNC)nl_strcmp },
		{ "nicmp", (NSP_CFUNC)nl_strcmp },

		{ "contains", (NSP_CFUNC)nl_strcontains },
		{ "endswith", (NSP_CFUNC)nl_strcontains },
		{ "startswith", (NSP_CFUNC)nl_strcontains },

		{ "join", (NSP_CFUNC)nl_strjoin },
		{ "len", (NSP_CFUNC)nl_strlen },
		{ "replace", (NSP_CFUNC)nl_strrep },
		{ "split", (NSP_CFUNC)nl_strsplit },
		{ "str", (NSP_CFUNC)nl_strstr },
		{ "istr", (NSP_CFUNC)nl_strstr },
		{ "sub", (NSP_CFUNC)nl_strsub },
		{ "tostring", (NSP_CFUNC)nl_tostring },
		{ "tolower", (NSP_CFUNC)nl_strtolower },
		{ "toupper", (NSP_CFUNC)nl_strtolower },

		{ "trim", (NSP_CFUNC)nl_strtrim},
		{ "trimstart", (NSP_CFUNC)nl_strtrim },
		{ "trimend", (NSP_CFUNC)nl_strtrim },

		{ NULL, NULL }
	};
	FUNCTION list_table[] = {
		{ "iname", (NSP_CFUNC)nl_iname },
		{ "ival", (NSP_CFUNC)nl_ival },
		{ "zlink", (NSP_CFUNC)nl_zlink },
		{ NULL, NULL }
	};
	FUNCTION list_time[] = {
		{ "asctime", (NSP_CFUNC)nl_asctime },
		{ "gettimeofday", (NSP_CFUNC)nl_gettimeofday },
		{ "gmtime", (NSP_CFUNC)nl_gmtime },
		{ "localtime", (NSP_CFUNC)nl_gmtime },
		{ "mktime", (NSP_CFUNC)nl_mktime },
		{ "now", (NSP_CFUNC)nl_time },
		{ "sqltime", (NSP_CFUNC)nl_asctime },
		{ NULL, NULL }
	};
	nsp_state *new_N;
	obj_t *cobj;
	short i;

	new_N = (nsp_state *)n_alloc(NULL, sizeof(nsp_state), 1);
	new_N->context = n_newexeccontext(new_N);
	new_N->outbuflen = 0;
	new_N->outbufmax = MAX_OUTBUFSIZE;
	new_N->outbuffer = (char *)n_alloc(NULL, new_N->outbufmax + 1, 1);// add one byte for null termination
	nc_gettimeofday(&new_N->ttime, NULL);
	srand(new_N->ttime.tv_usec);
	new_N->maxwarnings = 500;
#if defined(__GNUC__)
	new_N->warnformat = 'a';
#else
	new_N->warnformat = 'r';
#endif

	n_setname(new_N, &new_N->g, "!GLOBALS!");
	nsp_setvaltype(new_N, &new_N->g, NT_TABLE);
	new_N->g.val->attr |= NST_AUTOSORT;

	n_setname(new_N, &new_N->r, "!RETVAL!");
	//	cobj=nsp_settable(new_N, &new_N->g, "_GLOBALS");
	cobj = nsp_setbool(new_N, &new_N->g, "_GLOBALS", 0);
	//	cobj->val->attr|=NST_HIDDEN;
	nsp_linkval(new_N, cobj, &new_N->g);

	for (i = 0; list[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, &new_N->g, list[i].fn_name, list[i].fn_ptr);
	}

	cobj = nsp_settable(new_N, &new_N->g, "debug");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_debug[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_debug[i].fn_name, list_debug[i].fn_ptr);
	}

	cobj = nsp_settable(new_N, &new_N->g, "coroutine");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_coroutine[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_coroutine[i].fn_name, list_coroutine[i].fn_ptr);
	}

	cobj = nsp_settable(new_N, &new_N->g, "dl");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_dl[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_dl[i].fn_name, list_dl[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, cobj, "path");
#ifdef WIN32
	{
		char libbuf[MAX_PATH];

		// GetSystemWindowsDirectory(libbuf, sizeof(libbuf));
		// GetEnvironmentVariable("SystemRoot", libbuf, sizeof(libbuf));
		// GetWindowsDirectoryA(libbuf, sizeof(libbuf));
		//https://msdn.microsoft.com/en-us/library/bb762494(v=vs.85).aspx
		//CSIDL_PROGRAM_FILESX86
		SHGetSpecialFolderPathA(0, libbuf, CSIDL_PROGRAM_FILES, FALSE);
		//_snprintf(libbuf + strlen(libbuf), sizeof(libbuf) - strlen(libbuf) - 1, "\\NSP");
		nc_snprintf(new_N, libbuf + strlen(libbuf), sizeof(libbuf) - strlen(libbuf) - 1, "\\NSP\\LIB");
		nsp_setstr(new_N, cobj, "0", libbuf, -1);
	}
#else
	nsp_setstr(new_N, cobj, "0", "/usr/lib/nsp", -1);
#endif

	cobj = nsp_settable(new_N, &new_N->g, "file");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_file[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_file[i].fn_name, list_file[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, &new_N->g, "io");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_io[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_io[i].fn_name, list_io[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, &new_N->g, "math");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_math[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_math[i].fn_name, list_math[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, &new_N->g, "string");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_string[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_string[i].fn_name, list_string[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, &new_N->g, "table");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_table[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_table[i].fn_name, list_table[i].fn_ptr);
	}
	cobj = nsp_settable(new_N, &new_N->g, "time");
	cobj->val->attr |= NST_HIDDEN;
	for (i = 0; list_time[i].fn_name != NULL; i++) {
		nsp_setcfunc(new_N, cobj, list_time[i].fn_name, list_time[i].fn_ptr);
	}
	cobj = nsp_setnum(new_N, &new_N->g, "null", 0);
	cobj->val->type = NT_NULL; cobj->val->attr |= NST_SYSTEM;
	cobj = nsp_setnum(new_N, &new_N->g, "false", 0);
	cobj->val->type = NT_BOOLEAN; cobj->val->attr |= NST_SYSTEM;
	cobj = nsp_setnum(new_N, &new_N->g, "true", 1);
	cobj->val->type = NT_BOOLEAN; cobj->val->attr |= NST_SYSTEM;
	cobj = nsp_setstr(new_N, &new_N->g, "_version_", NSP_VERSION, -1);
	cobj = nsp_setstr(new_N, &new_N->g, "_ostype_", _OSTYPE_, -1);
	return new_N;
}

void nsp_freestate(nsp_state *N)
{
#define __FN__ __FILE__ ":nsp_freestate()"
	obj_t *cobj;

	if (N == NULL) return;
	settrace();
	if (N->outbuflen) nl_flush(N);
	if (nsp_istable((cobj = nsp_getobj(N, &N->g, "_GLOBALS")))) nsp_unlinkval(N, cobj);
	nsp_freetable(N, &N->g);
	n_freeval(N, &N->r);
	/* WHY THE F?*# IS IT THAT THIS IS NOT PORTABLE C / C++, */
	// if (N->g.val) n_free(N, (void *)&N->g.val, sizeof(val_t));
	/* AND THIS IS? */
	// if (N->g.val) { void *x=N->g.val; n_free(N, &x, sizeof(val_t)); }
	if (N->g.val) n_free(N, (void *)&N->g.val, sizeof(val_t));
	if (N->r.val) n_free(N, (void *)&N->r.val, sizeof(val_t));
	n_freeexeccontext(N, &N->context);
	//n_free(N, (void *)&N->context, sizeof(nsp_execcontext));
#undef __FN__
}

nsp_state *nsp_endstate(nsp_state *N)
{
#define __FN__ __FILE__ ":nsp_endstate()"
	if (N != NULL) {
		settrace();
		nsp_freestate(N);
		n_free(N, (void *)&N->outbuffer, N->outbufmax + 1);// one byte added for null termination
		n_free(N, (void *)&N, sizeof(nsp_t));
	}
	return NULL;
#undef __FN__
}
