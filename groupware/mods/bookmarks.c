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

void bookmarkfolderedit(CONNECTION *sid)
{
	REC_BOOKMARKFOLDER bookmarkfolder;
	int folderid;

	if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/foldereditnew", 24)==0) {
		folderid=0;
		db_read(sid, 2, DB_BOOKMARKFOLDERS, 0, &bookmarkfolder);
		if (getgetenv(sid, "PARENT")==NULL) return;
		bookmarkfolder.parentid=atoi(getgetenv(sid, "PARENT"));
	} else {
		if (getgetenv(sid, "FOLDERID")==NULL) return;
		folderid=atoi(getgetenv(sid, "FOLDERID"));
		if (db_read(sid, 2, DB_BOOKMARKFOLDERS, folderid, &bookmarkfolder)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", folderid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/foldersave NAME=folderedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=folderid VALUE='%d'>\n", bookmarkfolder.folderid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (folderid!=0) {
		prints(sid, "Folder %d</FONT></TH></TR>\n", folderid);
	} else {
		prints(sid, "New Folder</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=parentid style='width:182px'>\n", COLOR_EDITFORM);
	htselect_bookmarkfolder(sid, bookmarkfolder.parentid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=25></TD></TR>\n", COLOR_EDITFORM, str2html(sid, bookmarkfolder.foldername));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)&&(folderid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.folderedit.foldername.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void bookmarkfoldersave(CONNECTION *sid)
{
	REC_BOOKMARKFOLDER bookmarkfolder;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int folderid;
	int sqr;

	if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FOLDERID"))==NULL) return;
	folderid=atoi(ptemp);
	if (db_read(sid, 2, DB_BOOKMARKFOLDERS, folderid, &bookmarkfolder)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "PARENTID"))!=NULL) bookmarkfolder.parentid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "FOLDERNAME"))!=NULL) snprintf(bookmarkfolder.foldername, sizeof(bookmarkfolder.foldername)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sqlQueryf(sid, "SELECT folderid FROM gw_bookmarkfolders where parentid = %d", bookmarkfolder.folderid))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sqlFreeconnect(sqr);
			return;
		}
		sqlFreeconnect(sqr);
		if ((sqr=sqlQueryf(sid, "SELECT bookmarkid FROM gw_bookmarks where folderid = %d", bookmarkfolder.folderid))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sqlFreeconnect(sqr);
			return;
		}
		sqlFreeconnect(sqr);
		if (sqlUpdatef(sid, "DELETE FROM gw_bookmarkfolders WHERE folderid = %d", bookmarkfolder.folderid)<0) return;
		prints(sid, "<CENTER>Bookmark Folder %d deleted successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		logaccess(sid, 1, "%s - %s deleted bookmark folder %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	} else if (bookmarkfolder.folderid==0) {
		if (strlen(bookmarkfolder.foldername)<1) {
			prints(sid, "<CENTER>Folder name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sqlQuery(sid, "SELECT max(folderid) FROM gw_bookmarkfolders"))<0) return;
		bookmarkfolder.folderid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (bookmarkfolder.folderid<1) bookmarkfolder.folderid=1;
		strcpy(query, "INSERT INTO gw_bookmarkfolders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, parentid, foldername) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", bookmarkfolder.folderid, curdate, curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strcatf(query, "'%d', ", bookmarkfolder.parentid);
		strcatf(query, "'%s')", str2sql(sid, bookmarkfolder.foldername));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d added successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		logaccess(sid, 1, "%s - %s added bookmark folder %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	} else {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_bookmarkfolders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strcatf(query, "parentid = '%d', ", bookmarkfolder.parentid);
		strcatf(query, "foldername = '%s'", str2sql(sid, bookmarkfolder.foldername));
		strcatf(query, " WHERE folderid = %d", bookmarkfolder.folderid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d modified successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		logaccess(sid, 1, "%s - %s modified bookmark folder %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/bookmarks/list\">\n", sid->dat->in_ScriptName);
	return;
}

void bookmarksedit(CONNECTION *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;
	int err;

	if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/editnew", 18)==0) {
		bookmarkid=0;
		if ((err=db_read(sid, 2, DB_BOOKMARKS, 0, &bookmark))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", bookmarkid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) bookmark.folderid=atoi(ptemp);
	} else {
		if (getgetenv(sid, "BOOKMARKID")==NULL) return;
		bookmarkid=atoi(getgetenv(sid, "BOOKMARKID"));
		if ((err=db_read(sid, 2, DB_BOOKMARKS, bookmarkid, &bookmark))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", bookmarkid);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/bookmarks/save NAME=bookmarkedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=bookmarkid VALUE='%d'>\n", bookmark.bookmarkid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (bookmarkid!=0) {
		prints(sid, "Bookmark Number %d</FONT></TH></TR>\n", bookmark.bookmarkid);
	} else {
		prints(sid, "New Bookmark</FONT></TH></TR>");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Folder &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=folderid style='width:217px'>\n", COLOR_EDITFORM);
	htselect_bookmarkfolder(sid, bookmark.folderid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Name   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkname    VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, bookmark.bookmarkname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkurl     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, bookmark.bookmarkurl));
	prints(sid, "</TABLE></TD></TR>\n");
	if ((bookmark.obj_uid==sid->dat->user_uid)||(auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s STYLE='padding:0px'><TD><B>&nbsp;Owner&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmark.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, bookmark.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", bookmark.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", bookmark.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", bookmark.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", bookmark.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", bookmark.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", bookmark.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_DELETE)&&(bookmarkid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.bookmarkedit.bookmarkname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void bookmarkslist(CONNECTION *sid)
{
	char *ptemp;
	int folderid=0;
	int i;
	int numfolders=0;
	int sqr;
	
	if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) folderid=atoi(ptemp);
	prints(sid, "<CENTER>\n");
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, foldername, parentid FROM gw_bookmarkfolders WHERE folderid = %d ORDER BY foldername ASC", folderid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, foldername, parentid FROM gw_bookmarkfolders WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s><TH", COLOR_TH);
		if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) prints(sid, " COLSPAN=2");
		prints(sid, "><FONT COLOR=%s>%s</FONT></TH></TR>\n", COLOR_THTEXT, str2html(sid, sqlGetvalue(sqr, 0, 1)));
		prints(sid, "<TR BGCOLOR=%s><TD", COLOR_FVAL);
		if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) prints(sid, " COLSPAN=2");
		prints(sid, " WIDTH=250><IMG SRC=/groupware/images/file-foldero.gif HEIGHT=16 WIDTH=16>&nbsp;");
		prints(sid, "<A HREF=%s/bookmarks/list?folder=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, 0, 2)));
		prints(sid, "Parent Directory</A></TD></TR>\n");
		numfolders++;
	}
	sqlFreeconnect(sqr);
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, foldername FROM gw_bookmarkfolders WHERE parentid = %d ORDER BY foldername ASC", folderid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT folderid, foldername FROM gw_bookmarkfolders WHERE parentid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sqlNumtuples(sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s><TH", COLOR_TH);
			if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) prints(sid, " COLSPAN=2");
			prints(sid, "><FONT COLOR=%s>Bookmarks</FONT></TH></TR>\n", COLOR_THTEXT);
		}
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) {
				prints(sid, "<TD NOWRAP><A HREF=%s/bookmarks/folderedit?folderid=%d>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			}
			prints(sid, "<TD NOWRAP WIDTH=250><NOBR><IMG SRC=/groupware/images/file-folder.gif HEIGHT=16 WIDTH=16>&nbsp;");
			prints(sid, "<A HREF=%s/bookmarks/list?folder=%d>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</NOBR></TD></TR>\n", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
		numfolders++;
	}
	sqlFreeconnect(sqr);
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)) {
		if ((sqr=sqlQueryf(sid, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d ORDER BY bookmarkname ASC", folderid))<0) return;
	} else {
		if ((sqr=sqlQueryf(sid, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY bookmarkname ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sqlNumtuples(sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TH);
			if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) prints(sid, "<TH>&nbsp;</TH>");
			prints(sid, "<TH ALIGN=left WIDTH=200><FONT COLOR=%s>Name</FONT></TH></TR>\n", COLOR_THTEXT);
		}
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			if ((auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) {
				prints(sid, "<TD NOWRAP><A HREF=%s/bookmarks/edit?bookmarkid=%s>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			}
			prints(sid, "<TD NOWRAP WIDTH=250><NOBR><A HREF=\"%s\" TARGET=_blank>", sqlGetvalue(sqr, i, 2));
			prints(sid, "%s</A></NOBR></TD></TR>\n", str2html(sid, sqlGetvalue(sqr, i, 1)));
		}
	}
	if ((numfolders>0)||(sqlNumtuples(sqr)>0)) {
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no saved bookmarks<BR>\n");
	}
	sqlFreeconnect(sqr);
	prints(sid, "</CENTER>\n");
	return;
}

void bookmarkssave(CONNECTION *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;

	if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "BOOKMARKID"))==NULL) return;
	bookmarkid=atoi(ptemp);
	if (db_read(sid, 2, DB_BOOKMARKS, bookmarkid, &bookmark)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) bookmark.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) bookmark.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_BOOKMARKS)&A_ADMIN)||(bookmark.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) bookmark.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) bookmark.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "FOLDERID"))!=NULL) bookmark.folderid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "BOOKMARKNAME"))!=NULL) snprintf(bookmark.bookmarkname, sizeof(bookmark.bookmarkname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "BOOKMARKURL"))!=NULL) snprintf(bookmark.bookmarkurl, sizeof(bookmark.bookmarkurl)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_bookmarks WHERE bookmarkid = %d", bookmark.bookmarkid)<0) return;
		prints(sid, "<CENTER>Bookmark %d deleted successfully</CENTER><BR>\n", bookmark.bookmarkid);
		logaccess(sid, 1, "%s - %s deleted bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else if (bookmark.bookmarkid==0) {
		if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((bookmark.bookmarkid=db_write(sid, DB_BOOKMARKS, 0, &bookmark))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d added successfully</CENTER><BR>\n", bookmark.bookmarkid);
		logaccess(sid, 1, "%s - %s added bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else {
		if (!(auth_priv(sid, AUTH_BOOKMARKS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (db_write(sid, DB_BOOKMARKS, bookmarkid, &bookmark)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d modified successfully</CENTER><BR>\n", bookmark.bookmarkid);
		logaccess(sid, 1, "%s - %s modified bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\n<!--\n");
	prints(sid, "location.replace(\"%s/bookmarks/list\");\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/bookmarks/list\">\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void bookmarksmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_BOOKMARKS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/folderedit", 21)==0) {
		bookmarkfolderedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/foldersave", 21)==0) {
		bookmarkfoldersave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/edit", 15)==0) {
		bookmarksedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/list", 15)==0) {
		bookmarkslist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/bookmarks/save", 15)==0) {
		bookmarkssave(sid);
	}
	htpage_footer(sid);
	return;
}
