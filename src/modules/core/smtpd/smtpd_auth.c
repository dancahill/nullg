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
#include "smtpd_main.h"

static int auth_checkpass(char *rpassword, char *cpassword)
{
	char cpass[64];
	char salt[10];

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

int auth_login(CONN *sid, char *username, char *password, int mbox)
{
	int i;
	obj_t *qptr=NULL;

	if (strlen(username)==0) return -1;
	if (sql_queryf(proc->N, &qptr, "SELECT * FROM gw_users WHERE username = '%s'", username)<0) {
		return -1;
	}
	if (sql_numtuples(proc->N, &qptr)!=1) {
		sql_freeresult(proc->N, &qptr);
		return -1;
	}
	if (auth_checkpass(password, sql_getvaluebyname(proc->N, &qptr, 0, "password"))!=0) {
		sql_freeresult(proc->N, &qptr);
		return -1;
	}
	sid->dat->uid = atoi(sql_getvaluebyname(proc->N, &qptr, 0, "userid"));
	sid->dat->gid = atoi(sql_getvaluebyname(proc->N, &qptr, 0, "groupid"));
	sql_freeresult(proc->N, &qptr);
	if (mbox==0) {
		snprintf(sid->dat->username, sizeof(sid->dat->username)-1, "%s", username);
		sid->dat->mailcurrent=0;
		return 0;
	}
	if (sql_queryf(proc->N, &qptr, "SELECT mailaccountid, accountname FROM gw_email_accounts WHERE mailaccountid = %d AND obj_uid = %d order by mailaccountid ASC", mbox, sid->dat->uid)<0) {
		return -1;
	}
	i=sql_numtuples(proc->N, &qptr);
	sql_freeresult(proc->N, &qptr);
	if (i!=1) {
		return -1;
	}
	sid->dat->mailcurrent=mbox;
	return 0;
}
