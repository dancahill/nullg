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

union rec_u {
	REC_HEADER         *head;
	REC_BOOKMARK       *bookmark;
	REC_BOOKMARKFOLDER *bookmarkfolder;
	REC_CALL           *call;
	REC_CONTACT        *contact;
	REC_EVENT          *event;
	REC_FILE           *file;
	REC_FORUMGROUP     *forumgroup;
	REC_GROUP          *group;
	REC_MAILACCT       *mailacct;
	REC_NOTE           *note;
	REC_ORDER          *order;
	REC_ORDERITEM      *orderitem;
	REC_PRODUCT        *product;
	REC_QUERY          *query;
	REC_TASK           *task;
	REC_USER           *user;
	REC_ZONE           *zone;
};

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
			rec.call->callstart=time(NULL);
			rec.call->callfinish=time(NULL);
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
			rec.mailacct->lastcheck=time(NULL);
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
			strncpy(rec.user->availability, "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", sizeof(rec.user->availability)-1);
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
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_bookmarks where bookmarkid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_bookmarks where bookmarkid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_BOOKMARKFOLDERS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_bookmarkfolders where folderid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_bookmarkfolders where folderid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_CALLS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_calls where callid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_calls where callid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_CONTACTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_contacts where contactid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_contacts where contactid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_EVENTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_events where eventid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_events where eventid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_FILES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_files where fileid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_files where fileid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_FORUMGROUPS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_forumgroups where forumgroupid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_forumgroups where forumgroupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_GROUPS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_groups where groupid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_groups where groupid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_MAILACCOUNTS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_mailaccounts where mailaccountid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", index, sid->dat->user_uid))<0) return -1;
		}
		break;
	case DB_NOTES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_notes where noteid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_notes where noteid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_ORDERS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_orders where orderid = %d", index))<0) return -1;
		break;
	case DB_ORDERITEMS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_orderitems where orderitemid = %d", index))<0) return -1;
		break;
	case DB_PRODUCTS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_products where productid = %d", index))<0) return -1;
		break;
	case DB_QUERIES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_queries where queryid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_queries where queryid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_TASKS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_tasks where taskid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_tasks where taskid = %d and (obj_uid = %d or assignedby = %d or assignedto = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_PROFILE:
	case DB_USERS:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_users where userid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_users where userid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	case DB_ZONES:
		if (authlevel&A_ADMIN) {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_zones where zoneid = %d", index))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT * FROM gw_zones where zoneid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d)", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm))<0) return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	rec.head->recordid  = atoi(sql_getvalue(sqr, 0, 0));
	rec.head->obj_ctime = time_sql2unix(sql_getvalue(sqr, 0, 1));
	rec.head->obj_mtime = time_sql2unix(sql_getvalue(sqr, 0, 2));
	rec.head->obj_uid   = atoi(sql_getvalue(sqr, 0, 3));
	rec.head->obj_gid   = atoi(sql_getvalue(sqr, 0, 4));
	rec.head->obj_gperm = atoi(sql_getvalue(sqr, 0, 5));
	rec.head->obj_operm = atoi(sql_getvalue(sqr, 0, 6));
	switch (table) {
	case DB_BOOKMARKS:
		rec.bookmark->folderid=atoi(sql_getvalue(sqr, 0, 7));
		strncpy(rec.bookmark->bookmarkname,	sql_getvalue(sqr, 0, 8), sizeof(rec.bookmark->bookmarkname)-1);
		strncpy(rec.bookmark->bookmarkurl,	sql_getvalue(sqr, 0, 9), sizeof(rec.bookmark->bookmarkurl)-1);
		if (strlen(rec.bookmark->bookmarkname)==0) strncpy(rec.bookmark->bookmarkname, "unnamed", sizeof(rec.bookmark->bookmarkname)-1);
		break;
	case DB_BOOKMARKFOLDERS:
		rec.bookmarkfolder->parentid=atoi(sql_getvalue(sqr, 0, 7));
		strncpy(rec.bookmarkfolder->foldername,	sql_getvalue(sqr, 0, 8), sizeof(rec.bookmarkfolder->foldername)-1);
		if (strlen(rec.bookmarkfolder->foldername)==0) strncpy(rec.bookmarkfolder->foldername, "unnamed", sizeof(rec.bookmarkfolder->foldername)-1);
		break;
	case DB_CALLS:
		rec.call->assignedby=atoi(sql_getvalue(sqr, 0, 7));
		rec.call->assignedto=atoi(sql_getvalue(sqr, 0, 8));
		strncpy(rec.call->callname,		sql_getvalue(sqr, 0, 9), sizeof(rec.call->callname)-1);
		rec.call->callstart=time_sql2unix(sql_getvalue(sqr, 0, 10));
		rec.call->callfinish=time_sql2unix(sql_getvalue(sqr, 0, 11));
		rec.call->contactid=atoi(sql_getvalue(sqr, 0, 12));
		rec.call->action=atoi(sql_getvalue(sqr, 0, 13));
		rec.call->status=atoi(sql_getvalue(sqr, 0, 14));
		strncpy(rec.call->details,		sql_getvalue(sqr, 0, 15), sizeof(rec.call->details)-1);
		if (rec.call->callfinish<rec.call->callstart) rec.call->callfinish=rec.call->callstart;
		break;
	case DB_CONTACTS:
		strncpy(rec.contact->loginip,		sql_getvalue(sqr, 0, 7), sizeof(rec.contact->loginip)-1);
		rec.contact->logintime=time_sql2unix(sql_getvalue(sqr, 0, 8));
		strncpy(rec.contact->logintoken,	sql_getvalue(sqr, 0, 9), sizeof(rec.contact->logintoken)-1);
		strncpy(rec.contact->username,		sql_getvalue(sqr, 0, 10), sizeof(rec.contact->username)-1);
		strncpy(rec.contact->password,		sql_getvalue(sqr, 0, 11), sizeof(rec.contact->password)-1);
		rec.contact->enabled=atoi(sql_getvalue(sqr, 0, 12));
		rec.contact->geozone=atoi(sql_getvalue(sqr, 0, 13));
		rec.contact->timezone=atoi(sql_getvalue(sqr, 0, 14));
		strncpy(rec.contact->surname,		sql_getvalue(sqr, 0, 15), sizeof(rec.contact->surname)-1);
		strncpy(rec.contact->givenname,		sql_getvalue(sqr, 0, 16), sizeof(rec.contact->givenname)-1);
		strncpy(rec.contact->salutation,	sql_getvalue(sqr, 0, 17), sizeof(rec.contact->salutation)-1);
		strncpy(rec.contact->contacttype,	sql_getvalue(sqr, 0, 18), sizeof(rec.contact->contacttype)-1);
		strncpy(rec.contact->referredby,	sql_getvalue(sqr, 0, 19), sizeof(rec.contact->referredby)-1);
		strncpy(rec.contact->altcontact,	sql_getvalue(sqr, 0, 20), sizeof(rec.contact->altcontact)-1);
		strncpy(rec.contact->prefbilling,	sql_getvalue(sqr, 0, 21), sizeof(rec.contact->prefbilling)-1);
		strncpy(rec.contact->email,		sql_getvalue(sqr, 0, 22), sizeof(rec.contact->email)-1);
		strncpy(rec.contact->homenumber,	sql_getvalue(sqr, 0, 23), sizeof(rec.contact->homenumber)-1);
		strncpy(rec.contact->worknumber,	sql_getvalue(sqr, 0, 24), sizeof(rec.contact->worknumber)-1);
		strncpy(rec.contact->faxnumber,		sql_getvalue(sqr, 0, 25), sizeof(rec.contact->faxnumber)-1);
		strncpy(rec.contact->mobilenumber,	sql_getvalue(sqr, 0, 26), sizeof(rec.contact->mobilenumber)-1);
		strncpy(rec.contact->jobtitle,		sql_getvalue(sqr, 0, 27), sizeof(rec.contact->jobtitle)-1);
		strncpy(rec.contact->organization,	sql_getvalue(sqr, 0, 28), sizeof(rec.contact->organization)-1);
		strncpy(rec.contact->homeaddress,	sql_getvalue(sqr, 0, 29), sizeof(rec.contact->homeaddress)-1);
		strncpy(rec.contact->homelocality,	sql_getvalue(sqr, 0, 30), sizeof(rec.contact->homelocality)-1);
		strncpy(rec.contact->homeregion,	sql_getvalue(sqr, 0, 31), sizeof(rec.contact->homeregion)-1);
		strncpy(rec.contact->homecountry,	sql_getvalue(sqr, 0, 32), sizeof(rec.contact->homecountry)-1);
		strncpy(rec.contact->homepostalcode,	sql_getvalue(sqr, 0, 33), sizeof(rec.contact->homepostalcode)-1);
		strncpy(rec.contact->workaddress,	sql_getvalue(sqr, 0, 34), sizeof(rec.contact->workaddress)-1);
		strncpy(rec.contact->worklocality,	sql_getvalue(sqr, 0, 35), sizeof(rec.contact->worklocality)-1);
		strncpy(rec.contact->workregion,	sql_getvalue(sqr, 0, 36), sizeof(rec.contact->workregion)-1);
		strncpy(rec.contact->workcountry,	sql_getvalue(sqr, 0, 37), sizeof(rec.contact->workcountry)-1);
		strncpy(rec.contact->workpostalcode,	sql_getvalue(sqr, 0, 38), sizeof(rec.contact->workpostalcode)-1);
		break;
	case DB_EVENTS:
		rec.event->assignedby=atoi(sql_getvalue(sqr, 0, 7));
		rec.event->assignedto=atoi(sql_getvalue(sqr, 0, 8));
		strncpy(rec.event->eventname,		sql_getvalue(sqr, 0, 9), sizeof(rec.event->eventname)-1);
		rec.event->eventtype=atoi(sql_getvalue(sqr, 0, 10));
		rec.event->contactid=atoi(sql_getvalue(sqr, 0, 11));
		rec.event->priority=atoi(sql_getvalue(sqr, 0, 12));
		rec.event->reminder=atoi(sql_getvalue(sqr, 0, 13));
		rec.event->eventstart=time_sql2unix(sql_getvalue(sqr, 0, 14));
		rec.event->eventfinish=time_sql2unix(sql_getvalue(sqr, 0, 15));
		rec.event->busy=atoi(sql_getvalue(sqr, 0, 16));
		rec.event->status=atoi(sql_getvalue(sqr, 0, 17));
		rec.event->closingstatus=atoi(sql_getvalue(sqr, 0, 18));
		strncpy(rec.event->details,		sql_getvalue(sqr, 0, 19), sizeof(rec.event->details)-1);
		if (rec.event->eventfinish<rec.event->eventstart) rec.event->eventfinish=rec.event->eventstart;
		break;
	case DB_FILES:
		strncpy(rec.file->filename,		sql_getvalue(sqr, 0, 7), sizeof(rec.file->filename)-1);
		strncpy(rec.file->filepath,		sql_getvalue(sqr, 0, 8), sizeof(rec.file->filepath)-1);
		strncpy(rec.file->filetype,		sql_getvalue(sqr, 0, 9), sizeof(rec.file->filetype)-1);
		rec.file->uldate=time_sql2unix(sql_getvalue(sqr, 0, 10));
		rec.file->lastdldate=time_sql2unix(sql_getvalue(sqr, 0, 11));
		rec.file->numdownloads=atoi(sql_getvalue(sqr, 0, 12));
		strncpy(rec.file->description,		sql_getvalue(sqr, 0, 13), sizeof(rec.file->description)-1);
		break;
	case DB_FORUMGROUPS:
		strncpy(rec.forumgroup->title,		sql_getvalue(sqr, 0, 7), sizeof(rec.forumgroup->title)-1);
		strncpy(rec.forumgroup->description,	sql_getvalue(sqr, 0, 8), sizeof(rec.forumgroup->description)-1);
		break;
	case DB_GROUPS:
		strncpy(rec.group->groupname,		sql_getvalue(sqr, 0, 7), sizeof(rec.group->groupname)-1);
		strncpy(rec.group->availability,	sql_getvalue(sqr, 0, 8), sizeof(rec.group->availability)-1);
		strncpy(rec.group->motd,		sql_getvalue(sqr, 0, 9), sizeof(rec.group->motd)-1);
		strncpy(rec.group->members,		sql_getvalue(sqr, 0, 10), sizeof(rec.group->members)-1);
		if (strlen(rec.group->availability)==0) {
			strncpy(rec.group->availability, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(rec.group->availability)-1);
		}
		break;
	case DB_MAILACCOUNTS:
		strncpy(rec.mailacct->accountname,	sql_getvalue(sqr, 0, 7), sizeof(rec.mailacct->accountname)-1);
		strncpy(rec.mailacct->realname,		sql_getvalue(sqr, 0, 8), sizeof(rec.mailacct->realname)-1);
		strncpy(rec.mailacct->organization,	sql_getvalue(sqr, 0, 9), sizeof(rec.mailacct->organization)-1);
		strncpy(rec.mailacct->address,		sql_getvalue(sqr, 0, 10), sizeof(rec.mailacct->address)-1);
		strncpy(rec.mailacct->hosttype,		sql_getvalue(sqr, 0, 11), sizeof(rec.mailacct->hosttype)-1);
		strncpy(rec.mailacct->pophost,		sql_getvalue(sqr, 0, 12), sizeof(rec.mailacct->pophost)-1);
		rec.mailacct->popport=atoi(sql_getvalue(sqr, 0, 13));
		strncpy(rec.mailacct->smtphost,		sql_getvalue(sqr, 0, 14), sizeof(rec.mailacct->smtphost)-1);
		rec.mailacct->smtpport=atoi(sql_getvalue(sqr, 0, 15));
		strncpy(rec.mailacct->popusername,	sql_getvalue(sqr, 0, 16), sizeof(rec.mailacct->popusername)-1);
		strncpy(rec.mailacct->poppassword,	DecodeBase64string(sid, sql_getvalue(sqr, 0, 17)), sizeof(rec.mailacct->poppassword)-1);
		rec.mailacct->lastcount=atoi(sql_getvalue(sqr, 0, 18));
		rec.mailacct->notify=atoi(sql_getvalue(sqr, 0, 19));
		rec.mailacct->remove=atoi(sql_getvalue(sqr, 0, 20));
		rec.mailacct->lastcheck=time_sql2unix(sql_getvalue(sqr, 0, 21));
		strncpy(rec.mailacct->signature,	sql_getvalue(sqr, 0, 22), sizeof(rec.mailacct->signature)-1);
		break;
	case DB_NOTES:
		strncpy(rec.note->tablename,		sql_getvalue(sqr, 0, 7), sizeof(rec.note->tablename)-1);
		rec.note->tableindex=atoi(sql_getvalue(sqr, 0, 8));
		strncpy(rec.note->notetitle,		sql_getvalue(sqr, 0, 9), sizeof(rec.note->notetitle)-1);
		strncpy(rec.note->notetext,		sql_getvalue(sqr, 0, 10), sizeof(rec.note->notetext)-1);
		if (strlen(rec.note->notetitle)==0) strncpy(rec.note->notetitle, "unnamed", sizeof(rec.note->notetitle)-1);
		break;
	case DB_ORDERS:
		rec.order->contactid=atoi(sql_getvalue(sqr, 0, 7));
		rec.order->userid=atoi(sql_getvalue(sqr, 0, 8));
		rec.order->orderdate=time_sql2unix(sql_getvalue(sqr, 0, 9));
		strncpy(rec.order->ordertype,		sql_getvalue(sqr, 0, 10), sizeof(rec.order->ordertype)-1);
		strncpy(rec.order->paymentmethod,	sql_getvalue(sqr, 0, 11), sizeof(rec.order->paymentmethod)-1);
		rec.order->paymentdue=(float)atof(sql_getvalue(sqr, 0, 12));
		rec.order->paymentreceived=(float)atof(sql_getvalue(sqr, 0, 13));
		strncpy(rec.order->details,		sql_getvalue(sqr, 0, 14), sizeof(rec.order->details)-1);
		break;
	case DB_ORDERITEMS:
		rec.orderitem->orderid=atoi(sql_getvalue(sqr, 0, 7));
		rec.orderitem->productid=atoi(sql_getvalue(sqr, 0, 8));
		rec.orderitem->quantity=(float)atof(sql_getvalue(sqr, 0, 9));
		rec.orderitem->discount=(float)atof(sql_getvalue(sqr, 0, 10));
		rec.orderitem->unitprice=(float)atof(sql_getvalue(sqr, 0, 11));
		rec.orderitem->internalcost=(float)atof(sql_getvalue(sqr, 0, 12));
		rec.orderitem->tax1=(float)atof(sql_getvalue(sqr, 0, 13));
		rec.orderitem->tax2=(float)atof(sql_getvalue(sqr, 0, 14));
		break;
	case DB_PRODUCTS:
		strncpy(rec.product->productname,	sql_getvalue(sqr, 0, 7), sizeof(rec.product->productname)-1);
		strncpy(rec.product->category,		sql_getvalue(sqr, 0, 8), sizeof(rec.product->category)-1);
		rec.product->discount=(float)atof(sql_getvalue(sqr, 0, 9));
		rec.product->unitprice=(float)atof(sql_getvalue(sqr, 0, 10));
		rec.product->internalcost=(float)atof(sql_getvalue(sqr, 0, 11));
		rec.product->tax1=(float)atof(sql_getvalue(sqr, 0, 12));
		rec.product->tax2=(float)atof(sql_getvalue(sqr, 0, 13));
		strncpy(rec.product->details,		sql_getvalue(sqr, 0, 14), sizeof(rec.product->details)-1);
		break;
	case DB_QUERIES:
		strncpy(rec.query->queryname,		sql_getvalue(sqr, 0, 7), sizeof(rec.query->queryname)-1);
		strncpy(rec.query->query,		sql_getvalue(sqr, 0, 8), sizeof(rec.query->query)-1);
		break;
	case DB_TASKS:
		rec.task->assignedby=atoi(sql_getvalue(sqr, 0, 7));
		rec.task->assignedto=atoi(sql_getvalue(sqr, 0, 8));
		strncpy(rec.task->taskname,		sql_getvalue(sqr, 0, 9), sizeof(rec.task->taskname)-1);
		rec.task->duedate=time_sql2unix(sql_getvalue(sqr, 0, 10));
		rec.task->priority=atoi(sql_getvalue(sqr, 0, 11));
		rec.task->reminder=atoi(sql_getvalue(sqr, 0, 12));
		rec.task->status=atoi(sql_getvalue(sqr, 0, 13));
		strncpy(rec.task->details,		sql_getvalue(sqr, 0, 14), sizeof(rec.task->details)-1);
		break;
	case DB_PROFILE:
	case DB_USERS:
		strncpy(rec.user->loginip,          sql_getvalue(sqr, 0, 7), sizeof(rec.user->loginip)-1);
		rec.user->logintime=time_sql2unix(sql_getvalue(sqr, 0, 8));
		strncpy(rec.user->logintoken,       sql_getvalue(sqr, 0, 9), sizeof(rec.user->logintoken)-1);
		strncpy(rec.user->username,         sql_getvalue(sqr, 0, 10), sizeof(rec.user->username)-1);
		strncpy(rec.user->password,         sql_getvalue(sqr, 0, 11), sizeof(rec.user->password)-1);
		rec.user->groupid=atoi(sql_getvalue(sqr, 0, 12));
		rec.user->enabled=atoi(sql_getvalue(sqr, 0, 13));
		rec.user->authadmin=atoi(sql_getvalue(sqr, 0, 14));
		rec.user->authbookmarks=atoi(sql_getvalue(sqr, 0, 15));
		rec.user->authcalendar=atoi(sql_getvalue(sqr, 0, 16));
		rec.user->authcalls=atoi(sql_getvalue(sqr, 0, 17));
		rec.user->authcontacts=atoi(sql_getvalue(sqr, 0, 18));
		rec.user->authfiles=atoi(sql_getvalue(sqr, 0, 19));
		rec.user->authforums=atoi(sql_getvalue(sqr, 0, 20));
		rec.user->authmessages=atoi(sql_getvalue(sqr, 0, 21));
		rec.user->authorders=atoi(sql_getvalue(sqr, 0, 22));
		rec.user->authprofile=atoi(sql_getvalue(sqr, 0, 23));
		rec.user->authquery=atoi(sql_getvalue(sqr, 0, 24));
		rec.user->authwebmail=atoi(sql_getvalue(sqr, 0, 25));
		rec.user->prefdaystart=atoi(sql_getvalue(sqr, 0, 26));
		rec.user->prefdaylength=atoi(sql_getvalue(sqr, 0, 27));
		rec.user->prefmailcurrent=atoi(sql_getvalue(sqr, 0, 28));
		rec.user->prefmaildefault=atoi(sql_getvalue(sqr, 0, 29));
		rec.user->prefmaxlist=atoi(sql_getvalue(sqr, 0, 30));
		rec.user->prefmenustyle=atoi(sql_getvalue(sqr, 0, 31));
		rec.user->preftimezone=atoi(sql_getvalue(sqr, 0, 32));
		rec.user->prefgeozone=atoi(sql_getvalue(sqr, 0, 33));
		strncpy(rec.user->availability,     sql_getvalue(sqr, 0, 34), sizeof(rec.user->availability)-1);
		strncpy(rec.user->surname,          sql_getvalue(sqr, 0, 35), sizeof(rec.user->surname)-1);
		strncpy(rec.user->givenname,        sql_getvalue(sqr, 0, 36), sizeof(rec.user->givenname)-1);
		strncpy(rec.user->jobtitle,         sql_getvalue(sqr, 0, 37), sizeof(rec.user->jobtitle)-1);
		strncpy(rec.user->division,         sql_getvalue(sqr, 0, 38), sizeof(rec.user->division)-1);
		strncpy(rec.user->supervisor,       sql_getvalue(sqr, 0, 39), sizeof(rec.user->supervisor)-1);
		strncpy(rec.user->address,          sql_getvalue(sqr, 0, 40), sizeof(rec.user->address)-1);
		strncpy(rec.user->locality,         sql_getvalue(sqr, 0, 41), sizeof(rec.user->locality)-1);
		strncpy(rec.user->region,           sql_getvalue(sqr, 0, 42), sizeof(rec.user->region)-1);
		strncpy(rec.user->country,          sql_getvalue(sqr, 0, 43), sizeof(rec.user->country)-1);
		strncpy(rec.user->postalcode,       sql_getvalue(sqr, 0, 44), sizeof(rec.user->postalcode)-1);
		strncpy(rec.user->homenumber,       sql_getvalue(sqr, 0, 45), sizeof(rec.user->homenumber)-1);
		strncpy(rec.user->worknumber,       sql_getvalue(sqr, 0, 46), sizeof(rec.user->worknumber)-1);
		strncpy(rec.user->faxnumber,        sql_getvalue(sqr, 0, 47), sizeof(rec.user->faxnumber)-1);
		strncpy(rec.user->cellnumber,       sql_getvalue(sqr, 0, 48), sizeof(rec.user->cellnumber)-1);
		strncpy(rec.user->pagernumber,      sql_getvalue(sqr, 0, 49), sizeof(rec.user->pagernumber)-1);
		strncpy(rec.user->email,            sql_getvalue(sqr, 0, 50), sizeof(rec.user->email)-1);
		strncpy(rec.user->birthdate,        sql_getvalue(sqr, 0, 51), 10);
		strncpy(rec.user->hiredate,         sql_getvalue(sqr, 0, 52), 10);
		strncpy(rec.user->sin,              sql_getvalue(sqr, 0, 53), sizeof(rec.user->sin)-1);
		strncpy(rec.user->isactive,         sql_getvalue(sqr, 0, 54), sizeof(rec.user->isactive)-1);
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
		strncpy(rec.zone->zonename,       sql_getvalue(sqr, 0, 7), sizeof(rec.zone->zonename)-1);
		break;
	default:
		break;
	}
	sql_freeresult(sqr);
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
			if ((sqr=sql_query(sid, "SELECT max(bookmarkid) FROM gw_bookmarks"))<0) return -1;
			rec.bookmark->bookmarkid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.bookmark->bookmarkid<1) rec.bookmark->bookmarkid=1;
			strcpy(query, "INSERT INTO gw_bookmarks (bookmarkid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, folderid, bookmarkname, bookmarkurl) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.bookmark->bookmarkid, curdate, curdate, rec.bookmark->obj_uid, rec.bookmark->obj_gid, rec.bookmark->obj_gperm, rec.bookmark->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.bookmark->folderid);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.bookmark->bookmarkname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.bookmark->bookmarkurl));
			if (sql_update(sid, query)<0) return -1;
			return rec.bookmark->bookmarkid;
			break;
		case DB_CALLS:
			if ((sqr=sql_query(sid, "SELECT max(callid) FROM gw_calls"))<0) return -1;
			rec.call->callid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.call->callid<1) rec.call->callid=1;
			strcpy(query, "INSERT INTO gw_calls (callid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, assignedby, assignedto, callname, callstart, callfinish, contactid, action, status, details) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.call->callid, curdate, curdate, rec.call->obj_uid, rec.call->obj_gid, rec.call->obj_gperm, rec.call->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.call->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.call->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.call->callname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, rec.call->callstart));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, rec.call->callfinish));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.call->contactid);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.call->action);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.call->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.call->details));
			if (sql_update(sid, query)<0) return -1;
			return rec.call->callid;
			break;
		case DB_CONTACTS:
			if ((sqr=sql_query(sid, "SELECT max(contactid) FROM gw_contacts"))<0) return -1;
			rec.contact->contactid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.contact->contactid<1) rec.contact->contactid=1;
			strcpy(query, "INSERT INTO gw_contacts (contactid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, loginip, logintime, logintoken, username, password, enabled, geozone, timezone, surname, givenname, salutation, contacttype, referredby, altcontact, prefbilling, email, homenumber, worknumber, faxnumber, mobilenumber, jobtitle, organization, homeaddress, homelocality, homeregion, homecountry, homepostalcode, workaddress, worklocality, workregion, workcountry, workpostalcode) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.contact->contactid, curdate, curdate, rec.contact->obj_uid, rec.contact->obj_gid, rec.contact->obj_gperm, rec.contact->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->username));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->password));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.contact->enabled);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.contact->geozone);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.contact->timezone);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->surname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->givenname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->salutation));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->contacttype));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->referredby));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->altcontact));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->prefbilling));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->email));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homenumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->worknumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->faxnumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->mobilenumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->jobtitle));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->organization));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homeaddress));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homelocality));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homeregion));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homecountry));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->homepostalcode));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->workaddress));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->worklocality));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->workregion));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.contact->workcountry));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.contact->workpostalcode));
			if (sql_update(sid, query)<0) return -1;
			return rec.contact->contactid;
			break;
		case DB_EVENTS:
			if ((sqr=sql_query(sid, "SELECT max(eventid) FROM gw_events"))<0) return -1;
			rec.event->eventid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.event->eventid<1) rec.event->eventid=1;
			strcpy(query, "INSERT INTO gw_events (eventid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, eventname, assignedby, assignedto, eventtype, contactid, priority, reminder, eventstart, eventfinish, busy, status, closingstatus, details) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.event->eventid, curdate, curdate, rec.event->obj_uid, rec.event->obj_gid, rec.event->obj_gperm, rec.event->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.event->eventname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->eventtype);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->contactid);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->priority);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->reminder);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, rec.event->eventstart));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, rec.event->eventfinish));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->busy);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.event->closingstatus);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.event->details));
			if (sql_update(sid, query)<0) return -1;
			return rec.event->eventid;
			break;
		case DB_FORUMGROUPS:
			if ((sqr=sql_query(sid, "SELECT max(forumgroupid) FROM gw_forumgroups"))<0) return -1;
			rec.forumgroup->forumgroupid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.forumgroup->forumgroupid<1) rec.forumgroup->forumgroupid=1;
			strcpy(query, "INSERT INTO gw_forumgroups (forumgroupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, title, description) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.forumgroup->forumgroupid, curdate, curdate, rec.forumgroup->obj_uid, rec.forumgroup->obj_gid, rec.forumgroup->obj_gperm, rec.forumgroup->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.forumgroup->title));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.forumgroup->description));
			if (sql_update(sid, query)<0) return -1;
			return rec.forumgroup->forumgroupid;
			break;
		case DB_GROUPS:
			break;
		case DB_NOTES:
			if ((sqr=sql_query(sid, "SELECT max(noteid) FROM gw_notes"))<0) return -1;
			rec.query->queryid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.query->queryid<1) rec.query->queryid=1;
			strcpy(query, "INSERT INTO gw_notes (noteid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, tablename, tableindex, notetitle, notetext) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.note->noteid, curdate, curdate, rec.note->obj_uid, rec.note->obj_gid, rec.note->obj_gperm, rec.note->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.note->tablename));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.note->tableindex);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.note->notetitle));
			strncatf(query, sizeof(query)-strlen(query)-1, "'");
			strncat(query, rec.note->notetext, sizeof(query)-strlen(query)-3);
			strncat(query, "')", sizeof(query)-strlen(query)-1);
			if (sql_update(sid, query)<0) return -1;
			return rec.note->noteid;
			break;
		case DB_QUERIES:
			if ((sqr=sql_query(sid, "SELECT max(queryid) FROM gw_queries"))<0) return -1;
			rec.query->queryid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.query->queryid<1) rec.query->queryid=1;
			strcpy(query, "INSERT INTO gw_queries (queryid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, queryname, query) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.query->queryid, curdate, curdate, rec.query->obj_uid, rec.query->obj_gid, rec.query->obj_gperm, rec.query->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.query->queryname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.query->query));
			if (sql_update(sid, query)<0) return -1;
			return rec.query->queryid;
			break;
		case DB_TASKS:
			if ((sqr=sql_query(sid, "SELECT max(taskid) FROM gw_tasks"))<0) return -1;
			rec.task->taskid=atoi(sql_getvalue(sqr, 0, 0))+1;
			sql_freeresult(sqr);
			if (rec.task->taskid<1) rec.task->taskid=1;
			strcpy(query, "INSERT INTO gw_tasks (taskid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, assignedby, assignedto, taskname, status, priority, reminder, duedate, details) values (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", rec.task->taskid, curdate, curdate, rec.task->obj_uid, rec.task->obj_gid, rec.task->obj_gperm, rec.task->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.task->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.task->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, rec.task->taskname));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.task->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.task->priority);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", rec.task->reminder);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", time_unix2sql(sid, rec.task->duedate));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, rec.task->details));
			if (sql_update(sid, query)<0) return -1;
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
			strncatf(query, sizeof(query)-strlen(query)-1, "folderid = '%d', ", rec.bookmark->folderid);
			strncatf(query, sizeof(query)-strlen(query)-1, "bookmarkname = '%s', ", str2sql(sid, rec.bookmark->bookmarkname));
			strncatf(query, sizeof(query)-strlen(query)-1, "bookmarkurl = '%s'", str2sql(sid, rec.bookmark->bookmarkurl));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE bookmarkid = %d", rec.bookmark->bookmarkid);
			if (sql_update(sid, query)<0) return -1;
			return rec.bookmark->bookmarkid;
			break;
		case DB_CALLS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_calls SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.call->obj_uid, rec.call->obj_gid, rec.call->obj_gperm, rec.call->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", rec.call->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", rec.call->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "callname = '%s', ", str2sql(sid, rec.call->callname));
			strncatf(query, sizeof(query)-strlen(query)-1, "callstart = '%s', ", time_unix2sql(sid, rec.call->callstart));
			strncatf(query, sizeof(query)-strlen(query)-1, "callfinish = '%s', ", time_unix2sql(sid, rec.call->callfinish));
			strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", rec.call->contactid);
			strncatf(query, sizeof(query)-strlen(query)-1, "action = '%d', ", rec.call->action);
			strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", rec.call->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, rec.call->details));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE callid = %d", rec.call->callid);
			if (sql_update(sid, query)<0) return -1;
			return rec.call->callid;
			break;
		case DB_CONTACTS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_contacts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.contact->obj_uid, rec.contact->obj_gid, rec.contact->obj_gperm, rec.contact->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "username = '%s', ", str2sql(sid, rec.contact->username));
			strncatf(query, sizeof(query)-strlen(query)-1, "password = '%s', ", str2sql(sid, rec.contact->password));
			strncatf(query, sizeof(query)-strlen(query)-1, "enabled = '%d', ", rec.contact->enabled);
			strncatf(query, sizeof(query)-strlen(query)-1, "geozone = '%d', ", rec.contact->geozone);
			strncatf(query, sizeof(query)-strlen(query)-1, "timezone = '%d', ", rec.contact->timezone);
			strncatf(query, sizeof(query)-strlen(query)-1, "surname = '%s', ", str2sql(sid, rec.contact->surname));
			strncatf(query, sizeof(query)-strlen(query)-1, "givenname = '%s', ", str2sql(sid, rec.contact->givenname));
			strncatf(query, sizeof(query)-strlen(query)-1, "salutation = '%s', ", str2sql(sid, rec.contact->salutation));
			strncatf(query, sizeof(query)-strlen(query)-1, "contacttype = '%s', ", str2sql(sid, rec.contact->contacttype));
			strncatf(query, sizeof(query)-strlen(query)-1, "referredby = '%s', ", str2sql(sid, rec.contact->referredby));
			strncatf(query, sizeof(query)-strlen(query)-1, "altcontact = '%s', ", str2sql(sid, rec.contact->altcontact));
			strncatf(query, sizeof(query)-strlen(query)-1, "prefbilling = '%s', ", str2sql(sid, rec.contact->prefbilling));
			strncatf(query, sizeof(query)-strlen(query)-1, "email = '%s', ", str2sql(sid, rec.contact->email));
			strncatf(query, sizeof(query)-strlen(query)-1, "homenumber = '%s', ", str2sql(sid, rec.contact->homenumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "worknumber = '%s', ", str2sql(sid, rec.contact->worknumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "faxnumber = '%s', ", str2sql(sid, rec.contact->faxnumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "mobilenumber = '%s', ", str2sql(sid, rec.contact->mobilenumber));
			strncatf(query, sizeof(query)-strlen(query)-1, "jobtitle = '%s', ", str2sql(sid, rec.contact->jobtitle));
			strncatf(query, sizeof(query)-strlen(query)-1, "organization = '%s', ", str2sql(sid, rec.contact->organization));
			strncatf(query, sizeof(query)-strlen(query)-1, "homeaddress = '%s', ", str2sql(sid, rec.contact->homeaddress));
			strncatf(query, sizeof(query)-strlen(query)-1, "homelocality = '%s', ", str2sql(sid, rec.contact->homelocality));
			strncatf(query, sizeof(query)-strlen(query)-1, "homeregion = '%s', ", str2sql(sid, rec.contact->homeregion));
			strncatf(query, sizeof(query)-strlen(query)-1, "homecountry = '%s', ", str2sql(sid, rec.contact->homecountry));
			strncatf(query, sizeof(query)-strlen(query)-1, "homepostalcode = '%s', ", str2sql(sid, rec.contact->homepostalcode));
			strncatf(query, sizeof(query)-strlen(query)-1, "workaddress = '%s', ", str2sql(sid, rec.contact->workaddress));
			strncatf(query, sizeof(query)-strlen(query)-1, "worklocality = '%s', ", str2sql(sid, rec.contact->worklocality));
			strncatf(query, sizeof(query)-strlen(query)-1, "workregion = '%s', ", str2sql(sid, rec.contact->workregion));
			strncatf(query, sizeof(query)-strlen(query)-1, "workcountry = '%s', ", str2sql(sid, rec.contact->workcountry));
			strncatf(query, sizeof(query)-strlen(query)-1, "workpostalcode = '%s'", str2sql(sid, rec.contact->workpostalcode));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE contactid = %d", rec.contact->contactid);
			if (sql_update(sid, query)<0) return -1;
			return rec.contact->contactid;
			break;
		case DB_EVENTS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_events SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.event->obj_uid, rec.event->obj_gid, rec.event->obj_gperm, rec.event->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "eventname = '%s', ", str2sql(sid, rec.event->eventname));
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", rec.event->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", rec.event->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "eventtype = '%d', ", rec.event->eventtype);
			strncatf(query, sizeof(query)-strlen(query)-1, "contactid = '%d', ", rec.event->contactid);
			strncatf(query, sizeof(query)-strlen(query)-1, "priority = '%d', ", rec.event->priority);
			strncatf(query, sizeof(query)-strlen(query)-1, "reminder = '%d', ", rec.event->reminder);
			strncatf(query, sizeof(query)-strlen(query)-1, "eventstart = '%s', ", time_unix2sql(sid, rec.event->eventstart));
			strncatf(query, sizeof(query)-strlen(query)-1, "eventfinish = '%s', ", time_unix2sql(sid, rec.event->eventfinish));
			strncatf(query, sizeof(query)-strlen(query)-1, "busy = '%d', ", rec.event->busy);
			strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", rec.event->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "closingstatus = '%d', ", rec.event->closingstatus);
			strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, rec.event->details));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE eventid = %d", rec.event->eventid);
			if (sql_update(sid, query)<0) return -1;
			return rec.event->eventid;
			break;
		case DB_FORUMGROUPS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_forumgroups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.forumgroup->obj_uid, rec.forumgroup->obj_gid, rec.forumgroup->obj_gperm, rec.forumgroup->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "title = '%s', ", str2sql(sid, rec.forumgroup->title));
			strncatf(query, sizeof(query)-strlen(query)-1, "description = '%s'", str2sql(sid, rec.forumgroup->description));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE forumgroupid = %d", rec.forumgroup->forumgroupid);
			if (sql_update(sid, query)<0) return -1;
			return rec.forumgroup->forumgroupid;
			break;
		case DB_GROUPS:
			break;
		case DB_NOTES:
			snprintf(query, sizeof(query)-1, "UPDATE gw_notes SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.note->obj_uid, rec.note->obj_gid, rec.note->obj_gperm, rec.note->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "tablename = '%s', ", str2sql(sid, rec.note->tablename));
			strncatf(query, sizeof(query)-strlen(query)-1, "tableindex = '%d', ", rec.note->tableindex);
			strncatf(query, sizeof(query)-strlen(query)-1, "notetitle = '%s', ", str2sql(sid, rec.note->notetitle));
			strncatf(query, sizeof(query)-strlen(query)-1, "notetext = '");
			strncat(query, rec.note->notetext, sizeof(query)-strlen(query)-3);
			strncat(query, "'", sizeof(query)-strlen(query)-27);
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE noteid = %d", rec.note->noteid);
			if (sql_update(sid, query)<0) return -1;
			return rec.note->noteid;
			break;
		case DB_QUERIES:
			snprintf(query, sizeof(query)-1, "UPDATE gw_queries SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.query->obj_uid, rec.query->obj_gid, rec.query->obj_gperm, rec.query->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "queryname = '%s', ", str2sql(sid, rec.query->queryname));
			strncatf(query, sizeof(query)-strlen(query)-1, "query = '%s'", str2sql(sid, rec.query->query));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE queryid = %d", rec.query->queryid);
			if (sql_update(sid, query)<0) return -1;
			return rec.query->queryid;
			break;
		case DB_TASKS:
			snprintf(query, sizeof(query)-1, "UPDATE gw_tasks SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, rec.task->obj_uid, rec.task->obj_gid, rec.task->obj_gperm, rec.task->obj_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedby = '%d', ", rec.task->assignedby);
			strncatf(query, sizeof(query)-strlen(query)-1, "assignedto = '%d', ", rec.task->assignedto);
			strncatf(query, sizeof(query)-strlen(query)-1, "taskname = '%s', ", str2sql(sid, rec.task->taskname));
			strncatf(query, sizeof(query)-strlen(query)-1, "status = '%d', ", rec.task->status);
			strncatf(query, sizeof(query)-strlen(query)-1, "priority = '%d', ", rec.task->priority);
			strncatf(query, sizeof(query)-strlen(query)-1, "reminder = '%d', ", rec.task->reminder);
			strncatf(query, sizeof(query)-strlen(query)-1, "duedate = '%s', ", time_unix2sql(sid, rec.task->duedate));
			strncatf(query, sizeof(query)-strlen(query)-1, "details = '%s'", str2sql(sid, rec.task->details));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE taskid = %d", rec.task->taskid);
			if (sql_update(sid, query)<0) return -1;
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
