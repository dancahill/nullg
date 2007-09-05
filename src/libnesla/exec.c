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
#include "nesla/libnesla.h"
#include "opcodes.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#ifdef __TURBOC__
#else
#include <unistd.h>
#endif
#endif

static obj_t *n_setargs(nes_state *N, char *fname, obj_t *pobj)
{
#define __FUNCTION__ "n_setargs"
	obj_t listobj;
	obj_t *cobj, *nobj;
	unsigned short i;

	n_expect(N, __FUNCTION__, OP_POPAREN);
	listobj.val=n_newval(N, NT_TABLE);
	listobj.val->attr=0;
	cobj=nes_setstr(N, &listobj, "0", fname, nc_strlen(fname));
	for (i=1;;i++) {
		N->readptr++;
		if (*N->readptr==OP_PCPAREN) break;
		if ((nobj=nes_eval(N, (char *)N->readptr))!=NULL) {
			cobj->next=n_newiobj(N, i);
			cobj->next->prev=cobj;
			cobj=cobj->next;
			nes_linkval(N, cobj, nobj);
		}
		if (*N->readptr==OP_PCOMMA) continue;
		n_expect(N, __FUNCTION__, OP_PCPAREN);
		break;
	}
	if (pobj) {
		cobj->next=n_alloc(N, sizeof(obj_t), 1);
		cobj->next->prev=cobj;
		cobj=cobj->next;
		cobj->next=NULL;
		nc_strncpy(cobj->name, "this", MAX_OBJNAMELEN);
		nes_linkval(N, cobj, pobj);
	}
	N->readptr++;
	nes_linkval(N, &N->r, &listobj);
	nes_unlinkval(N, &listobj);
	return &N->r;
#undef __FUNCTION__
}

obj_t *n_execfunction(nes_state *N, obj_t *cobj, obj_t *pobj)
{
#define __FUNCTION__ "n_execfunction"
	NES_CFUNC cfunc;
	obj_t *cobj2;
	obj_t *lobj;
	val_t *olobj;
	uchar *oldbptr;
	uchar *oldbend;
	uchar *oldrptr;
	unsigned int i;

	DEBUG_IN();
	/* if (N->debug) n_warn(N, __FUNCTION__, "%s()", cobj->name); */
	/* if (N->debug) if (pobj) n_warn(N, __FUNCTION__, "parent object name = %s", pobj->name); */
	if (cobj->val->type==NT_CFUNC) {
		lobj=n_setargs(N, cobj->name, pobj);
		olobj=N->l.val; N->l.val=lobj->val; lobj->val=NULL;
		cfunc=cobj->val->d.cfunc;
		cfunc(N);
		nes_unlinkval(N, &N->l);
		N->l.val=olobj;
	} else if (cobj->val->type==NT_NFUNC) {
		lobj=n_setargs(N, cobj->name, pobj);
		olobj=N->l.val; N->l.val=lobj->val; lobj->val=NULL;
		oldbptr=N->blockptr;
		oldbend=N->blockend;
		oldrptr=N->readptr;
		N->blockptr=(uchar *)cobj->val->d.str;
		N->blockend=(uchar *)cobj->val->d.str+cobj->val->size;
		N->readptr=(uchar *)cobj->val->d.str;
		n_expect(N, __FUNCTION__, OP_POPAREN);
		for (i=1;;i++) {
			N->readptr++;
			cobj2=nes_getiobj(N, &N->l, i);
			if (*N->readptr==OP_LABEL) nc_strncpy(cobj2->name, n_getlabel(N, NULL), MAX_OBJNAMELEN);
			if (*N->readptr==OP_PCOMMA) continue;
			n_expect(N, __FUNCTION__, OP_PCPAREN);
			N->readptr++;
			break;
		}
		n_expect(N, __FUNCTION__, OP_POBRACE);
		nes_exec(N, (char *)N->readptr);
		n_skipto(N, __FUNCTION__, OP_PCBRACE);
		if (N->ret) { N->ret=0; }
		N->blockptr=oldbptr;
		N->blockend=oldbend;
		N->readptr=oldrptr;
		nes_unlinkval(N, &N->l);
		N->l.val=olobj;
	} else {
		n_error(N, NE_SYNTAX, __FUNCTION__, "'%s' is not a function", cobj->name);
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

//	new_N->g.val=n_newval(N, NT_TABLE);
	new_N->g.val=n_alloc(new_N, sizeof(val_t), 1);
	new_N->g.val->type=NT_TABLE;
	new_N->g.val->d.table=NULL;
	new_N->g.val->refs=1;
	new_N->g.val->attr|=NST_AUTOSORT;

	new_N->l.val=n_alloc(new_N, sizeof(val_t), 1);
	new_N->l.val->type=NT_TABLE;
	new_N->l.val->d.table=NULL;
	new_N->l.val->refs=1;
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
	cobj=nes_setstr(new_N, &new_N->g, "_version_", NESLA_VERSION, nc_strlen(NESLA_VERSION));
	return new_N;
}

nes_state *nes_endstate(nes_state *N)
{
	if (N!=NULL) {
		if (N->outbuflen) nl_flush(N);
		nes_freetable(N, &N->g);
		nes_freetable(N, &N->l);
		if (N->g.val) n_free(N, (void *)&N->g.val);
		if (N->l.val) n_free(N, (void *)&N->l.val);
		if (N->r.val) n_free(N, (void *)&N->r.val);
		n_free(N, (void *)&N);
	}
	return NULL;
}
