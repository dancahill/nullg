/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

int xmlrpc_auth_checkpass(CONNECTION *sid, char *username, char *password)
{
	char cpassword[64];
	char salt[10];
	int contactid;
	int sqr;

	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) return -1;
	if ((sqr=sqlQueryf(sid, "SELECT contactid, password FROM gw_contacts WHERE username = '%s' and enabled > 0", username))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	contactid=atoi(sqlGetvalue(sqr, 0, 0));
	strncpy(cpassword, sqlGetvalue(sqr, 0, 1), sizeof(cpassword)-1);
	sqlFreeconnect(sqr);
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
		if (strcmp(cpassword, MD5Crypt(password, salt))!=0) return -1;
	} else {
		return -1;
	}
	return contactid;
}

int xmlrpc_auth_login(CONNECTION *sid)
{
	MD5_CTX c;
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
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing username");
		return -1;
	} else if (getxmlparam(sid, 4, "string")==NULL) {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing password");
		return -1;
	} else if (getxmlparam(sid, 5, "string")==NULL) {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		xmlrpc_fault(sid, -1, "Missing loginip");
		return -1;
	}
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	memset(raddress, 0, sizeof(raddress));
	if ((getxmlparam(sid, 3, "string")!=NULL)&&(getxmlparam(sid, 4, "string")!=NULL)) {
		strncpy(username, getxmlparam(sid, 3, "string"), sizeof(username)-1);
		strncpy(password, getxmlparam(sid, 4, "string"), sizeof(password)-1);
		strncpy(raddress, getxmlparam(sid, 5, "string"), sizeof(raddress)-1);
		if (strlen(password)==32) {
			if ((strlen(username)==0)||(strlen(sid->dat->user_token)!=32)) return -1;
			if ((sqr=sqlQueryf(sid, "SELECT loginip, logintoken, contactid FROM gw_contacts WHERE username = '%s'", username))<0) {
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (sqlNumtuples(sqr)!=1) {
				sqlFreeconnect(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(raddress, sqlGetvalue(sqr, 0, 0))!=0) {
				sqlFreeconnect(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(password, sqlGetvalue(sqr, 0, 1))!=0) {
				sqlFreeconnect(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			contactid=atoi(sqlGetvalue(sqr, 0, 2));
			sqlFreeconnect(sqr);
			memset(password, 0, sizeof(password));
			return contactid;
		} else {
			if ((contactid=xmlrpc_auth_checkpass(sid, username, password))<1) {
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, time(NULL)));
			MD5Init(&c);
			MD5Update(&c, username, strlen(username));
			MD5Update(&c, timebuffer, strlen(timebuffer));
			MD5Final(&(md[0]),&c);
			memset(token, 0, sizeof(token));
			for (i=0;i<MD5_SIZE;i++) strcatf(token, "%02x", md[i]);
			sqlUpdatef(sid, "UPDATE gw_contacts SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", raddress, timebuffer, token, username);
			memset(password, 0, sizeof(password));
			return contactid;
		}
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	xmlrpc_fault(sid, -1, "Authentication failure");
	return -1;
}
/*
void xmlrpc_auth_logout(CONNECTION *sid)
{
	time_t t;
	char timebuffer[100];

	sqlUpdatef(sid, "UPDATE gw_contacts SET logintoken='NULL' WHERE username = '%s'", sid->dat->user_username);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
	snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=NULL; path=/");
	printlogout(sid);
}
*/

void xmlrpc_fault(CONNECTION *sid, int faultid, char *fault)
{
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n");
	prints(sid, "<fault>\r\n");
	prints(sid, "<value>\r\n");
	prints(sid, "<struct>\r\n");
	prints(sid, "<member><name>faultCode</name><value><int>%d</int></value></member>\r\n", faultid);
	prints(sid, "<member><name>faultString</name><value><string>%s</string></value></member>\r\n", fault);
	prints(sid, "</struct>\r\n");
	prints(sid, "</value>\r\n");
	prints(sid, "</fault>\r\n");
	prints(sid, "</methodResponse>\r\n");
	return;
}

void xmlrpc_contactread(CONNECTION *sid, int contactid)
{
	REC_CONTACT contact;
	char timebuf[20];

	send_header(sid, 0, 200, "OK", "1", "text/xml", -1, -1);
	if (!(auth_priv(sid, AUTH_CONTACTS)&A_READ)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (db_read(sid, 2, DB_CONTACTS, contactid, &contact)!=0) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n");
	prints(sid, "<params>\r\n");
	prints(sid, "<param>\r\n");
	prints(sid, "<value>\r\n");
	prints(sid, "<struct>\r\n");
	prints(sid, "<member><name>contactid</name><value><int>%d</int></value></member>\r\n", contact.contactid);
	strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&contact.obj_ctime));
	prints(sid, "<member><name>obj_ctime</name><value><dateTime.iso8601>%s</dateTime.iso8601></value></member>\r\n", timebuf);
	strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&contact.obj_mtime));
	prints(sid, "<member><name>obj_mtime</name><value><dateTime.iso8601>%s</dateTime.iso8601></value></member>\r\n", timebuf);
	prints(sid, "<member><name>obj_uid</name><value><int>%d</int></value></member>\r\n", contact.obj_uid);
	prints(sid, "<member><name>obj_gid</name><value><int>%d</int></value></member>\r\n", contact.obj_gid);
	prints(sid, "<member><name>obj_gperm</name><value><int>%d</int></value></member>\r\n", contact.obj_gperm);
	prints(sid, "<member><name>obj_operm</name><value><int>%d</int></value></member>\r\n", contact.obj_operm);
	prints(sid, "<member><name>loginip</name><value><string>%s</string></value></member>\r\n", contact.loginip);
	strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&contact.logintime));
	prints(sid, "<member><name>logintime</name><value><dateTime.iso8601>%s</dateTime.iso8601></value></member>\r\n", timebuf);
	prints(sid, "<member><name>logintoken</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.logintoken));
	prints(sid, "<member><name>username</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.username));
	prints(sid, "<member><name>password</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.password));
	prints(sid, "<member><name>enabled</name><value><int>%d</int></value></member>\r\n", contact.enabled);
	prints(sid, "<member><name>geozone</name><value><int>%d</int></value></member>\r\n", contact.geozone);
	prints(sid, "<member><name>timezone</name><value><int>%d</int></value></member>\r\n", contact.timezone);
	prints(sid, "<member><name>surname</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.surname));
	prints(sid, "<member><name>givenname</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.givenname));
	prints(sid, "<member><name>salutation</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.salutation));
	prints(sid, "<member><name>contacttype</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.contacttype));
	prints(sid, "<member><name>referredby</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.referredby));
	prints(sid, "<member><name>altcontact</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.altcontact));
	prints(sid, "<member><name>prefbilling</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.prefbilling));
	prints(sid, "<member><name>email</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.email));
	prints(sid, "<member><name>homenumber</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homenumber));
	prints(sid, "<member><name>worknumber</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.worknumber));
	prints(sid, "<member><name>faxnumber</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.faxnumber));
	prints(sid, "<member><name>mobilenumber</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.mobilenumber));
	prints(sid, "<member><name>jobtitle</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.jobtitle));
	prints(sid, "<member><name>organization</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.organization));
	prints(sid, "<member><name>homeaddress</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homeaddress));
	prints(sid, "<member><name>homelocality</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homelocality));
	prints(sid, "<member><name>homeregion</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homeregion));
	prints(sid, "<member><name>homecountry</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homecountry));
	prints(sid, "<member><name>homepostalcode</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.homepostalcode));
	prints(sid, "<member><name>workaddress</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.workaddress));
	prints(sid, "<member><name>worklocality</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.worklocality));
	prints(sid, "<member><name>workregion</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.workregion));
	prints(sid, "<member><name>workcountry</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.workcountry));
	prints(sid, "<member><name>workpostalcode</name><value><string>%s</string></value></member>\r\n", str2html(sid, contact.workpostalcode));
	prints(sid, "</struct>\r\n");
	prints(sid, "</value>\r\n");
	prints(sid, "</param>\r\n");
	prints(sid, "</params>\r\n");
	prints(sid, "</methodResponse>\r\n");
	return;
}

void xmlrpc_contactwrite(CONNECTION *sid)
{
	REC_CONTACT contact;
	char opassword[50];
	char *ptemp;
	int contactid;

	if (!(auth_priv(sid, AUTH_CONTACTS)&A_MODIFY)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) {
		xmlrpc_fault(sid, -1, "Incorrect Method");
		return;
	}
	if ((ptemp=getxmlstruct(sid, "CONTACTID", "INT"))==NULL) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	contactid=atoi(ptemp);
	if (db_read(sid, 2, DB_CONTACTS, contactid, &contact)!=0) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", contact.password);
	if (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN) {
		if ((ptemp=getxmlstruct(sid, "OBJ_UID", "INT"))!=NULL) contact.obj_uid=atoi(ptemp);
		if ((ptemp=getxmlstruct(sid, "OBJ_GID", "INT"))!=NULL) contact.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_CONTACTS)&A_ADMIN)||(contact.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getxmlstruct(sid, "OBJ_GPERM", "INT"))!=NULL) contact.obj_gperm=atoi(ptemp);
		if ((ptemp=getxmlstruct(sid, "OBJ_OPERM", "INT"))!=NULL) contact.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getxmlstruct(sid, "USERNAME",       "STRING"))!=NULL) snprintf(contact.username, sizeof(contact.username)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "PASSWORD",       "STRING"))!=NULL) snprintf(contact.password, sizeof(contact.password)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "ENABLED",        "INT"))!=NULL) contact.enabled=atoi(ptemp);
	if ((ptemp=getxmlstruct(sid, "GEOZONE",        "INT"))!=NULL) contact.geozone=atoi(ptemp);
	if ((ptemp=getxmlstruct(sid, "TIMEZONE",       "INT"))!=NULL) contact.timezone=atoi(ptemp);
	if ((ptemp=getxmlstruct(sid, "SURNAME",        "STRING"))!=NULL) snprintf(contact.surname, sizeof(contact.surname)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "GIVENNAME",      "STRING"))!=NULL) snprintf(contact.givenname, sizeof(contact.givenname)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "SALUTATION",     "STRING"))!=NULL) snprintf(contact.salutation, sizeof(contact.salutation)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "CONTACTTYPE",    "STRING"))!=NULL) snprintf(contact.contacttype, sizeof(contact.contacttype)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "REFERREDBY",     "STRING"))!=NULL) snprintf(contact.referredby, sizeof(contact.referredby)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "ALTCONTACT",     "STRING"))!=NULL) snprintf(contact.altcontact, sizeof(contact.altcontact)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "PREFBILLING",    "STRING"))!=NULL) snprintf(contact.prefbilling, sizeof(contact.prefbilling)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "EMAIL",          "STRING"))!=NULL) snprintf(contact.email, sizeof(contact.email)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMENUMBER",     "STRING"))!=NULL) snprintf(contact.homenumber, sizeof(contact.homenumber)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKNUMBER",     "STRING"))!=NULL) snprintf(contact.worknumber, sizeof(contact.worknumber)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "FAXNUMBER",      "STRING"))!=NULL) snprintf(contact.faxnumber, sizeof(contact.faxnumber)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "MOBILENUMBER",   "STRING"))!=NULL) snprintf(contact.mobilenumber, sizeof(contact.mobilenumber)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "JOBTITLE",       "STRING"))!=NULL) snprintf(contact.jobtitle, sizeof(contact.jobtitle)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "ORGANIZATION",   "STRING"))!=NULL) snprintf(contact.organization, sizeof(contact.organization)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMEADDRESS",    "STRING"))!=NULL) snprintf(contact.homeaddress, sizeof(contact.homeaddress)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMELOCALITY",   "STRING"))!=NULL) snprintf(contact.homelocality, sizeof(contact.homelocality)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMEREGION",     "STRING"))!=NULL) snprintf(contact.homeregion, sizeof(contact.homeregion)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMECOUNTRY",    "STRING"))!=NULL) snprintf(contact.homecountry, sizeof(contact.homecountry)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "HOMEPOSTALCODE", "STRING"))!=NULL) snprintf(contact.homepostalcode, sizeof(contact.homepostalcode)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKADDRESS",    "STRING"))!=NULL) snprintf(contact.workaddress, sizeof(contact.workaddress)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKLOCALITY",   "STRING"))!=NULL) snprintf(contact.worklocality, sizeof(contact.worklocality)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKREGION",     "STRING"))!=NULL) snprintf(contact.workregion, sizeof(contact.workregion)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKCOUNTRY",    "STRING"))!=NULL) snprintf(contact.workcountry, sizeof(contact.workcountry)-1, "%s", ptemp);
	if ((ptemp=getxmlstruct(sid, "WORKPOSTALCODE", "STRING"))!=NULL) snprintf(contact.workpostalcode, sizeof(contact.workpostalcode)-1, "%s", ptemp);
	if (contact.contactid==0) {
		if (!(auth_priv(sid, AUTH_CONTACTS)&A_INSERT)) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		if ((contact.contactid=db_write(sid, DB_CONTACTS, 0, &contact))<1) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		logaccess(sid, 1, "%s - %s added contact %d (xml-rpc)", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
	} else {
		if (!(auth_priv(sid, AUTH_CONTACTS)&A_MODIFY)) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		if (strcmp(opassword, contact.password)!=0) {
			snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		}
		if (db_write(sid, DB_CONTACTS, contactid, &contact)<1) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		logaccess(sid, 1, "%s - %s modified contact %d (xml-rpc)", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
	}
	xmlrpc_contactread(sid, contact.contactid);
	return;
}

void xmlrpc_main(CONNECTION *sid)
{
	char methodname[40];
	char *ptemp;
	int index;

	DEBUG_IN(sid, "xmlrpc_main()");
	if ((ptemp=getxmlenv(sid, "methodName"))==NULL) {
		if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/contacts/read", 22)==0) {
			if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) {
				xmlrpc_fault(sid, -1, "Missing contactid");
			}
			index=atoi(ptemp);
			xmlrpc_contactread(sid, index);
		} else if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/contacts/write", 23)==0) {
			xmlrpc_contactwrite(sid);
		} else {
			send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
			htpage_topmenu(sid, MENU_XMLRPC);
			prints(sid, "<BR>\r\n");
			htpage_footer(sid);
		}
		DEBUG_OUT(sid, "xmlrpc_main()");
		return;
	} else {
		memset(methodname, 0, sizeof(methodname));
		snprintf(methodname, sizeof(methodname)-1, "%s", ptemp);
		if (strncmp(methodname, "contacts.read", 13)==0) {
			if ((ptemp=getxmlparam(sid, 3, "int"))==NULL) {
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Missing contactid");
				DEBUG_OUT(sid, "xmlrpc_main()");
				return;
			}
			index=atoi(ptemp);
			xmlrpc_contactread(sid, index);
		} else if (strncmp(methodname, "contacts.login", 14)==0) {
			if ((index=xmlrpc_auth_login(sid))>0) {
				xmlrpc_contactread(sid, index);
			}
		} else if (strncmp(methodname, "contacts.write", 14)==0) {
			xmlrpc_contactwrite(sid);
		} else {
			send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
			xmlrpc_fault(sid, -1, "Invalid methodName");
		}
	}
	DEBUG_OUT(sid, "xmlrpc_main()");
	return;
}
