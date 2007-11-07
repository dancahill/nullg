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
#include <stdlib.h>

static val_t __null = { NT_NULL, 0, 1, 0, { 0 } };
static obj_t _null = { NULL, NULL, &__null, "" };

void *n_alloc(nes_state *N, int size, short zero)
{
	char *p;

	if ((p=zero?calloc(1, size):malloc(size))==NULL) n_error(N, NE_MEM, "n_alloc", "can't alloc %d bytes", size);
	return p;
}

void *n_realloc(nes_state *N, void **p, int size, short zero)
{
	char *p2;

	if ((p2=realloc(*p, size))==NULL) n_error(N, NE_MEM, "n_realloc", "can't realloc %d bytes", size);
	return p2;
}

void n_free(nes_state *N, void **p)
{
	if (*p) { free(*p); *p=NULL; }
//	else n_error(N, NE_MEM, "n_free", "freeing 0x%08X twice", *p); /* pedantic, but useful */
	return;
}

void n_copyval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
	unsigned short type2=nes_typeof(cobj2);

	if (cobj1==cobj2) return;
//	nes_setvaltype(N, cobj1, type2);
	if (cobj1->val) nes_unlinkval(N, cobj1);
	type2=nes_typeof(cobj2);
	cobj1->val=n_newval(N, type2);
	switch (type2) {
	case NT_NULL    : return;
	case NT_BOOLEAN :
	case NT_NUMBER  : cobj1->val->d.num=cobj2->val->d.num; break;
	case NT_CDATA   : n_warn(N, "n_copyval", "copying cdata '%s'?", cobj2->val->d.str);
	case NT_STRING  :
	case NT_NFUNC   : nes_strcat(N, cobj1, cobj2->val->d.str, cobj2->val->size); break;
	case NT_CFUNC   : cobj1->val->d.cfunc=cobj2->val->d.cfunc; break;
	default: n_error(N, NE_SYNTAX, "n_copyval", "unhandled object type");
	}
	return;
}

void n_freeval(nes_state *N, obj_t *cobj)
{
	NES_CDATA *chead;
	NES_CFREE cfunc;

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
			n_free(N, (void *)&cobj->val->d.str);
		}
		break;
	case NT_TABLE   : nes_freetable(N, cobj); break;
	}
	cobj->val->type=NT_NULL;
	cobj->val->size=0;
	return;
}

val_t *n_newval(nes_state *N, unsigned short type)
{
	val_t *val=n_alloc(N, sizeof(val_t), 0);

	val->type=type;
	val->attr=0;
	val->refs=1;
	val->size=0;
	val->d.num=0;
	return val;
}

obj_t *n_newiobj(nes_state *N, int index)
{
	obj_t *obj=n_alloc(N, sizeof(obj_t), 0);

	obj->prev=NULL;
	obj->next=NULL;
	obj->val=NULL;
	n_ntoa(N, obj->name, index, 10, 0);
	return obj;
}

/*
 * the following functions are public API functions
 */

void nes_setvaltype(nes_state *N, obj_t *cobj, unsigned short type)
{
	if (cobj->val==NULL) {
		cobj->val=n_alloc(N, sizeof(val_t), 0);
	} else if (cobj->val->refs<2) {
		n_freeval(N, cobj);
	} else {
		nes_unlinkval(N, cobj);
		cobj->val=n_alloc(N, sizeof(val_t), 0);
	}
	cobj->val->type=type;
	cobj->val->attr=0;
	cobj->val->refs=1;
	cobj->val->size=0;
	cobj->val->d.num=0;
	return;
}

void nes_linkval(nes_state *N, obj_t *cobj1, obj_t *cobj2)
{
	if (cobj1==NULL||cobj1==cobj2) return;
	if (cobj2==NULL||cobj2->val==NULL) {
		nes_unlinkval(N, cobj1);
		cobj1->val=n_alloc(N, sizeof(val_t), 1);
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
}

void nes_unlinkval(nes_state *N, obj_t *cobj)
{
/*
	if (cobj->val==NULL) return;
	switch (cobj->val->refs) {
	case 0 : return;
	case 1 : cobj->val->refs--;n_freeval(N, cobj);n_free(N, (void *)&cobj->val);break;
	default: cobj->val->refs--;cobj->val=NULL;break;
//	default: cobj->val->refs--;break;
	}
*/
	if (cobj==NULL||cobj->val==NULL) return;
	if (--cobj->val->refs<1) {
		n_freeval(N, cobj);
		n_free(N, (void *)&cobj->val);
	}
	return;
}

void nes_freetable(nes_state *N, obj_t *tobj)
{
	obj_t *cobj, *oobj;

	if (!nes_istable(tobj)) return;
	cobj=tobj->val->d.table;
	while (cobj!=NULL) {
		oobj=cobj;
		cobj=cobj->next;
		nes_unlinkval(N, oobj);
		n_free(N, (void *)&oobj);
	}
	tobj->val->d.table=NULL;
	return;
}

obj_t *nes_getobj(nes_state *N, obj_t *tobj, char *oname)
{
	obj_t *cobj;

	if (tobj==&N->r) return tobj;
	if (!nes_istable(tobj)) return &_null;
	if (N==NULL) {
		for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	} else if (tobj==&N->l||tobj==&N->g) {
		for (cobj=N->l.val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
		for (cobj=N->g.val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	} else {
		for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
			if (cobj->name[0]!=oname[0]) continue;
			if (nc_strcmp(cobj->name, oname)==0) return cobj;
		}
	}
	return &_null;
}

obj_t *nes_getiobj(nes_state *N, obj_t *tobj, unsigned long oindex)
{
	obj_t *cobj;
	unsigned long i=0;

	if (!nes_istable(tobj)) return &_null;
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (nes_isnull(cobj)||cobj->val->attr&NST_SYSTEM) continue;
		if (i==oindex) return cobj;
		i++;
	}
	return &_null;
}

/* change or create an object and return it */
obj_t *nes_setobj(nes_state *N, obj_t *tobj, char *oname, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, long _slen)
{
	obj_t *oobj, *cobj;
	char *ostr=NULL;
	char *p;
	int cmp=-1;
	unsigned short sortattr;

	if ((tobj==&N->r)||(!nes_istable(tobj))) {
		cobj=tobj;
		nes_unlinkval(N, cobj);
//		nes_setvaltype(N, cobj, otype);
		switch (nes_typeof(cobj)) {
		case NT_NULL   :
			if (cobj->val==NULL) cobj->val=n_newval(N, otype);
			break;
		case NT_STRING :
		case NT_NFUNC  :
			ostr=cobj->val->d.str;
			cobj->val->d.str=NULL;
			cobj->val->size=0;
		default:
			cobj->val->type=otype;
		}
	} else {
		if (oname[0]=='\0') { return &_null; }
		sortattr=tobj->val->attr&NST_AUTOSORT;
		if (tobj->val->d.table==NULL) {
			cobj=tobj->val->d.table=n_alloc(N, sizeof(obj_t), 0);
			cobj->prev=NULL;
			cobj->next=NULL;
			cobj->val=n_newval(N, otype);
			nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
		} else {
			oobj=tobj->val->d.table;
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
				cobj=n_alloc(N, sizeof(obj_t), 0);
				if (oobj==tobj->val->d.table) tobj->val->d.table=cobj;
				cobj->prev=oobj->prev;
				if (cobj->prev) cobj->prev->next=cobj;
				cobj->next=oobj;
				oobj->prev=cobj;
				cobj->val=n_newval(N, otype);
				nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
			} else if (cobj==NULL) {
				cobj=n_alloc(N, sizeof(obj_t), 0);
				if (oobj!=NULL) {
					oobj->next=cobj;
					cobj->prev=oobj;
				} else {
					cobj->prev=NULL;
				}
				cobj->next=NULL;
				cobj->val=n_newval(N, otype);
				nc_strncpy(cobj->name, oname, MAX_OBJNAMELEN);
			} else {
				switch (nes_typeof(cobj)) {
				case NT_NULL   :
					if (cobj->val==NULL) cobj->val=n_newval(N, otype);
					break;
				case NT_STRING :
				case NT_NFUNC  :
					ostr=cobj->val->d.str;
					cobj->val->d.str=NULL;
					cobj->val->size=0;
				default:
					cobj->val->type=otype;
				}
			}
		}
		if (cobj->val->type==NT_TABLE&&cobj->val->d.table==NULL) cobj->val->attr|=sortattr;
	}
	switch (otype) {
	case NT_NUMBER : cobj->val->d.num=_num; break;
	case NT_CFUNC  : cobj->val->d.cfunc=_fptr; break;
	case NT_STRING :
	case NT_NFUNC  :
	case NT_CDATA  : nes_strcat(N, cobj, _str, _slen); break;
	}
	if (ostr) n_free(N, (void *)&ostr);
	return cobj;
}

obj_t *nes_setiobj(nes_state *N, obj_t *tobj, int index, unsigned short otype, NES_CFUNC _fptr, num_t _num, char *_str, long _slen)
{
	char numbuf[12];

	return nes_setobj(N, tobj, n_ntoa(N, numbuf, index, 10, 0), otype, _fptr, _num, _str, _slen);
}

void nes_strcat(nes_state *N, obj_t *cobj, char *str, long len)
{
	unsigned short ctype=nes_typeof(cobj);
	unsigned long olen, tlen;
	char *p;

	if (ctype!=NT_STRING&&ctype!=NT_NFUNC&&ctype!=NT_CDATA) return;
	if (len==-1&&str!=NULL) len=nc_strlen(str);
	if (len<1) return;
	olen=cobj->val->size;
	tlen=olen+len;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, "nes_strcat", "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	nc_memcpy(cobj->val->d.str+olen, str, len);
	cobj->val->d.str[tlen]=0;
	return;
}

void nes_strmul(nes_state *N, obj_t *cobj, unsigned long n)
{
	unsigned short ctype=nes_typeof(cobj);
	unsigned long i, olen, tlen;
	char *p;

	if (ctype!=NT_STRING) return;
	if (n<2) return;
	olen=cobj->val->size;
	tlen=olen*n;
	p=olen?n_realloc(N, (void *)&cobj->val->d.str, tlen+1, 0):n_alloc(N, tlen+1, 0);
	if (p==NULL) n_error(N, NE_MEM, "nes_strmul", "can't alloc %d bytes", tlen+1);
	cobj->val->size=tlen;
	cobj->val->d.str=p;
	for (i=1;i<n;i++) nc_memcpy(p+olen*i, cobj->val->d.str, olen);
	cobj->val->d.str[tlen]=0;
	return;
}

short nes_tobool(nes_state *N, obj_t *cobj)
{
	switch (nes_typeof(cobj)) {
	case NT_NULL    : return 0;
	case NT_BOOLEAN : return cobj->val->d.num?1:0;
	case NT_NUMBER  : return cobj->val->d.num?1:0;
	case NT_STRING  : return cobj->val->size?1:0;
	case NT_TABLE   : return cobj->val->d.table?1:0;
	case NT_NFUNC   :
	case NT_CFUNC   : return 1;
	}
	return 0;
}

num_t nes_tonum(nes_state *N, obj_t *cobj)
{
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
}

char *nes_tostr(nes_state *N, obj_t *cobj)
{
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
}
