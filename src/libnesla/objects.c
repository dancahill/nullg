/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2009 Dan Cahill

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
#include "nesla/libnesla.h"
#endif
#include <stdlib.h>
//#include <sys/time.h>

static val_t __null = { NT_NULL, 0, 1, 0, NULL, { 0 } };
static obj_t _null = { NULL, NULL, &__null, 0, "" };
/* static obj_t _null = { NULL, NULL, NULL, 0, "" }; */

//#define RECORD_MEMUSAGE 1

//#define TIMER1 1
//#define TIMER2 1
//#define TIMER3 1

#define starttimer() 	struct timeval t1, t2; \
	gettimeofday(&t1, NULL);
#define endtimer()		gettimeofday(&t2, NULL); \
	if (N) N->counter1+=(t2.tv_sec-t1.tv_sec)*1000000+t2.tv_usec-t1.tv_usec;

void *n_alloc(nes_state *N, int size, short zero)
{
#define __FUNCTION__ __FILE__ ":n_alloc()"
	void *p;
#ifdef TIMER1
	starttimer();
#endif
	settrace();
	if ((p=zero?calloc(1, size):malloc(size))==NULL) n_error(N, NE_MEM, __FUNCTION__, "can't alloc %d bytes", size);
#ifdef RECORD_MEMUSAGE
	if (N) {
		N->allocs++;
		N->allocmem+=size;
		/* printf("%s   s=%4d 0x%08X\r\n", __FUNCTION__, size, p); */
		if (N->allocmem-N->freemem>N->peakmem) N->peakmem=N->allocmem-N->freemem;
	}
#endif
#ifdef TIMER1
	endtimer();
#endif
	return p;
#undef __FUNCTION__
}

void *n_realloc(nes_state *N, void **p, int size, int osize, short zero)
{
#define __FUNCTION__ __FILE__ ":n_realloc()"
	void *p2;
#ifdef TIMER2
	starttimer();
#endif
	settrace();
	if ((p2=realloc(*p, size))==NULL) n_error(N, NE_MEM, __FUNCTION__, "can't realloc %d bytes", size);
#ifdef RECORD_MEMUSAGE
	N->allocmem+=size-osize;
	if (size<osize) n_warn(N, __FUNCTION__, "realloc size<osize");
	/* printf("%s s=%4d o=%4d 0x%08X 0x%08X\r\n", __FUNCTION__, size, osize, *p, p2); */
#endif
#ifdef TIMER2
	endtimer();
#endif
	return p2;
#undef __FUNCTION__
}

void n_free(nes_state *N, void **p, int osize)
{
#define __FUNCTION__ __FILE__ ":n_free()"
#ifdef TIMER3
	starttimer();
#endif
	settrace();
	if (!*p) n_error(N, NE_MEM, __FUNCTION__, "freeing 0x%08X twice", *p);
#ifdef RECORD_MEMUSAGE
	if (N) {
		N->frees++;
		N->freemem+=osize;
		/* if (osize<1) n_warn(N, __FUNCTION__, "suspect osize in n_free of %d", osize); */
	}
#endif
	/* printf("%s    o=%4d 0x%08X\r\n", __FUNCTION__, osize, *p); */
	free(*p);
	*p=NULL;
#ifdef TIMER3
	endtimer();
#endif
	return;
#undef __FUNCTION__
}

void n_copytable(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FUNCTION__ __FILE__ ":n_copytable()"
	obj_t *cobj, *iobj, *oobj=NULL;

	if (!nes_istable(cobj2)) return;
	for (iobj=cobj2->val->d.table.f; iobj; iobj=iobj->next) {
		cobj=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
		if (oobj!=NULL) {
			oobj->next=cobj;
			cobj->prev=oobj;
		} else {
			cobj1->val->d.table.f=cobj;
			cobj->prev=NULL;
		}
		cobj->next=NULL;
		n_setname(N, cobj, iobj->name);
		n_copyval(N, cobj, iobj);
		oobj=cobj;
	}
	cobj1->val->d.table.l=oobj;
	return;
#undef __FUNCTION__
}

void n_copyval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FUNCTION__ __FILE__ ":n_copyval()"
	unsigned short type2=nes_typeof(cobj2);

	settrace();
	if (cobj1==cobj2) return;
	if (cobj1->val) nes_unlinkval(N, cobj1);
	cobj1->val=n_newval(N, type2);
	switch (type2) {
	case NT_NULL    : return;
	case NT_BOOLEAN :
	case NT_NUMBER  : cobj1->val->d.num=cobj2->val->d.num; break;
	case NT_TABLE   : n_copytable(N, cobj1, cobj2); break;
	case NT_CDATA   : n_warn(N, __FUNCTION__, "copying cdata '%s'?", cobj2->val->d.str);
	case NT_STRING  :
	case NT_NFUNC   : nes_strcat(N, cobj1, cobj2->val->d.str, cobj2->val->size); break;
	case NT_CFUNC   : cobj1->val->d.cfunc=cobj2->val->d.cfunc; break;
	default: n_error(N, NE_SYNTAX, __FUNCTION__, "unhandled object type");
	}
	return;
#undef __FUNCTION__
}

void n_freeval(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":n_freeval()"
	NES_CDATA *chead;
	NES_CFREE cfunc;

	settrace();
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return;
	case NT_BOOLEAN :
	case NT_NUMBER  : break;
	case NT_CDATA   :
		/* try to call this object's reaper function before freeing the data */
		if ((chead=(NES_CDATA *)cobj->val->d.cdata)!=NULL) {
			if ((cfunc=chead->obj_term)!=NULL) cfunc(N, cobj);
		}
	case NT_STRING  :
	case NT_NFUNC   :
		if (cobj->val->attr&NST_LINK) {
			cobj->val->attr^=NST_LINK;
			cobj->val->d.str=NULL;
		} else if (cobj->val->d.str!=NULL) {
			n_free(N, (void *)&cobj->val->d.str, cobj->val->size+1);
		}
		break;
	case NT_TABLE   : nes_freetable(N, cobj); break;
	}
	cobj->val->type=NT_NULL;
	cobj->val->size=0;

	if (cobj->val->ztable!=NULL) {
//		nes_freetable(N, cobj->val->ztable);
		nes_unlinkval(N, cobj->val->ztable);
		n_free(N, (void *)&cobj->val->ztable, sizeof(obj_t));
	}

	return;
#undef __FUNCTION__
}

val_t *n_newval(nes_state *N, unsigned short type)
{
#define __FUNCTION__ __FILE__ ":n_newval()"
	val_t *val=(val_t *)n_alloc(N, sizeof(val_t), 0);

	settrace();
	val->type=type;
	val->attr=0;
	val->refs=1;
	val->size=0;
	val->ztable=NULL;
	val->d.num=0;
	return val;
#undef __FUNCTION__
}

obj_t *n_newiobj(nes_state *N, int index)
{
#define __FUNCTION__ __FILE__ ":n_newiobj()"
	obj_t *obj=(obj_t *)n_alloc(N, sizeof(obj_t), 0);

	settrace();
	obj->prev=NULL;
	obj->next=NULL;
	obj->val=NULL;
	n_setnamei(N, obj, index);
	return obj;
#undef __FUNCTION__
}

static unsigned long cdb_hash(char *p)
{
	unsigned long hash=5381;

	while (*p) hash=(hash+(hash<<5))^*p++;
	return hash;
}

obj_t *n_setname(nes_state *N, obj_t *cobj, const char *name)
{
#define __FUNCTION__ __FILE__ ":n_setname()"
	nc_strncpy(cobj->name, name, MAX_OBJNAMELEN);
	cobj->hash=cdb_hash(cobj->name);
	return cobj;
#undef __FUNCTION__
}

obj_t *n_setnamei(nes_state *N, obj_t *cobj, unsigned long i)
{
#define __FUNCTION__ __FILE__ ":n_setname()"
	n_ntoa(N, cobj->name, i, 10, 0);
	cobj->hash=cdb_hash(cobj->name);
	return cobj;
#undef __FUNCTION__
}

/*
 * the following functions are public API functions
 */

void nes_setvaltype(nes_state *N, obj_t *cobj, unsigned short type)
{
#define __FUNCTION__ __FILE__ ":nes_setvaltype()"
	settrace();
	if (cobj->val==NULL) {
		cobj->val=(val_t *)n_alloc(N, sizeof(val_t), 0);
	} else if (cobj->val->refs<2) {
		n_freeval(N, cobj);
	} else {
		nes_unlinkval(N, cobj);
		cobj->val=(val_t *)n_alloc(N, sizeof(val_t), 0);
	}
	cobj->val->type=type;
	cobj->val->attr=0;
	cobj->val->refs=1;
	cobj->val->size=0;
	cobj->val->ztable=NULL;
	cobj->val->d.num=0;
	return;
#undef __FUNCTION__
}

void nes_linkval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FUNCTION__ __FILE__ ":nes_linkval()"
	settrace();
	if (cobj1==NULL||cobj1==cobj2) return;
	if (cobj2==NULL||cobj2->val==NULL) {
		nes_unlinkval(N, cobj1);
//		cobj1->val=(val_t *)n_alloc(N, sizeof(val_t), 1);
		cobj1->val=n_newval(N, NT_NULL);
		return;
	} else {
		if (cobj1->val==cobj2->val) return;
		nes_unlinkval(N, cobj1);
		if (cobj2==&N->r) {
			cobj1->val=cobj2->val;
			cobj2->val=NULL;
			return;
		}
		cobj1->val=cobj2->val;
	}
	if (cobj1->val) cobj1->val->refs++;
	return;
#undef __FUNCTION__
}

void nes_unlinkval(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":nes_unlinkval()"
	settrace();
	if (cobj!=NULL&&cobj->val!=NULL) {
		if (--cobj->val->refs<1) {
			n_freeval(N, cobj);
			n_free(N, (void *)&cobj->val, sizeof(val_t));
		}
		cobj->val=NULL;
	}
	return;
#undef __FUNCTION__
}

void nes_freetable(nes_state *N, obj_t *tobj)
{
#define __FUNCTION__ __FILE__ ":nes_freetable()"
	obj_t *cobj, *oobj;

	settrace();
	if (!nes_istable(tobj)) return;
	cobj=tobj->val->d.table.f;
	while (cobj!=NULL) {
		oobj=cobj;
		cobj=cobj->next;
		nes_unlinkval(N, oobj);
		n_free(N, (void *)&oobj, sizeof(obj_t));
	}
	tobj->val->d.table.f=NULL;
	tobj->val->d.table.l=NULL;
	return;
#undef __FUNCTION__
}

obj_t *nes_getobj_ex(nes_state *N, obj_t *tobj, char *oname, unsigned short followz, unsigned short *foundz)
{
#define __FUNCTION__ __FILE__ ":nes_getobj()"
	/* unsigned long hashthis=cdb_hash("this"); */
	static const unsigned long thishash=2087959715;
	unsigned long hash=cdb_hash(oname);
	obj_t *cobj, *cobj2, *thisobj=NULL;

	settrace();
	if (foundz) *foundz=0;
	if (tobj==&N->r) return tobj;
	if (N!=NULL&&tobj==NULL) {
		for (cobj=N->l.val->d.table.f; cobj; cobj=cobj->next) {
			/* printf("%s oname=%s 0x%08X %s\n", __FUNCTION__, oname, cobj, cobj->name); */
			if (cobj->hash==thishash&&nc_strcmp(cobj->name, "this")==0) thisobj=cobj;
			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
		}
		if (nes_istable(thisobj)) {
			for (cobj=thisobj->val->d.table.f; cobj; cobj=cobj->next) {
				if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
			}
		}
		for (cobj=N->g.val->d.table.f; cobj; cobj=cobj->next) {
			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	} else if (nes_istable(tobj)) {
		for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
		}
		if (followz && nes_istable(tobj->val->ztable)) {
			for (cobj=tobj->val->ztable->val->d.table.f; cobj; cobj=cobj->next) {
				if (!nes_istable(cobj)) continue;
				cobj2=nes_getobj_ex(N, cobj, oname, followz, foundz);
				if (!nes_isnull(cobj)) {
					if (foundz) *foundz=1;
					return cobj2;
				}
			}
		}
	}
	return &_null;
#undef __FUNCTION__
}

obj_t *nes_getobj(nes_state *N, obj_t *tobj, char *oname)
{
#define __FUNCTION__ __FILE__ ":nes_getobj()"
	return nes_getobj_ex(N, tobj, oname, 1, NULL);
#undef __FUNCTION__
}

obj_t *nes_getiobj(nes_state *N, obj_t *tobj, unsigned long oindex)
{
#define __FUNCTION__ __FILE__ ":nes_getiobj()"
	obj_t *cobj;
	unsigned long i=0;

	settrace();
	if (!nes_istable(tobj)) return &_null;
	for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
		if (nes_isnull(cobj)||cobj->val->attr&NST_SYSTEM) continue;
		if (i==oindex) return cobj;
		i++;
	}
	return &_null;
#undef __FUNCTION__
}

/* change or create an object and return it */
obj_t *nes_setobj(nes_state *N, obj_t *tobj, char *oname, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, long _slen)
{
#define __FUNCTION__ __FILE__ ":nes_setobj()"
	obj_t *oobj, *cobj;
	char *ostr=NULL;
	int osize=0;
	char *p;
	int cmp=-1;
	unsigned short sortattr;

	settrace();
	if ((tobj==&N->r)||(!nes_istable(tobj))) {
		cobj=tobj;
		nes_unlinkval(N, cobj);
		switch (nes_typeof(cobj)) {
		case NT_NULL   :
			if (cobj->val==NULL) cobj->val=n_newval(N, otype);
			break;
		case NT_STRING :
		case NT_NFUNC  :
			ostr=cobj->val->d.str;
			osize=cobj->val->size;
			cobj->val->d.str=NULL;
			cobj->val->size=0;
		default:
			cobj->val->type=otype;
		}
	} else {
		if (oname[0]=='\0') { return &_null; }
		sortattr=tobj->val->attr&NST_AUTOSORT;
		if (tobj->val->d.table.f==NULL) {
			cobj=tobj->val->d.table.l=tobj->val->d.table.f=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
			cobj->prev=NULL;
			cobj->next=NULL;
			cobj->val=n_newval(N, otype);
			n_setname(N, cobj, oname);
		} else {
			oobj=tobj->val->d.table.f;
			for (cobj=oobj; cobj; oobj=cobj,cobj=cobj->next) {
				/* both strings must be entirely numeric (not even decimals) */
				p=cobj->name; while (nc_isdigit(*p)) p++;
				if (!*p) { p=oname; while (nc_isdigit(*p)) p++; }
				if (!*p) {
					cmp=(int)(n_aton(N, cobj->name)-n_aton(N, oname));
				} else {
					if (cobj->name[0]!=oname[0]) cmp=cobj->name[0]-oname[0];else
					cmp=nc_strcmp(cobj->name, oname);
				}
				if (cmp==0||(cmp>0&&sortattr)) break;
			}
			if (cmp>0&&sortattr) {
				oobj=cobj;
				cobj=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
				if (oobj==tobj->val->d.table.f) tobj->val->d.table.f=cobj;
				cobj->prev=oobj->prev;
				if (cobj->prev) cobj->prev->next=cobj;
				cobj->next=oobj;
				oobj->prev=cobj;
				cobj->val=n_newval(N, otype);
				n_setname(N, cobj, oname);
				if (cobj->next==NULL) tobj->val->d.table.l=cobj;
			} else if (cobj==NULL) {
				cobj=(obj_t *)n_alloc(N, sizeof(obj_t), 0);
				if (oobj!=NULL) {
					oobj->next=cobj;
					cobj->prev=oobj;
				} else {
					cobj->prev=NULL;
				}
				cobj->next=NULL;
				cobj->val=n_newval(N, otype);
				n_setname(N, cobj, oname);
				if (cobj->next==NULL) tobj->val->d.table.l=cobj;
			} else {
				switch (nes_typeof(cobj)) {
				case NT_NULL   :
					if (cobj->val==NULL) cobj->val=n_newval(N, otype);
					break;
				case NT_STRING :
				case NT_NFUNC  :
					ostr=cobj->val->d.str;
					osize=cobj->val->size;
					cobj->val->d.str=NULL;
					cobj->val->size=0;
				default:
					cobj->val->type=otype;
				}
			}
		}
		if (cobj->val->type==NT_TABLE&&cobj->val->d.table.f==NULL) cobj->val->attr|=sortattr;
	}
	switch (otype) {
	case NT_BOOLEAN : cobj->val->d.num=_num?1:0; break;
	case NT_NUMBER : cobj->val->d.num=_num; break;
	case NT_CFUNC  : cobj->val->d.cfunc=_fptr; break;
	case NT_STRING :
	case NT_NFUNC  :
	case NT_CDATA  : nes_strcat(N, cobj, _str, _slen); break;
	}
	if (ostr) n_free(N, (void *)&ostr, osize+1);
	return cobj;
#undef __FUNCTION__
}

obj_t *nes_setiobj(nes_state *N, obj_t *tobj, int index, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, long _slen)
{
#define __FUNCTION__ __FILE__ ":nes_setiobj()"
	char numbuf[12];

	settrace();
	return nes_setobj(N, tobj, n_ntoa(N, numbuf, index, 10, 0), otype, _fptr, _num, _str, _slen);
#undef __FUNCTION__
}

void nes_strcat(nes_state *N, obj_t *cobj, char *str, long len)
{
#define __FUNCTION__ __FILE__ ":nes_strcat()"
	unsigned short ctype=nes_typeof(cobj);
	unsigned long olen, tlen;
	char *p;

	settrace();
	if (ctype!=NT_STRING&&ctype!=NT_NFUNC&&ctype!=NT_CDATA) return;
	if (len==-1&&str!=NULL) len=nc_strlen(str);
	if (len<1) return;
	olen=cobj->val->size;
	tlen=olen+len;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, olen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, __FUNCTION__, "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	nc_memcpy(cobj->val->d.str+olen, str, len);
	cobj->val->d.str[tlen]=0;
	return;
#undef __FUNCTION__
}

void nes_strmul(nes_state *N, obj_t *cobj, unsigned long n)
{
#define __FUNCTION__ __FILE__ ":nes_strmul()"
	unsigned short ctype=nes_typeof(cobj);
	unsigned long i, olen, tlen;
	char *p;

	settrace();
	if (ctype!=NT_STRING) return;
	if (n<2) return;
	olen=cobj->val->size;
	tlen=olen*n;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, olen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, __FUNCTION__, "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	for (i=1;i<n;i++) nc_memcpy(p+olen*i, cobj->val->d.str, olen);
	cobj->val->d.str[tlen]=0;
	return;
#undef __FUNCTION__
}

short nes_tobool(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":nes_tobool()"
	settrace();
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return 0;
	case NT_BOOLEAN : return cobj->val->d.num?1:0;
	case NT_NUMBER  : return cobj->val->d.num?1:0;
	case NT_STRING  : return cobj->val->size?1:0;
	case NT_TABLE   : return cobj->val->d.table.f?1:0;
	case NT_NFUNC   :
	case NT_CFUNC   : return 1;
	}
	return 0;
#undef __FUNCTION__
}

num_t nes_tonum(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":nes_tonum()"
	settrace();
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return 0;
	case NT_BOOLEAN : return cobj->val->d.num?1:0;
	case NT_NUMBER  : return cobj->val->d.num;
	case NT_STRING  : return n_aton(N,cobj->val->d.str);
	case NT_TABLE   : return 0;
	case NT_NFUNC   :
	case NT_CFUNC   : return 0;
	}
	return 0;
#undef __FUNCTION__
}

char *nes_tostr(nes_state *N, obj_t *cobj)
{
#define __FUNCTION__ __FILE__ ":nes_tostr()"
	settrace();
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return "null";
	case NT_BOOLEAN : return cobj->val->d.num?"true":"false";
	case NT_NUMBER  : return n_ntoa(N,N->numbuf,cobj->val->d.num,-10,6);
	case NT_STRING  : return cobj->val->d.str?cobj->val->d.str:"";
	case NT_TABLE   : return "";
	case NT_NFUNC   :
	case NT_CFUNC   : return "";
	}
	return "";
#undef __FUNCTION__
}
