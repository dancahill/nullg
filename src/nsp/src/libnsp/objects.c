/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdlib.h>
#define RECORD_MEMUSAGE 1

static val_t _nullval = { NT_NULL, NST_STACKVAL, 1, 0, NULL, { 0 } };
static obj_t _nullobj = { NULL, NULL, &_nullval, 0, 0, "" };
/* static obj_t _nullobj = { NULL, NULL, NULL, 0, "" }; */

void *n_alloc(nsp_state *N, int size, short zero)
{
#define __FN__ __FILE__ ":n_alloc()"
	void *p;
	settrace();
	if ((p = zero ? calloc(1, size) : malloc(size)) == NULL) {
#if defined(__TURBOC__)
		n_error(N, NE_MEM, __FN__, "can't alloc %d bytes %d 0x%08X", size, stackavail(), coreleft(), _SP);
#else
		n_error(N, NE_MEM, __FN__, "can't alloc %d bytes", size);
#endif
	}
#ifdef RECORD_MEMUSAGE
	if (N) {
		N->allocs++;
		N->allocmem += size;
		/* printf("%s   s=%4d 0x%08X\r\n", __FN__, size, p); */
		if (N->allocmem - N->freemem > N->peakmem) N->peakmem = N->allocmem - N->freemem;
	}
#endif
	return p;
#undef __FN__
}

void *n_realloc(nsp_state *N, void **p, int size, int osize, short zero)
{
#define __FN__ __FILE__ ":n_realloc()"
	void *p2;
	settrace();
	if (size < osize) n_warn(N, __FN__, "realloc size<osize");
	if ((p2 = realloc(*p, size)) == NULL) n_error(N, NE_MEM, __FN__, "can't realloc %d bytes", size);
#ifdef RECORD_MEMUSAGE
	if (N) {
		N->allocmem += size - osize;
		if (N->allocmem - N->freemem > N->peakmem) N->peakmem = N->allocmem - N->freemem;
		/* printf("%s s=%4d o=%4d 0x%08X 0x%08X\r\n", __FN__, size, osize, *p, p2); */
	}
#endif
	return p2;
#undef __FN__
}

void n_free(nsp_state *N, void **p, int osize)
{
#define __FN__ __FILE__ ":n_free()"
	settrace();
	if ((long long)p == 0x10) {
		n_error(N, NE_MEM, __FN__, "ptr is invalid 0x%08X", p);
		return;
	}
	if (!*p) n_error(N, NE_MEM, __FN__, "freeing 0x%08X twice", *p);
#ifdef RECORD_MEMUSAGE
	if (N) {
		N->frees++;
		N->freemem += osize;
	}
#endif
	free(*p);
	*p = NULL;
	return;
#undef __FN__
}

void n_copytable(nsp_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FN__ __FILE__ ":n_copytable()"
	obj_t *cobj, *iobj, *oobj = NULL;

	if (!nsp_istable(cobj2)) return;
	for (iobj = cobj2->val->d.table.f; iobj; iobj = iobj->next) {
		cobj = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		if (oobj != NULL) {
			oobj->next = cobj;
			cobj->prev = oobj;
		} else {
			cobj1->val->d.table.f = cobj;
			cobj->prev = NULL;
		}
		cobj->next = NULL;
		n_setname(N, cobj, iobj->name);
		n_copyval(N, cobj, iobj);
		oobj = cobj;
	}
	cobj1->val->d.table.i = cobj1->val->d.table.f;
	cobj1->val->d.table.l = oobj;
	cobj1->val->attr |= cobj2->val->attr & NST_AUTOSORT;
	return;
#undef __FN__
}

void n_copyval(nsp_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FN__ __FILE__ ":n_copyval()"
	unsigned short type2 = nsp_typeof(cobj2);

	settrace();
	if (cobj1 == cobj2) return;
	if (cobj1->val) {
		n_freeval(N, cobj1);
		cobj1->val->type = type2;
	} else {
		cobj1->val = n_newval(N, type2);
	}
	switch (type2) {
	case NT_NULL: return;
	case NT_BOOLEAN:
	case NT_NUMBER: cobj1->val->d.num = cobj2->val->d.num; break;
	case NT_TABLE: n_copytable(N, cobj1, cobj2); break;
	case NT_CDATA: n_warn(N, __FN__, "copying cdata '%s'?", cobj2->val->d.str);
	case NT_STRING:
	case NT_NFUNC: nsp_strcat(N, cobj1, cobj2->val->d.str, cobj2->val->size); break;
	case NT_CFUNC: cobj1->val->d.cfunc = cobj2->val->d.cfunc; break;
	default: n_error(N, NE_SYNTAX, __FN__, "unhandled object type");
	}
	return;
#undef __FN__
}

void n_moveval(nsp_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FN__ __FILE__ ":n_copyval()"
	unsigned short type2 = nsp_typeof(cobj2);

	settrace();
	if (cobj1 == cobj2) return;
	if (cobj1->val) {
		n_freeval(N, cobj1);
		cobj1->val->type = type2;
	} else {
		cobj1->val = n_newval(N, type2);
	}
	switch (type2) {
	case NT_NULL: return;
	case NT_BOOLEAN:
	case NT_NUMBER: cobj1->val->d.num = cobj2->val->d.num; break;
	case NT_TABLE:
		cobj1->val->d.table.f = cobj2->val->d.table.f;
		cobj1->val->d.table.i = cobj1->val->d.table.f;
		cobj1->val->d.table.l = cobj2->val->d.table.l;
		cobj1->val->attr |= cobj2->val->attr & NST_AUTOSORT;
		break;
	case NT_CDATA:
	case NT_STRING:
	case NT_NFUNC:
		cobj1->val->d.str = cobj2->val->d.str;
		cobj1->val->size = cobj2->val->size;
		break;
	case NT_CFUNC: cobj1->val->d.cfunc = cobj2->val->d.cfunc; break;
	default: n_error(N, NE_SYNTAX, __FN__, "unhandled object type");
	}
	cobj2->val->type = NT_NULL;
	return;
#undef __FN__
}

void n_freeval(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":n_freeval()"
	settrace();
	switch (nsp_typeof(cobj)) {
	case NT_NULL:
		if (cobj->val == &_nullval) n_warn(N, __FN__, "wrong null");
		return;
	case NT_BOOLEAN:
	case NT_NUMBER: break;
	case NT_CDATA: {
		NSP_CDATA *chead = (NSP_CDATA *)cobj->val->d.cdata;
		NSP_CFREE cfunc = chead != NULL ? cfunc = chead->obj_term : NULL;

		/* try to call this object's reaper function before freeing the data */
		if (cfunc) cfunc(N, cobj);
	}
	case NT_STRING:
	case NT_NFUNC:
		if (cobj->val->attr & NST_LINK) {
			cobj->val->attr ^= NST_LINK;
			cobj->val->d.str = NULL;
		} else if (cobj->val->d.str != NULL) {
			n_free(N, (void *)&cobj->val->d.str, cobj->val->size + 1);
		}
		break;
	case NT_TABLE: nsp_freetable(N, cobj); break;
	}
	cobj->val->type = NT_NULL;
	cobj->val->size = 0;
	cobj->val->d.num = 0;
	if (cobj->val->ztable != NULL) {
		nsp_unlinkval(N, cobj->val->ztable);
		n_free(N, (void *)&cobj->val->ztable, sizeof(obj_t));
	}
	return;
#undef __FN__
}

val_t *n_newval(nsp_state *N, unsigned short type)
{
#define __FN__ __FILE__ ":n_newval()"
	val_t *val = (val_t *)n_alloc(N, sizeof(val_t), 0);

	settrace();
	val->type = type;
	val->attr = 0;
	val->refs = 1;
	val->size = 0;
	val->ztable = NULL;
	val->d.num = 0;
	return val;
#undef __FN__
}

obj_t *n_newiobj(nsp_state *N, int index)
{
#define __FN__ __FILE__ ":n_newiobj()"
	obj_t *obj = (obj_t *)n_alloc(N, sizeof(obj_t), 0);

	settrace();
	obj->prev = NULL;
	obj->next = NULL;
	obj->val = NULL;
	n_setnamei(N, obj, index);
	return obj;
#undef __FN__
}

static uint32 cdb_hash(char *p)
{
	uint32 hash = 5381;

	while (*p) hash = (hash + (hash << 5)) ^ *p++;
	return hash;
}

obj_t *n_setname(nsp_state *N, obj_t *cobj, const char *name)
{
#define __FN__ __FILE__ ":n_setname()"
	nc_strncpy(cobj->name, name, MAX_OBJNAMELEN);
	cobj->hash = cdb_hash(cobj->name);
	if (nc_isdigit(name[0])) {
		char *p;

		for (p = cobj->name; nc_isdigit(*p); p++);
		if (!*p) cobj->nval = (signed long)n_aton(N, cobj->name); else cobj->nval = -1;
	} else cobj->nval = -1;
	return cobj;
#undef __FN__
}

obj_t *n_setnamei(nsp_state *N, obj_t *cobj, unsigned long i)
{
#define __FN__ __FILE__ ":n_setnamei()"
	n_ntoa(N, cobj->name, i, 10, 0);
	cobj->hash = cdb_hash(cobj->name);
	cobj->nval = i;
	return cobj;
#undef __FN__
}

/*
 * the following functions are public API functions
 */

void nsp_setvaltype(nsp_state *N, obj_t *cobj, unsigned short type)
{
#define __FN__ __FILE__ ":nsp_setvaltype()"
	settrace();
	if (cobj->val == NULL) {
		cobj->val = n_newval(N, type);
	} else if (cobj->val->type != NT_NULL) {
		if (nsp_istable(cobj)) {
			nsp_unlinkval(N, cobj);
			cobj->val = n_newval(N, type);
		} else {
			n_freeval(N, cobj);
			cobj->val->type = type;
		}
	} else {
		cobj->val->type = type;
	}
	if (type == NT_TABLE) cobj->val->attr |= NST_AUTOSORT;
	return;
#undef __FN__
}

void nsp_linkval(nsp_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FN__ __FILE__ ":nsp_linkval()"
	settrace();
	if (cobj1 == NULL || cobj1 == cobj2) return;
	if (cobj2 == NULL || cobj2->val == NULL) {
		nsp_unlinkval(N, cobj1);
		cobj1->val = n_newval(N, NT_NULL);
		return;
	} else {
		if (cobj1->val == cobj2->val) return;
		if (cobj1->val) nsp_unlinkval(N, cobj1);
		if (cobj2 == &N->r) {
			cobj1->val = cobj2->val;
			cobj2->val = NULL;
			return;
		}
		cobj1->val = cobj2->val;
	}
	if (cobj1->val) cobj1->val->refs++;
	return;
#undef __FN__
}

void nsp_unlinkval(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":nsp_unlinkval()"
	settrace();
	if (cobj == NULL || cobj->val == NULL) return;
	if (cobj->val->refs) {
		if (cobj->val->type == NT_TABLE && cobj->val->refs == 2 && cobj->val->d.table.f && cobj->val->d.table.f->val == cobj->val) {
			if (nc_strcmp(cobj->val->d.table.f->name, "this") == 0) {
				cobj->val->d.table.f->val->refs--;
				cobj->val->d.table.f->val = NULL;
			}
		}
		if (cobj->val != NULL && --cobj->val->refs < 1) {
			n_freeval(N, cobj);
			if (!(cobj->val->attr & NST_STACKVAL)) {
				n_free(N, (void *)&cobj->val, sizeof(val_t));
			}
		}
	}
	cobj->val = NULL;
	return;
#undef __FN__
}

void nsp_freetable(nsp_state *N, obj_t *tobj)
{
#define __FN__ __FILE__ ":nsp_freetable()"
	obj_t *cobj, *oobj;

	settrace();
	if (!nsp_istable(tobj)) return;
	cobj = tobj->val->d.table.f;
	while (cobj != NULL) {
		oobj = cobj;
		cobj = cobj->next;
		nsp_unlinkval(N, oobj);
		n_free(N, (void *)&oobj, sizeof(obj_t));
	}
	tobj->val->d.table.f = NULL;
	tobj->val->d.table.i = NULL;
	tobj->val->d.table.l = NULL;
	return;
#undef __FN__
}

static obj_t *n_locateobj(nsp_state *N, obj_t *tobj, char *oname, unsigned short setobj, unsigned short otype)
{
#define __FN__ __FILE__ ":n_locateobj()"
	obj_t *cobj, *oobj;
	unsigned short sortattr = tobj->val->attr & NST_AUTOSORT ? 1 : 0;
	signed long cmp = -1;
	signed long nval = -1;
	char *p;
	uchar *a, *b;

	if (!setobj && tobj->val->d.table.f == NULL) return NULL;
	if (nc_isdigit(oname[0])) {
		for (p = oname; nc_isdigit(*p); p++);
		if (!*p) nval = (signed long)n_aton(N, oname);
	}
	if (sortattr) {
		cobj = oobj = tobj->val->d.table.i ? tobj->val->d.table.i : tobj->val->d.table.f;
//		cmp=nval>-1?cobj->nval-nval:nc_strcmp(cobj->name, oname);
		if (nval > -1) {
			cmp = cobj->nval - nval;
		} else {
			for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
			cmp = *a - *b;
		}
	} else {
		oobj = tobj->val->d.table.f;
	}
	if (cmp < 0) {
		for (cobj = oobj; cobj; oobj = cobj, cobj = cobj->next) {
//			cmp=nval>-1?cobj->nval-nval:nc_strcmp(cobj->name, oname);
			if (nval > -1) {
				cmp = cobj->nval - nval;
			} else {
				for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
				cmp = *a - *b;
			}
			if (cmp == 0 || (cmp > 0 && sortattr)) break;
			tobj->val->d.table.i = cobj;
		}
		if (!setobj && cmp) return NULL;
	} else if (sortattr && cmp > 0) {
		for (cobj = oobj; cobj; oobj = cobj, cobj = cobj->prev) {
//			cmp=nval>-1?cobj->nval-nval:nc_strcmp(cobj->name, oname);
			if (nval > -1) {
				cmp = cobj->nval - nval;
			} else {
				for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
				cmp = *a - *b;
			}
			if (cmp <= 0) {
				oobj = cobj->prev ? cobj->prev : tobj->val->d.table.f;
				break;
			}
			tobj->val->d.table.i = cobj;
		}
		if (!setobj && cmp) return NULL;
		if (cmp < 0) {
			oobj = cobj;
			if (cobj->next) cobj = cobj->next;
			cmp = 1;
		} else if (cmp > 0) {
			cobj = oobj;
		}
	}
	if (!setobj) return cobj;
	if (cmp > 0 && sortattr) {
		oobj = cobj;
		cobj = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		if (oobj == tobj->val->d.table.f) tobj->val->d.table.f = cobj;
		cobj->prev = oobj->prev;
		if (cobj->prev) cobj->prev->next = cobj;
		cobj->next = oobj;
		oobj->prev = cobj;
		cobj->val = n_newval(N, otype);
		n_setname(N, cobj, oname);
		if (cobj->next == NULL) tobj->val->d.table.l = cobj;
	} else if (cobj == NULL) {
		cobj = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		if (oobj != NULL) {
			oobj->next = cobj;
			cobj->prev = oobj;
		} else {
			cobj->prev = NULL;
		}
		cobj->next = NULL;
		cobj->val = n_newval(N, otype);
		n_setname(N, cobj, oname);
		if (cobj->next == NULL) tobj->val->d.table.l = cobj;
	} else {
		switch (nsp_typeof(cobj)) {
		case NT_TABLE:
			cobj->val->type = otype;
			break;
		default:
			nsp_setvaltype(N, cobj, otype);
		}
	}
	return cobj;
#undef __FN__
}

obj_t *nsp_getobj_ex(nsp_state *N, obj_t *tobj, char *oname, unsigned short followz, unsigned short *foundz)
{
#define __FN__ __FILE__ ":nsp_getobj_ex()"
	/* unsigned long hashthis=cdb_hash("this"); */
	static const uint32 thishash = 2087959715;
	uint32 hash = cdb_hash(oname);
	obj_t *cobj, *cobj2, *thisobj = NULL;
	uchar *a, *b;

	settrace();
	if (foundz) *foundz = 0;
	if (tobj == &N->r) return tobj;
	if (N != NULL && tobj == NULL) {
		for (cobj = N->context->l.val->d.table.f; cobj; cobj = cobj->next) {
			/* printf("%s oname=%s 0x%08X %s\n", __FN__, oname, cobj, cobj->name); */
//			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
			if (cobj->hash == hash) {
				for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
				if (*a == *b) return cobj;
			}

			if (cobj->hash == thishash && nc_strcmp(cobj->name, "this") == 0) thisobj = cobj;
		}
		if (nsp_istable(thisobj)) {
			for (cobj = thisobj->val->d.table.f; cobj; cobj = cobj->next) {
//				if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
				if (cobj->hash == hash) {
					for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
					if (*a == *b) return cobj;
				}
			}
		}
		for (cobj = N->g.val->d.table.f; cobj; cobj = cobj->next) {
//			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
			if (cobj->hash == hash) {
				for (a = (uchar *)cobj->name, b = (uchar *)oname; *a && *a == *b; a++, b++);
				if (*a == *b) return cobj;
			}
		}
	} else if (nsp_istable(tobj)) {
		if ((cobj = n_locateobj(N, tobj, oname, 0, 0)) != NULL) return cobj;
//		for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
//			if (cobj->hash==hash&&nc_strcmp(cobj->name, oname)==0) return cobj;
//		}
		if (followz && nsp_istable(tobj->val->ztable)) {
			for (cobj = tobj->val->ztable->val->d.table.f; cobj; cobj = cobj->next) {
				if (!nsp_istable(cobj)) continue;
				cobj2 = nsp_getobj_ex(N, cobj, oname, followz, foundz);
				if (!nsp_isnull(cobj2)) {
					if (foundz) *foundz = 1;
					return cobj2;
				}
			}
		}
	}
	return &_nullobj;
#undef __FN__
}

obj_t *nsp_getobj(nsp_state *N, obj_t *tobj, char *oname)
{
#define __FN__ __FILE__ ":nsp_getobj()"
	return nsp_getobj_ex(N, tobj, oname, 1, NULL);
#undef __FN__
}

obj_t *nsp_getiobj(nsp_state *N, obj_t *tobj, unsigned long oindex)
{
#define __FN__ __FILE__ ":nsp_getiobj()"
	obj_t *cobj;
	unsigned long i = 0;

	settrace();
	if (!nsp_istable(tobj)) return &_nullobj;
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (nsp_isnull(cobj) || cobj->val->attr & NST_SYSTEM) continue;
		if (i == oindex) return cobj;
		i++;
	}
	return &_nullobj;
#undef __FN__
}

/* change or create an object and return it */
obj_t *nsp_setobj(nsp_state *N, obj_t *tobj, char *oname, unsigned short otype, NSP_CFUNC _fptr, num_t _num, char *_str, size_t _slen)
{
#define __FN__ __FILE__ ":nsp_setobj()"
	obj_t *cobj;

	settrace();
	if (tobj != &N->r && nsp_istable(tobj)) {
		if (oname[0] == '\0') { return &_nullobj; }
		if (tobj->val->d.table.f == NULL) {
			cobj = tobj->val->d.table.i = tobj->val->d.table.l = tobj->val->d.table.f = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
			cobj->prev = NULL;
			cobj->next = NULL;
			cobj->val = n_newval(N, otype);
			n_setname(N, cobj, oname);
			cobj->val->attr |= tobj->val->attr & NST_AUTOSORT;
		} else {
			cobj = n_locateobj(N, tobj, oname, 1, otype);
		}
	} else {
		cobj = tobj;
		if (cobj == NULL) n_error(N, NE_MEM, __FN__, "cobj=NULL,oname=%s", oname);
		nsp_setvaltype(N, cobj, otype);
	}
	switch (otype) {
	case NT_BOOLEAN: cobj->val->d.num = _num ? 1 : 0; break;
	case NT_NUMBER: cobj->val->d.num = _num; break;
	case NT_CFUNC: cobj->val->d.cfunc = _fptr; break;
	case NT_STRING:
	case NT_NFUNC:
	case NT_CDATA: nsp_strcat(N, cobj, _str, (unsigned long)_slen); break;
	}
	return cobj;
#undef __FN__
}

obj_t *nsp_setiobj(nsp_state *N, obj_t *tobj, int index, unsigned short otype, NSP_CFUNC _fptr, num_t _num, char *_str, long _slen)
{
#define __FN__ __FILE__ ":nsp_setiobj()"
	char numbuf[12];

	settrace();
	return nsp_setobj(N, tobj, n_ntoa(N, numbuf, index, 10, 0), otype, _fptr, _num, _str, _slen);
#undef __FN__
}

obj_t *nsp_appendobj(nsp_state *N, obj_t *tobj, char *name)
{
	obj_t *cobj;

	if (tobj->val->d.table.f == NULL) {
		cobj = tobj->val->d.table.f = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj->prev = NULL;
		cobj->next = NULL;
	} else {
		cobj = tobj->val->d.table.l;
		cobj->next = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj->next->prev = cobj;
		cobj->next->next = NULL;
		cobj = cobj->next;
	}
	tobj->val->d.table.l = cobj;
	if (name != NULL) n_setname(N, cobj, name);
	cobj->val = NULL;
	return cobj;
}

void nsp_strcat(nsp_state *N, obj_t *cobj, char *str, unsigned long len)
{
#define __FN__ __FILE__ ":nsp_strcat()"
	unsigned short ctype = nsp_typeof(cobj);
	unsigned long olen, tlen;
	char *p;

	settrace();
	if (ctype != NT_STRING && ctype != NT_NFUNC && ctype != NT_CDATA) return;
	if (len == -1 && str != NULL) len = nc_strlen(str);
	if (len < 1) return;
	olen = cobj->val->size;
	tlen = olen + len;
	p = olen ? n_realloc(N, (void *)&cobj->val->d.str, tlen + 1, olen + 1, 0) : n_alloc(N, tlen + 1, 0);
	if (p == NULL) n_error(N, NE_MEM, __FN__, "can't alloc %d bytes", tlen + 1);
	cobj->val->size = tlen;
	cobj->val->d.str = p;
	nc_memcpy(cobj->val->d.str + olen, str, len);
	cobj->val->d.str[tlen] = 0;
	return;
#undef __FN__
}

void nsp_strmul(nsp_state *N, obj_t *cobj, unsigned long n)
{
#define __FN__ __FILE__ ":nsp_strmul()"
	unsigned long i, olen, tlen;
	char *p;

	settrace();
	if (!nsp_isstr(cobj) || n < 2) return;
	olen = cobj->val->size;
	tlen = olen * n;
	p = olen ? n_realloc(N, (void *)&cobj->val->d.str, tlen + 1, olen + 1, 0) : n_alloc(N, tlen + 1, 0);
	if (p == NULL) n_error(N, NE_MEM, __FN__, "can't alloc %d bytes", tlen + 1);
	cobj->val->size = tlen;
	cobj->val->d.str = p;
	for (i = 1; i < n; i++) nc_memcpy(p + olen * i, cobj->val->d.str, olen);
	cobj->val->d.str[tlen] = 0;
	return;
#undef __FN__
}

short nsp_tobool(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":nsp_tobool()"
	settrace();
	switch (nsp_typeof(cobj)) {
	case NT_NULL: return 0;
	case NT_BOOLEAN: return cobj->val->d.num ? 1 : 0;
	case NT_NUMBER: return cobj->val->d.num ? 1 : 0;
	case NT_STRING: return cobj->val->size ? 1 : 0;
	case NT_TABLE: return cobj->val->d.table.f ? 1 : 0;
	case NT_NFUNC:
	case NT_CFUNC: return 1;
	}
	return 0;
#undef __FN__
}

num_t nsp_tonum(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":nsp_tonum()"
	settrace();
	switch (nsp_typeof(cobj)) {
	case NT_NUMBER: return cobj->val->d.num;
	case NT_BOOLEAN: return cobj->val->d.num ? 1 : 0;
	case NT_STRING: return n_aton(N, cobj->val->d.str);
	}
	return 0;
#undef __FN__
}

char *nsp_tostr(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":nsp_tostr()"
	settrace();
	switch (nsp_typeof(cobj)) {
	case NT_STRING: return cobj->val->d.str ? cobj->val->d.str : "";
	case NT_BOOLEAN: return cobj->val->d.num ? "true" : "false";
	case NT_NUMBER: return n_ntoa(N, N->numbuf, cobj->val->d.num, -10, 6);
	case NT_NULL: return "null";
	}
	return "";
#undef __FN__
}

char *nsp_zlink(nsp_state *N, obj_t *cobj1, obj_t *cobj2)
{
#define __FN__ __FILE__ ":nsp_zlink()"
	obj_t *cobj = NULL;
	int i = 0;

	settrace();
	if (cobj1->val->ztable == NULL) {
		cobj1->val->ztable = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj1->val->ztable->prev = NULL;
		cobj1->val->ztable->next = NULL;
		cobj1->val->ztable->val = n_newval(N, NT_TABLE);
		n_setname(N, cobj1->val->ztable, "z");
	}
	if (cobj1->val->ztable->val->d.table.f == NULL) {
		cobj = cobj1->val->ztable->val->d.table.f = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
		cobj->prev = NULL;
		cobj->next = NULL;
		cobj->val = NULL;
		n_setnamei(N, cobj, i);
		cobj1->val->ztable->val->d.table.l = cobj;
	} else {
		for (cobj = cobj1->val->ztable->val->d.table.f; cobj; cobj = cobj->next, i++) {
			if (cobj->next != NULL) continue;
			cobj->next = (obj_t *)n_alloc(N, sizeof(obj_t), 0);
			cobj->next->prev = cobj;
			cobj->next->next = NULL;
			cobj = cobj->next;
			cobj->val = NULL;
			n_setnamei(N, cobj, i);
			cobj1->val->ztable->val->d.table.l = cobj;
			break;
		}
	}
	nsp_linkval(N, cobj, cobj2);
	return "";
#undef __FN__
}
