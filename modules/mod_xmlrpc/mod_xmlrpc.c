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
#include "mod_stub.h"
#include <stdarg.h>

void xmlrpc_addmember(CONN *sid, char *name, char *type, const char *format, ...)
{
	char value[1024];
	char timebuf[20];
	va_list ap;
	time_t t;

	memset(value, 0, sizeof(value));
	va_start(ap, format);
	vsnprintf(value, sizeof(value)-1, format, ap);
	va_end(ap);
	if (strncasecmp(type, "dateTime", 8)==0) type="dateTime.iso8601";
	prints(sid, "<member><name>%s</name><value><%s>", name, type);
	if (strcasecmp("dateTime.iso8601", type)==0) {
		t=atoi(value);
		strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&t));
		prints(sid, "%s", timebuf);
	} else {
		prints(sid, "%s", str2html(sid, value));
	}
	prints(sid, "</%s></value></member>\r\n", type);
	return;
}

void xmlrpc_fault(CONN *sid, int faultid, char *fault)
{
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<fault>\r\n");
	prints(sid, "<value>\r\n<struct>\r\n");
	xmlrpc_addmember(sid, "faultCode",   "int",    "%d", faultid);
	xmlrpc_addmember(sid, "faultString", "string", "%s", fault);
	prints(sid, "</struct>\r\n</value>\r\n");
	prints(sid, "</fault>\r\n</methodResponse>\r\n");
	return;
}

int xmlrpc_auth_checkpass(CONN *sid, char *username, char *password)
{
	char cpassword[64];
	char salt[10];
	int contactid;
	int sqr;

	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) return -1;
	if ((sqr=sql_queryf(sid, "SELECT contactid, password FROM gw_contacts WHERE username = '%s' and enabled > 0", username))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -1;
	}
	contactid=atoi(sql_getvalue(sqr, 0, 0));
	strncpy(cpassword, sql_getvalue(sqr, 0, 1), sizeof(cpassword)-1);
	sql_freeresult(sqr);
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
		if (strcmp(cpassword, md5_crypt(sid, password, salt))!=0) return -1;
	} else {
		return -1;
	}
	return contactid;
}

int xmlrpc_auth_login(CONN *sid)
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
			if ((sqr=sql_queryf(sid, "SELECT loginip, logintoken, contactid FROM gw_contacts WHERE username = '%s'", username))<0) {
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (sql_numtuples(sqr)!=1) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(raddress, sql_getvalue(sqr, 0, 0))!=0) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			if (strcmp(password, sql_getvalue(sqr, 0, 1))!=0) {
				sql_freeresult(sqr);
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			contactid=atoi(sql_getvalue(sqr, 0, 2));
			sql_freeresult(sqr);
			memset(password, 0, sizeof(password));
			return contactid;
		} else {
			if ((contactid=xmlrpc_auth_checkpass(sid, username, password))<1) {
				send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Authentication failure");
				return -1;
			}
			snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, time(NULL)));
			md5_init(&c);
			md5_update(&c, username, strlen(username));
			md5_update(&c, timebuffer, strlen(timebuffer));
			md5_final(&(md[0]),&c);
			memset(token, 0, sizeof(token));
			for (i=0;i<MD5_SIZE;i++) strncatf(token, sizeof(token)-strlen(token)-1, "%02x", md[i]);
			sql_updatef(sid, "UPDATE gw_contacts SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", raddress, timebuffer, token, username);
			memset(password, 0, sizeof(password));
			return contactid;
		}
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	xmlrpc_fault(sid, -1, "Authentication failure");
	return -1;
}
/*
void xmlrpc_auth_logout(CONN *sid)
{
	time_t t;
	char timebuffer[100];

	sql_updatef(sid, "UPDATE gw_contacts SET logintoken='NULL' WHERE username = '%s'", sid->dat->user_username);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
	snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=NULL; path=/");
	printlogout(sid);
}
*/

void xmlrpc_contactread(CONN *sid, int contactid)
{
	MOD_CONTACTS_READ mod_contacts_read;
	REC_CONTACT contact;

	send_header(sid, 0, 200, "OK", "1", "text/xml", -1, -1);
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if ((mod_contacts_read=module_call(sid, "mod_contacts_read"))==NULL) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (mod_contacts_read(sid, 2, contactid, &contact)!=0) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<params>\r\n<param>\r\n");
	prints(sid, "<value>\r\n<struct>\r\n");
	xmlrpc_addmember(sid, "contactid",      "int",      "%d", contact.contactid);
	xmlrpc_addmember(sid, "obj_ctime",      "dateTime", "%d", contact.obj_ctime);
	xmlrpc_addmember(sid, "obj_mtime",      "dateTime", "%d", contact.obj_mtime);
	xmlrpc_addmember(sid, "obj_uid",        "int",      "%d", contact.obj_uid);
	xmlrpc_addmember(sid, "obj_gid",        "int",      "%d", contact.obj_gid);
	xmlrpc_addmember(sid, "obj_did",        "int",      "%d", contact.obj_did);
	xmlrpc_addmember(sid, "obj_gperm",      "int",      "%d", contact.obj_gperm);
	xmlrpc_addmember(sid, "obj_operm",      "int",      "%d", contact.obj_operm);
	xmlrpc_addmember(sid, "loginip",        "string",   "%s", contact.loginip);
	xmlrpc_addmember(sid, "logintime",      "dateTime", "%d", contact.logintime);
	xmlrpc_addmember(sid, "logintoken",     "string",   "%s", contact.logintoken);
	xmlrpc_addmember(sid, "username",       "string",   "%s", contact.username);
	xmlrpc_addmember(sid, "password",       "string",   "%s", contact.password);
	xmlrpc_addmember(sid, "enabled",        "int",      "%d", contact.enabled);
	xmlrpc_addmember(sid, "geozone",        "int",      "%d", contact.geozone);
	xmlrpc_addmember(sid, "timezone",       "int",      "%d", contact.timezone);
	xmlrpc_addmember(sid, "surname",        "string",   "%s", contact.surname);
	xmlrpc_addmember(sid, "givenname",      "string",   "%s", contact.givenname);
	xmlrpc_addmember(sid, "salutation",     "string",   "%s", contact.salutation);
	xmlrpc_addmember(sid, "contacttype",    "string",   "%s", contact.contacttype);
	xmlrpc_addmember(sid, "referredby",     "string",   "%s", contact.referredby);
	xmlrpc_addmember(sid, "altcontact",     "string",   "%s", contact.altcontact);
	xmlrpc_addmember(sid, "prefbilling",    "string",   "%s", contact.prefbilling);
	xmlrpc_addmember(sid, "email",          "string",   "%s", contact.email);
	xmlrpc_addmember(sid, "homenumber",     "string",   "%s", contact.homenumber);
	xmlrpc_addmember(sid, "worknumber",     "string",   "%s", contact.worknumber);
	xmlrpc_addmember(sid, "faxnumber",      "string",   "%s", contact.faxnumber);
	xmlrpc_addmember(sid, "mobilenumber",   "string",   "%s", contact.mobilenumber);
	xmlrpc_addmember(sid, "jobtitle",       "string",   "%s", contact.jobtitle);
	xmlrpc_addmember(sid, "organization",   "string",   "%s", contact.organization);
	xmlrpc_addmember(sid, "homeaddress",    "string",   "%s", contact.homeaddress);
	xmlrpc_addmember(sid, "homelocality",   "string",   "%s", contact.homelocality);
	xmlrpc_addmember(sid, "homeregion",     "string",   "%s", contact.homeregion);
	xmlrpc_addmember(sid, "homecountry",    "string",   "%s", contact.homecountry);
	xmlrpc_addmember(sid, "homepostalcode", "string",   "%s", contact.homepostalcode);
	xmlrpc_addmember(sid, "workaddress",    "string",   "%s", contact.workaddress);
	xmlrpc_addmember(sid, "worklocality",   "string",   "%s", contact.worklocality);
	xmlrpc_addmember(sid, "workregion",     "string",   "%s", contact.workregion);
	xmlrpc_addmember(sid, "workcountry",    "string",   "%s", contact.workcountry);
	xmlrpc_addmember(sid, "workpostalcode", "string",   "%s", contact.workpostalcode);
	prints(sid, "</struct>\r\n</value>\r\n");
	prints(sid, "</param>\r\n</params>\r\n</methodResponse>\r\n");
	return;
}

void xmlrpc_contactwrite(CONN *sid)
{
	MOD_CONTACTS_READ mod_contacts_read;
	MOD_CONTACTS_WRITE mod_contacts_write;
	REC_CONTACT contact;
	char opassword[50];
	char *ptemp;
	int contactid;

	if (!(auth_priv(sid, "contacts")&A_MODIFY)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) {
		xmlrpc_fault(sid, -1, "Incorrect Method");
		return;
	}
	if ((mod_contacts_read=module_call(sid, "mod_contacts_read"))==NULL) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if ((mod_contacts_write=module_call(sid, "mod_contacts_write"))==NULL) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if ((ptemp=getxmlstruct(sid, "CONTACTID", "INT"))==NULL) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	contactid=atoi(ptemp);
	if (mod_contacts_read(sid, 2, contactid, &contact)!=0) {
		xmlrpc_fault(sid, -1, "No matching record found");
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", contact.password);
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		if ((ptemp=getxmlstruct(sid, "OBJ_UID", "INT"))!=NULL) contact.obj_uid=atoi(ptemp);
		if ((ptemp=getxmlstruct(sid, "OBJ_GID", "INT"))!=NULL) contact.obj_gid=atoi(ptemp);
		if ((ptemp=getxmlstruct(sid, "OBJ_DID", "INT"))!=NULL) contact.obj_did=atoi(ptemp);
	}
	if ((auth_priv(sid, "contacts")&A_ADMIN)||(contact.obj_uid==sid->dat->user_uid)) {
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
		if (!(auth_priv(sid, "contacts")&A_INSERT)) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		if ((contact.contactid=mod_contacts_write(sid, 0, &contact))<1) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		db_log_activity(sid, 1, "contacts", contact.contactid, "insert", "%s - %s added contact %d (xml-rpc)", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
	} else {
		if (!(auth_priv(sid, "contacts")&A_MODIFY)) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		if (strcmp(opassword, contact.password)!=0) {
			snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		}
		if (mod_contacts_write(sid, contactid, &contact)<1) {
			xmlrpc_fault(sid, -1, ERR_NOACCESS);
			return;
		}
		db_log_activity(sid, 1, "contacts", contact.contactid, "modify", "%s - %s modified contact %d (xml-rpc)", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
	}
	xmlrpc_contactread(sid, contact.contactid);
	return;
}

void mod_main(CONN *sid)
{
	char methodname[40];
	char *ptemp;
	int index;

	DEBUG_IN(sid, "xmlrpc_main()");
	if (auth_setcookie(sid)!=0) {
		send_header(sid, 0, 200, "OK", "1", "text/xml", -1, -1);
		xmlrpc_fault(sid, -1, "Authentication failure");
		return;
	}
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
			prints(sid, "</BODY>\r\n</HTML>\r\n");
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

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_xmlrpc", "", "", "mod_main", "/xml-rpc/", mod_main)!=0) return -1;
//	if (mod_export_function("mod_xmlrpc", "mod_main", mod_main)!=0) return -1;
	return 0;
}
