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
#include "mod_orders.h"

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

int db_read(CONN *sid, short int perm, short int table, int index, void *record)
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
	case DB_ORDERS:
		memset(rec.order, 0, sizeof(REC_ORDER));
		authlevel=auth_priv(sid, "orders");
		break;
	case DB_ORDERITEMS:
		memset(rec.orderitem, 0, sizeof(REC_ORDERITEM));
		authlevel=auth_priv(sid, "orders");
		break;
	case DB_PRODUCTS:
		memset(rec.product, 0, sizeof(REC_PRODUCT));
		authlevel=auth_priv(sid, "orders");
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
			rec.product->tax1=proc->info.tax1percent;
			rec.product->tax2=proc->info.tax2percent;
			break;
		default:
			return -1;
			break;
		}
		return 0;
	}
	switch (table) {
	case DB_ORDERS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_orders where orderid = %d", index))<0) return -1;
		break;
	case DB_ORDERITEMS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_orderitems where orderitemid = %d", index))<0) return -1;
		break;
	case DB_PRODUCTS:
		if ((sqr=sql_queryf(sid, "SELECT * FROM gw_products where productid = %d", index))<0) return -1;
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
	rec.head->obj_did   = atoi(sql_getvalue(sqr, 0, 5));
	rec.head->obj_gperm = atoi(sql_getvalue(sqr, 0, 6));
	rec.head->obj_operm = atoi(sql_getvalue(sqr, 0, 7));
	switch (table) {
	case DB_ORDERS:
		rec.order->contactid=atoi(sql_getvalue(sqr, 0, 8));
		rec.order->userid=atoi(sql_getvalue(sqr, 0, 9));
		rec.order->orderdate=time_sql2unix(sql_getvalue(sqr, 0, 10));
		strncpy(rec.order->ordertype,		sql_getvalue(sqr, 0, 11), sizeof(rec.order->ordertype)-1);
		strncpy(rec.order->paymentmethod,	sql_getvalue(sqr, 0, 12), sizeof(rec.order->paymentmethod)-1);
		rec.order->paymentdue=(float)atof(sql_getvalue(sqr, 0, 13));
		rec.order->paymentreceived=(float)atof(sql_getvalue(sqr, 0, 14));
		strncpy(rec.order->details,		sql_getvalue(sqr, 0, 15), sizeof(rec.order->details)-1);
		break;
	case DB_ORDERITEMS:
		rec.orderitem->orderid=atoi(sql_getvalue(sqr, 0, 8));
		rec.orderitem->productid=atoi(sql_getvalue(sqr, 0, 9));
		rec.orderitem->quantity=(float)atof(sql_getvalue(sqr, 0, 10));
		rec.orderitem->discount=(float)atof(sql_getvalue(sqr, 0, 11));
		rec.orderitem->unitprice=(float)atof(sql_getvalue(sqr, 0, 12));
		rec.orderitem->internalcost=(float)atof(sql_getvalue(sqr, 0, 13));
		rec.orderitem->tax1=(float)atof(sql_getvalue(sqr, 0, 14));
		rec.orderitem->tax2=(float)atof(sql_getvalue(sqr, 0, 15));
		break;
	case DB_PRODUCTS:
		strncpy(rec.product->productname,	sql_getvalue(sqr, 0, 8), sizeof(rec.product->productname)-1);
		strncpy(rec.product->category,		sql_getvalue(sqr, 0, 9), sizeof(rec.product->category)-1);
		rec.product->discount=(float)atof(sql_getvalue(sqr, 0, 10));
		rec.product->unitprice=(float)atof(sql_getvalue(sqr, 0, 11));
		rec.product->internalcost=(float)atof(sql_getvalue(sqr, 0, 12));
		rec.product->tax1=(float)atof(sql_getvalue(sqr, 0, 13));
		rec.product->tax2=(float)atof(sql_getvalue(sqr, 0, 14));
		strncpy(rec.product->details,		sql_getvalue(sqr, 0, 15), sizeof(rec.product->details)-1);
		break;
	default:
		break;
	}
	sql_freeresult(sqr);
	return 0;
}
