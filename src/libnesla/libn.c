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
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#ifdef __TURBOC__
#else
#include <unistd.h>
#endif
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
	char *dst=N->outbuf;
	int i, n=N->outbuflen;

	for (i=0;i<len;i++) {
		if (MAX_OUTBUFLEN-n<32) { nl_flush(N); continue; }
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
}

static void n_dumpvars(nes_state *N, obj_t *tobj, int depth)
{
	obj_t *cobj=tobj;
	int i;
	char b;
	char *g;
	int ent=0;

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
}

/*
 * basic i/o functions
 */
/* windows is retarded..  from unistd.. */
NES_FUNCTION(nl_flush)
{
	obj_t *cobj=nes_getobj(N, &N->g, "io");
	NES_CFUNC cfunc=(NES_CFUNC)nl_flush;
	int rc;

	if (cobj->val->type!=NT_TABLE) {
		nes_setnum(N, &N->r, "", -1);
		return -1;
	}
	cobj=nes_getobj(N, cobj, "flush");
	if (cobj->val->type==NT_CFUNC) {
		cfunc=cobj->val->d.cfunc;
		if (cfunc!=(NES_CFUNC)nl_flush) {
			rc=cfunc(N);
			nes_setnum(N, &N->r, "", rc);
			return rc;
		}
	}
	N->outbuf[N->outbuflen]='\0';
	write(STDOUT_FILENO, N->outbuf, N->outbuflen);
	N->outbuflen=0;
	/* do NOT touch &N->r */
	return 0;
}

static int writestr(nes_state *N, obj_t *cobj)
{
	unsigned int i;
	unsigned int len=0;
	char *p;

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
}

NES_FUNCTION(nl_print)
{
	obj_t *cobj=N->l.val->d.table;
	int tlen=0;

	for (cobj=cobj->next;cobj;cobj=cobj->next) {
		if (cobj->name[0]!='n') tlen+=writestr(N, cobj);
	}
	if (N->debug) nl_flush(N);
	nes_setnum(N, &N->r, "", tlen);
	return 0;
}

NES_FUNCTION(nl_write)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int len;

	len=writestr(N, cobj1);
	nes_setnum(N, &N->r, "", len);
	return 0;
}

/*
 * file functions
 */
NES_FUNCTION(nl_fileread)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset=0;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
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
	for (;;) {
		r=read(fd, p, bl);
		p+=r;
		bl-=r;
		if (bl<1) break;
	}
	close(fd);
	robj->val->d.str[sb.st_size]='\0';
	return 0;
}

NES_FUNCTION(nl_filestat)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym=0;
	char *file;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	file=cobj1->val->d.str;
#ifdef WIN32
	rc=stat(file, &sb);
	if (rc!=0) {
		nes_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
#ifdef __TURBOC__
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
}

NES_FUNCTION(nl_fileunlink)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int rc=-1;
	int i;

	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	for (i=1;;i++) {
		cobj1=nes_getiobj(N, &N->l, i);
		if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) break;
		rc=unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nes_setnum(N, &N->r, "", rc);
	return 0;
}

NES_FUNCTION(nl_filewrite)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	int fd;
	int w=0;
	int offset=0;

	/* umask(022); */
	if (cobj1->val->type!=NT_STRING||cobj1->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if ((fd=open(cobj1->val->d.str, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE))==-1) {
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
}

/*
 * math functions
 */
NES_FUNCTION(nl_math1)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	num_t n;

	if (nc_strcmp(cobj0->val->d.str, "rand")==0) {
		n=rand();
		if (cobj1->val->type==NT_NUMBER&&(int)cobj1->val->d.num) {
			n=(int)n%(int)cobj1->val->d.num;
		}
		n=(int)n%65535;
		nes_setnum(N, &N->r, "", n);
		return 0;
	}
	if (cobj1->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number");
	if (nc_strcmp(cobj0->val->d.str, "abs")==0) {
		n=fabs(cobj1->val->d.num);
	} else if (nc_strcmp(cobj0->val->d.str, "ceil")==0) {
		n=cobj1->val->d.num;
		if ((int)n<n) n=(int)n+1; else n=(int)n;
	} else if (nc_strcmp(cobj0->val->d.str, "floor")==0) {
		n=(int)cobj1->val->d.num;
	} else {
		n=0;
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
}

NES_FUNCTION(nl_tonumber)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	nes_setnum(N, &N->r, "", nes_tonum(N, cobj1));
	return 0;
}

NES_FUNCTION(nl_tostring)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	unsigned short d;
	char *p;

	if (cobj1->val->type==NT_NUMBER&&cobj2->val->type==NT_NUMBER) {
		d=(unsigned short)cobj2->val->d.num;
		if (d>sizeof(N->numbuf)-2) d=sizeof(N->numbuf)-2;
		p=n_ntoa(N, N->numbuf, cobj1->val->d.num, 10, d);
	} else {
		p=nes_tostr(N, cobj1);
	}
	nes_setstr(N, &N->r, "", p, p?nc_strlen(p):0);
	return 0;
}

/*
 * string functions
 */
NES_FUNCTION(nl_strcat)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *robj;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	robj->val->size=cobj1->val->size+cobj2->val->size;
	robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
	nc_memcpy(robj->val->d.str, cobj1->val->d.str, cobj1->val->size);
	nc_memcpy(robj->val->d.str+cobj1->val->size, cobj2->val->d.str, cobj2->val->size);
	robj->val->d.str[robj->val->size]=0;
	return 0;
}

NES_FUNCTION(nl_strcmp)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	uchar *s1, *s2;
	int i, rval=0;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	if (cobj1->val->d.str==NULL) {
		if (cobj2->val->d.str==NULL) rval=0; else rval=(int)cobj2->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	} else if (cobj2->val->d.str==NULL) {
		rval=-(int)cobj1->val->d.str[0];
		nes_setnum(N, &N->r, "", rval); return 0;
	}
	if (nc_strcmp(cobj0->val->d.str, "cmp")==0) {
		rval=nc_strcmp(cobj1->val->d.str, cobj2->val->d.str);
	} else if (nc_strcmp(cobj0->val->d.str, "ncmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg3");
		rval=nc_strncmp(cobj1->val->d.str, cobj2->val->d.str, (int)cobj3->val->d.num);
	} else if (nc_strcmp(cobj0->val->d.str, "icmp")==0) {
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			s1++; s2++;
		} while (*s1!=0);
	} else if (nc_strcmp(cobj0->val->d.str, "nicmp")==0) {
		if (cobj3->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg3");
		s1=(uchar *)cobj1->val->d.str; s2=(uchar *)cobj2->val->d.str; i=(int)cobj3->val->d.num;
		do {
			if ((rval=(nc_tolower(*s1)-nc_tolower(*s2)))!=0) break;
			if (--i<1) break;
			s1++; s2++;
		} while (*s1!=0);
	}
	nes_setnum(N, &N->r, "", rval);
	return 0;
}

NES_FUNCTION(nl_strjoin)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj, *robj;
	char *p=NULL, *p2;
	int len1=0, len2=0;
	int cnt=0;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
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
}

NES_FUNCTION(nl_strlen)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	nes_setnum(N, &N->r, "", cobj1->val->size);
	return 0;
}

NES_FUNCTION(nl_strsplit)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t tobj;
	char *ss, *se;
	int i=0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN+1];

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
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
}

NES_FUNCTION(nl_strstr)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	unsigned int i=0, j=0;

	nes_unlinkval(N, &N->r);
	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg2");
	if (cobj2->val->size<1) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "zero length arg2");
	if (nc_strcmp(cobj0->val->d.str, "str")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (cobj2->val->d.str[j]!=cobj1->val->d.str[i]) j=-1;
		}
	} else if (nc_strcmp(cobj0->val->d.str, "istr")==0) {
		for (i=0,j=0;i<cobj1->val->size;i++,j++) {
			if (j==cobj2->val->size) break;
			if (cobj2->val->d.str[j]=='\0') { j=0; break; }
			if (nc_tolower(cobj2->val->d.str[j])!=nc_tolower(cobj1->val->d.str[i])) j=-1;
		}
	} else {
		n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "................");
	}
	if (i<=cobj1->val->size&&j==cobj2->val->size) {
		nes_setstr(N, &N->r, "", cobj1->val->d.str+i-j, cobj1->val->size-i+j);
	}
	return 0;
}

NES_FUNCTION(nl_strsub)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	unsigned int offset, max=0;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
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
}

NES_FUNCTION(nl_strtolower)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *robj;
	int i;

	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a string for arg1");
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str!=NULL) {
		robj->val->size=cobj1->val->size;
		robj->val->d.str=n_alloc(N, robj->val->size+1, 0);
		robj->val->d.str[0]=0;
		i=robj->val->size-1;
		if (nc_strcmp(cobj0->val->d.str, "tolower")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_tolower(cobj1->val->d.str[i]);
		} else if (nc_strcmp(cobj0->val->d.str, "toupper")==0) {
			for (;i>-1;i--) robj->val->d.str[i]=nc_toupper(cobj1->val->d.str[i]);
		}
		robj->val->d.str[robj->val->size]=0;
	}
	return 0;
}

NES_FUNCTION(nl_sqltime)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	struct timeval ttime;
	char timebuf[16];

	if (cobj1->val->type==NT_NUMBER) {
		ttime.tv_sec=(time_t)cobj1->val->d.num;
	} else {
		nc_gettimeofday(&ttime, NULL);
	}
	if (nc_strcmp(cobj0->val->d.str, "sqldate")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%Y-%m-%d", localtime((time_t *)&ttime.tv_sec));
	} else if (nc_strcmp(cobj0->val->d.str, "sqltime")==0) {
		strftime(timebuf, sizeof(timebuf)-1, "%H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	}
	nes_setstr(N, &N->r, "", timebuf, -1);
	return 0;	
}

NES_FUNCTION(nl_time)
{
	nes_setnum(N, &N->r, "", time(NULL));
	return 0;	
}

NES_FUNCTION(nl_gmtime)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t tobj;
	time_t t;
	struct tm *tp;

	if (cobj1->val->type==NT_NUMBER) {
		t=(time_t)cobj1->val->d.num;
	} else {
		t=time(NULL);
	}
	tobj.val=n_newval(N, NT_TABLE);
	tobj.val->attr&=~NST_AUTOSORT;
	if (nc_strcmp(cobj0->val->d.str, "gmtime")==0) {
		tp=gmtime(&t);
	} else if (nc_strcmp(cobj0->val->d.str, "localtime")==0) {
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
}

NES_FUNCTION(nl_runtime)
{
	struct timeval ttime;
	int totaltime;

	nc_gettimeofday(&ttime, NULL);
	totaltime=((ttime.tv_sec-N->ttime.tv_sec)*1000000)+(ttime.tv_usec-N->ttime.tv_usec);
	nes_setnum(N, &N->r, "", (num_t)totaltime/1000000);
	return 0;
}

NES_FUNCTION(nl_sleep)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=1;

	if (cobj1->val->type==NT_NUMBER) n=(int)cobj1->val->d.num;
#ifdef WIN32
	Sleep(n*1000);
#else
	sleep(n);
#endif
	nes_setnum(N, &N->r, "", 0);
	return 0;
}

NES_FUNCTION(nl_iname)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nes_isnull(cobj)) {
		nes_setnum(N, &N->r, "", 0);
	} else {
		nes_setstr(N, &N->r, "", cobj->name, -1);
	}
	return 0;
}

NES_FUNCTION(nl_ival)
{
	obj_t *cobj0=nes_getiobj(N, &N->l, 0);
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj;

	if (cobj1->val->type!=NT_TABLE) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a table for arg1");
	if (cobj2->val->type!=NT_NUMBER) n_error(N, NE_SYNTAX, nes_tostr(N, cobj0), "expected a number for arg2");
	cobj=nes_getiobj(N, cobj1, (int)cobj2->val->d.num);
	if (nes_isnull(cobj)) {
		nes_unlinkval(N, &N->r);
	} else {
		nes_linkval(N, &N->r, cobj);
	}
	return 0;
}

NES_FUNCTION(nl_include)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	uchar *p;
	int n=0;

	if (!nes_isnull(cobj1)) {
		p=N->readptr;
		n=nes_execfile(N, (char *)cobj1->val->d.str);
		N->readptr=p;
	}
	nes_setnum(N, &N->r, "", n);
	return n;
}

NES_FUNCTION(nl_printvar)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);

	if (!nes_isnull(cobj1)) {
		n_dumpvars(N, cobj1, 0);
		nes_setnum(N, &N->r, "", 0);
	}
	return 0;
}

NES_FUNCTION(nl_sizeof)
{
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	int size=0;

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
}

NES_FUNCTION(nl_typeof)
{
	obj_t *cobj=nes_getiobj(N, &N->l, 1);
	NES_CDATA *chead;
	char *p;

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
}

NES_FUNCTION(nl_system)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	int n=-1;

	if (cobj1->val->type==NT_STRING&&cobj1->val->d.str!=NULL) {
		nl_flush(N);
		n=system(cobj1->val->d.str);
	}
	nes_setnum(N, &N->r, "", n);
	return 0;
}
