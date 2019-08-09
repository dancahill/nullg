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

static int bookmarks_export_r(CONN *sid, SQLRES *sqr1, SQLRES *sqr2, int depth, int parentid, int selected)
{
	char indent[256];
	int foldercount=0;
	int fid, pid;
	int i, j;

	for (i=0;i<sql_numtuples(sqr1);i++) {
		pid=atoi(sql_getvalue(sqr1, i, 1));
		if (pid==parentid) {
			memset(indent, 0, sizeof(indent));
			memset(indent, ' ', depth*4);
			fid=atoi(sql_getvalue(sqr1, i, 0));
			prints(sid, "%s<DT><H3 ADD_DATE=\"1095110095\" LAST_MODIFIED=\"1095110122\">%s</H3>\r\n", indent, str2html(sid, sql_getvalue(sqr1, i, 2)));
			prints(sid, "%s<DL><P>\r\n", indent);
			foldercount+=bookmarks_export_r(sid, sqr1, sqr2, depth+1, fid, selected);
			for (j=0;j<sql_numtuples(sqr2);j++) {
				if (fid!=atoi(sql_getvalue(sqr2, j, 1))) continue;
				prints(sid, "    %s<DT><A HREF=\"%s\" ADD_DATE=\"1093977127\" LAST_CHARSET=\"ISO-8859-1\">%s</A>\r\n", indent, sql_getvalue(sqr2, j, 3), sql_getvalue(sqr2, j, 2));
			}
			prints(sid, "%s</DL><P>\r\n", indent);
		}
	}
	return foldercount;
}

void bookmarks_export(CONN *sid)
{
	SQLRES sqr1;
	SQLRES sqr2;
	short int rc1, rc2;
	int j;

	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		rc1=sql_queryf(&sqr1, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->user_did);
		rc2=sql_queryf(&sqr2, "SELECT bookmarkid, folderid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE obj_did = %d ORDER BY bookmarkname ASC", sid->dat->user_did);
	} else {
		rc1=sql_queryf(&sqr1, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did);
		rc2=sql_queryf(&sqr2, "SELECT bookmarkid, folderid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did);
	}
	if ((rc1<0)||(rc2<0)) {
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
	bookmarks_export_r(sid, &sqr1, &sqr2, 1, 0, 0);
	for (j=0;j<sql_numtuples(&sqr2);j++) {
		if (atoi(sql_getvalue(&sqr2, j, 1))!=0) continue;
		prints(sid, "    <DT><A HREF=\"%s\" ADD_DATE=\"1093977127\" LAST_CHARSET=\"ISO-8859-1\">%s</A>\r\n", sql_getvalue(&sqr2, j, 3), sql_getvalue(&sqr2, j, 2));
	}
	prints(sid, "</DL><P>\r\n");
	sql_freeresult(&sqr1);
	sql_freeresult(&sqr2);
	return;
}
