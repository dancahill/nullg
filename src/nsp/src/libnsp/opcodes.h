/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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
#ifndef _NSP_OPCODES_H
#define _NSP_OPCODES_H 1

/* ops and cmps */
#define OP_UNDEFINED    255 /* 0xFF */
/* punctuation */
#define	OP_POBRACE	254 /* 0xFE */
#define	OP_POPAREN	253 /* 0xFD */
#define	OP_POBRACKET	252 /* 0xFC */

#define	OP_PCBRACE	251 /* 0xFB */
#define	OP_PCPAREN	250 /* 0xFA */
#define	OP_PCBRACKET	249 /* 0xF9 */
#define	OP_PCOMMA	248 /* 0xF8 */
#define	OP_PSEMICOL	247 /* 0xF7 */

#define	OP_PDOT		246 /* 0xF6 */
#define	OP_PLQUOTE	245 /* 0xF5 */
#define	OP_PSQUOTE	244 /* 0xF5 */
#define	OP_PDQUOTE	243 /* 0xF4 */
#define	OP_PAT		242 /* 0xF3 */
#define	OP_PHASH	241 /* 0xF3 */
/* math ops */
#define	OP_MEQ		240 /* 0xF2 */
#define	OP_MADD		239 /* 0xF1 */
#define	OP_MSUB		238 /* 0xF0 */
#define	OP_MMUL		237 /* 0xEF */
#define	OP_MDIV		236 /* 0xEE */
#define	OP_MADDEQ	235 /* 0xED */
#define	OP_MSUBEQ	234 /* 0xEC */
#define	OP_MMULEQ	233 /* 0xEB */
#define	OP_MDIVEQ	232 /* 0xEA */
#define	OP_MADDADD	231 /* 0xE9 */
#define	OP_MSUBSUB	230 /* 0xE8 */
#define	OP_MMOD		229 /* 0xE7 */
#define	OP_MAND		228 /* 0xE6 */
#define	OP_MOR		227 /* 0xE5 */
#define	OP_MXOR		226 /* 0xE4 */
#define	OP_MLAND	225 /* 0xE3 */
#define	OP_MLOR		224 /* 0xE2 */
#define	OP_MLNOT	223 /* 0xE1 */
#define	OP_MCEQ		222 /* 0xE0 */
#define	OP_MCEEQ	221 /* 0xDF */
#define	OP_MCNE		220 /* 0xDE */
#define	OP_MCLE		219 /* 0xDD */
#define	OP_MCGE		218 /* 0xDC */
#define	OP_MCLT		217 /* 0xDB */
#define	OP_MCGT		216 /* 0xDA */
#define OP_MSHIFTR      215 /* 0xD9 */
#define OP_MSHIFTL      214 /* 0xD8 */
#define OP_MCOLON	213 /* 0xD7 */
#define OP_MQUESTION	212 /* 0xD6 */
/* keywords */
#define	OP_KBREAK	211 /* 0xD5 */
#define	OP_KCONT	210 /* 0xD4 */
#define	OP_KRET		209 /* 0xD3 */
#define	OP_KDELETE	208 /* 0xD2 */
#define	OP_KNEW		207 /* 0xD1 */
#define	OP_KNAMESPACE   206
#define	OP_KCLASS	205 /* 0xD0 */
#define	OP_KFUNC	204 /* 0xCF */
#define	OP_KGLOB	203 /* 0xCE */
#define	OP_KLOCAL	202 /* 0xCD */
#define	OP_KVAR		201 /* 0xCC */
#define	OP_KIF		200 /* 0xCB */
#define	OP_KELSE	199 /* 0xCA */
#define	OP_KFOR		198 /* 0xC9 */
#define	OP_KFOREACH	197 /* 0xC8 */
#define	OP_KDO		196 /* 0xC7 */
#define	OP_KWHILE	195 /* 0xC6 */
#define	OP_KSWITCH	194 /* 0xC5 */
#define	OP_KCASE	193 /* 0xC4 */
#define	OP_KDEFAULT	192 /* 0xC3 */
#define	OP_KTRY		191 /* 0xC2 */
#define	OP_KCATCH	190 /* 0xC1 */
#define	OP_KFINALLY	189 /* 0xC0 */
#define	OP_KTHROW	188 /* 0xBF */
#define	OP_KEXIT	187 /* 0xBE */

#define OP_LINENUM	186 /* 0xBD */
#define OP_LABEL	185 /* 0xBC */
#define OP_STRDATA	184 /* 0xBB */
#define OP_ESTRDATA	183 /* 0xBB */
#define OP_NUMDATA	182 /* 0xBA */

#define OP_ISPUNC(o)	(o>=OP_PHASH&&o<=OP_POBRACE)
#define OP_ISMATH(o)	(o>=OP_MQUESTION&&o<=OP_MEQ)
#define OP_ISKEY(o)	(o>=OP_KEXIT&&o<=OP_KBREAK)
#define OP_ISEND(o)	(o>=OP_PSEMICOL&&o<=OP_PCBRACE)

typedef struct {
	char *name;
	uchar value;
	uchar priority;
} optab;

extern const optab oplist[];

#define nextop() N->readptr=n_seekop(N, N->readptr, 0);

#endif /* _NSP_OPCODES_H */
