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
#ifndef _NESLA_H
#define _NESLA_H 1

#if defined(TINYCC)||defined(__TURBOC__)
struct timeval { long tv_sec; long tv_usec; };
#endif
#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
/* always include winsock2 before windows */
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#elif !defined(__TURBOC__)
#include <sys/time.h>
#endif
#include <setjmp.h>

#define NESLA_NAME      "nesla"
#define NESLA_VERSION   "0.9.0"

#define MAX_OBJNAMELEN  64
#define MAX_OUTBUFLEN   8192
#define OUTBUFLOWAT	4096

/* nesla object types */
#define NT_NULL         0
#define NT_BOOLEAN      1
#define NT_NUMBER       2
#define NT_STRING       3
#define NT_NFUNC        4
#define NT_CFUNC        5
#define NT_TABLE        6
#define NT_CDATA        7

/* nesla object status flags */
#define NST_HIDDEN	0x01
#define NST_READONLY	0x02
#define NST_SYSTEM	0x04
#define NST_AUTOSORT	0x08
#define NST_LINK	0x10

#define num_t double
#define uchar unsigned char
#define obj_t struct nes_objrec
#define val_t struct nes_valrec
#define nes_t struct nes_state

/* should be typedef int(*NES_CFUNC)(nes_state *); */
typedef int(*NES_CFUNC)(void *);
#define NES_FUNCTION(name) int name(nes_state *N)

/*
 * define a callback function type so CDATA objects
 * can choose the terms of their own death.
 */
/* should be typedef void(*NES_CFREE)(nes_state *, obj_t *); */
typedef void(*NES_CFREE)(void *, void *);

typedef struct NES_CDATA {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NES_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's type-specific */
} NES_CDATA;
typedef struct nes_valrec {
	unsigned short type; /* val type */
	unsigned short attr; /* status flags (hidden, readonly, system, autosort, etc...) */
	unsigned short refs; /* number of references to this node */
	unsigned int   size; /* storage size of string, nfunc or cdata */
	union {
		num_t  num;
		char  *str;
		NES_CFUNC cfunc;
		NES_CDATA *cdata;
		obj_t *table;
	} d;
} nes_valrec;
typedef struct nes_objrec {
	obj_t *prev;
	obj_t *next;
	val_t *val;
	char name[MAX_OBJNAMELEN+1];
} nes_objrec;
typedef struct nes_state {
	uchar *blockptr;
	uchar *blockend;
	uchar *readptr;
	obj_t g;
	obj_t l;
	obj_t r;
	short int brk;
	short int cnt;
	short int ret;
	short int err;
	short int debug;
	short int single;
	short int strict;
	short int warnings;
	jmp_buf *savjmp;
	struct timeval ttime;
	unsigned short int outbuflen;
	char numbuf[128];
	char outbuf[MAX_OUTBUFLEN+1];
	char errbuf[256];
} nes_state;

#ifndef NESLA_NOFUNCTIONS
/* exec */
nes_state *nes_newstate   (void);
nes_state *nes_endstate   (nes_state *N);
obj_t     *nes_exec       (nes_state *N, const char *string);
int        nes_execfile   (nes_state *N, char *file);
/* objects */
void       nes_setvaltype (nes_state *N, obj_t *cobj, unsigned short type);
void       nes_linkval    (nes_state *N, obj_t *cobj1, obj_t *cobj2);
void       nes_unlinkval  (nes_state *N, obj_t *cobj);
void       nes_freetable  (nes_state *N, obj_t *tobj);
obj_t     *nes_getobj     (nes_state *N, obj_t *tobj, char *oname);
obj_t     *nes_getiobj    (nes_state *N, obj_t *tobj, int oindex);
obj_t     *nes_setobj     (nes_state *N, obj_t *tobj, char *oname, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, int _slen);
void       nes_strcat     (nes_state *N, obj_t *cobj, char *str, int len);
void       nes_strmul     (nes_state *N, obj_t *cobj, int n);
short      nes_tobool     (nes_state *N, obj_t *cobj);
num_t      nes_tonum      (nes_state *N, obj_t *cobj);
char      *nes_tostr      (nes_state *N, obj_t *cobj);
/* parser */
obj_t     *nes_eval       (nes_state *N, const char *string);
obj_t     *nes_evalf      (nes_state *N, const char *fmt, ...);
#endif

#define    nes_isnull(o)            (o==NULL||o->val==NULL||o->val->type==NT_NULL)
#define    nes_isbool(o)            (o!=NULL&&o->val!=NULL&&o->val->type==NT_BOOLEAN)
#define    nes_isnum(o)             (o!=NULL&&o->val!=NULL&&o->val->type==NT_NUMBER)
#define    nes_isstr(o)             (o!=NULL&&o->val!=NULL&&o->val->type==NT_STRING)
#define    nes_istable(o)           (o!=NULL&&o->val!=NULL&&o->val->type==NT_TABLE)

#define    nes_istrue(o)            nes_tobool(N, o)?1:0

#define    nes_typeof(o)            nes_isnull(o)?NT_NULL:o->val->type

#define    nes_getnum(N,o,n)        nes_tonum(N, nes_getobj(N,o,n))
#define    nes_getstr(N,o,n)        nes_tostr(N, nes_getobj(N,o,n))

#define    nes_setnum(N,t,n,v)      nes_setobj(N, t, n, NT_NUMBER, (NES_CFUNC)NULL, v, NULL, 0)
#define    nes_setstr(N,t,n,s,l)    nes_setobj(N, t, n, NT_STRING, (NES_CFUNC)NULL, 0, s,    l)
#define    nes_settable(N,t,n)      nes_setobj(N, t, n, NT_TABLE,  (NES_CFUNC)NULL, 0, NULL, 0)
#define    nes_setcfunc(N,t,n,p)    nes_setobj(N, t, n, NT_CFUNC,  (NES_CFUNC)p,    0, NULL, 0)
#define    nes_setnfunc(N,t,n,s,l)  nes_setobj(N, t, n, NT_NFUNC,  (NES_CFUNC)NULL, 0, s,    l)
#define    nes_setcdata(N,t,n,s,l)  nes_setobj(N, t, n, NT_CDATA,  (NES_CFUNC)NULL, 0, (void *)s, l)

#define    nes_setinum(N,t,n,v)     nes_setiobj(N, t, n, NT_NUMBER, (NES_CFUNC)NULL, v, NULL, 0)
#define    nes_setistr(N,t,n,s,l)   nes_setiobj(N, t, n, NT_STRING, (NES_CFUNC)NULL, 0, s,    l)
#define    nes_setitable(N,t,n)     nes_setiobj(N, t, n, NT_TABLE,  (NES_CFUNC)NULL, 0, NULL, 0)
#define    nes_seticfunc(N,t,n,p)   nes_setiobj(N, t, n, NT_CFUNC,  (NES_CFUNC)p,    0, NULL, 0)
#define    nes_setinfunc(N,t,n,s,l) nes_setiobj(N, t, n, NT_NFUNC,  (NES_CFUNC)NULL, 0, s,    l)
#define    nes_seticdata(N,t,n,s,l) nes_setiobj(N, t, n, NT_CDATA,  (NES_CFUNC)NULL, 0, (void *)s, l)

#endif /* nesla.h */
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
#ifndef _LIBNESLA_H
#define _LIBNESLA_H 1


#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef WIN32
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

/* error numbers */
#define NE_MEM		1
#define NE_SYNTAX	2
#define NE_INTERNAL	3

#define sanetest()	/*{ if (N->readptr==NULL) n_error(N, NE_SYNTAX, __FUNCTION__, "NULL readptr"); }*/
#define DEBUG_IN()
#define DEBUG_OUT()

/* block.c */
void     n_skipto       (nes_state *N, const char *fn, unsigned short c);
void     n_if           (nes_state *N);
void     n_for          (nes_state *N);
void     n_do           (nes_state *N);
void     n_while        (nes_state *N);
void     n_try          (nes_state *N);
/* compile.c */
uchar   *n_decompose    (nes_state *N, uchar *rawtext);
void     n_decompile    (nes_state *N);
/* exec.c */
obj_t   *n_execfunction (nes_state *N, obj_t *fobj, obj_t *pobj);
/* lib.c */
NES_FUNCTION(nl_flush);
NES_FUNCTION(nl_print);
NES_FUNCTION(nl_write);
NES_FUNCTION(nl_fileread);
NES_FUNCTION(nl_filestat);
NES_FUNCTION(nl_fileunlink);
NES_FUNCTION(nl_filewrite);
NES_FUNCTION(nl_math1);
NES_FUNCTION(nl_tonumber);
NES_FUNCTION(nl_tostring);
NES_FUNCTION(nl_strcat);
NES_FUNCTION(nl_strcmp);
NES_FUNCTION(nl_strjoin);
NES_FUNCTION(nl_strlen);
NES_FUNCTION(nl_strsplit);
NES_FUNCTION(nl_strstr);
NES_FUNCTION(nl_strsub);
NES_FUNCTION(nl_strtolower);
NES_FUNCTION(nl_sqltime);
NES_FUNCTION(nl_time);
NES_FUNCTION(nl_gmtime);
NES_FUNCTION(nl_sleep);
NES_FUNCTION(nl_runtime);
NES_FUNCTION(nl_iname);
NES_FUNCTION(nl_ival);
NES_FUNCTION(nl_include);
NES_FUNCTION(nl_printvar);
NES_FUNCTION(nl_sizeof);
NES_FUNCTION(nl_sort_name);
NES_FUNCTION(nl_sort_key);
NES_FUNCTION(nl_typeof);
NES_FUNCTION(nl_system);
/* libc.c */
#define  nc_isdigit(c)  ((c>='0'&&c<='9')?1:0)
#define  nc_isalpha(c)  ((c>='A'&&c<='Z')||(c>='a'&&c<='z')?1:0)
#define  nc_isalnum(c)  ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')?1:0)
#define  nc_isupper(c)  ((c>='A'&&c<='Z')?1:0)
#define  nc_islower(c)  ((c>='a'&&c<='z')?1:0)
#define  nc_isspace(c)  (c=='\r'||c=='\n'||c=='\t'||c==' ')
#define  nc_tolower(c)  ((c>='A'&&c<='Z')?(c+('a'-'A')):c)
#define  nc_toupper(c)  ((c>='a'&&c<='z')?(c-('a'-'A')):c)
int      nc_snprintf    (nes_state *N, char *str, int size, const char *format, ...);
int      nc_printf      (nes_state *N, const char *format, ...);
int      nc_gettimeofday(struct timeval *tv, void *tz);
char    *nc_memcpy      (char *dst, const char *src, int n);
int      nc_strlen      (const char *s);
char    *nc_strchr      (const char *s, int c);
char    *nc_strncpy     (char *d, const char *s, int n);
int      nc_strcmp      (const char *s1, const char *s2);
int      nc_strncmp     (const char *s1, const char *s2, int n);
void    *nc_memset      (void *s, int c, int n);

void     n_error        (nes_state *N, short int err, const char *fname, const char *format, ...);
void     n_expect       (nes_state *N, const char *fname, uchar op);
void     n_warn         (nes_state *N, const char *fname, const char *format, ...);
/* libc */
num_t    n_aton         (nes_state *N, const char *str);
char    *n_ntoa         (nes_state *N, char *str, num_t num, short base, unsigned short dec);
/* obj.c */
void    *n_alloc        (nes_state *N, int size, short zero);
void    *n_realloc      (nes_state *N, void **p, int size, short zero);
void     n_free         (nes_state *N, void **p);
void     n_copyval      (nes_state *N, obj_t *cobj1, obj_t *cobj2);
val_t   *n_newval       (nes_state *N, unsigned short type);
void     n_freeval      (nes_state *N, obj_t *cobj);
obj_t   *n_newiobj      (nes_state *N, int index);
/* opcode.c */
short    n_getop        (nes_state *N, char *name);
char    *n_getsym       (nes_state *N, short op);
uchar   *n_seekop       (nes_state *N, uchar *readptr, int ops, int sb);
/* parse.c */
obj_t   *n_getfunction  (nes_state *N);
char    *n_getlabel     (nes_state *N, char *buf);
num_t    n_getnumber    (nes_state *N);
obj_t   *n_getstring    (nes_state *N);
obj_t   *n_readindex    (nes_state *N, obj_t *cobj, char *lastname);
obj_t   *n_readtable    (nes_state *N, obj_t *tobj);
obj_t   *n_readvar      (nes_state *N, obj_t *tobj, obj_t *cobj);
obj_t   *n_storeval     (nes_state *N, obj_t *cobj);

#define  readi4(ptr)    (int)(ptr[0]+ptr[1]*256+ptr[2]*65536+ptr[3]*16777216)
#define  readi2(ptr)    (int)(ptr[0]+ptr[1]*256)

#define  writei4(n,ptr) ptr[0]=n&255; ptr[1]=(n>>8)&255; ptr[2]=(n>>16)&255; ptr[3]=(n>>24)&255;
#define  writei2(n,ptr) ptr[0]=n&255; ptr[1]=(n>>8)&255;

#define striprn(s) { int n=strlen(s)-1; while (n>-1&&(s[n]=='\r'||s[n]=='\n')) s[n--]='\0'; }

#endif /* libnesla.h */
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
#define	OP_MCNE		223 /* 0xDF */
#define	OP_MCLE		222 /* 0xDE */
#define	OP_MCGE		221 /* 0xDD */
#define	OP_MCLT		220 /* 0xDC */
#define	OP_MCGT		219 /* 0xDB */
/* keywords */
#define	OP_KBREAK	218 /* 0xDA */
#define	OP_KCONT	217 /* 0xD9 */
#define	OP_KRET		216 /* 0xD8 */
#define	OP_KFUNC	215 /* 0xD7 */
#define	OP_KGLOB	214 /* 0xD6 */
#define	OP_KLOCAL	213 /* 0xD5 */
#define	OP_KVAR		212 /* 0xD4 */
#define	OP_KIF		211 /* 0xD3 */
#define	OP_KELSE	210 /* 0xD2 */
#define	OP_KFOR		209 /* 0xD1 */
#define	OP_KDO		208 /* 0xD0 */
#define	OP_KWHILE	207 /* 0xCF */
#define	OP_KTRY		206 /* 0xCE */
#define	OP_KCATCH	205 /* 0xCD */
#define	OP_KEXIT	204 /* 0xCC */

#define OP_LABEL	203 /* 0xCB */
#define OP_STRDATA	202 /* 0xCA */
#define OP_NUMDATA	201 /* 0xC9 */

#define OP_ISPUNC(o)	(o>=OP_PHASH&&o<=OP_POBRACE)
#define OP_ISMATH(o)	(o>=OP_MCGT&&o<=OP_MEQ)
#define OP_ISKEY(o)	(o>=OP_KEXIT&&o<=OP_KBREAK)
#define OP_ISEND(o)	(o>=OP_PSEMICOL&&o<=OP_PCBRACE)

typedef struct {
	char *name;
	uchar value;
	uchar priority;
} optab;

extern const optab oplist[];

#define  nextop()       N->readptr=n_seekop(N, N->readptr, 1, 0);
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



/* Advance readptr to next specified char */
void n_skipto(nes_state *N, const char *fn, unsigned short c)
{
	while (*N->readptr) {
		if (*N->readptr==c) return;
		N->readptr=n_seekop(N, N->readptr, 1, 1);
	}
	n_error(N, NE_SYNTAX, fn, "expected a '%s'", n_getsym(N, c));
	return;
}

void n_if(nes_state *N)
{
#define __FUNCTION__ "n_if"
	char done=0, t=0;
	obj_t *cobj;

	DEBUG_IN();
l1:
	if (*N->readptr==OP_POPAREN) {
		N->readptr++;
		if (!done) {
			cobj=nes_eval(N, (char *)N->readptr);
			t=nes_tonum(N, cobj)?1:0;
		}
		n_skipto(N, __FUNCTION__, OP_PCPAREN);
		N->readptr++;
	}
l2:
	if (*N->readptr==OP_POBRACE) {
		if ((t)&&(!done)) {
			done=1;
			nes_exec(N, (char *)N->readptr);
		} else {
			N->readptr++;
		}
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		N->readptr++;
		if (N->ret) { DEBUG_OUT(); return; }
	} else {
		if ((t)&&(!done)) {
			done=1;
			N->single=1;
			nes_exec(N, (char *)N->readptr);
		} else {
			n_skipto(N, __FUNCTION__, OP_PSEMICOL);
		}
		if (*N->readptr==OP_PSEMICOL) N->readptr++;
		if (N->ret) { DEBUG_OUT(); return; }
	}
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
#define __FUNCTION__ "n_for"
	uchar *arginit, *argcomp, *argexec;
	uchar *bs, *be;
	obj_t *cobj;
	short int single;

	DEBUG_IN();
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
#define __FUNCTION__ "n_do"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short int single;

	DEBUG_IN();
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
#define __FUNCTION__ "n_while"
	uchar *argcomp;
	uchar *bs, *be;
	obj_t *cobj;
	short int single;

	DEBUG_IN();
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
#define __FUNCTION__ "n_try"
	jmp_buf *savjmp;
	uchar *bs, *be;
	obj_t *tobj;

	DEBUG_IN();
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



/*
 * calling this a compiler is something of a misnomer. it does not _compile_
 * code from multiple sources.  it tokenizes and optimizes source chunks.  that's all.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef struct cstate {
	uchar *destbuf;
	int destmax;
	obj_t *tobj1;
	obj_t *lobj1;
	int index;
	int lineno;
	int offset;
} cstate;

static int n_unescape(nes_state *N, char *src, char *dst, int len, cstate *state)
{
	int i, n;
	short e=0;

	if (dst==NULL) return 0;
	for (i=0,n=0;i<len;i++) {
		if (!e) {
			if (src[i]=='\\') {
				e=1;
			} else {
				dst[n++]=src[i];
			}
			continue;
		}
		switch (src[i]) {
			case 'a'  : dst[n++]='\a'; break;
			case 't'  : dst[n++]='\t'; break;
			case 'f'  : dst[n++]='\f'; break;
			case 'e'  : dst[n++]=27;   break;
			case 'r'  : dst[n++]='\r'; break;
			case 'n'  : dst[n++]='\n'; break;
			case '\'' : dst[n++]='\''; break;
			case '\"' : dst[n++]='\"'; break;
			case '\\' : dst[n++]='\\'; break;
			default   : break;
		}
		e=0;
	}
	dst[n]='\0';
	return n;
}

static obj_t *n_newobj(nes_state *N, cstate *state)
{
	if (state->lobj1==NULL) {
		state->lobj1=state->tobj1->val->d.table=n_newiobj(N, state->index);
	} else {
		state->lobj1->next=n_newiobj(N, state->index);
		state->lobj1->next->prev=state->lobj1;
		state->lobj1=state->lobj1->next;
	}
	state->index++;
	return state->lobj1;
}

/* Advance readptr to next non-blank */
static void n_skipblank(nes_state *N, cstate *state)
{
	uchar *p=N->readptr;

	while (*p) {
		if (p[0]=='#') {
			p++;
			while (*p) {
				if (*p=='\n') { state->lineno++; break; }
				else if (*p=='\r') break;
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='/') {
			p+=2;
			while (*p) {
				if (*p=='\n') { state->lineno++; break; }
				else if (*p=='\r') break;
				p++;
			}
		} else if (p[0]=='/'&&p[1]=='*') {
			p+=2;
			while (*p) {
				if (*p=='\n') state->lineno++;
				else if (p[0]=='*'&&p[1]=='/') {
					p+=2;
					if (*p=='\n') state->lineno++;
					break;
				}
				p++;
			}
		} else {
			if (*p=='\n') state->lineno++;
		}
		if (!nc_isspace(*p)) break;
		p++;
	}
	N->readptr=p;
	return;
}

/* Advance readptr to next matching quote */
static void n_skipquote(nes_state *N, unsigned short c)
{
	while (*N->readptr) {
		if (*N->readptr=='\\') {
			N->readptr++;
		} else if (*N->readptr==c) {
			N->readptr++;
			break;
		}
		N->readptr++;
		if (!*N->readptr) n_error(N, NE_SYNTAX, "n_skipquote", "unterminated string");
	}
	return;
}

/* return the next quoted block */
static obj_t *n_extractquote(nes_state *N, cstate *state)
{
	obj_t *cobj;
	char q=*N->readptr;
	char *qs, *qe;

	DEBUG_IN();
	sanetest();
	if ((q!='\'')&&(q!='\"')) {
		DEBUG_OUT();
		return NULL;
	}
	N->readptr++;
	qs=(char *)N->readptr;
	n_skipquote(N, q);
	qe=(char *)N->readptr;
	cobj=nes_setstr(N, &N->r, "", NULL, qe-qs-1);
	cobj->val->size=n_unescape(N, qs, cobj->val->d.str, qe-qs-1, state);
	DEBUG_OUT();
	return cobj;
}

static void n_decompose_sub(nes_state *N, cstate *state)
{
	char lastname[MAX_OBJNAMELEN+1];
	char *p;
	short op;
	int len;

	while (*N->readptr) {
		n_skipblank(N, state);
		op=n_getop(N, lastname);
		if (op==OP_UNDEFINED) {
			n_warn(N, "x", "bad op? index=%d line=%d op=%d:%d name='%s'", state->index, state->lineno, op, N->readptr[0], lastname);
/*
			obj_t *cobj;

			n_warn(N, "x", "bad op? index=%d op=%d:%d name='%s'", state->index, op, N->readptr[0], lastname);
			for (cobj=state->tobj1->val->d.table;cobj;cobj=cobj->next) {
				cobj->val->attr=0;
			}
			n_dumpvars(N, &N->g, 0);
			n_error(N, NE_SYNTAX, "n_decompose_sub", "bad op");
*/
			return;
		}

		if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
/*
			if (op==OP_PCPAREN) return;
			if (op==OP_PCBRACE) return;
*/
			n_newobj(N, state);
			p=n_getsym(N, op);
			len=nc_strlen(p);
			nes_setstr(N, state->lobj1, NULL, p, len);
			state->lobj1->val->attr=op;
/*
			if (op==OP_POBRACE) {
				nobj=nes_settable(N, tobj, n_ntoa(N, x, *index, 10, 0));
				*index+=1;
				while (N->lastop!=OP_PCBRACE) {
					n_decompose_sub(N, rawtext, nobj, index);
				}
				if (N->lastop==OP_PCBRACE) {
					nes_setstr(N, tobj, n_ntoa(N, x, *index, 10, 0), lastname, -1);
					*index+=1;
				}
			}

			if (op==OP_POPAREN) {
				nobj=nes_settable(N, tobj, n_ntoa(N, x, *index, 10, 0));
				*index+=1;
				while (N->lastop!=OP_PCPAREN) {
					n_decompose_sub(N, rawtext, nobj, index);
				}
				if (N->lastop==OP_PCPAREN) {
					nes_setstr(N, tobj, n_ntoa(N, x, *index, 10, 0), lastname, -1);
					*index+=1;
				}
			}
*/
		} else if (op==OP_LABEL) {
			n_newobj(N, state);
			len=nc_strlen(lastname);
			nes_setstr(N, state->lobj1, NULL, lastname, len);
			state->lobj1->val->attr=op;
		} else if (nc_isdigit(*N->readptr)) {
			p=(char *)N->readptr;
			while (nc_isdigit(*N->readptr)||*N->readptr=='.') N->readptr++;
			n_newobj(N, state);
			len=(char *)N->readptr-p;
			nes_setstr(N, state->lobj1, NULL, p, len);
			state->lobj1->val->attr=OP_NUMDATA;
		}
x:
		if (!*N->readptr) return;
		n_skipblank(N, state);
		if ((*N->readptr=='\"')||(*N->readptr=='\'')) {
			n_newobj(N, state);
			nes_linkval(N, state->lobj1, n_extractquote(N, state));
			state->lobj1->val->attr=OP_STRDATA;
			len=state->lobj1->val->size;
			goto x;
		} else if (nc_isdigit(*N->readptr)) {
			p=(char *)N->readptr;
			while (nc_isdigit(*N->readptr)||*N->readptr=='.') N->readptr++;
			n_newobj(N, state);
			len=(char *)N->readptr-p;
			nes_setstr(N, state->lobj1, NULL, p, len);
			state->lobj1->val->attr=OP_NUMDATA;
			goto x;
		}
	}
	return;
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define testgrow(b) \
	if (state.offset+b+1>state.destmax) { \
		state.destmax+=b+1024; \
		state.destbuf=n_realloc(N, (void *)&state.destbuf, state.destmax, 0); \
		if (state.destbuf==NULL) n_warn(N, __FUNCTION__, "realloc error!!!"); \
	}

uchar *n_decompose(nes_state *N, uchar *rawtext)
{
#define __FUNCTION__ "n_decompose"
	cstate state;
	obj_t *cobj, *tobj;
	unsigned short op;

	if ((rawtext[0]==0x0D)&&((rawtext[1]==0xAC))) {
		n_warn(N, __FUNCTION__, "already chewed on this");
		return rawtext;
	}
	nc_memset((char *)&state, 0, sizeof(state));
	state.lineno=1;
	state.destmax=1024;
	state.destbuf=malloc(state.destmax);
	N->readptr=rawtext;
	tobj=nes_settable(N, &N->g, "decomped_script");
	nes_freetable(N, tobj);
	state.tobj1=nes_settable(N, tobj, "code");
	n_decompose_sub(N, &state);
	/* header - 8 bytes */
	testgrow(8); /* safe portable use of sprintf is still considered dangerous according to openbsd */
	state.destbuf[state.offset++]=0x0D;
	state.destbuf[state.offset++]=0xAC;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* file size - 4 bytes */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* optab offset  - 4 bytes (little endian) */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* symtab offset - 4 bytes (little endian) */
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	state.destbuf[state.offset++]=0;
	/* now write the ops */
	/* optab offset */
	writei4(state.offset, (state.destbuf+12));
	for (cobj=state.tobj1->val->d.table;cobj;cobj=cobj->next) {
		op=(unsigned short)cobj->val->attr;
		cobj->val->attr=0;
		if (!nes_isstr(cobj)) break;
		if (op==OP_UNDEFINED) break;
		if (op==OP_STRDATA) {
			testgrow((int)(6+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			writei4(cobj->val->size, (state.destbuf+state.offset));
			state.offset+=4;
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_NUMDATA) {
			testgrow((int)(3+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			state.destbuf[state.offset++]=cobj->val->size&255;
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]=0;
		} else if (op==OP_LABEL) {
			testgrow((int)(3+cobj->val->size));
			state.destbuf[state.offset++]=op&255;
			state.destbuf[state.offset++]=cobj->val->size&255;
			nc_memcpy((char *)state.destbuf+state.offset, cobj->val->d.str, cobj->val->size);
			state.offset+=cobj->val->size;
			state.destbuf[state.offset++]='\0';
		} else if (OP_ISMATH(op)||OP_ISKEY(op)||OP_ISPUNC(op)) {
			testgrow(1);
			state.destbuf[state.offset++]=op&255;
		} else {
			n_warn(N, __FUNCTION__, "bad op?");
		}
	}
	/* file size */
	writei4(state.offset, (state.destbuf+8));
	/* add some trailing nulls for fun... */
	testgrow(4); writei4(0, (state.destbuf+state.offset));
	state.offset+=4;
	/* n_dumpvars(N, &N->g, 0); */
	nes_freetable(N, tobj);
	return state.destbuf;
#undef __FUNCTION__
}
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



#include <stdio.h>

void dumpsyms(nes_state *N, char *file, int line, uchar *ptr, int count)
{
#define __FUNCTION__ "dumpsyms"
	int i=0;

	n_warn(N, __FUNCTION__, "%s:%d next %d ops 0x%08x [%d]", file, line, count, N->readptr, *N->readptr);
	do {
		switch (*ptr) {
		case 0          : n_warn(N, __FUNCTION__, "EOF"); return;
		case OP_LABEL   : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][label][%s]", i, ptr, *ptr, ptr+2); break;
		case OP_NUMDATA : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][num  ][%s]", i, ptr, *ptr, ptr+2); break;
		case OP_STRDATA : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][str  ][%s]", i, ptr, *ptr, ptr+5); break;
		default         : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][op   ][%s]", i, ptr, *ptr, n_getsym(N, *ptr));
		}
		N->warnings--;
		count--;
		i++;
		ptr=n_seekop(N, ptr, 1, 0);
	} while (count);
#undef __FUNCTION__
}

void n_decompile(nes_state *N)
{
#define __FUNCTION__ "n_decompile"
	uchar *p;
	int len;
	uchar *offset=NULL;
	int showbold;

	nl_flush(N);
	if (N->blockptr==NULL) { printf(" N->blockptr is NULL\n"); return; }
	if (N->readptr==NULL) { printf(" N->readptr is NULL\n"); return; }
	if (N->blockend==NULL) { printf(" N->blockend is NULL\n"); return; }
	printf("\n----\nrecomposed source is:\n\n");
	printf(" 0x%08X\n 0x%08X <-you are here\n 0x%08X\n\n", (unsigned int)N->blockptr, (unsigned int)N->readptr, (unsigned int)N->blockend);
	if (N->readptr>N->blockend) {
		printf(" N->readptr is %d bytes past the end of the block\n\n", (int)(N->readptr-N->blockend));
		N->blockptr=N->readptr;
	} else if (N->readptr<N->blockptr) {
		printf(" N->readptr is %d bytes before the block\n\n", (int)(N->blockptr-N->readptr));
		N->blockptr=N->readptr;
	} else {
		offset=N->readptr;
	}
	p=N->blockptr+readi4((N->blockptr+12));
	for (;*p;p++) {
		showbold=0;
		if ((offset)&&(p>=offset)) {
			showbold=1;
			printf("[01;33;40m[->");
		}
		switch (*p) {
		case OP_STRDATA:
			len=readi4((p+1));
			p+=5;
			printf("\"%s\" ", p);
			p+=len;
			break;
		case OP_NUMDATA:
			len=(int)(p[1]);
			p+=2;
			printf("%s ", p);
			p+=len;
			break;
		case OP_LABEL  :
			len=(int)(p[1]);
			p+=2;
			printf("%s ", p);
			p+=len;
			break;
		default:
			printf("%s ", n_getsym(N, *p));
		}
		if (showbold) {
			printf("<-][00m");
			offset=NULL;
		}
	}
	printf("\n\n----\n");
	return;
#undef __FUNCTION__
}
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



#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
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

obj_t *n_execfunction(nes_state *N, obj_t *fobj, obj_t *pobj)
{
#define __FUNCTION__ "n_execfunction"
	obj_t listobj;
	obj_t *cobj, *nobj;
	val_t *olobj;
	unsigned short i;
	int e;

	DEBUG_IN();
	if (fobj->val->type!=NT_CFUNC&&fobj->val->type!=NT_NFUNC) {
		n_error(N, NE_SYNTAX, __FUNCTION__, "'%s' is not a function", fobj->name);
	}
	n_expect(N, __FUNCTION__, OP_POPAREN);
	listobj.val=n_newval(N, NT_TABLE);
	cobj=nes_setstr(N, &listobj, "0", fobj->name, -1);
	for (i=1;;i++) {
		N->readptr++;
		if (*N->readptr==OP_PCPAREN) break;
		if ((nobj=nes_eval(N, (char *)N->readptr))!=NULL) {
			cobj->next=n_alloc(N, sizeof(obj_t), 0);
			cobj->next->prev=cobj;
			cobj->next->next=NULL;
			cobj=cobj->next;
			cobj->val=NULL;
			n_ntoa(N, cobj->name, i, 10, 0);
			nes_linkval(N, cobj, nobj);
		}
		if (*N->readptr==OP_PCOMMA) continue;
		n_expect(N, __FUNCTION__, OP_PCPAREN);
		break;
	}
	if (pobj) {
		cobj->next=n_alloc(N, sizeof(obj_t), 0);
		cobj->next->prev=cobj;
		cobj->next->next=NULL;
		cobj=cobj->next;
		cobj->val=NULL;
		nc_strncpy(cobj->name, "this", MAX_OBJNAMELEN);
		nes_linkval(N, cobj, pobj);
	}
	N->readptr++;
	olobj=N->l.val; N->l.val=listobj.val; listobj.val=NULL;
	/* if (N->debug) n_warn(N, __FUNCTION__, "%s()", fobj->name); */
	nes_unlinkval(N, &N->r);
	if (fobj->val->type==NT_CFUNC) {
		jmp_buf *savjmp;

		savjmp=N->savjmp;
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 0);
		e=setjmp(*N->savjmp);
		if (!e) {
			fobj->val->d.cfunc(N);
		}
		n_free(N, (void *)&N->savjmp);
		N->savjmp=savjmp;
		nes_unlinkval(N, &N->l);
		N->l.val=olobj;
	} else {
		uchar *oldbptr=N->blockptr;
		uchar *oldrptr=N->readptr;
		uchar *oldbend=N->blockend;
		jmp_buf *savjmp;

		N->blockptr=(uchar *)fobj->val->d.str;
		N->readptr=(uchar *)fobj->val->d.str;
		N->blockend=(uchar *)fobj->val->d.str+fobj->val->size;
		n_expect(N, __FUNCTION__, OP_POPAREN);
		for (i=1;;i++) {
			N->readptr++;
			cobj=nes_getiobj(N, &N->l, i);
			if (*N->readptr==OP_LABEL) n_getlabel(N, cobj->name);
			if (*N->readptr==OP_PCOMMA) continue;
			n_expect(N, __FUNCTION__, OP_PCPAREN);
			N->readptr++;
			break;
		}
		n_expect(N, __FUNCTION__, OP_POBRACE);
		savjmp=N->savjmp;
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 0);
		e=setjmp(*N->savjmp);
		if (!e) {
			nes_exec(N, (char *)N->readptr);
		}
		n_free(N, (void *)&N->savjmp);
		N->savjmp=savjmp;
		nes_unlinkval(N, &N->l);
		N->l.val=olobj;
		if (N->ret) N->ret=0;
//		if (!e) n_skipto(N, __FUNCTION__, OP_PCBRACE);
		N->blockptr=oldbptr;
		N->readptr=oldrptr;
		N->blockend=oldbend;
	}
	if (e) {
		// propagate exceptions before jumping?
		if (N->savjmp!=NULL) longjmp(*N->savjmp, 1);
	}
	DEBUG_OUT();
	return &N->r;
#undef __FUNCTION__
}

/*
 * the following functions are public API functions
 */

obj_t *nes_exec(nes_state *N, const char *string)
{
#define __FUNCTION__ "nes_exec"
	char namebuf[MAX_OBJNAMELEN+1];
	obj_t *cobj, *tobj;
	uchar block, ctype, op;
	uchar jmp=N->savjmp?1:0, single=(uchar)N->single;
	uchar *p;

	DEBUG_IN();
	N->single=0;
	if (jmp==0) {
		nes_unlinkval(N, &N->r);
		if (string==NULL||string[0]==0) goto end;
		p=n_decompose(N, (uchar *)string);
		if (p!=N->blockptr) {
			N->blockptr=p;
		} else {
			p=NULL;
		}
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)!=0) goto end;
	} else {
		N->readptr=(uchar *)string;
	}
	if (N->readptr==NULL) goto end;
	if (*N->readptr==OP_POBRACE) {
		N->readptr++;
		block=1;
	} else block=0;
	while (*N->readptr) {
		if (block&&(N->brk>0||N->cnt>0||*N->readptr==OP_PCBRACE)) goto end;
		if (*N->readptr==OP_PCBRACE) {
			N->readptr++;
			goto endstmt;
		} else if (OP_ISMATH(*N->readptr)) {
			n_warn(N, __FUNCTION__, "unexpected math op '%s'", n_getsym(N, *N->readptr));
			N->readptr++;
		} else if (OP_ISPUNC(*N->readptr)) {
			n_warn(N, __FUNCTION__, "unexpected punctuation '%s'", n_getsym(N, *N->readptr));
			N->readptr++;
		} else if (OP_ISKEY(*N->readptr)) {
			op=*N->readptr++;
			switch (op) {
			case OP_KLOCAL:
			case OP_KVAR:   n_readvar(N, &N->l, NULL); goto endstmt;
			case OP_KGLOB:  n_readvar(N, &N->g, NULL); goto endstmt;
			case OP_KIF:    n_if(N);    if (N->ret) goto end; else goto endstmt;
			case OP_KFOR:   n_for(N);   if (N->ret) goto end; else goto endstmt;
			case OP_KDO:    n_do(N);    if (N->ret) goto end; else goto endstmt;
			case OP_KWHILE: n_while(N); if (N->ret) goto end; else goto endstmt;
			case OP_KTRY:   n_try(N);   if (N->ret) goto end; else goto endstmt;
			case OP_KFUNC:  
				n_expect(N, __FUNCTION__, OP_LABEL);
				cobj=nes_setnfunc(N, &N->g, n_getlabel(N, NULL), NULL, 0);
				nes_linkval(N, cobj, n_getfunction(N));
				goto endstmt;
			case OP_KBREAK:
				if ((!block)&&(!single)) n_error(N, NE_SYNTAX, __FUNCTION__, "break without block");
				N->brk=(*N->readptr==OP_NUMDATA)?(int)n_ntoa(N, namebuf, n_getnumber(N), 10, 0):1;
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				goto end;
			case OP_KCONT:
				if ((!block)&&(!single)) n_error(N, NE_SYNTAX, __FUNCTION__, "continue without block");
				N->cnt=1;
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				goto end;
			case OP_KRET:
				n_storeval(N, &N->r);
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				N->ret=1;
				goto end;
			case OP_KEXIT:
				N->err=(*N->readptr==OP_NUMDATA)?(int)n_ntoa(N, namebuf, n_getnumber(N), 10, 0):0;
				n_error(N, N->err, __FUNCTION__, "exiting normally");
			case OP_KELSE:  n_error(N, NE_SYNTAX, __FUNCTION__, "stray else");
			}
		} else {
			obj_t *pobj=NULL;

			if (*N->readptr!=OP_LABEL) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a label [%d][%s]", *N->readptr, n_getsym(N, *N->readptr));
			tobj=&N->l;
			n_getlabel(N, namebuf);
			cobj=nes_getobj(N, tobj, namebuf);
			while (cobj->val->type==NT_TABLE) {
				tobj=cobj;
				if (*N->readptr!=OP_POBRACKET&&*N->readptr!=OP_PDOT) break;
				pobj=tobj;
				cobj=n_readindex(N, tobj, namebuf);
				if (nes_isnull(cobj)&&(namebuf[0]!=0)) {
					cobj=nes_setnum(N, tobj, namebuf, 0);
				}
			}
			ctype=nes_typeof(cobj);
			if ((ctype==NT_NFUNC)||(ctype==NT_CFUNC)) {
				n_execfunction(N, cobj, pobj);
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				if (single) break;
				continue;
			}
			if (ctype==NT_NULL) {
				if (namebuf[0]==0) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a label");
				cobj=nes_setnum(N, tobj, namebuf, 0);
			}
			n_readvar(N, tobj, cobj);
		}
endstmt:
		if (*N->readptr==OP_PSEMICOL) N->readptr++;
		if (single) break;
	}
end:
	if (jmp==0) {
		n_free(N, (void *)&N->savjmp);
		if (p) n_free(N, (void *)&p);
		N->blockend=NULL;
		N->readptr=NULL;
	}
	DEBUG_OUT();
	return NULL;
#undef __FUNCTION__
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <string.h>
int nes_writefile(nes_state *N, char *file, uchar *dat)
{
#define __FUNCTION__ "nes_writefile"
	char outfile[512];
	int fd;

	nc_snprintf(N, outfile, sizeof(outfile), "/tmp/%s", file);
	n_warn(N, __FUNCTION__, "writing '%s' from [%s]", outfile, file);
	if ((fd=open(outfile, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE))!=-1) {
		write(fd, dat, readi4((dat+8)));
		close(fd);
		return 1;
	}
	return 0;
#undef __FUNCTION__
}

int nes_execfile(nes_state *N, char *file)
{
#define __FUNCTION__ "nes_execfile"
	obj_t *cobj=nes_getobj(N, &N->g, "_filepath");
	char buf[512];
	char *pfile;
	struct stat sb;
	uchar *p;
	uchar *oldbptr=N->blockptr;
	uchar *oldbend=N->blockend;
	uchar *oldrptr=N->readptr;
	short int jmp=N->savjmp?1:0;
	int bl;
	int fd;
	int r;
	int rc;
	char *o;

	if (jmp==0) {
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)==0) {
		} else {
			rc=0;
			goto end1;
		}
	}
	pfile=file;
	if ((o=strrchr(pfile, '/'))==NULL) o=pfile; else o++;
	if ((stat(pfile, &sb)!=0)&&(cobj->val->type==NT_STRING)) {
		nc_snprintf(N, buf, sizeof(buf), "%s/%s", cobj->val->d.str, pfile);
		if (stat(buf, &sb)!=0) { rc=-1; goto end2; }
		pfile=buf;
	}
	if ((fd=open(pfile, O_RDONLY|O_BINARY))==-1) { rc=-1; goto end2; }
	N->blockptr=n_alloc(N, sb.st_size+2, 0);
	p=N->blockptr;
	bl=sb.st_size;
	for (;;) {
		r=read(fd, p, bl);
		p+=r;
		bl-=r;
		if (bl<1) break;
	}
	close(fd);
	rc=sb.st_size;
	N->blockptr[sb.st_size]='\0';
	p=n_decompose(N, N->blockptr);
	if (p!=N->blockptr) {
//		nes_writefile(N, o, p);
		n_free(N, (void *)&N->blockptr);
		N->blockptr=p;
	}
	N->blockend=N->blockptr+readi4((N->blockptr+8));
	N->readptr=N->blockptr+readi4((N->blockptr+12));
	nes_exec(N, (char *)N->readptr);
	if (N->outbuflen) nl_flush(N);
	rc=0;
end1:
	n_free(N, (void *)&N->blockptr);
	N->blockptr=oldbptr;
	N->blockend=oldbend;
	N->readptr=oldrptr;
end2:
	if (jmp==0) {
		n_free(N, (void *)&N->savjmp);
	}
	return rc;
#undef __FUNCTION__
}

typedef struct {
	char *fn_name;
	NES_CFUNC fn_ptr;
} FUNCTION;

nes_state *nes_newstate()
{
	FUNCTION list[]={
		{ "iname",	(NES_CFUNC)nl_iname	},
		{ "ival",	(NES_CFUNC)nl_ival	},
		{ "include",	(NES_CFUNC)nl_include	},
		{ "print",	(NES_CFUNC)nl_print	},
		{ "printvar",	(NES_CFUNC)nl_printvar	},
		{ "runtime",	(NES_CFUNC)nl_runtime	},
		{ "sizeof",	(NES_CFUNC)nl_sizeof	},
		{ "sleep",	(NES_CFUNC)nl_sleep	},
		{ "system",	(NES_CFUNC)nl_system	},
		{ "tonumber",	(NES_CFUNC)nl_tonumber	},
		{ "tostring",	(NES_CFUNC)nl_tostring	},
		{ "typeof",	(NES_CFUNC)nl_typeof	},
		{ "write",	(NES_CFUNC)nl_write	},
		{ NULL, NULL }
	};
	FUNCTION list_file[]={
		{ "read",	(NES_CFUNC)nl_fileread	},
		{ "stat",	(NES_CFUNC)nl_filestat  },
		{ "unlink",	(NES_CFUNC)nl_fileunlink},
		{ "write",	(NES_CFUNC)nl_filewrite	},
		{ NULL, NULL }
	};
	FUNCTION list_io[]={
		{ "print",	(NES_CFUNC)nl_print	},
		{ "write",	(NES_CFUNC)nl_write	},
		{ "flush",	(NES_CFUNC)nl_flush	},
		{ NULL, NULL }
	};
	FUNCTION list_math[]={
		{ "abs",	(NES_CFUNC)nl_math1	},
		{ "ceil",	(NES_CFUNC)nl_math1	},
		{ "floor",	(NES_CFUNC)nl_math1	},
		{ "rand",	(NES_CFUNC)nl_math1	},
		{ NULL, NULL }
	};
	FUNCTION list_string[]={
		{ "cat",	(NES_CFUNC)nl_strcat	},
		{ "cmp",	(NES_CFUNC)nl_strcmp	},
		{ "icmp",	(NES_CFUNC)nl_strcmp	},
		{ "ncmp",	(NES_CFUNC)nl_strcmp	},
		{ "nicmp",	(NES_CFUNC)nl_strcmp	},
		{ "join",	(NES_CFUNC)nl_strjoin	},
		{ "len",	(NES_CFUNC)nl_strlen	},
		{ "split",	(NES_CFUNC)nl_strsplit	},
		{ "str",	(NES_CFUNC)nl_strstr	},
		{ "istr",	(NES_CFUNC)nl_strstr	},
		{ "sub",	(NES_CFUNC)nl_strsub	},
		{ "tolower",	(NES_CFUNC)nl_strtolower},
		{ "toupper",	(NES_CFUNC)nl_strtolower},
		{ NULL, NULL }
	};
	FUNCTION list_time[]={
		{ "gmtime",	(NES_CFUNC)nl_gmtime	},
		{ "localtime",	(NES_CFUNC)nl_gmtime	},
		{ "sqldate",	(NES_CFUNC)nl_sqltime	},
		{ "sqltime",	(NES_CFUNC)nl_sqltime	},
		{ "now",	(NES_CFUNC)nl_time	},
		{ NULL, NULL }
	};
	nes_state *new_N;
	obj_t *cobj;
	short i;

	new_N=n_alloc(NULL, sizeof(nes_state), 1);
	nc_gettimeofday(&new_N->ttime, NULL);
	srand(new_N->ttime.tv_usec);

	nes_setvaltype(new_N, &new_N->g, NT_TABLE);
//	new_N->g.val=n_newval(N, NT_TABLE);
//	new_N->g.val=n_alloc(new_N, sizeof(val_t), 1);
//	new_N->g.val->type=NT_TABLE;
//	new_N->g.val->d.table=NULL;
//	new_N->g.val->refs=1;
	new_N->g.val->attr|=NST_AUTOSORT;

//	new_N->l.val=n_alloc(new_N, sizeof(val_t), 1);
//	new_N->l.val->type=NT_TABLE;
//	new_N->l.val->d.table=NULL;
//	new_N->l.val->refs=1;
	nes_setvaltype(new_N, &new_N->l, NT_TABLE);
	new_N->l.val->attr|=NST_AUTOSORT;

	nc_strncpy(new_N->g.name, "!GLOBALS!", MAX_OBJNAMELEN);
	nc_strncpy(new_N->l.name, "!LOCALS!", MAX_OBJNAMELEN);
	nc_strncpy(new_N->r.name, "!RETVAL!", MAX_OBJNAMELEN);
	cobj=nes_settable(new_N, &new_N->g, "_GLOBALS");
	cobj->val->attr|=NST_HIDDEN;
	nes_linkval(new_N, cobj, &new_N->g);

	for (i=0;list[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, &new_N->g, list[i].fn_name, list[i].fn_ptr);
	}
	cobj=nes_settable(new_N, &new_N->g, "file");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_file[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, cobj, list_file[i].fn_name, list_file[i].fn_ptr);
	}
	cobj=nes_settable(new_N, &new_N->g, "io");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_io[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, cobj, list_io[i].fn_name, list_io[i].fn_ptr);
	}
	cobj=nes_settable(new_N, &new_N->g, "math");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_math[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, cobj, list_math[i].fn_name, list_math[i].fn_ptr);
	}
	cobj=nes_settable(new_N, &new_N->g, "string");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_string[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, cobj, list_string[i].fn_name, list_string[i].fn_ptr);
	}
	cobj=nes_settable(new_N, &new_N->g, "time");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_time[i].fn_name!=NULL;i++) {
		nes_setcfunc(new_N, cobj, list_time[i].fn_name, list_time[i].fn_ptr);
	}
	cobj=nes_setnum(new_N, &new_N->g, "null", 0);
	cobj->val->type=NT_NULL; cobj->val->attr|=NST_SYSTEM;
	cobj=nes_setnum(new_N, &new_N->g, "false", 0);
	cobj->val->type=NT_BOOLEAN; cobj->val->attr|=NST_SYSTEM;
	cobj=nes_setnum(new_N, &new_N->g, "true", 1);
	cobj->val->type=NT_BOOLEAN; cobj->val->attr|=NST_SYSTEM;
	cobj=nes_setstr(new_N, &new_N->g, "_version_", NESLA_VERSION, -1);
	cobj=nes_setstr(new_N, &new_N->g, "_ostype_", _OSTYPE_, -1);
	return new_N;
}

nes_state *nes_endstate(nes_state *N)
{
	if (N!=NULL) {
		if (N->outbuflen) nl_flush(N);
		nes_freetable(N, &N->g);
		nes_freetable(N, &N->l);
		n_freeval(N, &N->r);
		if (N->g.val) n_free(N, (void *)&N->g.val);
		if (N->l.val) n_free(N, (void *)&N->l.val);
		if (N->r.val) n_free(N, (void *)&N->r.val);
		n_free(N, (void *)&N);
	}
	return NULL;
}
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


#include <math.h>
#include <stdarg.h>
#if defined(_MSC_VER)||defined(__TURBOC__)
#include <time.h>
#include <sys/timeb.h>
#endif

/* *printf() */
static const char nchars[]="0123456789ABCDEF";

int nc_vsnprintf(nes_state *N, char *dest, int max, const char *format, va_list ap)
{
	char tmp[80];
	char *d, *s, *p;
	char esc=0;
	int len=0;

	for (s=(char *)format,d=dest;*s!='\0';s++) {
		if (!esc) {
			if (*s=='%') esc=1; else { *d++=*s; len++; }
			continue;
		}
		/* flags */
		if (*s=='-') s++;
		/* width */
		if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		/* precision */
		if (*s=='.') {
			s++;
			if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		}
		switch (*s) {
		case 'c': tmp[0]=(char)va_arg(ap, int); tmp[1]=0; goto end;
		case 'd': p=n_ntoa(N, tmp, va_arg(ap, int),          -10, 0); break;
		case 'i': p=n_ntoa(N, tmp, va_arg(ap, int),          -10, 0); break;
		case 'o': p=n_ntoa(N, tmp, va_arg(ap, int),            8, 0); break;
		case 'u': p=n_ntoa(N, tmp, va_arg(ap, unsigned int),  10, 0); break;
		case 'x': p=n_ntoa(N, tmp, va_arg(ap, unsigned int),  16, 0); break;
		case 'f': p=n_ntoa(N, tmp, va_arg(ap, double),       -10, 6); break;
		case 's': p=va_arg(ap, char *); if (p==NULL) p="(null)"; break;
		default : p="orphan %"; break;
		}
		nc_strncpy(d, p, max-len); len+=nc_strlen(p); d=dest+len;
end:
		esc=0;
	}
	*d='\0';
	return len;
}

int nc_snprintf(nes_state *N, char *str, int size, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len=nc_vsnprintf(N, str, size, format, ap);
	va_end(ap);
	return len;
}

int nc_printf(nes_state *N, const char *format, ...)
{
	va_list ap;
	int len;

	if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
	va_start(ap, format);
	len=nc_vsnprintf(N, N->outbuf+N->outbuflen, MAX_OUTBUFLEN-N->outbuflen, format, ap);
	N->outbuflen+=len;
	va_end(ap);
	return len;
}

/* time stuff */
int nc_gettimeofday(struct timeval *tv, void *tz)
{
#ifdef WIN32
	struct timeb tb;

	if (tv==NULL) return -1;
	ftime(&tb);
	tv->tv_sec=tb.time;
	tv->tv_usec=tb.millitm*1000;
#else
#ifdef __TURBOC__
	struct timeb tb;

	if (tv==NULL) return -1;
	ftime(&tb);
	tv->tv_sec=tb.time;
	tv->tv_usec=tb.millitm*1000;
#else
	gettimeofday(tv, tz);
#endif
#endif
	return 0;
}

char *nc_memcpy(char *dst, const char *src, int n)
{
	uchar *s=(uchar *)src, *d=(uchar *)dst;

	if (src==NULL) return dst;
	while (n-->0) *d++=*s++;
	return dst;
}

int nc_strlen(const char *s)
{
	char *p=(char *)s;

	while (*p) p++;
	return p-s;
}

char *nc_strchr(const char *s, int c)
{
	uchar *a=(uchar *)s;

	do {
		if (*a==c) return (char *)a;
	} while (*a++);
	return NULL;
}

char *nc_strncpy(char *dst, const char *src, int n)
{
	uchar *s=(uchar *)src, *d=(uchar *)dst;

	if (n>0) {
		if (src!=NULL) {
			while (*s!=0&&--n>0) *d++=*s++;
		}
		*d=0;
	}
	return dst;
}

int nc_strcmp(const char *s1, const char *s2)
{
	uchar *a, *b;

	if (s1==s2) return 0;
	else if (!s1) return -(uchar)*s2;
	else if (!s2) return +(uchar)*s1;
	a=(uchar *)s1;
	b=(uchar *)s2;
	while ((*a==*b)&&(*a!='\0')) {
		a++; b++;
	}
	return *a-*b;
}

int nc_strncmp(const char *s1, const char *s2, int n)
{
	uchar *a=(uchar *)s1, *b=(uchar *)s2;

	if (a==b) return 0;
	do {
		if (*a!=*b) return *a-*b;
		if (*a==0) return 0;
		a++; b++;
	} while (--n>0);
	return 0;
}

void *nc_memset(void *s, int c, int n)
{
	uchar *a=s;

	while (n) a[--n]=(uchar)c;
	return s;
}

void n_error(nes_state *N, short int err, const char *fname, const char *format, ...)
{
	va_list ap;
	int len;

	N->err=err;
	if (N->err) {
		len=nc_snprintf(N, N->errbuf, sizeof(N->errbuf)-1, "%-15s : ", fname);
		va_start(ap, format);
		len+=nc_vsnprintf(N, N->errbuf+len, sizeof(N->errbuf)-len-1, format, ap);
		va_end(ap);
	}
	nl_flush(N);
	if (N->savjmp!=NULL) {
		longjmp(*N->savjmp, 1);
	} else {
		n_warn(N, "n_error", "jmp ptr not set - errno=%d :: \r\n%s", N->err, N->errbuf);
	}
	return;
}

void n_expect(nes_state *N, const char *fname, uchar op)
{
	if (*N->readptr!=op) n_error(N, NE_SYNTAX, fname, "expected a '%s'", n_getsym(N, op));
/*
	if (*N->readptr==op) return;
	switch (op) {
	case OP_STRDATA : n_error(N, NE_SYNTAX, fname, "expected a string");
	case OP_NUMDATA : n_error(N, NE_SYNTAX, fname, "expected a number");
	case OP_LABEL   : n_error(N, NE_SYNTAX, fname, "expected a label");
	default         : n_error(N, NE_SYNTAX, fname, "expected a '%s'", n_getsym(N, op));
	}
*/
}

void n_warn(nes_state *N, const char *fname, const char *format, ...)
{
	va_list ap;

	if (++N->warnings>10000) n_error(N, NE_SYNTAX, "n_warn", "too many warnings (%d)\n", N->warnings);
	if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
	nc_printf(N, "[01;33;40m%s\r\t\t : ", fname);
	va_start(ap, format);
	N->outbuflen+=nc_vsnprintf(N, N->outbuf+N->outbuflen, MAX_OUTBUFLEN-N->outbuflen, format, ap);
	va_end(ap);
	nc_printf(N, "\r\n[00m");
	nl_flush(N);
	if ((N->strict)&&(N->savjmp!=NULL)) {
		longjmp(*N->savjmp, 1);
	}
	return;
}


/*
 * the following functions are public API functions
 */
num_t n_aton(nes_state *N, const char *str)
{
	char *s=(char *)str;
	num_t rval=0;

	while (nc_isdigit(*s)) {
		rval=10*rval+(*s++-'0');
	}
	if (*s=='.') {
		num_t dot=1;

		s++;
		while (nc_isdigit(*s)) {
			dot*=0.1;
			rval+=(*s++-'0')*dot;
		}
	}
	return rval;
}

char *n_ntoa(nes_state *N, char *str, num_t num, short base, unsigned short dec)
{
#if defined(_MSC_VER) || defined(__BORLANDC__)
	/* long long int causes fires - need working substitute */
	long int n=(long int)num;
	num_t f=(num_t)num-(long int)num;
#else
	long long int n=(long long int)num;
	num_t f=(num_t)num-(long long int)num;
#endif
	unsigned int i;
	char c, sign='+';
	char *p, *q;

/*
	if (isinf(f)) {
		nc_strncpy(str, "inf", 4);
		return str;
	} else if (isnan(f)) {
		nc_strncpy(str, "nan", 4);
		return str;
	}
*/
	if (base<0) {
		if (num<0) {
			sign='-';
			n*=-1;
			f*=-1;
		}
		base=-base;
	}
	p=q=str;
	/* need a _real_ fix */
	if (n%base<0) {
		nc_strncpy(str, "nan", 4);
		return str;
	}
	do {
		*p++=nchars[n%base];
	} while ((n/=base)>0);
	if (sign=='-') *p++='-';
	*p='\0';
	/* and flip */
	while (q<--p) {
		c=*q;
		*q++=*p;
		*p=c;
	}
	if (dec) {
		while (*p) p++;
		*p++='.';
		for (i=0;i<dec;i++) {
			f*=base;
			*p++=nchars[(int)f%base];
		}
		*p='\0';
		/* insert rounding stuff around here.. */

		/* trim the zeros */
		i=nc_strlen(str)-1;
		while (str[i]=='0') str[i--]='\0';
		if (str[i]=='.') str[i--]='\0';
	}
	return str;
}
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

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
#elif !defined( __TURBOC__)
#include <unistd.h>
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* works ok, but written to handle direct output */
static int n_escape(nes_state *N, const char *str, int len)
{
	char *dst=N->outbuf;
	int i, n=N->outbuflen;

	for (i=0;i<len;i++) {
		if (MAX_OUTBUFLEN-n<32) { nl_flush(N); continue; }
		switch (str[i]) {
		case '\a' : dst[n++]='\\'; dst[n++]='a'; break;
		case '\t' : dst[n++]='\\'; dst[n++]='t'; break;
		case '\f' : dst[n++]='\\'; dst[n++]='f'; break;
		case 27   : dst[n++]='\\'; dst[n++]='e'; break;
		case '\r' : dst[n++]='\\'; dst[n++]='r'; break;
		case '\n' : dst[n++]='\\'; dst[n++]='n'; break;
		case '\'' : dst[n++]='\\'; dst[n++]='\''; break;
		case '\"' : dst[n++]='\\'; dst[n++]='\"'; break;
		case '\\' : dst[n++]='\\'; dst[n++]='\\'; break;
		default: dst[n++]=str[i];
		}
	}
	dst[n]='\0';
	N->outbuflen=n;
	return len;
}

static void n_dumpvars(nes_state *N, obj_t *tobj, int depth)
{
	obj_t *cobj=tobj;
	int i;
	char b;
	char *g;
	int ent=0;

	for (;cobj;cobj=cobj->next) {
		if (nes_isnull(cobj)||cobj->val->attr&NST_HIDDEN||cobj->val->attr&NST_SYSTEM) continue;
		g=(depth<1)?"global ":"";
		if (nc_isdigit(cobj->name[0])) b=1; else b=0;
		if (cobj->val->type==NT_BOOLEAN||cobj->val->type==NT_NUMBER) {
			if (ent++) nc_printf(N, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) nc_printf(N, "\t");
				nc_printf(N, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			nc_printf(N, "%s", nes_tostr(N, cobj));
		} else if (cobj->val->type==NT_STRING) {
			if (ent++) nc_printf(N, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) nc_printf(N, "\t");
				nc_printf(N, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			nc_printf(N, "\"");
			n_escape(N, cobj->val->d.str?cobj->val->d.str:"", cobj->val->size);
			nc_printf(N, "\"");
		} else if (cobj->val->type==NT_TABLE) {
			if (nc_strcmp(cobj->name, "_GLOBALS")==0) continue;
			if (ent++) nc_printf(N, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) nc_printf(N, "\t");
				nc_printf(N, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			nc_printf(N, "{");
			if (cobj->val->d.table) {
				nc_printf(N, "\n");
				n_dumpvars(N, cobj->val->d.table, depth+1);
				for (i=0;i<depth;i++) nc_printf(N, "\t");
				nc_printf(N, "}");
			} else {
				nc_printf(N, " }");
			}
		}
	}
	if (ent) nc_printf(N, "%s", depth?"\n":"");
	nl_flush(N);
	return;
}

/*
 * basic i/o functions
 */
/* windows is retarded..  from unistd.. */
NES_FUNCTION(nl_flush)
{
	obj_t *cobj=nes_getobj(N, &N->g, "io");
	NES_CFUNC cfunc=(NES_CFUNC)nl_flush;
	int rc;

	if (cobj->val->type!=NT_TABLE) {
		nes_setnum(N, &N->r, "", -1);
		return -1;
	}
	cobj=nes_getobj(N, cobj, "flush");
	if (cobj->val->type==NT_CFUNC) {
		cfunc=cobj->val->d.cfunc;
		if (cfunc!=(NES_CFUNC)nl_flush) {
			rc=cfunc(N);
			nes_setnum(N, &N->r, "", rc);
			return rc;
		}
	}
	N->outbuf[N->outbuflen]='\0';
	write(STDOUT_FILENO, N->outbuf, N->outbuflen);
	N->outbuflen=0;
	/* do NOT touch &N->r */
	return 0;
}

static int writestr(nes_state *N, obj_t *cobj)
{
	unsigned int i;
	unsigned int len=0;
	char *p;

	if (nes_isstr(cobj)) {
		p=cobj->val->d.str;
		len=cobj->val->size;
	} else {
		p=nes_tostr(N, cobj);
		len=nc_strlen(p);
	}
	for (i=0;i<len;i++) {
		if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
		N->outbuf[N->outbuflen++]=p[i];
	}
	N->outbuf[N->outbuflen]='\0';
	return len;
}

NES_FUNCTION(nl_print)
{
	obj_t *cobj=N->l.val->d.table;
	int tlen=0;

	for (cobj=cobj->next;cobj;cobj=cobj->next) {
		if (cobj->name[0]!='n') tlen+=writestr(N, cobj);
	}
	if (N->debug) nl_flush(N);
	nes_setnum(N, &N->r, "", tlen);
	return 0;
}

NES_FUNCTION(nl_write)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int len;

	len=writestr(N, cobj1);
	nes_setnum(N, &N->r, "", len);
	return 0;
}

/*
 * file functions
 */
NES_FUNCTION(nl_fileread)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset=0;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (stat(cobj1->val->d.str, &sb)!=0) {
		nes_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd=open(cobj1->val->d.str, O_RDONLY|O_BINARY))==-1) {
		nes_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type==NT_NUMBER) {
		offset=(int)cobj2->val->d.num;
		lseek(fd, offset, SEEK_SET);
	}
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	bl=sb.st_size-offset;
	robj->val->d.str=n_alloc(N, bl+2, 0);
	robj->val->size=bl;
	p=(char *)robj->val->d.str;
	for (;;) {
		r=read(fd, p, bl);
		p+=r;
		bl-=r;
		if (bl<1) break;
	}
	close(fd);
	robj->val->d.str[sb.st_size]='\0';
	return 0;
}

NES_FUNCTION(nl_filestat)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym=0;
	char *file;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	file=cobj1->val->d.str;
#ifdef WIN32
	rc=stat(file, &sb);
	if (rc!=0) {
		nes_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
#ifdef __TURBOC__
	rc=stat(file, &sb);
	if (rc!=0) {
		nes_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
	rc=lstat(file, &sb);
	if (rc!=0) {
		nes_setnum(N, &N->r, "", rc);
		return 0;
	}
	if (!(~sb.st_mode&S_IFLNK)) {
		sym=1;
		if (stat(file, &sb)!=0) sym=2;
	}
#endif
#endif
	tobj.val=n_newval(N, NT_TABLE);
	nes_setnum(N, &tobj, "mtime", sb.st_mtime);
	if (sym==2) {
		nes_setnum(N, &tobj, "size", 0);
		nes_setstr(N, &tobj, "type", "broken", 6);
	} else if ((sb.st_mode&S_IFDIR)) {
		nes_setnum(N, &tobj, "size", 0);
		nes_setstr(N, &tobj, "type", sym?"dirp":"dir", sym?4:3);
	} else {
		nes_setnum(N, &tobj, "size", sb.st_size);
		nes_setstr(N, &tobj, "type", sym?"filep":"file", sym?5:4);
	}
	nes_linkval(N, &N->r, &tobj);
	nes_unlinkval(N, &tobj);
	return 0;
}

NES_FUNCTION(nl_fileunlink)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int rc=-1;
	int i;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	for (i=1;;i++) {
		cobj1=nes_getiobj(N, &N->l, i);
		if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) break;
		rc=unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nes_setnum(N, &N->r, "", rc);
	return 0;
}

NES_FUNCTION(nl_filewrite)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	int fd;
	int w=0;
	int offset=0;

	/* umask(022); */
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if ((fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE))==-1) {
		nes_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type==NT_STRING) {
		if (cobj3->val->type==NT_NUMBER) {
			offset=(int)cobj3->val->d.num;
			lseek(fd, offset, SEEK_SET);
		}
		w=write(fd, cobj2->val->d.str, cobj2->val->size);
	}
	close(fd);
	nes_setnum(N, &N->r, "", w);
	return 0;
}

/*
 * math functions
 */
NES_FUNCTION(nl_math1)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	num_t n;

	if (nc_strcmp(cobj0->val->d.str, "rand")==0) {
		n=rand();
		if (cobj1->val->type==NT_NUMBER&&(int)cobj1->val->d.num) {
			n=(int)n%(int)cobj1->val->d.num;
		}
		n=(int)n%65535;
		nes_setnum(N, &N->r, "", n);
		return 0;
	}
	if (cobj1->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number");
	if (nc_strcmp(cobj0->val->d.str, "abs")==0) {
		n=fabs(cobj1->val->d.num);
	} else if (nc_strcmp(cobj0->val->d.str, "ceil")==0) {
		n=cobj1->val->d.num;
		if ((int)n<n) n=(int)n+1; else n=(int)n;
	} else if (nc_strcmp(cobj0->val->d.str, "floor")==0) {
		n=(int)cobj1->val->d.num;
	} else {
		n=0;
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
}

NES_FUNCTION(nl_tonumber)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	nes_setnum(N, &N->r, "", nes_tonum(N, cobj1));
	return 0;
}

NES_FUNCTION(nl_tostring)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	unsigned short d;
	char *p;

	if (cobj1->val->type==NT_NUMBER&&cobj2->val->type==NT_NUMBER) {
		d=(unsigned short)cobj2->val->d.num;
		if (d>sizeof(N->numbuf)-2) d=sizeof(N->numbuf)-2;
		p=n_ntoa(N, N->numbuf, cobj1->val->d.num, 10, d);
	} else {
		p=nes_tostr(N, cobj1);
	}
	nes_setstr(N, &N->r, "", p, p?nc_strlen(p):0);
	return 0;
}

/*
 * string functions
 */
NES_FUNCTION(nl_strcat)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	robj->val->size=cobj1->val->size+cobj2->val->size;
	robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
	nc_memcpy(robj->val->d.str, cobj1->val->d.str, cobj1->val->size);
	nc_memcpy(robj->val->d.str+cobj1->val->size, cobj2->val->d.str, cobj2->val->size);
	robj->val->d.str[robj->val->size]=0;
	return 0;
}

NES_FUNCTION(nl_strcmp)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	uchar *s1, *s2;
	int i, rval=0;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	if (cobj1->val->d.str==NULL) {
		if (cobj2->val->d.str==NULL) rval=0; else rval=(int)cobj2->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	} else if (cobj2->val->d.str==NULL) {
		rval=-(int)cobj1->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	}
	if (nc_strcmp(cobj0->val->d.str, "cmp")==0) {
		rval=nc_strcmp(cobj1->val->d.str, cobj2->val->d.str);
	} else if (nc_strcmp(cobj0->val->d.str, "ncmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg3");
		rval=nc_strncmp(cobj1->val->d.str, cobj2->val->d.str, (int)cobj3->val->d.num);
	} else if (nc_strcmp(cobj0->val->d.str, "icmp")==0) {
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			s1++; s2++;
		} while (*s1!=0);
	} else if (nc_strcmp(cobj0->val->d.str, "nicmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg3");
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str; i=(int)cobj3->val->d.num;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			if (--i<1) break;
			s1++; s2++;
		} while (*s1!=0);
	}
	nes_setnum(N, &N->r, "", rval);
	return 0;
}

NES_FUNCTION(nl_strjoin)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj, *robj;
	char *p=NULL, *p2;
	int len1=0, len2=0;
	int cnt=0;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
	if (cobj2->val->type==NT_STRING&&cobj2->val->size>0) { p=cobj2->val->d.str; len2=cobj2->val->size; }
	for (cobj=cobj1->val->d.table;cobj;cobj=cobj->next) {
		if (cobj->val->type==NT_STRING) {
			len1+=cobj->val->size;
			cnt++;
		} else if (cobj->val->type==NT_NUMBER) {
			len1++;
			cnt++;
		}
	}
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	if (cnt==0) return 0;
	if (cnt>1) {
		robj->val->size=len1+((cnt-1)*len2);
		robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
		p2=robj->val->d.str;
		for (cobj=cobj1->val->d.table;cobj;cobj=cobj->next) {
			if (cobj->val->type==NT_STRING) {
				nc_memcpy(p2, cobj->val->d.str, cobj->val->size);
				p2+=cobj->val->size;
				if (--cnt&&p) {
					nc_memcpy(p2, p, len2);
					p2+=len2;
				}
			} else if (cobj->val->type==NT_NUMBER) {
				p2[0]=(uchar)cobj->val->d.num&255;
				p2++;
				if (--cnt&&p) {
					nc_memcpy(p2, p, len2);
					p2+=len2;
				}
			}
		}
		robj->val->d.str[robj->val->size]=0;
	} else {
		robj->val->size=len1;
		robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
		for (cobj=cobj1->val->d.table;cobj;cobj=cobj->next) {
			if (cobj->val->type==NT_STRING) {
				nc_memcpy(robj->val->d.str, cobj->val->d.str, cobj->val->size);
				break;
			}
		}
		robj->val->d.str[robj->val->size]=0;
	}
	return 0;
}

NES_FUNCTION(nl_strlen)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	nes_setnum(N, &N->r, "", cobj1->val->size);
	return 0;
}

NES_FUNCTION(nl_strsplit)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t tobj;
	char *ss, *se;
	int i=0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN+1];

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	tobj.val=n_newval(N, NT_TABLE);
	if (cobj1->val->d.str) {
		if (cobj2->val->d.str) {
			se=ss=cobj1->val->d.str;
			s2=cobj2->val->d.str;
			l2=cobj2->val->size;
			for (;*se;se++) {
				if (nc_strncmp(se, s2, l2)!=0) continue;
				nes_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
				ss=se+=l2;
				if (!*se) {
					nes_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
					break;
				}
			}
			if (se>ss) {
				nes_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
			}
		} else {
			for (ss=cobj1->val->d.str;*ss;ss++) {
				nes_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, 1);
			}
		}
	}
	nes_linkval(N, &N->r, &tobj);
	nes_unlinkval(N, &tobj);
	return 0;
}

NES_FUNCTION(nl_strstr)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	unsigned int i=0, j=0;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	if (cobj2->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "zero length arg2");
	if (nc_strcmp(cobj0->val->d.str, "str")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (cobj2->val->d.str[j]!=cobj1->val->d.str[i]) j=-1;
		}
	} else if (nc_strcmp(cobj0->val->d.str, "istr")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (nc_tolower(cobj2->val->d.str[j])!=nc_tolower(cobj1->val->d.str[i])) j=-1;
		}
	} else {
		n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "................");
	}
	if (i<=cobj1->val->size&&j==cobj2->val->size) {
		nes_setstr(N, &N->r, "", cobj1->val->d.str+i-j, cobj1->val->size-i+j);
	}
	return 0;
}

NES_FUNCTION(nl_strsub)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	unsigned int offset, max=0;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
	if (cobj2->val->d.num<0) {
		offset=cobj1->val->size-abs((int)cobj2->val->d.num);
	} else {
		offset=(int)cobj2->val->d.num;
	}
	if (cobj3->val->type==NT_NUMBER) max=(int)cobj3->val->d.num; else max=cobj1->val->size;
	if (offset>cobj1->val->size) offset=cobj1->val->size;
	if (max>cobj1->val->size-offset) max=cobj1->val->size-offset;
	nes_setstr(N, &N->r, "", cobj1->val->d.str+offset, max);
	return 0;
}

NES_FUNCTION(nl_strtolower)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *robj;
	int i;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str!=NULL) {
		robj->val->size=cobj1->val->size;
		robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
		robj->val->d.str[0]=0;
		i=robj->val->size-1;
		if (nc_strcmp(cobj0->val->d.str, "tolower")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_tolower(cobj1->val->d.str[i]);
		} else if (nc_strcmp(cobj0->val->d.str, "toupper")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_toupper(cobj1->val->d.str[i]);
		}
		robj->val->d.str[robj->val->size]=0;
	}
	return 0;
}

NES_FUNCTION(nl_sqltime)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	struct timeval ttime;
	char timebuf[16];

	if (cobj1->val->type==NT_NUMBER) {
		ttime.tv_sec=(time_t)cobj1->val->d.num;
	} else {
		nc_gettimeofday(&ttime, NULL);
	}
	if (nc_strcmp(cobj0->val->d.str, "sqldate")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%Y-%m-%d", localtime((time_t *)&ttime.tv_sec));
	} else if (nc_strcmp(cobj0->val->d.str, "sqltime")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	}
	nes_setstr(N, &N->r, "", timebuf, -1);
	return 0;	
}

NES_FUNCTION(nl_time)
{
	nes_setnum(N, &N->r, "", time(NULL));
	return 0;	
}

NES_FUNCTION(nl_gmtime)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t tobj;
	time_t t;
	struct tm *tp;

	if (cobj1->val->type==NT_NUMBER) {
		t=(time_t)cobj1->val->d.num;
	} else {
		t=time(NULL);
	}
	tobj.val=n_newval(N, NT_TABLE);
	tobj.val->attr&=~NST_AUTOSORT;
	if (nc_strcmp(cobj0->val->d.str, "gmtime")==0) {
		tp=gmtime(&t);
	} else if (nc_strcmp(cobj0->val->d.str, "localtime")==0) {
		tp=localtime(&t);
	} else {
		nes_setnum(N, &N->r, "", 0);
		return 0;
	}
	if (tp->tm_year<1900) tp->tm_year+=1900;
	nes_setnum(N, &tobj, "tm_sec",   tp->tm_sec);
	nes_setnum(N, &tobj, "tm_min",   tp->tm_min);
	nes_setnum(N, &tobj, "tm_hour",  tp->tm_hour);
	nes_setnum(N, &tobj, "tm_mday",  tp->tm_mday);
	nes_setnum(N, &tobj, "tm_mon",   tp->tm_mon);
	nes_setnum(N, &tobj, "tm_year",  tp->tm_year);
	nes_setnum(N, &tobj, "tm_wday",  tp->tm_wday);
	nes_setnum(N, &tobj, "tm_yday",  tp->tm_yday);
	nes_setnum(N, &tobj, "tm_isdst", tp->tm_isdst);
	nes_linkval(N, &N->r, &tobj);
	nes_unlinkval(N, &tobj);
	return 0;	
}

NES_FUNCTION(nl_runtime)
{
	struct timeval ttime;
	int totaltime;

	nc_gettimeofday(&ttime, NULL);
	totaltime=((ttime.tv_sec-N->ttime.tv_sec)*1000000)+(ttime.tv_usec-N->ttime.tv_usec);
	nes_setnum(N, &N->r, "", (num_t)totaltime/1000000);
	return 0;
}

NES_FUNCTION(nl_sleep)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=1;

	if (cobj1->val->type==NT_NUMBER) n=(int)cobj1->val->d.num;
#if defined(__BORLANDC__)
	sleep(n);
#elif defined(WIN32)
	Sleep(n*1000);
#else
	sleep(n);
#endif
	nes_setnum(N, &N->r, "", 0);
	return 0;
}

NES_FUNCTION(nl_iname)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nes_isnull(cobj)) {
		nes_setnum(N, &N->r, "", 0);
	} else {
		nes_setstr(N, &N->r, "", cobj->name, -1);
	}
	return 0;
}

NES_FUNCTION(nl_ival)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nes_isnull(cobj)) {
		nes_unlinkval(N, &N->r);
	} else {
		nes_linkval(N, &N->r, cobj);
	}
	return 0;
}

NES_FUNCTION(nl_include)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	uchar *p;
	int n=0;

	if (!nes_isnull(cobj1)) {
		p=N->readptr;
		n=nes_execfile(N, (char *)cobj1->val->d.str);
		N->readptr=p;
	}
	nes_setnum(N, &N->r, "", n);
	return n;
}

NES_FUNCTION(nl_printvar)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	if (!nes_isnull(cobj1)) {
		n_dumpvars(N, cobj1, 0);
		nes_setnum(N, &N->r, "", 0);
	}
	return 0;
}

NES_FUNCTION(nl_sizeof)
{
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	int size=0;

	if (nes_isnull(cobj)) {
		nes_setnum(N, &N->r, "", 0);
		return 0;
	}
	switch (cobj->val->type) {
	case NT_BOOLEAN :
	case NT_NUMBER  : size=1; break;
	case NT_STRING  :
	case NT_NFUNC   : size=cobj->val->size; break;
	case NT_CFUNC   : size=1; break;
	case NT_TABLE   :
		for (cobj=cobj->val->d.table;cobj;cobj=cobj->next) {
			if (cobj->val->attr&NST_SYSTEM) { size--; continue; }
			if (!nes_isnull(cobj)) size++;
		}
		break;
	case NT_CDATA   : size=cobj->val->size; break;
	}
	nes_setnum(N, &N->r, "", size);
	return 0;
}

NES_FUNCTION(nl_typeof)
{
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	NES_CDATA *chead;
	char *p;

	switch (cobj->val->type) {
	case NT_BOOLEAN : p="boolean";  break;
	case NT_NUMBER  : p="number";   break;
	case NT_STRING  : p="string";   break;
	case NT_TABLE   : p="table";    break;
	case NT_NFUNC   :
	case NT_CFUNC   : p="function"; break;
	case NT_CDATA   :
		if ((chead=(NES_CDATA *)cobj->val->d.cdata)!=NULL) {
			p=chead->obj_type; break;
		}
	default         : p="null";     break;
	}
	nes_setstr(N, &N->r, "", p, -1);
	return 0;
}

NES_FUNCTION(nl_system)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=-1;

	if (cobj1->val->type==NT_STRING&&cobj1->val->d.str!=NULL) {
		nl_flush(N);
		n=system(cobj1->val->d.str);
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
}
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

#include <stdlib.h>

static val_t __null = { NT_NULL, 0, 1, 0, { 0 } };
static obj_t _null = { NULL, NULL, &__null, "" };

void *n_alloc(nes_state *N, int size, short zero)
{
	char *p;

	if ((p=zero?calloc(1, size):malloc(size))==NULL) n_error(N, NE_MEM, "n_alloc", "can't alloc %d bytes", size);
	return p;
}

void *n_realloc(nes_state *N, void **p, int size, short zero)
{
	char *p2;

	if ((p2=realloc(*p, size))==NULL) n_error(N, NE_MEM, "n_realloc", "can't realloc %d bytes", size);
	return p2;
}

void n_free(nes_state *N, void **p)
{
	if (*p) { free(*p); *p=NULL; }
//	else n_error(N, NE_MEM, "n_free", "freeing 0x%08X twice", *p); /* pedantic, but useful */
	return;
}

void n_copyval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
	unsigned short type2=nes_typeof(cobj2);

	if (cobj1==cobj2) return;
	nes_setvaltype(N, cobj1, type2);
	switch (type2) {
	case NT_NULL    : return;
	case NT_BOOLEAN :
	case NT_NUMBER  : cobj1->val->d.num=cobj2->val->d.num; break;
	case NT_CDATA   : n_warn(N, "n_copyval", "copying cdata '%s'?", cobj2->val->d.str);
	case NT_STRING  :
	case NT_NFUNC   : nes_strcat(N, cobj1, cobj2->val->d.str, cobj2->val->size); break;
	case NT_CFUNC   : cobj1->val->d.cfunc=cobj2->val->d.cfunc; break;
	default: n_error(N, NE_SYNTAX, "n_copyval", "unhandled object type");
	}
	return;
}

void n_freeval(nes_state *N, obj_t *cobj)
{
	NES_CDATA *chead;
	NES_CFREE cfunc;

	switch (nes_typeof(cobj)) {
	case NT_NULL    : return;
	case NT_BOOLEAN :
	case NT_NUMBER  : break;
	case NT_CDATA   :
		/* try to call this object's reaper function before freeing the data */
		if ((chead=(NES_CDATA *)cobj->val->d.cdata)!=NULL) {
			if ((cfunc=chead->obj_term)!=NULL) cfunc(N, cobj);
		}
	case NT_STRING  :
	case NT_NFUNC   :
		if (cobj->val->attr&NST_LINK) {
			cobj->val->attr^=NST_LINK;
			cobj->val->d.str=NULL;
		} else if (cobj->val->d.str!=NULL) {
			n_free(N, (void *)&cobj->val->d.str);
		}
		break;
	case NT_TABLE   : nes_freetable(N, cobj); break;
	}
	cobj->val->type=NT_NULL;
	cobj->val->size=0;
	return;
}

val_t *n_newval(nes_state *N, unsigned short type)
{
	val_t *val=n_alloc(N, sizeof(val_t), 0);

	val->type=type;
	val->attr=0;
	val->refs=1;
	val->size=0;
	val->d.num=0;
	return val;
}

obj_t *n_newiobj(nes_state *N, int index)
{
	obj_t *obj=n_alloc(N, sizeof(obj_t), 0);

	obj->prev=NULL;
	obj->next=NULL;
	obj->val=NULL;
	n_ntoa(N, obj->name, index, 10, 0);
	return obj;
}

/*
 * the following functions are public API functions
 */

void nes_setvaltype(nes_state *N, obj_t *cobj, unsigned short type)
{
	if (cobj->val==NULL) {
		cobj->val=n_alloc(N, sizeof(val_t), 0);
	} else if (cobj->val->refs<2) {
		n_freeval(N, cobj);
	} else {
		nes_unlinkval(N, cobj);
		cobj->val=n_alloc(N, sizeof(val_t), 0);
	}
	cobj->val->type=type;
	cobj->val->attr=0;
	cobj->val->refs=1;
	cobj->val->size=0;
	cobj->val->d.num=0;
	return;
}

void nes_linkval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
	if (cobj1==NULL||cobj1==cobj2) return;
	if (cobj2==NULL||cobj2->val==NULL) {
		nes_unlinkval(N, cobj1);
		cobj1->val=n_alloc(N, sizeof(val_t), 1);
	} else {
		if (cobj1->val==cobj2->val) return;
		nes_unlinkval(N, cobj1);
		if (cobj2==&N->r) {
			cobj1->val=cobj2->val;
			cobj2->val=NULL;
			return;
		}
		cobj1->val=cobj2->val;
	}
	if (cobj1->val) cobj1->val->refs++;
	return;
}

void nes_unlinkval(nes_state *N, obj_t *cobj)
{
	if (cobj->val==NULL) return;
	switch (cobj->val->refs) {
	case 0 : return;
	case 1 : cobj->val->refs--;n_freeval(N, cobj);n_free(N, (void *)&cobj->val);break;
	default: cobj->val->refs--;cobj->val=NULL;break;
	}
	return;
}

void nes_freetable(nes_state *N, obj_t *tobj)
{
	obj_t *cobj, *oobj;

	if (!nes_istable(tobj)) return;
	cobj=tobj->val->d.table;
	while (cobj!=NULL) {
		oobj=cobj;
		cobj=cobj->next;
		nes_unlinkval(N, oobj);
		n_free(N, (void *)&oobj);
	}
	tobj->val->d.table=NULL;
	return;
}

obj_t *nes_getobj(nes_state *N, obj_t *tobj, char *oname)
{
	obj_t *cobj;

	if (tobj==&N->r) return tobj;
	if (!nes_istable(tobj)) return &_null;
	if (N==NULL) {
		for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	} else if (tobj==&N->l||tobj==&N->g) {
		for (cobj=N->l.val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
		for (cobj=N->g.val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	} else {
		for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	}
	return &_null;
}

obj_t *nes_getiobj(nes_state *N, obj_t *tobj, int oindex)
{
	obj_t *cobj;
	int i=0;

	if (!nes_istable(tobj)) return &_null;
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (nes_isnull(cobj)||cobj->val->attr&NST_SYSTEM) continue;
		if (i==oindex) return cobj;
		i++;
	}
	return &_null;
}

/* change or create an object and return it */
obj_t *nes_setobj(nes_state *N, obj_t *tobj, char *oname, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, int _slen)
{
	obj_t *oobj, *cobj;
	char *ostr=NULL;
	char *p;
	int cmp=-1;
	unsigned short sortattr;

	if ((tobj==&N->r)||(!nes_istable(tobj))) {
		cobj=tobj;
//		nes_setvaltype(N, cobj, otype);
		switch (nes_typeof(cobj)) {
		case NT_NULL   :
			if (cobj->val==NULL) cobj->val=n_newval(N, otype);
			break;
		case NT_STRING :
		case NT_NFUNC  :
			ostr=cobj->val->d.str;
			cobj->val->d.str=NULL;
			cobj->val->size=0;
		default:
			cobj->val->type=otype;
		}
	} else {
		if (oname[0]=='\0') { return &_null; }
		sortattr=tobj->val->attr&NST_AUTOSORT;
		if (tobj->val->d.table==NULL) {
			cobj=tobj->val->d.table=n_alloc(N, sizeof(obj_t), 0);
			cobj->prev=NULL;
			cobj->next=NULL;
			cobj->val=n_newval(N, otype);
			nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
		} else {
			oobj=tobj->val->d.table;
			for (cobj=oobj; cobj; oobj=cobj,cobj=cobj->next) {
				/* both strings must be entirely numeric (not even decimals) */
				p=cobj->name; while (nc_isdigit(*p)) p++;
				if (!*p) { p=oname; while (nc_isdigit(*p)) p++; }
				if (!*p) {
					cmp=(int)(n_aton(N, cobj->name)-n_aton(N, oname));
				} else {
					if (cobj->name[0]!=oname[0]) cmp=cobj->name[0]-oname[0];else
					cmp=nc_strcmp(cobj->name, oname);
				}
				if (cmp==0||(cmp>0&&sortattr)) break;
			}
			if (cmp>0&&sortattr) {
				oobj=cobj;
				cobj=n_alloc(N, sizeof(obj_t), 0);
				if (oobj==tobj->val->d.table) tobj->val->d.table=cobj;
				cobj->prev=oobj->prev;
				if (cobj->prev) cobj->prev->next=cobj;
				cobj->next=oobj;
				oobj->prev=cobj;
				cobj->val=n_newval(N, otype);
				nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
			} else if (cobj==NULL) {
				cobj=n_alloc(N, sizeof(obj_t), 0);
				if (oobj!=NULL) {
					oobj->next=cobj;
					cobj->prev=oobj;
				} else {
					cobj->prev=NULL;
				}
				cobj->next=NULL;
				cobj->val=n_newval(N, otype);
				nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
			} else {
				switch (nes_typeof(cobj)) {
				case NT_NULL   :
					if (cobj->val==NULL) cobj->val=n_newval(N, otype);
					break;
				case NT_STRING :
				case NT_NFUNC  :
					ostr=cobj->val->d.str;
					cobj->val->d.str=NULL;
					cobj->val->size=0;
				default:
					cobj->val->type=otype;
				}
			}
		}
		if (cobj->val->type==NT_TABLE&&cobj->val->d.table==NULL) cobj->val->attr|=sortattr;
	}
	switch (otype) {
	case NT_NUMBER : cobj->val->d.num=_num; break;
	case NT_CFUNC  : cobj->val->d.cfunc=_fptr; break;
	case NT_STRING :
	case NT_NFUNC  :
	case NT_CDATA  : nes_strcat(N, cobj, _str, _slen); break;
	}
	if (ostr) n_free(N, (void *)&ostr);
	return cobj;
}

obj_t *nes_setiobj(nes_state *N, obj_t *tobj, int index, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, int _slen)
{
	char numbuf[12];

	return nes_setobj(N, tobj, n_ntoa(N, numbuf, index, 10, 0), otype, _fptr, _num, _str, _slen);
}

void nes_strcat(nes_state *N, obj_t *cobj, char *str, int len)
{
	unsigned short ctype=nes_typeof(cobj);
	int olen, tlen;
	char *p;

	if (ctype!=NT_STRING&&ctype!=NT_NFUNC&&ctype!=NT_CDATA) return;
	if (len==-1&&str!=NULL) len=nc_strlen(str);
	if (len<1) return;
	olen=cobj->val->size;
	tlen=olen+len;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, "nes_strcat", "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	nc_memcpy(cobj->val->d.str+olen, str, len);
	cobj->val->d.str[tlen]=0;
	return;
}

void nes_strmul(nes_state *N, obj_t *cobj, int n)
{
	unsigned short ctype=nes_typeof(cobj);
	int i, olen, tlen;
	char *p;

	if (ctype!=NT_STRING) return;
	if (n<2) return;
	olen=cobj->val->size;
	tlen=olen*n;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, "nes_strmul", "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	for (i=1;i<n;i++) nc_memcpy(p+olen*i, cobj->val->d.str, olen);
	cobj->val->d.str[tlen]=0;
	return;
}

short nes_tobool(nes_state *N, obj_t *cobj)
{
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return 0;
	case NT_BOOLEAN : return cobj->val->d.num?1:0;
	case NT_NUMBER  : return cobj->val->d.num?1:0;
	case NT_STRING  : return cobj->val->size?1:0;
	case NT_TABLE   : return cobj->val->d.table?1:0;
	case NT_NFUNC   :
	case NT_CFUNC   : return 1;
	}
	return 0;
}

num_t nes_tonum(nes_state *N, obj_t *cobj)
{
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return 0;
	case NT_BOOLEAN : return cobj->val->d.num?1:0;
	case NT_NUMBER  : return cobj->val->d.num;
	case NT_STRING  : return n_aton(N,cobj->val->d.str);
	case NT_TABLE   : return 0;
	case NT_NFUNC   :
	case NT_CFUNC   : return 0;
	}
	return 0;
}

char *nes_tostr(nes_state *N, obj_t *cobj)
{
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return "null";
	case NT_BOOLEAN : return cobj->val->d.num?"true":"false";
	case NT_NUMBER  : return n_ntoa(N,N->numbuf,cobj->val->d.num,-10,6);
	case NT_STRING  : return cobj->val->d.str?cobj->val->d.str:"";
	case NT_TABLE   : return "";
	case NT_NFUNC   :
	case NT_CFUNC   : return "";
	}
	return "";
}
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

	if (N->readptr==NULL) n_error(N, NE_SYNTAX, __FUNCTION__, "NULL readptr");
	name[0]=0;
	if (*N->readptr>127) return *N->readptr++;
	if (IS_DATA(*N->readptr)) {
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
		for (i=254-OP_KBREAK;i<=254-OP_KEXIT;i++) {
			if (nc_strcmp(oplist[i].name, name)==0) return oplist[i].value;
		}
		return OP_LABEL;
	} else if (IS_PUNCOP(*N->readptr)) {
		name[0]=*N->readptr++;
		name[1]=0;
		for (i=254-OP_POBRACE;i<=254-OP_PHASH;i++) {
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
		for (i=254-OP_MEQ;i<=254-OP_MCGT;i++) {
			if (nc_strcmp(oplist[i].name, name)==0) return oplist[i].value;
		}
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
	default         : if (op<OP_UNDEFINED&&op>OP_LABEL) return oplist[254-op].name;
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


#include <math.h>

/* read a function from N->readptr */
obj_t *n_getfunction(nes_state *N)
{
#define __FUNCTION__ "n_getfunction"
	uchar *as, *be;

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
	char *p=(char *)N->readptr+2;

	N->readptr+=3+N->readptr[1];
	if (buf) p=nc_strncpy(buf, p, MAX_OBJNAMELEN);
	return p;
}

/* read a number val from N->readptr */
num_t n_getnumber(nes_state *N)
{
	char *p=(char *)N->readptr+2;
	num_t rval=0, rdot=0.1;

	N->readptr+=3+N->readptr[1];
	while (nc_isdigit(*p)) {
		rval=10*rval+(*p++-'0');
	}
	if (*p!='.') return rval;
	p++;
	while (nc_isdigit(*p)) {
		rval+=(*p++-'0')*rdot;
		rdot*=0.1;
	}
	/* if (N->debug) n_warn(N, __FUNCTION__, "[%f]", rval); */
	return rval;
}

/* read a string val from N->readptr */
obj_t *n_getstring(nes_state *N)
{
	int size=readi4((N->readptr+1));

	nes_strcat(N, nes_setstr(N, &N->r, "", NULL, 0), (char *)N->readptr+5, size);
	N->readptr+=6+size;
	/* if (N->debug) n_warn(N, __FUNCTION__, "%d '%s'", size, nes_tostr(N, cobj)); */
	return &N->r;
}

/* return the next table index */
obj_t *n_readindex(nes_state *N, obj_t *tobj, char *lastname)
{
#define __FUNCTION__ "n_readindex"
	obj_t *cobj;
	char *p;

	DEBUG_IN();
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
#define __FUNCTION__ "n_readtable"
	char namebuf[MAX_OBJNAMELEN+1];
	unsigned int i;
	obj_t *cobj;

	DEBUG_IN();
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
#define __FUNCTION__ "n_readvar"
	char namebuf[MAX_OBJNAMELEN+1];
	char *nameptr=namebuf;
	uchar preop=0;

	DEBUG_IN();
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
#define __FUNCTION__ "n_storeval"
	obj_t *nobj;
	uchar op;

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
			nes_strmul(N, cobj, (int)nobj->val->d.num);
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
		case NT_NULL   : nes_unlinkval(N, cobj);
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
#define __FUNCTION__ "n_evalobj"
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
	int n, t;

#define __FUNCTION__ "n_evalmath"
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
				nes_strmul(N, cobj, (int)nobj->val->d.num);
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
#define __FUNCTION__ "n_evalsub"
	obj_t obj2;
	uchar op2;

	while (OP_ISMATH(*N->readptr)) {
		op2=*N->readptr++;
		n_evalobj(N, &obj2);
		if (OP_ISMATH(*N->readptr)&&(oplist[OP_UNDEFINED-*N->readptr].priority>oplist[OP_UNDEFINED-op2].priority)) {
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
#define __FUNCTION__ "nes_eval"
	obj_t obj1;
	uchar *p;
	uchar jmp=N->savjmp?1:0;

	DEBUG_IN();
	if (jmp==0) {
		nes_unlinkval(N, &N->r);
		if (string==NULL||string[0]==0) goto end;
		p=n_decompose(N, (uchar *)string);
		if (p!=N->blockptr) {
			N->blockptr=p;
		} else {
			p=NULL;
		}
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)!=0) goto end;
	} else {
		N->readptr=(uchar *)string;
	}
	sanetest();
	n_evalobj(N, &obj1);
	n_evalsub(N, 0, &obj1);
	nes_unlinkval(N, &N->r);
	N->r.val=obj1.val;
end:
	if (jmp==0) {
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
	va_list ap;
	uchar *buf;
	uchar *oldbptr=N->blockptr;
	uchar *oldbend=N->blockend;
	uchar *oldrptr=N->readptr;
	obj_t *cobj;

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
}
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

#ifdef HAVE_DL
#include "nesla/libdl.h"
#endif
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#else
#include <stdlib.h>
#ifdef __TURBOC__
#else
#include <unistd.h>
#endif
#endif

#include <signal.h>
nes_state *N;

#if defined(__BORLANDC__)
extern char **_environ;
#define environ _environ
#else
extern char **environ;
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
static int flush(nes_state *N)
{
	N->outbuf[N->outbuflen]='\0';
	write(STDOUT_FILENO, N->outbuf, N->outbuflen);
	N->outbuflen=0;
	return 0;
}

static void sig_trap(int sig)
{
	flush(N); /* if we die here, we should flush the buffer first */
	switch (sig) {
	case 11:
		printf("Segmentation Violation\r\n");
		if ((N)&&(N->readptr)) printf("[%.40s]\r\n", N->readptr);
		exit(-1);
	default:
		printf("Unexpected signal [%d] received\r\n", sig);
	}
}

static void setsigs(void)
{
#ifdef WIN32
	signal(SIGSEGV, sig_trap);
#else
#ifndef __TURBOC__
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler=sig_trap;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
#endif
#endif
	return;
}

#define striprn(s) { int n=strlen(s)-1; while (n>-1&&(s[n]=='\r'||s[n]=='\n')) s[n--]='\0'; }

static NES_FUNCTION(neslib_io_gets)
{
	char buf[1024];

	flush(N);
	fgets(buf, sizeof(buf)-1, stdin);
	striprn(buf);
	nes_setstr(N, &N->r, "", buf, -1);
	return 0;
}

static void preppath(nes_state *N, char *name)
{
	char buf[1024];
	char *p;
	unsigned int j;

	p=name;
	if ((name[0]=='/')||(name[0]=='\\')||(name[1]==':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf));
	} else if (name[0]=='.') {
		/* looks relative... */
		getcwd(buf, sizeof(buf)-strlen(name)-2);
		strncat(buf, "/", sizeof(buf)-strlen(buf));
		strncat(buf, name, sizeof(buf)-strlen(buf));
	} else {
		getcwd(buf, sizeof(buf)-strlen(name)-2);
		strncat(buf, "/", sizeof(buf)-strlen(buf));
		strncat(buf, name, sizeof(buf)-strlen(buf));
	}
	for (j=0;j<strlen(buf);j++) {
		if (buf[j]=='\\') buf[j]='/';
	}
	for (j=strlen(buf)-1;j>0;j--) {
		if (buf[j]=='/') { buf[j]='\0'; p=buf+j+1; break; }
	}
	nes_setstr(N, &N->g, "_filename", p, strlen(p));
	nes_setstr(N, &N->g, "_filepath", buf, strlen(buf));
	return;
}

void do_banner() {
	printf("\r\nNullLogic Embedded Scripting Language Version " NESLA_VERSION);
	printf("\r\nCopyright (C) 2007 Dan Cahill\r\n\r\n");
	return;
}

void do_help(char *arg0) {
	printf("Usage : %s [-e] [-f] file.nes\r\n", arg0);
	printf("  -e  execute string\r\n");
	printf("  -f  execute file\r\n\r\n");
	return;
}

int main(int argc, char *argv[])
{
	char tmpbuf[MAX_OBJNAMELEN+1];
	obj_t *tobj;
	int i;
	char *p;
	char c;

	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc<2) {
		do_banner();
		do_help(argv[0]);
		return -1;
	}
	if ((N=nes_newstate())==NULL) return -1;
	setsigs();
	N->debug=0;
	/*
	 * registering functions would go here in a normal cli
	 */
#ifdef HAVE_DL
	nesladl_register_all(N);
#endif
	/* add env */
	tobj=nes_settable(N, &N->g, "_ENV");
	for (i=0;environ[i]!=NULL;i++) {
		strncpy(tmpbuf, environ[i], MAX_OBJNAMELEN);
		p=strchr(tmpbuf, '=');
		if (!p) continue;
		*p='\0';
		p=strchr(environ[i], '=')+1;
		nes_setstr(N, tobj, tmpbuf, p, strlen(p));
	}
	/* add args */
	tobj=nes_settable(N, &N->g, "_ARGS");
	for (i=0;i<argc;i++) {
		n_ntoa(N, tmpbuf, i, 10, 0);
		nes_setstr(N, tobj, tmpbuf, argv[i], strlen(argv[i]));
	}
	tobj=nes_settable(N, &N->g, "io");
	nes_setcfunc(N, tobj, "gets", (NES_CFUNC)neslib_io_gets);
	for (i=1;i<argc;i++) {
		if (argv[i]==NULL) break;
		if (argv[i][0]=='-') {
			c=argv[i][1];
			if (!c) {
				break;
			} else if ((c=='d')||(c=='D')) {
				N->debug=1;
			} else if ((c=='e')||(c=='E')) {
				if (++i<argc) {
					nes_exec(N, argv[i]);
					if (N->err) goto err;
				}
			} else if ((c=='f')||(c=='F')) {
				if (++i<argc) {
					preppath(N, argv[i]);
					nes_execfile(N, argv[i]);
					if (N->err) goto err;
				}
			} else {
				do_help(argv[0]);
				return -1;
			}
		} else {
			preppath(N, argv[i]);
			nes_execfile(N, argv[i]);
			if (N->err) goto err;
		}
	}
err:
	if (N->err) {
		printf("errno=%d (%d) :: \r\n%s", N->err, N->warnings, N->errbuf);
	}
	nes_endstate(N);
	return 0;
}
