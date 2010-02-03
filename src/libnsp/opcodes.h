/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2010 Dan Cahill

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
#define	OP_PSQUOTE	245 /* 0xF5 */
#define	OP_PDQUOTE	244 /* 0xF4 */
#define	OP_PHASH	243 /* 0xF3 */
/* math ops */
#define	OP_MEQ		242 /* 0xF2 */
#define	OP_MADD		241 /* 0xF1 */
#define	OP_MSUB		240 /* 0xF0 */
#define	OP_MMUL		239 /* 0xEF */
#define	OP_MDIV		238 /* 0xEE */
#define	OP_MADDEQ	237 /* 0xED */
#define	OP_MSUBEQ	236 /* 0xEC */
#define	OP_MMULEQ	235 /* 0xEB */
#define	OP_MDIVEQ	234 /* 0xEA */
#define	OP_MADDADD	233 /* 0xE9 */
#define	OP_MSUBSUB	232 /* 0xE8 */
#define	OP_MMOD		231 /* 0xE7 */
#define	OP_MAND		230 /* 0xE6 */
#define	OP_MOR		229 /* 0xE5 */
#define	OP_MXOR		228 /* 0xE4 */
#define	OP_MLAND	227 /* 0xE3 */
#define	OP_MLOR		226 /* 0xE2 */
#define	OP_MLNOT	225 /* 0xE1 */
#define	OP_MCEQ		224 /* 0xE0 */
#define	OP_MCEEQ	223 /* 0xDF */
#define	OP_MCNE		222 /* 0xDE */
#define	OP_MCLE		221 /* 0xDD */
#define	OP_MCGE		220 /* 0xDC */
#define	OP_MCLT		219 /* 0xDB */
#define	OP_MCGT		218 /* 0xDA */

#define OP_MCOLON	217 /* 0xD9 */
#define OP_MQUESTION	216 /* 0xD8 */

/* keywords */
#define	OP_KBREAK	215 /* 0xD7 */
#define	OP_KCONT	214 /* 0xD6 */
#define	OP_KRET		213 /* 0xD5 */
#define	OP_KDELETE	212 /* 0xD4 */
#define	OP_KNEW		211 /* 0xD3 */
#define	OP_KCLASS	210 /* 0xD2 */
#define	OP_KFUNC	209 /* 0xD1 */
#define	OP_KGLOB	208 /* 0xD0 */
#define	OP_KLOCAL	207 /* 0xCF */
#define	OP_KVAR		206 /* 0xCE */
#define	OP_KIF		205 /* 0xCD */
#define	OP_KELSE	204 /* 0xCC */
#define	OP_KFOR		203 /* 0xCB */
#define	OP_KFOREACH	202 /* 0xCA */
#define	OP_KDO		201 /* 0xC9 */
#define	OP_KWHILE	200 /* 0xC8 */
#define	OP_KSWITCH	199 /* 0xC7 */
#define	OP_KCASE	198 /* 0xC6 */
#define	OP_KDEFAULT	197 /* 0xC5 */
#define	OP_KTRY		196 /* 0xC4 */
#define	OP_KCATCH	195 /* 0xC3 */
#define	OP_KFINALLY	194 /* 0xC2 */
#define	OP_KTHROW	193 /* 0xC1 */
#define	OP_KEXIT	192 /* 0xC0 */

#define OP_LABEL	191 /* 0xBF */
#define OP_STRDATA	190 /* 0xBE */
#define OP_NUMDATA	189 /* 0xBD */

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

#define  nextop()       N->readptr=n_seekop(N, N->readptr, 1, 0);

#endif /* _NSP_OPCODES_H */
