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
#include "mod_substub.h"
#include "mod_xmlrpc.h"

void xmlrpc_contact_newstruct(CONN *sid)
{
	prints(sid, "<value>\r\n<struct>\r\n");
	xmlrpc_addmember(sid, "contactid",      "int",      "0");
	xmlrpc_addmember(sid, "obj_ctime",      "dateTime", "%d", time(NULL));
	xmlrpc_addmember(sid, "obj_mtime",      "dateTime", "%d", time(NULL));
	xmlrpc_addmember(sid, "obj_uid",        "int",      "%d", sid->dat->user_uid);
	xmlrpc_addmember(sid, "obj_gid",        "int",      "%d", sid->dat->user_gid);
	xmlrpc_addmember(sid, "obj_did",        "int",      "0");
	xmlrpc_addmember(sid, "obj_gperm",      "int",      "1");
	xmlrpc_addmember(sid, "obj_operm",      "int",      "1");
	xmlrpc_addmember(sid, "loginip",        "string",   "0.0.0.0");
	xmlrpc_addmember(sid, "logintime",      "dateTime", "0");
	xmlrpc_addmember(sid, "logintoken",     "string",   "");
	xmlrpc_addmember(sid, "username",       "string",   "");
	xmlrpc_addmember(sid, "password",       "string",   "");
	xmlrpc_addmember(sid, "enabled",        "int",      "0");
	xmlrpc_addmember(sid, "geozone",        "int",      "1");
	xmlrpc_addmember(sid, "timezone",       "int",      "%d", sid->dat->user_timezone);
	xmlrpc_addmember(sid, "surname",        "string",   "");
	xmlrpc_addmember(sid, "givenname",      "string",   "");
	xmlrpc_addmember(sid, "salutation",     "string",   "");
	xmlrpc_addmember(sid, "contacttype",    "string",   "");
	xmlrpc_addmember(sid, "referredby",     "string",   "");
	xmlrpc_addmember(sid, "altcontact",     "string",   "");
	xmlrpc_addmember(sid, "prefbilling",    "string",   "");
	xmlrpc_addmember(sid, "email",          "string",   "");
	xmlrpc_addmember(sid, "homenumber",     "string",   "");
	xmlrpc_addmember(sid, "worknumber",     "string",   "");
	xmlrpc_addmember(sid, "faxnumber",      "string",   "");
	xmlrpc_addmember(sid, "mobilenumber",   "string",   "");
	xmlrpc_addmember(sid, "jobtitle",       "string",   "");
	xmlrpc_addmember(sid, "organization",   "string",   "");
	xmlrpc_addmember(sid, "homeaddress",    "string",   "");
	xmlrpc_addmember(sid, "homelocality",   "string",   "");
	xmlrpc_addmember(sid, "homeregion",     "string",   "");
	xmlrpc_addmember(sid, "homecountry",    "string",   "");
	xmlrpc_addmember(sid, "homepostalcode", "string",   "");
	xmlrpc_addmember(sid, "workaddress",    "string",   "");
	xmlrpc_addmember(sid, "worklocality",   "string",   "");
	xmlrpc_addmember(sid, "workregion",     "string",   "");
	xmlrpc_addmember(sid, "workcountry",    "string",   "");
	xmlrpc_addmember(sid, "workpostalcode", "string",   "");
	prints(sid, "</struct>\r\n</value>\r\n");
	return;
}

void xmlrpc_contact_addstruct(CONN *sid, int sqr, int tuple)
{
	if ((tuple<0)||(tuple>=sql_numtuples(sqr))) return;
	prints(sid, "<value>\r\n<struct>\r\n");
	xmlrpc_addmember(sid, "contactid",      "int",      "%d", atoi(sql_getvalue(sqr, tuple, 0)));
	xmlrpc_addmember(sid, "obj_ctime",      "dateTime", "%d", time_sql2unix(sql_getvalue(sqr, tuple, 1)));
	xmlrpc_addmember(sid, "obj_mtime",      "dateTime", "%d", time_sql2unix(sql_getvalue(sqr, tuple, 2)));
	xmlrpc_addmember(sid, "obj_uid",        "int",      "%d", atoi(sql_getvalue(sqr, tuple, 3)));
	xmlrpc_addmember(sid, "obj_gid",        "int",      "%d", atoi(sql_getvalue(sqr, tuple, 4)));
	xmlrpc_addmember(sid, "obj_did",        "int",      "%d", atoi(sql_getvalue(sqr, tuple, 5)));
	xmlrpc_addmember(sid, "obj_gperm",      "int",      "%d", atoi(sql_getvalue(sqr, tuple, 6)));
	xmlrpc_addmember(sid, "obj_operm",      "int",      "%d", atoi(sql_getvalue(sqr, tuple, 7)));
	xmlrpc_addmember(sid, "loginip",        "string",   "%s", sql_getvalue(sqr, tuple, 8));
	xmlrpc_addmember(sid, "logintime",      "dateTime", "%d", time_sql2unix(sql_getvalue(sqr, tuple, 9)));
	xmlrpc_addmember(sid, "logintoken",     "string",   "%s", sql_getvalue(sqr, tuple, 10));
	xmlrpc_addmember(sid, "username",       "string",   "%s", sql_getvalue(sqr, tuple, 11));
	xmlrpc_addmember(sid, "password",       "string",   "%s", sql_getvalue(sqr, tuple, 12));
	xmlrpc_addmember(sid, "enabled",        "int",      "%d", atoi(sql_getvalue(sqr, tuple, 13)));
	xmlrpc_addmember(sid, "geozone",        "int",      "%d", atoi(sql_getvalue(sqr, tuple, 14)));
	xmlrpc_addmember(sid, "timezone",       "int",      "%d", atoi(sql_getvalue(sqr, tuple, 15)));
	xmlrpc_addmember(sid, "surname",        "string",   "%s", sql_getvalue(sqr, tuple, 16));
	xmlrpc_addmember(sid, "givenname",      "string",   "%s", sql_getvalue(sqr, tuple, 17));
	xmlrpc_addmember(sid, "salutation",     "string",   "%s", sql_getvalue(sqr, tuple, 18));
	xmlrpc_addmember(sid, "contacttype",    "string",   "%s", sql_getvalue(sqr, tuple, 19));
	xmlrpc_addmember(sid, "referredby",     "string",   "%s", sql_getvalue(sqr, tuple, 20));
	xmlrpc_addmember(sid, "altcontact",     "string",   "%s", sql_getvalue(sqr, tuple, 21));
	xmlrpc_addmember(sid, "prefbilling",    "string",   "%s", sql_getvalue(sqr, tuple, 22));
	xmlrpc_addmember(sid, "email",          "string",   "%s", sql_getvalue(sqr, tuple, 23));
	xmlrpc_addmember(sid, "homenumber",     "string",   "%s", sql_getvalue(sqr, tuple, 24));
	xmlrpc_addmember(sid, "worknumber",     "string",   "%s", sql_getvalue(sqr, tuple, 25));
	xmlrpc_addmember(sid, "faxnumber",      "string",   "%s", sql_getvalue(sqr, tuple, 26));
	xmlrpc_addmember(sid, "mobilenumber",   "string",   "%s", sql_getvalue(sqr, tuple, 27));
	xmlrpc_addmember(sid, "jobtitle",       "string",   "%s", sql_getvalue(sqr, tuple, 28));
	xmlrpc_addmember(sid, "organization",   "string",   "%s", sql_getvalue(sqr, tuple, 29));
	xmlrpc_addmember(sid, "homeaddress",    "string",   "%s", sql_getvalue(sqr, tuple, 30));
	xmlrpc_addmember(sid, "homelocality",   "string",   "%s", sql_getvalue(sqr, tuple, 31));
	xmlrpc_addmember(sid, "homeregion",     "string",   "%s", sql_getvalue(sqr, tuple, 32));
	xmlrpc_addmember(sid, "homecountry",    "string",   "%s", sql_getvalue(sqr, tuple, 33));
	xmlrpc_addmember(sid, "homepostalcode", "string",   "%s", sql_getvalue(sqr, tuple, 34));
	xmlrpc_addmember(sid, "workaddress",    "string",   "%s", sql_getvalue(sqr, tuple, 35));
	xmlrpc_addmember(sid, "worklocality",   "string",   "%s", sql_getvalue(sqr, tuple, 36));
	xmlrpc_addmember(sid, "workregion",     "string",   "%s", sql_getvalue(sqr, tuple, 37));
	xmlrpc_addmember(sid, "workcountry",    "string",   "%s", sql_getvalue(sqr, tuple, 38));
	xmlrpc_addmember(sid, "workpostalcode", "string",   "%s", sql_getvalue(sqr, tuple, 39));
	prints(sid, "</struct>\r\n</value>\r\n");
	return;
}

void xmlrpc_contactread(CONN *sid, int contactid)
{
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/xml", -1, -1);
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (contactid==0) {
		prints(sid, "<?xml version=\"1.0\"?>\r\n");
		prints(sid, "<methodResponse>\r\n<params>\r\n<param>\r\n");
		xmlrpc_contact_newstruct(sid);
		prints(sid, "</param>\r\n</params>\r\n</methodResponse>\r\n");
		return;
	} else if (auth_priv(sid, "contacts")&A_ADMIN) {
		sqr=sql_queryf(sid, "SELECT * from gw_contacts where contactid = %d ORDER BY contactid ASC", contactid);
	} else {
		sqr=sql_queryf(sid, "SELECT * from gw_contacts WHERE contactid = %d  AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY contactid ASC", contactid, sid->dat->user_uid, sid->dat->user_gid);
	}
	if (sqr<0) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (sql_numtuples(sqr)!=1) {
		xmlrpc_fault(sid, -1, "No matching record found");
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<params>\r\n<param>\r\n");
	xmlrpc_contact_addstruct(sid, sqr, 0);
	prints(sid, "</param>\r\n</params>\r\n</methodResponse>\r\n");
	sql_freeresult(sqr);
	return;
}

void xmlrpc_contactlist(CONN *sid)
{
	int i;
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/xml", -1, -1);
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		sqr=sql_queryf(sid, "SELECT * from gw_contacts ORDER BY contactid ASC");
	} else {
		sqr=sql_queryf(sid, "SELECT * from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY contactid ASC", sid->dat->user_uid, sid->dat->user_gid);
	}
	if (sqr<0) {
		xmlrpc_fault(sid, -1, ERR_NOACCESS);
		return;
	}
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<params>\r\n<param>\r\n");
	prints(sid, "<value>\r\n<array>\r\n<data>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		xmlrpc_contact_addstruct(sid, sqr, i);
	}
	prints(sid, "</data>\r\n</array>\r\n</value>\r\n");
	prints(sid, "</param>\r\n</params>\r\n</methodResponse>\r\n");
	sql_freeresult(sqr);
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
	if (contact.obj_uid<1) contact.obj_uid=sid->dat->user_uid;
	if (contact.obj_gid<1) contact.obj_gid=sid->dat->user_gid;
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
