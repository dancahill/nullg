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
#define SRVMOD_MAIN 1
#include "mod_bookmarks.h"

static int htselect_bookmarkfolder_r(CONN *sid, SQLRES *sqr, int depth, int parentid, int selected)
{
	int foldercount=0;
	int fid, pid;
	int i, j;

	for (i=0;i<sql_numtuples(sqr);i++) {
		pid=atoi(sql_getvalue(sqr, i, 1));
		if (pid==parentid) {
			fid=atoi(sql_getvalue(sqr, i, 0));
			prints(sid, "<OPTION VALUE='%d'%s>", fid, fid==selected?" SELECTED":"");
			for (j=0;j<depth;j++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
			prints(sid, "%s\r\n", str2html(sid, sql_getvalue(sqr, i, 2)));
			foldercount+=htselect_bookmarkfolder_r(sid, sqr, depth+1, fid, selected);
		}
	}
	return foldercount;
}

void htselect_bookmarkfolder(CONN *sid, int selected)
{
	SQLRES sqr;

	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if (sql_queryf(&sqr, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->user_did)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did)<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>Root Folder\r\n");
	if (sql_numtuples(&sqr)<1) {
		sql_freeresult(&sqr);
		return;
	}
	htselect_bookmarkfolder_r(sid, &sqr, 1, 0, selected);
	sql_freeresult(&sqr);
	return;
}

void bookmarkfolderedit(CONN *sid)
{
	REC_BOOKMARKFOLDER bookmarkfolder;
	char *ptemp;
	int folderid;

	if (!(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/foldereditnew", 24)==0) {
		folderid=0;
		dbread_bookmarkfolder(sid, 2, 0, &bookmarkfolder);
		if ((ptemp=getgetenv(sid, "PARENT"))==NULL) return;
		bookmarkfolder.parentid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "FOLDERID"))==NULL) return;
		folderid=atoi(ptemp);
		if (dbread_bookmarkfolder(sid, 2, folderid, &bookmarkfolder)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", folderid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (folderid>0) {
		prints(sid, "<B>Bookmark Folder %d</B>\r\n", folderid);
	} else {
		prints(sid, "<B>New Bookmark Folder</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/foldersave NAME=folderedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=folderid VALUE='%d'>\r\n", bookmarkfolder.folderid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SETTINGS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=parentid style='width:217px'>\r\n");
	htselect_bookmarkfolder(sid, bookmarkfolder.parentid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, bookmarkfolder.foldername));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((bookmarkfolder.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmarkfolder.obj_uid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "bookmarks"), bookmarkfolder.obj_gid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\r\n", bookmarkfolder.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\r\n", bookmarkfolder.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\r\n", bookmarkfolder.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\r\n", bookmarkfolder.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\r\n", bookmarkfolder.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\r\n", bookmarkfolder.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)&&(folderid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.folderedit.foldername.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}

void bookmarkfoldersave(CONN *sid)
{
	REC_BOOKMARKFOLDER bookmarkfolder;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int folderid;
	SQLRES sqr;

	if (!(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FOLDERID"))==NULL) return;
	folderid=atoi(ptemp);
	if (dbread_bookmarkfolder(sid, 2, folderid, &bookmarkfolder)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "bookmarks")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) bookmarkfolder.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) bookmarkfolder.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)||(bookmarkfolder.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) bookmarkfolder.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) bookmarkfolder.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "PARENTID"))!=NULL) bookmarkfolder.parentid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "FOLDERNAME"))!=NULL) snprintf(bookmarkfolder.foldername, sizeof(bookmarkfolder.foldername)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (bookmarkfolder.parentid==bookmarkfolder.folderid) bookmarkfolder.parentid=0;
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders where parentid = %d", bookmarkfolder.folderid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\r\n");
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT bookmarkid FROM gw_bookmarks where folderid = %d", bookmarkfolder.folderid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\r\n");
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (sql_updatef("DELETE FROM gw_bookmarks_folders WHERE folderid = %d", bookmarkfolder.folderid)<0) return;
		prints(sid, "<CENTER>Bookmark Folder %d deleted successfully</CENTER><BR>\r\n", bookmarkfolder.folderid);
		db_log_activity(sid, "bookmarkfolders", bookmarkfolder.folderid, "delete", "%s - %s deleted bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
		folderid=bookmarkfolder.parentid;
	} else if (bookmarkfolder.folderid==0) {
		if (strlen(bookmarkfolder.foldername)<1) {
			prints(sid, "<CENTER>Folder name is too short</CENTER><BR>\r\n");
			return;
		}
		if (sql_query(&sqr, "SELECT max(folderid) FROM gw_bookmarks_folders")<0) return;
		bookmarkfolder.folderid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (bookmarkfolder.folderid<1) bookmarkfolder.folderid=1;
		strcpy(query, "INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", bookmarkfolder.folderid, curdate, curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_did, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", bookmarkfolder.parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmarkfolder.foldername));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d added successfully</CENTER><BR>\r\n", bookmarkfolder.folderid);
		db_log_activity(sid, "bookmarkfolders", bookmarkfolder.folderid, "insert", "%s - %s added bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
		folderid=bookmarkfolder.folderid;
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_bookmarks_folders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "parentid = '%d', ", bookmarkfolder.parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "foldername = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, bookmarkfolder.foldername));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE folderid = %d", bookmarkfolder.folderid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d modified successfully</CENTER><BR>\r\n", bookmarkfolder.folderid);
		db_log_activity(sid, "bookmarkfolders", bookmarkfolder.folderid, "modify", "%s - %s modified bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
		folderid=bookmarkfolder.parentid;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/bookmarks/list?folder=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, folderid);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/bookmarks/list?folder=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, folderid);
	return;
}

void bookmarksedit(CONN *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;
	int err;

	if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/editnew", 18)==0) {
		bookmarkid=0;
		if ((err=dbread_bookmark(sid, 2, 0, &bookmark))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", bookmarkid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) bookmark.folderid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "BOOKMARKID"))==NULL) return;
		bookmarkid=atoi(ptemp);
		if ((err=dbread_bookmark(sid, 2, bookmarkid, &bookmark))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", bookmarkid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (bookmarkid>0) {
		prints(sid, "<B>Bookmark %d</B>\r\n", bookmarkid);
	} else {
		prints(sid, "<B>New Bookmark</B>\r\n");
	}


	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/save NAME=bookmarkedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=bookmarkid VALUE='%d'>\r\n", bookmark.bookmarkid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SETTINGS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Folder &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=folderid style='width:217px'>\r\n");
	htselect_bookmarkfolder(sid, bookmark.folderid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Name   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkname    VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, bookmark.bookmarkname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkurl     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, bookmark.bookmarkurl));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((bookmark.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmark.obj_uid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "bookmarks"), bookmark.obj_gid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\r\n", bookmark.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\r\n", bookmark.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\r\n", bookmark.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\r\n", bookmark.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\r\n", bookmark.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\r\n", bookmark.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "bookmarks")&A_DELETE)&&(bookmarkid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.bookmarkedit.bookmarkname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}

void bookmarkslist(CONN *sid)
{
	char *ptemp;
	int folderid=-1;
	int i;
	int numfolders=0;
	SQLRES sqr;
	int modify=0;

	if (!(auth_priv(sid, "bookmarks")&A_READ)) {
		htpage_topmenu(sid, MENU_BOOKMARKS);
		prints(sid, "<BR>\r\n");
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) folderid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "PARENT"))!=NULL) folderid=atoi(ptemp);
	if (folderid==-1) {
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders WHERE foldername = 'users' AND parentid = 0 AND obj_did = %d", sid->dat->user_did)<0) return;
		if (sql_numtuples(&sqr)>0) folderid=atoi(sql_getvalue(&sqr, 0, 0));
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders WHERE foldername = '%s' AND parentid = %d AND obj_did = %d", sid->dat->user_username, folderid, sid->dat->user_did)<0) return;
		if (sql_numtuples(&sqr)>0) folderid=atoi(sql_getvalue(&sqr, 0, 0));
		sql_freeresult(&sqr);
		if (strcmp("/bookmarks/list", sid->dat->in_RequestURI)==0) {
			snprintf(sid->dat->in_QueryString, sizeof(sid->dat->in_QueryString)-1, "folder=%d", folderid);
//			snprintf(sid->dat->in_RequestURI, sizeof(sid->dat->in_RequestURI)-1, "/bookmarks/list?folder=%d", folderid);
		}
	}
	htpage_topmenu(sid, MENU_BOOKMARKS);
	prints(sid, "<BR>\r\n");
	prints(sid, "<CENTER>\r\n");
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if (sql_queryf(&sqr, "SELECT folderid, foldername, parentid FROM gw_bookmarks_folders WHERE folderid = %d AND obj_did = %d ORDER BY foldername ASC", folderid, sid->dat->user_did)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT folderid, foldername, parentid FROM gw_bookmarks_folders WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did)<0) return;
	}
	modify=auth_priv(sid, "bookmarks")&A_MODIFY;
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH%s STYLE='border-style:solid'>%s</TH></TR>\r\n", modify?" COLSPAN=2":"", str2html(sid, sql_getvalue(&sqr, 0, 1)));
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD%s WIDTH=300 STYLE='border-style:solid'>", modify?" COLSPAN=2":"");
		prints(sid, "<A HREF=%s/bookmarks/list?folder=%d onClick=\"location.replace('%s/bookmarks/list?folder=%d');return false;\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, 0, 2)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, 0, 2)));
		prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-foldero.png HEIGHT=16 WIDTH=16>");
		prints(sid, "&nbsp;Parent Directory</A></TD></TR>\r\n");
		numfolders++;
	}
	sql_freeresult(&sqr);
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if (sql_queryf(&sqr, "SELECT folderid, foldername FROM gw_bookmarks_folders WHERE parentid = %d AND obj_did = %d ORDER BY foldername ASC", folderid, sid->dat->user_did)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT folderid, foldername FROM gw_bookmarks_folders WHERE parentid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did)<0) return;
	}
	if (sql_numtuples(&sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH%s STYLE='border-style:solid'>Bookmarks</TH></TR>\r\n", modify?" COLSPAN=2":"");
		}
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/folderedit?folderid=%d>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR>");
			prints(sid, "<A HREF=%s/bookmarks/list?folder=%d onClick=\"location.replace('%s/bookmarks/list?folder=%d');return false;\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-folder.png HEIGHT=16 WIDTH=16>&nbsp;");
			prints(sid, "%s</A>&nbsp;</NOBR></TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
		numfolders++;
	}
	sql_freeresult(&sqr);
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if (sql_queryf(&sqr, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND obj_did = %d ORDER BY bookmarkname ASC", folderid, sid->dat->user_did)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", folderid, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did)<0) return;
	}
	if (sql_numtuples(&sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>");
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) prints(sid, "<TH STYLE='border-style:solid'>&nbsp;</TH>");
			prints(sid, "<TH ALIGN=left WIDTH=250 STYLE='border-style:solid'>Name</TH></TR>\r\n");
		}
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/edit?bookmarkid=%s>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, sql_getvalue(&sqr, i, 0));
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR><A HREF=\"%s\" TARGET=_blank>", sql_getvalue(&sqr, i, 2));
			prints(sid, "%s</A></NOBR></TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
	}
	if ((numfolders>0)||(sql_numtuples(&sqr)>0)) {
		prints(sid, "</TABLE>\r\n");
	} else {
		prints(sid, "There are no saved bookmarks<BR>\r\n");
	}
	sql_freeresult(&sqr);
	prints(sid, "</CENTER><BR>\r\n");
	return;
}

void bookmarkssave(CONN *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;

	if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "BOOKMARKID"))==NULL) return;
	bookmarkid=atoi(ptemp);
	if (dbread_bookmark(sid, 2, bookmarkid, &bookmark)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "bookmarks")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) bookmark.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) bookmark.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)||(bookmark.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) bookmark.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) bookmark.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "FOLDERID"))!=NULL) bookmark.folderid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "BOOKMARKNAME"))!=NULL) snprintf(bookmark.bookmarkname, sizeof(bookmark.bookmarkname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "BOOKMARKURL"))!=NULL) snprintf(bookmark.bookmarkurl, sizeof(bookmark.bookmarkurl)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "bookmarks")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_bookmarks WHERE bookmarkid = %d", bookmark.bookmarkid)<0) return;
		prints(sid, "<CENTER>Bookmark %d deleted successfully</CENTER><BR>\r\n", bookmark.bookmarkid);
		db_log_activity(sid, "bookmarks", bookmark.bookmarkid, "delete", "%s - %s deleted bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else if (bookmark.bookmarkid==0) {
		if (!(auth_priv(sid, "bookmarks")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((bookmark.bookmarkid=dbwrite_bookmark(sid, 0, &bookmark))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d added successfully</CENTER><BR>\r\n", bookmark.bookmarkid);
		db_log_activity(sid, "bookmarks", bookmark.bookmarkid, "insert", "%s - %s added bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else {
		if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (dbwrite_bookmark(sid, bookmarkid, &bookmark)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d modified successfully</CENTER><BR>\r\n", bookmark.bookmarkid);
		db_log_activity(sid, "bookmarks", bookmark.bookmarkid, "modify", "%s - %s modified bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/bookmarks/list?folder=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, bookmark.folderid);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/bookmarks/list?folder=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, bookmark.folderid);
	return;
}

DllExport int mod_main(CONN *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/export", 17)==0) {
		send_header(sid, 0, 200, "1", "text/plain", -1, -1);
		bookmarks_export(sid);
		return 0;
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/list", 15)==0) {
		bookmarkslist(sid);
	} else {
		htpage_topmenu(sid, MENU_BOOKMARKS);
		prints(sid, "<BR>\r\n");
		if (strncmp(sid->dat->in_RequestURI, "/bookmarks/folderedit", 21)==0) {
			bookmarkfolderedit(sid);
		} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/foldersave", 21)==0) {
			bookmarkfoldersave(sid);
		} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/edit", 15)==0) {
			bookmarksedit(sid);
		} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/save", 15)==0) {
			bookmarkssave(sid);
		}
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_bookmarks",	// mod_name
		3,			// mod_submenu
		"BOOKMARKS",		// mod_menuname
		"bookmarks.png",	// mod_menupic
		"/bookmarks/list",	// mod_menuuri
		"bookmarks",		// mod_menuperm
		"mod_main",		// fn_name
		"/bookmarks/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}