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
#define SRVMOD_MAIN 1
#include "mod_bookmarks.h"

static int htselect_bookmarkfolder_r(CONN *sid, obj_t **qobj, int depth, int parentid, int selected)
{
	int foldercount=0;
	int fid, pid;
	int i, j;

	for (i=0;i<sql_numtuples(proc->N, qobj);i++) {
		pid=atoi(sql_getvaluebyname(proc->N, qobj, i, "pid"));
		if (pid==parentid) {
			fid=atoi(sql_getvaluebyname(proc->N, qobj, i, "id"));
			prints(sid, "<OPTION VALUE='%d'%s>", fid, fid==selected?" SELECTED":"");
			for (j=0;j<depth;j++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;");
			prints(sid, "%s\r\n", str2html(sid, sql_getvaluebyname(proc->N, qobj, i, "name")));
			foldercount+=htselect_bookmarkfolder_r(sid, qobj, depth+1, fid, selected);
		}
	}
	return foldercount;
}

void htselect_bookmarkfolder(CONN *sid, int selected)
{
	obj_t *qobj=NULL;
	int pid=sid->dat->did;

	if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullgs_entries WHERE class = 'organizationalunit' AND name = 'OtherData' AND pid = %d AND did = %d", sid->dat->did, sid->dat->did)<0) return;
	if (sql_numtuples(proc->N, &qobj)>0) pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "id"));
	sql_freeresult(proc->N, &qobj);
//	if ((auth_priv(sid, "bookmarks")&A_ADMIN)) {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT * FROM nullgs_entries WHERE class = 'bookmarkfolder' AND did = %d", sid->dat->did)<0) return;
//	} else {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, parentid, foldername FROM gw_bookmarks_folders WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY parentid ASC, foldername ASC", sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		if (sql_queryf(proc->N, &qobj, "SELECT * FROM nullgs_entries WHERE class = 'bookmarkfolder' AND did = %d", sid->dat->did)<0) return;
//	}
	prints(sid, "<OPTION VALUE='0'>Root Folder\r\n");
	if (sql_numtuples(proc->N, &qobj)>0) {
		htselect_bookmarkfolder_r(sid, &qobj, 1, pid, selected);
	}
	sql_freeresult(proc->N, &qobj);
	return;
}

void bookmarkfolderedit(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *tobj, *qobj=NULL;
	char *ptemp;
	int id, pid;
	int priv=auth_priv(sid, "bookmarks");

	if (!(priv&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/bookmarks/foldereditnew", 24)==0) {
		id=0;
		if ((ptemp=getgetenv(sid, "PID"))==NULL) return;
		pid=atoi(ptemp);
		dbread_bookmarkfolder(sid, 2, 0, &qobj);
	} else {
		if ((ptemp=getgetenv(sid, "ID"))==NULL) return;
		id=atoi(ptemp);
		if (dbread_bookmarkfolder(sid, 2, id, &qobj)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", id);
			return;
		}
		pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
	}
	if (qobj==NULL) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	tobj=nes_getobj(sid->N, qobj, "_rows");
	tobj=nes_getiobj(sid->N, tobj, 0);
	tobj=nes_getobj(sid->N, tobj, "_data");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (id>0) {
		prints(sid, "<B>Bookmark Folder %d</B>\r\n", id);
	} else {
		prints(sid, "<B>New Bookmark Folder</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/foldersave NAME=folderedit>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<INPUT TYPE=hidden NAME=id VALUE='%d'>\r\n", id);
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
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=pid style='width:217px'>\r\n");
	htselect_bookmarkfolder(sid, pid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, nes_getstr(sid->N, tobj, "cn")));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*
	if ((bookmarkfolder.obj_uid==sid->dat->uid)||(priv&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmarkfolder.obj_uid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, priv, bookmarkfolder.obj_gid, sid->dat->did);
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
*/
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((priv&A_ADMIN)&&(id>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.folderedit.foldername.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void bookmarkfoldersave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	obj_t *qobj=NULL, *tobj;
	char *ptemp;
	int id;
	int num;
	int pid=0;
	int priv=auth_priv(sid, "bookmarks");

	if (!(priv&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_bookmarkfolder(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (priv&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) bookmarkfolder.obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) bookmarkfolder.obj_gid=atoi(ptemp);
//	}
//	if ((priv&A_ADMIN)||(bookmarkfolder.obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) bookmarkfolder.obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) bookmarkfolder.obj_operm=atoi(ptemp);
//	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	if ((ptemp=getpostenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	nes_setnum(sid->N, tobj, "pid", pid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	if ((ptemp=getpostenv(sid, "FOLDERNAME"))!=NULL) nes_setstr(sid->N, tobj, "cn", ptemp, strlen(ptemp));
//	if (pid==id) pid=0;
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullgs_entries WHERE pid = %d", id)<0) goto cleanup;
		num=sql_numtuples(proc->N, &qobj);
		sql_freeresult(proc->N, &qobj);
		if (num>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\r\n");
			goto cleanup;
		}
		if (ldir_deleteentry("bookmarkfolder", id, sid->dat->did)<0) goto cleanup;
		prints(sid, "<CENTER>Bookmark Folder %d deleted successfully</CENTER>", id);
		db_log_activity(sid, "bookmarkfolders", id, "delete", "%s - %s deleted bookmarkfolders %d", sid->socket.RemoteAddr, sid->dat->username, id);
		id=pid;
	} else if (id==0) {
//		if (strlen(bookmarkfolder.foldername)<1) {
//			prints(sid, "<CENTER>Folder name is too short</CENTER><BR />\r\n");
//			goto cleanup;
//		}
		if ((id=dbwrite_bookmarkfolder(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Bookmark folder %d added successfully</CENTER>", id);
		db_log_activity(sid, "bookmarkfolders", pid, "insert", "%s - %s added bookmarkfolders %d", sid->socket.RemoteAddr, sid->dat->username, pid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (dbwrite_bookmarkfolder(sid, id, &qobj)<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Bookmark folder %d modified successfully</CENTER>", id);
		db_log_activity(sid, "bookmarkfolders", pid, "modify", "%s - %s modified bookmarkfolders %d", sid->socket.RemoteAddr, sid->dat->username, pid);
		id=pid;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/bookmarks/list?pid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/bookmarks/list?pid=%d\"></NOSCRIPT>\r\n", ScriptName, id);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void bookmarksedit(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *tobj, *qobj=NULL;
	char *ptemp;
	int id, pid;
	int err;
	int priv=auth_priv(sid, "bookmarks");

	if (!(priv&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/bookmarks/editnew", 18)==0) {
		id=0;
		if ((err=dbread_bookmark(sid, 2, 0, &qobj))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", id);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
		if ((ptemp=getgetenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "ID"))==NULL) return;
		id=atoi(ptemp);
		if ((err=dbread_bookmark(sid, 2, id, &qobj))==-2) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", id);
			return;
		} else if (err!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
	}
	if (qobj==NULL) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	tobj=nes_getobj(sid->N, qobj, "_rows");
	tobj=nes_getiobj(sid->N, tobj, 0);
	tobj=nes_getobj(sid->N, tobj, "_data");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (id>0) {
		prints(sid, "<B>Bookmark %d</B>\r\n", id);
	} else {
		prints(sid, "<B>New Bookmark</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/bookmarks/save NAME=bookmarkedit>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<INPUT TYPE=hidden NAME=id VALUE='%d'>\r\n", id);
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
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Folder &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=pid style='width:217px'>\r\n");
	htselect_bookmarkfolder(sid, pid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Name   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkname    VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, nes_getstr(sid->N, tobj, "cn")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Bookmark Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=bookmarkurl     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, nes_getstr(sid->N, tobj, "labeleduri")));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*
	if ((bookmark.obj_uid==sid->dat->uid)||(priv&A_ADMIN)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, bookmark.obj_uid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, priv, bookmark.obj_gid, sid->dat->did);
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
*/
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((priv&A_DELETE)&&(id!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.bookmarkedit.bookmarkname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void bookmarkslist(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	char tmpstr[64];
	int i;
	int id;
	int pid=-1;
	int modify=0;
	int numfolders=0;
	int priv=auth_priv(sid, "bookmarks");

	if (!(priv&A_READ)) {
		htpage_topmenu(sid, "bookmarks");
		prints(sid, "<BR />\r\n");
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	if (pid==-1) {
		if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullgs_entries WHERE class = 'organizationalunit' AND name = 'OtherData' AND pid = %d AND did = %d", sid->dat->did, sid->dat->did)<0) return;
		if (sql_numtuples(proc->N, &qobj)>0) pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "id"));
		sql_freeresult(proc->N, &qobj);
		if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullgs_entries WHERE class = 'bookmarkfolder' AND name = 'users' AND pid = %d AND did = %d", pid, sid->dat->did)<0) return;
		if (sql_numtuples(proc->N, &qobj)>0) pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "id"));
		sql_freeresult(proc->N, &qobj);
		if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullgs_entries WHERE class = 'bookmarkfolder' AND name = '%s' AND pid = %d AND did = %d", sid->dat->username, pid, sid->dat->did)<0) return;
		if (sql_numtuples(proc->N, &qobj)>0) pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "id"));
		sql_freeresult(proc->N, &qobj);
		if (strcmp("/bookmarks/list", nes_getstr(sid->N, htobj, "REQUEST_URI"))==0) {
			snprintf(tmpstr, sizeof(tmpstr)-1, "pid=%d", pid);
			nes_setstr(sid->N, htobj, "QUERY_STRING", tmpstr, strlen(tmpstr));
			snprintf(tmpstr, sizeof(tmpstr)-1, "/bookmarks/list?pid=%d", pid);
			nes_setstr(sid->N, htobj, "REQUEST_URI", tmpstr, strlen(tmpstr));
		}
	}
	htpage_topmenu(sid, "bookmarks");
	prints(sid, "<BR />\r\n");
	prints(sid, "<CENTER>\r\n");
	/* query the parent folder */
//	if ((priv&A_ADMIN)) {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, foldername, parentid FROM gw_bookmarks_folders WHERE folderid = %d AND obj_did = %d ORDER BY foldername ASC", pid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT pid, name FROM nullgs_entries WHERE class = 'bookmarkfolder' AND id = %d AND did = %d", pid, sid->dat->did)<0) return;
//	} else {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, foldername, parentid FROM gw_bookmarks_folders WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY foldername ASC", pid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
		if (sql_queryf(proc->N, &qobj, "SELECT pid, name FROM nullgs_entries WHERE class = 'bookmarkfolder' AND id = %d AND did = %d", pid, sid->dat->did)<0) return;
//	}
	modify=priv&A_MODIFY;
	if (sql_numtuples(proc->N, &qobj)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH%s STYLE='border-style:solid'>%s</TH></TR>\r\n", modify?" COLSPAN=2":"", str2html(sid, sql_getvaluebyname(proc->N, &qobj, 0, "name")));
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD%s WIDTH=300 STYLE='border-style:solid'>", modify?" COLSPAN=2":"");
		id=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
		prints(sid, "<A HREF=%s/bookmarks/list?pid=%d onClick=\"location.replace('%s/bookmarks/list?pid=%d');return false;\">", ScriptName, id, ScriptName, id);
		prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/images/file-foldero.png HEIGHT=16 WIDTH=16>");
		prints(sid, "&nbsp;Parent Directory</A></TD></TR>\r\n");
		numfolders++;
	}
	sql_freeresult(proc->N, &qobj);
	/* list subfolders */
//	if ((priv&A_ADMIN)) {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, foldername FROM gw_bookmarks_folders WHERE parentid = %d AND obj_did = %d ORDER BY foldername ASC", pid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT id, name FROM nullgs_entries WHERE class = 'bookmarkfolder' AND pid = %d AND did = %d ORDER BY name ASC", pid, sid->dat->did)<0) return;
//		if ((qobj=ldir_getlist(sid->N, "bookmarkfolder", pid, sid->dat->did))==NULL) return;
//	} else {
//		if (sql_queryf(proc->N, &qobj, "SELECT folderid, foldername FROM gw_bookmarks_folders WHERE parentid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY foldername ASC", pid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT id, name FROM nullgs_entries WHERE class = 'bookmarkfolder' AND pid = %d AND did = %d ORDER BY name ASC", pid, sid->dat->did)<0) return;
		if ((qobj=ldir_getlist(sid->N, "bookmarkfolder", pid, sid->dat->did))==NULL) return;
//	}
	if (ldir_numentries(&qobj)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH%s STYLE='border-style:solid'>Bookmarks</TH></TR>\r\n", modify?" COLSPAN=2":"");
		}
		for (i=0;i<ldir_numentries(&qobj);i++) {
			id=atoi(ldir_getval(&qobj, i, "id"));
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			if ((priv&A_MODIFY)) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/folderedit?id=%d>edit</A>&nbsp;</TD>", ScriptName, id);
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR>");
			prints(sid, "<A HREF=%s/bookmarks/list?pid=%d onClick=\"location.replace('%s/bookmarks/list?pid=%d');return false;\">", ScriptName, id, ScriptName, id);
			prints(sid, "<IMG ALIGN=TOP BORDER=0 SRC=/images/file-folder.png HEIGHT=16 WIDTH=16>&nbsp;");
			prints(sid, "%s</A>&nbsp;</NOBR></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, i, "name")));
		}
		numfolders++;
	}
	ldir_freeresult(&qobj);
	/* list urls */
//	if ((priv&A_ADMIN)) {
//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND obj_did = %d ORDER BY bookmarkname ASC", pid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT id, name, data FROM nullgs_entries WHERE class = 'bookmark' AND pid = %d AND did = %d ORDER BY name ASC", pid, sid->dat->did)<0) return;
//		if ((qobj=ldir_getlist(sid->N, "bookmark", pid, sid->dat->did))==NULL) return;
//	} else {
//		if (sql_queryf(proc->N, &qobj, "SELECT bookmarkid, bookmarkname, bookmarkurl FROM gw_bookmarks WHERE folderid = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY bookmarkname ASC", pid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
//		if (sql_queryf(proc->N, &qobj, "SELECT id, name, data FROM nullgs_entries WHERE class = 'bookmark' AND pid = %d AND did = %d ORDER BY name ASC", pid, sid->dat->did)<0) return;
		if ((qobj=ldir_getlist(sid->N, "bookmark", pid, sid->dat->did))==NULL) return;
//	}
	if (ldir_numentries(&qobj)>0) {
		if (numfolders==0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>");
			if ((priv&A_MODIFY)) prints(sid, "<TH STYLE='border-style:solid'>&nbsp;</TH>");
			prints(sid, "<TH ALIGN=left WIDTH=250 STYLE='border-style:solid'>Name</TH></TR>\r\n");
		}
		for (i=0;i<ldir_numentries(&qobj);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			if ((priv&A_MODIFY)) {
				id=atoi(ldir_getval(&qobj, i, "id"));
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/edit?id=%d>edit</A>&nbsp;</TD>", ScriptName, id);
			}
			prints(sid, "<TD NOWRAP WIDTH=300 STYLE='border-style:solid'><NOBR><A HREF=\"%s\" TARGET=_blank>", ldir_getval(&qobj, i, "labeleduri"));
			prints(sid, "%s</A></NOBR></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, i, "cn")));
		}
	}
	if ((numfolders>0)||(ldir_numentries(&qobj)>0)) {
		prints(sid, "</TABLE>\r\n");
	} else {
		prints(sid, "There are no saved bookmarks<BR />\r\n");
	}
	ldir_freeresult(&qobj);
	prints(sid, "</CENTER><BR />\r\n");
	return;
}

void bookmarkssave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int id, pid;
	int priv=auth_priv(sid, "bookmarks");

	if (!(priv&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_bookmark(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (priv&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) bookmark.obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) bookmark.obj_gid=atoi(ptemp);
//	}
//	if ((priv&A_ADMIN)||(bookmark.obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) bookmark.obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) bookmark.obj_operm=atoi(ptemp);
//	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	if ((ptemp=getpostenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	nes_setnum(sid->N, tobj, "pid", pid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	if ((ptemp=getpostenv(sid, "BOOKMARKNAME"))!=NULL) nes_setstr(sid->N, tobj, "cn", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "BOOKMARKURL"))!=NULL) nes_setstr(sid->N, tobj, "labeleduri", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(priv&A_DELETE)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (ldir_deleteentry("bookmark", id, sid->dat->did)<0) goto cleanup;
		prints(sid, "<CENTER>Bookmark %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "bookmarks", id, "delete", "%s - %s deleted bookmark %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else if (id==0) {
		if (!(priv&A_INSERT)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_bookmark(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Bookmark %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "bookmarks", id, "insert", "%s - %s added bookmark %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_bookmark(sid, id, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Bookmark %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "bookmarks", id, "modify", "%s - %s modified bookmark %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/bookmarks/list?pid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, pid);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/bookmarks/list?pid=%d\"></NOSCRIPT>\r\n", ScriptName, pid);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	if (strncmp(RequestURI, "/bookmarks/export", 17)==0) {
		send_header(sid, 0, 200, "1", "text/plain", -1, -1);
		bookmarks_export(sid);
		return 0;
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (strncmp(RequestURI, "/bookmarks/list", 15)==0) {
		bookmarkslist(sid);
	} else {
		htpage_topmenu(sid, "bookmarks");
		prints(sid, "<BR />\r\n");
		if (strncmp(RequestURI, "/bookmarks/folderedit", 21)==0) {
			bookmarkfolderedit(sid);
		} else if (strncmp(RequestURI, "/bookmarks/foldersave", 21)==0) {
			bookmarkfoldersave(sid);
		} else if (strncmp(RequestURI, "/bookmarks/edit", 15)==0) {
			bookmarksedit(sid);
		} else if (strncmp(RequestURI, "/bookmarks/save", 15)==0) {
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

DllExport int mod_init(_PROC *_proc)
{
	obj_t *modobj, *tobj, *cobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	tobj=nes_settable(proc->N, modobj, "bookmarks");
	cobj=nes_setnum(proc->N, tobj, "submenu", 3);
	cobj=nes_setstr(proc->N, tobj, "name",    "BOOKMARKS",       strlen("BOOKMARKS"));
	cobj=nes_setstr(proc->N, tobj, "pic",     "bookmarks.png",   strlen("bookmarks.png"));
	cobj=nes_setstr(proc->N, tobj, "uri",     "/bookmarks/list", strlen("/bookmarks/list"));
	cobj=nes_setstr(proc->N, tobj, "perm",    "bookmarks",       strlen("bookmarks"));
	cobj=nes_setstr(proc->N, tobj, "fn_name", "mod_main",        strlen("mod_main"));
	cobj=nes_setstr(proc->N, tobj, "fn_uri",  "/bookmarks/",     strlen("/bookmarks/"));
	cobj=nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	cobj=nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	cobj=nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
