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
#include "pop3d_main.h"

static int auth_checkpass(CONN *sid, char *rpassword, char *cpassword)
{
	char cpass[64];
	char salt[10];
	short int i;

	memset(salt, 0, sizeof(salt));
	if (strncmp(cpassword, "$1$", 3)==0) {
		for (i=0;i<8;i++) salt[i]=cpassword[i+3];
		md5_crypt(cpass, rpassword, salt);
		if (strcmp(cpassword, cpass)!=0) {
			memset(cpass, 0, sizeof(cpass));
			return -1;
		}
		memset(cpass, 0, sizeof(cpass));
	} else {
		return -1;
	}
	return 0;
}

int auth_login(CONN *sid, char *username, char *domain, char *password, int mbox)
{
	int domainid;
	int i;
	int sqr;

	if (strlen(username)==0) return -1;

	domainid=domain_getid(domain);
	if (domainid<0) domainid=1;
	sid->dat->user_did=domainid;

	if ((sqr=sql_queryf("SELECT * FROM gw_users WHERE username = '%s' AND domainid = %d", username, domainid))<0) {
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -1;
	}
	if (auth_checkpass(sid, password, sql_getvaluebyname(sqr, 0, "password"))!=0) {
		sql_freeresult(sqr);
		return -1;
	}
	sid->dat->user_uid = atoi(sql_getvaluebyname(sqr, 0, "userid"));
	sid->dat->user_gid = atoi(sql_getvaluebyname(sqr, 0, "groupid"));
	sql_freeresult(sqr);
	if (mbox==0) {
		snprintf(sid->dat->user_username, sizeof(sid->dat->user_username)-1, "%s", username);
		sid->dat->user_mailcurrent=0;
		return 0;
	}
	if ((sqr=sql_queryf("SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE mailaccountid = %d AND obj_uid = %d AND obj_did = %d ORDER BY mailaccountid ASC", mbox, sid->dat->user_uid, sid->dat->user_did))<0) {
		return -1;
	}
	i=sql_numtuples(sqr);
	sql_freeresult(sqr);
	if (i!=1) {
		return -1;
	}
	sid->dat->user_mailcurrent=mbox;
	return 0;
}
