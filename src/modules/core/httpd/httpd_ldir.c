/*
    NullLogic Groupware - Copyright (C) 2000-2007 Dan Cahill

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
static void sort_bykey(nes_state *N, obj_t *tobj, char *key, char *subtab, int order)
{
	obj_t *cobj, *nobj, *robj=tobj->val->d.table;
	obj_t *sub1, *sub2;
	short change, swap;

reloop:
	if (nes_isnull(robj)) goto end;
	cobj=robj;
	change=0;
	for (;;) {
		if (nes_isnull(cobj)) break;
		if ((nobj=cobj->next)==NULL) break;
		if (cobj->val->type!=NT_TABLE) break;
		if (nobj->val->type!=NT_TABLE) break;
		if (subtab) {
			sub1=nes_getobj(N, nes_getobj(N, cobj, subtab), key);
			sub2=nes_getobj(N, nes_getobj(N, nobj, subtab), key);
		} else {
			sub1=nes_getobj(N, cobj, key);
			sub2=nes_getobj(N, nobj, key);
		}
		swap=0;
		if ((sub1->val->type==NT_NUMBER)&&(sub2->val->type==NT_NUMBER)) {
			if (order<0) {
				if (sub1->val->d.num<sub2->val->d.num) swap=1;
			} else {
				if (sub1->val->d.num>sub2->val->d.num) swap=1;
			}
		} else {
			if (order<0) {
				if (strcmp(nes_tostr(N, sub1), nes_tostr(N, sub2))<0) swap=1;
			} else {
				if (strcmp(nes_tostr(N, sub1), nes_tostr(N, sub2))>0) swap=1;
			}
		}
		if (swap) {
			if ((cobj->prev==NULL)&&(cobj==robj)) {
				robj=nobj;
			}
			nobj=cobj;
			cobj=cobj->next;
			cobj->prev=nobj->prev;
			nobj->next=cobj->next;
			cobj->next=nobj;
			nobj->prev=cobj;
			if (cobj->prev!=NULL) cobj->prev->next=cobj;
			if (nobj->next!=NULL) nobj->next->prev=nobj;
			change=1;
		}
		cobj=cobj->next;
	}
	if (change) goto reloop;
end:
	tobj->val->d.table=robj;
	return;
}

static char *unescape(char *src, char *dst)
{
	char *o=dst;
	short e=0;

	while (*src) {
		if (!e) {
			if (*src=='\\') {
				e=1;
			} else {
				*dst++=*src;
			}
			src++;
			continue;
		}
		switch (*src) {
			case 'a'  : *dst++='\a'; break;
			case 't'  : *dst++='\t'; break;
			case 'f'  : *dst++='\f'; break;
			case 'e'  : *dst++=27;   break;
			case 'r'  : *dst++='\r'; break;
			case 'n'  : *dst++='\n'; break;
			case '\'' : *dst++='\''; break;
			case '\"' : *dst++='\"'; break;
			case '\\' : *dst++='\\'; break;
			default   : break;
		}
		src++;
		e=0;
	}
	*dst='\0';
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

	rc=sql_updatef(proc->N, "DELETE FROM nullgs_entries WHERE class = '%s' AND id = %d AND did = %d", oc, id, did);
	return rc;
}

static int dump_sub(nes_state *N, char *out, int max, obj_t *tobj)
{
	char *p;
	int ent=0;
	int len=0;
	int i;

	for (tobj=tobj->val->d.table;tobj;tobj=tobj->next) {
		if (ent) { strncatf(out+len, max-len, ", "); len+=strlen(out+len); }
		if (tobj->val->type==NT_TABLE) {
			strncatf(out+len, max-len-1, "%s={ ", tobj->name);
			len+=strlen(out+len);
			len+=dump_sub(N, out+len, max-len, tobj);
			strncatf(out+len, max-len-1, " }");
			len+=strlen(out+len);
			ent++;
		} else if (tobj->val->type==NT_NUMBER) {
			strncatf(out+len, max-len-1, "%s=", tobj->name);
			len+=strlen(out+len);
			p=nes_tostr(N, tobj);
			for (i=0;*p;i++) out[len++]=*p++;
			ent++;
		} else {
			strncatf(out+len, max-len-1, "%s=''", tobj->name);
			len+=strlen(out+len);
			p=nes_tostr(N, tobj);
			for (i=0;*p;i++) {
				switch (*p) {
				case '\\': strncatf(out+len, max-len-1, "\\\\"); break;
				case '\"': strncatf(out+len, max-len-1, "\\\""); break;
				case '\'': strncatf(out+len, max-len-1, "\\\'\'"); break;
				case '\r': break;
				case '\n': strncatf(out+len, max-len-1, "\\n"); break;
				default: out[len]=*p; break;
				}
				p++;
				len+=strlen(out+len);
			}
			strncatf(out+len, max-len-1, "''");
			len+=strlen(out+len);
			ent++;
		}
	}
	return len;
}

int ldir_saveentry(CONN *sid, int id, char *oc, obj_t **qobj)
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
	time_t t=time(NULL);

	if ((query=calloc(MAX_TUPLE_SIZE, sizeof(char)))==NULL) return -1;
	tobj=nes_getobj(proc->N, *qobj, "_rows");
	if (!nes_istable(tobj)) { free(query); prints(sid, "broken table"); return -1; }
	tobj=nes_getiobj(proc->N, tobj, 0);
	if (!nes_istable(tobj)) { free(query); prints(sid, "broken table"); return -1; }
	pid=(int)nes_getnum(proc->N, tobj, "pid");
	did=(int)nes_getnum(proc->N, tobj, "did");
	dobj=nes_settable(proc->N, tobj, "_data");
	if (strcmp(oc, "person")==0) {
		cobj=nes_getobj(sid->N, dobj, "uid");
	} else {
		cobj=nes_getobj(sid->N, dobj, "cn");
		if (nes_isnull(cobj)) cobj=nes_getobj(sid->N, dobj, "name");
		if (nes_isnull(cobj)) cobj=nes_getobj(sid->N, tobj, "name");
	}
	name=nes_tostr(sid->N, cobj);
//	memset(curdate, 0, sizeof(curdate));
//	time_unix2sql(curdate, sizeof(curdate)-1, t);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (id==0) {
//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
		len=snprintf(query, MAX_TUPLE_SIZE-1, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', '%s', '%s', '{ ", pid, did, curdate, curdate, oc, name);
		len+=dump_sub(sid->N, query+len, MAX_TUPLE_SIZE-len, dobj);
		strncatf(query+len, MAX_TUPLE_SIZE-len-1, "}');");
		len+=strlen(query+len);
		id=sql_update(proc->N, query);
		if (id<0) id=-1;
	} else {
//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
		len=snprintf(query, MAX_TUPLE_SIZE-1, "UPDATE nullgs_entries SET pid=%d, did=%d, mtime='%s', class='%s', name='%s', data='{ ", pid, did, curdate, oc, name);
		len+=dump_sub(sid->N, query+len, MAX_TUPLE_SIZE-len, dobj);
		strncatf(query+len, MAX_TUPLE_SIZE-len-1, "}' WHERE id=%d;", id);
		len+=strlen(query+len);
		rc=sql_update(proc->N, query);
		if (rc<0) id=-1;
	}
//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	prints(sid, "<BR /><B>[id=%d][len=%d]<BR>%s</B>", id, len, query);
//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	free(query);
//log_error(proc->N, "shit", __FILE__, __LINE__, 1, "[%s:%d]", __FILE__, __LINE__);
	return id;
}

obj_t *ldir_getlist(nes_state *N, char *oc, int pid, int did)
{
	obj_t *qobj1=NULL;
	obj_t *tobj;
	int rc=-1;
	int i;
	char *p;

	if (oc==NULL) oc="";
//	if ((priv&A_ADMIN)) {
//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND obj_did = %d ORDER BY bookmarkname ASC", pid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT id, name, data FROM nullgs_entries WHERE class = 'bookmark' AND pid = %d AND did = %d ORDER BY name ASC", pid, sid->dat->did)<0) return;
//	} else {
//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", pid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		if (pid) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND pid = %d AND did = %d ORDER BY name ASC", oc, pid, did);
		} else if (strlen(oc)>0) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d ORDER BY name ASC", oc, did);
		} else {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE did = %d ORDER BY did, id ASC", did);
		}
//	}
	if (rc<0) return NULL;
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		tobj=nes_getobj(proc->N, qobj1, "_rows");
		tobj=nes_getiobj(proc->N, tobj, i);
		tobj=nes_settable(proc->N, tobj, "_data");
		if ((p=sql_getvaluebyname(proc->N, &qobj1, i, "data"))==NULL) continue;
		nes_linkval(N, tobj, nes_eval(N, p));
		for (tobj=tobj->val->d.table;tobj;tobj=tobj->next) {
			if ((tobj->val->type==NT_STRING)&&(tobj->val->d.str)) {
				unescape(tobj->val->d.str, tobj->val->d.str);
			}
		}
	}
	if (strcmp(oc, "weblogentry")==0) {
		nes_linkval(N, nes_settable(N, &N->g, "weblogentryresults"), qobj1);
		sort_bykey(N, nes_getobj(N, qobj1, "_rows"), "ctime", NULL, 1);
//	} else if (strcmp(oc, "person")==0) {
//		ldir_sortlist(N, qobj1, "sn", "_data", 1);
	}
	return qobj1;
}

void ldir_sortlist(nes_state *N, obj_t *qobj, char *key, char *subtab, int order)
{
	obj_t *tobj;
	unsigned int i;

	tobj=nes_getobj(N, qobj, "_rows");
	if (!nes_istable(tobj)) return;
	sort_bykey(N, tobj, "sn", "_data", order);
	for (i=0,tobj=tobj->val->d.table;tobj;i++,tobj=tobj->next) {
		sprintf(tobj->name, "%d", i);
	}
	return;
}

obj_t *ldir_getentry(nes_state *N, char *oc, char *name, int id, int did)
{
	obj_t *qobj1=NULL;
	obj_t *tobj;
	int rc=-1;
	int i;
	char *p;

	if (strcmp(oc, "person")==0) {
		if (id) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		} else if (name) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	} else if (strcmp(oc, "dbquery")==0) {
		if (id) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		} else if (name) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	} else if (strcmp(oc, "bookmarkfolder")==0) {
		if (id) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		} else if (name) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	} else if (strcmp(oc, "bookmark")==0) {
		if (id) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		} else if (name) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	} else {
		if (id) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND id = %d;", oc, did, id);
		} else if (name) {
			rc=sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE class = '%s' AND did = %d AND name = '%s';", oc, did, name);
		}
	}
	if (sql_numtuples(proc->N, &qobj1)!=1) {
		sql_freeresult(proc->N, &qobj1);
		return NULL;
	}
	if (rc<0) return NULL;
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		tobj=nes_getobj(proc->N, qobj1, "_rows");
		tobj=nes_getiobj(proc->N, tobj, i);
		tobj=nes_settable(proc->N, tobj, "_data");
		if ((p=sql_getvaluebyname(proc->N, &qobj1, i, "data"))==NULL) continue;
		nes_linkval(N, tobj, nes_eval(N, p));
		for (tobj=tobj->val->d.table;tobj;tobj=tobj->next) {
			if ((tobj->val->type==NT_STRING)&&(tobj->val->d.str)) {
				unescape(tobj->val->d.str, tobj->val->d.str);
			}
		}
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

	tobj=nes_getobj(proc->N, *qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return "";
	tobj=nes_getiobj(proc->N, tobj, rowset);
	if (tobj->val->type!=NT_TABLE) return "";
	tobj2=nes_getobj(proc->N, tobj, "_data");
	if (tobj2->val->type==NT_TABLE) {
		for (cobj=tobj2->val->d.table; cobj; cobj=cobj->next) {
			if (strcasecmp(cobj->name, name)!=0) continue;
			return nes_tostr(proc->N, cobj);
		}
	}
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (strcasecmp(cobj->name, name)!=0) continue;
		return nes_tostr(proc->N, cobj);
	}
	return "";
}
