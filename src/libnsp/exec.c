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
#ifndef _NSPLIB_H
#include "nsp/nsplib.h"
#endif
#include "opcodes.h"

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

obj_t *n_execfunction(nsp_state *N, obj_t *fobj, obj_t *pobj, uchar isclass)
{
#define __FN__ __FILE__ ":n_execfunction()"
	uchar *oldbptr;
	uchar *oldrptr;
	uchar *oldbend;
	char *oldfunc;
	jmp_buf *savjmp;
	obj_t listobj, *cobj;
	val_t *olobj;
	unsigned short ftype, i;
	int e;

	DEBUG_IN();
	settrace();
	ftype=nsp_typeof(fobj);
	if (ftype!=NT_CFUNC&&ftype!=NT_NFUNC) {
		n_error(N, NE_SYNTAX, __FN__, "'%s' is not a function", fobj->name);
	}
	oldfunc=N->func;
	N->func=fobj->name;
	n_expect(N, __FN__, OP_POPAREN);
	listobj.val=n_newval(N, NT_TABLE);
	/* set this */
	cobj=listobj.val->d.table.f=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
	cobj->prev=NULL;
	cobj->next=NULL;
	cobj->val=NULL;
	n_setname(N, cobj, "this");
	if (isclass) {
		nsp_linkval(N, cobj, &listobj);
	} else if (pobj) {
		nsp_linkval(N, cobj, pobj);
	}
	if (cobj->val) cobj->val->attr|=NST_HIDDEN;
	/* set fn name */
	cobj->next=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
	cobj->next->prev=cobj;
	cobj->next->next=NULL;
	cobj=cobj->next;
	n_setname(N, cobj, "0");
	cobj->val=NULL;
	nsp_setstr(N, cobj, "0", fobj->name, -1);
	/* set args */
	for (i=1;;i++) {
		N->readptr++;
		if (*N->readptr==OP_PCPAREN) break;
		cobj->next=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj->next->prev=cobj;
		cobj->next->next=NULL;
		cobj=cobj->next;
		n_setnamei(N, cobj, i);
		cobj->val=NULL;

		if (*N->readptr==OP_MAND) {
			N->readptr++;
			n_expect(N, __FN__, OP_LABEL);
			nsp_linkval(N, cobj, nsp_getobj(N, NULL, n_getlabel(N, NULL)));
		} else {
			nsp_linkval(N, cobj, nsp_eval(N, (char *)N->readptr));
		}
		if (*N->readptr==OP_PCOMMA) continue;
		n_expect(N, __FN__, OP_PCPAREN);
		break;
	}
	listobj.val->d.table.l=cobj;
	N->readptr++;
	olobj=N->l.val; N->l.val=listobj.val; listobj.val=NULL;
	/* if (N->debug) n_warn(N, __FN__, "%s()", fobj->name); */
	oldbptr=N->blockptr;
	oldrptr=N->readptr;
	oldbend=N->blockend;
	nsp_unlinkval(N, &N->r);
	if (ftype==NT_CFUNC) {
		savjmp=N->savjmp;
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if ((e=setjmp(*N->savjmp))==0) {
			fobj->val->d.cfunc(N);
		}
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
		N->savjmp=savjmp;
	} else {
		N->blockptr=(uchar *)fobj->val->d.str;
		N->readptr=(uchar *)fobj->val->d.str;
		N->blockend=(uchar *)fobj->val->d.str+fobj->val->size;
		n_expect(N, __FN__, OP_POPAREN);
		for (i=1;;i++) {
			N->readptr++;
			cobj=nsp_getobj(N, &N->l, n_ntoa(N, N->numbuf, i, 10, 0));
			if (*N->readptr==OP_LABEL) n_setname(N, cobj, n_getlabel(N, NULL));
			if (*N->readptr==OP_PCOMMA) continue;
			n_expect(N, __FN__, OP_PCPAREN);
			N->readptr++;
			break;
		}
		n_expect(N, __FN__, OP_POBRACE);
		savjmp=N->savjmp;
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if ((e=setjmp(*N->savjmp))==0) {
			nsp_exec(N, (char *)N->readptr);
		}
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
		N->savjmp=savjmp;
	}
	if (isclass) {
		/* forced decrementing is an ugly hack, but an effective fix */
/*
		obj_t *cobj=nsp_getobj(N, &N->l, "this");
		n_warn(N, __FN__, "2]%d", cobj->val->refs);
		cobj->val->refs--;
		cobj->val=NULL;
		n_warn(N, __FN__, "2]%d", N->l.val->refs);
*/
		N->l.val->refs--;
		nsp_linkval(N, &N->r, &N->l);
	}
	nsp_unlinkval(N, &N->l);
	N->l.val=olobj;
	N->blockptr=oldbptr;
	N->readptr=oldrptr;
	N->blockend=oldbend;
	if (N->ret) N->ret=0;
	N->func=oldfunc;
	if (e && N->savjmp!=NULL) longjmp(*N->savjmp, 1);
	DEBUG_OUT();
	return &N->r;
#undef __FN__
}

/*
 * the following functions are public API functions
 */

obj_t *nsp_exec(nsp_state *N, const char *string)
{
#define __FN__ __FILE__ ":nsp_exec()"
	char namebuf[MAX_OBJNAMELEN+1];
	obj_t *cobj, *tobj;
	uchar block, ctype, op;
	uchar jmp=N->savjmp?1:0, single=(uchar)N->single;
	uchar *p;
	int psize;

	DEBUG_IN();
	settrace();
	N->single=0;
	if (jmp==0) {
		nsp_unlinkval(N, &N->r);
		if (string==NULL||string[0]==0) goto end;
		n_decompose(N, (uchar *)string, &p, &psize);
		if (p) N->blockptr=p;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
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
		if (N->signal) {
			n_error(N, NE_INTERNAL, __FN__, "killed by external request");
		}
		if (block&&(N->brk>0||N->cnt>0||*N->readptr==OP_PCBRACE)) goto end;
		if (*N->readptr==OP_PCBRACE) {
			N->readptr++;
			goto endstmt;
		} else if (OP_ISMATH(*N->readptr)) {
			n_warn(N, __FN__, "unexpected math op '%s'", n_getsym(N, *N->readptr));
			N->readptr++;
		} else if (OP_ISPUNC(*N->readptr)) {
			n_warn(N, __FN__, "unexpected punctuation '%s'", n_getsym(N, *N->readptr));
			N->readptr++;
		} else if (OP_ISKEY(*N->readptr)) {
			op=*N->readptr++;
			switch (op) {
			case OP_KLOCAL:
			case OP_KVAR:     n_readvar(N, &N->l, NULL); goto endstmt;
			case OP_KGLOB:    n_readvar(N, &N->g, NULL); goto endstmt;
			case OP_KIF:      n_if(N);      if (N->ret) goto end; else goto endstmt;
			case OP_KFOR:     n_for(N);     if (N->ret) goto end; else goto endstmt;
			case OP_KFOREACH: n_foreach(N); if (N->ret) goto end; else goto endstmt;
			case OP_KSWITCH:  n_switch(N);  if (N->ret) goto end; else goto endstmt;
			case OP_KDO:      n_do(N);      if (N->ret) goto end; else goto endstmt;
			case OP_KWHILE:   n_while(N);   if (N->ret) goto end; else goto endstmt;
			case OP_KTRY:     n_try(N);     if (N->ret) goto end; else goto endstmt;
			case OP_KCLASS:
			case OP_KFUNC:
				n_expect(N, __FN__, OP_LABEL);
				cobj=nsp_setnfunc(N, &N->g, n_getlabel(N, NULL), NULL, 0);
				n_getfunction(N, cobj);
				goto endstmt;
			case OP_KBREAK:
				if ((!block)&&(!single)) n_error(N, NE_SYNTAX, __FN__, "break without block");
				N->brk=(short)(*N->readptr==OP_NUMDATA?n_getnumber(N):1);
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				goto end;
			case OP_KCONT:
				if ((!block)&&(!single)) n_error(N, NE_SYNTAX, __FN__, "continue without block");
				N->cnt=1;
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				goto end;
			case OP_KRET:
				n_storeval(N, &N->r);
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				N->ret=1;
				goto end;
			case OP_KTHROW:
				n_storeval(N, &N->r);
				n_error(N, NE_EXCEPTION, NULL, "%s",  nsp_tostr(N, &N->r));
				if (*N->readptr==OP_PSEMICOL) N->readptr++;
				goto end;
			case OP_KEXIT:
				N->err=(short)(*N->readptr==OP_NUMDATA?n_getnumber(N):0);
				n_error(N, N->err, __FN__, "exiting normally");
			case OP_KELSE:
				n_error(N, NE_SYNTAX, __FN__, "stray else");
			case OP_KDELETE:
				n_expect(N, __FN__, OP_LABEL);
				nsp_linkval(N, nsp_getobj(N, NULL, n_getlabel(N, namebuf)), NULL);
				goto endstmt;
			default:
				n_warn(N, __FN__, "? %d %s", op, n_getsym(N, op));
			}
		} else {
			obj_t *pobj=NULL;
			unsigned short z;
			uchar *p=N->readptr;
			uchar *e;

			if (*N->readptr!=OP_LABEL) n_error(N, NE_SYNTAX, __FN__, "expected a label [%d][%s]", *N->readptr, n_getsym(N, *N->readptr));
			tobj=&N->l;
			n_getlabel(N, namebuf);
			cobj=nsp_getobj(N, NULL, namebuf);
			while (*N->readptr==OP_POBRACKET||*N->readptr==OP_PDOT) {
				pobj=tobj=cobj;
				cobj=n_readindex(N, tobj, namebuf, &z);
				if (namebuf[0]&&(*N->readptr!=OP_POPAREN)&&(z||nsp_isnull(cobj))) {
					cobj=nsp_setnum(N, tobj, namebuf, 0);
				}
			}
			ctype=nsp_typeof(cobj);
			if (*N->readptr==OP_POPAREN) {
				if (ctype==NT_NFUNC||ctype==NT_CFUNC) {
					n_execfunction(N, cobj, pobj, 0);
					goto endstmt;
				} else {
					char errbuf[80];

					e=N->readptr-1;
					nc_memset(errbuf, 0, sizeof(errbuf));
					n_decompile(N, p, e, errbuf, sizeof(errbuf)-1);
					n_error(N, NE_SYNTAX, __FN__, "'%s' is not a function", errbuf);
				}
			}
			if (ctype==NT_NULL) {
				if (namebuf[0]==0) n_error(N, NE_SYNTAX, __FN__, "expected a label");
				cobj=nsp_setbool(N, tobj, namebuf, 0);
			}
			n_readvar(N, tobj, cobj);
		}
endstmt:
		if (*N->readptr==OP_PSEMICOL) N->readptr++;
		if (single) break;
	}
end:
	if (jmp==0) {
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
		if (p) n_free(N, (void *)&p, psize);
		N->blockend=NULL;
		N->readptr=NULL;
	}
	DEBUG_OUT();
	return NULL;
#undef __FN__
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <string.h>
int nsp_writefile(nsp_state *N, char *file, uchar *dat)
{
#define __FN__ __FILE__ ":nsp_writefile()"
	char outfile[512];
	int fd;

	nc_snprintf(N, outfile, sizeof(outfile), "/tmp/%s", file);
	n_warn(N, __FN__, "writing '%s' from [%s]", outfile, file);
	if ((fd=open(outfile, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE))!=-1) {
		write(fd, dat, readi4((dat+8)));
		close(fd);
		return 1;
	}
	return 0;
#undef __FN__
}

int nsp_execfile(nsp_state *N, char *file)
{
#define __FN__ __FILE__ ":nsp_execfile()"
	obj_t *cobj=nsp_getobj(N, &N->g, "_filepath");
	char buf[512];
	char *pfile;
	struct stat sb;
	uchar *p;
	int psize;
	uchar *oldbptr=N->blockptr;
	uchar *oldbend=N->blockend;
	uchar *oldrptr=N->readptr;
	short int jmp=N->savjmp?1:0;
	int bl;
	int fd;
	int r;
	int rc;
	char *o;

	settrace();
	if (jmp==0) {
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
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
	N->blockptr=(uchar *)n_alloc(N, sb.st_size+2, 0);
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
	n_decompose(N, N->blockptr, &p, &psize);
	if (p) {
		/* nsp_writefile(N, o, p); */
		n_free(N, (void *)&N->blockptr, sb.st_size+2);
		N->blockptr=p;
		p=NULL;
	}
	N->blockend=N->blockptr+readi4((N->blockptr+8));
	N->readptr=N->blockptr+readi4((N->blockptr+12));
	nsp_exec(N, (char *)N->readptr);
	if (N->outbuflen) nl_flush(N);
	rc=0;
end1:
	n_free(N, (void *)&N->blockptr, psize);
	N->blockptr=oldbptr;
	N->blockend=oldbend;
	N->readptr=oldrptr;
end2:
	if (jmp==0) {
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
	}
	return rc;
#undef __FN__
}

typedef struct {
	char *fn_name;
	NSP_CFUNC fn_ptr;
} FUNCTION;

nsp_state *nsp_newstate()
{
	FUNCTION list[]={
		{ "copy",	(NSP_CFUNC)nl_copy	},
		{ "eval",	(NSP_CFUNC)nl_eval	},
		{ "exec",	(NSP_CFUNC)nl_exec	},
		{ "exportvar",	(NSP_CFUNC)nl_exportvar	},
		{ "iname",	(NSP_CFUNC)nl_iname	},
		{ "include",	(NSP_CFUNC)nl_include	},
		{ "ival",	(NSP_CFUNC)nl_ival	},
		{ "print",	(NSP_CFUNC)nl_print	},
		{ "runtime",	(NSP_CFUNC)nl_runtime	},
		{ "sizeof",	(NSP_CFUNC)nl_sizeof	},
		{ "sleep",	(NSP_CFUNC)nl_sleep	},
		{ "system",	(NSP_CFUNC)nl_system	},
		{ "tonumber",	(NSP_CFUNC)nl_tonumber	},
		{ "tostring",	(NSP_CFUNC)nl_tostring	},
		{ "typeof",	(NSP_CFUNC)nl_typeof	},
		{ "write",	(NSP_CFUNC)nl_write	},
		{ "zlink",	(NSP_CFUNC)nl_zlink	},
		{ NULL, NULL }
	};
	FUNCTION list_file[]={
		{ "append",	(NSP_CFUNC)nl_filewrite	},
		{ "mkdir",	(NSP_CFUNC)nl_filemkdir	},
		{ "read",	(NSP_CFUNC)nl_fileread	},
		{ "rename",	(NSP_CFUNC)nl_filerename},
		{ "stat",	(NSP_CFUNC)nl_filestat  },
		{ "unlink",	(NSP_CFUNC)nl_fileunlink},
		{ "write",	(NSP_CFUNC)nl_filewrite	},
		{ NULL, NULL }
	};
	FUNCTION list_io[]={
		{ "print",	(NSP_CFUNC)nl_print	},
		{ "write",	(NSP_CFUNC)nl_write	},
		{ "flush",	(NSP_CFUNC)nl_flush	},
		{ NULL, NULL }
	};
	FUNCTION list_math[]={
		{ "abs",	(NSP_CFUNC)nl_math1	},
		{ "ceil",	(NSP_CFUNC)nl_math1	},
		{ "floor",	(NSP_CFUNC)nl_math1	},
		{ "rand",	(NSP_CFUNC)nl_math1	},
		{ NULL, NULL }
	};
	FUNCTION list_string[]={
		{ "atoi",	(NSP_CFUNC)nl_atoi	},
		{ "itoa",	(NSP_CFUNC)nl_itoa	},
		{ "cat",	(NSP_CFUNC)nl_strcat	},
		{ "cmp",	(NSP_CFUNC)nl_strcmp	},
		{ "icmp",	(NSP_CFUNC)nl_strcmp	},
		{ "ncmp",	(NSP_CFUNC)nl_strcmp	},
		{ "nicmp",	(NSP_CFUNC)nl_strcmp	},
		{ "join",	(NSP_CFUNC)nl_strjoin	},
		{ "len",	(NSP_CFUNC)nl_strlen	},
		{ "replace",	(NSP_CFUNC)nl_strrep	},
		{ "split",	(NSP_CFUNC)nl_strsplit	},
		{ "str",	(NSP_CFUNC)nl_strstr	},
		{ "istr",	(NSP_CFUNC)nl_strstr	},
		{ "sub",	(NSP_CFUNC)nl_strsub	},
		{ "tolower",	(NSP_CFUNC)nl_strtolower},
		{ "toupper",	(NSP_CFUNC)nl_strtolower},
		{ NULL, NULL }
	};
	FUNCTION list_time[]={
		{ "gmtime",	(NSP_CFUNC)nl_gmtime	},
		{ "gettimeofday",(NSP_CFUNC)nl_gettimeofday	},
		{ "localtime",	(NSP_CFUNC)nl_gmtime	},
		{ "sqldate",	(NSP_CFUNC)nl_sqltime	},
		{ "sqltime",	(NSP_CFUNC)nl_sqltime	},
		{ "now",	(NSP_CFUNC)nl_time	},
		{ NULL, NULL }
	};
	nsp_state *new_N;
	obj_t *cobj;
	short i;

	new_N=(nsp_state *)n_alloc(NULL, sizeof(nsp_state), 1);
	nc_gettimeofday(&new_N->ttime, NULL);
	srand(new_N->ttime.tv_usec);
	new_N->maxwarnings=500;
#if defined(__GNUC__)
	new_N->warnformat='a';
#else
	new_N->warnformat='r';
#endif

	nsp_setvaltype(new_N, &new_N->g, NT_TABLE);
	new_N->g.val->attr|=NST_AUTOSORT;

	nsp_setvaltype(new_N, &new_N->l, NT_TABLE);
	new_N->l.val->attr|=NST_AUTOSORT;

	n_setname(new_N, &new_N->g, "!GLOBALS!");
	n_setname(new_N, &new_N->l, "!LOCALS!");
	n_setname(new_N, &new_N->r, "!RETVAL!");
	cobj=nsp_settable(new_N, &new_N->g, "_GLOBALS");
	cobj->val->attr|=NST_HIDDEN;
	nsp_linkval(new_N, cobj, &new_N->g);

	for (i=0;list[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, &new_N->g, list[i].fn_name, list[i].fn_ptr);
	}
	cobj=nsp_settable(new_N, &new_N->g, "file");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_file[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, cobj, list_file[i].fn_name, list_file[i].fn_ptr);
	}
	cobj=nsp_settable(new_N, &new_N->g, "io");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_io[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, cobj, list_io[i].fn_name, list_io[i].fn_ptr);
	}
	cobj=nsp_settable(new_N, &new_N->g, "math");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_math[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, cobj, list_math[i].fn_name, list_math[i].fn_ptr);
	}
	cobj=nsp_settable(new_N, &new_N->g, "string");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_string[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, cobj, list_string[i].fn_name, list_string[i].fn_ptr);
	}
	cobj=nsp_settable(new_N, &new_N->g, "time");
	cobj->val->attr|=NST_HIDDEN;
	for (i=0;list_time[i].fn_name!=NULL;i++) {
		nsp_setcfunc(new_N, cobj, list_time[i].fn_name, list_time[i].fn_ptr);
	}
	cobj=nsp_setnum(new_N, &new_N->g, "null", 0);
	cobj->val->type=NT_NULL; cobj->val->attr|=NST_SYSTEM;
	cobj=nsp_setnum(new_N, &new_N->g, "false", 0);
	cobj->val->type=NT_BOOLEAN; cobj->val->attr|=NST_SYSTEM;
	cobj=nsp_setnum(new_N, &new_N->g, "true", 1);
	cobj->val->type=NT_BOOLEAN; cobj->val->attr|=NST_SYSTEM;
	cobj=nsp_setstr(new_N, &new_N->g, "_version_", NSP_VERSION, -1);
	cobj=nsp_setstr(new_N, &new_N->g, "_ostype_", _OSTYPE_, -1);
	return new_N;
}

nsp_state *nsp_endstate(nsp_state *N)
{
#define __FN__ __FILE__ ":nsp_endstate()"
	if (N!=NULL) {
		settrace();
		if (N->outbuflen) nl_flush(N);
		nsp_freetable(N, &N->g);
		nsp_freetable(N, &N->l);
		n_freeval(N, &N->r);
		if (N->g.val) n_free(N, (void *)&N->g.val, sizeof(val_t));
		if (N->l.val) n_free(N, (void *)&N->l.val, sizeof(val_t));
		if (N->r.val) n_free(N, (void *)&N->r.val, sizeof(val_t));
		n_free(N, (void *)&N, sizeof(nsp_t));
	}
	return NULL;
#undef __FN__
}
