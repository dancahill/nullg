/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "mod_weblog.h"

int dblist_weblog_comments(CONN *sid, obj_t **qobj, int entryid)
{
//	char query[256];
	int rc;

//	memset(query, 0, sizeof(query));
//	snprintf(query, sizeof(query)-1, "SELECT * from gw_weblog_comments WHERE entryid = %d AND obj_did = %d ORDER BY obj_ctime ASC", entryid, sid->dat->did);
	rc=sql_queryf(proc->N, qobj, "SELECT * from nullgs_entries WHERE class = 'weblogcomment' AND pid = %d AND did = %d ORDER BY id ASC", entryid, sid->dat->did);
	if (rc<0) return EC_UNKNOWN;
//	sql_permprune(sid, qobj, NULL);
	return rc;
}

int dblist_weblog_entries(CONN *sid, obj_t **qobj, int userid, int groupid)
{
	char query[256];
	int rc;

//	if ((qobj=ldir_getlist(sid->N, "weblogentry", pid, sid->dat->did))==NULL) return;
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "SELECT * from nullgs_entries WHERE class = 'weblogentry' AND ");
//	if (userid>0) {
//		strncatf(query, sizeof(query)-strlen(query)-1, "obj_uid = %d AND ", userid);
//	}
//	if (groupid>0) {
//		strncatf(query, sizeof(query)-strlen(query)-1, "entrygroupid = %d AND ", groupid);
//	}
	strncatf(query, sizeof(query)-strlen(query)-1, "did = %d ORDER BY id DESC", sid->dat->did);
	rc=sql_query(proc->N, qobj, query);
	if (rc<0) return EC_UNKNOWN;
//	sql_permprune(sid, sqr, NULL);
	return rc;
}

int dbread_weblog_comment(CONN *sid, short int perm, int index, int entryindex, obj_t **qobj)
{
	int auth=auth_priv(sid, "admin");
	obj_t *cobj, *tobj;

	if (auth<1) return -1;
	if (!(auth&A_MODIFY)&&(perm==2)) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Weblog Comment'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return 0;
	}
	if ((*qobj=ldir_getentry(sid->N, "weblogcomment", NULL, index, sid->dat->did))==NULL) return -1;
	if (ldir_numentries(qobj)!=1) { ldir_freeresult(qobj); return -2; }
	return 0;
}

int dbread_weblog_entry(CONN *sid, short int perm, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "admin");
	obj_t *cobj, *tobj;

	if (auth<1) return -1;
	if (!(auth&A_MODIFY)&&(perm==2)) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Weblog Entry'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return 0;
	}
	if ((*qobj=ldir_getentry(sid->N, "weblogentry", NULL, index, sid->dat->did))==NULL) return -1;
	if (ldir_numentries(qobj)!=1) { ldir_freeresult(qobj); return -2; }
	return 0;
}

int dbwrite_weblog_comment(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "admin");

	if (auth<2) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	return ldir_saveentry(sid, index, "weblogcomment", qobj);
}

int dbwrite_weblog_entry(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "admin");

	if (auth<2) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	return ldir_saveentry(sid, index, "weblogentry", qobj);
}
