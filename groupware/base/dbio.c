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

int db_read(CONNECTION *sid, short int perm, short int table, int index, void *record)
{
	union rec_u rec;
	int authlevel;
	int sqr;

	rec.head=record;
	rec.bookmark=record;
	rec.bookmarkfolder=record;
	rec.call=record;
	rec.contact=record;
	rec.event=record;
	rec.file=record;
	rec.forumgroup=record;
	rec.group=record;
	rec.mailacct=record;
	rec.note=record;
	rec.order=record;
	rec.orderitem=record;
	rec.product=record;
	rec.query=record;
	rec.task=record;
	rec.user=record;
	rec.zone=record;
	switch (table) {
	case DB_BOOKMARKS:
		memset(rec.bookmark, 0, sizeof(REC_BOOKMARK));
		authlevel=auth_priv(sid, AUTH_BOOKMARKS);
		break;
	case DB_BOOKMARKFOLDERS:
		memset(rec.bookmarkfolder, 0, sizeof(REC_BOOKMARKFOLDER));
		authlevel=auth_priv(sid, AUTH_BOOKMARKS);
		break;
	case DB_CALLS:
		memset(rec.call, 0, sizeof(REC_CALL));
		authlevel=auth_priv(sid, AUTH_CALLS);
		break;
	case DB_CONTACTS:
		memset(rec.contact, 0, sizeof(REC_CONTACT));
		authlevel=auth_priv(sid, AUTH_CONTACTS);
		break;
	case DB_EVENTS:
		memset(rec.event, 0, sizeof(REC_EVENT));
		authlevel=auth_priv(sid, AUTH_CALENDAR);
		break;
	case DB_FILES:
		memset(rec.file, 0, sizeof(REC_FILE));
		authlevel=auth_priv(sid, AUTH_FILES);
		break;
	case DB_FORUMGROUPS:
		memset(rec.forumgroup, 0, sizeof(REC_FORUMGROUP));
		authlevel=auth_priv(sid, AUTH_FORUMS);
		break;
	case DB_GROUPS:
		memset(rec.group, 0, sizeof(REC_GROUP));
		authlevel=auth_priv(sid, AUTH_ADMIN);
		break;
	case DB_MAILACCOUNTS:
		memset(rec.mailacct, 0, sizeof(REC_MAILACCT));
		authlevel=auth_priv(sid, AUTH_WEBMAIL);
		break;
	case DB_NOTES:
		memset(rec.note, 0, sizeof(REC_NOTE));
		authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		if (auth_priv(sid, AUTH_ADMIN)&A_ADMIN) {
			authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		}
		break;
	case DB_ORDERS:
		memset(rec.order, 0, sizeof(REC_ORDER));
		authlevel=auth_priv(sid, AUTH_ORDERS);
		break;
	case DB_ORDERITEMS:
		memset(rec.orderitem, 0, sizeof(REC_ORDERITEM));
		authlevel=auth_priv(sid, AUTH_ORDERS);
		break;
	case DB_PRODUCTS:
		memset(rec.product, 0, sizeof(REC_PRODUCT));
		authlevel=auth_priv(sid, AUTH_ORDERS);
		break;
	case DB_PROFILE:
		memset(rec.user, 0, sizeof(REC_USER));
		authlevel=auth_priv(sid, AUTH_PROFILE);
		if (authlevel&A_MODIFY) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		index=sid->dat->user_uid;
		break;
	case DB_QUERIES:
		memset(rec.query, 0, sizeof(REC_QUERY));
		authlevel=auth_priv(sid, AUTH_QUERIES);
		break;
	case DB_TASKS:
		memset(rec.task, 0, sizeof(REC_TASK));
		authlevel=auth_priv(sid, AUTH_CALENDAR);
		break;
	case DB_USERS:
		memset(rec.user, 0, sizeof(REC_USER));
		authlevel=auth_priv(sid, AUTH_ADMIN);
		break;
	case DB_ZONES:
		memset(rec.zone, 0, sizeof(REC_ZONE));
		authlevel=auth_priv(sid, AUTH_ADMIN);
		break;
	default:
		return -1;
		break;
	}
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if ((perm>1)&&(table==DB_PRODUCTS)&&(!(authlevel&A_ADMIN))) return -1;
	if (index==0) {
		rec.head->obj_uid=sid->dat->user_uid;
		rec.head->obj_gid=sid->dat->user_gid;
		rec.head->obj_gperm=1;
		rec.head->obj_operm=1;
		switch (table) {
		case DB_BOOKMARKS:
			snprintf(rec.bookmark->bookmarkurl, sizeof(rec.bookmark->bookmarkurl)-1, "http://");
			break;
		case DB_BOOKMARKFOLDERS:
			snprintf(rec.bookmarkfolder->foldername, sizeof(rec.bookmarkfolder->foldername)-1, "New Folder");
			break;
		case DB_CALLS:
			rec.call->assignedby=sid->dat->user_uid;
			rec.call->assignedto=sid->dat->user_uid;
			snprintf(rec.call->callname, sizeof(rec.call->callname)-1, "New Call");
			rec.call->calldate=(time(NULL)/900)*900;
			break;
		case DB_CONTACTS:
			rec.contact->enabled=1;
			rec.contact->timezone=sid->dat->user_timezone;
			break;
		case DB_EVENTS:
			snprintf(rec.event->eventname, sizeof(rec.event->eventname)-1, "New Event");
			rec.event->assignedby=sid->dat->user_uid;
			rec.event->assignedto=sid->dat->user_uid;
			rec.event->priority=2;
			rec.event->eventstart=(time(NULL)/900)*900;
			rec.event->eventfinish=(time(NULL)/900)*900+900;
			rec.event->busy=1;
			break;
		case DB_FILES:
			break;
		case DB_FORUMGROUPS:
			snprintf(rec.forumgroup->title, sizeof(rec.forumgroup->title)-1, "New Forum Group");
			break;
		case DB_GROUPS:
			strncpy(rec.group->availability, "000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000", sizeof(rec.user->availability)-1);
			break;
		case DB_MAILACCOUNTS:
			snprintf(rec.mailacct->hosttype, sizeof(rec.mailacct->hosttype)-1, "POP3");
			rec.mailacct->popport=110;
			rec.mailacct->smtpport=25;
			break;
		case DB_NOTES:
			snprintf(rec.note->notetitle, sizeof(rec.note->notetitle)-1, "New Note");
			break;
		case DB_ORDERS:
			rec.order->orderdate=(time(NULL)/60)*60;
			rec.order->userid=sid->dat->user_uid;
			snprintf(rec.order->ordertype, sizeof(rec.order->ordertype)-1, "Sale");
			snprintf(rec.order->paymentmethod, sizeof(rec.order->paymentmethod)-1, "Cash");
			break;
		case DB_ORDERITEMS:
			rec.orderitem->quantity=1;
			break;
		case DB_PRODUCTS:
			rec.product->tax1=config.info.tax1percent;
			rec.product->tax2=config.info.tax2percent;
			break;
		case DB_QUERIES:
			break;
		case DB_TASKS:
			rec.task->assignedby=sid->dat->user_uid;
			rec.task->assignedto=sid->dat->user_uid;
			snprintf(rec.task->taskname, sizeof(rec.task->taskname)-1, "New Task");
			rec.task->priority=2;
			rec.task->duedate=(time(NULL)/86400)*86400;
			break;
		case DB_USERS:
			rec.user->groupid=2;
			rec.user->enabled=1;
			rec.user->authbookmarks=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authcalendar=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authcalls=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authcontacts=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authwebmail=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authfiles=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authforums=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authmessages=A_READ+A_INSERT+A_DELETE;
			rec.user->authorders=A_READ+A_MODIFY+A_INSERT+A_DELETE;
			rec.user->authprofile=A_READ+A_MODIFY;
			rec.user->authquery=A_READ;
			rec.user->prefdaystart=0;
			rec.user->prefdaylength=24;
			rec.user->prefmailcurrent=0;
			rec.user->prefmaildefault=0;
			rec.user->prefmaxlist=15;
			rec.user->prefmenustyle=1;
			rec.user->preftimezone=10;
			strncpy(rec.user->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(rec.user->availability)-1);
			break;
		case DB_ZONES:
			snprintf(rec.zone->zonename, sizeof(rec.zone->zonename)-1, "New Zone");
			break;
		default:
			return -1;
			break;
		}
		return 0;
	}
	switch (table) {
	case DB_BOOKMARKS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_bookmarks where bookmarkid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_bookmarks where bookmarkid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_BOOKMARKFOLDERS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_bookmarkfolders where folderid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_bookmarkfolders where folderid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_CALLS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_calls where callid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_calls where callid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_CONTACTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_contacts where contactid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_contacts where contactid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_EVENTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_events where eventid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_events where eventid = %d and (obj_uid = %d or assignedby = '%s' or assignedto = '%s' or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_username, sid->dat->user_username, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_FILES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_files where fileid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_files where fileid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_FORUMGROUPS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_forumgroups where forumgroupid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_forumgroups where forumgroupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_GROUPS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_groups where groupid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_groups where groupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_MAILACCOUNTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_mailaccounts where mailaccountid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", index, sid->dat->user_uid))<0) return -1;
		}
		break;
	case DB_NOTES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_notes where noteid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_notes where noteid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_ORDERS:
		if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_orders where orderid = %d", index))<0) return -1;
		break;
	case DB_ORDERITEMS:
		if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_orderitems where orderitemid = %d", index))<0) return -1;
		break;
	case DB_PRODUCTS:
		if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_products where productid = %d", index))<0) return -1;
		break;
	case DB_QUERIES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_queries where queryid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_queries where queryid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_TASKS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_tasks where taskid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_tasks where taskid = %d and (obj_uid = %d or assignedby = '%s' or assignedto = '%s' or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_username, sid->dat->user_username, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_PROFILE:
	case DB_USERS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_users where userid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_users where userid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_ZONES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_zones where zoneid = %d", index))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT * FROM gw_zones where zoneid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -2;
	}
	rec.head->recordid  = atoi(sqlGetvalue(sqr, 0, 0));
	rec.head->obj_ctime = time_sql2unix(sqlGetvalue(sqr, 0, 1));
	rec.head->obj_mtime = time_sql2unix(sqlGetvalue(sqr, 0, 2));
	rec.head->obj_uid   = atoi(sqlGetvalue(sqr, 0, 3));
	rec.head->obj_gid   = atoi(sqlGetvalue(sqr, 0, 4));
	rec.head->obj_gperm = atoi(sqlGetvalue(sqr, 0, 5));
	rec.head->obj_operm = atoi(sqlGetvalue(sqr, 0, 6));
	switch (table) {
	case DB_BOOKMARKS:
		rec.bookmark->folderid=atoi(sqlGetvalue(sqr, 0, 7));
		strncpy(rec.bookmark->bookmarkname,	sqlGetvalue(sqr, 0, 8), sizeof(rec.bookmark->bookmarkname)-1);
		strncpy(rec.bookmark->bookmarkurl,	sqlGetvalue(sqr, 0, 9), sizeof(rec.bookmark->bookmarkurl)-1);
		if (strlen(rec.bookmark->bookmarkname)==0) strncpy(rec.bookmark->bookmarkname, "unnamed", sizeof(rec.bookmark->bookmarkname)-1);
		break;
	case DB_BOOKMARKFOLDERS:
		rec.bookmarkfolder->parentid=atoi(sqlGetvalue(sqr, 0, 7));
		strncpy(rec.bookmarkfolder->foldername,	sqlGetvalue(sqr, 0, 8), sizeof(rec.bookmarkfolder->foldername)-1);
		if (strlen(rec.bookmarkfolder->foldername)==0) strncpy(rec.bookmarkfolder->foldername, "unnamed", sizeof(rec.bookmarkfolder->foldername)-1);
		break;
	case DB_CALLS:
		rec.call->assignedby=atoi(sqlGetvalue(sqr, 0, 7));
		rec.call->assignedto=atoi(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.call->callname,		sqlGetvalue(sqr, 0, 9), sizeof(rec.call->callname)-1);
		rec.call->calldate=time_sql2unix(sqlGetvalue(sqr, 0, 10));
		rec.call->contactid=atoi(sqlGetvalue(sqr, 0, 11));
		rec.call->action=atoi(sqlGetvalue(sqr, 0, 12));
		rec.call->status=atoi(sqlGetvalue(sqr, 0, 13));
		strncpy(rec.call->details,		sqlGetvalue(sqr, 0, 14), sizeof(rec.call->details)-1);
		break;
	case DB_CONTACTS:
		strncpy(rec.contact->loginip,		sqlGetvalue(sqr, 0, 7), sizeof(rec.contact->loginip)-1);
		rec.contact->logintime=time_sql2unix(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.contact->logintoken,	sqlGetvalue(sqr, 0, 9), sizeof(rec.contact->logintoken)-1);
		strncpy(rec.contact->username,		sqlGetvalue(sqr, 0, 10), sizeof(rec.contact->username)-1);
		strncpy(rec.contact->password,		sqlGetvalue(sqr, 0, 11), sizeof(rec.contact->password)-1);
		rec.contact->enabled=atoi(sqlGetvalue(sqr, 0, 12));
		rec.contact->geozone=atoi(sqlGetvalue(sqr, 0, 13));
		rec.contact->timezone=atoi(sqlGetvalue(sqr, 0, 14));
		strncpy(rec.contact->surname,		sqlGetvalue(sqr, 0, 15), sizeof(rec.contact->surname)-1);
		strncpy(rec.contact->givenname,		sqlGetvalue(sqr, 0, 16), sizeof(rec.contact->givenname)-1);
		strncpy(rec.contact->salutation,	sqlGetvalue(sqr, 0, 17), sizeof(rec.contact->salutation)-1);
		strncpy(rec.contact->contacttype,	sqlGetvalue(sqr, 0, 18), sizeof(rec.contact->contacttype)-1);
		strncpy(rec.contact->referredby,	sqlGetvalue(sqr, 0, 19), sizeof(rec.contact->referredby)-1);
		strncpy(rec.contact->altcontact,	sqlGetvalue(sqr, 0, 20), sizeof(rec.contact->altcontact)-1);
		strncpy(rec.contact->prefbilling,	sqlGetvalue(sqr, 0, 21), sizeof(rec.contact->prefbilling)-1);
		strncpy(rec.contact->email,		sqlGetvalue(sqr, 0, 22), sizeof(rec.contact->email)-1);
		strncpy(rec.contact->homenumber,	sqlGetvalue(sqr, 0, 23), sizeof(rec.contact->homenumber)-1);
		strncpy(rec.contact->worknumber,	sqlGetvalue(sqr, 0, 24), sizeof(rec.contact->worknumber)-1);
		strncpy(rec.contact->faxnumber,		sqlGetvalue(sqr, 0, 25), sizeof(rec.contact->faxnumber)-1);
		strncpy(rec.contact->mobilenumber,	sqlGetvalue(sqr, 0, 26), sizeof(rec.contact->mobilenumber)-1);
		strncpy(rec.contact->jobtitle,		sqlGetvalue(sqr, 0, 27), sizeof(rec.contact->jobtitle)-1);
		strncpy(rec.contact->organization,	sqlGetvalue(sqr, 0, 28), sizeof(rec.contact->organization)-1);
		strncpy(rec.contact->homeaddress,	sqlGetvalue(sqr, 0, 29), sizeof(rec.contact->homeaddress)-1);
		strncpy(rec.contact->homelocality,	sqlGetvalue(sqr, 0, 30), sizeof(rec.contact->homelocality)-1);
		strncpy(rec.contact->homeregion,	sqlGetvalue(sqr, 0, 31), sizeof(rec.contact->homeregion)-1);
		strncpy(rec.contact->homecountry,	sqlGetvalue(sqr, 0, 32), sizeof(rec.contact->homecountry)-1);
		strncpy(rec.contact->homepostalcode,	sqlGetvalue(sqr, 0, 33), sizeof(rec.contact->homepostalcode)-1);
		strncpy(rec.contact->workaddress,	sqlGetvalue(sqr, 0, 34), sizeof(rec.contact->workaddress)-1);
		strncpy(rec.contact->worklocality,	sqlGetvalue(sqr, 0, 35), sizeof(rec.contact->worklocality)-1);
		strncpy(rec.contact->workregion,	sqlGetvalue(sqr, 0, 36), sizeof(rec.contact->workregion)-1);
		strncpy(rec.contact->workcountry,	sqlGetvalue(sqr, 0, 37), sizeof(rec.contact->workcountry)-1);
		strncpy(rec.contact->workpostalcode,	sqlGetvalue(sqr, 0, 38), sizeof(rec.contact->workpostalcode)-1);
		break;
	case DB_EVENTS:
		rec.event->assignedby=atoi(sqlGetvalue(sqr, 0, 7));
		rec.event->assignedto=atoi(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.event->eventname,		sqlGetvalue(sqr, 0, 9), sizeof(rec.event->eventname)-1);
		rec.event->eventtype=atoi(sqlGetvalue(sqr, 0, 10));
		rec.event->contactid=atoi(sqlGetvalue(sqr, 0, 11));
		rec.event->priority=atoi(sqlGetvalue(sqr, 0, 12));
		rec.event->reminder=atoi(sqlGetvalue(sqr, 0, 13));
		rec.event->eventstart=time_sql2unix(sqlGetvalue(sqr, 0, 14));
		rec.event->eventfinish=time_sql2unix(sqlGetvalue(sqr, 0, 15));
		rec.event->busy=atoi(sqlGetvalue(sqr, 0, 16));
		rec.event->status=atoi(sqlGetvalue(sqr, 0, 17));
		rec.event->closingstatus=atoi(sqlGetvalue(sqr, 0, 18));
		strncpy(rec.event->details,		sqlGetvalue(sqr, 0, 19), sizeof(rec.event->details)-1);
		if (rec.event->eventfinish<rec.event->eventstart) rec.event->eventfinish=rec.event->eventstart;
		break;
	case DB_FILES:
		strncpy(rec.file->filename,		sqlGetvalue(sqr, 0, 7), sizeof(rec.file->filename)-1);
		strncpy(rec.file->filepath,		sqlGetvalue(sqr, 0, 8), sizeof(rec.file->filepath)-1);
		strncpy(rec.file->filetype,		sqlGetvalue(sqr, 0, 9), sizeof(rec.file->filetype)-1);
		rec.file->uldate=time_sql2unix(sqlGetvalue(sqr, 0, 10));
		rec.file->lastdldate=time_sql2unix(sqlGetvalue(sqr, 0, 11));
		rec.file->numdownloads=atoi(sqlGetvalue(sqr, 0, 12));
		strncpy(rec.file->description,		sqlGetvalue(sqr, 0, 13), sizeof(rec.file->description)-1);
		break;
	case DB_FORUMGROUPS:
		strncpy(rec.forumgroup->title,		sqlGetvalue(sqr, 0, 7), sizeof(rec.forumgroup->title)-1);
		strncpy(rec.forumgroup->description,	sqlGetvalue(sqr, 0, 8), sizeof(rec.forumgroup->description)-1);
		break;
	case DB_GROUPS:
		strncpy(rec.group->groupname,		sqlGetvalue(sqr, 0, 7), sizeof(rec.group->groupname)-1);
		strncpy(rec.group->availability,	sqlGetvalue(sqr, 0, 8), sizeof(rec.group->availability)-1);
		strncpy(rec.group->motd,		sqlGetvalue(sqr, 0, 9), sizeof(rec.group->motd)-1);
		strncpy(rec.group->members,		sqlGetvalue(sqr, 0, 10), sizeof(rec.group->members)-1);
		if (strlen(rec.group->availability)==0) {
			strncpy(rec.group->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(rec.group->availability)-1);
		}
		break;
	case DB_MAILACCOUNTS:
		strncpy(rec.mailacct->accountname,	sqlGetvalue(sqr, 0, 7), sizeof(rec.mailacct->accountname)-1);
		strncpy(rec.mailacct->realname,		sqlGetvalue(sqr, 0, 8), sizeof(rec.mailacct->realname)-1);
		strncpy(rec.mailacct->organization,	sqlGetvalue(sqr, 0, 9), sizeof(rec.mailacct->organization)-1);
		strncpy(rec.mailacct->address,		sqlGetvalue(sqr, 0, 10), sizeof(rec.mailacct->address)-1);
		strncpy(rec.mailacct->hosttype,		sqlGetvalue(sqr, 0, 11), sizeof(rec.mailacct->hosttype)-1);
		strncpy(rec.mailacct->pophost,		sqlGetvalue(sqr, 0, 12), sizeof(rec.mailacct->pophost)-1);
		rec.mailacct->popport=atoi(sqlGetvalue(sqr, 0, 13));
		strncpy(rec.mailacct->smtphost,		sqlGetvalue(sqr, 0, 14), sizeof(rec.mailacct->smtphost)-1);
		rec.mailacct->smtpport=atoi(sqlGetvalue(sqr, 0, 15));
		strncpy(rec.mailacct->popusername,	sqlGetvalue(sqr, 0, 16), sizeof(rec.mailacct->popusername)-1);
		strncpy(rec.mailacct->poppassword,	DecodeBase64string(sid, sqlGetvalue(sqr, 0, 17)), sizeof(rec.mailacct->poppassword)-1);
		rec.mailacct->lastcount=atoi(sqlGetvalue(sqr, 0, 18));
		rec.mailacct->notify=atoi(sqlGetvalue(sqr, 0, 19));
		strncpy(rec.mailacct->signature,	sqlGetvalue(sqr, 0, 20), sizeof(rec.mailacct->signature)-1);
		break;
	case DB_NOTES:
		strncpy(rec.note->tablename,		sqlGetvalue(sqr, 0, 7), sizeof(rec.note->tablename)-1);
		rec.note->tableindex=atoi(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.note->notetitle,		sqlGetvalue(sqr, 0, 9), sizeof(rec.note->notetitle)-1);
		strncpy(rec.note->notetext,		sqlGetvalue(sqr, 0, 10), sizeof(rec.note->notetext)-1);
		if (strlen(rec.note->notetitle)==0) strncpy(rec.note->notetitle, "unnamed", sizeof(rec.note->notetitle)-1);
		break;
	case DB_ORDERS:
		rec.order->contactid=atoi(sqlGetvalue(sqr, 0, 7));
		rec.order->userid=atoi(sqlGetvalue(sqr, 0, 8));
		rec.order->orderdate=time_sql2unix(sqlGetvalue(sqr, 0, 9));
		strncpy(rec.order->ordertype,		sqlGetvalue(sqr, 0, 10), sizeof(rec.order->ordertype)-1);
		strncpy(rec.order->paymentmethod,	sqlGetvalue(sqr, 0, 11), sizeof(rec.order->paymentmethod)-1);
		rec.order->paymentdue=(float)atof(sqlGetvalue(sqr, 0, 12));
		rec.order->paymentreceived=(float)atof(sqlGetvalue(sqr, 0, 13));
		strncpy(rec.order->details,		sqlGetvalue(sqr, 0, 14), sizeof(rec.order->details)-1);
		break;
	case DB_ORDERITEMS:
		rec.orderitem->orderid=atoi(sqlGetvalue(sqr, 0, 7));
		rec.orderitem->productid=atoi(sqlGetvalue(sqr, 0, 8));
		rec.orderitem->quantity=(float)atof(sqlGetvalue(sqr, 0, 9));
		rec.orderitem->discount=(float)atof(sqlGetvalue(sqr, 0, 10));
		rec.orderitem->unitprice=(float)atof(sqlGetvalue(sqr, 0, 11));
		rec.orderitem->internalcost=(float)atof(sqlGetvalue(sqr, 0, 12));
		rec.orderitem->tax1=(float)atof(sqlGetvalue(sqr, 0, 13));
		rec.orderitem->tax2=(float)atof(sqlGetvalue(sqr, 0, 14));
		break;
	case DB_PRODUCTS:
		strncpy(rec.product->productname,	sqlGetvalue(sqr, 0, 7), sizeof(rec.product->productname)-1);
		strncpy(rec.product->category,		sqlGetvalue(sqr, 0, 8), sizeof(rec.product->category)-1);
		rec.product->discount=(float)atof(sqlGetvalue(sqr, 0, 9));
		rec.product->unitprice=(float)atof(sqlGetvalue(sqr, 0, 10));
		rec.product->internalcost=(float)atof(sqlGetvalue(sqr, 0, 11));
		rec.product->tax1=(float)atof(sqlGetvalue(sqr, 0, 12));
		rec.product->tax2=(float)atof(sqlGetvalue(sqr, 0, 13));
		strncpy(rec.product->details,		sqlGetvalue(sqr, 0, 14), sizeof(rec.product->details)-1);
		break;
	case DB_QUERIES:
		strncpy(rec.query->queryname,		sqlGetvalue(sqr, 0, 7), sizeof(rec.query->queryname)-1);
		strncpy(rec.query->query,		sqlGetvalue(sqr, 0, 8), sizeof(rec.query->query)-1);
		break;
	case DB_TASKS:
		rec.task->assignedby=atoi(sqlGetvalue(sqr, 0, 7));
		rec.task->assignedto=atoi(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.task->taskname,		sqlGetvalue(sqr, 0, 9), sizeof(rec.task->taskname)-1);
		rec.task->duedate=time_sql2unix(sqlGetvalue(sqr, 0, 10));
		rec.task->priority=atoi(sqlGetvalue(sqr, 0, 11));
		rec.task->reminder=atoi(sqlGetvalue(sqr, 0, 12));
		rec.task->status=atoi(sqlGetvalue(sqr, 0, 13));
		strncpy(rec.task->details,		sqlGetvalue(sqr, 0, 14), sizeof(rec.task->details)-1);
		break;
	case DB_PROFILE:
	case DB_USERS:
		strncpy(rec.user->loginip,          sqlGetvalue(sqr, 0, 7), sizeof(rec.user->loginip)-1);
		rec.user->logintime=time_sql2unix(sqlGetvalue(sqr, 0, 8));
		strncpy(rec.user->logintoken,       sqlGetvalue(sqr, 0, 9), sizeof(rec.user->logintoken)-1);
		strncpy(rec.user->username,         sqlGetvalue(sqr, 0, 10), sizeof(rec.user->username)-1);
		strncpy(rec.user->password,         sqlGetvalue(sqr, 0, 11), sizeof(rec.user->password)-1);
		rec.user->groupid=atoi(sqlGetvalue(sqr, 0, 12));
		rec.user->enabled=atoi(sqlGetvalue(sqr, 0, 13));
		rec.user->authadmin=atoi(sqlGetvalue(sqr, 0, 14));
		rec.user->authbookmarks=atoi(sqlGetvalue(sqr, 0, 15));
		rec.user->authcalendar=atoi(sqlGetvalue(sqr, 0, 16));
		rec.user->authcalls=atoi(sqlGetvalue(sqr, 0, 17));
		rec.user->authcontacts=atoi(sqlGetvalue(sqr, 0, 18));
		rec.user->authfiles=atoi(sqlGetvalue(sqr, 0, 19));
		rec.user->authforums=atoi(sqlGetvalue(sqr, 0, 20));
		rec.user->authmessages=atoi(sqlGetvalue(sqr, 0, 21));
		rec.user->authorders=atoi(sqlGetvalue(sqr, 0, 22));
		rec.user->authprofile=atoi(sqlGetvalue(sqr, 0, 23));
		rec.user->authquery=atoi(sqlGetvalue(sqr, 0, 24));
		rec.user->authwebmail=atoi(sqlGetvalue(sqr, 0, 25));
		rec.user->prefdaystart=atoi(sqlGetvalue(sqr, 0, 26));
		rec.user->prefdaylength=atoi(sqlGetvalue(sqr, 0, 27));
		rec.user->prefmailcurrent=atoi(sqlGetvalue(sqr, 0, 28));
		rec.user->prefmaildefault=atoi(sqlGetvalue(sqr, 0, 29));
		rec.user->prefmaxlist=atoi(sqlGetvalue(sqr, 0, 30));
		rec.user->prefmenustyle=atoi(sqlGetvalue(sqr, 0, 31));
		rec.user->preftimezone=atoi(sqlGetvalue(sqr, 0, 32));
		rec.user->prefgeozone=atoi(sqlGetvalue(sqr, 0, 33));
		strncpy(rec.user->availability,     sqlGetvalue(sqr, 0, 34), sizeof(rec.user->availability)-1);
		strncpy(rec.user->surname,          sqlGetvalue(sqr, 0, 35), sizeof(rec.user->surname)-1);
		strncpy(rec.user->givenname,        sqlGetvalue(sqr, 0, 36), sizeof(rec.user->givenname)-1);
		strncpy(rec.user->jobtitle,         sqlGetvalue(sqr, 0, 37), sizeof(rec.user->jobtitle)-1);
		strncpy(rec.user->division,         sqlGetvalue(sqr, 0, 38), sizeof(rec.user->division)-1);
		strncpy(rec.user->supervisor,       sqlGetvalue(sqr, 0, 39), sizeof(rec.user->supervisor)-1);
		strncpy(rec.user->address,          sqlGetvalue(sqr, 0, 40), sizeof(rec.user->address)-1);
		strncpy(rec.user->locality,         sqlGetvalue(sqr, 0, 41), sizeof(rec.user->locality)-1);
		strncpy(rec.user->region,           sqlGetvalue(sqr, 0, 42), sizeof(rec.user->region)-1);
		strncpy(rec.user->country,          sqlGetvalue(sqr, 0, 43), sizeof(rec.user->country)-1);
		strncpy(rec.user->postalcode,       sqlGetvalue(sqr, 0, 44), sizeof(rec.user->postalcode)-1);
		strncpy(rec.user->homenumber,       sqlGetvalue(sqr, 0, 45), sizeof(rec.user->homenumber)-1);
		strncpy(rec.user->worknumber,       sqlGetvalue(sqr, 0, 46), sizeof(rec.user->worknumber)-1);
		strncpy(rec.user->faxnumber,        sqlGetvalue(sqr, 0, 47), sizeof(rec.user->faxnumber)-1);
		strncpy(rec.user->cellnumber,       sqlGetvalue(sqr, 0, 48), sizeof(rec.user->cellnumber)-1);
		strncpy(rec.user->pagernumber,      sqlGetvalue(sqr, 0, 49), sizeof(rec.user->pagernumber)-1);
		strncpy(rec.user->email,            sqlGetvalue(sqr, 0, 50), sizeof(rec.user->email)-1);
		strncpy(rec.user->birthdate,        sqlGetvalue(sqr, 0, 51), 10);
		strncpy(rec.user->hiredate,         sqlGetvalue(sqr, 0, 52), 10);
		strncpy(rec.user->sin,              sqlGetvalue(sqr, 0, 53), sizeof(rec.user->sin)-1);
		strncpy(rec.user->isactive,         sqlGetvalue(sqr, 0, 54), sizeof(rec.user->isactive)-1);
		if (strlen(rec.user->availability)==0) {
			strncpy(rec.user->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(rec.user->availability)-1);
		}
		if (rec.user->authadmin&A_ADMIN) rec.user->authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authbookmarks&A_ADMIN) rec.user->authbookmarks=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authcalendar&A_ADMIN) rec.user->authcalendar=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authcalls&A_ADMIN) rec.user->authcalls=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authcontacts&A_ADMIN) rec.user->authcontacts=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authfiles&A_ADMIN) rec.user->authfiles=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authforums&A_ADMIN) rec.user->authforums=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authmessages&A_ADMIN) rec.user->authmessages=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authorders&A_ADMIN) rec.user->authorders=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authprofile&A_ADMIN) rec.user->authprofile=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authquery&A_ADMIN) rec.user->authquery=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->authwebmail&A_ADMIN) rec.user->authwebmail=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		if (rec.user->prefdaylength+rec.user->prefdaystart>24) rec.user->prefdaylength=24-rec.user->prefdaystart;
		break;
	case DB_ZONES:
		strncpy(rec.zone->zonename,       sqlGetvalue(sqr, 0, 7), sizeof(rec.zone->zonename)-1);
		break;
	default:
		break;
	}
	sqlFreeconnect(sqr);
	return 0;
}

int db_write(CONNECTION *sid, short int table, int index, void *record)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	int sqr;
	union rec_u rec;

	rec.head=record;
	rec.bookmark=record;
	rec.call=record;
	rec.contact=record;
	rec.event=record;
	rec.forumgroup=record;
	rec.group=record;
	rec.note=record;
	rec.task=record;
	rec.user=record;
	switch (table) {
	case DB_BOOKMARKS:
		authlevel=auth_priv(sid, AUTH_BOOKMARKS);
		break;
	case DB_CALLS:
		authlevel=auth_priv(sid, AUTH_CALLS);
		break;
	case DB_CONTACTS:
		authlevel=auth_priv(sid, AUTH_CONTACTS);
		break;
	case DB_EVENTS:
		authlevel=auth_priv(sid, AUTH_CALENDAR);
		break;
	case DB_FORUMGROUPS:
		authlevel=auth_priv(sid, AUTH_FORUMS);
		break;
	case DB_GROUPS:
		authlevel=auth_priv(sid, AUTH_ADMIN);
		break;
	case DB_NOTES:
		authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE;
		if (auth_priv(sid, AUTH_ADMIN)&A_ADMIN) {
			authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		}
		break;
	case DB_PROFILE:
		authlevel=auth_priv(sid, AUTH_PROFILE);
		if (authlevel&A_MODIFY) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		index=sid->dat->user_uid;
		break;
	case DB_QUERIES:
		authlevel=auth_priv(sid, AUTH_QUERIES);
		break;
	case DB_TASKS:
		authlevel=auth_priv(sid, AUTH_CALENDAR);
		break;
	case DB_USERS:
		authlevel=auth_priv(sid, AUTH_ADMIN);
		break;
	default:
		return -1;
		break;
	}
	if (authlevel<2) return -1;
//	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (index==0) {
		switch (table) {
		case DB_BOOKMARKS:
			if ((sqr=sqlQuery(sid, "SELECT max(bookmarkid) FROM gw_bookmarks"))<0) return -1;
			rec.bookmark->bookmarkid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.bookmark->bookmarkid<1) rec.bookmark->bookmarkid=1;
			strcpy(query, "INSERT INTO gw_bookmarks (bookmarkid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, folderid, bookmarkname, bookmarkurl) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.bookmark->bookmarkid, curdate, curdate, rec.bookmark->obj_uid, rec.bookmark->obj_gid, rec.bookmark->obj_gperm, rec.bookmark->obj_operm);
			strcatf(query, "'%d', ", rec.bookmark->folderid);
			strcatf(query, "'%s', ", str2sql(sid, rec.bookmark->bookmarkname));
			strcatf(query, "'%s')", str2sql(sid, rec.bookmark->bookmarkurl));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.bookmark->bookmarkid;
			break;
		case DB_CALLS:
			if ((sqr=sqlQuery(sid, "SELECT max(callid) FROM gw_calls"))<0) return -1;
			rec.call->callid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.call->callid<1) rec.call->callid=1;
			strcpy(query, "INSERT INTO gw_calls (callid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, assignedby, assignedto, callname, calldate, contactid, action, status, details) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.call->callid, curdate, curdate, rec.call->obj_uid, rec.call->obj_gid, rec.call->obj_gperm, rec.call->obj_operm);
			strcatf(query, "'%d', ", rec.call->assignedby);
			strcatf(query, "'%d', ", rec.call->assignedto);
			strcatf(query, "'%s', ", str2sql(sid, rec.call->callname));
			strcatf(query, "'%s', ", time_unix2sql(sid, rec.call->calldate));
			strcatf(query, "'%d', ", rec.call->contactid);
			strcatf(query, "'%d', ", rec.call->action);
			strcatf(query, "'%d', ", rec.call->status);
			strcatf(query, "'%s')", str2sql(sid, rec.call->details));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.call->callid;
			break;
		case DB_CONTACTS:
			if ((sqr=sqlQuery(sid, "SELECT max(contactid) FROM gw_contacts"))<0) return -1;
			rec.contact->contactid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.contact->contactid<1) rec.contact->contactid=1;
			strcpy(query, "INSERT INTO gw_contacts (contactid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, loginip, logintime, logintoken, username, password, enabled, geozone, timezone, surname, givenname, salutation, contacttype, referredby, altcontact, prefbilling, email, homenumber, worknumber, faxnumber, mobilenumber, jobtitle, organization, homeaddress, homelocality, homeregion, homecountry, homepostalcode, workaddress, worklocality, workregion, workcountry, workpostalcode) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.contact->contactid, curdate, curdate, rec.contact->obj_uid, rec.contact->obj_gid, rec.contact->obj_gperm, rec.contact->obj_operm);
			strcatf(query, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->username));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->password));
			strcatf(query, "'%d', ", rec.contact->enabled);
			strcatf(query, "'%d', ", rec.contact->geozone);
			strcatf(query, "'%d', ", rec.contact->timezone);
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->surname));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->givenname));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->salutation));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->contacttype));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->referredby));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->altcontact));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->prefbilling));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->email));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homenumber));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->worknumber));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->faxnumber));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->mobilenumber));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->jobtitle));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->organization));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homeaddress));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homelocality));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homeregion));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homecountry));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->homepostalcode));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->workaddress));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->worklocality));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->workregion));
			strcatf(query, "'%s', ", str2sql(sid, rec.contact->workcountry));
			strcatf(query, "'%s')", str2sql(sid, rec.contact->workpostalcode));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.contact->contactid;
			break;
		case DB_EVENTS:
			if ((sqr=sqlQuery(sid, "SELECT max(eventid) FROM gw_events"))<0) return -1;
			rec.event->eventid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.event->eventid<1) rec.event->eventid=1;
			strcpy(query, "INSERT INTO gw_events (eventid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, eventname, assignedby, assignedto, eventtype, contactid, priority, reminder, eventstart, eventfinish, busy, status, closingstatus, details) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.event->eventid, curdate, curdate, rec.event->obj_uid, rec.event->obj_gid, rec.event->obj_gperm, rec.event->obj_operm);
			strcatf(query, "'%s', ", str2sql(sid, rec.event->eventname));
			strcatf(query, "'%d', ", rec.event->assignedby);
			strcatf(query, "'%d', ", rec.event->assignedto);
			strcatf(query, "'%d', ", rec.event->eventtype);
			strcatf(query, "'%d', ", rec.event->contactid);
			strcatf(query, "'%d', ", rec.event->priority);
			strcatf(query, "'%d', ", rec.event->reminder);
			strcatf(query, "'%s', ", time_unix2sql(sid, rec.event->eventstart));
			strcatf(query, "'%s', ", time_unix2sql(sid, rec.event->eventfinish));
			strcatf(query, "'%d', ", rec.event->busy);
			strcatf(query, "'%d', ", rec.event->status);
			strcatf(query, "'%d', ", rec.event->closingstatus);
			strcatf(query, "'%s')", str2sql(sid, rec.event->details));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.event->eventid;
			break;
		case DB_FORUMGROUPS:
			if ((sqr=sqlQuery(sid, "SELECT max(forumgroupid) FROM gw_forumgroups"))<0) return -1;
			rec.forumgroup->forumgroupid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.forumgroup->forumgroupid<1) rec.forumgroup->forumgroupid=1;
			strcpy(query, "INSERT INTO gw_forumgroups (forumgroupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, title, description) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.forumgroup->forumgroupid, curdate, curdate, rec.forumgroup->obj_uid, rec.forumgroup->obj_gid, rec.forumgroup->obj_gperm, rec.forumgroup->obj_operm);
			strcatf(query, "'%s', ", str2sql(sid, rec.forumgroup->title));
			strcatf(query, "'%s')", str2sql(sid, rec.forumgroup->description));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.forumgroup->forumgroupid;
			break;
		case DB_GROUPS:
			break;
		case DB_NOTES:
			if ((sqr=sqlQuery(sid, "SELECT max(noteid) FROM gw_notes"))<0) return -1;
			rec.query->queryid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.query->queryid<1) rec.query->queryid=1;
			strcpy(query, "INSERT INTO gw_notes (noteid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, tablename, tableindex, notetitle, notetext) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.note->noteid, curdate, curdate, rec.note->obj_uid, rec.note->obj_gid, rec.note->obj_gperm, rec.note->obj_operm);
			strcatf(query, "'%s', ", str2sql(sid, rec.note->tablename));
			strcatf(query, "'%d', ", rec.note->tableindex);
			strcatf(query, "'%s', ", str2sql(sid, rec.note->notetitle));
			strcatf(query, "'");
			strncat(query, rec.note->notetext, sizeof(query)-strlen(query)-3);
			strncat(query, "')", sizeof(query)-strlen(query)-1);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.note->noteid;
			break;
		case DB_QUERIES:
			if ((sqr=sqlQuery(sid, "SELECT max(queryid) FROM gw_queries"))<0) return -1;
			rec.query->queryid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.query->queryid<1) rec.query->queryid=1;
			strcpy(query, "INSERT INTO gw_queries (queryid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, queryname, query) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.query->queryid, curdate, curdate, rec.query->obj_uid, rec.query->obj_gid, rec.query->obj_gperm, rec.query->obj_operm);
			strcatf(query, "'%s', ", str2sql(sid, rec.query->queryname));
			strcatf(query, "'%s')", str2sql(sid, rec.query->query));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.query->queryid;
			break;
		case DB_TASKS:
			if ((sqr=sqlQuery(sid, "SELECT max(taskid) FROM gw_tasks"))<0) return -1;
			rec.task->taskid=atoi(sqlGetvalue(sqr, 0, 0))+1;
			sqlFreeconnect(sqr);
			if (rec.task->taskid<1) rec.task->taskid=1;
			strcpy(query, "INSERT INTO gw_tasks (taskid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, assignedby, assignedto, taskname, status, priority, reminder, duedate, details) values (");
			strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.task->taskid, curdate, curdate, rec.task->obj_uid, rec.task->obj_gid, rec.task->obj_gperm, rec.task->obj_operm);
			strcatf(query, "'%d', ", rec.task->assignedby);
			strcatf(query, "'%d', ", rec.task->assignedto);
			strcatf(query, "'%s', ", str2sql(sid, rec.task->taskname));
			strcatf(query, "'%d', ", rec.task->status);
			strcatf(query, "'%d', ", rec.task->priority);
			strcatf(query, "'%d', ", rec.task->reminder);
			strcatf(query, "'%s', ", time_unix2sql(sid, rec.task->duedate));
			strcatf(query, "'%s')", str2sql(sid, rec.task->details));
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.task->taskid;
			break;
		case DB_USERS:
			break;
		default:
			return -1;
			break;
		}
		return -1;
	} else {
		switch (table) {
		case DB_BOOKMARKS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_bookmarks SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.bookmark->obj_uid, rec.bookmark->obj_gid, rec.bookmark->obj_gperm, rec.bookmark->obj_operm);
			strcatf(query, "folderid = '%d', ", rec.bookmark->folderid);
			strcatf(query, "bookmarkname = '%s', ", str2sql(sid, rec.bookmark->bookmarkname));
			strcatf(query, "bookmarkurl = '%s'", str2sql(sid, rec.bookmark->bookmarkurl));
			strcatf(query, " WHERE bookmarkid = %d", rec.bookmark->bookmarkid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.bookmark->bookmarkid;
			break;
		case DB_CALLS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_calls SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.call->obj_uid, rec.call->obj_gid, rec.call->obj_gperm, rec.call->obj_operm);
			strcatf(query, "assignedby = '%d', ", rec.call->assignedby);
			strcatf(query, "assignedto = '%d', ", rec.call->assignedto);
			strcatf(query, "callname = '%s', ", str2sql(sid, rec.call->callname));
			strcatf(query, "calldate = '%s', ", time_unix2sql(sid, rec.call->calldate));
			strcatf(query, "contactid = '%d', ", rec.call->contactid);
			strcatf(query, "action = '%d', ", rec.call->action);
			strcatf(query, "status = '%d', ", rec.call->status);
			strcatf(query, "details = '%s'", str2sql(sid, rec.call->details));
			strcatf(query, " WHERE callid = %d", rec.call->callid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.call->callid;
			break;
		case DB_CONTACTS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_contacts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.contact->obj_uid, rec.contact->obj_gid, rec.contact->obj_gperm, rec.contact->obj_operm);
			strcatf(query, "username = '%s', ", str2sql(sid, rec.contact->username));
			strcatf(query, "password = '%s', ", str2sql(sid, rec.contact->password));
			strcatf(query, "enabled = '%d', ", rec.contact->enabled);
			strcatf(query, "geozone = '%d', ", rec.contact->geozone);
			strcatf(query, "timezone = '%d', ", rec.contact->timezone);
			strcatf(query, "surname = '%s', ", str2sql(sid, rec.contact->surname));
			strcatf(query, "givenname = '%s', ", str2sql(sid, rec.contact->givenname));
			strcatf(query, "salutation = '%s', ", str2sql(sid, rec.contact->salutation));
			strcatf(query, "contacttype = '%s', ", str2sql(sid, rec.contact->contacttype));
			strcatf(query, "referredby = '%s', ", str2sql(sid, rec.contact->referredby));
			strcatf(query, "altcontact = '%s', ", str2sql(sid, rec.contact->altcontact));
			strcatf(query, "prefbilling = '%s', ", str2sql(sid, rec.contact->prefbilling));
			strcatf(query, "email = '%s', ", str2sql(sid, rec.contact->email));
			strcatf(query, "homenumber = '%s', ", str2sql(sid, rec.contact->homenumber));
			strcatf(query, "worknumber = '%s', ", str2sql(sid, rec.contact->worknumber));
			strcatf(query, "faxnumber = '%s', ", str2sql(sid, rec.contact->faxnumber));
			strcatf(query, "mobilenumber = '%s', ", str2sql(sid, rec.contact->mobilenumber));
			strcatf(query, "jobtitle = '%s', ", str2sql(sid, rec.contact->jobtitle));
			strcatf(query, "organization = '%s', ", str2sql(sid, rec.contact->organization));
			strcatf(query, "homeaddress = '%s', ", str2sql(sid, rec.contact->homeaddress));
			strcatf(query, "homelocality = '%s', ", str2sql(sid, rec.contact->homelocality));
			strcatf(query, "homeregion = '%s', ", str2sql(sid, rec.contact->homeregion));
			strcatf(query, "homecountry = '%s', ", str2sql(sid, rec.contact->homecountry));
			strcatf(query, "homepostalcode = '%s', ", str2sql(sid, rec.contact->homepostalcode));
			strcatf(query, "workaddress = '%s', ", str2sql(sid, rec.contact->workaddress));
			strcatf(query, "worklocality = '%s', ", str2sql(sid, rec.contact->worklocality));
			strcatf(query, "workregion = '%s', ", str2sql(sid, rec.contact->workregion));
			strcatf(query, "workcountry = '%s', ", str2sql(sid, rec.contact->workcountry));
			strcatf(query, "workpostalcode = '%s'", str2sql(sid, rec.contact->workpostalcode));
			strcatf(query, " WHERE contactid = %d", rec.contact->contactid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.contact->contactid;
			break;
		case DB_EVENTS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_events SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.event->obj_uid, rec.event->obj_gid, rec.event->obj_gperm, rec.event->obj_operm);
			strcatf(query, "eventname = '%s', ", str2sql(sid, rec.event->eventname));
			strcatf(query, "assignedby = '%d', ", rec.event->assignedby);
			strcatf(query, "assignedto = '%d', ", rec.event->assignedto);
			strcatf(query, "eventtype = '%d', ", rec.event->eventtype);
			strcatf(query, "contactid = '%d', ", rec.event->contactid);
			strcatf(query, "priority = '%d', ", rec.event->priority);
			strcatf(query, "reminder = '%d', ", rec.event->reminder);
			strcatf(query, "eventstart = '%s', ", time_unix2sql(sid, rec.event->eventstart));
			strcatf(query, "eventfinish = '%s', ", time_unix2sql(sid, rec.event->eventfinish));
			strcatf(query, "busy = '%d', ", rec.event->busy);
			strcatf(query, "status = '%d', ", rec.event->status);
			strcatf(query, "closingstatus = '%d', ", rec.event->closingstatus);
			strcatf(query, "details = '%s'", str2sql(sid, rec.event->details));
			strcatf(query, " WHERE eventid = %d", rec.event->eventid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.event->eventid;
			break;
		case DB_FORUMGROUPS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_forumgroups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.forumgroup->obj_uid, rec.forumgroup->obj_gid, rec.forumgroup->obj_gperm, rec.forumgroup->obj_operm);
			strcatf(query, "title = '%s', ", str2sql(sid, rec.forumgroup->title));
			strcatf(query, "description = '%s'", str2sql(sid, rec.forumgroup->description));
			strcatf(query, " WHERE forumgroupid = %d", rec.forumgroup->forumgroupid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.forumgroup->forumgroupid;
			break;
		case DB_GROUPS:
			break;
		case DB_NOTES:
			snprintf(query, sizeof(query)-1, "UPDATE gw_notes SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.note->obj_uid, rec.note->obj_gid, rec.note->obj_gperm, rec.note->obj_operm);
			strcatf(query, "tablename = '%s', ", str2sql(sid, rec.note->tablename));
			strcatf(query, "tableindex = '%d', ", rec.note->tableindex);
			strcatf(query, "notetitle = '%s', ", str2sql(sid, rec.note->notetitle));
			strcatf(query, "notetext = '");
			strncat(query, rec.note->notetext, sizeof(query)-strlen(query)-3);
			strncat(query, "'", sizeof(query)-strlen(query)-27);
			strcatf(query, " WHERE noteid = %d", rec.note->noteid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.note->noteid;
			break;
		case DB_QUERIES:
			snprintf(query, sizeof(query)-1, "UPDATE gw_queries SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.query->obj_uid, rec.query->obj_gid, rec.query->obj_gperm, rec.query->obj_operm);
			strcatf(query, "queryname = '%s', ", str2sql(sid, rec.query->queryname));
			strcatf(query, "query = '%s'", str2sql(sid, rec.query->query));
			strcatf(query, " WHERE queryid = %d", rec.query->queryid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.query->queryid;
			break;
		case DB_TASKS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_tasks SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.task->obj_uid, rec.task->obj_gid, rec.task->obj_gperm, rec.task->obj_operm);
			strcatf(query, "assignedby = '%d', ", rec.task->assignedby);
			strcatf(query, "assignedto = '%d', ", rec.task->assignedto);
			strcatf(query, "taskname = '%s', ", str2sql(sid, rec.task->taskname));
			strcatf(query, "status = '%d', ", rec.task->status);
			strcatf(query, "priority = '%d', ", rec.task->priority);
			strcatf(query, "reminder = '%d', ", rec.task->reminder);
			strcatf(query, "duedate = '%s', ", time_unix2sql(sid, rec.task->duedate));
			strcatf(query, "details = '%s'", str2sql(sid, rec.task->details));
			strcatf(query, " WHERE taskid = %d", rec.task->taskid);
			if (sqlUpdate(sid, query)<0) return -1;
			return rec.task->taskid;
			break;
		case DB_PROFILE:
		case DB_USERS:
			break;
		default:
			break;
		}
	}
	return -1;
}

char *db_avail_getweek(int userid, char *availability)
{
	CONNECTION *sid=&conn[getsid()];
	char availbuff[170];
	char gavailability[673];
	char uavailability[673];
	int sqr;
	int groupid;
	int i;

	if ((sqr=sqlQueryf(sid, "SELECT groupid, availability FROM gw_users WHERE userid = %d", userid))<0) return NULL;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return NULL;
	}
	memset(gavailability, 0, sizeof(gavailability));
	memset(uavailability, 0, sizeof(uavailability));
	groupid=atoi(sqlGetvalue(sqr, 0, 0));
	memset(availbuff, 0, sizeof(availbuff));
	strncpy(availbuff, sqlGetvalue(sqr, 0, 1), sizeof(availbuff)-1);
	sqlFreeconnect(sqr);
	if (strlen(availbuff)!=168) {
		for (i=0;i<672;i++) {
			uavailability[i]='0';
		}
	} else {
		for (i=0;i<672;i++) {
			uavailability[i]=availbuff[(int)(i/4)];
		}
	}
	if ((sqr=sqlQueryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", groupid))<0) return NULL;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return NULL;
	}
	memset(availbuff, 0, sizeof(availbuff));
	strncpy(availbuff, sqlGetvalue(sqr, 0, 0), sizeof(availbuff)-1);
	sqlFreeconnect(sqr);
	if (strlen(availbuff)!=168) {
		for (i=0;i<672;i++) {
			gavailability[i]='0';
		}
	} else {
		for (i=0;i<672;i++) {
			gavailability[i]=availbuff[(int)(i/4)];
		}
	}
	for (i=0;i<672;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='0';
		}
	}
	snprintf(availability, 673, "%s", uavailability);
	return availability;
}

char *db_avail_getfullweek(int userid, int record, time_t eventstart, char *availability)
{
	CONNECTION *sid=&conn[getsid()];
	char timebuf1[40];
	char timebuf2[40];
	int i, i2, j, k;
	int sqr;
	struct tm t1;
	struct tm t2;
	time_t eventfinish;

//	You MUST call db_avail_getweek() first!
	if (strlen(availability)!=672) return NULL;
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&eventstart));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	eventstart=((int)(eventstart/86400)*86400)-(t1.tm_wday*86400);
	eventstart-=time_tzoffset2(sid, eventstart, userid);
	eventfinish=eventstart+604800;
	strftime(timebuf1, sizeof(timebuf1)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventstart));
	strftime(timebuf2, sizeof(timebuf2)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventfinish));
	if (strcmp(config.sql_type, "ODBC")==0) {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= #%s# and eventstart < #%s#) or (eventfinish > #%s# and eventfinish < #%s#) or (eventstart < #%s# and eventfinish >= #%s#)) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return NULL;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT eventid, eventstart, eventfinish FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= '%s' and eventstart < '%s') or (eventfinish > '%s' and eventfinish < '%s') or (eventstart < '%s' and eventfinish >= '%s')) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return NULL;
	}
	for (i=0;i<sqlNumtuples(sqr);i++) {
		eventstart=time_sql2unix(sqlGetvalue(sqr, i, 1));
		eventstart+=time_tzoffset2(sid, eventstart, userid);
		eventfinish=time_sql2unix(sqlGetvalue(sqr, i, 2));
		eventfinish+=time_tzoffset2(sid, eventfinish, userid);
		strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&eventstart));
		sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
		strftime(timebuf2, sizeof(timebuf2), "%w %H %M", gmtime(&eventfinish));
		sscanf(timebuf2, "%d %d %d", &t2.tm_wday, &t2.tm_hour, &t2.tm_min);
		j=(t1.tm_wday*96)+(t1.tm_hour*4)+(t1.tm_min/15);
		k=(t2.tm_wday*96)+(t2.tm_hour*4)+(t2.tm_min/15);
		if (k<j) k=j;
		if (k==j) k++;
		for (i2=j;i2<k;i2++) availability[i2]='0';
	}
	sqlFreeconnect(sqr);
//	logerror(sid, __FILE__, __LINE__, "================================================================================================");
//	logerror(sid, __FILE__, __LINE__, "000011112222333344445555666677778888999900001111222233334444555566667777888899990000111122223333");
//	for (i=0;i<7;i++) { ptemp=availability+(i*96); logerror(sid, __FILE__, __LINE__, "%.96s", ptemp); }
//	logerror(sid, __FILE__, __LINE__, "================================================================================================");
	return availability;
}

int db_availcheck(int userid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	CONNECTION *sid=&conn[getsid()];
	char availability[673];
	char timebuf1[40];
	char timebuf2[40];
	char *ptemp;
	int i, j, k;
	int ret;
	int sqr;
	time_t ts, tf;
	struct tm t1;
	struct tm t2;

	if (!busy) return 0;
	memset(availability, 0, sizeof(availability));
	ts=eventstart+time_tzoffset2(sid, eventstart, userid);
	tf=eventfinish+time_tzoffset2(sid, eventfinish, userid);
	if ((ptemp=db_avail_getweek(userid, availability))==NULL) return -1;
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&ts));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	strftime(timebuf2, sizeof(timebuf2), "%w %H %M", gmtime(&tf));
	sscanf(timebuf2, "%d %d %d", &t2.tm_wday, &t2.tm_hour, &t2.tm_min);
	j=(t1.tm_wday*96)+(t1.tm_hour*4)+(t1.tm_min/15);
	k=(t2.tm_wday*96)+(t2.tm_hour*4)+(t2.tm_min/15);
	if (k<j) k=j;
	if (k==j) k++;
	for (i=j;i<k;i++) {
		if (availability[i]!='1') return -1;
	}
	if ((ptemp=db_avail_getfullweek(userid, record, eventstart, availability))==NULL) return -2;
	for (i=j;i<k;i++) {
		if (availability[i]=='1') continue;
		strftime(timebuf1, sizeof(timebuf1)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventstart));
		strftime(timebuf2, sizeof(timebuf2)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventfinish));
		if (strcmp(config.sql_type, "ODBC")==0) {
			if ((sqr=sqlQueryf(sid, "SELECT eventid FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= #%s# and eventstart < #%s#) or (eventfinish > #%s# and eventfinish < #%s#) or (eventstart < #%s# and eventfinish >= #%s#)) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return -1;
		} else {
			if ((sqr=sqlQueryf(sid, "SELECT eventid FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= '%s' and eventstart < '%s') or (eventfinish > '%s' and eventfinish < '%s') or (eventstart < '%s' and eventfinish >= '%s')) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return -1;
		}
		if (sqlNumtuples(sqr)>0) {
			ret=atoi(sqlGetvalue(sqr, 0, 0));
			sqlFreeconnect(sqr);
			return ret;
		}
	}
	return 0;
}

int db_autoschedule(int userid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	CONNECTION *sid=&conn[getsid()];
	char availability[673];
	char availweekbuf[673];
	char timeblock[100];
	char timebuf1[40];
	char *ptemp;
	int i, j;
	int week=0;
	struct tm t1;
	time_t ts;
	time_t newtime;
//	int lasttz=time_tzoffset2(sid, eventstart, userid);

	if (!busy) return eventstart;
	memset(timeblock, 0, sizeof(timeblock));
	memset(availability, 0, sizeof(availability));
	if ((ptemp=db_avail_getweek(userid, availability))==NULL) return -1;
	memcpy(availweekbuf, availability, sizeof(availweekbuf));
	i=(int)((eventfinish-eventstart)/900);
	if (i<0) i=0;
	if (i>96) return -1;
	for (j=0;j<i;j++) timeblock[j]='1';
	if ((ptemp=strstr(availability, timeblock))==NULL) return -1;
	ts=eventstart+time_tzoffset2(sid, eventstart, userid);
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&ts));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	newtime=((int)(ts/86400)*86400)-(t1.tm_wday*86400);
	do {
/*
		if (lasttz!=time_tzoffset(sid, newtime+10800)) {
			lasttz=time_tzoffset(sid, newtime+10800);
//			memset(availability, 0, sizeof(availability));
//			if ((ptemp=db_avail_getweek(userid, availability))==NULL) return -1;
		}
*/
		memcpy(availweekbuf, availability, sizeof(availweekbuf));
		if ((ptemp=db_avail_getfullweek(userid, record, newtime, availweekbuf))==NULL) return -1;
		if (week==0) {
			if ((ptemp=strstr(availweekbuf+((ts/900)-(newtime/900)), timeblock))==NULL) {
				newtime+=604800;
				week++;
				continue;
			}
		} else {
			if ((ptemp=strstr(availweekbuf, timeblock))==NULL) {
				newtime+=604800;
				week++;
				continue;
			}
		}
		newtime-=time_tzoffset2(sid, newtime, userid);
		newtime+=(strlen(availweekbuf)-strlen(ptemp))*900;
		return newtime;
	} while (1);
	return newtime;
}

int db_autoassign(u_avail *uavail, int groupid, int zoneid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	CONNECTION *sid=&conn[getsid()];
	int i;
	int j;
	int rc;
	int sqr;

	uavail->userid=0;
	uavail->time=0;
	if (zoneid==0) {
		if ((sqr=sqlQueryf(sid, "SELECT userid FROM gw_users where groupid = %d", groupid))<0) return -1;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT userid FROM gw_users where groupid = %d and prefgeozone = %d", groupid, zoneid))<0) return -1;
	}
	for (i=0;i<sqlNumtuples(sqr);i++) {
		j=atoi(sqlGetvalue(sqr, i, 0));
		if ((rc=db_autoschedule(j, record, busy, eventstart, eventfinish))<0) continue;
		if ((rc<uavail->time)||(uavail->time==0)) {
			uavail->userid=j;
			uavail->time=rc;
		}
	}
	sqlFreeconnect(sqr);
	if (uavail->userid<=0) return -1;
	if (uavail->time<=0) return -1;
	return 0;
}
