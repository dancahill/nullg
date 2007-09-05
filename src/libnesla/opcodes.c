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

#define IS_MATHOP(c) (c=='='||c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='&'||c=='|'||c=='^'||c=='!'||c=='<'||c=='>')
#define IS_PUNCOP(c) (c=='('||c==')'||c==','||c=='{'||c=='}'||c==';'||c=='.'||c=='['||c==']')
#define IS_DATA(c)   (c=='\''||c=='\"'||nc_isdigit(c))
#define IS_LABEL(c)  (c=='_'||c=='$'||nc_isalpha(c))

const optab oplist[] = {
	/* PUNCTUATION */
	{ "{",        OP_POBRACE  , -1 },
	{ "(",        OP_POPAREN  , -1 },
	{ "[",        OP_POBRACKET, -1 },
	{ "}",        OP_PCBRACE  , -1 },
	{ ")",        OP_PCPAREN  , -1 },
	{ "]",        OP_PCBRACKET, -1 },
	{ ",",        OP_PCOMMA   , -1 },
	{ ";",        OP_PSEMICOL , -1 },
	{ ".",        OP_PDOT     , -1 },
	{ "\'",       OP_PSQUOTE  , -1 },
	{ "\"",       OP_PDQUOTE  , -1 },
	{ "#",        OP_PHASH    , -1 },
	/* MATH */
	{ "=",        OP_MEQ      , 0  },
	{ "+",        OP_MADD     , 3  },
	{ "-",        OP_MSUB     , 3  },
	{ "*",        OP_MMUL     , 4  },
	{ "/",        OP_MDIV     , 4  },
	{ "+=",       OP_MADDEQ   , 0  },
	{ "-=",       OP_MSUBEQ   , 0  },
	{ "*=",       OP_MMULEQ   , 0  },
	{ "/=",       OP_MDIVEQ   , 0  },
	{ "++",       OP_MADDADD  , 3  },
	{ "--",       OP_MSUBSUB  , 3  },
	{ "%",        OP_MMOD     , 4  },
	{ "&",        OP_MAND     , 5  },
	{ "|",        OP_MOR      , 5  },
	{ "^",        OP_MXOR     , 5  },
	{ "&&",       OP_MLAND    , 1  },
	{ "||",       OP_MLOR     , 1  },
	{ "!",        OP_MLNOT    , 1  },
	{ "==",       OP_MCEQ     , 2  },
	{ "!=",       OP_MCNE     , 2  },
	{ "<=",       OP_MCLE     , 2  },
	{ ">=",       OP_MCGE     , 2  },
	{ "<",        OP_MCLT     , 2  },
	{ ">",        OP_MCGT     , 2  },
	/* KEYWORDS */
	{ "break",    OP_KBREAK   , -1 },
	{ "continue", OP_KCONT    , -1 },
	{ "return",   OP_KRET     , -1 },
	{ "function", OP_KFUNC    , -1 },
	{ "global",   OP_KGLOB    , -1 },
	{ "local",    OP_KLOCAL   , -1 },
	{ "var",      OP_KVAR     , -1 },
	{ "if",       OP_KIF      , -1 },
	{ "else",     OP_KELSE    , -1 },
	{ "for",      OP_KFOR     , -1 },
	{ "do",       OP_KDO      , -1 },
	{ "while",    OP_KWHILE   , -1 },
	{ "try",      OP_KTRY     , -1 },
	{ "catch",    OP_KCATCH   , -1 },
	{ "exit",     OP_KEXIT    , -1 },
	{ NULL,       0           , -1 }
};

/* return the next op/cmp and advance the readptr */
short n_getop(nes_state *N, char *name)
{
#define __FUNCTION__ "n_getop"
	short i;
	short op;

	if (N->readptr==NULL) n_error(N, NE_SYNTAX, __FUNCTION__, "NULL readptr");
	name[0]=0;
	if (*N->readptr>127) {
		op=*N->readptr++;
		return op;
	}
	op=OP_UNDEFINED;
	if (IS_DATA(*N->readptr)) {
		if (*N->readptr=='\''||*N->readptr=='\"') 
			op=OP_STRDATA;
		else if (nc_isdigit(*N->readptr))
			op=OP_NUMDATA;

		return op;
	} else if (IS_LABEL(*N->readptr)) {
		name[0]=*N->readptr++;
		for (i=1;i<MAX_OBJNAMELEN;i++) {
			if (!IS_LABEL(*N->readptr)&&!nc_isdigit(*N->readptr)) break;
			name[i]=*N->readptr++;
		}
		name[i]=0;
		op=OP_LABEL;
		for (i=254-OP_KBREAK;i<=254-OP_KEXIT;i++) {
			if (oplist[i].name[0]!=name[0]) continue;
			if (nc_strcmp(oplist[i].name, name)!=0) continue;
			op=oplist[i].value;
			break;
		}
	} else if (IS_PUNCOP(*N->readptr)) {
		name[0]=*N->readptr++;
		name[1]=0;
		for (i=254-OP_POBRACE;i<=254-OP_PHASH;i++) {
			if (oplist[i].name[0]!=name[0]) continue;
			if (nc_strcmp(oplist[i].name, name)!=0) continue;
			op=oplist[i].value;
			break;
		}
	} else if (IS_MATHOP(*N->readptr)) {
		i=0;
		name[i++]=*N->readptr++;
		if (IS_MATHOP(*N->readptr)) {
			if (name[0]=='=') {
				if (N->readptr[0]=='=') name[i++]=*N->readptr++;
			} else {
				name[i++]=*N->readptr++;
			}
		}
		name[i]=0;
		for (i=254-OP_MEQ;i<=254-OP_MCGT;i++) {
			if (oplist[i].name[0]!=name[0]) continue;
			if (nc_strcmp(oplist[i].name, name)!=0) continue;
			op=oplist[i].value;
			break;
		}
	}
	return op;
#undef __FUNCTION__
}

char *n_getsym(nes_state *N, short op)
{
	switch (op) {
	case OP_STRDATA : return "string";
	case OP_NUMDATA : return "number";
	case OP_LABEL   : return "label";
	default         : if (op<OP_UNDEFINED&&op>OP_LABEL) return oplist[254-op].name;
	}
	return "(unknown)";
}

#define skipblock(op) { while (*p) { if (*p==op) return p+1; else p=n_seekop(N, p, 1, 1); } }

uchar *n_seekop(nes_state *N, uchar *p, int ops, int sb)
{
	while ((ops-->0)&&(*p)) {
		switch (*p) {
		case OP_STRDATA : p+=6+readi4((p+1)); break;
		case OP_NUMDATA : p+=3+(int)(p[1]); break;
		case OP_LABEL   : p+=3+(int)(p[1]); break;
		case OP_POBRACE : p+=1; if (sb) skipblock(OP_PCBRACE); break;
		case OP_POPAREN : p+=1; if (sb) skipblock(OP_PCPAREN); break;
		default: p+=1; break;
		}
	}
	return p;
}
