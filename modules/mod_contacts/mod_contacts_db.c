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
#include "mod_contacts.h"

int dblist_contacts(CONN *sid, char *searchfield, char *searchstring)
{
	char query[2048];
	int i;
	int sqr1;
	int sqr2;

	memset(query, 0, sizeof(query));
	if ((sqr1=sql_queryf("SELECT groupid FROM gw_groupmembers where userid = %d AND obj_did = %d", sid->dat->user_uid, sid->dat->user_did))<0) return EC_UNKNOWN;
	snprintf(query, sizeof(query)-1, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE ");
	if (strcasecmp(searchstring, "All")!=0) {
		strncatf(query, sizeof(query)-strlen(query)-1, "(lower(%s) like lower('%s%%')) AND ", searchfield, searchstring);
	}
	if (!(auth_priv(sid, "contacts")&A_ADMIN)) {
		strncatf(query, sizeof(query)-strlen(query)-1, "(obj_uid = %d or (obj_gperm>=1 AND (obj_gid = %d", sid->dat->user_uid, sid->dat->user_gid);
		for (i=0;i<sql_numtuples(sqr1);i++) {
			strncatf(query, sizeof(query)-strlen(query)-1, " or obj_gid = %d", atoi(sql_getvalue(sqr1, i, 0)));
		}
		strncatf(query, sizeof(query)-strlen(query)-1, ")) or obj_operm>=1) AND ");
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "obj_did = %d ORDER BY surname, givenname ASC", sid->dat->user_did);
	if ((sqr2=sql_query(query))<0) {
		sql_freeresult(sqr1);
		return EC_UNKNOWN;
	}
	sql_freeresult(sqr1);
	return sqr2;
}

int dbread_contact(CONN *sid, short int perm, int index, REC_CONTACT *contact)
{
	short int approved;
	int authlevel;
	int sqr;

	memset(contact, 0, sizeof(REC_CONTACT));
	authlevel=auth_priv(sid, "contacts");
	if (authlevel<1) return EC_NOPERM;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return EC_NOPERM;
	if (!(authlevel&A_INSERT)&&(index==0)) return EC_NOPERM;
	if (index==0) {
		contact->obj_uid=sid->dat->user_uid;
		contact->obj_gid=sid->dat->user_gid;
		contact->obj_did=sid->dat->user_did;
		contact->obj_gperm=1;
		contact->obj_operm=1;
		contact->enabled=1;
		contact->timezone=sid->dat->user_timezone;
		return EC_NOERROR;
	}
	if ((sqr=sql_queryf("SELECT * FROM gw_contacts where contactid = %d AND obj_did = %d", index, sid->dat->user_did))<0) return EC_UNKNOWN;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return EC_NORECORD;
	}
	contact->contactid = atoi(sql_getvalue(sqr, 0, 0));
	contact->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	contact->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	contact->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	contact->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	contact->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	contact->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	contact->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	contact->folderid=atoi(sql_getvalue(sqr, 0, 8));
	strncpy(contact->loginip,		sql_getvalue(sqr, 0, 9), sizeof(contact->loginip)-1);
	contact->logintime=time_sql2unix(sql_getvalue(sqr, 0, 10));
	strncpy(contact->logintoken,		sql_getvalue(sqr, 0, 11), sizeof(contact->logintoken)-1);
	strncpy(contact->username,		sql_getvalue(sqr, 0, 12), sizeof(contact->username)-1);
	strncpy(contact->password,		sql_getvalue(sqr, 0, 13), sizeof(contact->password)-1);
	contact->enabled=atoi(sql_getvalue(sqr, 0, 14));
	contact->geozone=atoi(sql_getvalue(sqr, 0, 15));
	contact->timezone=atoi(sql_getvalue(sqr, 0, 16));
	strncpy(contact->surname,		sql_getvalue(sqr, 0, 17), sizeof(contact->surname)-1);
	strncpy(contact->givenname,		sql_getvalue(sqr, 0, 18), sizeof(contact->givenname)-1);
	strncpy(contact->salutation,		sql_getvalue(sqr, 0, 19), sizeof(contact->salutation)-1);
	strncpy(contact->contacttype,		sql_getvalue(sqr, 0, 20), sizeof(contact->contacttype)-1);
	strncpy(contact->referredby,		sql_getvalue(sqr, 0, 21), sizeof(contact->referredby)-1);
	strncpy(contact->altcontact,		sql_getvalue(sqr, 0, 22), sizeof(contact->altcontact)-1);
	strncpy(contact->prefbilling,		sql_getvalue(sqr, 0, 23), sizeof(contact->prefbilling)-1);
	strncpy(contact->website,		sql_getvalue(sqr, 0, 24), sizeof(contact->website)-1);
	strncpy(contact->email,			sql_getvalue(sqr, 0, 25), sizeof(contact->email)-1);
	strncpy(contact->homenumber,		sql_getvalue(sqr, 0, 26), sizeof(contact->homenumber)-1);
	strncpy(contact->worknumber,		sql_getvalue(sqr, 0, 27), sizeof(contact->worknumber)-1);
	strncpy(contact->faxnumber,		sql_getvalue(sqr, 0, 28), sizeof(contact->faxnumber)-1);
	strncpy(contact->mobilenumber,		sql_getvalue(sqr, 0, 29), sizeof(contact->mobilenumber)-1);
	strncpy(contact->jobtitle,		sql_getvalue(sqr, 0, 30), sizeof(contact->jobtitle)-1);
	strncpy(contact->organization,		sql_getvalue(sqr, 0, 31), sizeof(contact->organization)-1);
	strncpy(contact->homeaddress,		sql_getvalue(sqr, 0, 32), sizeof(contact->homeaddress)-1);
	strncpy(contact->homelocality,		sql_getvalue(sqr, 0, 33), sizeof(contact->homelocality)-1);
	strncpy(contact->homeregion,		sql_getvalue(sqr, 0, 34), sizeof(contact->homeregion)-1);
	strncpy(contact->homecountry,		sql_getvalue(sqr, 0, 35), sizeof(contact->homecountry)-1);
	strncpy(contact->homepostalcode,	sql_getvalue(sqr, 0, 36), sizeof(contact->homepostalcode)-1);
	strncpy(contact->workaddress,		sql_getvalue(sqr, 0, 37), sizeof(contact->workaddress)-1);
	strncpy(contact->worklocality,		sql_getvalue(sqr, 0, 38), sizeof(contact->worklocality)-1);
	strncpy(contact->workregion,		sql_getvalue(sqr, 0, 39), sizeof(contact->workregion)-1);
	strncpy(contact->workcountry,		sql_getvalue(sqr, 0, 40), sizeof(contact->workcountry)-1);
	strncpy(contact->workpostalcode,	sql_getvalue(sqr, 0, 41), sizeof(contact->workpostalcode)-1);
	sql_freeresult(sqr);
	approved=0;
	if (auth_priv(sid, "contacts")&A_ADMIN) approved=2;
	if (approved==0) {
		if (contact->obj_uid==sid->dat->user_uid) approved=2;
	}
	if (approved==0) {
		if (contact->obj_gperm>=perm) {
			if (contact->obj_gid==sid->dat->user_gid) {
				approved=contact->obj_gperm;
			} else {
				if ((sqr=sql_queryf("SELECT * FROM gw_groupmembers WHERE userid = %d AND groupid = %d AND obj_did = %d", sid->dat->user_uid, contact->obj_gid, sid->dat->user_did))<0) return EC_UNKNOWN;
				if (sql_numtuples(sqr)>0) approved=contact->obj_gperm;
				sql_freeresult(sqr);
			}
		}
	}
	if (approved==0) {
		if (contact->obj_operm>=perm) {
			approved=contact->obj_operm;
		}
	}
	if (approved==0) {
		memset(contact, 0, sizeof(REC_CONTACT));
		return EC_NOPERM;
	}
	return approved;
}

int dbwrite_contact(CONN *sid, int index, REC_CONTACT *contact)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	int sqr;

	authlevel=auth_priv(sid, "contacts");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if ((sqr=sql_query("SELECT max(contactid) FROM gw_contacts"))<0) return -1;
		contact->contactid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (contact->contactid<1) contact->contactid=1;
		strcpy(query, "INSERT INTO gw_contacts (contactid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, folderid, loginip, logintime, logintoken, username, password, enabled, geozone, timezone, surname, givenname, salutation, contacttype, referredby, altcontact, prefbilling, email, homenumber, worknumber, faxnumber, mobilenumber, jobtitle, organization, homeaddress, homelocality, homeregion, homecountry, homepostalcode, workaddress, worklocality, workregion, workcountry, workpostalcode) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", contact->contactid, curdate, curdate, contact->obj_uid, contact->obj_gid, contact->obj_did, contact->obj_gperm, contact->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", contact->folderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->username));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->password));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", contact->enabled);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", contact->geozone);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", contact->timezone);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->salutation));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->contacttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->referredby));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->altcontact));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->prefbilling));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->website));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->email));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->mobilenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->organization));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homeaddress));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homelocality));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homeregion));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homecountry));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homepostalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workaddress));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->worklocality));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workregion));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workcountry));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workpostalcode));
		if (sql_update(query)<0) return -1;
		return contact->contactid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_contacts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, contact->obj_uid, contact->obj_gid, contact->obj_gperm, contact->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "folderid = '%d', ",       contact->folderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "username = '%s', ",       str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->username));
		strncatf(query, sizeof(query)-strlen(query)-1, "password = '%s', ",       str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->password));
		strncatf(query, sizeof(query)-strlen(query)-1, "enabled = '%d', ",        contact->enabled);
		strncatf(query, sizeof(query)-strlen(query)-1, "geozone = '%d', ",        contact->geozone);
		strncatf(query, sizeof(query)-strlen(query)-1, "timezone = '%d', ",       contact->timezone);
		strncatf(query, sizeof(query)-strlen(query)-1, "surname = '%s', ",        str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "givenname = '%s', ",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "salutation = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->salutation));
		strncatf(query, sizeof(query)-strlen(query)-1, "contacttype = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->contacttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "referredby = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->referredby));
		strncatf(query, sizeof(query)-strlen(query)-1, "altcontact = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->altcontact));
		strncatf(query, sizeof(query)-strlen(query)-1, "prefbilling = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->prefbilling));
		strncatf(query, sizeof(query)-strlen(query)-1, "website = '%s', ",        str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->website));
		strncatf(query, sizeof(query)-strlen(query)-1, "email = '%s', ",          str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->email));
		strncatf(query, sizeof(query)-strlen(query)-1, "homenumber = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "worknumber = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "faxnumber = '%s', ",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "mobilenumber = '%s', ",   str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->mobilenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "jobtitle = '%s', ",       str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "organization = '%s', ",   str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->organization));
		strncatf(query, sizeof(query)-strlen(query)-1, "homeaddress = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homeaddress));
		strncatf(query, sizeof(query)-strlen(query)-1, "homelocality = '%s', ",   str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homelocality));
		strncatf(query, sizeof(query)-strlen(query)-1, "homeregion = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homeregion));
		strncatf(query, sizeof(query)-strlen(query)-1, "homecountry = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homecountry));
		strncatf(query, sizeof(query)-strlen(query)-1, "homepostalcode = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->homepostalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "workaddress = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workaddress));
		strncatf(query, sizeof(query)-strlen(query)-1, "worklocality = '%s', ",   str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->worklocality));
		strncatf(query, sizeof(query)-strlen(query)-1, "workregion = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workregion));
		strncatf(query, sizeof(query)-strlen(query)-1, "workcountry = '%s', ",    str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workcountry));
		strncatf(query, sizeof(query)-strlen(query)-1, "workpostalcode = '%s'",   str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, contact->workpostalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE contactid = %d", contact->contactid);
		if (sql_update(query)<0) return -1;
		return contact->contactid;
	}
	return -1;
}
