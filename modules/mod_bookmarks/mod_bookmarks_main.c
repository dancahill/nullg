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
#include "mod_stub.h"
#include "mod_bookmarks.h"

void htselect_bookmarkfolder(CONN *sid, int selected)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j;
	int x;
	int sqr;

	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if ((sqr=sql_queryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders ORDER BY parentid ASC, foldername ASC"))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT folderid, parentid, foldername FROM gw_bookmarkfolders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY parentid ASC, foldername ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	prints(sid, "<OPTION VALUE='0'>Root Folder\n");
	if (sql_numtuples(sqr)<1) {
		sql_freeresult(sqr);
		return;
	}
	btree=calloc(sql_numtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(sqr);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr, i, 0));
			btree[i].printed=1;
			depth++;
		}
	}
	if (depth>0) {
		depth--;
		goto widthloop;
	}
	base++;
	btree[depth].lastref=0;
	depth=0;
	if (base<sql_numtuples(sqr)) {
		goto widthloop;
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		x=atoi(sql_getvalue(sqr, ptree[i].id, 0));
		prints(sid, "<OPTION VALUE='%d'%s>", x, x==selected?" SELECTED":"");
		for (indent=0;indent<ptree[i].depth;indent++) prints(sid, "&nbsp;&nbsp;");
		prints(sid, "%s\n", str2html(sid, sql_getvalue(sqr, ptree[i].id, 2)));
	}
	free(ptree);
	free(btree);
	sql_freeresult(sqr);
	return;
}

void bookmarkfolderedit(CONN *sid)
{
	REC_BOOKMARKFOLDER bookmarkfolder;
	int folderid;

	if (!(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/foldereditnew", 24)==0) {
		folderid=0;
		dbread_bookmarkfolder(sid, 2, 0, &bookmarkfolder);
		if (getgetenv(sid, "PARENT")==NULL) return;
		bookmarkfolder.parentid=atoi(getgetenv(sid, "PARENT"));
	} else {
		if (getgetenv(sid, "FOLDERID")==NULL) return;
		folderid=atoi(getgetenv(sid, "FOLDERID"));
		if (dbread_bookmarkfolder(sid, 2, folderid, &bookmarkfolder)!=0) {
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/foldersave NAME=folderedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n");
	prints(sid, "<INPUT TYPE=hidden NAME=folderid VALUE='%d'>\n", bookmarkfolder.folderid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (folderid!=0) {
		prints(sid, "Bookmark Folder %d</FONT></TH></TR>\n", folderid);
	} else {
		prints(sid, "New Bookmark Folder</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=parentid style='width:217px'>\n", config->colour_editform);
	htselect_bookmarkfolder(sid, bookmarkfolder.parentid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", config->colour_editform, str2html(sid, bookmarkfolder.foldername));
	prints(sid, "</TABLE></TD></TR>\n");
	if ((bookmarkfolder.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmarkfolder.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, bookmarkfolder.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", bookmarkfolder.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", bookmarkfolder.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", bookmarkfolder.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", bookmarkfolder.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", bookmarkfolder.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", bookmarkfolder.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)&&(folderid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.folderedit.foldername.focus();\n// -->\n</SCRIPT>\n");
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
	int sqr;

	if (!(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FOLDERID"))==NULL) return;
	folderid=atoi(ptemp);
	if (dbread_bookmarkfolder(sid, 2, folderid, &bookmarkfolder)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_queryf(sid, "SELECT folderid FROM gw_bookmarkfolders where parentid = %d", bookmarkfolder.folderid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if ((sqr=sql_queryf(sid, "SELECT bookmarkid FROM gw_bookmarks where folderid = %d", bookmarkfolder.folderid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if (sql_updatef(sid, "DELETE FROM gw_bookmarkfolders WHERE folderid = %d", bookmarkfolder.folderid)<0) return;
		prints(sid, "<CENTER>Bookmark Folder %d deleted successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/bookmarks/list?folder=%d\">\n", sid->dat->in_ScriptName, bookmarkfolder.parentid);
		db_log_activity(sid, 1, "bookmarkfolders", bookmarkfolder.folderid, "delete", "%s - %s deleted bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	} else if (bookmarkfolder.folderid==0) {
		if (strlen(bookmarkfolder.foldername)<1) {
			prints(sid, "<CENTER>Folder name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_query(sid, "SELECT max(folderid) FROM gw_bookmarkfolders"))<0) return;
		bookmarkfolder.folderid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (bookmarkfolder.folderid<1) bookmarkfolder.folderid=1;
		strcpy(query, "INSERT INTO gw_bookmarkfolders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, parentid, foldername) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", bookmarkfolder.folderid, curdate, curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", bookmarkfolder.parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, bookmarkfolder.foldername));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d added successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/bookmarks/list?folder=%d\">\n", sid->dat->in_ScriptName, bookmarkfolder.folderid);
		db_log_activity(sid, 1, "bookmarkfolders", bookmarkfolder.folderid, "insert", "%s - %s added bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_bookmarkfolders SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, bookmarkfolder.obj_uid, bookmarkfolder.obj_gid, bookmarkfolder.obj_gperm, bookmarkfolder.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "parentid = '%d', ", bookmarkfolder.parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "foldername = '%s'", str2sql(sid, bookmarkfolder.foldername));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE folderid = %d", bookmarkfolder.folderid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Bookmark folder %d modified successfully</CENTER><BR>\n", bookmarkfolder.folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/bookmarks/list?folder=%d\">\n", sid->dat->in_ScriptName, bookmarkfolder.parentid);
		db_log_activity(sid, 1, "bookmarkfolders", bookmarkfolder.folderid, "modify", "%s - %s modified bookmarkfolders %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmarkfolder.folderid);
	}
	return;
}

void bookmarksedit(CONN *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;
	int err;

	if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/bookmarks/editnew", 18)==0) {
		bookmarkid=0;
		if ((err=dbread_bookmark(sid, 2, 0, &bookmark))==-2) {
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
		if ((err=dbread_bookmark(sid, 2, bookmarkid, &bookmark))==-2) {
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
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (bookmarkid!=0) {
		prints(sid, "Bookmark Number %d</FONT></TH></TR>\n", bookmark.bookmarkid);
	} else {
		prints(sid, "New Bookmark</FONT></TH></TR>");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Folder &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=folderid style='width:217px'>\n", config->colour_editform);
	htselect_bookmarkfolder(sid, bookmark.folderid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Name   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkname    VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", config->colour_editform, str2html(sid, bookmark.bookmarkname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Bookmark Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkurl     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", config->colour_editform, str2html(sid, bookmark.bookmarkurl));
	prints(sid, "</TABLE></TD></TR>\n");
	if ((bookmark.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "bookmarks")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmark.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "bookmarks")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, bookmark.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", bookmark.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", bookmark.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", bookmark.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", bookmark.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", bookmark.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", bookmark.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "bookmarks")&A_DELETE)&&(bookmarkid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.bookmarkedit.bookmarkname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void bookmarkslist(CONN *sid)
{
	char *ptemp;
	int folderid=0;
	int i;
	int numfolders=0;
	int sqr;
	
	if (!(auth_priv(sid, "bookmarks")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "FOLDER"))!=NULL) folderid=atoi(ptemp);
	prints(sid, "<CENTER>\n");
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if ((sqr=sql_queryf(sid, "SELECT folderid, foldername, parentid FROM gw_bookmarkfolders WHERE folderid = %d ORDER BY foldername ASC", folderid))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT folderid, foldername, parentid FROM gw_bookmarkfolders WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s><TH", config->colour_th);
		if ((auth_priv(sid, "bookmarks")&A_MODIFY)) prints(sid, " COLSPAN=2");
		prints(sid, " STYLE='border-style:solid'><FONT COLOR=%s>%s</FONT></TH></TR>\n", config->colour_thtext, str2html(sid, sql_getvalue(sqr, 0, 1)));
		prints(sid, "<TR BGCOLOR=%s><TD", config->colour_fieldval);
		if ((auth_priv(sid, "bookmarks")&A_MODIFY)) prints(sid, " COLSPAN=2");
		prints(sid, " WIDTH=300 STYLE='border-style:solid'><A HREF=%s/bookmarks/list?folder=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, 0, 2)));
		prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-foldero.gif HEIGHT=16 WIDTH=16>&nbsp;");
		prints(sid, "Parent Directory</A></TD></TR>\n");
		numfolders++;
	}
	sql_freeresult(sqr);
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if ((sqr=sql_queryf(sid, "SELECT folderid, foldername FROM gw_bookmarkfolders WHERE parentid = %d ORDER BY foldername ASC", folderid))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT folderid, foldername FROM gw_bookmarkfolders WHERE parentid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY foldername ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s><TH", config->colour_th);
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) prints(sid, " COLSPAN=2");
			prints(sid, " STYLE='border-style:solid'><FONT COLOR=%s>Bookmarks</FONT></TH></TR>\n", config->colour_thtext);
		}
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/folderedit?folderid=%d>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR><A HREF=%s/bookmarks/list?folder=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-folder.gif HEIGHT=16 WIDTH=16>&nbsp;");
			prints(sid, "%s</A>&nbsp;</NOBR></TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		numfolders++;
	}
	sql_freeresult(sqr);
	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
		if ((sqr=sql_queryf(sid, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d ORDER BY bookmarkname ASC", folderid))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY bookmarkname ASC", folderid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s>", config->colour_th);
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) prints(sid, "<TH STYLE='border-style:solid'>&nbsp;</TH>");
			prints(sid, "<TH ALIGN=left WIDTH=250 STYLE='border-style:solid'><FONT COLOR=%s>Name</FONT></TH></TR>\n", config->colour_thtext);
		}
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			if ((auth_priv(sid, "bookmarks")&A_MODIFY)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/edit?bookmarkid=%s>edit</A>&nbsp;</TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR><A HREF=\"%s\" TARGET=_blank>", sql_getvalue(sqr, i, 2));
			prints(sid, "%s</A></NOBR></TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	}
	if ((numfolders>0)||(sql_numtuples(sqr)>0)) {
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no saved bookmarks<BR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "</CENTER>\n");
	return;
}

void bookmarkssave(CONN *sid)
{
	REC_BOOKMARK bookmark;
	char *ptemp;
	int bookmarkid;

	if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "BOOKMARKID"))==NULL) return;
	bookmarkid=atoi(ptemp);
	if (dbread_bookmark(sid, 2, bookmarkid, &bookmark)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_bookmarks WHERE bookmarkid = %d", bookmark.bookmarkid)<0) return;
		prints(sid, "<CENTER>Bookmark %d deleted successfully</CENTER><BR>\n", bookmark.bookmarkid);
		db_log_activity(sid, 1, "bookmarks", bookmark.bookmarkid, "delete", "%s - %s deleted bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else if (bookmark.bookmarkid==0) {
		if (!(auth_priv(sid, "bookmarks")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((bookmark.bookmarkid=dbwrite_bookmark(sid, 0, &bookmark))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d added successfully</CENTER><BR>\n", bookmark.bookmarkid);
		db_log_activity(sid, 1, "bookmarks", bookmark.bookmarkid, "insert", "%s - %s added bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	} else {
		if (!(auth_priv(sid, "bookmarks")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (dbwrite_bookmark(sid, bookmarkid, &bookmark)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Bookmark %d modified successfully</CENTER><BR>\n", bookmark.bookmarkid);
		db_log_activity(sid, 1, "bookmarks", bookmark.bookmarkid, "modify", "%s - %s modified bookmark %d", sid->dat->in_RemoteAddr, sid->dat->user_username, bookmark.bookmarkid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\n<!--\n");
	prints(sid, "location.replace(\"%s/bookmarks/list?folder=%d\");\n", sid->dat->in_ScriptName, bookmark.folderid);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/bookmarks/list?folder=%d\">\n", sid->dat->in_ScriptName, bookmark.folderid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void mod_main(CONN *sid)
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

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod;

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	memset((char *)&newmod, 0, sizeof(newmod));
	newmod.mod_submenu=3;
	snprintf(newmod.mod_name,     sizeof(newmod.mod_name)-1,     "mod_bookmarks");
	snprintf(newmod.mod_menuname, sizeof(newmod.mod_menuname)-1, "BOOKMARKS");
	snprintf(newmod.mod_menuperm, sizeof(newmod.mod_menuperm)-1, "bookmarks");
	snprintf(newmod.mod_menuuri,  sizeof(newmod.mod_menuuri)-1,  "/bookmarks/list");
	snprintf(newmod.fn_name,      sizeof(newmod.fn_name)-1,      "mod_main");
	snprintf(newmod.fn_uri,       sizeof(newmod.fn_uri)-1,       "/bookmarks/");
	newmod.fn_ptr=mod_main;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
