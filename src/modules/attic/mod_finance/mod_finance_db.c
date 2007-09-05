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
#include "mod_finance.h"
/*
union rec_u {
	REC_HEADER          *head;
	REC_BOOKMARK        *bookmark;
	REC_BOOKMARKFOLDER  *bookmarkfolder;
	REC_CALL            *call;
	REC_CONTACT         *contact;
	REC_EVENT           *event;
	REC_FILE            *file;
	REC_FIN_INVENTORY   *inventory;
	REC_FIN_INVOICE     *invoice;
	REC_FIN_INVOICEITEM *invoiceitem;
	REC_FORUMGROUP      *forumgroup;
	REC_GROUP           *group;
	REC_MAILACCT        *mailacct;
	REC_NOTE            *note;
	REC_QUERY           *query;
	REC_TASK            *task;
	REC_USER            *user;
	REC_ZONE            *zone;
};
*/
int db_read(CONN *sid, short int perm, short int table, int index, void *record)
{
/*
	union rec_u rec;
	int authlevel;
	SQLRES sqr;

	rec.head=record;
	rec.bookmark=record;
	rec.bookmarkfolder=record;
	rec.call=record;
	rec.contact=record;
	rec.event=record;
	rec.file=record;
	rec.inventory=record;
	rec.invoice=record;
	rec.invoiceitem=record;
	rec.forumgroup=record;
	rec.group=record;
	rec.mailacct=record;
	rec.note=record;
	rec.query=record;
	rec.task=record;
	rec.user=record;
	rec.zone=record;
	switch (table) {
	case DB_INVENTORY:
		memset(rec.inventory, 0, sizeof(REC_FIN_INVENTORY));
		authlevel=auth_priv(sid, "finance");
		break;
	case DB_INVOICE:
		memset(rec.invoice, 0, sizeof(REC_FIN_INVOICE));
		authlevel=auth_priv(sid, "finance");
		break;
	case DB_INVOICEITEMS:
		memset(rec.invoiceitem, 0, sizeof(REC_FIN_INVOICEITEM));
		authlevel=auth_priv(sid, "finance");
		break;
	default:
		return -1;
		break;
	}
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if ((perm>1)&&(table==DB_INVENTORY)&&(!(authlevel&A_ADMIN))) return -1;
	if (index==0) {
		rec.head->obj_uid=sid->dat->uid;
		rec.head->obj_gid=sid->dat->gid;
		rec.head->obj_did=sid->dat->did;
		rec.head->obj_gperm=1;
		rec.head->obj_operm=1;
		switch (table) {
		case DB_INVENTORY:
			rec.inventory->tax1=proc->info.tax1percent;
			rec.inventory->tax2=proc->info.tax2percent;
			break;
		case DB_INVOICE:
			rec.invoice->invoicedate=(time(NULL)/60)*60;
			rec.invoice->userid=sid->dat->uid;
			snprintf(rec.invoice->invoicetype, sizeof(rec.invoice->invoicetype)-1, "Sale");
			snprintf(rec.invoice->paymentmethod, sizeof(rec.invoice->paymentmethod)-1, "Cash");
			break;
		case DB_INVOICEITEMS:
			rec.invoiceitem->quantity=1;
			break;
		default:
			return -1;
			break;
		}
		return 0;
	}
	switch (table) {
	case DB_INVENTORY:
		if (sql_queryf(&sqr, "SELECT * FROM gw_finance_inventory where inventoryid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
		break;
	case DB_INVOICE:
		if (sql_queryf(&sqr, "SELECT * FROM gw_finance_invoices where invoiceid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
		break;
	case DB_INVOICEITEMS:
		if (sql_queryf(&sqr, "SELECT * FROM gw_finance_invoices_items where invoiceitemid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
		break;
	default:
		return -1;
		break;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	rec.head->recordid  = atoi(sql_getvalue(&sqr, 0, 0));
	rec.head->obj_ctime = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	rec.head->obj_mtime = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	rec.head->obj_uid   = atoi(sql_getvalue(&sqr, 0, 3));
	rec.head->obj_gid   = atoi(sql_getvalue(&sqr, 0, 4));
	rec.head->obj_did   = atoi(sql_getvalue(&sqr, 0, 5));
	rec.head->obj_gperm = atoi(sql_getvalue(&sqr, 0, 6));
	rec.head->obj_operm = atoi(sql_getvalue(&sqr, 0, 7));
	switch (table) {
	case DB_INVENTORY:
		strncpy(rec.inventory->itemname,	sql_getvalue(&sqr, 0, 8), sizeof(rec.inventory->itemname)-1);
		strncpy(rec.inventory->category,	sql_getvalue(&sqr, 0, 9), sizeof(rec.inventory->category)-1);
		rec.inventory->discount=(float)atof(sql_getvalue(&sqr, 0, 10));
		rec.inventory->unitprice=(float)atof(sql_getvalue(&sqr, 0, 11));
		rec.inventory->internalcost=(float)atof(sql_getvalue(&sqr, 0, 12));
		rec.inventory->tax1=(float)atof(sql_getvalue(&sqr, 0, 13));
		rec.inventory->tax2=(float)atof(sql_getvalue(&sqr, 0, 14));
		strncpy(rec.inventory->details,		sql_getvalue(&sqr, 0, 15), sizeof(rec.inventory->details)-1);
		break;
	case DB_INVOICE:
		rec.invoice->contactid=atoi(sql_getvalue(&sqr, 0, 8));
		rec.invoice->userid=atoi(sql_getvalue(&sqr, 0, 9));
		rec.invoice->invoicedate=time_sql2unix(sql_getvalue(&sqr, 0, 10));
		strncpy(rec.invoice->invoicetype,		sql_getvalue(&sqr, 0, 11), sizeof(rec.invoice->invoicetype)-1);
		strncpy(rec.invoice->paymentmethod,	sql_getvalue(&sqr, 0, 12), sizeof(rec.invoice->paymentmethod)-1);
		rec.invoice->paymentdue=(float)atof(sql_getvalue(&sqr, 0, 13));
		rec.invoice->paymentreceived=(float)atof(sql_getvalue(&sqr, 0, 14));
		rec.invoice->status=atoi(sql_getvalue(&sqr, 0, 15));
		strncpy(rec.invoice->details,		sql_getvalue(&sqr, 0, 16), sizeof(rec.invoice->details)-1);
		break;
	case DB_INVOICEITEMS:
		rec.invoiceitem->invoiceid=atoi(sql_getvalue(&sqr, 0, 8));
		rec.invoiceitem->inventoryid=atoi(sql_getvalue(&sqr, 0, 9));
		rec.invoiceitem->quantity=(float)atof(sql_getvalue(&sqr, 0, 10));
		rec.invoiceitem->discount=(float)atof(sql_getvalue(&sqr, 0, 11));
		rec.invoiceitem->unitprice=(float)atof(sql_getvalue(&sqr, 0, 12));
		rec.invoiceitem->internalcost=(float)atof(sql_getvalue(&sqr, 0, 13));
		rec.invoiceitem->tax1=(float)atof(sql_getvalue(&sqr, 0, 14));
		rec.invoiceitem->tax2=(float)atof(sql_getvalue(&sqr, 0, 15));
		break;
	default:
		break;
	}
	sql_freeresult(&sqr);
*/
	return 0;
}

int dbread_account(CONN *sid, short int perm, int index, obj_t **qobj)
{
/*	int authlevel;
	SQLRES sqr;

	memset(account, 0, sizeof(REC_FIN_ACCOUNT));
	authlevel=auth_priv(sid, "finance");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		account->obj_uid=sid->dat->uid;
		account->obj_gid=sid->dat->gid;
		account->obj_did=sid->dat->did;
		account->obj_gperm=1;
		account->obj_operm=1;
		return 0;
	}
//	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_finance_accounts where accountid = %d AND obj_did = %d", index, sid->dat->did)<0) return -1;
//	} else {
//		if (sql_queryf(&sqr, "SELECT * FROM gw_finance_accounts where accountid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->uid, sid->dat->gid, perm, perm, sid->dat->did)<0) return -1;
//	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	account->accountid = atoi(sql_getvalue(&sqr, 0, 0));
	account->obj_ctime  = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	account->obj_mtime  = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	account->obj_uid    = atoi(sql_getvalue(&sqr, 0, 3));
	account->obj_gid    = atoi(sql_getvalue(&sqr, 0, 4));
	account->obj_did    = atoi(sql_getvalue(&sqr, 0, 5));
	account->obj_gperm  = atoi(sql_getvalue(&sqr, 0, 6));
	account->obj_operm  = atoi(sql_getvalue(&sqr, 0, 7));
	strncpy(account->accounttype,	sql_getvalue(&sqr, 0, 8), sizeof(account->accounttype)-1);
	strncpy(account->accountname,	sql_getvalue(&sqr, 0, 9), sizeof(account->accountname)-1);
	if (strlen(account->accountname)==0) strncpy(account->accountname, "unnamed", sizeof(account->accountname)-1);
	sql_freeresult(&sqr);
*/
	return 0;
}

int dbwrite_account(CONN *sid, int index, obj_t **qobj)
{
/*
	char curdate[32];
	char query[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=auth_priv(sid, "finance");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(accountid) FROM gw_finance_accounts")<0) return -1;
		account->accountid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (account->accountid<1) account->accountid=1;
		strcpy(query, "INSERT INTO gw_finance_accounts (accountid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accounttype, accountname) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", account->accountid, curdate, curdate, account->obj_uid, account->obj_gid, account->obj_did, account->obj_gperm, account->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, account->accounttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, account->accountname));
		if (sql_update(query)<0) return -1;
		return account->accountid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_finance_accounts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, account->obj_uid, account->obj_gid, account->obj_gperm, account->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "accounttype = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, account->accounttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "accountname = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, account->accountname));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE accountid = %d", account->accountid);
		if (sql_update(query)<0) return -1;
		return account->accountid;
	}
*/
	return -1;
}
