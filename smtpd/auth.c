/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "smtp_main.h"

static int auth_checkpass(CONN *sid, char *rpassword, char *cpassword)
{
	char salt[10];

	DEBUG_IN(sid, "auth_checkpass()");
	memset(salt, 0, sizeof(salt));
	if (strncmp(cpassword, "$1$", 3)==0) {
		salt[0]=cpassword[3];
		salt[1]=cpassword[4];
		salt[2]=cpassword[5];
		salt[3]=cpassword[6];
		salt[4]=cpassword[7];
		salt[5]=cpassword[8];
		salt[6]=cpassword[9];
		salt[7]=cpassword[10];
		if (strcmp(cpassword, md5_crypt(sid, rpassword, salt))!=0) {
			DEBUG_OUT(sid, "auth_checkpass()");
			return -1;
		}
	} else {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	DEBUG_OUT(sid, "auth_checkpass()");
	return 0;
}

int auth_login(CONN *sid, char *username, char *password, int mbox)
{
	int i;
	int sqr;

	DEBUG_IN(sid, "auth_login()");
	if (strlen(username)==0) return -1;
	if ((sqr=sql_queryf(sid, "SELECT * FROM gw_users WHERE username = '%s'", username))<0) {
		DEBUG_OUT(sid, "auth_login()");
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		DEBUG_OUT(sid, "auth_login()");
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
		sid->dat->user_mailcurrent=0;
		DEBUG_OUT(sid, "auth_login()");
		return 0;
	}
	if ((sqr=sql_queryf(sid, "SELECT mailaccountid, accountname FROM gw_mailaccounts WHERE mailaccountid = %d AND obj_uid = %d order by mailaccountid ASC", mbox, sid->dat->user_uid))<0) {
		DEBUG_OUT(sid, "auth_login()");
		return -1;
	}
	i=sql_numtuples(sqr);
	sql_freeresult(sqr);
	if (i!=1) {
		DEBUG_OUT(sid, "auth_login()");
		return -1;
	}
	sid->dat->user_mailcurrent=mbox;
	DEBUG_OUT(sid, "auth_login()");
	return 0;
}
