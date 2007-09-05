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

#ifdef TINYCC
/* missing in the supplied win32 headers */
struct timeval { long tv_sec; long tv_usec; };
#endif
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
/* always include winsock2 before windows */
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#else
#ifdef __TURBOC__
struct timeval { long tv_sec; long tv_usec; };
#else
#include <sys/time.h>
#endif
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
obj_t     *nes_linkval    (nes_state *N, obj_t *cobj1, obj_t *cobj2);
void       nes_unlinkval  (nes_state *N, obj_t *cobj);
void       nes_freetable  (nes_state *N, obj_t *tobj);
obj_t     *nes_getobj     (nes_state *N, obj_t *tobj, char *oname);
obj_t     *nes_getiobj    (nes_state *N, obj_t *tobj, int oindex);
obj_t     *nes_setobj     (nes_state *N, obj_t *tobj, char *oname, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, int _slen);
obj_t     *nes_strcat     (nes_state *N, obj_t *cobj, char *str, int len);
num_t      nes_tobool     (nes_state *N, obj_t *cobj);
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
