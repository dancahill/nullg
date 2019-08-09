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
#ifndef _NSPLIB_H
#include "nsp/nsplib.h"
#endif
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <direct.h>
#include <io.h>
#elif !defined( __TURBOC__)
#include <signal.h>
#include <unistd.h>
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* works ok, but written to handle direct output */
static int n_escape(nsp_state *N, const char *str, int len)
{
#define __FN__ __FILE__ ":n_escape()"
	char buf[512];
	char *dst = buf;
	int i, n = 0;

	settrace();
	for (i = 0; i < len; i++) {
		if (sizeof(buf) - n < 32) { nsp_strcat(N, &N->r, buf, n); n = 0; }
		switch (str[i]) {
		case '\0': dst[n++] = '\\'; dst[n++] = 0;   break;
		case '\a': dst[n++] = '\\'; dst[n++] = 'a'; break;
		case '\t': dst[n++] = '\\'; dst[n++] = 't'; break;
		case '\f': dst[n++] = '\\'; dst[n++] = 'f'; break;
		case 27: dst[n++] = '\\'; dst[n++] = 'e'; break;
		case '\r': dst[n++] = '\\'; dst[n++] = 'r'; break;
		case '\n': dst[n++] = '\\'; dst[n++] = 'n'; break;
		case '\'': dst[n++] = '\\'; dst[n++] = '\''; break;
		case '\"': dst[n++] = '\\'; dst[n++] = '\"'; break;
		case '\\': dst[n++] = '\\'; dst[n++] = '\\'; break;
		default: dst[n++] = str[i];
		}
	}
	nsp_strcat(N, &N->r, buf, n);
	return len;
#undef __FN__
}

void n_dumpvars(nsp_state *N, obj_t *tobj, int depth)
{
#define __FN__ __FILE__ ":n_dumpvars()"
	obj_t *cobj = tobj, *robj;
	int i;
	char b;
	char *g;
	int ent = 0;

	char buf[512];
	unsigned short buflen = 0;
	char *p;

	if (depth == 0) robj = nsp_setstr(N, &N->r, "", NULL, 0); else robj = &N->r;
	settrace();
	for (; cobj; cobj = cobj->next) {
		if (buflen > 512) {
			nsp_strcat(N, robj, buf, buflen);
			buflen = 0;
		}
		if (nsp_isnull(cobj) || cobj->val->attr&NST_HIDDEN || cobj->val->attr&NST_SYSTEM) continue;
		g = (depth < 1) ? "global " : "";

		b = 0;
		if (nc_isdigit(cobj->name[0])) b = 1;
		for (p = cobj->name; *p != '\0'; p++) {
			if (!nc_isalnum(*p) && *p != '_') b = 1;
		}
		if (n_iskeyword(N, cobj->name)) b = 1;

		if (cobj->val->type == NT_BOOLEAN || cobj->val->type == NT_NUMBER) {
			if (ent++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s\n", depth ? "," : "");
			if (depth) {
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s = ", g, b ? "[\"" : "", cobj->name, b ? "\"]" : "");
			}
			buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s", nsp_tostr(N, cobj));
		}
		else if (cobj->val->type == NT_STRING) {
			if (ent++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s\n", depth ? "," : "");
			if (depth) {
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s = ", g, b ? "[\"" : "", cobj->name, b ? "\"]" : "");
			}
			buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\"");
			nsp_strcat(N, robj, buf, buflen);
			buflen = 0;
			n_escape(N, cobj->val->d.str ? cobj->val->d.str : "", cobj->val->size);
			buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\"");
		}
		else if (cobj->val->type == NT_TABLE) {
			if (nc_strcmp(cobj->name, "_GLOBALS") == 0) continue;
			/* if (nc_strcmp(cobj->name, "this")==0) continue; */
			if (ent++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s\n", depth ? "," : "");
			if (depth) {
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s = ", g, b ? "[\"" : "", cobj->name, b ? "\"]" : "");
			}
			buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "{");
			if (cobj->val->d.table.f) {
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\n");
				nsp_strcat(N, robj, buf, buflen);
				buflen = 0;
				n_dumpvars(N, cobj->val->d.table.f, depth + 1);
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "}");
			}
			else {
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, " }");
			}
		}
	}
	if (ent) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s", depth ? "\n" : "");
	nsp_strcat(N, robj, buf, buflen);
	buflen = 0;
	return;
#undef __FN__
}

/*
 * basic i/o functions
 */
NSP_FUNCTION(nl_flush)
{
#define __FN__ __FILE__ ":nl_flush()"
	obj_t *cobj;
	int rc;

	if (N == NULL || N->outbuflen == 0) return 0;
	cobj = &N->g;
	if (!nsp_istable(cobj)) goto flush;
	for (cobj = cobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (nc_strcmp(cobj->name, "io") != 0) continue;
		if (cobj->val->type != NT_TABLE) goto flush;
		for (cobj = cobj->val->d.table.f; cobj; cobj = cobj->next) {
			if (nc_strcmp(cobj->name, "flush") != 0) continue;
			if (cobj->val->type != NT_CFUNC) goto flush;
			if (cobj->val->d.cfunc == (NSP_CFUNC)nl_flush) goto flush;
			return cobj->val->d.cfunc(N);
		}
	}
flush:
	if (N->outbuflen == 0) return 0;
	N->outbuffer[N->outbuflen] = '\0';
	if ((rc = write(STDOUT_FILENO, N->outbuffer, N->outbuflen)) != N->outbuflen) {
#if defined(WIN32) && defined(_DEBUG)
		_RPT1(_CRT_WARN, "nl_flush write() wrote less bytes than expected\r\n", "");
#endif
		N->outbuflen = 0;
	}
	N->outbuflen = 0;
	/* do NOT touch &N->r */
	return 0;
#undef __FN__
}

static int writestr(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":writestr()"
	unsigned int i;
	unsigned int len = 0;
	char *p;

	settrace();
	if (nsp_isstr(cobj)) {
		p = cobj->val->d.str;
		len = cobj->val->size;
	}
	else {
		p = nsp_tostr(N, cobj);
		len = nc_strlen(p);
	}
	for (i = 0; i < len; i++) {
		if (N->outbuflen > OUTBUFLOWAT) nl_flush(N);
		N->outbuffer[N->outbuflen++] = p[i];
	}
	N->outbuffer[N->outbuflen] = '\0';
	return len;
#undef __FN__
}

NSP_FUNCTION(nl_print)
{
#define __FN__ __FILE__ ":nl_print()"
	/* obj_t *cobj=N->l.val->d.table.f; */
	obj_t *cobj = nsp_getobj(N, &N->context->l, "0");
	int tlen = 0;

	settrace();
	for (cobj = cobj->next; cobj; cobj = cobj->next) {
		if (cobj->name[0] != 'n') tlen += writestr(N, cobj);
	}
	if (N->debug) nl_flush(N);
	nsp_setnum(N, &N->r, "", tlen);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_write)
{
#define __FN__ __FILE__ ":nl_write()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int len;

	settrace();
	len = writestr(N, cobj1);
	nsp_setnum(N, &N->r, "", len);
	return 0;
#undef __FN__
}

/* debug */
NSP_FUNCTION(nl_break)
{
#define __FN__ __FILE__ ":nl_break()"
#if defined(WIN32)
#if defined(_DEBUG)
	__debugbreak();
	//DebugBreak();
#endif
#else
	//__builtin_trap();
	raise(SIGINT);
#endif
	return 0;
#undef __FN__
}

/*
 * coroutine functions
 */

typedef struct COROUTINE {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's object-specific */
	nsp_execcontext *ctx;
	short firstrun;
	//uchar *p;
	int plen;
	uchar *lastreadptr;
} COROUTINE;

static COROUTINE *getcoroutineconn(nsp_state *N)
{
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	COROUTINE *conn;

	cobj = nsp_getobj(N, thisobj, "costate");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "coroutine") != 0))
	{
		//n_warn(N, "", "coroutine: conn not found");
		return NULL;
	}
	//n_error(N, NE_SYNTAX, "coroutine", "expected a coroutine");
	conn = (COROUTINE *)cobj->val->d.str;
	return conn;
}

static void coroutine_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":coroutine_murder()"
	COROUTINE *conn;

	//n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "coroutine") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a coroutine");

	conn = (COROUTINE *)cobj->val->d.str;
	//conn->ctx->coroutine = 0;
	n_freeexeccontext(N, &conn->ctx);

	//if (conn->p) n_free(N, (void *)&conn->p, conn->plen);
	n_free(N, (void *)&cobj->val->d.str, sizeof(COROUTINE) + 1);
	return;
#undef __FN__
}

NSP_FUNCTION(nl_coroutine_constructor)
{
#define __FN__ __FILE__ ":nl_coroutine()"
	COROUTINE *conn;
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;

	settrace();

	nsp_setstr(N, thisobj, "status", "new", -1);


	//n_warn(N, __FN__, "coroutine constructor called");

	conn = n_alloc(N, sizeof(COROUTINE) + 1, 1);
	nc_strncpy(conn->obj_type, "coroutine", sizeof(conn->obj_type) - 1);
	conn->obj_term = (NSP_CFREE)coroutine_murder;

	cobj = nsp_setcdata(N, thisobj, "costate", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(COROUTINE) + 1;

	//n_dumpvars(N, &N->context->l, 0);
	//n_warn(N, __FN__, "coroutine constructor: &N->context->l='%s'", N->r.val->d.str);

	conn->ctx = N->context;
	conn->firstrun = 1;
	//conn->p = NULL;
	conn->plen = 0;

	//n_warn(N, __FN__, "sizeof(COROUTINE)=%d", sizeof(COROUTINE));


	//nsp_linkval(N, &conn->ctx->l, &N->context->l);

	//cobj = nsp_appendobj(N, &N->context->l, "this");
	//nsp_linkval(N, cobj, nsp_getobj(N, thisobj, "this"));

	//n_dumpvars(N, &N->context->l, 0);
	//n_warn(N, __FN__, "coroutine constructor b: &N->context->l='%s'", N->r.val->d.str);

	nsp_execcontext *oldctx = N->context;
	N->context = conn->ctx;

	//nsp_setbool(N, thisobj, "yieldadded", 1);

	N->context = oldctx;

	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_coroutine)
{
#define __FN__ __FILE__ ":nl_coroutine()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	//obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	COROUTINE *conn;
	jmp_buf *savjmp;
	int e;

	settrace();
	if (nc_strcmp(fname, "coroutine") == 0) {
	}
	else if (nc_strcmp(fname, "resume") == 0) {
		//n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "coroutine resumed: &N->context->l='%s'", N->r.val->d.str);
		//nsp_unlinkval(N, &N->r);

		nsp_execcontext *oldctx = N->context;
		conn = getcoroutineconn(N);
		if (!conn) {
			//n_warn(N, __FN__, "coroutine resume: conn not found");
			return 0;
		}

		N->context = conn->ctx;

		//n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "coroutine resumed: &N->context->l='%s'", N->r.val->d.str);
		//nsp_unlinkval(N, &N->r);

		if (conn->firstrun) {
			//n_warn(N, __FN__, "coroutine resumed - first run, context=0x%08x readptr=0x%08x", N->context, n_context_readptr);
			conn->lastreadptr = n_context_readptr;
		}
		else {
			//n_warn(N, __FN__, "coroutine resumed, context=0x%08x readptr=0x%08x", N->context, n_context_readptr);
			n_context_readptr = conn->lastreadptr;
		}
		conn->firstrun = 0;

		//n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "coroutine resumed: &N->context->l='%s'", N->r.val->d.str);
		//nsp_unlinkval(N, &N->r);

		savjmp = n_context_savjmp;
		n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if ((e = setjmp(*n_context_savjmp)) == 0) {
			//if (N->yielded) {
			//	n_warn(N, __FN__, "already yielded?");
			//}
			//N->yielded = 1;
			//nsp_exec(N, NULL);
			nsp_exec(N, (char *)n_context_readptr);
			conn->lastreadptr = n_context_readptr;
			//if (N->yielded) {
			//	n_warn(N, __FN__, "coroutine was yielded at context=0x%08x readptr=0x%08x", N->context, n_context_readptr);
			//}

			//n_warn(N, __FN__, "coroutine resume = N->yielded=%d", N->yielded);
		}
		n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
		n_context_savjmp = savjmp;
		if (!N->yielded) {
			obj_t *cobj = nsp_getobj(N, thisobj, "costate");
			if (cobj) coroutine_murder(N, cobj);
			nsp_setstr(N, thisobj, "status", "done", -1);
		}
		else {
			nsp_setstr(N, thisobj, "status", "waiting", -1);
		}
		N->yielded = 0;
		N->context = oldctx;
		//n_warn(N, __FN__, "coroutine resume: done");
	}
	else if (nc_strcmp(fname, "yield") == 0) {
		//if (n_peekop(N) == OP_PSEMICOL) n_context_readptr++;
		N->yielded = 1;
		//n_warn(N, __FN__, "coroutine yield");
		//n_warn(N, __FN__, "coroutine '%s' called at '0x%08x'", fname, n_context_readptr);
		conn = getcoroutineconn(N);
		if (!conn) n_error(N, NE_SYNTAX, __FN__, "object is missing a costate");
		conn->lastreadptr = n_context_readptr;
		n_dumpvars(N, &N->context->l, 0);
		//n_warn(N, __FN__, "coroutine yield: &newctx->l='%s'", N->r.val->d.str);
		nsp_unlinkval(N, &N->r);
	}
	else if (nc_strcmp(fname, "status") == 0) {
		conn = getcoroutineconn(N);
		n_warn(N, __FN__, "coroutine '%s' empty", fname);
	}
	else {
		n_warn(N, __FN__, "coroutine method '%s' missing?", fname);
	}
	return 0;
#undef __FN__
}

/*
 * dl
 */

#include <stdio.h>
#ifdef WIN32
#define snprintf _snprintf
#endif

#define HAVE_DL
#ifdef HAVE_DL

 //#include "libdl.h"
#ifdef WIN32
#define LIBEXT "dll"
#else
#include <dlfcn.h>
#define LIBEXT "so"
#endif

static void *lib_open(const char *file)
{
#ifdef WIN32
	return LoadLibraryA(file);
#else
	return dlopen(file, RTLD_NOW);
#endif
}

static void *lib_sym(void *handle, const char *name)
{
#ifdef WIN32
	return GetProcAddress(handle, name);
#else
	return dlsym(handle, name);
#endif
}

static char *lib_error(nsp_state *N)
{
#ifdef WIN32
	LPTSTR lpMsgBuf;
	DWORD rc;

	rc = GetLastError();
	if (!rc) return "";
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, rc, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
	nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", lpMsgBuf, -1);
	LocalFree(lpMsgBuf);
	SetLastError(0);
	return nsp_getstr(N, nsp_getobj(N, &N->g, "dl"), "last_error");
#else
	nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", dlerror(), -1);
	return nsp_getstr(N, nsp_getobj(N, &N->g, "dl"), "last_error");
#endif
}

static int lib_close(void *handle)
{
#ifdef WIN32
	return FreeLibrary(handle);
#else
	return dlclose(handle);
#endif
}

NSP_FUNCTION(nl_dl_load)
{
#define __FN__ __FILE__ ":libnsp_dl_load()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj, *tobj;
	NSP_CFUNC cfunc;
#ifdef WIN32
	HINSTANCE l;
#else
	void *l;
#endif
	char namebuf[512];

	nsp_setnull(N, nsp_getobj(N, &N->g, "dl"), "last_error");
	nsp_setbool(N, &N->r, "", 0);

	if (!nsp_isstr(cobj1) || cobj1->val->size < 1) {
		return 0;
}
	tobj = nsp_getobj(N, nsp_getobj(N, &N->g, "dl"), "path");
	if (!nsp_istable(tobj)) {
		nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", "dl.path not found", -1);
		n_error(N, NE_SYNTAX, __FN__, "dl.path not found");
		return 0;
	}
	nsp_setbool(N, &N->r, "", 0);
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (!nsp_isstr(cobj)) continue;
		nc_memset(namebuf, 0, sizeof(namebuf));
		snprintf(namebuf, sizeof(namebuf) - 1, "%s/libnsp_%s.%s", cobj->val->d.str, cobj1->val->d.str, LIBEXT);
		if ((l = lib_open(namebuf)) != NULL) {
			if ((cfunc = (NSP_CFUNC)lib_sym(l, "nsplib_init")) != NULL) {
				cfunc(N);
				nsp_setbool(N, &N->r, "", 1);
				return 0;
			}
			else {
				lib_error(N);
				lib_close(l);
				n_error(N, NE_SYNTAX, __FN__, "%s", nsp_getstr(N, nsp_getobj(N, &N->g, "dl"), "last_error"));
				return 0;
			}
		}
	}
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "dl"), "last_error");
	if (nsp_isnull(cobj)) {
		nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", "failed to open library", -1);
	}
	n_error(N, NE_SYNTAX, __FN__, "%s", nsp_tostr(N, cobj));
	return 0;
#undef __FN__
}

#endif /* HAVE_DL */

/*
 * file functions
 */
NSP_FUNCTION(nl_filechdir)
{
#define __FN__ __FILE__ ":nl_filechdir()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int rc;

	settrace();
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	// if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	rc = chdir(cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	return rc;
#undef __FN__
}

NSP_FUNCTION(nl_fileexists)
{
#define __FN__ __FILE__ ":nl_fileexists()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	struct stat sb;
	int rc;
	char *file;

	settrace();
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	file = cobj1->val->d.str;
#if defined(WIN32) || defined(__TURBOC__)
	rc = stat(file, &sb);
#else
	rc = lstat(file, &sb);
#endif
	if (rc != 0) {
		nsp_setbool(N, &N->r, "", 0);
		return 0;
	}
	nsp_setbool(N, &N->r, "", 1);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_filemkdir)
{
#define __FN__ __FILE__ ":nl_filemkdir()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	mode_t umask = 0755;
#endif
	int rc;

	settrace();
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	if (cobj2->val->type == NT_NUMBER) umask = (mode_t)cobj2->val->d.num;
	rc = mkdir(cobj1->val->d.str, umask & 0777);
#else
	rc = mkdir(cobj1->val->d.str);
#endif
	nsp_setnum(N, &N->r, "", rc);
	return rc;
#undef __FN__
}

NSP_FUNCTION(nl_filereadall)
{
#define __FN__ __FILE__ ":nl_filereadall()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset = 0;

	settrace();
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	if (stat(cobj1->val->d.str, &sb) != 0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd = open(cobj1->val->d.str, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type == NT_NUMBER) {
		offset = (int)cobj2->val->d.num;
		lseek(fd, offset, SEEK_SET);
	}
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	bl = sb.st_size - offset;
	robj->val->d.str = (char *)n_alloc(N, bl + 2, 0);
	robj->val->size = bl;
	p = (char *)robj->val->d.str;
	while (bl > 0) {
		if ((r = read(fd, p, bl)) < 0) break;
		p += r;
		bl -= r;
	}
	close(fd);
	robj->val->d.str[sb.st_size - offset] = '\0';
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_filerename)
{
#define __FN__ __FILE__ ":nl_filerename()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	struct stat sb;

	settrace();
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	//if (cobj2->val->type != NT_STRING || cobj2->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	n_expect_argtype(N, NT_STRING, 2, cobj2, 0);
	if (stat(cobj1->val->d.str, &sb) != 0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	rename(cobj1->val->d.str, cobj2->val->d.str);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_filestat)
{
#define __FN__ __FILE__ ":nl_filestat()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym = 0;
	char *file;

	settrace();
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	file = cobj1->val->d.str;
#if defined(WIN32) || defined(__TURBOC__)
	rc = stat(file, &sb);
	if (rc != 0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
	rc = lstat(file, &sb);
	if (rc != 0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	if (!(~sb.st_mode&S_IFLNK)) {
		sym = 1;
		if (stat(file, &sb) != 0) sym = 2;
	}
#endif
	tobj.val = n_newval(N, NT_TABLE);
	nsp_setnum(N, &tobj, "mtime", (num_t)sb.st_mtime);
	if (sym == 2) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", "broken", 6);
	}
	else if ((sb.st_mode&S_IFDIR)) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", sym ? "dirp" : "dir", sym ? 4 : 3);
	}
	else {
		nsp_setnum(N, &tobj, "size", sb.st_size);
		nsp_setstr(N, &tobj, "type", sym ? "filep" : "file", sym ? 5 : 4);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_fileunlink)
{
#define __FN__ __FILE__ ":nl_fileunlink()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int rc = -1;
	int i;

	settrace();
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	for (i = 1;; i++) {
		cobj1 = nsp_getobj(N, &N->context->l, n_ntoa(N, N->numbuf, i, 10, 0));
		if (!nsp_isstr(cobj1) || cobj1->val->size < 1) break;
		rc = unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nsp_setnum(N, &N->r, "", rc);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_filewriteall)
{
#define __FN__ __FILE__ ":nl_filewriteall()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	int fd = -1;
	int w = 0;
	int offset = 0;

	settrace();
	/* umask(022); */
	//if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	n_expect_argtype(N, NT_STRING, 1, cobj1, 0);
	if (nc_strcmp(fname, "writeall") == 0) {
		fd = open(cobj1->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	}
	else if (nc_strcmp(fname, "append") == 0) {
		fd = open(cobj1->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
	}
	if (fd == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type == NT_STRING) {
		if (cobj3->val->type == NT_NUMBER) {
			offset = (int)cobj3->val->d.num;
			lseek(fd, offset, SEEK_SET);
		}
		w = write(fd, cobj2->val->d.str, cobj2->val->size);
	}
	close(fd);
	nsp_setnum(N, &N->r, "", w);
	return 0;
#undef __FN__
}

/*
 * math functions
 */
NSP_FUNCTION(nl_math)
{
#define __FN__ __FILE__ ":nl_math1()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	num_t n;

	settrace();
	if (nc_strcmp(fname, "rand") == 0) {
		n = rand();
		if (cobj1->val->type == NT_NUMBER && (int)cobj1->val->d.num) {
			n = (int)n % (int)cobj1->val->d.num;
		}
		n = (int)n % 65535;
		nsp_setnum(N, &N->r, "", n);
		return 0;
	}
	//if (cobj1->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number");
	n_expect_argtype(N, NT_NUMBER, 1, cobj1, 0);
	if (nc_strcmp(fname, "abs") == 0) {
		n = fabs(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "ceil") == 0) {
		n = cobj1->val->d.num;
		if ((int)n < n) n = (int)n + 1; else n = (int)n;
	}
	else if (nc_strcmp(fname, "floor") == 0) {
		n = (int)cobj1->val->d.num;
	}
	else if (nc_strcmp(fname, "ceil") == 0) {
		n = ceil(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "floor") == 0) {
		n = floor(cobj1->val->d.num);
	}
	/* add trig functions */
	else if (nc_strcmp(fname, "acos") == 0) {
		n = acos(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "asin") == 0) {
		n = asin(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "atan") == 0) {
		n = atan(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "atan2") == 0) {
		//if (cobj2->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, fname, "expected a number for arg2");
		n_expect_argtype(N, NT_NUMBER, 2, cobj2, 0);
		n = atan2(cobj1->val->d.num, cobj2->val->d.num);
	}
	else if (nc_strcmp(fname, "cos") == 0) {
		n = cos(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "sin") == 0) {
		n = sin(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "tan") == 0) {
		n = tan(cobj1->val->d.num);
	}
	/* add exp and log functions */
	else if (nc_strcmp(fname, "exp") == 0) {
		n = exp(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "log") == 0) {
		if (cobj1->val->d.num <= 0) n_error(N, NE_SYNTAX, fname, "arg must not be zero");
		n = log(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "log10") == 0) {
		if (cobj1->val->d.num <= 0) n_error(N, NE_SYNTAX, fname, "arg must not be zero");
		n = log10(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "pow") == 0) {
		if (cobj2->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, fname, "expected a number for arg2");
		n = pow(cobj1->val->d.num, cobj2->val->d.num);
	}
	/* add hyperbolic functions */
	else if (nc_strcmp(fname, "cosh") == 0) {
		n = cosh(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "sinh") == 0) {
		n = sinh(cobj1->val->d.num);
	}
	else if (nc_strcmp(fname, "tanh") == 0) {
		n = tanh(cobj1->val->d.num);
	}
	/* add other functions */
	else if (nc_strcmp(fname, "sqrt") == 0) {
		n = sqrt(cobj1->val->d.num);
	}
	else {
		n = 0;
	}
	nsp_setnum(N, &N->r, "", n);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_tonumber)
{
#define __FN__ __FILE__ ":nl_tonumber()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	settrace();
	nsp_setnum(N, &N->r, "", nsp_tonum(N, cobj1));
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_tostring)
{
#define __FN__ __FILE__ ":nl_tostring()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	unsigned short d;
	char *p;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type == NT_NUMBER && cobj2->val->type == NT_NUMBER) {
		d = (unsigned short)cobj2->val->d.num;
		if (d > sizeof(N->numbuf) - 2) d = sizeof(N->numbuf) - 2;
		p = n_ntoa(N, N->numbuf, cobj1->val->d.num, 10, d);
	}
	else {
		p = nsp_tostr(N, cobj1);
	}
	nsp_setstr(N, &N->r, "", p, p ? nc_strlen(p) : 0);
	return 0;
#undef __FN__
}

/*
 * string functions
 */
NSP_FUNCTION(nl_atoi)
{
#define __FN__ __FILE__ ":nl_atoi()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	unsigned int offset = 0;
	short i = -1;

	settrace();
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type == NT_NUMBER) offset = (int)cobj2->val->d.num;
	if (offset < cobj1->val->size && cobj1->val->size>0) {
		i = (int)cobj1->val->d.str[offset] & 255;
	}
	nsp_setnum(N, &N->r, "", i);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_itoa)
{
#define __FN__ __FILE__ ":nl_itoa()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;

	settrace();
	if (cobj1->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg1");
	robj = nsp_setstr(N, &N->r, "", " ", 1);
	robj->val->d.str[0] = (int)cobj1->val->d.num & 255;
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strcat)
{
#define __FN__ __FILE__ ":nl_strcat()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *robj;

	settrace();
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	robj->val->size = cobj1->val->size + cobj2->val->size;
	robj->val->d.str = (char *)n_alloc(N, robj->val->size + 1, 0);
	nc_memcpy(robj->val->d.str, cobj1->val->d.str, cobj1->val->size);
	nc_memcpy(robj->val->d.str + cobj1->val->size, cobj2->val->d.str, cobj2->val->size);
	robj->val->d.str[robj->val->size] = 0;
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strcmp)
{
#define __FN__ __FILE__ ":nl_strcmp()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	uchar *s1, *s2;
	int i, rval = 0;

	settrace();
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj1->val->d.str == NULL) {
		if (cobj2->val->d.str == NULL) rval = 0; else rval = (int)cobj2->val->d.str[0];
		nsp_setnum(N, &N->r, "", rval); return 0;
	}
	else if (cobj2->val->d.str == NULL) {
		rval = -(int)cobj1->val->d.str[0];
		nsp_setnum(N, &N->r, "", rval); return 0;
	}
	if (nc_strcmp(fname, "cmp") == 0) {
		rval = nc_strcmp(cobj1->val->d.str, cobj2->val->d.str);
	}
	else if (nc_strcmp(fname, "ncmp") == 0) {
		if (cobj3->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
		rval = nc_strncmp(cobj1->val->d.str, cobj2->val->d.str, (int)cobj3->val->d.num);
	}
	else if (nc_strcmp(fname, "icmp") == 0) {
		s1 = (uchar *)cobj1->val->d.str; s2 = (uchar *)cobj2->val->d.str;
		do {
			if ((rval = (nc_tolower(*s1) - nc_tolower(*s2))) != 0) break;
			s1++; s2++;
		} while (*s1 != 0);
	}
	else if (nc_strcmp(fname, "nicmp") == 0) {
		if (cobj3->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
		s1 = (uchar *)cobj1->val->d.str; s2 = (uchar *)cobj2->val->d.str; i = (int)cobj3->val->d.num;
		do {
			if ((rval = (nc_tolower(*s1) - nc_tolower(*s2))) != 0) break;
			if (--i < 1) break;
			s1++; s2++;
		} while (*s1 != 0);
	}
	nsp_setnum(N, &N->r, "", rval);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strcontains)
{
#define __FN__ __FILE__ ":nl_strcontains()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	unsigned int i = 0, j = 0;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	n_expect_argtype(N, NT_STRING, 1, cobj1, 1);
	n_expect_argtype(N, NT_STRING, 2, cobj2, 1);
	if (cobj1->val->size < cobj2->val->size) {
		nsp_setbool(N, &N->r, "", 0);
		return 0;
	}
	else if (cobj1->val->size == 0 && cobj2->val->size == 0) {
		nsp_setbool(N, &N->r, "", 1);
		return 0;
	}
	if (nc_strcmp(fname, "contains") == 0) {
		for (i = 0, j = 0; i < cobj1->val->size; i++, j++) {
			if (j == cobj2->val->size) break;
			if (cobj2->val->d.str[j] == '\0') { j = 0; break; }
			if (cobj2->val->d.str[j] != cobj1->val->d.str[i]) j = -1;
		}
	}
	else if (nc_strcmp(fname, "endswith") == 0) {
		for (i = cobj1->val->size - cobj2->val->size, j = 0; i < cobj1->val->size; i++, j++) {
			if (j == cobj2->val->size) break;
			if (cobj2->val->d.str[j] == '\0') { j = 0; break; }
			if (cobj2->val->d.str[j] != cobj1->val->d.str[i]) j = -1;
		}
	}
	else if (nc_strcmp(fname, "startswith") == 0) {
		for (i = 0, j = 0; i < cobj1->val->size && i < cobj2->val->size; i++, j++) {
			if (j == cobj2->val->size) break;
			if (cobj2->val->d.str[j] == '\0') { j = 0; break; }
			if (cobj2->val->d.str[j] != cobj1->val->d.str[i]) j = -1;
		}
	}
	nsp_setbool(N, &N->r, "", (i <= cobj1->val->size&&j == cobj2->val->size) ? 1 : 0);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strjoin)
{
#define __FN__ __FILE__ ":nl_strjoin()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj, *robj;
	char *p = NULL, *p2;
	int len1 = 0, len2 = 0;
	int cnt = 0;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type == NT_STRING && cobj2->val->size > 0) { p = cobj2->val->d.str; len2 = cobj2->val->size; }
	for (cobj = cobj1->val->d.table.f; cobj; cobj = cobj->next) {
		if (cobj->val->type == NT_STRING) {
			len1 += cobj->val->size;
			cnt++;
		}
		else if (cobj->val->type == NT_NUMBER) {
			len1++;
			cnt++;
		}
	}
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if (cnt == 0) return 0;
	if (cnt > 1) {
		robj->val->size = len1 + ((cnt - 1)*len2);
		robj->val->d.str = (char *)n_alloc(N, robj->val->size + 1, 0);
		p2 = robj->val->d.str;
		for (cobj = cobj1->val->d.table.f; cobj; cobj = cobj->next) {
			if (cobj->val->type == NT_STRING) {
				nc_memcpy(p2, cobj->val->d.str, cobj->val->size);
				p2 += cobj->val->size;
				if (--cnt&&p) {
					nc_memcpy(p2, p, len2);
					p2 += len2;
				}
			}
			else if (cobj->val->type == NT_NUMBER) {
				p2[0] = (uchar)cobj->val->d.num & 255;
				p2++;
				if (--cnt&&p) {
					nc_memcpy(p2, p, len2);
					p2 += len2;
				}
			}
		}
		robj->val->d.str[robj->val->size] = 0;
	}
	else {
		robj->val->size = len1;
		robj->val->d.str = (char *)n_alloc(N, robj->val->size + 1, 0);
		for (cobj = cobj1->val->d.table.f; cobj; cobj = cobj->next) {
			if (cobj->val->type == NT_STRING) {
				nc_memcpy(robj->val->d.str, cobj->val->d.str, cobj->val->size);
				break;
			}
		}
		robj->val->d.str[robj->val->size] = 0;
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strlen)
{
#define __FN__ __FILE__ ":nl_strlen()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nsp_setnum(N, &N->r, "", cobj1->val->size);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strrep)
{
#define __FN__ __FILE__ ":nl_strrep()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	char *ss, *se;
	char *s2;
	int l2;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj3 = cobj2;
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str == NULL) {
		return 0;
	}
	if (cobj2->val->d.str == NULL) {
		nsp_strcat(N, &N->r, cobj1->val->d.str, cobj1->val->size);
		return 0;
	}
	se = ss = cobj1->val->d.str;
	s2 = cobj2->val->d.str;
	l2 = cobj2->val->size;
	for (; *se; se++) {
		if (nc_strncmp(se, s2, l2) != 0) continue;
		nsp_strcat(N, &N->r, ss, se - ss);
		nsp_strcat(N, &N->r, cobj3->val->d.str, cobj3->val->size);
		ss = se += l2;
		if (*se) { --se; continue; }
		nsp_strcat(N, &N->r, ss, se - ss);
		break;
	}
	if (se > ss) {
		nsp_strcat(N, &N->r, ss, se - ss);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strsplit)
{
#define __FN__ __FILE__ ":nl_strsplit()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t tobj;
	char *ss, *se;
	int i = 0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN + 1];

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr |= NST_AUTOSORT;
	if (cobj1->val->d.str == NULL) {
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	if (cobj2->val->d.str == NULL) {
		unsigned int j = 0;

		for (j = 0, ss = cobj1->val->d.str; j < cobj1->val->size; j++, ss++) {
			nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, 1);
		}
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	se = ss = cobj1->val->d.str;
	s2 = cobj2->val->d.str;
	l2 = cobj2->val->size;
	for (; *se; se++) {
		if (nc_strncmp(se, s2, l2) != 0) continue;
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
		ss = se += l2;
		if (*se) { --se; continue; }
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
		break;
	}
	if (se > ss) {
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strstr)
{
#define __FN__ __FILE__ ":nl_strstr()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	unsigned int i = 0, j = 0;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj2->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "zero length arg2");
	if (nc_strcmp(fname, "str") == 0) {
		for (i = 0, j = 0; i < cobj1->val->size; i++, j++) {
			if (j == cobj2->val->size) break;
			if (cobj2->val->d.str[j] == '\0') { j = 0; break; }
			if (cobj2->val->d.str[j] != cobj1->val->d.str[i]) j = -1;
		}
	}
	else if (nc_strcmp(fname, "istr") == 0) {
		for (i = 0, j = 0; i < cobj1->val->size; i++, j++) {
			if (j == cobj2->val->size) break;
			if (cobj2->val->d.str[j] == '\0') { j = 0; break; }
			if (nc_tolower(cobj2->val->d.str[j]) != nc_tolower(cobj1->val->d.str[i])) j = -1;
		}
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "................");
	}
	if (i <= cobj1->val->size&&j == cobj2->val->size) {
		nsp_setstr(N, &N->r, "", cobj1->val->d.str + i - j, cobj1->val->size - i + j);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strsub)
{
#define __FN__ __FILE__ ":nl_strsub()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	unsigned int offset, max = 0;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj3 = cobj2;
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	if (cobj2->val->d.num < 0) {
		offset = cobj1->val->size - abs((int)cobj2->val->d.num);
	}
	else {
		offset = (int)cobj2->val->d.num;
	}
	if (cobj3->val->type == NT_NUMBER) max = (int)cobj3->val->d.num; else max = cobj1->val->size;
	if (offset > cobj1->val->size) offset = cobj1->val->size;
	if (max > cobj1->val->size - offset) max = cobj1->val->size - offset;
	nsp_setstr(N, &N->r, "", cobj1->val->d.str + offset, max);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strtolower)
{
#define __FN__ __FILE__ ":nl_strtolower()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;
	char *fname = nsp_getstr(N, &N->context->l, "0");
	int i;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj1 = thisobj;
	}
	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str != NULL) {
		robj->val->size = cobj1->val->size;
		robj->val->d.str = (char *)n_alloc(N, robj->val->size + 1, 0);
		robj->val->d.str[0] = 0;
		i = robj->val->size - 1;
		if (nc_strcmp(fname, "tolower") == 0) {
			for (; i > -1; i--) robj->val->d.str[i] = nc_tolower(cobj1->val->d.str[i]);
		}
		else if (nc_strcmp(fname, "toupper") == 0) {
			for (; i > -1; i--) robj->val->d.str[i] = nc_toupper(cobj1->val->d.str[i]);
		}
		robj->val->d.str[robj->val->size] = 0;
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strtrim)
{
#define __FN__ __FILE__ ":nl_strtrim()"
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	//	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	char *p;
	int plen;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj1 = thisobj;
	}
	n_expect_argtype(N, NT_STRING, 1, cobj1, 1);
	if (cobj1->val->size < 1) {
		nsp_setstr(N, &N->r, "", NULL, 0);
		return 0;
	}
	p = cobj1->val->d.str;
	plen = cobj1->val->size;
	if (nc_strcmp(fname, "trim") == 0 || nc_strcmp(fname, "trimstart") == 0) {
		while (nc_isspace(p[0])) { p++; plen--; }
	}
	if (nc_strcmp(fname, "trim") == 0 || nc_strcmp(fname, "trimend") == 0) {
		while (plen && nc_isspace(p[plen - 1])) { plen--; }
	}
	nsp_setstr(N, &N->r, "", p, plen);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_asctime)
{
#define __FN__ __FILE__ ":nl_asctime()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	//char *fname = nsp_getstr(N, &N->l, "0");
	struct timeval ttime;
	time_t t;
	char timebuf[32];

	settrace();
	if (nsp_istable(cobj1)) {
		int yy = (int)nsp_getnum(N, cobj1, "tm_year");
		int mm = (int)nsp_getnum(N, cobj1, "tm_mon");
		int dd = (int)nsp_getnum(N, cobj1, "tm_mday");
		int h = (int)nsp_getnum(N, cobj1, "tm_hour");
		int m = (int)nsp_getnum(N, cobj1, "tm_min");
		int s = (int)nsp_getnum(N, cobj1, "tm_sec");

		//if (yy) mm++;
		snprintf(timebuf, sizeof(timebuf), "%04d-%02d-%02d %02d:%02d:%02d", yy, mm, dd, h, m, s);
		nsp_setstr(N, &N->r, "", timebuf, -1);
		return 0;
	}
	if (cobj1->val->type == NT_NUMBER) {
		ttime.tv_sec = (long)cobj1->val->d.num;
	}
	else {
		nc_gettimeofday(&ttime, NULL);
	}
	t = ttime.tv_sec;
	//	if (nc_strcmp(fname, "sqldate") == 0) {
	//		strftime(timebuf, sizeof(timebuf) - 1, "%Y-%m-%d", gmtime(&t));
	//	}
	//	else if (nc_strcmp(fname, "sqltime") == 0) {
	//		strftime(timebuf, sizeof(timebuf) - 1, "%H:%M:%S", gmtime(&t));
	//	}
	//	else {
			//strftime(timebuf, sizeof(timebuf) - 1, "%Y-%m-%d %H:%M:%S", localtime((time_t *)&ttime.tv_sec)); <- this crashes in VS now because tv_sec is expected to be LONG
	strftime(timebuf, sizeof(timebuf) - 1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	//	}
	nsp_setstr(N, &N->r, "", timebuf, -1);
	return 0;
#undef __FN__
}

static int getunixtime(int y, int m, int d, int hh, int mm, int ss)
{
	int dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int unixdate = 0;
	int i;

	m -= 1;
	d -= 1;

	for (i = 1970; i < y; i++) {
		unixdate += 365;
		if ((i / 4.0f) == (int)(i / 4)) {
			if ((i / 400.0f) == (int)(i / 400)) {
				unixdate++;
			}
			else if ((i / 100.0f) != (int)(i / 100)) {
				unixdate++;
			}
		}
	}
	for (i = 0; i < m; i++) {
		unixdate += dim[i];
		if (i != 1) continue;
		if ((y / 4.0f) == (int)(y / 4)) {
			if ((y / 400.0f) == (int)(y / 400)) {
				unixdate++;
			}
			else if ((y / 100.0f) != (int)(y / 100)) {
				unixdate++;
			}
		}
	}
	unixdate += d;
	unixdate *= 86400;
	if (unixdate < 0) unixdate = 0;
	unixdate += hh * 3600;
	unixdate += mm * 60;
	unixdate += ss;
	return unixdate;
}

static int getunixfromstring(char *datestr)
{
	char *pdate;
	int unixdate = 0;
	int y = 0, m = 0, d = 0, hh, mm, ss;

	pdate = datestr;
	if (atoi(pdate) >= 1970) {
		y = atoi(pdate);
		while ((*pdate) && (*pdate != '-')) pdate++;
		while ((*pdate) && (!nc_isdigit(*pdate))) pdate++;
		m = atoi(pdate);// - 1;
		while ((*pdate) && (*pdate != '-')) pdate++;
		while ((*pdate) && (!nc_isdigit(*pdate))) pdate++;
		d = atoi(pdate);// - 1;
		while ((*pdate) && (*pdate != ' ')) pdate++;
	}
	if (unixdate < 0) unixdate = 0;
	while ((*pdate) && (!nc_isdigit(*pdate))) pdate++;
	if (*pdate == '0') pdate++;
	hh = atoi(pdate);// * 3600;
	while ((*pdate) && (*pdate != ':')) pdate++;
	while ((*pdate) && (!nc_isdigit(*pdate))) pdate++;
	mm = atoi(pdate);// * 60;
	while ((*pdate) && (*pdate != ':')) pdate++;
	while ((*pdate) && (!nc_isdigit(*pdate))) pdate++;
	ss = atoi(pdate);
	unixdate = getunixtime(y, m, d, hh, mm, ss);
	return unixdate;
}

NSP_FUNCTION(nl_sqltounix)
{
#define __FN__ __FILE__ ":nl_sqltounix()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int unixdate = 0;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	unixdate = getunixfromstring(cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", (num_t)unixdate);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_mktime)
{
#define __FN__ __FILE__ ":nl_mktime()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	settrace();
	if (nsp_istable(cobj1)) {
		int yy = (int)nsp_getnum(N, cobj1, "tm_year");
		int mm = (int)nsp_getnum(N, cobj1, "tm_mon");
		int dd = (int)nsp_getnum(N, cobj1, "tm_mday");
		int h = (int)nsp_getnum(N, cobj1, "tm_hour");
		int m = (int)nsp_getnum(N, cobj1, "tm_min");
		int s = (int)nsp_getnum(N, cobj1, "tm_sec");
		nsp_setnum(N, &N->r, "", getunixtime(yy, mm, dd, h, m, s));
	}
	else {
		if (!nsp_isstr(cobj1)) n_error(N, NE_SYNTAX, __FN__, "expected a table or string for arg1");
		if (cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "zero length arg1");
		nsp_setnum(N, &N->r, "", getunixfromstring(cobj1->val->d.str));
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_time)
{
#define __FN__ __FILE__ ":nl_time()"
	settrace();
	nsp_setnum(N, &N->r, "", (num_t)time(NULL));
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_gmtime)
{
#define __FN__ __FILE__ ":nl_gmtime()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	obj_t tobj;
	time_t t;
	struct tm *tp;

	settrace();
	if (cobj1->val->type == NT_NUMBER) {
		t = (time_t)cobj1->val->d.num;
	}
	else if (cobj1->val->type == NT_STRING) {
		t = (time_t)getunixfromstring(cobj1->val->d.str);
	}
	else {
		t = time(NULL);
	}
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	if (nc_strcmp(fname, "localtime") == 0) {
		tp = localtime(&t);
	}
	else {
		tp = gmtime(&t);
	}
	if (tp->tm_year < 1900) tp->tm_year += 1900;
	nsp_setnum(N, &tobj, "tm_sec", tp->tm_sec);
	nsp_setnum(N, &tobj, "tm_min", tp->tm_min);
	nsp_setnum(N, &tobj, "tm_hour", tp->tm_hour);
	nsp_setnum(N, &tobj, "tm_mday", tp->tm_mday);
	nsp_setnum(N, &tobj, "tm_mon", tp->tm_mon + 1);
	nsp_setnum(N, &tobj, "tm_year", tp->tm_year);
	nsp_setnum(N, &tobj, "tm_wday", tp->tm_wday);
	nsp_setnum(N, &tobj, "tm_yday", tp->tm_yday);
	nsp_setnum(N, &tobj, "tm_isdst", tp->tm_isdst);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_gettimeofday)
{
#define __FN__ __FILE__ ":nl_gettimeofday()"
	//obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t tobj;
	//time_t t;
	struct timeval ttime;

	settrace();
	//	if (cobj1->val->type==NT_NUMBER) {
	//		t=(time_t)cobj1->val->d.num;
	//	} else {
	//		t=time(NULL);
	//	}
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	nc_gettimeofday(&ttime, NULL);
	nsp_setnum(N, &tobj, "tv_sec", ttime.tv_sec);
	nsp_setnum(N, &tobj, "tv_usec", ttime.tv_usec);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_runtime)
{
#define __FN__ __FILE__ ":nl_runtime()"
	struct timeval ttime;
	num_t totaltime;

	settrace();
	nc_gettimeofday(&ttime, NULL);
	totaltime = ttime.tv_sec - N->ttime.tv_sec + (num_t)(ttime.tv_usec - N->ttime.tv_usec) / 1000000;
	nsp_setnum(N, &N->r, "", totaltime);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_sleep)
{
#define __FN__ __FILE__ ":nl_sleep()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	double n = 1;

	settrace();
	if (cobj1->val->type == NT_NUMBER) n = cobj1->val->d.num;
#if defined(__BORLANDC__)
	sleep(n);
#elif defined(WIN32)
	Sleep((DWORD)(n * 1000.0));
#else
	usleep(n * 1000000.0);
#endif
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_eval)
{
#define __FN__ __FILE__ ":nl_eval()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	uchar *p;
	int psize;

	settrace();
	if (nc_strcmp(fname, "eval") != 0) {
		n_error(N, NE_SYNTAX, __FN__, "cowardly refusing to run aliased eval");
	}
	if ((cobj1->val->type == NT_STRING) && (cobj1->val->d.str != NULL)) {
		uchar *oldbptr = n_context_blockptr;
		uchar *oldbend = n_context_blockend;
		uchar *oldrptr = n_context_readptr;
		jmp_buf *savjmp;

		n_decompose(N, NULL, (uchar *)cobj1->val->d.str, &p, &psize);
		if (p) n_context_blockptr = p;
		n_context_blockend = n_context_blockptr + readi4((n_context_blockptr + 8));
		n_context_readptr = n_context_blockptr + readi4((n_context_blockptr + 12));

		savjmp = n_context_savjmp;
		n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*n_context_savjmp) == 0) {
			nsp_linkval(N, &N->r, nsp_eval(N, (char *)n_context_readptr));
			//nsp_linkval(N, &N->r, nsp_exec(N, (char *)N->readptr));
		}
		n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
		n_context_savjmp = savjmp;

		if (p) n_free(N, (void *)&p, psize);

		n_context_blockptr = oldbptr;
		n_context_blockend = oldbend;
		n_context_readptr = oldrptr;
	}
	else {
		nsp_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_exec)
{
#define __FN__ __FILE__ ":nl_exec()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	uchar *p;
	int psize;

	settrace();
	if (nc_strcmp(fname, "exec") != 0) {
		n_error(N, NE_SYNTAX, __FN__, "cowardly refusing to run aliased exec");
	}
	if ((cobj1->val->type == NT_STRING) && (cobj1->val->d.str != NULL)) {
		uchar *oldbptr = n_context_blockptr;
		uchar *oldbend = n_context_blockend;
		uchar *oldrptr = n_context_readptr;
		jmp_buf *savjmp;

		n_decompose(N, NULL, (uchar *)cobj1->val->d.str, &p, &psize);
		if (p) n_context_blockptr = p;
		n_context_blockend = n_context_blockptr + readi4((n_context_blockptr + 8));
		n_context_readptr = n_context_blockptr + readi4((n_context_blockptr + 12));

		savjmp = n_context_savjmp;
		n_context_savjmp = (jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*n_context_savjmp) == 0) {
			nsp_linkval(N, &N->r, nsp_exec(N, (char *)n_context_readptr));
		}
		n_free(N, (void *)&n_context_savjmp, sizeof(jmp_buf));
		n_context_savjmp = savjmp;

		if (p) n_free(N, (void *)&p, psize);

		n_context_blockptr = oldbptr;
		n_context_blockend = oldbend;
		n_context_readptr = oldrptr;
	}
	else {
		nsp_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_iname)
{
#define __FN__ __FILE__ ":nl_iname()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj;

	settrace();
	if (cobj1->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type != NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	cobj = nsp_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nsp_isnull(cobj)) {
		nsp_setnum(N, &N->r, "", 0);
	}
	else {
		nsp_setstr(N, &N->r, "", cobj->name, -1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_ival)
{
#define __FN__ __FILE__ ":nl_ival()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj;

	settrace();
	if (!nsp_istable(cobj1)) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (!nsp_isnum(cobj2)) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	cobj = nsp_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (!nsp_isnull(cobj)) nsp_linkval(N, &N->r, cobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_include)
{
#define __FN__ __FILE__ ":nl_include()"
	//	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	//	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	//	uchar *p;
	int n = 0;
	//n_execfunction hijacks the actual execution of this function - fix later

	//settrace();
	//if (nsp_isstr(cobj2)) {
	//	nsp_exec(N, (char *)cobj2->val->d.str);
	//}
	//else if (nsp_isstr(cobj1)) {
	//	p = N->readptr;
	//	n = nsp_execfile(N, (char *)cobj1->val->d.str);
	//	N->readptr = p;
	//}
	//nsp_setbool(N, &N->r, "", n ? 0 : 1);
	//if (n < 0) n_error(N, NE_SYNTAX, __FN__, "failed to include '%s'", cobj1->val->d.str);
	return n;
#undef __FN__
}

NSP_FUNCTION(nl_serialize)
{
#define __FN__ __FILE__ ":nl_serialize()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	settrace();
	if (!nsp_isnull(cobj1)) {
		n_dumpvars(N, cobj1, 0);
	}
	else {
		nsp_setstr(N, &N->r, "", "N U L L", -1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_sizeof)
{
#define __FN__ __FILE__ ":nl_sizeof()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int size = 0;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj1 = thisobj;
	}
	if (nsp_isnull(cobj1)) {
		nsp_setnum(N, &N->r, "", 0);
		return 0;
	}
	switch (cobj1->val->type) {
	case NT_BOOLEAN:
	case NT_NUMBER: size = 1; break;
	case NT_STRING:
	case NT_NFUNC: size = cobj1->val->size; break;
	case NT_CFUNC: size = 1; break;
	case NT_TABLE:
		for (cobj1 = cobj1->val->d.table.f; cobj1; cobj1 = cobj1->next) {
			if (cobj1->val && cobj1->val->attr&NST_SYSTEM) { size--; continue; }
			if (!nsp_isnull(cobj1)) size++;
		}
		break;
	case NT_CDATA: size = cobj1->val->size; break;
	}
	nsp_setnum(N, &N->r, "", size);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_typeof)
{
#define __FN__ __FILE__ ":nl_typeof()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	NSP_CDATA *chead;
	char *p;

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj1 = thisobj;
	}
	switch (cobj1->val->type) {
	case NT_BOOLEAN: p = "boolean";  break;
	case NT_NUMBER: p = "number";   break;
	case NT_STRING: p = "string";   break;
	case NT_TABLE: p = "table";    break;
	case NT_NFUNC:
	case NT_CFUNC: p = "function"; break;
	case NT_CDATA:
		if ((chead = (NSP_CDATA *)cobj1->val->d.cdata) != NULL) {
			p = chead->obj_type; break;
		}
	default: p = "null";     break;
	}
	nsp_setstr(N, &N->r, "", p, -1);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_system)
{
#define __FN__ __FILE__ ":nl_system()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int n = -1;

	settrace();
	if (cobj1->val->type == NT_STRING && cobj1->val->d.str != NULL) {
		nl_flush(N);
		n = system(cobj1->val->d.str);
#ifdef WEXITSTATUS
		n = WEXITSTATUS(n);
#endif
	}
	nsp_setnum(N, &N->r, "", n);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_copy)
{
#define __FN__ __FILE__ ":nl_copy()"
	settrace();
	n_copyval(N, &N->r, nsp_getobj(N, &N->context->l, "1"));
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_printf)
{
#define __FN__ __FILE__ ":nl_printf()"
	char *fname = nsp_getstr(N, &N->context->l, "0");
	obj_t *fobj = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj, *robj;
	char tmp[80];
	char *ss, *s, *p;
	char esc = 0;

	settrace();
	if (!nsp_isstr(fobj) || fobj->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	cobj = fobj->next;
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	for (s = ss = (char *)fobj->val->d.str; *s != '\0'; s++) {
		if (!esc) {
			if (*s == '%') {
				esc = 1;
				nsp_strcat(N, &N->r, ss, s - ss);
				ss = s;
			}
			continue;
		}
		/* flags */
		if (*s == '-') s++;
		/* width */
		if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		/* precision */
		if (*s == '.') {
			s++;
			if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		}
		switch (*s) {
			// case 'c': tmp[0]=(char)va_arg(ap, int); tmp[1]=0; goto end;
		case 'd': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), -10, 0); break;
		case 'i': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), -10, 0); break;
		case 'o': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), 8, 0); break;
		case 'u': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), 10, 0); break;
		case 'x': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), 16, 0); break;
		case 'f': p = n_ntoa(N, tmp, nsp_tonum(N, cobj), -10, 6); break;
		case 's': p = nsp_tostr(N, cobj); break;
		case '%': p = "%"; break;
		default: p = "orphan %"; break;
		}
		if (*s != '%')
			if (cobj) cobj = cobj->next;
		nsp_strcat(N, robj, p, -1);
		ss = s + 1;
		esc = 0;
	}
	nsp_strcat(N, &N->r, ss, s - ss);
	if (nc_strcmp(fname, "printf") == 0) {
		writestr(N, robj);
		nsp_setnum(N, robj, "", robj->val->size);

	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_append)
{
#define __FN__ __FILE__ ":nl_append()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *nobj;
	//int size = 0;

	// this function is bad, but working well enough for testing ns code
	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	if (!nsp_istable(cobj1)) {
		n_error(N, NE_SYNTAX, __FN__, "can't append to a non-table");
	}
	if (!nsp_isstr(cobj2)) {
		n_error(N, NE_SYNTAX, __FN__, "can't append non-string");
	}
	nobj = nsp_appendobj(N, cobj1, "0");
	if (nobj->prev != NULL) {
		n_setnamei(N, nobj, atoi(nobj->prev->name) + 1);
	}
	nsp_setstr(N, nobj, "", cobj2->val->d.str, cobj2->val->size);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_zlink)
{
#define __FN__ __FILE__ ":nl_zlink()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *basetype = nsp_getobj(N, &N->context->l, "basetype");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");

	settrace();
	if (!nsp_isnull(basetype)) {
		cobj2 = cobj1;
		cobj1 = thisobj;
	}
	nsp_zlink(N, cobj1, cobj2);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(nl_base_method)
{
#define __FN__ __FILE__ ":nl_base_method()"
	char *fname = nsp_getstr(N, &N->context->l, "0");
	unsigned short ftype = nsp_typeof(nsp_getobj(N, &N->context->l, "this"));

	settrace();
	nsp_setnum(N, &N->context->l, "basetype", ftype);
	if (nc_strcmp(fname, "gettype") == 0) return nl_typeof(N);
	if (ftype == NT_STRING) {
		obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "string"), fname);
		if (nsp_typeof(cobj) == NT_CFUNC) {
			return cobj->val->d.cfunc(N);
		}
		else if (nsp_typeof(cobj) == NT_NFUNC) {
			uchar *p = n_context_readptr;
			nsp_exec(N, (char *)cobj->val->d.str);
			n_context_readptr = p;
			return 0;
		}
		else if (nc_strcmp(fname, "len") == 0 || nc_strcmp(fname, "length") == 0) {
			return nl_sizeof(N);
		}
		else {
			n_warn(N, __FN__, "string method '%s' missing?", fname);
		}
	}
	else if (ftype == NT_TABLE) {
		if (nc_strcmp(fname, "join") == 0) {
			return nl_strjoin(N);
		}
		else if (nc_strcmp(fname, "inherit") == 0) {
			return nl_zlink(N);
		}
		else if (nc_strcmp(fname, "append") == 0) {
			return nl_append(N);
		}
	}
	if (nc_strcmp(fname, "tostr") == 0 || nc_strcmp(fname, "tostring") == 0) {
		return nl_tostring(N);
	}
	else if (nc_strcmp(fname, "len") == 0 || nc_strcmp(fname, "length") == 0) {
		return nl_sizeof(N);
	}
	n_error(N, NE_SYNTAX, __FN__, "object has no method named '%s'.", fname);
	return 0;
#undef __FN__
}
