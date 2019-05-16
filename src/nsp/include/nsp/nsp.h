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
#ifndef _NSP_H
#define _NSP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define NSP_NAME      "nesla"
#define NSP_VERSION   "0.9.4"

#if defined(TINYCC)||defined(__TURBOC__)
struct timeval { long tv_sec; long tv_usec; };
struct timezone { int tz_minuteswest; int tz_dsttime; };
#endif
#if defined(_MSC_VER)
struct timezone { int tz_minuteswest; int tz_dsttime; };
#pragma warning(disable:4996)
#define WIN32_LEAN_AND_MEAN
#define _USE_32BIT_TIME_T
/* always include winsock2 before windows */
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#if defined(WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif


#elif !defined(__TURBOC__)
#include <sys/time.h>
#include <stddef.h>
#endif
#include <setjmp.h>

/* need to add size sanity here */
#if defined(__TURBOC__)
typedef signed long int   int32;
typedef unsigned long int uint32;
#elif defined(_LP64)
typedef signed int        int32;
typedef unsigned int      uint32;
#else
typedef signed int        int32;
typedef unsigned int      uint32;
#endif
typedef signed char       int8;
typedef unsigned char     uint8;

#define MAX_OBJNAMELEN  64
#define MAX_OUTBUFSIZE  4096
#define OUTBUFLOWAT	2048

/* object types */
#define NT_NULL         0
#define NT_BOOLEAN      1
#define NT_NUMBER       2
#define NT_STRING       3
#define NT_NFUNC        4
#define NT_CFUNC        5
#define NT_TABLE        6
#define NT_CDATA        7

/* object status flags */
#define NST_HIDDEN	0x01
#define NST_READONLY	0x02
#define NST_SYSTEM	0x04
#define NST_AUTOSORT	0x08
#define NST_LINK	0x10
#define NST_STACKVAL	0x20

#define num_t double
#define uchar unsigned char
#define obj_t struct nsp_objrec
#define tab_t struct nsp_tablerec
#define val_t struct nsp_valrec
#define nsp_t struct nsp_state

/* should be typedef int(*NSP_CFUNC)(nsp_state *); */
typedef int(*NSP_CFUNC)(void *);
#define NSP_FUNCTION(name)    int name(nsp_state *N)
#define NSP_CLASS(name)       int name(nsp_state *N)
#define NSP_CLASSMETHOD(name) int name(nsp_state *N)

/*
 * define a callback function type so CDATA objects
 * can choose the terms of their own death.
 */
/* should be typedef void(*NSP_CFREE)(nsp_state *, obj_t *); */
typedef void(*NSP_CFREE)(void *, void *);

typedef struct NSP_CDATA {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's type-specific */
} NSP_CDATA;
typedef struct nsp_tablerec {
	obj_t *f;
	obj_t *i;
	obj_t *l;
} nsp_tablerec;
typedef struct nsp_valrec {
	unsigned short type; /* val type */
	unsigned short attr; /* status flags (hidden, readonly, system, autosort, etc...) */
	unsigned short refs; /* number of references to this node */
	unsigned long  size; /* storage size of string, nfunc or cdata */
	obj_t *ztable;       /* 'z' table for hierarchical lookups */
	union {
		num_t  num;
		char  *str;
		NSP_CFUNC cfunc;
		NSP_CDATA *cdata;
		tab_t table;
	} d;
} nsp_valrec;
typedef struct nsp_objrec {
	obj_t *prev;
	obj_t *next;
	val_t *val;
	uint32 hash;
	signed long nval;
	char name[MAX_OBJNAMELEN + 1];
} nsp_objrec;
typedef struct nsp_execcontext {
	obj_t l; // local variables
	obj_t t; // 'this' object
	uchar *blockptr;
	uchar *blockend;
	uchar *readptr;
	uchar yielded;
	char *funcname;
	char *filename;
	char *tracefn;
	long int linenum;
} nsp_execcontext;
typedef struct nsp_state {
	jmp_buf *savjmp;
	nsp_execcontext *context; // local execution context
	obj_t g; // global variables
	obj_t r; // return variable
	short brk;
	short cnt;
	short ret;
	short err;
	short signal; /* intended for external signals to the parser.  for now, non-zero just means to shut down */
	short debug;
	short single;
	short yielded;
	short strict;
	short warnings;
	short maxwarnings;
	char warnformat;
	struct timeval ttime;
	char numbuf[128];
	char *outbuffer;
	unsigned short outbuflen;
	unsigned short outbufmax;
	char errbuf[256];
	/* debug info */
	long int allocs;
	long int allocmem;
	long int frees;
	long int freemem;
	long int peakmem;
	long int counter1;
} nsp_state;

#ifndef NSP_NOFUNCTIONS
/* exec */
nsp_state *nsp_newstate(void);
void       nsp_freestate(nsp_state *N);
nsp_state *nsp_endstate(nsp_state *N);
obj_t     *nsp_exec(nsp_state *N, const char *string);
int        nsp_execfile(nsp_state *N, char *file);
/* objects */
void       nsp_setvaltype(nsp_state *N, obj_t *cobj, unsigned short type);
void       nsp_linkval(nsp_state *N, obj_t *cobj1, obj_t *cobj2);
void       nsp_unlinkval(nsp_state *N, obj_t *cobj);
void       nsp_freetable(nsp_state *N, obj_t *tobj);
obj_t     *nsp_getobj_ex(nsp_state *N, obj_t *tobj, char *oname, unsigned short followz, unsigned short *foundz);
obj_t     *nsp_getobj(nsp_state *N, obj_t *tobj, char *oname);
obj_t     *nsp_getiobj(nsp_state *N, obj_t *tobj, unsigned long oindex);
obj_t     *nsp_setobj(nsp_state *N, obj_t *tobj, char *oname, unsigned short otype, NSP_CFUNC _fptr, num_t _num, char *_str, size_t _slen);
obj_t     *nsp_appendobj(nsp_state *N, obj_t *tobj, char *name);
void       nsp_strcat(nsp_state *N, obj_t *cobj, char *str, long len);
void       nsp_strmul(nsp_state *N, obj_t *cobj, unsigned long n);
short      nsp_tobool(nsp_state *N, obj_t *cobj);
num_t      nsp_tonum(nsp_state *N, obj_t *cobj);
char      *nsp_tostr(nsp_state *N, obj_t *cobj);
char      *nsp_zlink(nsp_state *N, obj_t *cobj1, obj_t *cobj2);
/* parser */
obj_t     *nsp_eval(nsp_state *N, const char *string);
obj_t     *nsp_evalf(nsp_state *N, const char *fmt, ...);
#endif

//#define    nsp_isnull(o)            (o==NULL||o->val==NULL||o->val->type==NT_NULL)
#define    nsp_isnull(o)            (o==NULL||o->val==NULL||o->val->type==NT_NULL)
#define    nsp_isbool(o)            (o!=NULL&&o->val!=NULL&&o->val->type==NT_BOOLEAN)
#define    nsp_isnum(o)             (o!=NULL&&o->val!=NULL&&o->val->type==NT_NUMBER)
#define    nsp_isstr(o)             (o!=NULL&&o->val!=NULL&&o->val->type==NT_STRING)
#define    nsp_istable(o)           (o!=NULL&&o->val!=NULL&&o->val->type==NT_TABLE)

#define    nsp_istrue(o)            (nsp_tobool(N, o)?1:0)

#define    nsp_typeof(o)            (nsp_isnull(o)?NT_NULL:o->val->type)

#define    nsp_getnum(N,o,n)        nsp_tonum(N, nsp_getobj(N,o,n))
#define    nsp_getstr(N,o,n)        nsp_tostr(N, nsp_getobj(N,o,n))

#define    nsp_setnull(N,t,n)       nsp_setobj(N, t, n, NT_NULL,    (NSP_CFUNC)NULL, 0, NULL, 0)
#define    nsp_setnum(N,t,n,v)      nsp_setobj(N, t, n, NT_NUMBER,  (NSP_CFUNC)NULL, v, NULL, 0)
#define    nsp_setbool(N,t,n,v)     nsp_setobj(N, t, n, NT_BOOLEAN, (NSP_CFUNC)NULL, v?1:0, NULL, 0)
#define    nsp_setstr(N,t,n,s,l)    nsp_setobj(N, t, n, NT_STRING,  (NSP_CFUNC)NULL, 0, s,    l)
#define    nsp_settable(N,t,n)      nsp_setobj(N, t, n, NT_TABLE,   (NSP_CFUNC)NULL, 0, NULL, 0)
#define    nsp_setcfunc(N,t,n,p)    nsp_setobj(N, t, n, NT_CFUNC,   (NSP_CFUNC)p,    0, NULL, 0)
#define    nsp_setnfunc(N,t,n,s,l)  nsp_setobj(N, t, n, NT_NFUNC,   (NSP_CFUNC)NULL, 0, s,    l)
#define    nsp_setcdata(N,t,n,s,l)  nsp_setobj(N, t, n, NT_CDATA,   (NSP_CFUNC)NULL, 0, (void *)s, l)

#define    nsp_setinum(N,t,n,v)     nsp_setiobj(N, t, n, NT_NUMBER, (NSP_CFUNC)NULL, v, NULL, 0)
#define    nsp_setistr(N,t,n,s,l)   nsp_setiobj(N, t, n, NT_STRING, (NSP_CFUNC)NULL, 0, s,    l)
#define    nsp_setitable(N,t,n)     nsp_setiobj(N, t, n, NT_TABLE,  (NSP_CFUNC)NULL, 0, NULL, 0)
#define    nsp_seticfunc(N,t,n,p)   nsp_setiobj(N, t, n, NT_CFUNC,  (NSP_CFUNC)p,    0, NULL, 0)
#define    nsp_setinfunc(N,t,n,s,l) nsp_setiobj(N, t, n, NT_NFUNC,  (NSP_CFUNC)NULL, 0, s,    l)
#define    nsp_seticdata(N,t,n,s,l) nsp_setiobj(N, t, n, NT_CDATA,  (NSP_CFUNC)NULL, 0, (void *)s, l)

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class NesObject {
public:
	NesObject()
	{
		/* memset(O, 0, sizeof(O)); */
	}
	~NesObject()
	{
		/* nsp_unlink(O); */
	}
	const char *errbuf()
	{
		/* return this->N->errbuf; */
	}
private:
	obj_t O;
};

class NSPState {
private:
	nsp_state *N;
public:
	NSPState()
	{
		this->N=nsp_newstate();
	}
	~NSPState()
	{
		nsp_endstate(this->N);
	}
	obj_t *exec(const char *string)
	{
		return nsp_exec(this->N, string);
	}
	int execfile(char *file)
	{
		return nsp_execfile(this->N, file);
	}
	obj_t *eval(const char *string)
	{
		return nsp_eval(this->N, string);
	}
	/*
	obj_t *evalf(const char *fmt, ...)
	{
	return nsp_evalf(this->N, fmt, ...);
	}
	*/
	obj_t *getG()
	{
		return &this->N->g;
	}
	obj_t *getL()
	{
		return &this->N->context->l;
	}
	nsp_t *getN()
	{
		return this->N;
	}
	void setdebug(int x)
	{
		this->N->debug=x;
	}
	int err()
	{
		return this->N->err;
	}
	int warnings()
	{
		return this->N->warnings;
	}
	const char *errbuf()
	{
		return this->N->errbuf;
	}
};
#endif

#endif /* _NSP_H */
