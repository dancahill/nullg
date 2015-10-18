/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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
#include "nsp/nsplib.h"
#include "data.h"
#include <string.h>
#ifdef WIN32
#define strcasecmp stricmp
#endif
#ifdef __TURBOC__
#define strcasecmp stricmp
#endif

static void libnsp_data_xml_trimtree(nsp_state *N, obj_t *tobj)
{
	obj_t *cobj = tobj, *nobj;

	if (!nsp_istable(cobj) || cobj->val->d.table.f == NULL) return;
	nobj = cobj->val->d.table.f;
	if (nobj->next == NULL&&nsp_istable(nobj) && nc_strcmp(nobj->name, "0") == 0) {
		n_free(N, (void *)&cobj->val, sizeof(val_t));
		cobj->val = nobj->val;
		n_free(N, (void *)&nobj, sizeof(obj_t));
	}
	if (!nsp_istable(cobj)) return;
	for (cobj = cobj->val->d.table.f;cobj;cobj = cobj->next) {
		if (nsp_istable(cobj)) libnsp_data_xml_trimtree(N, cobj);
	}
	return;
}

#include <stdio.h>

/*
 * do NOT use this. this is TEST code.
 * this code is dangerous, sloppy and exists solely for test purposes.
 * please either ignore it until it goes away, or help fix it.
 */
static char *libnsp_data_xml_readsub(nsp_state *N, obj_t *tobj, char *ptr, char *otag, int *eb)
{
#define __FN__ __FILE__ ":libnsp_data_xml_readsub()"
	char tagbuf[MAX_OBJNAMELEN + 1];
	char namebuf[MAX_OBJNAMELEN + 1];
	obj_t *cobj;
	//	obj_t *iobj;
	obj_t *nobj;
	obj_t *aobj;
	int i, j;
	char *b = ptr, *e;
	//	char *t, *tb;
	char *tb;
	//	int end;
	char q;
	char *p1, *p2;
	uchar is_end;
	uchar is_dat;

	for (i = 0;;i++) {
		*eb = 0;
		nobj = tobj;
		if (b == NULL) break;
		while (nc_isspace(*b)) b++;
		if (*b == '\0') break;
		p1 = b;
		is_end = 0;
		tagbuf[0] = '\0';
		if (*p1 == '<') {
			p1++;
			while (nc_isspace(*p1)) p1++;
			if (*p1 == '/') {
				is_end = 1;
				/* end tag */
				p1++;
				while (nc_isspace(*p1)) p1++;
				if (nc_isalnum(*p1) || *p1 == ':' || *p1 == '_' || *p1 == '-') {
					j = 0;
					while (j < MAX_OBJNAMELEN && (nc_isalnum(*p1) || *p1 == ':' || *p1 == '_' || *p1 == '-')) tagbuf[j++] = *p1++;
					tagbuf[j] = '\0';
				}
				else {
					n_error(N, NE_SYNTAX, __FN__, "unexpected data");
				}

				//				printf("end [%s][%s]\n", otag, tagbuf);

				p2 = p1;
				while (*p2&&*p2 != '>') p2++;
				if (*p2 != '>') n_error(N, NE_SYNTAX, __FN__, "missing >");
				p2++;
				e = p2;
				if (strcmp(otag, tagbuf) == 0) break;
				//				n_warn(N, __FN__, "end tag '%s' '%s'", otag, tagbuf);
			}
			else if (p1[0] == '!'&&p1[1] == '-'&&p1[2] == '-') {
				/* skip comment */
				is_end = 1;
				//				while (nc_isspace(*p1)) p1++;
				p2 = p1;
				while (*p2&&*p2 != '>') {
					//					if (*p2=='/') is_end=1;
					p2++;
				}
				if (*p2 != '>') n_error(N, NE_SYNTAX, __FN__, "missing >");
				p2++;
				e = p2;
				b = e;
				//				continue;
			}
			else {
				/* read tag data */
				while (nc_isspace(*p1)) p1++;
				if (nc_isalnum(*p1) || *p1 == ':' || *p1 == '_' || *p1 == '-') {
					j = 0;
					while (j < MAX_OBJNAMELEN && (nc_isalnum(*p1) || *p1 == ':' || *p1 == '_' || *p1 == '-')) tagbuf[j++] = *p1++;
					tagbuf[j] = '\0';

					j = -1;
					nobj = nsp_settable(N, tobj, tagbuf);
					for (cobj = nobj->val->d.table.f;cobj;cobj = cobj->next) {
						if (nc_isdigit(cobj->name[0])) j = (int)n_aton(N, cobj->name);
					}
					nobj = nsp_settable(N, nobj, n_ntoa(N, namebuf, ++j, 10, 0));

					/* if (N->debug) n_warn(N, __FN__, "new node '%s'", namebuf); */
					/* start adding attributes */
					aobj = NULL;
					for (;;) {
						while (nc_isspace(*p1)) p1++;
						if ((*p1 == '\0') || (*p1 == '>')) break;
						j = 0;
						while (nc_isalnum(*p1) || (*p1 == ':') || (*p1 == '_') || (*p1 == '-')) namebuf[j++] = *p1++;
						namebuf[j] = '\0';
						while (nc_isspace(*p1)) p1++;
						if ((*p1 == '\0') || (*p1 == '>')) break;
						if (*p1 == '=') {
							p1++;
							while (nc_isspace(*p1)) p1++;
							if ((*p1 == '\0') || (*p1 == '>')) break;
							if ((*p1 == '\'') || (*p1 == '\"')) {
								q = *p1;
								tb = ++p1;
								while ((*p1) && (*p1 != q)) p1++;
							}
							else {
								q = '\0';
								tb = p1;
								while ((*p1) && (*p1 != '>') && (!nc_isspace(*p1))) p1++;
							}
							if (*p1) {
								if (aobj == NULL) aobj = nsp_settable(N, nobj, "@attributes");
								cobj = nsp_setstr(N, aobj, namebuf, tb, p1 - tb);
								if ((q) && (*p1 == q)) p1++;
								/* if (N->debug) n_warn(N, __FN__, "new attr  '%s->%s' = '%s'", cobj->parent->parent->parent->name, cobj->name, nsp_tostr(N, cobj)); */
							}
						}
						while (nc_isspace(*p1)) p1++;
						if (*p1 == '/') {
							*eb = 1;
							while (nc_isspace(*p1)) p1++;
							break;
						}
					}
					/* finish adding attributes */

				}
				else if (*p1 == '?' || *p1 == '!') {
				}
				else {
					n_error(N, NE_SYNTAX, __FN__, "unexpected data [%s]", p1);
				}
				while (nc_isspace(*p1)) p1++;
				p2 = p1;
				while (*p2&&*p2 != '>') {
					if (*p2 == '/') is_end = 1;
					p2++;
				}
				if (*p2 != '>') n_error(N, NE_SYNTAX, __FN__, "missing >");
				p2++;
				e = p2;
			}
			//			nl_flush(N);cobj=nsp_setstr(N, &N->l, "debug", b, e-b);printf("tag %d[%s]\n", is_end, nsp_tostr(N, cobj));
			b = e;
			if (!is_end) {
				b = libnsp_data_xml_readsub(N, nobj, b, tagbuf, eb);
			}
		}
		/* check for data between tags */
		is_dat = 0;
		p2 = p1 = b;
		while (*p2&&*p2 != '<') {
			if (is_dat == 0 && !nc_isspace(*p2)) {
				is_dat = 1;
			}
			p2++;
		}
		e = p2;
		if (is_dat) {
			nsp_setstr(N, nobj, "value", b, e - b);
			//			nl_flush(N);cobj=nsp_setstr(N, &N->l, "debug", b, e-b);	printf("dat %d[%s]\n", is_dat, nsp_tostr(N, cobj));
		}
		b = e;
		/*
		continue;

				if ((e=nc_strchr(b, '>'))==NULL) break;
				e++;
		//		end=0;
				if ((t=nc_strchr(b, '<'))!=NULL) if (t[1]=='/') {
		//			n_warn(N, __FN__, "x end label '%s' '%s'", tobj->name, nobj->name);
		//			if (!strlen(tobj->name))
					*eb=1;
					b=e;
		//			goto x;
					break;
		//			return e;
				} / * this tag probably closes a previously opened block * /
				if (b[e-b-2]=='/') *eb=1; / * this block is complete * /
				/ * parse the tag name and attributes here * /
				t=b;
				if ((t[0]=='<')&&(t[1]!='?')&&(t[1]!='!')) {
					t++;
					j=0;
					while (nc_isspace(*t)) t++;
					while (nc_isalnum(*t)||(*t==':')||(*t=='_')||(*t=='-')) namebuf[j++]=*t++;
					namebuf[j]='\0';
					nobj=nsp_settable(N, tobj, namebuf);
		//			if (N->debug)
		//			n_warn(N, __FN__, "new label '%s'", namebuf);
					j=-1;
					for (cobj=nobj->val->d.table.f; cobj; cobj=cobj->next) {
						if (nc_isdigit(cobj->name[0])) j=(int)n_aton(N, cobj->name);
					}
					nobj=nsp_settable(N, nobj, n_ntoa(N, namebuf, ++j, 10, 0));
		//			if (N->debug) n_warn(N, __FN__, "new node '%s'", namebuf);
					aobj=NULL;
					for (;;) {
						while (nc_isspace(*t)) t++;
						if ((*t=='\0')||(*t=='>')) break;
						j=0;
						while (nc_isalnum(*t)||(*t==':')||(*t=='_')||(*t=='-')) namebuf[j++]=*t++;
						namebuf[j]='\0';
						while (nc_isspace(*t)) t++;
						if ((*t=='\0')||(*t=='>')) break;
						if (*t=='=') {
							t++;
							while (nc_isspace(*t)) t++;
							if ((*t=='\0')||(*t=='>')) break;
							if ((*t=='\'')||(*t=='\"')) {
								q=*t;
								tb=++t;
								while ((*t)&&(*t!=q)) t++;
							} else {
								q='\0';
								tb=t;
								while ((*t)&&(*t!='>')&&(!nc_isspace(*t))) t++;
							}
							if (*t) {
								if (aobj==NULL) aobj=nsp_settable(N, nobj, "@attributes");
								cobj=nsp_setstr(N, aobj, namebuf, tb, t-tb);
								if ((q)&&(*t==q)) t++;
								/ * if (N->debug) n_warn(N, __FN__, "new attr  '%s->%s' = '%s'", cobj->parent->parent->parent->name, cobj->name, nsp_tostr(N, cobj)); * /
							}
						}
						while (nc_isspace(*t)) t++;
						if (*t=='/') {
							*eb=1;
							while (nc_isspace(*t)) t++;
							break;
						}
					}
				}
				b=e;
				t=b;
				while (nc_isspace(*t)) t++;
				e=nc_strchr(t, '<');
				if ((*t)&&(*t!='<')&&(e!=NULL)) { / * might be actual data * /
					nsp_setstr(N, nobj, "value", b, e-b);
					/ * if (b[e-b-2]=='/') *eb=1; */ /* this block is complete * /
					b=e;
		//			continue;
				}
				/ * if no closing tag, then recurse for the next tree level * /
				if ((tobj!=NULL)&&(tobj->name[0]!='!')) {
		//		if ((tobj!=NULL)&&(tobj->name[0]!='@')) {
					/ * this is a HACK. * /
					/ * i'm too lazy to research *ml, but these tags aren't expected to have a closing / * /
					if (strcasecmp(tobj->name, "META")==0) *eb=1;
					if (strcasecmp(tobj->name, "LINK")==0) *eb=1;
					if (strcasecmp(tobj->name, "IMG")==0) *eb=1;
					if (strcasecmp(tobj->name, "BR")==0) *eb=1;
					if (strcasecmp(tobj->name, "HR")==0) *eb=1;
				}
				if (N->debug) n_warn(N, __FN__, "... '%s'", tobj->name);
		//x:
		//		n_warn(N, __FN__, "eb1=%d", *eb);
				if (!*eb) {
		//			n_warn(N, __FN__, "begin label '%s' '%s'", tobj->name, nobj->name);
		//			b=libnsp_data_xml_readsub(N, nobj, b, eb);
		//			n_warn(N, __FN__, "end label '%s' '%s'", tobj->name, nobj->name);
		//			if (*eb) { *eb=0; break; }
		//			if (*eb) { *eb=0; }
				}
		//		n_warn(N, __FN__, "eb2=%d", *eb);
		//			if (*eb) { *eb=0; }
		//		if (*eb) { *eb=0; }
				if (b==NULL) break;
		*/
	}
	return b;
#undef __FN__
}

NSP_FUNCTION(libnsp_data_xml_read)
{
#define __FN__ __FILE__ ":libnsp_data_xml_read()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	obj_t tobj;
	int eb = 0;
	int trim = 1;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	libnsp_data_xml_readsub(N, &tobj, cobj1->val->d.str, "", &eb);
	if (!nsp_isnull(cobj2)) trim = nsp_tobool(N, cobj2) ? 1 : 0;
	if (trim) libnsp_data_xml_trimtree(N, &tobj);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}
