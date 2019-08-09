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
#include "nullgw/ghttpd/mod.h"
#include "mod_weblog_lang.h"

typedef struct {
	int commentid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int entryid;
	int referenceid;
	char subject[121];
	char message[MAX_FIELD_SIZE];
} REC_BLOG_COMMENT;
typedef struct {
	int entryid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int entrygroupid;
	char subject[121];
	char summary[MAX_FIELD_SIZE];
	char message[MAX_FIELD_SIZE];
} REC_BLOG_ENTRY;
typedef struct {
	int groupid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char name[51];
	char description[241];
} REC_BLOG_GROUP;


/* mod_weblog_db.c */
int dblist_weblog_comments(CONN *sid, SQLRES *sqr, int entryid);
int dbread_weblog_comment(CONN *sid, short int perm, int index, int entryindex, REC_BLOG_COMMENT *comment);
int dbwrite_weblog_comment(CONN *sid, int index, REC_BLOG_COMMENT *comment);

int dblist_weblog_entries(CONN *sid, SQLRES *sqr, int userid, int groupid);
int dbread_weblog_entry(CONN *sid, short int perm, int index, REC_BLOG_ENTRY *entry);
int dbwrite_weblog_entry(CONN *sid, int index, REC_BLOG_ENTRY *entry);
/* mod_weblog_lang.c */
int lang_read(void);
