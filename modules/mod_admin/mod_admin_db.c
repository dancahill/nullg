/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_admin.h"

int dbread_group(CONN *sid, short int perm, int index, REC_GROUP *group)
{
	int authlevel;
	int sqr;

	memset(group, 0, sizeof(REC_GROUP));
	authlevel=auth_priv(sid, "admin");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
//	if ((perm>1)&&(table==DB_PRODUCTS)&&(!(authlevel&A_ADMIN))) return -1;
	if (index==0) {
		group->obj_uid=sid->dat->user_uid;
		group->obj_gid=sid->dat->user_gid;
		group->obj_gperm=1;
		group->obj_operm=1;
		strncpy(group->availability, "000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000", sizeof(group->availability)-1);
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_groups where groupid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_groups where groupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	group->groupid   = atoi(sql_getvalue(sqr, 0, 0));
	group->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	group->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	group->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	group->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	group->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	group->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	group->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(group->groupname,	sql_getvalue(sqr, 0, 8), sizeof(group->groupname)-1);
	strncpy(group->availability,	sql_getvalue(sqr, 0, 9), sizeof(group->availability)-1);
	strncpy(group->motd,		sql_getvalue(sqr, 0, 10), sizeof(group->motd)-1);
	strncpy(group->members,		sql_getvalue(sqr, 0, 11), sizeof(group->members)-1);
	if (strlen(group->availability)==0) {
		strncpy(group->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(group->availability)-1);
	}
	sql_freeresult(sqr);
	return 0;
}

int dbread_user(CONN *sid, short int perm, int index, REC_USER *user)
{
	int authlevel;
	int sqr;

	memset(user, 0, sizeof(REC_USER));
	authlevel=auth_priv(sid, "admin");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
//	if ((perm>1)&&(table==DB_PRODUCTS)&&(!(authlevel&A_ADMIN))) return -1;
	if (index==0) {
		user->obj_uid=sid->dat->user_uid;
		user->obj_gid=sid->dat->user_gid;
		user->obj_gperm=1;
		user->obj_operm=1;
		user->groupid=2;
		user->enabled=1;
		user->authbookmarks=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authcalendar=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authcalls=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authcontacts=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authwebmail=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authfiles=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authforums=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authmessages=A_READ+A_INSERT+A_DELETE;
		user->authorders=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		user->authprofile=A_READ+A_MODIFY;
		user->authquery=A_READ;
		user->prefdaystart=0;
		user->prefdaylength=24;
		user->prefmailcurrent=0;
		user->prefmaildefault=0;
		user->prefmaxlist=15;
		user->prefmenustyle=1;
		user->preftimezone=10;
		user->prefgeozone=1;
		strncpy(user->availability, "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", sizeof(user->availability)-1);
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_users where userid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_users where userid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	user->userid   = atoi(sql_getvalue(sqr, 0, 0));
	user->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	user->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	user->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	user->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	user->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	user->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	user->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(user->loginip,          sql_getvalue(sqr, 0, 8), sizeof(user->loginip)-1);
	user->logintime=time_sql2unix(sql_getvalue(sqr, 0, 9));
	strncpy(user->logintoken,       sql_getvalue(sqr, 0, 10), sizeof(user->logintoken)-1);
	strncpy(user->username,         sql_getvalue(sqr, 0, 11), sizeof(user->username)-1);
	strncpy(user->password,         sql_getvalue(sqr, 0, 12), sizeof(user->password)-1);
	user->groupid=atoi(sql_getvalue(sqr, 0, 13));
	user->enabled=atoi(sql_getvalue(sqr, 0, 14));
	user->authadmin=atoi(sql_getvalue(sqr, 0, 15));
	user->authbookmarks=atoi(sql_getvalue(sqr, 0, 16));
	user->authcalendar=atoi(sql_getvalue(sqr, 0, 17));
	user->authcalls=atoi(sql_getvalue(sqr, 0, 18));
	user->authcontacts=atoi(sql_getvalue(sqr, 0, 19));
	user->authfiles=atoi(sql_getvalue(sqr, 0, 20));
	user->authforums=atoi(sql_getvalue(sqr, 0, 21));
	user->authmessages=atoi(sql_getvalue(sqr, 0, 22));
	user->authorders=atoi(sql_getvalue(sqr, 0, 23));
	user->authprofile=atoi(sql_getvalue(sqr, 0, 24));
	user->authquery=atoi(sql_getvalue(sqr, 0, 25));
	user->authwebmail=atoi(sql_getvalue(sqr, 0, 26));
	user->prefdaystart=atoi(sql_getvalue(sqr, 0, 27));
	user->prefdaylength=atoi(sql_getvalue(sqr, 0, 28));
	user->prefmailcurrent=atoi(sql_getvalue(sqr, 0, 29));
	user->prefmaildefault=atoi(sql_getvalue(sqr, 0, 30));
	user->prefmaxlist=atoi(sql_getvalue(sqr, 0, 31));
	user->prefmenustyle=atoi(sql_getvalue(sqr, 0, 32));
	user->preftimezone=atoi(sql_getvalue(sqr, 0, 33));
	user->prefgeozone=atoi(sql_getvalue(sqr, 0, 34));
	strncpy(user->availability,     sql_getvalue(sqr, 0, 35), sizeof(user->availability)-1);
	strncpy(user->surname,          sql_getvalue(sqr, 0, 36), sizeof(user->surname)-1);
	strncpy(user->givenname,        sql_getvalue(sqr, 0, 37), sizeof(user->givenname)-1);
	strncpy(user->jobtitle,         sql_getvalue(sqr, 0, 38), sizeof(user->jobtitle)-1);
	strncpy(user->division,         sql_getvalue(sqr, 0, 39), sizeof(user->division)-1);
	strncpy(user->supervisor,       sql_getvalue(sqr, 0, 40), sizeof(user->supervisor)-1);
	strncpy(user->address,          sql_getvalue(sqr, 0, 41), sizeof(user->address)-1);
	strncpy(user->locality,         sql_getvalue(sqr, 0, 42), sizeof(user->locality)-1);
	strncpy(user->region,           sql_getvalue(sqr, 0, 43), sizeof(user->region)-1);
	strncpy(user->country,          sql_getvalue(sqr, 0, 44), sizeof(user->country)-1);
	strncpy(user->postalcode,       sql_getvalue(sqr, 0, 45), sizeof(user->postalcode)-1);
	strncpy(user->homenumber,       sql_getvalue(sqr, 0, 46), sizeof(user->homenumber)-1);
	strncpy(user->worknumber,       sql_getvalue(sqr, 0, 47), sizeof(user->worknumber)-1);
	strncpy(user->faxnumber,        sql_getvalue(sqr, 0, 48), sizeof(user->faxnumber)-1);
	strncpy(user->cellnumber,       sql_getvalue(sqr, 0, 49), sizeof(user->cellnumber)-1);
	strncpy(user->pagernumber,      sql_getvalue(sqr, 0, 50), sizeof(user->pagernumber)-1);
	strncpy(user->email,            sql_getvalue(sqr, 0, 51), sizeof(user->email)-1);
	strncpy(user->birthdate,        sql_getvalue(sqr, 0, 52), 10);
	strncpy(user->hiredate,         sql_getvalue(sqr, 0, 53), 10);
	strncpy(user->sin,              sql_getvalue(sqr, 0, 54), sizeof(user->sin)-1);
	strncpy(user->isactive,         sql_getvalue(sqr, 0, 55), sizeof(user->isactive)-1);
	if (strlen(user->availability)==0) {
		strncpy(user->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(user->availability)-1);
	}
	if (user->authadmin&A_ADMIN) user->authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authbookmarks&A_ADMIN) user->authbookmarks=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authcalendar&A_ADMIN) user->authcalendar=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authcalls&A_ADMIN) user->authcalls=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authcontacts&A_ADMIN) user->authcontacts=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authfiles&A_ADMIN) user->authfiles=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authforums&A_ADMIN) user->authforums=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authmessages&A_ADMIN) user->authmessages=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authorders&A_ADMIN) user->authorders=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authprofile&A_ADMIN) user->authprofile=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authquery&A_ADMIN) user->authquery=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->authwebmail&A_ADMIN) user->authwebmail=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (user->prefdaylength+user->prefdaystart>24) user->prefdaylength=24-user->prefdaystart;
	sql_freeresult(sqr);
	return 0;
}

int dbread_zone(CONN *sid, short int perm, int index, REC_ZONE *zone)
{
	int authlevel;
	int sqr;

	memset(zone, 0, sizeof(REC_ZONE));
	authlevel=auth_priv(sid, "admin");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
//	if ((perm>1)&&(table==DB_PRODUCTS)&&(!(authlevel&A_ADMIN))) return -1;
	if (index==0) {
		zone->obj_uid=sid->dat->user_uid;
		zone->obj_gid=sid->dat->user_gid;
		zone->obj_gperm=1;
		zone->obj_operm=1;
		snprintf(zone->zonename, sizeof(zone->zonename)-1, "New Zone");
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_zones where zoneid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_zones where zoneid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	zone->zoneid    = atoi(sql_getvalue(sqr, 0, 0));
	zone->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	zone->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	zone->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	zone->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	zone->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	zone->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	zone->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(zone->zonename, sql_getvalue(sqr, 0, 8), sizeof(zone->zonename)-1);
	sql_freeresult(sqr);
	return 0;
}
