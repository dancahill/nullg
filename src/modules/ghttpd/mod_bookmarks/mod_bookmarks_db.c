/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_bookmarks.h"

int dbread_bookmark(CONN *sid, short int perm, int index, REC_BOOKMARK *bookmark)
{
	int authlevel;
	SQLRES sqr;

	memset(bookmark, 0, sizeof(REC_BOOKMARK));
	authlevel=auth_priv(sid, "bookmarks");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		bookmark->obj_uid=sid->dat->user_uid;
		bookmark->obj_gid=sid->dat->user_gid;
		bookmark->obj_did=sid->dat->user_did;
		bookmark->obj_gperm=1;
		bookmark->obj_operm=1;
		snprintf(bookmark->bookmarkurl, sizeof(bookmark->bookmarkurl)-1, "http://");
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_bookmarks where bookmarkid = %d AND obj_did = %d", index, sid->dat->user_did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_bookmarks where bookmarkid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	bookmark->bookmarkid = atoi(sql_getvalue(&sqr, 0, 0));
	bookmark->obj_ctime  = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	bookmark->obj_mtime  = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	bookmark->obj_uid    = atoi(sql_getvalue(&sqr, 0, 3));
	bookmark->obj_gid    = atoi(sql_getvalue(&sqr, 0, 4));
	bookmark->obj_did    = atoi(sql_getvalue(&sqr, 0, 5));
	bookmark->obj_gperm  = atoi(sql_getvalue(&sqr, 0, 6));
	bookmark->obj_operm  = atoi(sql_getvalue(&sqr, 0, 7));
	bookmark->folderid=atoi(sql_getvalue(&sqr, 0, 8));
	strncpy(bookmark->bookmarkname,	sql_getvalue(&sqr, 0, 9), sizeof(bookmark->bookmarkname)-1);
	strncpy(bookmark->bookmarkurl,	sql_getvalue(&sqr, 0, 10), sizeof(bookmark->bookmarkurl)-1);
	if (strlen(bookmark->bookmarkname)==0) strncpy(bookmark->bookmarkname, "unnamed", sizeof(bookmark->bookmarkname)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbread_bookmarkfolder(CONN *sid, short int perm, int index, REC_BOOKMARKFOLDER *bookmarkfolder)
{
	int authlevel;
	SQLRES sqr;

	memset(bookmarkfolder, 0, sizeof(REC_BOOKMARKFOLDER));
	authlevel=auth_priv(sid, "bookmarks");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		bookmarkfolder->obj_uid=sid->dat->user_uid;
		bookmarkfolder->obj_gid=sid->dat->user_gid;
		bookmarkfolder->obj_did=sid->dat->user_did;
		bookmarkfolder->obj_gperm=1;
		bookmarkfolder->obj_operm=1;
		snprintf(bookmarkfolder->foldername, sizeof(bookmarkfolder->foldername)-1, "New Folder");
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_bookmarks_folders where folderid = %d AND obj_did = %d", index, sid->dat->user_did)<0) return -1;
	} else {
		if (sql_queryf(&sqr, "SELECT * FROM gw_bookmarks_folders where folderid = %d and (obj_uid = %d or (obj_gid = %d and obj_gperm>=%d) or obj_operm>=%d) AND obj_did = %d", index, sid->dat->user_uid, sid->dat->user_gid, perm, perm, sid->dat->user_did)<0) return -1;
	}
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	bookmarkfolder->folderid   = atoi(sql_getvalue(&sqr, 0, 0));
	bookmarkfolder->obj_ctime  = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	bookmarkfolder->obj_mtime  = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	bookmarkfolder->obj_uid    = atoi(sql_getvalue(&sqr, 0, 3));
	bookmarkfolder->obj_gid    = atoi(sql_getvalue(&sqr, 0, 4));
	bookmarkfolder->obj_did    = atoi(sql_getvalue(&sqr, 0, 5));
	bookmarkfolder->obj_gperm  = atoi(sql_getvalue(&sqr, 0, 6));
	bookmarkfolder->obj_operm  = atoi(sql_getvalue(&sqr, 0, 7));
	bookmarkfolder->parentid=atoi(sql_getvalue(&sqr, 0, 8));
	strncpy(bookmarkfolder->foldername,	sql_getvalue(&sqr, 0, 9), sizeof(bookmarkfolder->foldername)-1);
	if (strlen(bookmarkfolder->foldername)==0) strncpy(bookmarkfolder->foldername, "unnamed", sizeof(bookmarkfolder->foldername)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbwrite_bookmark(CONN *sid, int index, REC_BOOKMARK *bookmark)
{
	char curdate[32];
	char query[12288];
	int authlevel;
	SQLRES sqr;

	authlevel=auth_priv(sid, "bookmarks");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	memset(query, 0, sizeof(query));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(bookmarkid) FROM gw_bookmarks")<0) return -1;
		bookmark->bookmarkid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (bookmark->bookmarkid<1) bookmark->bookmarkid=1;
		strcpy(query, "INSERT INTO gw_bookmarks (bookmarkid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, folderid, bookmarkname, bookmarkurl) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", bookmark->bookmarkid, curdate, curdate, bookmark->obj_uid, bookmark->obj_gid, bookmark->obj_did, bookmark->obj_gperm, bookmark->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", bookmark->folderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmark->bookmarkname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmark->bookmarkurl));
		if (sql_update(query)<0) return -1;
		return bookmark->bookmarkid;
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_bookmarks SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, bookmark->obj_uid, bookmark->obj_gid, bookmark->obj_gperm, bookmark->obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "folderid = '%d', ", bookmark->folderid);
		strncatf(query, sizeof(query)-strlen(query)-1, "bookmarkname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmark->bookmarkname));
		strncatf(query, sizeof(query)-strlen(query)-1, "bookmarkurl = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmark->bookmarkurl));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE bookmarkid = %d", bookmark->bookmarkid);
		if (sql_update(query)<0) return -1;
		return bookmark->bookmarkid;
	}
	return -1;
}
