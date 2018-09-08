/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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

#ifdef __cplusplus
extern "C" {
#endif

#define _NSPLIB_H 1
#ifndef _NSP_H
#include "nsp/nsp.h"
#endif

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
#define NE_EXCEPTION	3
#define NE_INTERNAL	4

#define sanetest()	/*{ if (N->readptr==NULL) n_error(N, NE_SYNTAX, __FN__, "NULL readptr"); }*/
#define settrace()	/* if (N!=NULL) { N->tracefn=__FN__; nl_flush(N); printf("%s\n", __FN__); } */
#define DEBUG_IN()
#define DEBUG_OUT()

/* block.c */
void     n_skipto(nsp_state *N, const char *fn, unsigned short c);
void     n_if(nsp_state *N);
void     n_for(nsp_state *N);
void     n_foreach(nsp_state *N);
void     n_switch(nsp_state *N);
void     n_do(nsp_state *N);
void     n_while(nsp_state *N);
void     n_try(nsp_state *N);
/* compile.c */
uchar   *n_decompose(nsp_state *N, char *srcfile, uchar *srctext, uchar **dsttext, int *dstsize);
/* debug.c */
void     n_decompile(nsp_state *N, uchar *start, uchar *end, char *errbuf, unsigned short errmax);
/* exec.c */
obj_t   *n_execfunction(nsp_state *N, obj_t *fobj, obj_t *pobj, uchar isnewobject);
obj_t   *n_execbasemethod(nsp_state *N, char *name, obj_t *pobj);
void     n_execconstructor(nsp_state *N, obj_t *cobj, obj_t *pobj);
void     n_execdestructor(nsp_state *N, obj_t *cobj, char *cname);
/* libc.c */
#define  nc_isdigit(c)  (c>='0'&&c<='9')
#define  nc_isalpha(c)  ((c>='A'&&c<='Z')||(c>='a'&&c<='z'))
#define  nc_isalnum(c)  ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9'))
#define  nc_isupper(c)  (c>='A'&&c<='Z')
#define  nc_islower(c)  (c>='a'&&c<='z')
#define  nc_isspace(c)  (c=='\r'||c=='\n'||c=='\t'||c==' ')
#define  nc_tolower(c)  ((c>='A'&&c<='Z')?(c+('a'-'A')):c)
#define  nc_toupper(c)  ((c>='a'&&c<='z')?(c-('a'-'A')):c)
int      nc_snprintf(nsp_state *N, char *str, int size, const char *format, ...);
int      nc_printf(nsp_state *N, const char *format, ...);
int      nc_gettimeofday(struct timeval *tv, struct timezone *tz);

char    *_nc_memcpy(char *dst, const char *src, int n);
int      _nc_strlen(const char *s);
char    *_nc_strchr(const char *s, int c);
char    *_nc_strncpy(char *d, const char *s, int n);
int      _nc_strcmp(const char *s1, const char *s2);
int      _nc_strncmp(const char *s1, const char *s2, int n);
void    *_nc_memset(void *s, int c, int n);

#define nc_memcpy  _nc_memcpy  // native is unsafe
#define nc_strlen  _nc_strlen  // slower?
#define nc_strchr  _nc_strchr  // same
#define nc_strncpy _nc_strncpy // same
#define nc_strcmp  _nc_strcmp  // native is unsafe
#define nc_strncmp _nc_strncmp // same
#define nc_memset  _nc_memset  // same

void     n_error(nsp_state *N, short int err, const char *fname, const char *format, ...);
/* #define  n_expect(N,fn,op)	{ if (*N->readptr!=op) n_error(N, NE_SYNTAX, fn, "expected a '%s'", n_getsym(N, op)); } */
void     _n_expect(nsp_state *N, const char *fname, uchar op);
int      _n_expect_argtype(nsp_state *N, unsigned short argn, unsigned short type, obj_t *obj);
void     n_warn(nsp_state *N, const char *fname, const char *format, ...);
num_t    n_aton(nsp_state *N, const char *str);
/* char    *n_itoa         (nsp_state *N, char *str, int num, short base); */
char    *n_ntoa(nsp_state *N, char *str, num_t num, short base, unsigned short dec);
/* libn.c */
NSP_FUNCTION(nl_break);
NSP_FUNCTION(nl_flush);
NSP_FUNCTION(nl_print);
NSP_FUNCTION(nl_write);
NSP_FUNCTION(nl_dl_load);
NSP_FUNCTION(nl_filemkdir);
NSP_FUNCTION(nl_filechdir);
NSP_FUNCTION(nl_fileexists);
NSP_FUNCTION(nl_filereadall);
NSP_FUNCTION(nl_filerename);
NSP_FUNCTION(nl_filestat);
NSP_FUNCTION(nl_fileunlink);
NSP_FUNCTION(nl_filewriteall);
NSP_FUNCTION(nl_math);
NSP_FUNCTION(nl_tonumber);
NSP_FUNCTION(nl_tostring);
NSP_FUNCTION(nl_atoi);
NSP_FUNCTION(nl_itoa);
NSP_FUNCTION(nl_strcat);
NSP_FUNCTION(nl_strcmp);
NSP_FUNCTION(nl_strcontains);
NSP_FUNCTION(nl_strjoin);
NSP_FUNCTION(nl_strlen);
NSP_FUNCTION(nl_strrep);
NSP_FUNCTION(nl_strsplit);
NSP_FUNCTION(nl_strstr);
NSP_FUNCTION(nl_strsub);
NSP_FUNCTION(nl_strtolower);
NSP_FUNCTION(nl_strtrim);
NSP_FUNCTION(nl_asctime);
NSP_FUNCTION(nl_sqltounix);
NSP_FUNCTION(nl_mktime);
NSP_FUNCTION(nl_time);
NSP_FUNCTION(nl_gmtime);
NSP_FUNCTION(nl_gettimeofday);
NSP_FUNCTION(nl_sleep);
NSP_FUNCTION(nl_runtime);
NSP_FUNCTION(nl_eval);
NSP_FUNCTION(nl_exec);
NSP_FUNCTION(nl_iname);
NSP_FUNCTION(nl_ival);
NSP_FUNCTION(nl_include);
NSP_FUNCTION(nl_serialize);
NSP_FUNCTION(nl_sizeof);
NSP_FUNCTION(nl_sort_name);
NSP_FUNCTION(nl_sort_key);
NSP_FUNCTION(nl_typeof);
NSP_FUNCTION(nl_system);
NSP_FUNCTION(nl_copy);
NSP_FUNCTION(nl_printf);
NSP_FUNCTION(nl_zlink);
NSP_FUNCTION(nl_base_method);
/* objects.c */
void    *n_alloc(nsp_state *N, int size, short zero);
void    *n_realloc(nsp_state *N, void **p, int size, int osize, short zero);
void     n_free(nsp_state *N, void **p, int osize);
void     n_copyval(nsp_state *N, obj_t *cobj1, obj_t *cobj2);
void     n_moveval(nsp_state *N, obj_t *cobj1, obj_t *cobj2);
val_t   *n_newval(nsp_state *N, unsigned short type);
void     n_freeval(nsp_state *N, obj_t *cobj);
obj_t   *n_newiobj(nsp_state *N, int index);
obj_t   *n_setname(nsp_state *N, obj_t *cobj, const char *name);
obj_t   *n_setnamei(nsp_state *N, obj_t *cobj, unsigned long i);
/* opcodes.c */
short    n_iskeyword(nsp_state *N, char *name);
short    n_getop(nsp_state *N, char *name);
char    *n_getsym(nsp_state *N, short op);
char    *n_gettype(nsp_state *N, short type);
uchar   *n_seekop(nsp_state *N, uchar *readptr, unsigned short sb);
uchar    n_skip_ws(nsp_state *N);
/* parser.c */
void     n_getfunction(nsp_state *N, obj_t *cobj);
char    *n_getlabel(nsp_state *N, char *buf);
num_t    n_getnumber(nsp_state *N);
void     n_getstring(nsp_state *N, obj_t *cobj);
obj_t   *n_readindex(nsp_state *N, obj_t *cobj, char *lastname, unsigned short *z);
void     n_readtable(nsp_state *N, obj_t *tobj);
obj_t   *n_readvar(nsp_state *N, obj_t *tobj, obj_t *cobj);
void     n_storeval(nsp_state *N, obj_t *cobj);

#define  readi4(ptr)     (int)(ptr[0]+(ptr[1]<<8)+(ptr[2]<<16)+(ptr[3]<<24))
#define  readi2(ptr)     (int)(ptr[0]+(ptr[1]<<8))

#define  writei4(n,ptr)  ptr[0]=(uchar)(n&255); ptr[1]=(uchar)((n>>8)&255); ptr[2]=(uchar)((n>>16)&255); ptr[3]=(uchar)((n>>24)&255);
#define  writei2(n,ptr)  ptr[0]=(uchar)(n&255); ptr[1]=(uchar)((n>>8)&255);

#define  striprn(s)      { int n=nc_strlen(s)-1; while (n>-1&&(s[n]=='\r'||s[n]=='\n')) s[n--]='\0'; }

#define  n_peekop(N)     (*N->readptr!=OP_LINENUM?*N->readptr:n_skip_ws(N))
#define  n_expect(N,f,o) if (n_peekop(N)!=o) _n_expect(N, f, o);

int n_expect_argtype(nsp_state *N, unsigned short type, unsigned short argnum, obj_t *obj, unsigned short allow_blank);
//#define  n_expect_argtype(N,a,t,o,ab) _n_expect_argtype(N, a, t, o, ab);

#ifdef __cplusplus
}
#endif

#endif /* _NSPLIB_H */
