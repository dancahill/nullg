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
#include "nesla/libnesla.h"
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
#define __FUNCTION__ __FILE__ ":n_escape()"
	char *dst=N->outbuf;
	int i, n=N->outbuflen;

	settrace();
	for (i=0;i<len;i++) {
		if (MAX_OUTBUFLEN-n<32) { N->outbuflen=n; nl_flush(N); n=N->outbuflen; }
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
#undef __FUNCTION__
}

static void n_dumpvars(nes_state *N, obj_t *tobj, int depth)
{
#define __FUNCTION__ __FILE__ ":n_dumpvars()"
	obj_t *cobj=tobj;
	int i;
	char b;
	char *g;
	int ent=0;

	settrace();
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
#undef __FUNCTION__
}

/*
 * basic i/o functions
 */
NES_FUNCTION(nl_flush)
{
#define __FUNCTION__ __FILE__ ":nl_flush()"
	obj_t *cobj;

	if (N==NULL||N->outbuflen==0) return 0;
	cobj=&N->g;
	if (!nes_istable(cobj)) goto flush;
	for (cobj=cobj->val->d.table;cobj;cobj=cobj->next) {
		if (nc_strcmp(cobj->name, "io")!=0) continue;
		if (cobj->val->type!=NT_TABLE) goto flush;
		for (cobj=cobj->val->d.table;cobj;cobj=cobj->next) {
			if (nc_strcmp(cobj->name, "flush")!=0) continue;
			if (cobj->val->type!=NT_CFUNC) goto flush;
			if (cobj->val->d.cfunc==(NES_CFUNC)nl_flush) goto flush;
			return cobj->val->d.cfunc(N);
		}
	}
flush:
	if (N->outbuflen==0) return 0;
	N->outbuf[N->outbuflen]='\0';
	write(STDOUT_FILENO, N->outbuf, N->outbuflen);
	N->outbuflen=0;
	/* do NOT touch &N->r */
	return 0;
#undef __FUNCTION__
}

static int writestr(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":writestr()"
	unsigned int i;
	unsigned int len=0;
	char *p;

	settrace();
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_print)
{
#define __FUNCTION__ __FILE__ ":nl_print()"
	obj_t *cobj=N->l.val->d.table;
	int tlen=0;

	settrace();
	for (cobj=cobj->next;cobj;cobj=cobj->next) {
		if (cobj->name[0]!='n') tlen+=writestr(N, cobj);
	}
	if (N->debug) nl_flush(N);
	nes_setnum(N, &N->r, "", tlen);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_write)
{
#define __FUNCTION__ __FILE__ ":nl_write()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int len;

	settrace();
	len=writestr(N, cobj1);
	nes_setnum(N, &N->r, "", len);
	return 0;
#undef __FUNCTION__
}

/*
 * file functions
 */
NES_FUNCTION(nl_filemkdir)
{
#define __FUNCTION__ __FILE__ ":nl_filemkdir()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	mode_t umask=755;
#endif
	int rc;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	if (cobj2->val->type==NT_NUMBER) umask=(mode_t)cobj2->val->d.num;
	rc=mkdir(cobj1->val->d.str, ~umask&0777);
#else
	rc=mkdir(cobj1->val->d.str);
#endif
	nes_setnum(N, &N->r, "", rc);
	return rc;
#undef __FUNCTION__
}


NES_FUNCTION(nl_fileread)
{
#define __FUNCTION__ __FILE__ ":nl_fileread()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset=0;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
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
	while (bl>0) {
		if ((r=read(fd, p, bl))<0) break;
		p+=r;
		bl-=r;
	}
	close(fd);
	robj->val->d.str[sb.st_size]='\0';
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_filestat)
{
#define __FUNCTION__ __FILE__ ":nl_filestat()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym=0;
	char *file;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	file=cobj1->val->d.str;
#if defined(WIN32) || defined(__TURBOC__)
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_fileunlink)
{
#define __FUNCTION__ __FILE__ ":nl_fileunlink()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int rc=-1;
	int i;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	for (i=1;;i++) {
		cobj1=nes_getiobj(N, &N->l, i);
		if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) break;
		rc=unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nes_setnum(N, &N->r, "", rc);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_filewrite)
{
#define __FUNCTION__ __FILE__ ":nl_filewrite()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	char *fname=nes_getstr(N, &N->l, "0");
	int fd=-1;
	int w=0;
	int offset=0;

	settrace();
	/* umask(022); */
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (nc_strcmp(fname, "write")==0) {
		fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
	} else if (nc_strcmp(fname, "append")==0) {
		fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
	}
	if (fd==-1) {
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
#undef __FUNCTION__
}

/*
 * math functions
 */
NES_FUNCTION(nl_math1)
{
#define __FUNCTION__ __FILE__ ":nl_math1()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char *fname=nes_getstr(N, &N->l, "0");
	num_t n;

	settrace();
	if (nc_strcmp(fname, "rand")==0) {
		n=rand();
		if (cobj1->val->type==NT_NUMBER&&(int)cobj1->val->d.num) {
			n=(int)n%(int)cobj1->val->d.num;
		}
		n=(int)n%65535;
		nes_setnum(N, &N->r, "", n);
		return 0;
	}
	if (cobj1->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number");
	if (nc_strcmp(fname, "abs")==0) {
		n=fabs(cobj1->val->d.num);
	} else if (nc_strcmp(fname, "ceil")==0) {
		n=cobj1->val->d.num;
		if ((int)n<n) n=(int)n+1; else n=(int)n;
	} else if (nc_strcmp(fname, "floor")==0) {
		n=(int)cobj1->val->d.num;
	} else {
		n=0;
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_tonumber)
{
#define __FUNCTION__ __FILE__ ":nl_tonumber()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	settrace();
	nes_setnum(N, &N->r, "", nes_tonum(N, cobj1));
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_tostring)
{
#define __FUNCTION__ __FILE__ ":nl_tostring()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	unsigned short d;
	char *p;

	settrace();
	if (cobj1->val->type==NT_NUMBER&&cobj2->val->type==NT_NUMBER) {
		d=(unsigned short)cobj2->val->d.num;
		if (d>sizeof(N->numbuf)-2) d=sizeof(N->numbuf)-2;
		p=n_ntoa(N, N->numbuf, cobj1->val->d.num, 10, d);
	} else {
		p=nes_tostr(N, cobj1);
	}
	nes_setstr(N, &N->r, "", p, p?nc_strlen(p):0);
	return 0;
#undef __FUNCTION__
}

/*
 * string functions
 */
NES_FUNCTION(nl_strcat)
{
#define __FUNCTION__ __FILE__ ":nl_strcat()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg2");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	robj->val->size=cobj1->val->size+cobj2->val->size;
	robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
	nc_memcpy(robj->val->d.str, cobj1->val->d.str, cobj1->val->size);
	nc_memcpy(robj->val->d.str+cobj1->val->size, cobj2->val->d.str, cobj2->val->size);
	robj->val->d.str[robj->val->size]=0;
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_strcmp)
{
#define __FUNCTION__ __FILE__ ":nl_strcmp()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	char *fname=nes_getstr(N, &N->l, "0");
	uchar *s1, *s2;
	int i, rval=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg2");
	if (cobj1->val->d.str==NULL) {
		if (cobj2->val->d.str==NULL) rval=0; else rval=(int)cobj2->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	} else if (cobj2->val->d.str==NULL) {
		rval=-(int)cobj1->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	}
	if (nc_strcmp(fname, "cmp")==0) {
		rval=nc_strcmp(cobj1->val->d.str, cobj2->val->d.str);
	} else if (nc_strcmp(fname, "ncmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number for arg3");
		rval=nc_strncmp(cobj1->val->d.str, cobj2->val->d.str, (int)cobj3->val->d.num);
	} else if (nc_strcmp(fname, "icmp")==0) {
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			s1++; s2++;
		} while (*s1!=0);
	} else if (nc_strcmp(fname, "nicmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number for arg3");
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str; i=(int)cobj3->val->d.num;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			if (--i<1) break;
			s1++; s2++;
		} while (*s1!=0);
	}
	nes_setnum(N, &N->r, "", rval);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_strjoin)
{
#define __FUNCTION__ __FILE__ ":nl_strjoin()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj, *robj;
	char *p=NULL, *p2;
	int len1=0, len2=0;
	int cnt=0;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a table for arg1");
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_strlen)
{
#define __FUNCTION__ __FILE__ ":nl_strlen()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	nes_setnum(N, &N->r, "", cobj1->val->size);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_strsplit)
{
#define __FUNCTION__ __FILE__ ":nl_strsplit()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t tobj;
	char *ss, *se;
	int i=0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN+1];

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg2");
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_strstr)
{
#define __FUNCTION__ __FILE__ ":nl_strstr()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	char *fname=nes_getstr(N, &N->l, "0");
	unsigned int i=0, j=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg2");
	if (cobj2->val->size<1) n_error(N, NE_SYNTAX, __FUNCTION__, "zero length arg2");
	if (nc_strcmp(fname, "str")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (cobj2->val->d.str[j]!=cobj1->val->d.str[i]) j=-1;
		}
	} else if (nc_strcmp(fname, "istr")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (nc_tolower(cobj2->val->d.str[j])!=nc_tolower(cobj1->val->d.str[i])) j=-1;
		}
	} else {
		n_error(N, NE_SYNTAX, __FUNCTION__, "................");
	}
	if (i<=cobj1->val->size&&j==cobj2->val->size) {
		nes_setstr(N, &N->r, "", cobj1->val->d.str+i-j, cobj1->val->size-i+j);
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_strsub)
{
#define __FUNCTION__ __FILE__ ":nl_strsub()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	unsigned int offset, max=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number for arg2");
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_strtolower)
{
#define __FUNCTION__ __FILE__ ":nl_strtolower()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *robj;
	char *fname=nes_getstr(N, &N->l, "0");
	int i;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a string for arg1");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str!=NULL) {
		robj->val->size=cobj1->val->size;
		robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
		robj->val->d.str[0]=0;
		i=robj->val->size-1;
		if (nc_strcmp(fname, "tolower")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_tolower(cobj1->val->d.str[i]);
		} else if (nc_strcmp(fname, "toupper")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_toupper(cobj1->val->d.str[i]);
		}
		robj->val->d.str[robj->val->size]=0;
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_sqltime)
{
#define __FUNCTION__ __FILE__ ":nl_sqltime()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char *fname=nes_getstr(N, &N->l, "0");
	struct timeval ttime;
	char timebuf[16];

	settrace();
	if (cobj1->val->type==NT_NUMBER) {
		ttime.tv_sec=(time_t)cobj1->val->d.num;
	} else {
		nc_gettimeofday(&ttime, NULL);
	}
	if (nc_strcmp(fname, "sqldate")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%Y-%m-%d", localtime((time_t *)&ttime.tv_sec));
	} else if (nc_strcmp(fname, "sqltime")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	}
	nes_setstr(N, &N->r, "", timebuf, -1);
	return 0;	
#undef __FUNCTION__
}

NES_FUNCTION(nl_time)
{
#define __FUNCTION__ __FILE__ ":nl_time()"
	settrace();
	nes_setnum(N, &N->r, "", time(NULL));
	return 0;	
#undef __FUNCTION__
}

NES_FUNCTION(nl_gmtime)
{
#define __FUNCTION__ __FILE__ ":nl_gmtime()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char *fname=nes_getstr(N, &N->l, "0");
	obj_t tobj;
	time_t t;
	struct tm *tp;

	settrace();
	if (cobj1->val->type==NT_NUMBER) {
		t=(time_t)cobj1->val->d.num;
	} else {
		t=time(NULL);
	}
	tobj.val=n_newval(N, NT_TABLE);
	tobj.val->attr&=~NST_AUTOSORT;
	if (nc_strcmp(fname, "gmtime")==0) {
		tp=gmtime(&t);
	} else if (nc_strcmp(fname, "localtime")==0) {
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_runtime)
{
#define __FUNCTION__ __FILE__ ":nl_runtime()"
	struct timeval ttime;
	int totaltime;

	settrace();
	nc_gettimeofday(&ttime, NULL);
	totaltime=((ttime.tv_sec-N->ttime.tv_sec)*1000000)+(ttime.tv_usec-N->ttime.tv_usec);
	nes_setnum(N, &N->r, "", (num_t)totaltime/1000000);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_sleep)
{
#define __FUNCTION__ __FILE__ ":nl_sleep()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=1;

	settrace();
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_eval)
{
#define __FUNCTION__ __FILE__ ":nl_eval()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char *fname=nes_getstr(N, &N->l, "0");
	uchar *p;

	settrace();
	if (nc_strcmp(fname, "eval")!=0) {
		n_error(N, NE_SYNTAX, __FUNCTION__, "cowardly refusing to run aliased eval");
	}
	if ((cobj1->val->type==NT_STRING)&&(cobj1->val->d.str!=NULL)) {
		uchar *oldbptr=N->blockptr;
		uchar *oldbend=N->blockend;
		uchar *oldrptr=N->readptr;
		jmp_buf *savjmp;

		p=n_decompose(N, (uchar *)cobj1->val->d.str);
		if (p!=N->blockptr) N->blockptr=p; else p=NULL;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));

		savjmp=N->savjmp;
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)==0) {
			nes_linkval(N, &N->r, nes_eval(N, (char *)N->readptr));
		}
		n_free(N, (void *)&N->savjmp);
		N->savjmp=savjmp;

		if (p) n_free(N, (void *)&p);

		N->blockptr=oldbptr;
		N->blockend=oldbend;
		N->readptr=oldrptr;
	} else {
		nes_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_exec)
{
#define __FUNCTION__ __FILE__ ":nl_exec()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char *fname=nes_getstr(N, &N->l, "0");
	uchar *p;

	settrace();
	if (nc_strcmp(fname, "exec")!=0) {
		n_error(N, NE_SYNTAX, __FUNCTION__, "cowardly refusing to run aliased exec");
	}
	if ((cobj1->val->type==NT_STRING)&&(cobj1->val->d.str!=NULL)) {
		uchar *oldbptr=N->blockptr;
		uchar *oldbend=N->blockend;
		uchar *oldrptr=N->readptr;
		jmp_buf *savjmp;

		p=n_decompose(N, (uchar *)cobj1->val->d.str);
		if (p!=N->blockptr) N->blockptr=p; else p=NULL;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));

		savjmp=N->savjmp;
		N->savjmp=n_alloc(N, sizeof(jmp_buf), 1);
		if (setjmp(*N->savjmp)==0) {
			nes_linkval(N, &N->r, nes_exec(N, (char *)N->readptr));
		}
		n_free(N, (void *)&N->savjmp);
		N->savjmp=savjmp;

		if (p) n_free(N, (void *)&p);

		N->blockptr=oldbptr;
		N->blockend=oldbend;
		N->readptr=oldrptr;
	} else {
		nes_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_iname)
{
#define __FUNCTION__ __FILE__ ":nl_iname()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nes_isnull(cobj)) {
		nes_setnum(N, &N->r, "", 0);
	} else {
		nes_setstr(N, &N->r, "", cobj->name, -1);
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_ival)
{
#define __FUNCTION__ __FILE__ ":nl_ival()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FUNCTION__, "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (!nes_isnull(cobj)) nes_linkval(N, &N->r, cobj);
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_include)
{
#define __FUNCTION__ __FILE__ ":nl_include()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	uchar *p;
	int n=0;

	settrace();
	if (!nes_isnull(cobj1)) {
		p=N->readptr;
		n=nes_execfile(N, (char *)cobj1->val->d.str);
		N->readptr=p;
	}
	nes_setnum(N, &N->r, "", n);
	return n;
#undef __FUNCTION__
}

NES_FUNCTION(nl_printvar)
{
#define __FUNCTION__ __FILE__ ":nl_printvar()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	settrace();
	if (!nes_isnull(cobj1)) {
		n_dumpvars(N, cobj1, 0);
		nes_setnum(N, &N->r, "", 0);
	}
	return 0;
#undef __FUNCTION__
}

NES_FUNCTION(nl_sizeof)
{
#define __FUNCTION__ __FILE__ ":nl_sizeof()"
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	int size=0;

	settrace();
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_typeof)
{
#define __FUNCTION__ __FILE__ ":nl_typeof()"
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	NES_CDATA *chead;
	char *p;

	settrace();
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
#undef __FUNCTION__
}

NES_FUNCTION(nl_system)
{
#define __FUNCTION__ __FILE__ ":nl_system()"
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=-1;

	settrace();
	if (cobj1->val->type==NT_STRING&&cobj1->val->d.str!=NULL) {
		nl_flush(N);
		n=system(cobj1->val->d.str);
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
#undef __FUNCTION__
}
