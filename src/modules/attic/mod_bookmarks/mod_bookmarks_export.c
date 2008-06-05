/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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

static int bookmarks_export_r(CONN *sid, obj_t **qobj1, obj_t **qobj2, int depth, int parentid, int selected)
{
	char indent[256];
	int foldercount=0;
	int fid, pid;
	int i, j;

	for (i=0;i<ldir_numentries(qobj1);i++) {
		if ((pid=atoi(ldir_getval(qobj1, i, "pid")))!=parentid) continue;
		memset(indent, 0, sizeof(indent));
		memset(indent, ' ', depth*4);
		fid=atoi(ldir_getval(qobj1, i, "id"));
		prints(sid, "%s<DT><H3 ADD_DATE=\"1095110095\" LAST_MODIFIED=\"1095110122\">%s</H3>\r\n", indent, str2html(sid, ldir_getval(qobj1, i, "cn")));
		prints(sid, "%s<DL><P>\r\n", indent);
		foldercount+=bookmarks_export_r(sid, qobj1, qobj2, depth+1, fid, selected);
		for (j=0;j<ldir_numentries(qobj2);j++) {
			if (fid!=atoi(ldir_getval(qobj2, j, "pid"))) continue;
			prints(sid, "    %s<DT><A HREF=\"%s\" ADD_DATE=\"1093977127\" LAST_CHARSET=\"ISO-8859-1\">%s</A>\r\n", indent, ldir_getval(qobj2, j, "labeleduri"), ldir_getval(qobj2, j, "cn"));
		}
		prints(sid, "%s</DL><P>\r\n", indent);
	}
	return foldercount;
}

void bookmarks_export(CONN *sid)
{
	obj_t *qobj1=NULL;
	obj_t *qobj2=NULL;
	int j;
	int pid=sid->dat->did;

	if (sql_queryf(proc->N, &qobj1, "SELECT id FROM nullgs_entries WHERE class = 'organizationalunit' AND name = 'OtherData' AND pid = %d AND did = %d", sid->dat->did, sid->dat->did)<0) return;
	if (sql_numtuples(proc->N, &qobj1)>0) pid=atoi(sql_getvaluebyname(proc->N, &qobj1, 0, "id"));
	sql_freeresult(proc->N, &qobj1);
//	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
//		if ((qobj1=ldir_getlist(sid->N, "bookmarkfolder", pid, sid->dat->did))==NULL) return;
		if ((qobj1=ldir_getlist(sid->N, "bookmarkfolder", 0, sid->dat->did))==NULL) return;
		if ((qobj2=ldir_getlist(sid->N, "bookmark", 0, sid->dat->did))==NULL) return;
//	} else {
//		rc1=sql_queryf(proc->N, &qobj1, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->uid, sid->dat->gid, sid->dat->did);
//		rc2=sql_queryf(proc->N, &qobj2, "SELECT bookmarkid, folderid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", sid->dat->uid, sid->dat->gid, sid->dat->did);
//	}
	if ((qobj1==NULL)||(qobj2==NULL)) {
		if (qobj1) ldir_freeresult(&qobj1);
		if (qobj2) ldir_freeresult(&qobj2);
		return;
	}
	prints(sid, "<!DOCTYPE NETSCAPE-Bookmark-file-1>\r\n");
	prints(sid, "<!-- This is an automatically generated file.\r\n");
	prints(sid, "     It will be read and overwritten.\r\n");
	prints(sid, "     DO NOT EDIT! -->\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\r\n");
	prints(sid, "<TITLE>Bookmarks</TITLE>\r\n");
	prints(sid, "<H1>Bookmarks</H1>\r\n\r\n");
	prints(sid, "<DL><P>\r\n");
	bookmarks_export_r(sid, &qobj1, &qobj2, 1, pid, 0);
	for (j=0;j<ldir_numentries(&qobj2);j++) {
		if (atoi(ldir_getval(&qobj2, j, "pid"))!=pid) continue;
		prints(sid, "    <DT><A HREF=\"%s\" ADD_DATE=\"1093977127\" LAST_CHARSET=\"ISO-8859-1\">%s</A>\r\n", ldir_getval(&qobj2, j, "labeleduri"), ldir_getval(&qobj2, j, "cn"));
	}
	prints(sid, "</DL><P>\r\n");
	ldir_freeresult(&qobj1);
	ldir_freeresult(&qobj2);
	return;
}
