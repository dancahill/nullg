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
#include "mod_weblog.h"

void htselect_weblog_groups(CONN *sid, int selected)
{
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT groupid, name FROM gw_weblog_groups ORDER BY name ASC")<0) return;
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

void htselect_entryfilter(CONN *sid, int selected, char *baseuri)
{
	SQLRES sqr1;
	SQLRES sqr2;
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
		selected=sid->dat->user_uid;
	}
	if (sql_queryf(&sqr1, "SELECT userid, username FROM gw_users WHERE domainid = %d order by username ASC", sid->dat->user_did)<0) return;
	if (sql_queryf(&sqr2, "SELECT groupid, name FROM gw_weblog_groups WHERE obj_did = %d ORDER BY name ASC", sid->dat->user_did)<0) return;
	prints(sid, "<FORM METHOD=GET NAME=entryfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location.replace(document.entryfilter.userid.options[document.entryfilter.userid.selectedIndex].value);\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "&groupid=%d\"%s>%s');\r\n", groupid, atoi(sql_getvalue(&sqr1, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr1, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location.replace(document.entryfilter.groupid.options[document.entryfilter.groupid.selectedIndex].value);\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=groupid onChange=\"go2()\">');\r\n");
	prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
	prints(sid, "&groupid=0\"%s>All Groups');\r\n", groupid==0?" SELECTED":"");
	for (i=0;i<sql_numtuples(&sqr2);i++) {
		j=atoi(sql_getvalue(&sqr2, i, 0));
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d", sid->dat->in_ScriptName, baseuri, selected);
		prints(sid, "&groupid=%d\"%s>%s');\r\n", j, j==groupid?" SELECTED":"", str2html(sid, sql_getvalue(&sqr2, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	prints(sid, "<OPTION VALUE='0'>All Users\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		j=atoi(sql_getvalue(&sqr1, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr1, i, 1)));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=groupid>\r\n");
	prints(sid, "<OPTION VALUE='0'>All Groups\r\n");
	for (i=0;i<sql_numtuples(&sqr2);i++) {
		j=atoi(sql_getvalue(&sqr2, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr2, i, 1)));
	}
	prints(sid, "</SELECT>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(&sqr1);
	sql_freeresult(&sqr2);
	return;
}

void weblog_comment_edit(CONN *sid)
{
	REC_BLOG_COMMENT *comment=NULL;
	REC_BLOG_ENTRY *entry=NULL;
	int editperms=0;
	int commentid=0;
	int parentid=0;
	int entryid=0;
	char *ptemp;
	time_t ltime;

	if ((ptemp=getgetenv(sid, "COMMENTID"))!=NULL) commentid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "PARENTID"))!=NULL) parentid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "ENTRYID"))!=NULL) entryid=atoi(ptemp);
	if (entryid<1) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		goto cleanup;
	}
	if (parentid>0) {
		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 1, parentid, entryid, comment)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		// show the parent and free it
		prints(sid, "<BR>\r\n<CENTER>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
		prints(sid, "%s", str2html(sid, comment->subject));
		if (comment->obj_uid==sid->dat->user_uid) {
			prints(sid, " [<A HREF=%s/weblog/comments/edit?entryid=%d&commentid=%d onClick=\"location.replace('%s/weblog/comments/edit?entryid=%d&commentid=%d');return false;\">Edit</A>]", sid->dat->in_ScriptName, comment->entryid, comment->commentid, sid->dat->in_ScriptName, comment->entryid, comment->commentid);
		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=comment->obj_ctime+time_tzoffset(sid, comment->obj_ctime);
		prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=comment->obj_mtime+time_tzoffset(sid, comment->obj_mtime);
		prints(sid, "<BR>Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR>");
		printline2(sid, 1, comment->message);
		prints(sid, "\r\n<BR><BR>\r\n");
		prints(sid, "</TD></TR></TABLE><P>\r\n");
		prints(sid, "</CENTER>\r\n");
		if (comment!=NULL) { free(comment); comment=NULL; }
	} else if (commentid==0) {
		if ((entry=calloc(1, sizeof(REC_BLOG_ENTRY)))==NULL) return;
		if (dbread_weblog_entry(sid, 1, entryid, entry)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		// show the entry and free it
		prints(sid, "<BR>\r\n<CENTER>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=80%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
		prints(sid, "%s", str2html(sid, entry->subject));
		if (entry->obj_uid==sid->dat->user_uid) {
			prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", sid->dat->in_ScriptName, entry->entryid, sid->dat->in_ScriptName, entry->entryid);
		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=entry->obj_ctime+time_tzoffset(sid, entry->obj_ctime);
		prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=entry->obj_mtime+time_tzoffset(sid, entry->obj_mtime);
		prints(sid, "<BR>Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR>");
		printline2(sid, 1, entry->summary);
		prints(sid, "<BR><BR><HR><BR>");
		printline2(sid, 1, entry->message);
		prints(sid, "\r\n<BR><BR>\r\n");
		prints(sid, "</TD></TR></TABLE><P>\r\n");
		prints(sid, "</CENTER>\r\n");
		if (entry!=NULL) { free(entry); entry=NULL; }
	}
	if (commentid>0) {
		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 2, commentid, entryid, comment)!=0) {
			prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\r\n", entryid);
			goto cleanup;
		}
	} else {
		if ((comment=calloc(1, sizeof(REC_BLOG_COMMENT)))==NULL) return;
		if (dbread_weblog_comment(sid, 1, 0, entryid, comment)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		if (parentid) comment->referenceid=parentid;
	}
	prints(sid, "<script src=\"/groupware/js/spellcheck/spellChecker.js\"></script>\r\n");
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
		prints(sid, "<BR><B>Comment Number %d - ", comment->commentid);
		prints(sid, "<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Log %d</A></B>\r\n", sid->dat->in_ScriptName, comment->entryid, sid->dat->in_ScriptName, comment->entryid, comment->entryid);
	} else {
		prints(sid, "<B>New Log Comment</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=80%%>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/weblog/comments/save NAME=commentedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=commentid VALUE='%d'>\r\n", comment->commentid);
	prints(sid, "<INPUT TYPE=hidden NAME=entryid VALUE='%d'>\r\n", comment->entryid);
	prints(sid, "<INPUT TYPE=hidden NAME=referenceid VALUE='%d'>\r\n", comment->referenceid);
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
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject value=\"%s\" SIZE=45 STYLE='width:300px'></TD></TR>\r\n", str2html(sid, comment->subject));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Comment&nbsp;</B><span id=\"size1\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker1();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=18 COLS=79 STYLE='width:100%%' onchange=\"checkSize1();\" onkeypress=\"checkSize1();\">");
	printline2(sid, 0, comment->message);
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((comment->obj_uid==sid->dat->user_uid)||(auth_priv(sid, "admin")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, comment->obj_uid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "weblog"), comment->obj_gid, sid->dat->user_did);
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
	prints(sid, "<BR>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "checkSize1();\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.commentedit.message.focus();\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
cleanup:
	if (comment!=NULL) { free(comment); comment=NULL; }
	if (entry!=NULL) { free(entry); entry=NULL; }
	return;
}

void weblog_comment_save(CONN *sid)
{
	REC_BLOG_COMMENT *comment=calloc(1, sizeof(REC_BLOG_COMMENT));
	char *ptemp;
	int commentid;
	int entryid;

	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "COMMENTID"))==NULL) goto cleanup;
	commentid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ENTRYID"))==NULL) goto cleanup;
	entryid=atoi(ptemp);
	if (dbread_weblog_comment(sid, 2, commentid, entryid, comment)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		goto cleanup;
	}
	if (auth_priv(sid, "admin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) comment->obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) comment->obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "admin")&A_ADMIN)||(comment->obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) comment->obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) comment->obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ENTRYID"))!=NULL) comment->entryid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "REFERENCEID"))!=NULL) comment->referenceid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL) snprintf(comment->subject, sizeof(comment->subject)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) snprintf(comment->message, sizeof(comment->message)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_weblog_comments WHERE commentid = %d", comment->commentid)<0) goto cleanup;
		prints(sid, "<BR><CENTER>Log comment %d deleted successfully</CENTER><BR>\r\n", comment->commentid);
		db_log_activity(sid, "weblogs", comment->commentid, "delete", "%s - %s deleted comment %d", sid->dat->in_RemoteAddr, sid->dat->user_username, comment->commentid);
//		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/list\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
//		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/list\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
	} else if (comment->commentid==0) {
		if ((comment->commentid=dbwrite_weblog_comment(sid, 0, comment))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		prints(sid, "<BR><CENTER>Log comment %d added successfully</CENTER><BR>\r\n", comment->commentid);
		db_log_activity(sid, "weblogs", comment->commentid, "insert", "%s - %s added comment %d", sid->dat->in_RemoteAddr, sid->dat->user_username, comment->commentid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
	} else {
		if (dbwrite_weblog_comment(sid, commentid, comment)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		prints(sid, "<BR><CENTER>Log comment %d modified successfully</CENTER><BR>\r\n", comment->commentid);
		db_log_activity(sid, "weblogs", comment->commentid, "modify", "%s - %s modified comment %d", sid->dat->in_RemoteAddr, sid->dat->user_username, comment->commentid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, comment->entryid);
	}
cleanup:
	if (comment!=NULL) { free(comment); comment=NULL; }
	return;
}

void weblog_entry_edit(CONN *sid)
{
	REC_BLOG_ENTRY *entry=calloc(1, sizeof(REC_BLOG_ENTRY));
	int editperms=0;
	int entryid;

	if (strncmp(sid->dat->in_RequestURI, "/weblog/editnew", 15)==0) {
		entryid=0;
		if (dbread_weblog_entry(sid, 2, 0, entry)!=0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
	} else {
		if (getgetenv(sid, "ENTRYID")==NULL) goto cleanup;
		entryid=atoi(getgetenv(sid, "ENTRYID"));
		if (dbread_weblog_entry(sid, 2, entryid, entry)!=0) {
			prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\r\n", entryid);
			goto cleanup;
		}
	}
	prints(sid, "<BR>\r\n");
	prints(sid, "<script src=\"/groupware/js/spellcheck/spellChecker.js\"></script>\r\n");
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
	if (entryid>0) {
		prints(sid, "<B><A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Entry Number %d</A></B>\r\n", sid->dat->in_ScriptName, entry->entryid, sid->dat->in_ScriptName, entry->entryid, entry->entryid);
	} else {
		prints(sid, "<B>New Log Entry</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=80%%>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/weblog/save NAME=entryedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=entryid VALUE='%d'>\r\n", entry->entryid);
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
	prints(sid, "<TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject value=\"%s\" SIZE=45 STYLE='width:300px'></TD></TR>\r\n", str2html(sid, entry->subject));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=entrygroupid style='width:300px'>\r\n");
	htselect_weblog_groups(sid, entry->entrygroupid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Summary&nbsp;</B><span id=\"size1\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker1();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=summary ROWS=20 COLS=79 STYLE='width:100%%' onchange=\"checkSize1();\" onkeypress=\"checkSize1();\">");
	printline2(sid, 0, entry->summary);
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Message&nbsp;</B><span id=\"size2\"></span></TD>");
	prints(sid, "<TD ALIGN=RIGHT><A HREF=\"javascript:openSpellChecker2();\">Check Spelling</A></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=23 COLS=79 STYLE='width:100%%' onchange=\"checkSize2();\" onkeypress=\"checkSize2();\">");
	printline2(sid, 0, entry->message);
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((entry->obj_uid==sid->dat->user_uid)||(auth_priv(sid, "admin")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, entry->obj_uid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:300px'%s>\r\n", (auth_priv(sid, "weblog")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "weblog"), entry->obj_gid, sid->dat->user_did);
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
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	if (entryid!=0) {
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
	if (entryid==0) {
		prints(sid, "document.entryedit.summary.focus();\r\n");
	} else {
		prints(sid, "document.entryedit.subject.focus();\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
cleanup:
	if (entry!=NULL) { free(entry); entry=NULL; }
	return;
}

void weblog_entry_subview(CONN *sid, SQLRES *sqr, int parentid, int depth)
{
	int i, j;
	time_t ltime;
	int f=0;

	for (i=0;i<sql_numtuples(sqr);i++) {
		if (atoi(sql_getvalue(sqr, i, 9))!=parentid) continue;
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
		prints(sid, "&nbsp;%s<BR>", str2html(sid, sql_getvalue(sqr, i, 10)));
		prints(sid, "&nbsp;By %d", atoi(sql_getvalue(sqr, i, 3)));
		if (atoi(sql_getvalue(sqr, i, 3))==sid->dat->user_uid) {
			prints(sid, " [<A HREF=%s/weblog/comments/edit?entryid=%d&commentid=%d onClick=\"location.replace('%s/weblog/comments/edit?entryid=%d&commentid=%d');return false;\">Edit</A>]", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 8)), atoi(sql_getvalue(sqr, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 8)), atoi(sql_getvalue(sqr, i, 0)));
		}
		prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
		ltime=time_sql2unix(sql_getvalue(sqr, i, 1));
		ltime+=time_tzoffset(sid, ltime);
		prints(sid, "%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		ltime=time_sql2unix(sql_getvalue(sqr, i, 2));
		ltime+=time_tzoffset(sid, ltime);
//		prints(sid, "<BR>%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
		prints(sid, "</TH></TR></TABLE>");
		prints(sid, "</TH></TR>\r\n");
		prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR>");
		printline2(sid, 1, sql_getvalue(sqr, i, 11));
		prints(sid, "<BR><BR>[<A HREF=%s/weblog/comments/editnew?entryid=%d&parentid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d&parentid=%d');return false;\">Reply</A>]\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 8)), atoi(sql_getvalue(sqr, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 8)), atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "</TD></TR></TABLE>");
		prints(sid, "</TD></TR></TABLE>\r\n");
		prints(sid, "</TD></TR>\r\n");
		weblog_entry_subview(sid, sqr, atoi(sql_getvalue(sqr, i, 0)), depth+1);
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
	REC_BLOG_ENTRY *entry=calloc(1, sizeof(REC_BLOG_ENTRY));
	SQLRES sqr;
	char *ptemp;
	int entryid;
	time_t ltime;

	if ((ptemp=getgetenv(sid, "ENTRYID"))==NULL) goto cleanup;
	entryid=atoi(ptemp);
	if (dbread_weblog_entry(sid, 1, entryid, entry)!=0) {
		prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\r\n", entryid);
		goto cleanup;
	}
	prints(sid, "<BR>\r\n<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
	prints(sid, "%s", str2html(sid, entry->subject));
	if (entry->obj_uid==sid->dat->user_uid) {
		prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", sid->dat->in_ScriptName, entry->entryid, sid->dat->in_ScriptName, entry->entryid);
	}
	prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
	ltime=entry->obj_ctime+time_tzoffset(sid, entry->obj_ctime);
	prints(sid, "Created: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
	ltime=entry->obj_mtime+time_tzoffset(sid, entry->obj_mtime);
	prints(sid, "<BR>Modified: %s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
	prints(sid, "</TH></TR></TABLE>");
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR>");
	printline2(sid, 1, entry->summary);
	prints(sid, "<BR><BR><HR><BR>");
	printline2(sid, 1, entry->message);
	prints(sid, "\r\n<BR><BR>\r\n");
	prints(sid, "[<A HREF=%s/weblog/comments/editnew?entryid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d');return false;\">Reply</A>]\r\n", sid->dat->in_ScriptName, entry->entryid, sid->dat->in_ScriptName, entry->entryid);
	prints(sid, "</TD></TR></TABLE>\r\n");
	if (dblist_weblog_comments(sid, &sqr, entryid)<0) return;
	prints(sid, "%d comments<BR>\r\n", sql_numtuples(&sqr));
	weblog_entry_subview(sid, &sqr, 0, 0);
	sql_freeresult(&sqr);
	prints(sid, "</CENTER><BR>\r\n");
cleanup:
	if (entry!=NULL) { free(entry); entry=NULL; }
}

void weblog_entry_list(CONN *sid)
{
	char *ptemp;
	int i, j;
	int offset=0;
	SQLRES sqr1;
	SQLRES sqr2;
	SQLRES sqr3;
	int total;
	int userid=-1;
	int entrygroupid=-1;
	time_t ltime;

	if (getgetenv(sid, "OFFSET")!=NULL) {
		offset=atoi(getgetenv(sid, "OFFSET"));
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) {
		entrygroupid=atoi(ptemp);
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_entryfilter(sid, userid, "/weblog/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER><BR>\r\n");
	if (dblist_weblog_entries(sid, &sqr1, userid, entrygroupid)<0) return;
	total=sql_numtuples(&sqr1);
	if (total>0) {
		prints(sid, "Listing %d log entries<BR>\r\n", total);
		if (sql_queryf(&sqr2, "SELECT groupid, name FROM gw_weblog_groups WHERE obj_did = %d", sid->dat->user_did)<0) return;
		for (i=offset;(i<sql_numtuples(&sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
			prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;");
			for (j=0;j<sql_numtuples(&sqr2);j++) {
				if ((entrygroupid<1)&&(atoi(sql_getvalue(&sqr1, i, 8))==atoi(sql_getvalue(&sqr2, j, 0)))) {
					prints(sid, "[%s] - ", sql_getvalue(&sqr2, j, 1));
				}
			}
			prints(sid, "<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">%s</A>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), str2html(sid, sql_getvalue(&sqr1, i, 9)));
			if (sql_queryf(&sqr3, "SELECT count(commentid) FROM gw_weblog_comments where entryid = %d AND obj_did = %d", atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->user_did)<0) return;
			prints(sid, " (%d comments)", atoi(sql_getvalue(&sqr3, 0, 0)));
			sql_freeresult(&sqr3);
			if (atoi(sql_getvalue(&sqr1, i, 3))==sid->dat->user_uid) {
				prints(sid, " [<A HREF=%s/weblog/edit?entryid=%d onClick=\"location.replace('%s/weblog/edit?entryid=%d');return false;\">Edit</A>]", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			}
			prints(sid, "</TH><TH ALIGN=RIGHT VALIGN=TOP NOWRAP>");
			ltime=time_sql2unix(sql_getvalue(&sqr1, i, 1));
			ltime+=time_tzoffset(sid, ltime);
			prints(sid, "%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
			ltime=time_sql2unix(sql_getvalue(&sqr1, i, 2));
			ltime+=time_tzoffset(sid, ltime);
//			prints(sid, "<BR>%s %s", time_unix2datetext(sid, ltime), time_unix2timetext(sid, ltime));
			prints(sid, "</TH></TR></TABLE>");
			prints(sid, "</TH></TR>\r\n");
			prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid;text-align:justify'><BR>");
			printline2(sid, 1, sql_getvalue(&sqr1, i, 10));
			prints(sid, "\r\n<BR><BR>%d bytes in body<BR>", strlen(sql_getvalue(&sqr1, i, 11)));
			prints(sid, "[<A HREF=%s/weblog/view?entryid=%d onClick=\"location.replace('%s/weblog/view?entryid=%d');return false;\">Read More</A>]\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "[<A HREF=%s/weblog/comments/editnew?entryid=%d onClick=\"location.replace('%s/weblog/comments/editnew?entryid=%d');return false;\">Reply</A>]\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "</TD></TR></TABLE><P>\r\n");
		}
		sql_freeresult(&sqr2);
		if (sql_numtuples(&sqr1)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/weblog/list?offset=%d&userid=%d>Previous Page</A>]\r\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, userid);
			} else {
				prints(sid, "[Previous Page]\r\n");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(&sqr1)) {
				prints(sid, "[<A HREF=%s/weblog/list?offset=%d&userid=%d>Next Page</A>]\r\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, userid);
			} else {
				prints(sid, "[Next Page]\r\n");
			}
		}
	} else {
		prints(sid, "<B>No entries found</B>\r\n");
	}
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
	return;
}

void weblog_entry_save(CONN *sid)
{
	REC_BLOG_ENTRY *entry=calloc(1, sizeof(REC_BLOG_ENTRY));
	char *ptemp;
	int entryid;

	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ENTRYID"))==NULL) goto cleanup;
	entryid=atoi(ptemp);
	if (dbread_weblog_entry(sid, 2, entryid, entry)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		goto cleanup;
	}
	if (auth_priv(sid, "admin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) entry->obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) entry->obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "admin")&A_ADMIN)||(entry->obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) entry->obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) entry->obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ENTRYGROUPID"))!=NULL) entry->entrygroupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL) snprintf(entry->subject, sizeof(entry->subject)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SUMMARY"))!=NULL) snprintf(entry->summary, sizeof(entry->summary)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL) snprintf(entry->message, sizeof(entry->message)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_weblog_entries WHERE entryid = %d", entry->entryid)<0) goto cleanup;
		prints(sid, "<BR><CENTER>Log entry %d deleted successfully</CENTER><BR>\r\n", entry->entryid);
		db_log_activity(sid, "weblogs", entry->entryid, "delete", "%s - %s deleted entry %d", sid->dat->in_RemoteAddr, sid->dat->user_username, entry->entryid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/list\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/list\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName);
	} else if (entry->entryid==0) {
		if ((entry->entryid=dbwrite_weblog_entry(sid, 0, entry))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		prints(sid, "<BR><CENTER>Log entry %d added successfully</CENTER><BR>\r\n", entry->entryid);
		db_log_activity(sid, "weblogs", entry->entryid, "insert", "%s - %s added entry %d", sid->dat->in_RemoteAddr, sid->dat->user_username, entry->entryid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, entry->entryid);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, entry->entryid);
	} else {
		if (dbwrite_weblog_entry(sid, entryid, entry)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			goto cleanup;
		}
		prints(sid, "<BR><CENTER>Log entry %d modified successfully</CENTER><BR>\r\n", entry->entryid);
		db_log_activity(sid, "weblogs", entry->entryid, "modify", "%s - %s modified entry %d", sid->dat->in_RemoteAddr, sid->dat->user_username, entry->entryid);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/weblog/view?entryid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, entry->entryid);
		prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/weblog/view?entryid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, entry->entryid);
	}
cleanup:
	if (entry!=NULL) { free(entry); entry=NULL; }
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_WEBLOG);
	if (strncmp(sid->dat->in_RequestURI, "/weblog/comments/edit", 21)==0) {
		weblog_comment_edit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/weblog/comments/save", 21)==0) {
		weblog_comment_save(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/weblog/edit", 12)==0) {
		weblog_entry_edit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/weblog/view", 12)==0) {
		weblog_entry_view(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/weblog/list", 12)==0) {
		weblog_entry_list(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/weblog/save", 12)==0) {
		weblog_entry_save(sid);
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
		"mod_weblog",		// mod_name
		1,			// mod_submenu
		"WEB LOG",		// mod_menuname
		"default.png",		// mod_menupic
		"/weblog/list",		// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/weblog/",		// fn_uri
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
