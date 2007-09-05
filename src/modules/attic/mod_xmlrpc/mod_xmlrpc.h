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
#include "nullgs/httpd_mod.h"

typedef struct {
	int contactid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int folderid;
	char username[51];
	char password[51];
	int enabled;
	short int geozone;
	short int timezone;
	char surname[51];
	char givenname[51];
	char salutation[51];
	char contacttype[51];
	char referredby[51];
	char altcontact[51];
	char prefbilling[51];
	char website[251];
	char email[251];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char mobilenumber[26];
	char jobtitle[51];
	char organization[51];
	char homeaddress[51];
	char homelocality[51];
	char homeregion[51];
	char homecountry[51];
	char homepostalcode[11];
	char workaddress[51];
	char worklocality[51];
	char workregion[51];
	char workcountry[51];
	char workpostalcode[11];
} REC_CONTACT;

/* mod_xmlrpc_auth.c */
int xmlrpc_auth_checkpass(CONN *sid, char *username, char *password);
int xmlrpc_auth_login(CONN *sid);
void xmlrpc_auth_logout(CONN *sid);
/* mod_xmlrpc_contacts.c */
void xmlrpc_contactread(CONN *sid, int contactid);
void xmlrpc_contactlist(CONN *sid);
void xmlrpc_contactwrite(CONN *sid);
/* mod_xmlrpc_main.c */
void xmlrpc_addmember(CONN *sid, char *name, char *type, const char *format, ...);
void xmlrpc_fault(CONN *sid, int faultid, char *fault);

typedef	int (*HTMOD_CONTACTS_READ)(CONN *, short int, int, REC_CONTACT *);
typedef	int (*HTMOD_CONTACTS_WRITE)(CONN *, int, REC_CONTACT *);
