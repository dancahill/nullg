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
#define SRVMOD_MAIN 1
#include "mod_weblog.h"

void htselect_weblog_groups(CONN *sid, int selected)
{
	obj_t *qobj=NULL;
	int i, id;
	short found=0;

	if ((qobj=ldir_getlist(sid->N, "webloggroup", 0, sid->dat->did))==NULL) return;
	for (i=0;i<ldir_numentries(&qobj);i++) {
		id=atoi(ldir_getval(&qobj, i, "id"));
		if (id==selected) found=1;
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", id, id==selected?" SELECTED":"", str2html(sid, ldir_getval(&qobj, i, "cn")));
	}
	ldir_freeresult(&qobj);
	if (!found) prints(sid, "<OPTION VALUE='0' SELECTED>\r\n");
	return;
}

void htselect_entryfilter(CONN *sid, int selected, char *baseuri)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	obj_t *qobj1=NULL;
	obj_t *qobj2=NULL;
	char *ptemp;
	int groupid;
	int i;
	int j;

	if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		groupid=atoi(ptemp);
	} else {
		groupid=0;
	}
	if (selected<1) {
		selected=sid->dat->uid;
	}
	if (sql_queryf(proc->N, &qobj1, "SELECT id, name FROM nullgs_entries WHERE class = 'person' AND did = %d ORDER BY name ASC", sid->dat->did)<0) return;
//	if (sql_queryf(proc->N, &qobj2, "SELECT id, name FROM nullgs_entries WHERE class = 'webloggroup' AND did = %d ORDER BY name ASC", sid->dat->did)<0) return;
	if ((qobj2=ldir_getlist(sid->N, "webloggroup", 0, sid->dat->did))==NULL) return;
	prints(sid, "<FORM METHOD=GET NAME=entryfilter ACTION=%s%s>\r\n", ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location.replace(document.entryfilter.userid.options[document.entryfilter.userid.selectedIndex].value);\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", ScriptName, baseuri, atoi(sql_getvalue(proc->N, &qobj1, i, 0)));
		prints(sid, "&groupid=%d\"%s>%s');\r\n", groupid, atoi(sql_getvalue(proc->N, &qobj1, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj1, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location.replace(document.entryfilter.groupid.options[document.entryfilter.groupid.selectedIndex].value);\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=groupid onChange=\"go2()\">');\r\n");
	prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", ScriptName, baseuri, selected);
	prints(sid, "&groupid=0\"%s>All Groups');\r\n", groupid==0?" SELECTED":"");
	for (i=0;i<ldir_numentries(&qobj2);i++) {
		j=atoi(ldir_getval(&qobj2, i, "id"));
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", ScriptName, baseuri, selected);
		prints(sid, "&groupid=%d\"%s>%s');\r\n", j, j==groupid?" SELECTED":"", str2html(sid, ldir_getval(&qobj2, i, "cn")));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	prints(sid, "<OPTION VALUE='0'>All Users\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		j=atoi(sql_getvalue(proc->N, &qobj1, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(proc->N, &qobj1, i, 1)));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=groupid>\r\n");
	prints(sid, "<OPTION VALUE='0'>All Groups\r\n");
	for (i=0;i<ldir_numentries(&qobj2);i++) {
		j=atoi(ldir_getval(&qobj2, i, "id"));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, ldir_getval(&qobj2, i, "cn")));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(proc->N, &qobj1);
	ldir_freeresult(&qobj2);
	return;
}

void weblog_comment_edit(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
//	REC_BLOG_COMMENT *comment=NULL;
//	REC_BLOG_ENTRY *entry=NULL;
	obj_t *qobj=NULL;
//	int editperms=0;
	int commentid=0;
	int parentid=0;
	int entryid=0;
	char *ptemp;
	int ltime;

	if ((ptemp=getgetenv(sid, "COMMENTID"))!=NULL) commentid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "PARENTID"))!=NULL) parentid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "ENTRYID"))!=NULL) entryid=atoi(ptemp);
	if (entryid<1) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (parentid>0) {
//		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 1, parentid, entryid, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		// show the parent and free it
		prints(sid, "<BR />\r\n<CENTER>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
		prints(sid, "%s", str2html(sid, ldir_getval(&qobj, 0, "subject")));
//		if (comment->obj_uid==sid->dat->uid) {
			prints(sid, " [<A HREF=%s/weblog/comments/edit?entryid=%d&commentid=%d onClick=\"location.replace('%s/weblog/comments/edit?entryid=%d&commentid=%d');return false;\">Edit</A>]", ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")), atoi(ldir_getval(&qobj, 0, "commentid")), ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")), atoi(ldir_getval(&qobj, 0, "commentid")));
//		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=time_sql2unix(ldir_getval(&qobj, 0, "ctime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=time_sql2unix(ldir_getval(&qobj, 0, "mtime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "<BR />Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR />");
		printline2(sid, 1, ldir_getval(&qobj, 0, "message"));
		prints(sid, "\r\n<BR /><BR />\r\n");
		prints(sid, "</TD></TR></TABLE><P>\r\n");
		prints(sid, "</CENTER>\r\n");
//		if (comment!=NULL) { free(comment); comment=NULL; }
	} else if (commentid==0) {
//		if ((entry=calloc(1, sizeof(REC_BLOG_ENTRY)))==NULL) return;
		if (dbread_weblog_entry(sid, 1, entryid, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		// show the entry and free it
		prints(sid, "<BR />\r\n<CENTER>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
		prints(sid, "%s", str2html(sid, ldir_getval(&qobj, 0, "subject")));
//		if (entry->obj_uid==sid->dat->uid) {
			prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")), ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")));
//		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=time_sql2unix(ldir_getval(&qobj, 0, "ctime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=time_sql2unix(ldir_getval(&qobj, 0, "mtime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "<BR />Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR />");
		printline2(sid, 1, ldir_getval(&qobj, 0, "summary"));
		prints(sid, "<BR /><BR /><HR><BR />");
		printline2(sid, 1, ldir_getval(&qobj, 0, "message"));
		prints(sid, "\r\n<BR /><BR />\r\n");
		prints(sid, "</TD></TR></TABLE><P>\r\n");
		prints(sid, "</CENTER>\r\n");
//		if (entry!=NULL) { free(entry); entry=NULL; }
	}
	if (qobj) sql_freeresult(proc->N, &qobj);
	if (commentid>0) {
//		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 2, commentid, entryid, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", entryid);
			goto cleanup;
		}
	} else {
//		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 1, 0, entryid, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
//		if (parentid) comment->referenceid=parentid;
	}
	prints(sid, "<script src=\"/scripts/spellChecker.js\"></script>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function openSpellChecker1() {\r\n");
	prints(sid, "	var message = document.commentedit.message;\r\n");
	prints(sid, "	var speller = new spellChecker( message );\r\n");
	prints(sid, "	speller.openChecker();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function checkSize1() {\r\n");
	prints(sid, "	document.getElementById('size1').innerHTML=document.commentedit.message.value.length+' characters';\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 2);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (commentid>0) {
		prints(sid, "<BR /><B>Comment Number %d - ", atoi(ldir_getval(&qobj, 0, "commentid")));
		prints(sid, "<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Log %d</A></B>\r\n", ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")), ScriptName, atoi(ldir_getval(&qobj, 0, "entryid")), atoi(ldir_getval(&qobj, 0, "entryid")));
	} else {
		prints(sid, "<B>New Log Comment</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=80%%>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/weblog/comments/save NAME=commentedit>\r\n", ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=commentid VALUE='%d'>\r\n", atoi(ldir_getval(&qobj, 0, "commentid")));
	prints(sid, "<INPUT TYPE=hidden NAME=entryid VALUE='%d'>\r\n", atoi(ldir_getval(&qobj, 0, "entryid")));
	prints(sid, "<INPUT TYPE=hidden NAME=referenceid VALUE='%d'>\r\n", atoi(ldir_getval(&qobj, 0, "referenceid")));
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>COMMENT</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Subject&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject value=\"%s\" SIZE=45 STYLE='width:300px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "subject")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Comment&nbsp;</B><span id=\"size1\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker1();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=18 COLS=79 STYLE='width:100%%' onchange=\"checkSize1();\" onkeypress=\"checkSize1();\">");
	printline2(sid, 0, ldir_getval(&qobj, 0, "message"));
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*
	if ((comment->obj_uid==sid->dat->uid)||(auth_priv(sid, "admin")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, comment->obj_uid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "weblog"), comment->obj_gid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", comment->obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", comment->obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", comment->obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", comment->obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", comment->obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", comment->obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
*/
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (commentid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
		if (auth_priv(sid, "admin")&A_DELETE) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
		}
		prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Create'>\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<BR />\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "checkSize1();\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.commentedit.message.focus();\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
cleanup:
//	if (comment!=NULL) { free(comment); comment=NULL; }
//	if (entry!=NULL) { free(entry); entry=NULL; }
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

void weblog_comment_save(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	obj_t *qobj=NULL, *tobj;
	char *ptemp;
	int commentid;
	int id;
	int pid=0;

	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "COMMENTID"))==NULL) goto cleanup;
	commentid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ENTRYID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_weblog_comment(sid, 2, commentid, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (auth_priv(sid, "admin")&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) comment->obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) comment->obj_gid=atoi(ptemp);
//	}
//	if ((auth_priv(sid, "admin")&A_ADMIN)||(comment->obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) comment->obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) comment->obj_operm=atoi(ptemp);
//	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	if ((ptemp=getpostenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	nes_setnum(sid->N, tobj, "pid", pid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		return;
	}
	if ((ptemp=getpostenv(sid, "ENTRYID"))!=NULL) nes_setnum(sid->N, tobj, "id", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "REFERENCEID"))!=NULL) nes_setnum(sid->N, tobj, "reference", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL) nes_setstr(sid->N, tobj, "subject", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) nes_setstr(sid->N, tobj, "message", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
//		if (sql_updatef(proc->N, "DELETE FROM gw_weblog_comments WHERE commentid = %d", comment->commentid)<0) goto cleanup;
		if (ldir_deleteentry("weblogcomment", id, sid->dat->did)<0) return;
		prints(sid, "<BR /><CENTER>Log comment %d deleted successfully</CENTER><BR />\r\n", id);
//		db_log_activity(sid, "weblogs", comment->commentid, "delete", "%s - %s deleted comment %d", sid->dat->in_RemoteAddr, sid->dat->username, comment->commentid);
//		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/list\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
//		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/list\">\r\n</NOSCRIPT>\r\n", ScriptName);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", ScriptName, id);
	} else if (id==0) {
		if ((id=dbwrite_weblog_comment(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>Log comment %d added successfully</CENTER><BR />\r\n", id);
//		db_log_activity(sid, "weblogs", comment->commentid, "insert", "%s - %s added comment %d", sid->dat->in_RemoteAddr, sid->dat->username, comment->commentid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", ScriptName, id);
	} else {
		if (dbwrite_weblog_comment(sid, commentid, &qobj)<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>Log comment %d modified successfully</CENTER><BR />\r\n", id);
//		db_log_activity(sid, "weblogs", comment->commentid, "modify", "%s - %s modified comment %d", sid->dat->in_RemoteAddr, sid->dat->username, comment->commentid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", ScriptName, id);
	}
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
//	if (comment!=NULL) { free(comment); comment=NULL; }
	return;
}

void weblog_entry_edit(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
//	REC_BLOG_ENTRY *entry=calloc(1, sizeof(REC_BLOG_ENTRY));
	obj_t *qobj=NULL;
//	int editperms=0;
	int id;
	char *ptemp;

	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/weblog/editnew", 15)==0) {
		id=0;
		if (dbread_weblog_entry(sid, 2, 0, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
	} else {
		if ((ptemp=getgetenv(sid, "ENTRYID"))==NULL) return;
		id=atoi(ptemp);
		if (dbread_weblog_entry(sid, 2, id, &qobj)!=0) {
			prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", id);
			goto cleanup;
		}
	}
	prints(sid, "<BR />\r\n");
	prints(sid, "<script src=\"/scripts/spellChecker.js\"></script>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function openSpellChecker1() {\r\n");
	prints(sid, "	var summary = document.entryedit.summary;\r\n");
	prints(sid, "	var speller = new spellChecker( summary );\r\n");
	prints(sid, "	speller.openChecker();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function openSpellChecker2() {\r\n");
	prints(sid, "	var message = document.entryedit.message;\r\n");
	prints(sid, "	var speller = new spellChecker( message );\r\n");
	prints(sid, "	speller.openChecker();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function checkSize1() {\r\n");
	prints(sid, "	document.getElementById('size1').innerHTML=document.entryedit.summary.value.length+' characters';\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function checkSize2() {\r\n");
	prints(sid, "	document.getElementById('size2').innerHTML=document.entryedit.message.value.length+' characters';\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 3);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (id>0) {
		prints(sid, "<B><A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Entry Number %d</A></B>\r\n", ScriptName, id, ScriptName, id, id);
	} else {
		prints(sid, "<B>New Log Entry</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=80%%>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/weblog/save NAME=entryedit>\r\n", ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=entryid VALUE='%d'>\r\n", id);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>MESSAGE</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Subject&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject value=\"%s\" SIZE=45 STYLE='width:300px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "subject")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=pid style='width:300px'>\r\n");
	htselect_weblog_groups(sid, atoi(ldir_getval(&qobj, 0, "pid")));
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Summary&nbsp;</B><span id=\"size1\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker1();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=summary ROWS=20 COLS=79 STYLE='width:100%%' onchange=\"checkSize1();\" onkeypress=\"checkSize1();\">");
	printline2(sid, 0, ldir_getval(&qobj, 0, "summary"));
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Message&nbsp;</B><span id=\"size2\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker2();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=23 COLS=79 STYLE='width:100%%' onchange=\"checkSize2();\" onkeypress=\"checkSize2();\">");
	printline2(sid, 0, ldir_getval(&qobj, 0, "message"));
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*
	if ((entry->obj_uid==sid->dat->uid)||(auth_priv(sid, "admin")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, entry->obj_uid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "weblog"), entry->obj_gid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", entry->obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", entry->obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", entry->obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", entry->obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", entry->obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", entry->obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
*/
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (id!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
		if (auth_priv(sid, "admin")&A_DELETE) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
		}
		prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	} else {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Create'>\r\n");
	}
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "checkSize1();\r\n");
	prints(sid, "checkSize2();\r\n");
	prints(sid, "showpage(1);\r\n");
	if (id==0) {
		prints(sid, "document.entryedit.summary.focus();\r\n");
	} else {
		prints(sid, "document.entryedit.subject.focus();\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
//	if (entry!=NULL) { free(entry); entry=NULL; }
	return;
}

void weblog_entry_subview(CONN *sid, obj_t **qobj, int parentid, int depth)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	int i, j;
	int ltime;
	int f=0;

	for (i=0;i<sql_numtuples(proc->N, qobj);i++) {
		if (atoi(sql_getvalue(proc->N, qobj, i, 9))!=parentid) continue;
		if ((!f)&&(depth==0)) {
			prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=90%%>\r\n");
		}
		if ((f)&&(depth==0)) {
			prints(sid, "<TR><TD NOWRAP WIDTH=100%% COLSPAN=2>&nbsp;</TD></TR>\r\n");
		}
//		if (!f) {
			prints(sid, "<TR><TD><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD NOWRAP>");
			if (depth>0) for (j=0;j<depth;j++) prints(sid, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
			prints(sid, "</TD><TD NOWRAP WIDTH=100%%>\r\n");
//		}
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>");
		prints(sid, "&nbsp;%s<BR />", str2html(sid, sql_getvalue(proc->N, qobj, i, 10)));
		prints(sid, "&nbsp;By %d", atoi(sql_getvalue(proc->N, qobj, i, 3)));
		if (atoi(sql_getvalue(proc->N, qobj, i, 3))==sid->dat->uid) {
			prints(sid, " [<A HREF=%s/weblog/comments/edit?entryid=%d&commentid=%d onClick=\"location.replace('%s/weblog/comments/edit?entryid=%d&commentid=%d');return false;\">Edit</A>]", ScriptName, atoi(sql_getvalue(proc->N, qobj, i, 8)), atoi(sql_getvalue(proc->N, qobj, i, 0)), ScriptName, atoi(sql_getvalue(proc->N, qobj, i, 8)), atoi(sql_getvalue(proc->N, qobj, i, 0)));
		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=time_sql2unix(ldir_getval(qobj, i, "ctime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=time_sql2unix(ldir_getval(qobj, i, "mtime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "<BR />%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR />");
		printline2(sid, 1, sql_getvalue(proc->N, qobj, i, 11));
		prints(sid, "<BR /><BR />[<A HREF=%s/weblog/comments/editnew?entryid=%d&parentid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d&parentid=%d');return false;\">Reply</A>]\r\n", ScriptName, atoi(sql_getvalue(proc->N, qobj, i, 8)), atoi(sql_getvalue(proc->N, qobj, i, 0)), ScriptName, atoi(sql_getvalue(proc->N, qobj, i, 8)), atoi(sql_getvalue(proc->N, qobj, i, 0)));
		prints(sid, "</TD></TR></TABLE>");
		prints(sid, "</TD></TR></TABLE>\r\n");
		prints(sid, "</TD></TR>\r\n");
		weblog_entry_subview(sid, qobj, atoi(sql_getvalue(proc->N, qobj, i, 0)), depth+1);
		if (!f) {
			f=1;
		}
	}
	if ((f)&&(depth==0)) {
		prints(sid, "</TABLE>\r\n\r\n");
	}
	return;
}

void weblog_entry_view(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
//	REC_BLOG_ENTRY *entry=calloc(1, sizeof(REC_BLOG_ENTRY));
	obj_t *qobj=NULL;
	char *ptemp;
	int id;
	int ltime;

	if ((ptemp=getgetenv(sid, "ENTRYID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_weblog_entry(sid, 1, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", id);
		goto cleanup;
	}
	prints(sid, "<BR />\r\n<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
	prints(sid, "%s", str2html(sid, ldir_getval(&qobj, 0, "subject")));
//	if (entry->obj_uid==sid->dat->uid) {
		prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", ScriptName, id, ScriptName, id);
//	}
	prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
	ltime=time_sql2unix(ldir_getval(&qobj, 0, "ctime"));
	ltime+=time_tzoffset(sid, ltime);
	prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
	ltime=time_sql2unix(ldir_getval(&qobj, 0, "mtime"));
	ltime+=time_tzoffset(sid, ltime);
	prints(sid, "<BR />Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
	prints(sid, "</TH></TR></TABLE>");
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR />");
	printline2(sid, 1, ldir_getval(&qobj, 0, "summary"));
	prints(sid, "<BR /><BR /><HR><BR />");
	printline2(sid, 1, ldir_getval(&qobj, 0, "message"));
	prints(sid, "\r\n<BR /><BR />\r\n");
	prints(sid, "[<A HREF=%s/weblog/comments/editnew?entryid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d');return false;\">Reply</A>]\r\n", ScriptName, id, ScriptName, id);
	prints(sid, "</TD></TR></TABLE>\r\n");
	if (dblist_weblog_comments(sid, &qobj, id)<0) return;
	prints(sid, "%d comments<BR />\r\n", sql_numtuples(proc->N, &qobj));
	weblog_entry_subview(sid, &qobj, 0, 0);
	prints(sid, "</CENTER><BR />\r\n");
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
//	if (entry!=NULL) { free(entry); entry=NULL; }
}

void weblog_entry_list(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj1=NULL, *qobj2=NULL, *qobj3=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int i, j;
	int offset=0;
	int total;
	int userid=sid->dat->uid;
	int pid=0;
	int ltime;
	int id;
	int max;

	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) userid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) pid=atoi(ptemp);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_entryfilter(sid, userid, "/weblog/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER><BR />\r\n");
	if ((qobj1=ldir_getlist(sid->N, "weblogentry", pid, sid->dat->did))==NULL) return;
//	if (dblist_weblog_entries(sid, &qobj1, userid, pid)<0) return;
	total=ldir_numentries(&qobj1);
	if (total<1) {
		prints(sid, "<B>No entries found</B></CENTER>\r\n");
		if (qobj1) ldir_freeresult(&qobj1);
		return;
	}
	prints(sid, "Listing %d log entries<BR />\r\n", total);
	max=ldir_numentries(&qobj1);
	if (max>offset+sid->dat->maxlist) max=offset+sid->dat->maxlist;
//	for (i=offset;i<max;i++) {
//	if (sql_queryf(&sqr2, "SELECT groupid, name FROM gw_weblog_groups WHERE obj_did = %d", sid->dat->user_did)<0) return;
	if ((qobj2=ldir_getlist(sid->N, "webloggroup", pid, sid->dat->did))==NULL) return;
	for (i=max-1;i>=offset;i--) {
		id=atoi(ldir_getval(&qobj1, i, "id"));
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
		for (j=0;j<ldir_numentries(&qobj2);j++) {
			if ((pid<1)&&(atoi(ldir_getval(&qobj1, i, "pid"))==atoi(ldir_getval(&qobj2, j, "id")))) {
				prints(sid, "[%s] - ", ldir_getval(&qobj2, j, "cn"));
			}
		}
		prints(sid, "<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">%s</A>", ScriptName, id, ScriptName, id, str2html(sid, ldir_getval(&qobj1, i, "subject")));
		if (sql_queryf(proc->N, &qobj3, "SELECT count(*) FROM nullgs_entries WHERE class = 'weblogcomment' AND pid = %d AND did = %d", id, sid->dat->did)<0) return;
		prints(sid, " (%d comments)", atoi(sql_getvalue(proc->N, &qobj3, 0, 0)));
		sql_freeresult(proc->N, &qobj3);
//		if (atoi(ldir_getval(&qobj1, i, "uid"))==sid->dat->uid) {
			prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", ScriptName, id, ScriptName, id);
//		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=time_sql2unix(ldir_getval(&qobj1, i, "ctime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=time_sql2unix(ldir_getval(&qobj1, i, "mtime"));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "<BR />%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR />");
		ptemp=ldir_getval(&qobj1, i, "summary");
		if (ptemp) printline2(sid, 1, ptemp);
		else prints(sid, "NULL POINTER");
		prints(sid, "\r\n<BR /><BR />%d bytes in body<BR />", strlen(ldir_getval(&qobj1, i, "message")));
		prints(sid, "[<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Read More</A>]\r\n", ScriptName, id, ScriptName, id);
		prints(sid, "[<A HREF=%s/weblog/comments/editnew?entryid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d');return false;\">Reply</A>]\r\n", ScriptName, id, ScriptName, id);
		prints(sid, "</TD></TR></TABLE><P>\r\n");
	}
	ldir_freeresult(&qobj2);
	if (ldir_numentries(&qobj1)>sid->dat->maxlist) {
		if (offset>sid->dat->maxlist-1) {
			prints(sid, "[<A HREF=%s/weblog/list?offset=%d&userid=%d>Previous Page</A>]\r\n", ScriptName, offset-sid->dat->maxlist, userid);
		} else {
			prints(sid, "[Previous Page]\r\n");
		}
		if (offset+sid->dat->maxlist<ldir_numentries(&qobj1)) {
			prints(sid, "[<A HREF=%s/weblog/list?offset=%d&userid=%d>Next Page</A>]\r\n", ScriptName, offset+sid->dat->maxlist, userid);
		} else {
			prints(sid, "[Next Page]\r\n");
		}
	}
	prints(sid, "</CENTER>\r\n");
	if (qobj1) ldir_freeresult(&qobj1);
	return;
}

void weblog_entry_save(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int id, pid;
//	int priv=auth_priv(sid, "bookmarks");

	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ENTRYID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_weblog_entry(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (auth_priv(sid, "admin")&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) entry->obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) entry->obj_gid=atoi(ptemp);
//	}
//	if ((auth_priv(sid, "admin")&A_ADMIN)||(entry->obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) entry->obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) entry->obj_operm=atoi(ptemp);
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
//	if ((ptemp=getpostenv(sid, "PID"))!=NULL) nes_setnum(sid->N, tobj, "pid", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL) nes_setstr(sid->N, tobj, "subject", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "SUMMARY"))!=NULL) nes_setstr(sid->N, tobj, "summary", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) nes_setstr(sid->N, tobj, "message", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (ldir_deleteentry("weblogentry", id, sid->dat->did)<0) goto cleanup;
		prints(sid, "<BR /><CENTER>Log entry %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "weblogs", id, "delete", "%s - %s deleted entry %d", sid->socket.RemoteAddr, sid->dat->username, id);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/list\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/list\"></NOSCRIPT>\r\n", ScriptName);
		goto cleanup;
	} else if (id==0) {
		if ((id=dbwrite_weblog_entry(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>Log entry %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "weblogs", id, "insert", "%s - %s added entry %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else {
		if ((id=dbwrite_weblog_entry(sid, id, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>Log entry %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "weblogs", id, "modify", "%s - %s modified entry %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\"></NOSCRIPT>\r\n", ScriptName, id);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

DllExport int mod_main(CONN *sid)
{
	char *RequestURI=nes_getstr(sid->N, nes_settable(sid->N, &sid->N->g, "_SERVER"), "REQUEST_URI");

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, "weblog");
	if (strncmp(RequestURI, "/weblog/comments/edit", 21)==0) {
		weblog_comment_edit(sid);
	} else if (strncmp(RequestURI, "/weblog/comments/save", 21)==0) {
		weblog_comment_save(sid);
	} else if (strncmp(RequestURI, "/weblog/edit", 12)==0) {
		weblog_entry_edit(sid);
	} else if (strncmp(RequestURI, "/weblog/view", 12)==0) {
		weblog_entry_view(sid);
	} else if (strncmp(RequestURI, "/weblog/list", 12)==0) {
		weblog_entry_list(sid);
	} else if (strncmp(RequestURI, "/weblog/save", 12)==0) {
		weblog_entry_save(sid);
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
	obj_t *tobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "weblog");
	nes_exec(proc->N, "GWMODULES.weblog={submenu=1,name='WEB LOG',pic='default.png',uri='/weblog/list',fn_name='mod_main',fn_uri='/weblog/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
