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
	{ NULL,       OP_UNDEFINED, -1 }, /*  0 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  1 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  2 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  3 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  4 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  5 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  6 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  7 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  8 */
	{ NULL,       OP_UNDEFINED, -1 }, /*  9 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 10 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 11 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 12 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 13 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 14 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 15 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 16 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 17 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 18 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 19 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 20 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 21 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 22 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 23 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 24 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 25 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 26 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 27 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 28 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 29 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 30 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 31 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 32 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 33 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 34 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 35 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 36 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 37 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 38 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 39 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 40 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 41 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 42 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 43 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 44 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 45 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 46 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 47 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 48 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 49 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 50 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 51 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 52 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 53 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 54 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 55 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 56 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 57 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 58 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 59 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 60 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 61 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 62 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 63 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 64 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 65 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 66 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 67 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 68 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 69 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 70 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 71 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 72 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 73 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 74 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 75 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 76 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 77 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 78 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 79 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 80 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 81 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 82 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 83 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 84 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 85 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 86 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 87 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 88 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 89 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 90 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 91 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 92 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 93 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 94 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 95 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 96 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 97 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 98 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 99 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 100 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 101 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 102 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 103 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 104 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 105 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 106 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 107 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 108 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 109 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 110 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 111 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 112 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 113 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 114 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 115 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 116 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 117 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 118 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 119 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 120 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 121 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 122 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 123 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 124 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 125 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 126 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 127 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 128 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 129 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 130 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 131 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 132 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 133 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 134 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 135 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 136 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 137 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 138 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 139 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 140 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 141 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 142 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 143 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 144 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 145 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 146 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 147 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 148 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 149 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 150 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 151 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 152 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 153 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 154 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 155 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 156 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 157 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 158 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 159 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 160 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 161 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 162 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 163 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 164 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 165 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 166 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 167 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 168 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 169 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 170 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 171 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 172 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 173 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 174 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 175 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 176 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 177 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 178 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 179 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 180 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 181 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 182 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 183 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 184 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 185 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 186 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 187 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 188 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 189 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 190 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 191 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 192 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 193 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 194 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 195 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 196 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 197 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 198 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 199 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 200 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 201 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 202 */
	{ NULL,       OP_UNDEFINED, -1 }, /* 203 */

	/* KEYWORDS */
	{ "exit",     OP_KEXIT    , -1 },  /* 204 */
	{ "catch",    OP_KCATCH   , -1 },  /* 205 */
	{ "try",      OP_KTRY     , -1 },  /* 206 */
	{ "while",    OP_KWHILE   , -1 },  /* 207 */
	{ "do",       OP_KDO      , -1 },  /* 208 */
	{ "for",      OP_KFOR     , -1 },  /* 209 */
	{ "else",     OP_KELSE    , -1 },  /* 210 */
	{ "if",       OP_KIF      , -1 },  /* 211 */
	{ "var",      OP_KVAR     , -1 },  /* 212 */
	{ "local",    OP_KLOCAL   , -1 },  /* 213 */
	{ "global",   OP_KGLOB    , -1 },  /* 214 */
	{ "function", OP_KFUNC    , -1 },  /* 215 */
	{ "return",   OP_KRET     , -1 },  /* 216 */
	{ "continue", OP_KCONT    , -1 },  /* 217 */
	{ "break",    OP_KBREAK   , -1 },  /* 218 */
	/* MATH */
	{ ">",        OP_MCGT     , 2  },  /* 219 */
	{ "<",        OP_MCLT     , 2  },  /* 220 */
	{ ">=",       OP_MCGE     , 2  },  /* 221 */
	{ "<=",       OP_MCLE     , 2  },  /* 222 */
	{ "!=",       OP_MCNE     , 2  },  /* 223 */
	{ "==",       OP_MCEQ     , 2  },  /* 224 */
	{ "!",        OP_MLNOT    , 1  },  /* 225 */
	{ "||",       OP_MLOR     , 1  },  /* 226 */
	{ "&&",       OP_MLAND    , 1  },  /* 227 */
	{ "^",        OP_MXOR     , 5  },  /* 228 */
	{ "|",        OP_MOR      , 5  },  /* 229 */
	{ "&",        OP_MAND     , 5  },  /* 230 */
	{ "%",        OP_MMOD     , 4  },  /* 231 */
	{ "--",       OP_MSUBSUB  , 3  },  /* 232 */
	{ "++",       OP_MADDADD  , 3  },  /* 233 */
	{ "/=",       OP_MDIVEQ   , 0  },  /* 234 */
	{ "*=",       OP_MMULEQ   , 0  },  /* 235 */
	{ "-=",       OP_MSUBEQ   , 0  },  /* 236 */
	{ "+=",       OP_MADDEQ   , 0  },  /* 237 */
	{ "/",        OP_MDIV     , 4  },  /* 238 */
	{ "*",        OP_MMUL     , 4  },  /* 239 */
	{ "-",        OP_MSUB     , 3  },  /* 240 */
	{ "+",        OP_MADD     , 3  },  /* 241 */
	{ "=",        OP_MEQ      , 0  },  /* 242 */
	/* PUNCTUATION */
	{ "#",        OP_PHASH    , -1 },  /* 243 */
	{ "\"",       OP_PDQUOTE  , -1 },  /* 244 */
	{ "\'",       OP_PSQUOTE  , -1 },  /* 245 */
	{ ".",        OP_PDOT     , -1 },  /* 246 */
	{ ";",        OP_PSEMICOL , -1 },  /* 247 */
	{ ",",        OP_PCOMMA   , -1 },  /* 248 */
	{ "]",        OP_PCBRACKET, -1 },  /* 249 */
	{ ")",        OP_PCPAREN  , -1 },  /* 250 */
	{ "}",        OP_PCBRACE  , -1 },  /* 251 */
	{ "[",        OP_POBRACKET, -1 },  /* 252 */
	{ "(",        OP_POPAREN  , -1 },  /* 253 */
	{ "{",        OP_POBRACE  , -1 },  /* 254 */
	{ NULL,       OP_UNDEFINED, -1 }   /* 255 */
};

/* return the next op/cmp and advance the readptr */
short n_getop(nes_state *N, char *name)
{
#define __FUNCTION__ "n_getop"
	short i;

	if (N->readptr==NULL) n_error(N, NE_SYNTAX, __FUNCTION__, "NULL readptr");
	if (*N->readptr>127) {
		name[0]=0;
		return *N->readptr++;
	} else if (IS_DATA(*N->readptr)) {
		name[0]=0;
		if (*N->readptr=='\''||*N->readptr=='\"') 
			return OP_STRDATA;
		else if (nc_isdigit(*N->readptr))
			return OP_NUMDATA;
	} else if (IS_LABEL(*N->readptr)) {
		name[0]=*N->readptr++;
		for (i=1;i<MAX_OBJNAMELEN;i++) {
			if (!IS_LABEL(*N->readptr)&&!nc_isdigit(*N->readptr)) break;
			name[i]=*N->readptr++;
		}
		name[i]=0;
		for (i=OP_KEXIT;i<=OP_KBREAK;i++) {
			if (nc_strcmp(oplist[i].name, name)==0) return oplist[i].value;
		}
		return OP_LABEL;
	} else if (IS_PUNCOP(*N->readptr)) {
		name[0]=*N->readptr++;
		name[1]=0;
		for (i=OP_PHASH;i<=OP_POBRACE;i++) {
			if (nc_strcmp(oplist[i].name, name)==0) return oplist[i].value;
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
		for (i=OP_MCGT;i<=OP_MEQ;i++) {
			if (nc_strcmp(oplist[i].name, name)==0) return oplist[i].value;
		}
	} else {
		name[0]=0;
	}
	return OP_UNDEFINED;
#undef __FUNCTION__
}

char *n_getsym(nes_state *N, short op)
{
	switch (op) {
	case OP_STRDATA : return "string";
	case OP_NUMDATA : return "number";
	case OP_LABEL   : return "label";
	default         : if (op>OP_LABEL&&op<OP_UNDEFINED) return oplist[op].name;
	}
	return "(unknown)";
}

#define skipblock(op) { while (*p) { if (*p==op) return p+1; else p=n_seekop(N, p, 1, 1); } }

uchar *n_seekop(nes_state *N, uchar *p, int ops, int sb)
{
	while (ops-->0) {
		switch (*p) {
		case '\0'       : return p;
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