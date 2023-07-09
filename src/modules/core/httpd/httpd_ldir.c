/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "httpd_main.h"

/*
 * this can easily be rehacked (by someone less lazy) to sort by any key depths and combos
 */
static void sort_bykey(nsp_state *N, obj_t *tobj, char *key, char *subtab, int order)
{
	obj_t *cobj, *nobj, *robj = tobj->val->d.table.f;
	obj_t *sub1, *sub2;
	short change, swap;

reloop:
	if (nsp_isnull(robj)) goto end;
	cobj = robj;
	change = 0;
	for (;;) {
		if (nsp_isnull(cobj)) break;
		if ((nobj = cobj->next) == NULL) break;
		if (cobj->val->type != NT_TABLE) break;
		if (nobj->val->type != NT_TABLE) break;
		if (subtab) {
			sub1 = nsp_getobj(N, nsp_getobj(N, cobj, subtab), key);
			sub2 = nsp_getobj(N, nsp_getobj(N, nobj, subtab), key);
		}
		else {
			sub1 = nsp_getobj(N, cobj, key);
			sub2 = nsp_getobj(N, nobj, key);
		}
		swap = 0;
		if ((sub1->val->type == NT_NUMBER) && (sub2->val->type == NT_NUMBER)) {
			if (order < 0) {
				if (sub1->val->d.num < sub2->val->d.num) swap = 1;
			}
			else {
				if (sub1->val->d.num > sub2->val->d.num) swap = 1;
			}
		}
		else {
			if (order < 0) {
				if (strcmp(nsp_tostr(N, sub1), nsp_tostr(N, sub2)) < 0) swap = 1;
			}
			else {
				if (strcmp(nsp_tostr(N, sub1), nsp_tostr(N, sub2)) > 0) swap = 1;
			}
		}
		if (swap) {
			if ((cobj->prev == NULL) && (cobj == robj)) {
				robj = nobj;
			}
			nobj = cobj;
			cobj = cobj->next;
			cobj->prev = nobj->prev;
			nobj->next = cobj->next;
			cobj->next = nobj;
			nobj->prev = cobj;
			if (cobj->prev != NULL) cobj->prev->next = cobj;
			if (nobj->next != NULL) nobj->next->prev = nobj;
			change = 1;
		}
		cobj = cobj->next;
	}
	if (change) goto reloop;
end:
	tobj->val->d.table.f = robj;
	return;
}

static char *unescape(char *src, char *dst)
{
	char *o = dst;
	short e = 0;

	while (*src) {
		if (!e) {
			if (*src == '\\') {
				e = 1;
			}
			else {
				*dst++ = *src;
			}
			src++;
			continue;
		}
		switch (*src) {
		case 'a': *dst++ = '\a'; break;
		case 't': *dst++ = '\t'; break;
		case 'f': *dst++ = '\f'; break;
		case 'e': *dst++ = 27;   break;
		case 'r': *dst++ = '\r'; break;
		case 'n': *dst++ = '\n'; break;
		case '\'': *dst++ = '\''; break;
		case '\"': *dst++ = '\"'; break;
		case '\\': *dst++ = '\\'; break;
		default: break;
		}
		src++;
		e = 0;
	}
	*dst = '\0';
	return o;
}

void ldir_freeresult(obj_t **qobj)
{
	sql_freeresult(proc->N, qobj);
	return;
}

int ldir_deleteentry(char *oc, int id, int did)
{
	int rc;

	rc = sql_updatef(proc->N, NULL, "DELETE FROM nullg_entries WHERE class = '%s' AND id = %d AND did = %d", oc, id, did);
	return rc;
}

static int dump_sub(nsp_state *N, char *out, int max, obj_t *tobj)
{
	char *p;
	int ent = 0;
	int len = 0;
	int i;

	for (tobj = tobj->val->d.table.f;tobj;tobj = tobj->next) {
		if (ent) { strncatf(out + len, max - len, ", "); len += strlen(out + len); }
		if (tobj->val->type == NT_TABLE) {
			strncatf(out + len, max - len - 1, "%s={ ", tobj->name);
			len += strlen(out + len);
			len += dump_sub(N, out + len, max - len, tobj);
			strncatf(out + len, max - len - 1, " }");
			len += strlen(out + len);
			ent++;
		}
		else if (tobj->val->type == NT_NUMBER) {
			strncatf(out + len, max - len - 1, "%s=", tobj->name);
			len += strlen(out + len);
			p = nsp_tostr(N, tobj);
			for (i = 0;*p;i++) out[len++] = *p++;
			ent++;
		}
		else {
			strncatf(out + len, max - len - 1, "%s=''", tobj->name);
			len += strlen(out + len);
			p = nsp_tostr(N, tobj);
			for (i = 0;*p;i++) {
				switch (*p) {
				case '\\': strncatf(out + len, max - len - 1, "\\\\"); break;
				case '\"': strncatf(out + len, max - len - 1, "\\\""); break;
				case '\'': strncatf(out + len, max - len - 1, "\\\'\'"); break;
				case '\r': break;
				case '\n': strncatf(out + len, max - len - 1, "\\n"); break;
				default: out[len] = *p; break;
				}
				p++;
				len += strlen(out + len);
			}
			strncatf(out + len, max - len - 1, "''");
			len += strlen(out + len);
			ent++;
		}
	}
	return len;
}

int ldir_saveentry(CONN *conn, int id, char *oc, obj_t **qobj)
{
	/*
	 * there is stack corruption in this function when the local vars are initialized in a different order
	 * and it seems to relate to curdate (or its memset()) and free() triggers it
	 * is this a libc bug or mine?
	 *
	 * ... may actually be a bug in old dump_sub()
	 * right now it looks fixed
	 */
	char curdate[32];
	obj_t *cobj, *dobj, *tobj;
	char *name, *query;
	int did, pid;
	int len, rc;
	time_t t = time(NULL);

	if ((query = calloc(MAX_TUPLE_SIZE, sizeof(char))) == NULL) return -1;
	tobj = nsp_getobj(proc->N, *qobj, "rows");
	if (!nsp_istable(tobj)) { free(query); prints(conn, "broken table"); return -1; }
	tobj = nsp_getiobj(proc->N, tobj, 0);
	if (!nsp_istable(tobj)) { free(query); prints(conn, "broken table"); return -1; }
	pid = (int)nsp_getnum(proc->N, tobj, "pid");
	did = (int)nsp_getnum(proc->N, tobj, "did");
	dobj = nsp_settable(proc->N, tobj, "_data");
	if (strcmp(oc, "user") == 0) {
		cobj = nsp_getobj(conn->N, dobj, "uid");
	}
	else {
		cobj = nsp_getobj(conn->N, dobj, "cn");
		if (nsp_isnull(cobj)) cobj = nsp_getobj(conn->N, dobj, "name");
		if (nsp_isnull(cobj)) cobj = nsp_getobj(conn->N, tobj, "name");
	}
	name = nsp_tostr(conn->N, cobj);
	//	memset(curdate, 0, sizeof(curdate));
	//	time_unix2sql(curdate, sizeof(curdate)-1, t);
	strftime(curdate, sizeof(curdate) - 1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (id == 0) {
		//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
		len = snprintf(query, MAX_TUPLE_SIZE - 1, "INSERT INTO nullg_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', '%s', '%s', '{ ", pid, did, curdate, curdate, oc, name);
		len += dump_sub(conn->N, query + len, MAX_TUPLE_SIZE - len, dobj);
		strncatf(query + len, MAX_TUPLE_SIZE - len - 1, "}');");
		len += strlen(query + len);
		id = sql_update(proc->N, NULL, query);
		if (id < 0) id = -1;
	}
	else {
		//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
		len = snprintf(query, MAX_TUPLE_SIZE - 1, "UPDATE nullg_entries SET pid=%d, did=%d, mtime='%s', class='%s', name='%s', data='{ ", pid, did, curdate, oc, name);
		len += dump_sub(conn->N, query + len, MAX_TUPLE_SIZE - len, dobj);
		strncatf(query + len, MAX_TUPLE_SIZE - len - 1, "}' WHERE id=%d;", id);
		len += strlen(query + len);
		rc = sql_update(proc->N, NULL, query);
		if (rc < 0) id = -1;
	}
	//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	prints(conn, "<BR /><B>[id=%d][len=%d]<BR>%s</B>", id, len, query);
	//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	free(query);
	//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	return id;
}

obj_t *ldir_getlist(nsp_state *N, char *oc, int pid, int did)
{
	obj_t *qobj1 = NULL;
	obj_t *tobj;
	int rc = -1;
	int i;
	char *p;

	if (oc == NULL) oc = "";
	//	if ((priv&A_ADMIN)) {
	//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND obj_did = %d ORDER BY bookmarkname ASC", pid, conn->dat->did)<0) return;
	//		if (sql_queryf(proc->N, &qobj, "SELECT id, name, data FROM nullg_entries WHERE class = 'bookmark' AND pid = %d AND did = %d ORDER BY name ASC", pid, conn->dat->did)<0) return;
	//	} else {
	//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", pid, conn->dat->uid, conn->dat->gid, conn->dat->did)<0) return;
	if (pid) {
		rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND pid = %d AND did = %d ORDER BY name ASC;", oc, pid, did);
	}
	else if (strlen(oc) > 0) {
		if (strcmp(oc, "domain") == 0) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_domains ORDER BY domainname ASC;", did);
		}
		else if (strcmp(oc, "user") == 0) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_users WHERE obj_did = %d ORDER BY obj_did, userid ASC;", did);
		}
		else if (strcmp(oc, "contact") == 0) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_contacts WHERE obj_did = %d ORDER BY obj_did, contactid ASC;", did);
		}
		else if (strcmp(oc, "dbquery") == 0) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_queries WHERE obj_did = %d ORDER BY obj_did, queryid ASC;", did);
		}
		else {
			log_error(proc->N, "shit", __FILE__, __LINE__, 1, "oc '%s' missing", oc);
		}
	}
	else {
		if (strcmp(oc, "contact") == 0) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_contacts WHERE obj_did = %d ORDER BY obj_did, contactid ASC;", did);
		}
		else {
			log_error(proc->N, "shit", __FILE__, __LINE__, 1, "oc '%s' missing", oc);
		}
	}
	//	}
	if (rc < 0) return NULL;
	for (i = 0;i < sql_numtuples(proc->N, &qobj1);i++) {
		tobj = nsp_getobj(proc->N, qobj1, "rows");
		tobj = nsp_getiobj(proc->N, tobj, i);
		tobj = nsp_settable(proc->N, tobj, "_data");
		if ((p = sql_getvaluebyname(proc->N, &qobj1, i, "data")) == NULL) continue;
		nsp_linkval(N, tobj, nsp_eval(N, p));
		for (tobj = tobj->val->d.table.f;tobj;tobj = tobj->next) {
			if ((tobj->val->type == NT_STRING) && (tobj->val->d.str)) {
				unescape(tobj->val->d.str, tobj->val->d.str);
			}
		}
	}
	if (strcmp(oc, "weblogentry") == 0) {
		nsp_linkval(N, nsp_settable(N, &N->g, "weblogentryresults"), qobj1);
		sort_bykey(N, nsp_getobj(N, qobj1, "rows"), "ctime", NULL, 1);
		//	} else if (strcmp(oc, "user")==0) {
		//		ldir_sortlist(N, qobj1, "sn", "_data", 1);
	}
	return qobj1;
}

void ldir_sortlist(nsp_state *N, obj_t *qobj, char *key, char *subtab, int order)
{
	obj_t *tobj;
	unsigned int i;

	tobj = nsp_getobj(N, qobj, "rows");
	if (!nsp_istable(tobj)) return;
	sort_bykey(N, tobj, "sn", "_data", order);
	for (i = 0, tobj = tobj->val->d.table.f;tobj;i++, tobj = tobj->next) {
		sprintf(tobj->name, "%d", i);
	}
	return;
}

obj_t *ldir_getentry(nsp_state *N, char *oc, char *name, int id, int did)
{
	obj_t *qobj1 = NULL;
	obj_t *tobj;
	int rc = -1;
	int i;
	//	char *p;

	if (strcmp(oc, "user") == 0) {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_users WHERE userid = %d;", id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_users WHERE username = '%s';", name);
			return qobj1;
		}
	}
	else if (strcmp(oc, "contact") == 0) {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_contacts WHERE obj_did = %d AND contactid = %d;", did, id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_contacts WHERE obj_did = %d AND username = '%s';", did, name);
		}
	}
	else if (strcmp(oc, "dbquery") == 0) {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM gw_queries WHERE obj_did = %d AND queryid = %d;", did, id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	}
	else if (strcmp(oc, "bookmarkfolder") == 0) {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	}
	else if (strcmp(oc, "bookmark") == 0) {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	}
	else {
		if (id) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		}
		else if (name) {
			rc = sql_queryf(proc->N, &qobj1, "SELECT * FROM nullg_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	}
	if (sql_numtuples(proc->N, &qobj1) != 1) {
		sql_freeresult(proc->N, &qobj1);
		return NULL;
	}
	if (rc < 0) return NULL;
	for (i = 0;i < sql_numtuples(proc->N, &qobj1);i++) {
		tobj = nsp_getobj(proc->N, qobj1, "rows");
		tobj = nsp_getiobj(proc->N, tobj, i);
		tobj = nsp_settable(proc->N, tobj, "_data");
		//		if ((p=sql_getvaluebyname(proc->N, &qobj1, i, "data"))==NULL) continue;
		//		nsp_linkval(N, tobj, nsp_eval(N, p));
		//		for (tobj=tobj->val->d.table.f;tobj;tobj=tobj->next) {
		//			if ((tobj->val->type==NT_STRING)&&(tobj->val->d.str)) {
		//				unescape(tobj->val->d.str, tobj->val->d.str);
		//			}
		//		}
	}
	return qobj1;
}

int ldir_numentries(obj_t **qobj)
{
	return sql_numtuples(proc->N, qobj);
}

char *ldir_getval(obj_t **qobj, int rowset, char *name)
{
	obj_t *tobj, *tobj2;
	obj_t *cobj;

	tobj = nsp_getobj(proc->N, *qobj, "rows");
	if (tobj->val->type != NT_TABLE) return "";
	tobj = nsp_getiobj(proc->N, tobj, rowset);
	if (tobj->val->type != NT_TABLE) return "";
	tobj2 = nsp_getobj(proc->N, tobj, "_data");
	if (tobj2->val->type == NT_TABLE) {
		for (cobj = tobj2->val->d.table.f; cobj; cobj = cobj->next) {
			if (strcasecmp(cobj->name, name) != 0) continue;
			return nsp_tostr(proc->N, cobj);
		}
	}
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (strcasecmp(cobj->name, name) != 0) continue;
		return nsp_tostr(proc->N, cobj);
	}
	return "";
}
