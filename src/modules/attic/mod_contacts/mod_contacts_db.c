/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_contacts.h"

int user_in_group(CONN *sid, int groupid)
{
	int rc=0;
//	SQLRES sqr;

//	if (sid->dat->gid==groupid) {
//		rc=1;
//	} else {
//		if (sql_queryf(&sqr, "SELECT * FROM gw_groups_members WHERE userid = %d AND groupid = %d AND obj_did = %d", sid->dat->uid, groupid, sid->dat->did)<0) return 0;
//		if (sql_numtuples(&sqr)>0) rc=1;
//		sql_freeresult(&sqr);
//	}
	return rc;
}

int dblist_contacts(CONN *sid, obj_t **qobj, char *searchfield, char *searchstring)
{
	if ((*qobj=ldir_getlist(sid->N, "person", 0, sid->dat->did))==NULL) return -1;
	return 0;
/*
	char query[256];
	int rc;

	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "SELECT * from gw_contacts WHERE ");
	if (strcasecmp(searchstring, "All")!=0) {
		strncatf(query, sizeof(query)-strlen(query)-1, "(lower(%s) like lower('%s%%')) AND ", searchfield, searchstring);
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "obj_did = %d ORDER BY surname, givenname ASC", sid->dat->did);
	if ((rc=sql_query(sqr, query))<0) {
		return EC_UNKNOWN;
	}
	sql_permprune(sid, sqr, "contacts");
	return rc;
*/
}

int dbread_contact(CONN *sid, short int perm, int index, obj_t **qobj)
{
	short int approved;
	int auth=auth_priv(sid, "contacts");
	obj_t *cobj, *tobj;

	if (auth<1) return EC_NOPERM;
	if (!(auth&A_MODIFY)&&(perm==2)) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Contact',labeleduri='http://'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return EC_NOERROR;
	}
	if ((tobj=ldir_getentry(sid->N, "person", NULL, index, sid->dat->did))==NULL) return EC_NORECORD;
	if (ldir_numentries(&tobj)!=1) { ldir_freeresult(&tobj); return EC_NORECORD; }
	if (*qobj) {
		cobj=*qobj;
		cobj->val->d.table=tobj->val->d.table;
	} else {
		*qobj=tobj;
	}
	/* this would be a good time to check record permissions..... */
/*
	contact->enabled=1;
	contact->timezone=sid->dat->timezone;
*/
	approved=0;
/*
	if (auth_priv(sid, "contacts")&A_ADMIN) approved=2;
	if (approved==0) {
		if (contact->obj_uid==sid->dat->uid) approved=2;
	}
	if ((approved==0)&&(contact->obj_gperm>=perm)) {
		if (user_in_group(sid, contact->obj_gid)) {
			approved=contact->obj_gperm;
		}
	}
	if ((approved==0)&&(contact->obj_operm>=perm)) {
		approved=contact->obj_operm;
	}
	if (approved==0) {
		memset(contact, 0, sizeof(REC_CONTACT));
		return EC_NOPERM;
	}
*/
	return approved;
}

int dbwrite_contact(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "contacts");

	if (auth<2) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	return ldir_saveentry(sid, index, "person", qobj);
}
