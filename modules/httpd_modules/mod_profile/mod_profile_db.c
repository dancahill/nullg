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
#include "mod_profile.h"

int dbread_profile(CONN *sid, short int perm, int index, REC_USER *profile)
{
	int authlevel;
	int sqr;

	memset(profile, 0, sizeof(REC_USER));
	authlevel=auth_priv(sid, "profile");
	if (authlevel&A_MODIFY) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	index=sid->dat->user_uid;
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_users where userid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_users where userid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	profile->userid    = atoi(sql_getvalue(sqr, 0, 0));
	profile->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	profile->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	profile->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	profile->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	profile->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	profile->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	profile->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(profile->username,         sql_getvalue(sqr, 0, 8), sizeof(profile->username)-1);
	strncpy(profile->password,         sql_getvalue(sqr, 0, 9), sizeof(profile->password)-1);
	profile->groupid		= atoi(sql_getvalue(sqr, 0, 10));
	profile->domainid		= atoi(sql_getvalue(sqr, 0, 11));
	profile->enabled		= atoi(sql_getvalue(sqr, 0, 12));
	profile->authdomainadmin	= atoi(sql_getvalue(sqr, 0, 13));
	profile->authadmin		= atoi(sql_getvalue(sqr, 0, 14));
	profile->authbookmarks		= atoi(sql_getvalue(sqr, 0, 15));
	profile->authcalendar		= atoi(sql_getvalue(sqr, 0, 16));
	profile->authcalls		= atoi(sql_getvalue(sqr, 0, 17));
	profile->authcontacts		= atoi(sql_getvalue(sqr, 0, 18));
	profile->authfiles		= atoi(sql_getvalue(sqr, 0, 19));
	profile->authforums		= atoi(sql_getvalue(sqr, 0, 20));
	profile->authmessages		= atoi(sql_getvalue(sqr, 0, 21));
	profile->authorders		= atoi(sql_getvalue(sqr, 0, 22));
	profile->authprofile		= atoi(sql_getvalue(sqr, 0, 23));
	profile->authprojects		= atoi(sql_getvalue(sqr, 0, 24));
	profile->authquery		= atoi(sql_getvalue(sqr, 0, 25));
	profile->authwebmail		= atoi(sql_getvalue(sqr, 0, 26));
	profile->prefdaystart		= atoi(sql_getvalue(sqr, 0, 27));
	profile->prefdaylength		= atoi(sql_getvalue(sqr, 0, 28));
	profile->prefmailcurrent	= atoi(sql_getvalue(sqr, 0, 29));
	profile->prefmaildefault	= atoi(sql_getvalue(sqr, 0, 30));
	profile->prefmaxlist		= atoi(sql_getvalue(sqr, 0, 31));
	profile->prefmenustyle		= atoi(sql_getvalue(sqr, 0, 32));
	profile->preftimezone		= atoi(sql_getvalue(sqr, 0, 33));
	profile->prefgeozone		= atoi(sql_getvalue(sqr, 0, 34));
	strncpy(profile->preflanguage,     sql_getvalue(sqr, 0, 35), sizeof(profile->preflanguage)-1);
	strncpy(profile->preftheme,        sql_getvalue(sqr, 0, 36), sizeof(profile->preftheme)-1);
	strncpy(profile->availability,     sql_getvalue(sqr, 0, 37), sizeof(profile->availability)-1);
	strncpy(profile->surname,          sql_getvalue(sqr, 0, 38), sizeof(profile->surname)-1);
	strncpy(profile->givenname,        sql_getvalue(sqr, 0, 39), sizeof(profile->givenname)-1);
	strncpy(profile->jobtitle,         sql_getvalue(sqr, 0, 40), sizeof(profile->jobtitle)-1);
	strncpy(profile->division,         sql_getvalue(sqr, 0, 41), sizeof(profile->division)-1);
	strncpy(profile->supervisor,       sql_getvalue(sqr, 0, 42), sizeof(profile->supervisor)-1);
	strncpy(profile->address,          sql_getvalue(sqr, 0, 43), sizeof(profile->address)-1);
	strncpy(profile->locality,         sql_getvalue(sqr, 0, 44), sizeof(profile->locality)-1);
	strncpy(profile->region,           sql_getvalue(sqr, 0, 45), sizeof(profile->region)-1);
	strncpy(profile->country,          sql_getvalue(sqr, 0, 46), sizeof(profile->country)-1);
	strncpy(profile->postalcode,       sql_getvalue(sqr, 0, 47), sizeof(profile->postalcode)-1);
	strncpy(profile->homenumber,       sql_getvalue(sqr, 0, 48), sizeof(profile->homenumber)-1);
	strncpy(profile->worknumber,       sql_getvalue(sqr, 0, 49), sizeof(profile->worknumber)-1);
	strncpy(profile->faxnumber,        sql_getvalue(sqr, 0, 50), sizeof(profile->faxnumber)-1);
	strncpy(profile->cellnumber,       sql_getvalue(sqr, 0, 51), sizeof(profile->cellnumber)-1);
	strncpy(profile->pagernumber,      sql_getvalue(sqr, 0, 52), sizeof(profile->pagernumber)-1);
	strncpy(profile->email,            sql_getvalue(sqr, 0, 53), sizeof(profile->email)-1);
	strncpy(profile->birthdate,        sql_getvalue(sqr, 0, 54), 10);
	strncpy(profile->hiredate,         sql_getvalue(sqr, 0, 55), 10);
	strncpy(profile->sin,              sql_getvalue(sqr, 0, 56), sizeof(profile->sin)-1);
	strncpy(profile->isactive,         sql_getvalue(sqr, 0, 57), sizeof(profile->isactive)-1);
	if (strlen(profile->availability)==0) {
		strncpy(profile->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(profile->availability)-1);
	}
	if (profile->authadmin&A_ADMIN) profile->authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authbookmarks&A_ADMIN) profile->authbookmarks=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authcalendar&A_ADMIN) profile->authcalendar=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authcalls&A_ADMIN) profile->authcalls=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authcontacts&A_ADMIN) profile->authcontacts=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authfiles&A_ADMIN) profile->authfiles=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authforums&A_ADMIN) profile->authforums=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authmessages&A_ADMIN) profile->authmessages=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authorders&A_ADMIN) profile->authorders=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authprofile&A_ADMIN) profile->authprofile=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authprojects&A_ADMIN) profile->authprojects=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authquery&A_ADMIN) profile->authquery=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->authwebmail&A_ADMIN) profile->authwebmail=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	if (profile->prefdaylength+profile->prefdaystart>24) profile->prefdaylength=24-profile->prefdaystart;
	sql_freeresult(sqr);
	return 0;
}
