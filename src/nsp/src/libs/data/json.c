/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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

/* JavaScript Object Notation */

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
	//	case '\'': dst[n++] = '\\'; dst[n++] = '\''; break;
		case '\"': dst[n++] = '\\'; dst[n++] = '\"'; break;
		case '\\': dst[n++] = '\\'; dst[n++] = '\\'; break;
		default: dst[n++] = str[i];
		}
	}
	nsp_strcat(N, &N->r, buf, n);
	return len;
#undef __FN__
}

static void n_dumpvars(nsp_state *N, obj_t *tobj, int depth)
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

		b = 1;

		if (cobj->val->type == NT_BOOLEAN || cobj->val->type == NT_NUMBER) {
			if (ent++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s\n", depth ? "," : "");
			if (depth) {
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s: ", g, b ? "\"" : "", cobj->name, b ? "\"" : "");
			}
			buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s", nsp_tostr(N, cobj));
		}
		else if (cobj->val->type == NT_STRING) {
			if (ent++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s\n", depth ? "," : "");
			if (depth) {
				for (i = 0; i < depth; i++) buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "\t");
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s: ", g, b ? "\"" : "", cobj->name, b ? "\"" : "");
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
				buflen += nc_snprintf(N, buf + buflen, sizeof(buf) - buflen, "%s%s%s%s: ", g, b ? "\"" : "", cobj->name, b ? "\"" : "");
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

NSP_FUNCTION(libnsp_data_json_encode)
{
#define __FN__ __FILE__ ":libnsp_data_json_encode()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");

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
