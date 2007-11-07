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
#include "nesla/nesla.h"

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
NES_FUNCTION(nl_eval);
NES_FUNCTION(nl_exec);
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

#define  writei4(n,ptr) ptr[0]=(uchar)(n&255); ptr[1]=(uchar)((n>>8)&255); ptr[2]=(uchar)((n>>16)&255); ptr[3]=(uchar)((n>>24)&255);
#define  writei2(n,ptr) ptr[0]=(uchar)(n&255); ptr[1]=(uchar)((n>>8)&255);

#define striprn(s) { int n=strlen(s)-1; while (n>-1&&(s[n]=='\r'||s[n]=='\n')) s[n--]='\0'; }

#endif /* libnesla.h */
