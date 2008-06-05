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
#include <math.h>

/* read a function from N->readptr */
obj_t *n_getfunction(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_getfunction()"
	uchar *as, *be;

	settrace();
	/* if (N->debug) n_warn(N, __FUNCTION__, "snorting lines from %s()", cobj->name); */
	n_expect(N, __FUNCTION__, OP_POPAREN);
	as=N->readptr++;
	n_skipto(N, __FUNCTION__, OP_PCPAREN);
	N->readptr++;
	/* bs=N->readptr; */
	n_expect(N, __FUNCTION__, OP_POBRACE);
	N->readptr++;
	n_skipto(N, __FUNCTION__, OP_PCBRACE);
	be=++N->readptr;
	nes_setnfunc(N, &N->r, "", NULL, 0);
	nes_strcat(N, &N->r, (char *)as, be-as);
	return &N->r;
#undef __FUNCTION__
}

/* read a label from N->readptr */
char *n_getlabel(nes_state *N, char *buf)
{
#define __FUNCTION__ __FILE__ ":n_getlabel()"
	char *p=(char *)N->readptr+2;

	settrace();
	N->readptr+=3+N->readptr[1];
	if (buf) p=nc_strncpy(buf, p, MAX_OBJNAMELEN);
	return p;
#undef __FUNCTION__
}

/* read a number val from N->readptr */
num_t n_getnumber(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_getnumber()"
	char *s=(char *)N->readptr+2;
	num_t rval=0;
	num_t rdot=0.1;

	settrace();
	N->readptr+=3+N->readptr[1];
	while (nc_isdigit(*s)) {
		rval=10*rval+(*s++-'0');
	}
	if (*s!='.') return rval;
	s++;
	while (nc_isdigit(*s)) {
		rval+=(*s++-'0')*rdot;
		rdot*=0.1;
	}
	/* if (N->debug) n_warn(N, __FUNCTION__, "[%f]", rval); */
	return rval;
#undef __FUNCTION__
}

/* read a string val from N->readptr */
obj_t *n_getstring(nes_state *N)
{
#define __FUNCTION__ __FILE__ ":n_getstring()"
	int size=readi4((N->readptr+1));

	settrace();
	nes_strcat(N, nes_setstr(N, &N->r, "", NULL, 0), (char *)N->readptr+5, size);
	N->readptr+=6+size;
	/* if (N->debug) n_warn(N, __FUNCTION__, "%d '%s'", size, nes_tostr(N, cobj)); */
	return &N->r;
#undef __FUNCTION__
}

/* return the next table index */
obj_t *n_readindex(nes_state *N, obj_t *tobj, char *lastname)
{
#define __FUNCTION__ __FILE__ ":n_readindex()"
	obj_t *cobj;
	char *p;

	DEBUG_IN();
	settrace();
	sanetest();
	if (lastname) lastname[0]=0;
	if (*N->readptr==OP_PDOT) {
		N->readptr++;
		n_expect(N, __FUNCTION__, OP_LABEL);
		p=n_getlabel(N, NULL);
	} else if (*N->readptr==OP_POBRACKET) {
		N->readptr++;
		cobj=nes_eval(N, (char *)N->readptr);
		n_expect(N, __FUNCTION__, OP_PCBRACKET);
		N->readptr++;
		p=nes_isnull(cobj)?"":nes_tostr(N, cobj);
	} else {
		cobj=tobj;
		goto end;
	}
	cobj=nes_getobj(N, tobj, p);
	if (lastname&&cobj->val->type!=NT_TABLE) nc_strncpy(lastname, p, MAX_OBJNAMELEN);
end:
	/* if (N->debug) n_warn(N, __FUNCTION__, "[%d '%s' '%s']", cobj->val->type, cobj->name, lastname?lastname:""); */
	DEBUG_OUT();
	return cobj;
#undef __FUNCTION__
}

/* read the following list of values into the supplied table */
obj_t *n_readtable(nes_state *N, obj_t *tobj)
{
#define __FUNCTION__ __FILE__ ":n_readtable()"
	char namebuf[MAX_OBJNAMELEN+1];
	unsigned int i;
	obj_t *cobj;

	DEBUG_IN();
	settrace();
	sanetest();
	n_expect(N, __FUNCTION__, OP_POBRACE);
	N->readptr++;
	i=0;
	while (*N->readptr) {
		namebuf[0]=0;
		/* first get a name */
		if (*N->readptr==OP_POBRACE) goto data;
		if (*N->readptr==OP_LABEL) {
			/* either a label to use, or a function to run... */
			uchar *p=N->readptr;

			n_getlabel(N, namebuf);
			/* if it's a function, do something useful... */
			if (*N->readptr==OP_POPAREN||*N->readptr==OP_POBRACKET||*N->readptr==OP_PDOT) {
				/* either a function or some kind of reference - NOT an lval */
				namebuf[0]=0;
				N->readptr=p;
			}
		} else if (*N->readptr==OP_POBRACKET) {
			/* [ ] */
			N->readptr++;
			if (*N->readptr==OP_LABEL) {
				n_getlabel(N, namebuf);
			} else if (*N->readptr==OP_NUMDATA) {
				n_ntoa(N, namebuf, n_getnumber(N), 10, 0);
			}
			n_expect(N, __FUNCTION__, OP_PCBRACKET);
			N->readptr++;
		} else if (*N->readptr==OP_PCBRACE) {
			goto end;
		} else if (*N->readptr==OP_NUMDATA||*N->readptr==OP_STRDATA||*N->readptr==OP_KFUNC) {
			goto data;
		} else if ((*N->readptr==OP_MSUB||*N->readptr==OP_MADD)&&N->readptr[1]==OP_NUMDATA) {
			goto data;
		} else {
			n_warn(N, __FUNCTION__, "unhandled data.  probably an error [%d]", *N->readptr);
		}
		if (*N->readptr==OP_MEQ) N->readptr++;
data:
		if (namebuf[0]==0) n_ntoa(N, namebuf, i++, 10, 0);
		/* one way or another, we have a name.  now figure out what the val is. */
		/* if (N->debug) n_warn(N, __FUNCTION__, "[%s][%s]%d", namebuf, n_getsym(N, *N->readptr), *N->readptr); */
		if (*N->readptr==OP_POBRACE) {
			cobj=nes_getobj(N, tobj, namebuf);
			if (cobj->val->type!=NT_TABLE) {
				cobj=nes_settable(N, tobj, namebuf);
			} else {
				cobj=tobj;
			}
			n_readtable(N, cobj);
			n_expect(N, __FUNCTION__, OP_PCBRACE);
			N->readptr++;
		} else if (*N->readptr!=OP_PCOMMA&&*N->readptr!=OP_PSEMICOL&&*N->readptr!=OP_PCBRACE) {
			cobj=nes_getobj(N, tobj, namebuf);
			if (nes_isnull(cobj)) cobj=nes_setnum(N, tobj, namebuf, 0);
			n_storeval(N, cobj);
		}
		if (*N->readptr==OP_PCOMMA||*N->readptr==OP_PSEMICOL) {
			N->readptr++;
			continue;
		}
		n_expect(N, __FUNCTION__, OP_PCBRACE);
		break;
	}
end:
	DEBUG_OUT();
	return tobj;
#undef __FUNCTION__
}

/* read a var (or table) from N->readptr */
obj_t *n_readvar(nes_state *N, obj_t *tobj, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":n_readvar()"
	char namebuf[MAX_OBJNAMELEN+1];
	char *nameptr=namebuf;
	uchar preop=0;

	DEBUG_IN();
	settrace();
	sanetest();
	if (cobj==NULL) {
		if (*N->readptr=='\0') { DEBUG_OUT(); return NULL; }
		if (*N->readptr==OP_MADDADD||*N->readptr==OP_MSUBSUB) preop=*N->readptr++;
		n_expect(N, __FUNCTION__, OP_LABEL);
		nameptr=n_getlabel(N, NULL);
		cobj=nes_getobj(N, tobj, nameptr);
	}
	while (cobj->val->type==NT_TABLE) {
		tobj=cobj;
		if (*N->readptr!=OP_POBRACKET&&*N->readptr!=OP_PDOT) break;
		cobj=n_readindex(N, tobj, namebuf);
		nameptr=namebuf;
	}
	if (nes_isnull(cobj)) cobj=nes_setnum(N, tobj, nameptr, 0);
	if (cobj->val->type==NT_NUMBER) {
		if (preop==OP_MADDADD) {
			cobj->val->d.num++;
		} else if (preop==OP_MSUBSUB) {
			cobj->val->d.num--;
		}
	}
	if (OP_ISMATH(*N->readptr)) cobj=n_storeval(N, cobj);
	DEBUG_OUT();
	return cobj;
#undef __FUNCTION__
}

/* store a val in the supplied object */
obj_t *n_storeval(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":n_storeval()"
	obj_t *nobj;
	uchar op;

	settrace();
	switch (*N->readptr) {
	case OP_MADDADD:
		N->readptr++;
		if (nes_typeof(cobj)!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "object is not a number");
		cobj->val->d.num++;
		return cobj;
	case OP_MSUBSUB:
		N->readptr++;
		if (nes_typeof(cobj)!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "object is not a number");
		cobj->val->d.num--;
		return cobj;
	case OP_MADDEQ :
		if (nes_typeof(cobj)==NT_STRING) {
			N->readptr++;
			nobj=nes_eval(N, (char *)N->readptr);
			if (nes_isstr(nobj)) {
				nes_strcat(N, cobj, nobj->val->d.str, nobj->val->size);
			} else {
				nes_strcat(N, cobj, nes_tostr(N, nobj), -1);
			}
			return cobj;
		}
	case OP_MMULEQ :
		if (nes_typeof(cobj)==NT_STRING) {
			N->readptr++;
			nobj=nes_eval(N, (char *)N->readptr);
			if (nobj->val->d.num<0) nes_strmul(N, cobj, 0);
			else nes_strmul(N, cobj, (int)nobj->val->d.num);
			return cobj;
		}
	case OP_MSUBEQ :
	case OP_MDIVEQ :
		op=*N->readptr++;
		if (nes_typeof(cobj)!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "object is not a number");
		nobj=nes_eval(N, (char *)N->readptr);
		if (nes_typeof(nobj)!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "object is not a number");
		if (!OP_ISEND(*N->readptr)) n_error(N, NE_SYNTAX, __FUNCTION__, "expected ';'");
		switch (op) {
		case OP_MADDEQ : cobj->val->d.num+=nobj->val->d.num; return cobj;
		case OP_MSUBEQ : cobj->val->d.num-=nobj->val->d.num; return cobj;
		case OP_MMULEQ : cobj->val->d.num*=nobj->val->d.num; return cobj;
		case OP_MDIVEQ : cobj->val->d.num/=nobj->val->d.num; return cobj;
		}
	case OP_MEQ :
		op=*N->readptr++;
	default:
		if (*N->readptr==OP_KFUNC) {
			N->readptr++;
			nes_linkval(N, cobj, n_getfunction(N));
			return cobj;
		}
		nobj=nes_eval(N, (char *)N->readptr);
		/* if (!OP_ISEND(N->lastop)) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a ';' %d", N->lastop); */
		switch (nes_typeof(nobj)) {
		case NT_NULL   :
			nes_unlinkval(N, cobj);
		case NT_TABLE  :
		case NT_STRING :
			if (cobj!=nobj) {
				nes_linkval(N, cobj, nobj);
				nes_unlinkval(N, nobj);
			}
			break;
		case NT_CDATA  :
			nes_linkval(N, cobj, nobj);
			break;
		default:
			n_copyval(N, cobj, nobj);
		}
	}
	return cobj;
#undef __FUNCTION__
}

static obj_t *n_evalobj(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":n_evalobj()"
	settrace();
	cobj->val=NULL;
	if (*N->readptr==OP_POBRACE) {
		nes_setvaltype(N, cobj, NT_TABLE);
//		cobj->val=n_newval(N, NT_TABLE);
		n_readtable(N, cobj);
		n_expect(N, __FUNCTION__, OP_PCBRACE);
		N->readptr++;
	} else if (*N->readptr==OP_POPAREN) {
		N->readptr++;
		nes_linkval(N, cobj, nes_eval(N, (char *)N->readptr));
		n_expect(N, __FUNCTION__, OP_PCPAREN);
		N->readptr++;
	} else {
		short preop=OP_ISMATH(*N->readptr)?*N->readptr++:0;
		obj_t *nobj, *pobj=NULL;
		uchar *p;
		char *l;

		if (*N->readptr==OP_STRDATA) {
			nes_linkval(N, cobj, n_getstring(N));
			return cobj;
		} else if (*N->readptr==OP_NUMDATA) {
			nes_setnum(N, cobj, NULL, (preop!=OP_MSUB)?+n_getnumber(N):-n_getnumber(N));
			return cobj;
		} else if (*N->readptr==OP_LABEL) {
			p=N->readptr;
			l=n_getlabel(N, NULL);
			nobj=nes_getobj(N, &N->l, l);
			while (*N->readptr==OP_POBRACKET||*N->readptr==OP_PDOT) {
				pobj=nobj;
				nobj=n_readindex(N, (nobj->val->type==NT_TABLE)?nobj:NULL, NULL);
			}
			switch (*N->readptr) {
			case OP_POPAREN :
				if (nobj->val->type!=NT_NFUNC&&nobj->val->type!=NT_CFUNC) {
					n_error(N, NE_SYNTAX, __FUNCTION__, "'%s' is not a function", l);
				}
				nobj=n_execfunction(N, nobj, pobj);
				break;
			case OP_MADDEQ  :
			case OP_MSUBEQ  :
			case OP_MMULEQ  :
			case OP_MDIVEQ  : if (nes_isnull(nobj)) n_error(N, NE_SYNTAX, __FUNCTION__, "null object");
			case OP_MEQ     : N->readptr=p; nobj=n_readvar(N, &N->l, NULL); break;
			}
			switch (nes_typeof(nobj)) {
			case NT_NULL   :
//				nes_setvaltype(N, cobj, NT_NULL); break;
				cobj->val=n_newval(N, NT_NULL); break;
			case NT_NUMBER :
//				nes_setvaltype(N, cobj, NT_NUMBER);
				cobj->val=n_newval(N, NT_NUMBER);
				/* shouldn't ~ and ! be here? */
				switch (preop) {
				case OP_MSUB    : cobj->val->d.num=-nobj->val->d.num; break;
				case OP_MADDADD : cobj->val->d.num=++nobj->val->d.num; break;
				case OP_MSUBSUB : cobj->val->d.num=--nobj->val->d.num; break;
				default : cobj->val->d.num=nobj->val->d.num;
				}
				switch (*N->readptr) {
				case OP_MADDADD : N->readptr++; nobj->val->d.num++; break;
				case OP_MSUBSUB : N->readptr++; nobj->val->d.num--; break;
				}
				break;
			case NT_STRING : if (nobj!=cobj) n_copyval(N, cobj, nobj); break;
			case NT_TABLE  : nes_linkval(N, cobj, nobj); break;
			case NT_CDATA  : nes_linkval(N, cobj, nobj); break;
			default: n_copyval(N, cobj, nobj);
			}
		}
	}
	return cobj;
#undef __FUNCTION__
}

static obj_t *n_evalmath(nes_state *N, obj_t *cobj, uchar op, obj_t *nobj)
{
#define __FUNCTION__ __FILE__ ":n_evalmath()"
	int n, t;

	settrace();
	if (nes_isnull(nobj)) {
		n=nes_isnull(cobj);
		t=nes_istrue(cobj);
//		nes_unlinkval(N, cobj);
//		cobj->val=n_newval(N, NT_BOOLEAN);
		nes_setvaltype(N, cobj, NT_BOOLEAN);
		switch (op) {
		case OP_MCEQ   : cobj->val->d.num=n?1:0; break;
		case OP_MCNE   : cobj->val->d.num=n?0:1; break;
		case OP_MLAND  : cobj->val->d.num=0; break;
		case OP_MLOR   : cobj->val->d.num=n?0:t; break;
		default: nes_unlinkval(N, cobj); n_warn(N, __FUNCTION__, "unhandled null comparison %s", n_getsym(N, op));
		}
		return cobj;
	}
	switch (nes_typeof(cobj)) {
	case NT_NULL:
		n=nes_isnull(nobj);
		t=nes_istrue(nobj);
//		nes_unlinkval(N, cobj);
//		cobj->val=n_newval(N, NT_BOOLEAN);
		nes_setvaltype(N, cobj, NT_BOOLEAN);
		switch (op) {
		case OP_MCEQ   : cobj->val->d.num=n?1:0; break;
		case OP_MCNE   : cobj->val->d.num=n?0:1; break;
		case OP_MLAND  : cobj->val->d.num=0; break;
		case OP_MLOR   : cobj->val->d.num=nes_tonum(N, nobj)?1:0; break;
		}
		return cobj;
	case NT_BOOLEAN:
		switch (nes_typeof(nobj)) {
/*
		case NT_NULL:
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=0; break;
			case OP_MCNE   : cobj->val->d.num=1; break;
			}
			return cobj;
*/
		case NT_NUMBER:
		case NT_BOOLEAN:
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=(cobj->val->d.num?1:0)==(nobj->val->d.num?1:0)?1:0; break;
			case OP_MCNE   : cobj->val->d.num=(cobj->val->d.num?1:0)!=(nobj->val->d.num?1:0)?1:0; break;
			case OP_MLAND  : cobj->val->d.num=cobj->val->d.num && nobj->val->d.num; break;
			case OP_MLOR   : cobj->val->d.num=cobj->val->d.num || nobj->val->d.num; break;
			}
			return cobj;
		case NT_STRING:
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=(cobj->val->d.num&&(nobj->val->d.str&&nobj->val->d.str[0]))?1:0; break;
			}
			return cobj;
		case NT_TABLE:
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=(cobj->val->d.num?1:0)==(nobj->val->d.table?1:0)?1:0; break;
			case OP_MCNE   : cobj->val->d.num=(cobj->val->d.num?1:0)!=(nobj->val->d.table?1:0)?1:0; break;
			}
			return cobj;
		}
		return cobj;
	case NT_NUMBER:
		switch (nes_typeof(nobj)) {
/*
		case NT_NULL:
			switch (op) {
			case OP_MCEQ   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=0; break;
			case OP_MCNE   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=1; break;
			}
			return cobj;
*/
		case NT_STRING:
			nes_setnum(N, nobj, "", n_aton(N, nobj->val->d.str));
		case NT_NUMBER:
			switch (op) {
			case OP_MCEQ   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num == nobj->val->d.num; break;
			case OP_MCNE   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num != nobj->val->d.num; break;
			case OP_MCLT   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num <  nobj->val->d.num; break;
			case OP_MCLE   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num <= nobj->val->d.num; break;
			case OP_MCGT   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num >  nobj->val->d.num; break;
			case OP_MCGE   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num >= nobj->val->d.num; break;
			case OP_MLAND  : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num && nobj->val->d.num; break;
			case OP_MLOR   : cobj->val->type=NT_BOOLEAN; cobj->val->d.num=cobj->val->d.num || nobj->val->d.num; break;
			case OP_MADD   :
			case OP_MADDADD:
			case OP_MADDEQ : cobj->val->d.num=cobj->val->d.num + nobj->val->d.num; break;
			case OP_MSUB   :
			case OP_MSUBSUB:
			case OP_MSUBEQ : cobj->val->d.num=cobj->val->d.num - nobj->val->d.num; break;
			case OP_MMUL   :
			case OP_MMULEQ : cobj->val->d.num=cobj->val->d.num * nobj->val->d.num; break;
			case OP_MDIV   :
			case OP_MDIVEQ : if (nobj->val->d.num==0) { cobj->val->type=NT_NULL; break; }
				cobj->val->d.num=cobj->val->d.num / nobj->val->d.num; break;
			case OP_MMOD   : if (nobj->val->d.num==0) { cobj->val->type=NT_NULL; break; }
				cobj->val->d.num=(int)cobj->val->d.num % (int)nobj->val->d.num; break;
			case OP_MAND   : cobj->val->d.num=(int)cobj->val->d.num & (int)nobj->val->d.num; break;
			case OP_MOR    : cobj->val->d.num=(int)cobj->val->d.num | (int)nobj->val->d.num; break;
			case OP_MXOR   : cobj->val->d.num=(int)cobj->val->d.num ^ (int)nobj->val->d.num; break;
			}
			return cobj;
		case NT_BOOLEAN:
			cobj->val->type=NT_BOOLEAN;
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=(cobj->val->d.num?1:0)==(nobj->val->d.num?1:0)?1:0; break;
			}
			return cobj;
		}
		return cobj;
	case NT_STRING:
		switch (nes_typeof(nobj)) {
/*
		case NT_NULL:
			switch (op) {
			case OP_MCEQ   : nes_setnum(N, cobj, "", 0); cobj->val->type=NT_BOOLEAN; break;
			case OP_MCNE   : nes_setnum(N, cobj, "", 1); cobj->val->type=NT_BOOLEAN; break;
			}
			return cobj;
*/
		case NT_NUMBER:
			if (op==OP_MMUL) {
				if (nobj->val->d.num<0) nes_strmul(N, cobj, 0);
				else nes_strmul(N, cobj, (int)nobj->val->d.num);
				return cobj;
			} else {
				nes_setstr(N, nobj, "", n_ntoa(N, N->numbuf, nobj->val->d.num, 10, 6), -1);
			}
		case NT_STRING:
			if (op==OP_MADD) {
				nes_strcat(N, cobj, nobj->val->d.str, nobj->val->size);
			} else {
				int cmp=nc_strcmp(cobj->val->d.str, nobj->val->d.str);

				n_freeval(N, cobj);
				cobj->val->type=NT_BOOLEAN;
				switch (op) {
				case OP_MCEQ : cobj->val->d.num=cmp?0:1;    break;
				case OP_MCNE : cobj->val->d.num=cmp?1:0;    break;
				case OP_MCLE : cobj->val->d.num=cmp<=0?1:0; break;
				case OP_MCGE : cobj->val->d.num=cmp>=0?1:0; break;
				case OP_MCLT : cobj->val->d.num=cmp<0?1:0;  break;
				case OP_MCGT : cobj->val->d.num=cmp>0?1:0;  break;
				}
			}
			return cobj;
		case NT_BOOLEAN:
			switch (op) {
			case OP_MCEQ   :
				nes_setnum(N, cobj, "", ((cobj->val->d.str&&cobj->val->d.str[0])?1:0)==(nobj->val->d.num?1:0)?1:0);
				cobj->val->type=NT_BOOLEAN;
			}
			return cobj;
		}
		break;
	case NT_TABLE:
		switch (nes_typeof(nobj)) {
/*
		case NT_NULL:
//			nes_unlinkval(N, cobj);
//			cobj->val=n_newval(N, NT_BOOLEAN);
			nes_setvaltype(N, cobj, NT_BOOLEAN);
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=0; break;
			case OP_MCNE   : cobj->val->d.num=1; break;
			}
			return cobj;
*/
		case NT_TABLE: {
			int cmp=(cobj->val->d.table==nobj->val->d.table);

//			nes_unlinkval(N, cobj);
//			cobj->val=n_newval(N, NT_BOOLEAN);
			nes_setvaltype(N, cobj, NT_BOOLEAN);
			switch (op) {
			case OP_MCEQ   : cobj->val->d.num=cmp?1:0; break;
			case OP_MCNE   : cobj->val->d.num=cmp?0:1; break;
			}
			return cobj;
		}
		}
		return cobj;
	}
	return cobj;
#undef __FUNCTION__
}

static void n_evalsub(nes_state *N, uchar op1, obj_t *obj1)
{
#define __FUNCTION__ __FILE__ ":n_evalsub()"
	obj_t obj2;
	uchar op2;

	settrace();
	while (OP_ISMATH(*N->readptr)) {
		op2=*N->readptr++;
		n_evalobj(N, &obj2);
		/* this should do something based on the type of the _first_ object in the list... i.e. string addition... */
		if (OP_ISMATH(*N->readptr)&&(oplist[(uchar)*N->readptr].priority>oplist[(uchar)op2].priority)) {
			n_evalsub(N, op2, &obj2);
		}
		n_evalmath(N, obj1, op2, &obj2);
		nes_unlinkval(N, &obj2);
	}
	return;
#undef __FUNCTION__
}

/*
 * the following functions are public API functions
 */

obj_t *nes_eval(nes_state *N, const char *string)
{
#define __FUNCTION__ __FILE__ ":nes_eval()"
	obj_t obj1;

	DEBUG_IN();
	settrace();
	sanetest();
	if (N->savjmp!=NULL) {
		N->readptr=(uchar *)string;
		n_evalobj(N, &obj1);
		n_evalsub(N, 0, &obj1);
		nes_unlinkval(N, &N->r);
		N->r.val=obj1.val;
	} else {
		uchar *p;

		if (string==NULL||string[0]==0) {
			nes_unlinkval(N, &N->r);
			DEBUG_OUT();
			return &N->r;
		}
		p=n_decompose(N, (uchar *)string);
		if (p!=N->blockptr) N->blockptr=p; else p=NULL;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)==0) {
			n_evalobj(N, &obj1);
			n_evalsub(N, 0, &obj1);
			nes_unlinkval(N, &N->r);
			N->r.val=obj1.val;
		}
		n_free(N, (void *)&N->savjmp);
		if (p) n_free(N, (void *)&p);
		N->blockend=NULL;
		N->readptr=NULL;
	}
	DEBUG_OUT();
	return &N->r;
#undef __FUNCTION__
}

#include <stdarg.h>
int nc_vsnprintf(nes_state *N, char *dest, int max, const char *format, va_list ap);
#define MAXBUF 8192

obj_t *nes_evalf(nes_state *N, const char *fmt, ...)
{
#define __FUNCTION__ __FILE__ ":nes_evalf()"
	va_list ap;
	uchar *buf;
	uchar *oldbptr=N->blockptr;
	uchar *oldbend=N->blockend;
	uchar *oldrptr=N->readptr;
	obj_t *cobj;

	settrace();
	if ((buf=n_alloc(N, MAXBUF, 1))==NULL) return NULL;
	va_start(ap, fmt);
	nc_vsnprintf(N, (char *)buf, MAXBUF, fmt, ap);
	va_end(ap);
	N->blockptr=buf;
	cobj=nes_eval(N, (char *)N->blockptr);
	n_free(N, (void *)&N->blockptr);
	N->blockptr=oldbptr;
	N->blockend=oldbend;
	N->readptr=oldrptr;
	return cobj;
#undef __FUNCTION__
}
