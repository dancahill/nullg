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
static int n_escape(nsp_state *N, const char *str, int len)
{
#define __FN__ __FILE__ ":n_escape()"
	char buf[512];
	char *dst=buf;
	int i, n=0;

	settrace();
	for (i=0;i<len;i++) {
		if (sizeof(buf)-n<32) { nsp_strcat(N, &N->r, buf, n); n=0; }
		switch (str[i]) {
		case '\0' : dst[n++]='\\'; dst[n++]=0;   break;
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
	nsp_strcat(N, &N->r, buf, n);
	return len;
#undef __FN__
}

void n_dumpvars(nsp_state *N, obj_t *tobj, int depth)
{
#define __FN__ __FILE__ ":n_dumpvars()"
	obj_t *cobj=tobj, *robj;
	int i;
	char b;
	char *g;
	int ent=0;

	char buf[512];
	unsigned short buflen=0;

	if (depth==0) robj=nsp_setstr(N, &N->r, "", NULL, 0); else robj=&N->r;
	settrace();
	for (;cobj;cobj=cobj->next) {
		if (buflen>512) {
			nsp_strcat(N, robj, buf, buflen);
			buflen=0;
		}
		if (nsp_isnull(cobj)||cobj->val->attr&NST_HIDDEN||cobj->val->attr&NST_SYSTEM) continue;
		g=(depth<1)?"global ":"";
		if (nc_isdigit(cobj->name[0])) b=1; else b=0;
		if (cobj->val->type==NT_BOOLEAN||cobj->val->type==NT_NUMBER) {
			if (ent++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\t");
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s", nsp_tostr(N, cobj));
		} else if (cobj->val->type==NT_STRING) {
			if (ent++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\t");
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\"");
			nsp_strcat(N, robj, buf, buflen);
			buflen=0;
			n_escape(N, cobj->val->d.str?cobj->val->d.str:"", cobj->val->size);
			buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\"");
		} else if (cobj->val->type==NT_TABLE) {
			if (nc_strcmp(cobj->name, "_GLOBALS")==0) continue;
			/* if (nc_strcmp(cobj->name, "this")==0) continue; */
			if (ent++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s\n", depth?",":"");
			if (depth) {
				for (i=0;i<depth;i++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\t");
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s%s%s%s = ", g, b?"[":"", cobj->name, b?"]":"");
			}
			buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "{");
			if (cobj->val->d.table.f) {
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\n");
				nsp_strcat(N, robj, buf, buflen);
				buflen=0;
				n_dumpvars(N, cobj->val->d.table.f, depth+1);
				for (i=0;i<depth;i++) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "\t");
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "}");
			} else {
				buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, " }");
			}
		}
	}
	if (ent) buflen+=nc_snprintf(N, buf+buflen, sizeof(buf)-buflen, "%s", depth?"\n":"");
	nsp_strcat(N, robj, buf, buflen);
	buflen=0;
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

	if (N==NULL||N->outbuflen==0) return 0;
	cobj=&N->g;
	if (!nsp_istable(cobj)) goto flush;
	for (cobj=cobj->val->d.table.f;cobj;cobj=cobj->next) {
		if (nc_strcmp(cobj->name, "io")!=0) continue;
		if (cobj->val->type!=NT_TABLE) goto flush;
		for (cobj=cobj->val->d.table.f;cobj;cobj=cobj->next) {
			if (nc_strcmp(cobj->name, "flush")!=0) continue;
			if (cobj->val->type!=NT_CFUNC) goto flush;
			if (cobj->val->d.cfunc==(NSP_CFUNC)nl_flush) goto flush;
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
#undef __FN__
}

static int writestr(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":writestr()"
	unsigned int i;
	unsigned int len=0;
	char *p;

	settrace();
	if (nsp_isstr(cobj)) {
		p=cobj->val->d.str;
		len=cobj->val->size;
	} else {
		p=nsp_tostr(N, cobj);
		len=nc_strlen(p);
	}
	for (i=0;i<len;i++) {
		if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
		N->outbuf[N->outbuflen++]=p[i];
	}
	N->outbuf[N->outbuflen]='\0';
	return len;
#undef __FN__
}

NSP_FUNCTION(nl_print)
{
#define __FN__ __FILE__ ":nl_print()"
	/* obj_t *cobj=N->l.val->d.table.f; */
	obj_t *cobj=nsp_getobj(N, &N->l, "0");
	int tlen=0;

	settrace();
	for (cobj=cobj->next;cobj;cobj=cobj->next) {
		if (cobj->name[0]!='n') tlen+=writestr(N, cobj);
	}
	if (N->debug) nl_flush(N);
	nsp_setnum(N, &N->r, "", tlen);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_write)
{
#define __FN__ __FILE__ ":nl_write()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	int len;

	settrace();
	len=writestr(N, cobj1);
	nsp_setnum(N, &N->r, "", len);
	return 0;
#undef __FN__
}

/*
 * file functions
 */
NSP_FUNCTION(nl_filemkdir)
{
#define __FN__ __FILE__ ":nl_filemkdir()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	mode_t umask=0755;
#endif
	int rc;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	if (cobj2->val->type==NT_NUMBER) umask=(mode_t)cobj2->val->d.num;
	rc=mkdir(cobj1->val->d.str, umask&0777);
#else
	rc=mkdir(cobj1->val->d.str);
#endif
	nsp_setnum(N, &N->r, "", rc);
	return rc;
#undef __FN__
}


NSP_FUNCTION(nl_fileread)
{
#define __FN__ __FILE__ ":nl_fileread()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset=0;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (stat(cobj1->val->d.str, &sb)!=0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd=open(cobj1->val->d.str, O_RDONLY|O_BINARY))==-1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type==NT_NUMBER) {
		offset=(int)cobj2->val->d.num;
		lseek(fd, offset, SEEK_SET);
	}
	robj=nsp_setstr(N, &N->r, "", NULL, 0);
	bl=sb.st_size-offset;
	robj->val->d.str=(char *)n_alloc(N, bl+2, 0);
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
#undef __FN__
}

NSP_FUNCTION(nl_filerename)
{
#define __FN__ __FILE__ ":nl_filerename()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	struct stat sb;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING||cobj2->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (stat(cobj1->val->d.str, &sb)!=0) {
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
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym=0;
	char *file;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	file=cobj1->val->d.str;
#if defined(WIN32) || defined(__TURBOC__)
	rc=stat(file, &sb);
	if (rc!=0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
	rc=lstat(file, &sb);
	if (rc!=0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	if (!(~sb.st_mode&S_IFLNK)) {
		sym=1;
		if (stat(file, &sb)!=0) sym=2;
	}
#endif
	tobj.val=n_newval(N, NT_TABLE);
	nsp_setnum(N, &tobj, "mtime", (num_t)sb.st_mtime);
	if (sym==2) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", "broken", 6);
	} else if ((sb.st_mode&S_IFDIR)) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", sym?"dirp":"dir", sym?4:3);
	} else {
		nsp_setnum(N, &tobj, "size", sb.st_size);
		nsp_setstr(N, &tobj, "type", sym?"filep":"file", sym?5:4);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_fileunlink)
{
#define __FN__ __FILE__ ":nl_fileunlink()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	int rc=-1;
	int i;

	settrace();
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	for (i=1;;i++) {
		cobj1=nsp_getobj(N, &N->l, n_ntoa(N, N->numbuf, i, 10, 0));
		if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) break;
		rc=unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nsp_setnum(N, &N->r, "", rc);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_filewrite)
{
#define __FN__ __FILE__ ":nl_filewrite()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj3=nsp_getobj(N, &N->l, "3");
	char *fname=nsp_getstr(N, &N->l, "0");
	int fd=-1;
	int w=0;
	int offset=0;

	settrace();
	/* umask(022); */
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (nc_strcmp(fname, "write")==0) {
		fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
	} else if (nc_strcmp(fname, "append")==0) {
		fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE);
	}
	if (fd==-1) {
		nsp_setnum(N, &N->r, "", -1);
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
	nsp_setnum(N, &N->r, "", w);
	return 0;
#undef __FN__
}

/*
 * math functions
 */
NSP_FUNCTION(nl_math1)
{
#define __FN__ __FILE__ ":nl_math1()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	char *fname=nsp_getstr(N, &N->l, "0");
	num_t n;

	settrace();
	if (nc_strcmp(fname, "rand")==0) {
		n=rand();
		if (cobj1->val->type==NT_NUMBER&&(int)cobj1->val->d.num) {
			n=(int)n%(int)cobj1->val->d.num;
		}
		n=(int)n%65535;
		nsp_setnum(N, &N->r, "", n);
		return 0;
	}
	if (cobj1->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number");
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
	nsp_setnum(N, &N->r, "", n);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_tonumber)
{
#define __FN__ __FILE__ ":nl_tonumber()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");

	settrace();
	nsp_setnum(N, &N->r, "", nsp_tonum(N, cobj1));
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_tostring)
{
#define __FN__ __FILE__ ":nl_tostring()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	unsigned short d;
	char *p;

	settrace();
	if (cobj1->val->type==NT_NUMBER&&cobj2->val->type==NT_NUMBER) {
		d=(unsigned short)cobj2->val->d.num;
		if (d>sizeof(N->numbuf)-2) d=sizeof(N->numbuf)-2;
		p=n_ntoa(N, N->numbuf, cobj1->val->d.num, 10, d);
	} else {
		p=nsp_tostr(N, cobj1);
	}
	nsp_setstr(N, &N->r, "", p, p?nc_strlen(p):0);
	return 0;
#undef __FN__
}

/*
 * string functions
 */
NSP_FUNCTION(nl_atoi)
{
#define __FN__ __FILE__ ":nl_atoi()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	unsigned int offset=0;
	short i=-1;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type==NT_NUMBER) offset=(int)cobj2->val->d.num;
	if (offset<cobj1->val->size && cobj1->val->size>0) {
		i=(int)cobj1->val->d.str[offset]&255;
	}
	nsp_setnum(N, &N->r, "", i);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_itoa)
{
#define __FN__ __FILE__ ":nl_itoa()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *robj;

	settrace();
	if (cobj1->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg1");
	robj=nsp_setstr(N, &N->r, "", " ", 1);
	robj->val->d.str[0]=(int)cobj1->val->d.num&255;
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strcat)
{
#define __FN__ __FILE__ ":nl_strcat()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *robj;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	robj=nsp_setstr(N, &N->r, "", NULL, 0);
	robj->val->size=cobj1->val->size+cobj2->val->size;
	robj->val->d.str=(char *)n_alloc(N, robj->val->size+1, 0);
	nc_memcpy(robj->val->d.str, cobj1->val->d.str, cobj1->val->size);
	nc_memcpy(robj->val->d.str+cobj1->val->size, cobj2->val->d.str, cobj2->val->size);
	robj->val->d.str[robj->val->size]=0;
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strcmp)
{
#define __FN__ __FILE__ ":nl_strcmp()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj3=nsp_getobj(N, &N->l, "3");
	char *fname=nsp_getstr(N, &N->l, "0");
	uchar *s1, *s2;
	int i, rval=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj1->val->d.str==NULL) {
		if (cobj2->val->d.str==NULL) rval=0; else rval=(int)cobj2->val->d.str[0];
		nsp_setnum(N, &N->r, "", rval); return 0;
	} else if (cobj2->val->d.str==NULL) {
		rval=-(int)cobj1->val->d.str[0];
		nsp_setnum(N, &N->r, "", rval); return 0;
	}
	if (nc_strcmp(fname, "cmp")==0) {
		rval=nc_strcmp(cobj1->val->d.str, cobj2->val->d.str);
	} else if (nc_strcmp(fname, "ncmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
		rval=nc_strncmp(cobj1->val->d.str, cobj2->val->d.str, (int)cobj3->val->d.num);
	} else if (nc_strcmp(fname, "icmp")==0) {
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			s1++; s2++;
		} while (*s1!=0);
	} else if (nc_strcmp(fname, "nicmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str; i=(int)cobj3->val->d.num;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			if (--i<1) break;
			s1++; s2++;
		} while (*s1!=0);
	}
	nsp_setnum(N, &N->r, "", rval);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strjoin)
{
#define __FN__ __FILE__ ":nl_strjoin()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj, *robj;
	char *p=NULL, *p2;
	int len1=0, len2=0;
	int cnt=0;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type==NT_STRING&&cobj2->val->size>0) { p=cobj2->val->d.str; len2=cobj2->val->size; }
	for (cobj=cobj1->val->d.table.f;cobj;cobj=cobj->next) {
		if (cobj->val->type==NT_STRING) {
			len1+=cobj->val->size;
			cnt++;
		} else if (cobj->val->type==NT_NUMBER) {
			len1++;
			cnt++;
		}
	}
	robj=nsp_setstr(N, &N->r, "", NULL, 0);
	if (cnt==0) return 0;
	if (cnt>1) {
		robj->val->size=len1+((cnt-1)*len2);
		robj->val->d.str=(char *)n_alloc(N, robj->val->size+1, 0);
		p2=robj->val->d.str;
		for (cobj=cobj1->val->d.table.f;cobj;cobj=cobj->next) {
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
		robj->val->d.str=(char *)n_alloc(N, robj->val->size+1, 0);
		for (cobj=cobj1->val->d.table.f;cobj;cobj=cobj->next) {
			if (cobj->val->type==NT_STRING) {
				nc_memcpy(robj->val->d.str, cobj->val->d.str, cobj->val->size);
				break;
			}
		}
		robj->val->d.str[robj->val->size]=0;
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strlen)
{
#define __FN__ __FILE__ ":nl_strlen()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nsp_setnum(N, &N->r, "", cobj1->val->size);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strrep)
{
#define __FN__ __FILE__ ":nl_strrep()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj3=nsp_getobj(N, &N->l, "3");
	char *ss, *se;
	char *s2;
	int l2;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str==NULL) {
		return 0;
	}
	if (cobj2->val->d.str==NULL) {
		nsp_strcat(N, &N->r, cobj1->val->d.str, cobj1->val->size);
		return 0;
	}
	se=ss=cobj1->val->d.str;
	s2=cobj2->val->d.str;
	l2=cobj2->val->size;
	for (;*se;se++) {
		if (nc_strncmp(se, s2, l2)!=0) continue;
		nsp_strcat(N, &N->r, ss, se-ss);
		nsp_strcat(N, &N->r, cobj3->val->d.str, cobj3->val->size);
		ss=se+=l2;
		if (*se) { --se; continue; }
		nsp_strcat(N, &N->r, ss, se-ss);
		break;
	}
	if (se>ss) {
		nsp_strcat(N, &N->r, ss, se-ss);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strsplit)
{
#define __FN__ __FILE__ ":nl_strsplit()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t tobj;
	char *ss, *se;
	int i=0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN+1];

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	tobj.val=n_newval(N, NT_TABLE);
	tobj.val->attr|=NST_AUTOSORT;
	if (cobj1->val->d.str==NULL) {
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	if (cobj2->val->d.str==NULL) {
		for (ss=cobj1->val->d.str;*ss;ss++) {
			nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, 1);
		}
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	se=ss=cobj1->val->d.str;
	s2=cobj2->val->d.str;
	l2=cobj2->val->size;
	for (;*se;se++) {
		if (nc_strncmp(se, s2, l2)!=0) continue;
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
		ss=se+=l2;
		if (*se) { --se; continue; }
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
		break;
	}
	if (se>ss) {
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se-ss);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strstr)
{
#define __FN__ __FILE__ ":nl_strstr()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	char *fname=nsp_getstr(N, &N->l, "0");
	unsigned int i=0, j=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj2->val->size<1) n_error(N, NE_SYNTAX, __FN__, "zero length arg2");
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
		n_error(N, NE_SYNTAX, __FN__, "................");
	}
	if (i<=cobj1->val->size&&j==cobj2->val->size) {
		nsp_setstr(N, &N->r, "", cobj1->val->d.str+i-j, cobj1->val->size-i+j);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strsub)
{
#define __FN__ __FILE__ ":nl_strsub()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj3=nsp_getobj(N, &N->l, "3");
	unsigned int offset, max=0;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	if (cobj2->val->d.num<0) {
		offset=cobj1->val->size-abs((int)cobj2->val->d.num);
	} else {
		offset=(int)cobj2->val->d.num;
	}
	if (cobj3->val->type==NT_NUMBER) max=(int)cobj3->val->d.num; else max=cobj1->val->size;
	if (offset>cobj1->val->size) offset=cobj1->val->size;
	if (max>cobj1->val->size-offset) max=cobj1->val->size-offset;
	nsp_setstr(N, &N->r, "", cobj1->val->d.str+offset, max);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_strtolower)
{
#define __FN__ __FILE__ ":nl_strtolower()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *robj;
	char *fname=nsp_getstr(N, &N->l, "0");
	int i;

	settrace();
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	robj=nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str!=NULL) {
		robj->val->size=cobj1->val->size;
		robj->val->d.str=(char *)n_alloc(N, robj->val->size+1, 0);
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
#undef __FN__
}

NSP_FUNCTION(nl_sqltime)
{
#define __FN__ __FILE__ ":nl_sqltime()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	char *fname=nsp_getstr(N, &N->l, "0");
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
	nsp_setstr(N, &N->r, "", timebuf, -1);
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
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	char *fname=nsp_getstr(N, &N->l, "0");
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
		nsp_setnum(N, &N->r, "", 0);
		return 0;
	}
	if (tp->tm_year<1900) tp->tm_year+=1900;
	nsp_setnum(N, &tobj, "tm_sec",   tp->tm_sec);
	nsp_setnum(N, &tobj, "tm_min",   tp->tm_min);
	nsp_setnum(N, &tobj, "tm_hour",  tp->tm_hour);
	nsp_setnum(N, &tobj, "tm_mday",  tp->tm_mday);
	nsp_setnum(N, &tobj, "tm_mon",   tp->tm_mon);
	nsp_setnum(N, &tobj, "tm_year",  tp->tm_year);
	nsp_setnum(N, &tobj, "tm_wday",  tp->tm_wday);
	nsp_setnum(N, &tobj, "tm_yday",  tp->tm_yday);
	nsp_setnum(N, &tobj, "tm_isdst", tp->tm_isdst);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;	
#undef __FN__
}

NSP_FUNCTION(nl_gettimeofday)
{
#define __FN__ __FILE__ ":nl_gettimeofday()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t tobj;
	time_t t;
	struct timeval ttime;

	settrace();
	if (cobj1->val->type==NT_NUMBER) {
		t=(time_t)cobj1->val->d.num;
	} else {
		t=time(NULL);
	}
	tobj.val=n_newval(N, NT_TABLE);
	tobj.val->attr&=~NST_AUTOSORT;
	nc_gettimeofday(&ttime, NULL);
	nsp_setnum(N, &tobj, "tv_sec",  ttime.tv_sec);
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
	totaltime=ttime.tv_sec-N->ttime.tv_sec+(num_t)(ttime.tv_usec-N->ttime.tv_usec)/1000000;
	nsp_setnum(N, &N->r, "", totaltime);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_sleep)
{
#define __FN__ __FILE__ ":nl_sleep()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
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
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_eval)
{
#define __FN__ __FILE__ ":nl_eval()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	char *fname=nsp_getstr(N, &N->l, "0");
	uchar *p;
	int psize;

	settrace();
	if (nc_strcmp(fname, "eval")!=0) {
		n_error(N, NE_SYNTAX, __FN__, "cowardly refusing to run aliased eval");
	}
	if ((cobj1->val->type==NT_STRING)&&(cobj1->val->d.str!=NULL)) {
		uchar *oldbptr=N->blockptr;
		uchar *oldbend=N->blockend;
		uchar *oldrptr=N->readptr;
		jmp_buf *savjmp;

		n_decompose(N, (uchar *)cobj1->val->d.str, &p, &psize);
		if (p) N->blockptr=p;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));

		savjmp=N->savjmp;
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*N->savjmp)==0) {
			nsp_linkval(N, &N->r, nsp_eval(N, (char *)N->readptr));
		}
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
		N->savjmp=savjmp;

		if (p) n_free(N, (void *)&p, psize);

		N->blockptr=oldbptr;
		N->blockend=oldbend;
		N->readptr=oldrptr;
	} else {
		nsp_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_exec)
{
#define __FN__ __FILE__ ":nl_exec()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	char *fname=nsp_getstr(N, &N->l, "0");
	uchar *p;
	int psize;

	settrace();
	if (nc_strcmp(fname, "exec")!=0) {
		n_error(N, NE_SYNTAX, __FN__, "cowardly refusing to run aliased exec");
	}
	if ((cobj1->val->type==NT_STRING)&&(cobj1->val->d.str!=NULL)) {
		uchar *oldbptr=N->blockptr;
		uchar *oldbend=N->blockend;
		uchar *oldrptr=N->readptr;
		jmp_buf *savjmp;

		n_decompose(N, (uchar *)cobj1->val->d.str, &p, &psize);
		if (p) N->blockptr=p;
		N->blockend=N->blockptr+readi4((N->blockptr+8));
		N->readptr=N->blockptr+readi4((N->blockptr+12));

		savjmp=N->savjmp;
		N->savjmp=(jmp_buf *)n_alloc(N, sizeof(jmp_buf), 0);
		if (setjmp(*N->savjmp)==0) {
			nsp_linkval(N, &N->r, nsp_exec(N, (char *)N->readptr));
		}
		n_free(N, (void *)&N->savjmp, sizeof(jmp_buf));
		N->savjmp=savjmp;

		if (p) n_free(N, (void *)&p, psize);

		N->blockptr=oldbptr;
		N->blockend=oldbend;
		N->readptr=oldrptr;
	} else {
		nsp_linkval(N, &N->r, cobj1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_iname)
{
#define __FN__ __FILE__ ":nl_iname()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	cobj=nsp_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nsp_isnull(cobj)) {
		nsp_setnum(N, &N->r, "", 0);
	} else {
		nsp_setstr(N, &N->r, "", cobj->name, -1);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_ival)
{
#define __FN__ __FILE__ ":nl_ival()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj;

	settrace();
	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	cobj=nsp_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (!nsp_isnull(cobj)) nsp_linkval(N, &N->r, cobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_include)
{
#define __FN__ __FILE__ ":nl_include()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	uchar *p;
	int n=0;

	settrace();
	if (!nsp_isnull(cobj1)) {
		p=N->readptr;
		n=nsp_execfile(N, (char *)cobj1->val->d.str);
		N->readptr=p;
	}
	nsp_setbool(N, &N->r, "", n?0:1);
	return n;
#undef __FN__
}

NSP_FUNCTION(nl_exportvar)
{
#define __FN__ __FILE__ ":nl_exportvar()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");

	settrace();
	if (!nsp_isnull(cobj1)) {
		n_dumpvars(N, cobj1, 0);
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_sizeof)
{
#define __FN__ __FILE__ ":nl_sizeof()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	int size=0;

	settrace();
	if (nsp_isnull(cobj1)) {
		nsp_setnum(N, &N->r, "", 0);
		return 0;
	}
	switch (cobj1->val->type) {
	case NT_BOOLEAN :
	case NT_NUMBER  : size=1; break;
	case NT_STRING  :
	case NT_NFUNC   : size=cobj1->val->size; break;
	case NT_CFUNC   : size=1; break;
	case NT_TABLE   :
		for (cobj1=cobj1->val->d.table.f;cobj1;cobj1=cobj1->next) {
			if (cobj1->val->attr&NST_SYSTEM) { size--; continue; }
			if (!nsp_isnull(cobj1)) size++;
		}
		break;
	case NT_CDATA   : size=cobj1->val->size; break;
	}
	nsp_setnum(N, &N->r, "", size);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_typeof)
{
#define __FN__ __FILE__ ":nl_typeof()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	NSP_CDATA *chead;
	char *p;

	settrace();
	switch (cobj1->val->type) {
	case NT_BOOLEAN : p="boolean";  break;
	case NT_NUMBER  : p="number";   break;
	case NT_STRING  : p="string";   break;
	case NT_TABLE   : p="table";    break;
	case NT_NFUNC   :
	case NT_CFUNC   : p="function"; break;
	case NT_CDATA   :
		if ((chead=(NSP_CDATA *)cobj1->val->d.cdata)!=NULL) {
			p=chead->obj_type; break;
		}
	default         : p="null";     break;
	}
	nsp_setstr(N, &N->r, "", p, -1);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_system)
{
#define __FN__ __FILE__ ":nl_system()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	int n=-1;

	settrace();
	if (cobj1->val->type==NT_STRING&&cobj1->val->d.str!=NULL) {
		nl_flush(N);
		n=system(cobj1->val->d.str);
	}
	nsp_setnum(N, &N->r, "", n);
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_copy)
{
#define __FN__ __FILE__ ":nl_copy()"
	settrace();
	n_copyval(N, &N->r, nsp_getobj(N, &N->l, "1"));
	return 0;
#undef __FN__
}

NSP_FUNCTION(nl_zlink)
{
#define __FN__ __FILE__ ":nl_zlink()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	obj_t *cobj2=nsp_getobj(N, &N->l, "2");
	obj_t *cobj=NULL;
	int i=0;

	settrace();
	if (cobj1->val->ztable==NULL) {
		cobj1->val->ztable=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj1->val->ztable->prev=NULL;
		cobj1->val->ztable->next=NULL;
		cobj1->val->ztable->val=n_newval(N, NT_TABLE);
		n_setname(N, cobj1->val->ztable, "z");
	}
	if (cobj1->val->ztable->val->d.table.f==NULL) {
		cobj=cobj1->val->ztable->val->d.table.f=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj->prev=NULL;
		cobj->next=NULL;
		cobj->val=NULL;
		n_setnamei(N, cobj, i);
		cobj1->val->ztable->val->d.table.l=cobj;
	} else {
		for (cobj=cobj1->val->ztable->val->d.table.f; cobj; cobj=cobj->next, i++) {
			if (cobj->next!=NULL) continue;
			cobj->next=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
			cobj->next->prev=cobj;
			cobj->next->next=NULL;
			cobj=cobj->next;
			cobj->val=NULL;
			n_setnamei(N, cobj, i);
			cobj1->val->ztable->val->d.table.l=cobj;
			break;
		}
	}
	nsp_linkval(N, cobj, cobj2);
	return 0;
#undef __FN__
}
