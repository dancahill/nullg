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
#include "mod_weblog.h"

#define QUERYBUFSIZE 81920 /* 80k */

int dblist_weblog_comments(CONN *sid, SQLRES *sqr, int entryid)
{
	char query[256];
	int rc;

	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "SELECT * from gw_weblog_comments WHERE ");
	strncatf(query, sizeof(query)-strlen(query)-1, "entryid = %d AND ", entryid);
	strncatf(query, sizeof(query)-strlen(query)-1, "obj_did = %d ORDER BY obj_ctime ASC", sid->dat->user_did);
	if ((rc=sql_query(sqr, query))<0) {
		return EC_UNKNOWN;
	}
	sql_permprune(sid, sqr, NULL);
	return rc;
}

int dbread_weblog_comment(CONN *sid, short int perm, int index, int entryindex, REC_BLOG_COMMENT *comment)
{
	int authlevel;
	SQLRES sqr;

	memset(comment, 0, sizeof(REC_BLOG_COMMENT));
	authlevel=auth_priv(sid, "forums");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		comment->obj_uid=sid->dat->user_uid;
		comment->obj_gid=sid->dat->user_gid;
		comment->obj_did=sid->dat->user_did;
		comment->obj_gperm=1;
		comment->obj_operm=1;
		comment->entryid=entryindex;
		snprintf(comment->subject, sizeof(comment->subject)-1, "New Weblog Comment");
		return 0;
	}
	if (sql_queryf(&sqr, "SELECT * FROM gw_weblog_comments where commentid = %d AND entryid = %d AND obj_did = %d", index, entryindex, sid->dat->user_did)<0) return -1;
	sql_permprune(sid, &sqr, NULL);
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	comment->commentid   = atoi(sql_getvalue(&sqr, 0, 0));
	comment->obj_ctime   = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	comment->obj_mtime   = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	comment->obj_uid     = atoi(sql_getvalue(&sqr, 0, 3));
	comment->obj_gid     = atoi(sql_getvalue(&sqr, 0, 4));
	comment->obj_did     = atoi(sql_getvalue(&sqr, 0, 5));
	comment->obj_gperm   = atoi(sql_getvalue(&sqr, 0, 6));
	comment->obj_operm   = atoi(sql_getvalue(&sqr, 0, 7));
	comment->entryid     = atoi(sql_getvalue(&sqr, 0, 8));
	comment->referenceid = atoi(sql_getvalue(&sqr, 0, 9));
	strncpy(comment->subject, sql_getvalue(&sqr, 0, 10),  sizeof(comment->subject)-1);
	strncpy(comment->message, sql_getvalue(&sqr, 0, 11), sizeof(comment->message)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbwrite_weblog_comment(CONN *sid, int index, REC_BLOG_COMMENT *comment)
{
	char curdate[32];
	char *query=NULL;
	int authlevel;
	SQLRES sqr;
	int rc;

	authlevel=auth_priv(sid, "forums");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_queryf(&sqr, "SELECT max(commentid) FROM gw_weblog_comments WHERE entryid = %d AND obj_did = %d", comment->entryid, sid->dat->user_did)<0) return -1;
		comment->commentid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (comment->commentid<1) comment->commentid=1;
		query=calloc(QUERYBUFSIZE, sizeof(char));
		strcpy(query, "INSERT INTO gw_weblog_comments (commentid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, entryid, referenceid, subject, message) values (");
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", comment->commentid, curdate, curdate, comment->obj_uid, comment->obj_gid, comment->obj_did, comment->obj_gperm, comment->obj_operm);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%d', ", comment->entryid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%d', ", comment->referenceid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, comment->subject));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, comment->message));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "')");
		rc=sql_update(query);
		if (query!=NULL) { free(query); query=NULL; }
		if (rc<0) return -1;
		return comment->commentid;
	} else {
		query=calloc(QUERYBUFSIZE, sizeof(char));
		snprintf(query, QUERYBUFSIZE-1, "UPDATE gw_weblog_comments SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, comment->obj_uid, comment->obj_gid, comment->obj_gperm, comment->obj_operm);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "entryid = '%d', ", comment->entryid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "referenceid = '%d', ", comment->referenceid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "subject = '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, comment->subject));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', message = '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, comment->message));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "' WHERE commentid = %d AND entryid = %d", comment->commentid, comment->entryid);
		rc=sql_update(query);
		if (query!=NULL) { free(query); query=NULL; }
		if (rc<0) return -1;
		return comment->commentid;
	}
	return -1;
}

int dblist_weblog_entries(CONN *sid, SQLRES *sqr, int userid, int groupid)
{
	char query[256];
	int rc;

	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "SELECT * from gw_weblog_entries WHERE ");
	if (userid>0) {
		strncatf(query, sizeof(query)-strlen(query)-1, "obj_uid = %d AND ", userid);
	}
	if (groupid>0) {
		strncatf(query, sizeof(query)-strlen(query)-1, "entrygroupid = %d AND ", groupid);
	}
	strncatf(query, sizeof(query)-strlen(query)-1, "obj_did = %d ORDER BY obj_ctime DESC", sid->dat->user_did);
	if ((rc=sql_query(sqr, query))<0) {
		return EC_UNKNOWN;
	}
	sql_permprune(sid, sqr, NULL);
	return rc;
}

int dbread_weblog_entry(CONN *sid, short int perm, int index, REC_BLOG_ENTRY *entry)
{
	int authlevel;
	SQLRES sqr;

	memset(entry, 0, sizeof(REC_BLOG_ENTRY));
	authlevel=auth_priv(sid, "forums");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		entry->obj_uid=sid->dat->user_uid;
		entry->obj_gid=sid->dat->user_gid;
		entry->obj_did=sid->dat->user_did;
		entry->obj_gperm=1;
		entry->obj_operm=1;
		snprintf(entry->subject, sizeof(entry->subject)-1, "New Weblog Entry");
		return 0;
	}
	if (sql_queryf(&sqr, "SELECT * FROM gw_weblog_entries where entryid = %d AND obj_did = %d", index, sid->dat->user_did)<0) return -1;
	sql_permprune(sid, &sqr, NULL);
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		return -2;
	}
	entry->entryid      = atoi(sql_getvalue(&sqr, 0, 0));
	entry->obj_ctime    = time_sql2unix(sql_getvalue(&sqr, 0, 1));
	entry->obj_mtime    = time_sql2unix(sql_getvalue(&sqr, 0, 2));
	entry->obj_uid      = atoi(sql_getvalue(&sqr, 0, 3));
	entry->obj_gid      = atoi(sql_getvalue(&sqr, 0, 4));
	entry->obj_did      = atoi(sql_getvalue(&sqr, 0, 5));
	entry->obj_gperm    = atoi(sql_getvalue(&sqr, 0, 6));
	entry->obj_operm    = atoi(sql_getvalue(&sqr, 0, 7));
	entry->entrygroupid = atoi(sql_getvalue(&sqr, 0, 8));
	strncpy(entry->subject, sql_getvalue(&sqr, 0, 9),  sizeof(entry->subject)-1);
	strncpy(entry->summary, sql_getvalue(&sqr, 0, 10), sizeof(entry->summary)-1);
	strncpy(entry->message, sql_getvalue(&sqr, 0, 11), sizeof(entry->message)-1);
	sql_freeresult(&sqr);
	return 0;
}

int dbwrite_weblog_entry(CONN *sid, int index, REC_BLOG_ENTRY *entry)
{
	char curdate[32];
	char *query=NULL;
	int authlevel;
	SQLRES sqr;
	int rc;

	authlevel=auth_priv(sid, "forums");
	if (authlevel<2) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (index==0) {
		if (sql_query(&sqr, "SELECT max(entryid) FROM gw_weblog_entries")<0) return -1;
		entry->entryid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (entry->entryid<1) entry->entryid=1;
		query=calloc(QUERYBUFSIZE, sizeof(char));
		strcpy(query, "INSERT INTO gw_weblog_entries (entryid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, entrygroupid, subject, summary, message) values (");
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", entry->entryid, curdate, curdate, entry->obj_uid, entry->obj_gid, entry->obj_did, entry->obj_gperm, entry->obj_operm);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%d', ", entry->entrygroupid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "'%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, entry->subject));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, entry->summary));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, entry->message));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "')");
		rc=sql_update(query);
		if (query!=NULL) { free(query); query=NULL; }
		if (rc<0) return -1;
		return entry->entryid;
	} else {
		query=calloc(QUERYBUFSIZE, sizeof(char));
		snprintf(query, QUERYBUFSIZE-1, "UPDATE gw_weblog_entries SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, entry->obj_uid, entry->obj_gid, entry->obj_gperm, entry->obj_operm);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "entrygroupid = '%d', ", entry->entrygroupid);
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "subject = '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, entry->subject));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', summary = '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, entry->summary));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "', message = '%s", str2sql(sid->dat->largebuf, sizeof(sid->dat->largebuf)-1, entry->message));
		strncatf(query, QUERYBUFSIZE-strlen(query)-1, "' WHERE entryid = %d", entry->entryid);
		rc=sql_update(query);
		if (query!=NULL) { free(query); query=NULL; }
		if (rc<0) return -1;
		return entry->entryid;
	}
	return -1;
}
