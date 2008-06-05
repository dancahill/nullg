/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2008 Dan Cahill

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

#ifdef WIN32
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
/* pthread_ typedefs */
typedef HANDLE pthread_t;
typedef struct thread_attr {
	DWORD dwStackSize;
	DWORD dwCreatingFlag;
	int priority;
} pthread_attr_t;
typedef struct {
	int dummy;
} pthread_condattr_t;
typedef unsigned int uint;
typedef struct {
	uint waiting;
	HANDLE semaphore;
} pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

/* MD5 */
#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif
struct MD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};
typedef struct MD5Context MD5_CONTEXT;
#define MD5_SIZE 16
/* end MD5 */

#ifndef NGS_NOFUNCTIONS
/* config.c */
int config_new(nes_state *N);
int config_read(nes_state *N, char *section, void *callback);
int config_write(nes_state *N);
/* dirlist.c */
int dir_list(nes_state *N, obj_t **dobj, const char *dirname);
/* format.c */
char *fixslashes(char *string);
char *striprn(char *string);
void swapchar(char *string, char oldchar, char newchar);
/* lib.c */
void *lib_open(const char *file);
void *lib_sym(void *handle, const char *name);
char *lib_error(void);
int   lib_close(void *handle);
/* log.c */
void log_access(nes_state *N, char *logsrc, const char *format, ...);
void log_error(nes_state *N, char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...);
/* md5.c */
void    md5_init(struct MD5Context *ctx);
void    md5_update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);
void    md5_final(unsigned char digest[16], struct MD5Context *ctx);
char   *md5_crypt(char *passwd, char *pw, char *salt);
/* sql.c */
void  _sql_disconnect(nes_state *N);
void  _sql_freeresult(nes_state *N, obj_t **qptr);
int   _sql_update(nes_state *N, char *sqlquery);
int   _sql_query(nes_state *N, obj_t **qptr, char *query);
int   _sql_updatef(nes_state *N, char *format, ...);
int   _sql_queryf(nes_state *N, obj_t **qptr, char *format, ...);
char *sql_getname(nes_state *N, obj_t **qptr, int fieldnumber);
char *sql_getvalue(nes_state *N, obj_t **qptr, int tuple, int field);
char *sql_getvaluebyname(nes_state *N, obj_t **qptr, int tuple, char *fieldname);
int   sql_numfields(nes_state *N, obj_t **qptr);
int   sql_numtuples(nes_state *N, obj_t **qptr);
char *str2sql(char *outstring, int outlen, char *instring);
/* sys.c */
int     sys_system(const char *format, ...);
/* win32.c functions */
#ifdef WIN32
int      gettimeofday(struct timeval *tv, struct timezone *tz);
unsigned sleep(unsigned seconds);
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
#define pthread_exit(A) _endthread()
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
int pthread_kill(pthread_t handle, int sig);
#endif
#endif
