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
#define	OP_KBREAK	215 /* 0xD9 */
#define	OP_KCONT	214 /* 0xD8 */
#define	OP_KRET		213 /* 0xD7 */
#define	OP_KNEW		212 /* 0xD6 */
#define	OP_KCLASS	211 /* 0xD5 */
#define	OP_KFUNC	210 /* 0xD4 */
#define	OP_KGLOB	209 /* 0xD3 */
#define	OP_KLOCAL	208 /* 0xD2 */
#define	OP_KVAR		207 /* 0xD1 */
#define	OP_KIF		206 /* 0xD0 */
#define	OP_KELSE	205 /* 0xCF */
#define	OP_KFOR		204 /* 0xCE */
#define	OP_KFOREACH	203 /* 0xCD */
#define	OP_KDO		202 /* 0xCC */
#define	OP_KWHILE	201 /* 0xCB */
#define	OP_KTRY		200 /* 0xCA */
#define	OP_KCATCH	199 /* 0xC9 */
#define	OP_KFINALLY	198 /* 0xC8 */
#define	OP_KTHROW	197 /* 0xC7 */
#define	OP_KEXIT	196 /* 0xC6 */

#define OP_LABEL	195 /* 0xC5 */
#define OP_STRDATA	194 /* 0xC4 */
#define OP_NUMDATA	193 /* 0xC3 */

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
