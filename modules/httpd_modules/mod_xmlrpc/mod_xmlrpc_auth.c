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
#include "mod_xmlrpc.h"

int xmlrpc_auth_checkpass(CONN *sid, char *username, char *password)
{
	char cpassword[64];
	char cpass[64];
	char salt[10];
	short int i;
	int contactid;
	int sqr;

	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) return -1;
	if ((sqr=sql_queryf("SELECT contactid, password FROM gw_contacts WHERE username = '%s' AND obj_did = %d AND enabled > 0", username, sid->dat->user_did))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -1;
	}
	contactid=atoi(sql_getvalue(sqr, 0, 0));
	strncpy(cpassword, sql_getvalue(sqr, 0, 1), sizeof(cpassword)-1);
	sql_freeresult(sqr);
	memset(salt, 0, sizeof(salt));
	memset(cpass, 0, sizeof(cpass));
	if (strncmp(cpassword, "$1$", 3)==0) {
		for (i=0;i<8;i++) salt[i]=cpassword[i+3];
		md5_crypt(cpass, password, salt);
		if (strcmp(cpassword, cpass)!=0) return -1;
	} else {
		return -1;
	}
	return contactid;
}

int xmlrpc_auth_login(CONN *sid)
{
	MD5_CONTEXT c;
	unsigned char md[MD5_SIZE];
	char timebuffer[100];
	char username[64];
	char password[64];
	char raddress[64];
	char token[64];
	int contactid;
	int i;
	int sqr;

	if (getxmlparam(sid, 3, "string")==NULL) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing username");
		return -1;
	} else if (getxmlparam(sid, 4, "string")==NULL) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing password");
		return -1;
	} else if (getxmlparam(sid, 5, "string")==NULL) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing loginip");
		return -1;
	}
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	memset(raddress, 0, sizeof(raddress));
	if ((getxmlparam(sid, 3, "string")!=NULL)&&(getxmlparam(sid, 4, "string")!=NULL)&&(getxmlparam(sid, 5, "string")!=NULL)) {
		strncpy(username, getxmlparam(sid, 3, "string"), sizeof(username)-1);
		strncpy(password, getxmlparam(sid, 4, "string"), sizeof(password)-1);
		strncpy(raddress, getxmlparam(sid, 5, "string"), sizeof(raddress)-1);
		if (strlen(password)==32) {
			if ((strlen(username)==0)||(strlen(sid->dat->user_token)!=32)) return -1;
			if ((sqr=sql_queryf("SELECT loginip, logintoken, contactid FROM gw_contacts WHERE username = '%s' AND obj_did = %d", username, sid->dat->user_did))<0) {
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (sql_numtuples(sqr)!=1) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(raddress, sql_getvalue(sqr, 0, 0))!=0) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(password, sql_getvalue(sqr, 0, 1))!=0) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			contactid=atoi(sql_getvalue(sqr, 0, 2));
			sql_freeresult(sqr);
			memset(password, 0, sizeof(password));
			return contactid;
		} else {
			if ((contactid=xmlrpc_auth_checkpass(sid, username, password))<1) {
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			time_unix2sql(timebuffer, sizeof(timebuffer)-1, time(NULL));
			md5_init(&c);
			md5_update(&c, username, strlen(username));
			md5_update(&c, timebuffer, strlen(timebuffer));
			md5_final(&(md[0]),&c);
			memset(token, 0, sizeof(token));
			for (i=0;i<MD5_SIZE;i++) strncatf(token, sizeof(token)-strlen(token)-1, "%02x", md[i]);
			sql_updatef("UPDATE gw_contacts SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s' AND obj_did = %d", raddress, timebuffer, token, username, sid->dat->user_did);
			memset(password, 0, sizeof(password));
			return contactid;
		}
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	xmlrpc_fault(sid, -1, "Authentication failure");
	return -1;
}
/*
void xmlrpc_auth_logout(CONN *sid)
{
	time_t t;
	char timebuffer[100];

	sql_updatef("UPDATE gw_contacts SET logintoken='NULL' WHERE username = '%s'", sid->dat->user_username);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
	snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=NULL; path=/");
	printlogout(sid);
}
*/
