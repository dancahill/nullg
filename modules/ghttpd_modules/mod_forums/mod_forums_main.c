/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "ghttpd/mod.h"
#include "mod_forums.h"

typedef struct {
	int id;
	int pid;
	int depth;
	int numchildren;
	time_t posttime;
	char postername[64];
	char subject[64];
} BTREE;

static int forum_treesort_r(CONN *sid, BTREE *btree, SQLRES *sqr, int depth, int parentid, int foldercount)
{
	int fid, pid;
	int i;

	if (depth==0) {
		i=sql_numtuples(sqr);
	} else {
		i=0;
	}
//	for (i=0;i<sql_numtuples(sqr);i++) {
	for (;;) {
		if (depth==0) {
			i--;
			if (i<0) break;
		} else {
			i++;
			if (i>=sql_numtuples(sqr)) break;
		}
		pid=atoi(sql_getvalue(sqr, i, 1));
		if (pid==parentid) {
			fid=atoi(sql_getvalue(sqr, i, 0));
			btree[foldercount].depth=depth;
			btree[foldercount].id=atoi(sql_getvalue(sqr, i, 0));
			btree[foldercount].pid=atoi(sql_getvalue(sqr, i, 1));
			snprintf(btree[foldercount].postername, sizeof(btree[foldercount].postername)-1, "%s", sql_getvalue(sqr, i, 2));
			btree[foldercount].posttime=time_sql2unix(sql_getvalue(sqr, i, 3));
			snprintf(btree[foldercount].subject, sizeof(btree[foldercount].subject)-1, "%s", sql_getvalue(sqr, i, 4));
			foldercount++;
			foldercount=forum_treesort_r(sid, btree, sqr, depth+1, fid, foldercount);
		}
	}
	return foldercount;
}

void forumsave(CONN *sid);
void fmessagesave(CONN *sid);

void htselect_forumgroup(CONN *sid, int selected)
{
	int i, j;
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT forumgroupid, title FROM gw_forumgroups WHERE obj_did = %d ORDER BY title ASC", sid->dat->user_did)<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	sql_freeresult(&sqr);
	return;
}

char *htview_forumgroup(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	SQLRES sqr;

	if (sql_queryf(&sqr, "SELECT title FROM gw_forumgroups WHERE forumgroupid = %d AND obj_did = %d", selected, sid->dat->user_did)<0) return buffer;
	if (sql_numtuples(&sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(&sqr, 0, 0)));
	}
	sql_freeresult(&sqr);
	return buffer;
}

void forumgroupedit(CONN *sid)
{
	REC_FORUMGROUP forumgroup;
	char *ptemp;
	int forumgroupid;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/forums/groupeditnew")==0) {
		forumgroupid=0;
		dbread_forumgroup(sid, 2, 0, &forumgroup);
	} else {
		if ((ptemp=getgetenv(sid, "FORUMGROUPID"))==NULL) return;
		forumgroupid=atoi(ptemp);
		if (dbread_forumgroup(sid, 2, forumgroupid, &forumgroup)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", forumgroupid);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/forums/groupsave NAME=groupedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=forumgroupid VALUE='%d'>\n", forumgroup.forumgroupid);
	if (forumgroupid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Forum Group %d</TH></TR>\n", forumgroupid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Forum Group</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Title</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=title value=\"%s\" SIZE=50></TD></TR>\n", str2html(sid, forumgroup.title));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>Description</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=description ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\n", str2html(sid, forumgroup.description));
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
//	if ((auth_priv(sid, "forums")&A_ADMIN)&&(forumgroupid>1)) {
//		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
//	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.groupedit.title.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void forumgrouplist(CONN *sid)
{
	int i;
	SQLRES sqr;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr, "SELECT forumgroupid, title FROM gw_forumgroups WHERE  obj_did = %d ORDER BY title ASC", sid->dat->user_did)<0) return;
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=150 STYLE='border-style:solid'>&nbsp;Forum Group Name&nbsp;</TH></TR>\n");
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/forums/groupedit?forumgroupid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "<A HREF=%s/forums/groupedit?forumgroupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "<B>There are no forum groups</B><BR>\n");
	}
	sql_freeresult(&sqr);
	prints(sid, "<A HREF=%s/forums/groupeditnew>New Forum Group</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void forumgroupsave(CONN *sid)
{
	REC_FORUMGROUP forumgroup;
	char *ptemp;
	int forumgroupid;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FORUMGROUPID"))==NULL) return;
	forumgroupid=atoi(ptemp);
	if (dbread_forumgroup(sid, 2, forumgroupid, &forumgroup)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "TITLE"))!=NULL) snprintf(forumgroup.title, sizeof(forumgroup.title)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DESCRIPTION"))!=NULL) snprintf(forumgroup.description, sizeof(forumgroup.description)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_forumgroups WHERE forumgroupid = %d", forumgroup.forumgroupid)<0) return;
		prints(sid, "<CENTER>Forum Group %d deleted successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, "forumgroups", forumgroup.forumgroupid, "delete", "%s - %s deleted forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	} else if (forumgroup.forumgroupid==0) {
		if ((forumgroup.forumgroupid=dbwrite_forumgroup(sid, 0, &forumgroup))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Forum Group %d added successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, "forumgroups", forumgroup.forumgroupid, "insert", "%s - %s added forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	} else {
		if (dbwrite_forumgroup(sid, forumgroupid, &forumgroup)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Forum Group %d modified successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, "forumgroups", forumgroup.forumgroupid, "modify", "%s - %s modified forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/forums/grouplist\">\n", sid->dat->in_ScriptName);
	return;
}

void forumview(CONN *sid, int forumid)
{
	int forumgroupid;
	SQLRES sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr, "SELECT forumid, forumgroupid, postername, subject, message FROM gw_forums WHERE forumid = %d AND obj_did = %d", forumid, sid->dat->user_did)<0) return;
	prints(sid, "<CENTER>\n");
	forumgroupid=atoi(sql_getvalue(&sqr, 0, 1));
	if (forumgroupid) {
		prints(sid, "<B><FONT STYLE='font-size: 14px'>");
		prints(sid, "<A HREF=\"%s/forums/list?forumgroupid=%d\">%s</A></FONT></B><BR><BR>\n", sid->dat->in_ScriptName, forumgroupid, htview_forumgroup(sid, forumgroupid));
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;<B>%s</B>", str2html(sid, sql_getvalue(&sqr, 0, 3)));
	prints(sid, " by <B>%s</B>", str2html(sid, sql_getvalue(&sqr, 0, 2)));
	prints(sid, "</TH></TR>\n<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>\n");
	printline2(sid, 1, sql_getvalue(&sqr, 0, 4));
	prints(sid, "<FONT SIZE=2>\n<BR><BR>[<A HREF=%s/forums/msglist?forum=%d>Top</A>]&nbsp;[<A HREF=%s/forums/msgpost?forum=%d>Reply</A>]</FONT>\n", sid->dat->in_ScriptName, forumid, sid->dat->in_ScriptName, forumid);
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(&sqr);
	return;
}

void fmessageview(CONN *sid, int forumid, int messageid)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int previous=0;
	int next=0;
	int i;
	int j;
	SQLRES sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr, "SELECT messageid, referenceid FROM gw_forumposts WHERE forumid = %d AND obj_did = %d ORDER BY messageid ASC", forumid, sid->dat->user_did)<0) return;
	if (sql_numtuples(&sqr)<1) return;
	btree=calloc(sql_numtuples(&sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(&sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(&sqr);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(&sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(&sqr, i, 0));
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
	if (base<sql_numtuples(&sqr)) {
		goto widthloop;
	}
	for (i=0; i<sql_numtuples(&sqr); i++) {
		if (ptree[i].id+1==messageid) {
			if (i>0) {
				previous=ptree[i-1].id+1;
			}
			if (i<sql_numtuples(&sqr)-1) {
				next=ptree[i+1].id+1;
			}
			break;
		}
	}
	free(ptree);
	free(btree);
	sql_freeresult(&sqr);
	if (sql_queryf(&sqr, "SELECT messageid, postername, posttime, subject, message FROM gw_forumposts WHERE messageid = %d and forumid = %d AND obj_did = %d", messageid, forumid, sid->dat->user_did)<0) return;
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
	prints(sid, "&nbsp;<B>%s</B><BR>", str2html(sid, sql_getvalue(&sqr, 0, 3)));
	prints(sid, "&nbsp;By <B>%s</B> on ", str2html(sid, sql_getvalue(&sqr, 0, 1)));
	prints(sid, "%s ", time_sql2datetext(sid, sql_getvalue(&sqr, 0, 2)));
	prints(sid, "%s", time_sql2timetext(sid, sql_getvalue(&sqr, 0, 2)));
	prints(sid, "</FONT></TH></TR>\n<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>\n");
	prints(sid, "<FONT SIZE=2>\n[<A HREF=%s/forums/msglist?forum=%d>Top</A>]", sid->dat->in_ScriptName, forumid);
	if (previous>0) {
		prints(sid, "&nbsp;[<A HREF=%s/forums/msgread?forum=%d&message=%d>Previous</A>]", sid->dat->in_ScriptName, forumid, previous);
	} else {
		prints(sid, "&nbsp;[Previous]");
	}
	if (next>0) {
		prints(sid, "&nbsp;[<A HREF=%s/forums/msgread?forum=%d&message=%d>Next</A>]", sid->dat->in_ScriptName, forumid, next);
	} else {
		prints(sid, "&nbsp;[Next]\n");
	}
	prints(sid, "</FONT><BR><BR>\n");
	printline2(sid, 1, sql_getvalue(&sqr, 0, 4));
	prints(sid, "<BR><BR><FONT SIZE=2>\n[<A HREF=%s/forums/msglist?forum=%d>Top</A>]", sid->dat->in_ScriptName, forumid);
	if (previous>0) {
		prints(sid, "&nbsp;[<A HREF=%s/forums/msgread?forum=%d&message=%d>Previous</A>]", sid->dat->in_ScriptName, forumid, previous);
	} else {
		prints(sid, "&nbsp;[Previous]");
	}
	if (next>0) {
		prints(sid, "&nbsp;[<A HREF=%s/forums/msgread?forum=%d&message=%d>Next</A>]", sid->dat->in_ScriptName, forumid, next);
	} else {
		prints(sid, "&nbsp;[Next]");
	}
	prints(sid, "</FONT>\n");
	prints(sid, "</TD></TR></TABLE><P>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(&sqr);
	return;
}

void forumpost(CONN *sid)
{
	char message[12288];
	char subject[64];
	char submit[10];
	char *pTemp;
	int forumgroupid=0;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(submit, 0, sizeof(submit));
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		if ((pTemp=getpostenv(sid, "SUBMIT"))!=NULL) {
			strncpy(submit, pTemp, sizeof(submit)-1);
			if (strcmp(submit, "Save")==0) {
				forumsave(sid);
				return;
			}
		}
		if ((pTemp=getpostenv(sid, "FORUMGROUPID"))!=NULL) forumgroupid=atoi(pTemp);
		if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL) strncpy(message, pTemp, sizeof(message)-1);
		if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL) strncpy(subject, pTemp, sizeof(subject)-1);
		if (strcmp(submit, "Preview")==0) {
			prints(sid, "<CENTER>\n");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>\n");
			prints(sid, "<B>%s</B> by ", str2html(sid, subject));
			prints(sid, "<B>%s</B>\n", str2html(sid, sid->dat->user_username));
			prints(sid, "</FONT></TH></TR><TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>\n");
			printline2(sid, 1, message);
			prints(sid, "&nbsp;\n");
			prints(sid, "</TD></TR></TABLE><P>\n");
			prints(sid, "</CENTER>\n");
		}
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/forums/post NAME=forumpost>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR><TH COLSPAN=2>New Forum</TH></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Subject&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject VALUE=\"%s\" SIZE=60 style='width:427px'></TD></TR>\n", str2html(sid, subject));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Forum Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=forumgroupid style='width:427px'>\n");
	htselect_forumgroup(sid, forumgroupid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=10 COLS=70>");
	printline2(sid, 0, message);
	prints(sid, "</TEXTAREA></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Preview'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>document.forumpost.subject.focus();</SCRIPT>\n");
}

void fmessagepost(CONN *sid)
{
	char *ptemp;
	char message[12288];
	char subject[64];
	char submit[10];
	int forumid=0;
	int referenceid=0;
	SQLRES sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(submit, 0, sizeof(submit));
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		if ((ptemp=getpostenv(sid, "SUBMIT"))!=NULL) {
			strncpy(submit, ptemp, sizeof(submit)-1);
			if (strcmp(submit, "Save")==0) {
				fmessagesave(sid);
				return;
			}
		}
		if ((ptemp=getpostenv(sid, "FORUM"))!=NULL)     forumid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "REFERENCE"))!=NULL) referenceid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "MESSAGE"))!=NULL)   strncpy(message, ptemp, sizeof(message)-1);
		if ((ptemp=getpostenv(sid, "SUBJECT"))!=NULL)   strncpy(subject, ptemp, sizeof(subject)-1);
		if (strcmp(submit, "Preview")==0) {
			prints(sid, "<CENTER>\n");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>\n");
			prints(sid, "<B>%s</B> by ", str2html(sid, subject));
			prints(sid, "<B>%s</B>\n", str2html(sid, sid->dat->user_username));
			prints(sid, "</TH></TR><TR><TD STYLE='border-style:solid'>\n");
			printline2(sid, 1, message);
			prints(sid, "&nbsp;\n");
			prints(sid, "</TD></TR></TABLE><P>\n");
			prints(sid, "</CENTER>\n");
		}
	} else {
		if ((ptemp=getgetenv(sid, "FORUM"))!=NULL) forumid=atoi(ptemp);
		if ((ptemp=getgetenv(sid, "MESSAGE"))!=NULL) {
			referenceid=atoi(ptemp);
		} else if ((ptemp=getgetenv(sid, "REFERENCE"))!=NULL) {
			referenceid=atoi(ptemp);
		} else {
			referenceid=0;
		}
		if (referenceid>0) {
			fmessageview(sid, forumid, referenceid);
		} else {
			forumview(sid, forumid);
		}
	}
	if (!(auth_priv(sid, "forums")&A_INSERT)) return;
	if (sql_queryf(&sqr, "SELECT subject FROM gw_forumposts WHERE messageid = %d and forumid = %d AND obj_did = %d", referenceid, forumid, sid->dat->user_did)<0) return;
	if (sql_numtuples(&sqr)>0) {
		if (strncasecmp(sql_getvalue(&sqr, 0, 0), "RE:", 3)!=0) {
			snprintf(subject, sizeof(subject)-1, "Re: %s", sql_getvalue(&sqr, 0, 0));
		} else {
			snprintf(subject, sizeof(subject)-1, "%s", sql_getvalue(&sqr, 0, 0));
		}
	}
	sql_freeresult(&sqr);
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/forums/msgpost NAME=msgpost>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=forum VALUE=%d>\n", forumid);
	prints(sid, "<INPUT TYPE=hidden NAME=reference VALUE=%d>\n", referenceid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR><TH COLSPAN=2>New Message</TH></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>Subject</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject VALUE=\"%s\" SIZE=70></TD></TR>\n", str2html(sid, subject));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=10 COLS=70>");
	printline2(sid, 0, message);
	prints(sid, "</TEXTAREA></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Preview'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
}

void forumlist(CONN *sid, int longlist)
{
	char body[512];
	char *ptemp;
	int forumgroupid=0;
	int i;
	int selected=0;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "FORUMGROUPID"))!=NULL) { selected=1; forumgroupid=atoi(ptemp); }
	if (sql_queryf(&sqr1, "SELECT distinct forumgroupid FROM gw_forums WHERE obj_did = %d ORDER BY forumgroupid ASC", sid->dat->user_did)<0) return;
	if (sql_numtuples(&sqr1)>1) {
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			if (forumgroupid==atoi(sql_getvalue(&sqr1, i, 0))) {
				if (selected==1) selected=2;
				break;
			}
		}
		if (selected!=2) {
			prints(sid, "<CENTER>\n");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=250 STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Forum Groups</TH></TR>\n");
			for (i=0;i<sql_numtuples(&sqr1);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				prints(sid, "<TD NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=window.location.href=\"%s/forums/list?forumgroupid=%d\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
				prints(sid, "<A HREF=%s/forums/list?forumgroupid=%d", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
				prints(sid, ">%s</A>&nbsp;</TD>", htview_forumgroup(sid, atoi(sql_getvalue(&sqr1, i, 0))));
				if (sql_queryf(&sqr2, "SELECT count(forumid) FROM gw_forums WHERE forumgroupid=%d AND obj_did = %d", atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->user_did)<0) {
					sql_freeresult(&sqr1);
					return;
				}
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%d forum%s</TD>", atoi(sql_getvalue(&sqr2, 0, 0)), atoi(sql_getvalue(&sqr2, 0, 0))==1?"":"s");
				sql_freeresult(&sqr2);
				prints(sid, "</TR>\n");
			}
			prints(sid, "</TABLE>\n");
			prints(sid, "</CENTER>\n");
			sql_freeresult(&sqr1);
			return;
		}
	} else if (sql_numtuples(&sqr1)==1) {
		forumgroupid=atoi(sql_getvalue(&sqr1, 0, 0));
	}
	sql_freeresult(&sqr1);
	prints(sid, "<CENTER>\n");
	if ((selected==2)&&(forumgroupid)) {
		prints(sid, "<B><FONT STYLE='font-size: 14px'>%s</FONT></B><BR><BR>\n", htview_forumgroup(sid, forumgroupid));
	}
	if (longlist==0) {
		if (sql_queryf(&sqr1, "SELECT forumid, postername, posttime, subject FROM gw_forums WHERE forumgroupid = %d AND obj_did = %d ORDER BY forumid ASC", forumgroupid, sid->dat->user_did)<0) return;
		if (sql_numtuples(&sqr1)>0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH ALIGN=left STYLE='border-style:solid'>Forum List</TH></TR>\n");
			for (i=0;i<sql_numtuples(&sqr1);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP STYLE='border-style:solid'>");
				prints(sid, "[%d] <B><A HREF=%s/forums/msglist?forum=%d>", atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
				prints(sid, "%s</A></B> by ", str2html(sid, sql_getvalue(&sqr1, i, 3)));
				prints(sid, "<B>%s</B>", str2html(sid, sql_getvalue(&sqr1, i, 1)));
				if (sql_queryf(&sqr2, "SELECT count(messageid) FROM gw_forumposts where forumid = %d AND obj_did = %d", atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->user_did)<0) return;
				prints(sid, " (%d posts)", atoi(sql_getvalue(&sqr2, 0, 0)));
				sql_freeresult(&sqr2);
				prints(sid, "</TD></TR>\n");
			}
			prints(sid, "</TABLE>\n");
		} else {
			prints(sid, "<B>There are no forums</B>\n");
		}
	} else {
		if (sql_queryf(&sqr1, "SELECT forumid, postername, posttime, subject, message FROM gw_forums WHERE forumgroupid = %d AND obj_did = %d ORDER BY forumid DESC", forumgroupid, sid->dat->user_did)<0) return;
		if (sql_numtuples(&sqr1)>0) {
			for (i=0;i<sql_numtuples(&sqr1);i++) {
				prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%% STYLE='border-style:solid'>\r\n");
				prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>");
				prints(sid, "&nbsp;%s by ", str2html(sid, sql_getvalue(&sqr1, i, 3)));
				prints(sid, "%s", str2html(sid, sql_getvalue(&sqr1, i, 1)));
				if (sql_queryf(&sqr2, "SELECT count(messageid) FROM gw_forumposts where forumid = %d AND obj_did = %d", atoi(sql_getvalue(&sqr1, i, 0)), sid->dat->user_did)<0) return;
				prints(sid, " (%d posts)", atoi(sql_getvalue(&sqr2, 0, 0)));
				sql_freeresult(&sqr2);
				prints(sid, "</FONT></TD></TR>\n");
				prints(sid, "<TR><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>");
				memset(body, 0, sizeof(body));
				snprintf(body, sizeof(body)-1, "%s", sql_getvalue(&sqr1, i, 4));
//				printline2(sid, 1, sql_getvalue(&sqr1, 0, 4));
				printline2(sid, 1, body);
				prints(sid, "\n<BR><BR>[<A HREF=%s/forums/msglist?forum=%s>Read More</A>]\n", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 0));
				prints(sid, "</TD></TR></TABLE><P>\n");
			}
		} else {
			prints(sid, "<B>There are no forums</B>\n");
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(&sqr1);
	return;
}

void fmessagelist(CONN *sid)
{
	BTREE *xtree;
	char *ptemp;
	int msgcount;
	int forumid;
	int indent;
	int i;
	int j;
	int x;
	SQLRES sqr;
	short int bgtoggle=0;
	int tzoffset;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "FORUM"))==NULL) return;
	tzoffset=time_tzoffset(sid, time(NULL));
	forumid=atoi(ptemp);
	forumview(sid, forumid);
	if (sql_queryf(&sqr, "SELECT messageid, referenceid, postername, posttime, subject FROM gw_forumposts WHERE forumid = %d AND obj_did = %d ORDER BY messageid ASC", forumid, sid->dat->user_did)<0) return;
	msgcount=sql_numtuples(&sqr);
	if (msgcount<1) {
		sql_freeresult(&sqr);
		return;
	}
	xtree=calloc(msgcount, sizeof(BTREE));
	forum_treesort_r(sid, xtree, &sqr, 0, 0, 0);
	sql_freeresult(&sqr);
	for (i=0;i<msgcount;i++) {
		for (j=i+1;j<msgcount;j++) {
			if (xtree[j].depth<xtree[i].depth+1) break;
			if (xtree[j].depth>xtree[i].depth+1) continue;
			xtree[i].numchildren++;
		}
	}
	prints(sid, "<CENTER><BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=90%%>\n");
	prints(sid, "<TR STYLE='padding:2px'><TH ALIGN=left>&nbsp;Topic&nbsp;</TH><TH ALIGN=left>&nbsp;Author&nbsp;</TH><TH ALIGN=left>&nbsp;Date&nbsp;</TH></TR>\n");
	for (i=0;i<msgcount;i++) {
		bgtoggle=bgtoggle?0:1;
		prints(sid, "<TR BGCOLOR=\"%s\">", bgtoggle?"#F0F0F0":"#E0E0E0");
		prints(sid, "<TD NOWRAP ALIGN=TOP WIDTH=100%%>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>");
		for (indent=0;indent<xtree[i].depth;indent++) {
			x=0;
			for (j=i-1;j>-1;j--) {
				if (indent==xtree[i].depth-1) {
					if (xtree[j].numchildren>1) {
						prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-t.png HEIGHT=21 WIDTH=9>");
						xtree[j].numchildren--;
						x=1;
						break;
					} else if (xtree[j].numchildren==1) {
						prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-l.png HEIGHT=21 WIDTH=9>");
						xtree[j].numchildren--;
						x=1;
						break;
					}
				} else if (indent<xtree[i].depth-1) {
					if (indent==xtree[j].depth) {
						if (xtree[j].numchildren>0) {
							prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-i.png HEIGHT=21 WIDTH=9>");
							x=1;
							break;
						}
					}
				}
			}
			if (!x) {
				prints(sid, "<IMG BORDER=0 SRC=/groupware/images/blank.png HEIGHT=21 WIDTH=9>");
			}
		}
		if (xtree[i].numchildren>0) {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-m.png HEIGHT=21 WIDTH=9>");
		} else if (xtree[i].depth<1) {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-o.png HEIGHT=21 WIDTH=9>");
		} else {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/tree-c.png HEIGHT=21 WIDTH=9>");
		}
		prints(sid, "</TD><TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=\"window.location.href='%s/forums/msgread?forum=%d&message=%d'\">", sid->dat->in_ScriptName, forumid, xtree[i].id);
		prints(sid, "<A HREF=%s/forums/msgread?forum=%d&message=%d>", sid->dat->in_ScriptName, forumid, xtree[i].id);
		prints(sid, "<B>&nbsp;%s</B></A>&nbsp;", str2html(sid, xtree[i].subject));
		prints(sid, "</TD></TR></TABLE></TD>");
		prints(sid, "<TD NOWRAP>%s&nbsp;&nbsp;</TD>", str2html(sid, xtree[i].postername));
		prints(sid, "<TD NOWRAP ALIGN=RIGHT>%s ", time_unix2datetext(sid, xtree[i].posttime+tzoffset));
		prints(sid, "%s</TD></TR>\n", time_unix2timetext(sid, xtree[i].posttime+tzoffset));
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
	free(xtree);
	return;
}

void forumsave(CONN *sid)
{
	char query[16384];
	char message[12288];
	char posttime[100];
	char subject[64];
	time_t t;
	int forumid;
	int forumgroupid=0;
	SQLRES sqr;
	char *pTemp;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(query, 0, sizeof(query));
	if (sql_query(&sqr, "SELECT max(forumid) FROM gw_forums")<0) return;
	forumid=atoi(sql_getvalue(&sqr, 0, 0))+1;
	sql_freeresult(&sqr);
	if (forumid<1) forumid=1;
	if ((pTemp=getpostenv(sid, "FORUMGROUPID"))!=NULL) forumgroupid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)   strncpy(subject, pTemp, sizeof(subject)-1);
	if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL) {
		strncpy(query, pTemp, sizeof(query)-1);
		str2sql(message, sizeof(message)-1, query);
	}
	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forums (forumid,obj_ctime,obj_mtime,obj_uid,obj_gid,obj_did,obj_gperm,obj_operm,forumgroupid,postername,posttime,subject,message) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d','%s','%s',0,0,%d,0,0,'%d','%s','%s','%s','", forumid, posttime, posttime, sid->dat->user_did, forumgroupid, sid->dat->user_username, posttime, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, subject));
	strncat(query, message, sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sql_update(query)<0) return;
	prints(sid, "<CENTER>Forum %d added successfully</CENTER>\n", forumid);
	forumview(sid, forumid);
	db_log_activity(sid, "forums", forumid, "insert", "%s - %s added forum %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumid);
	return;
}

void fmessagesave(CONN *sid)
{
	char query[16384];
	char message[12288];
	char posttime[100];
	char subject[64];
	time_t t;
	int forumid=0;
	int messageid=0;
	int referenceid=0;
	SQLRES sqr;
	char *pTemp;

	if (!(auth_priv(sid, "forums")&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(query, 0, sizeof(query));
	if ((pTemp=getpostenv(sid, "FORUM"))!=NULL)
		forumid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "REFERENCE"))!=NULL)
		referenceid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)
		strncpy(subject, pTemp, sizeof(subject)-1);
	if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL) {
		strncpy(query, pTemp, sizeof(query)-1);
		str2sql(message, sizeof(message)-1, query);
	}
	memset(query, 0, sizeof(query));
	if (sql_queryf(&sqr, "SELECT max(messageid) FROM gw_forumposts where forumid = %d", forumid)<0) return;
	messageid=atoi(sql_getvalue(&sqr, 0, 0))+1;
	if (messageid<1) messageid=1;
	sql_freeresult(&sqr);
	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forumposts (messageid,obj_ctime,obj_mtime,obj_uid,obj_gid,obj_did,obj_gperm,obj_operm,forumid,referenceid,postername,posttime,subject,message) values (%d, '%s', '%s', 0, 0, %d, 0, 0, %d, %d, '%s', '%s', '%s', '", messageid, posttime, posttime, sid->dat->user_did, forumid, referenceid, sid->dat->user_username, posttime, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, subject));
	strncat(query, message, sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sql_update(query)<0) return;
	prints(sid, "<CENTER>Message %d added successfully</CENTER>\n", messageid);
	fmessageview(sid, forumid, messageid);
	db_log_activity(sid, "forumposts", forumid, "insert", "%s - %s added message %d on forum %d", sid->dat->in_RemoteAddr, sid->dat->user_username, messageid, forumid);
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_FORUMS);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/forums/msgread",      15)==0) {
		fmessagepost(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/post",    12)==0) {
		forumpost(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/msgpost", 15)==0) {
		fmessagepost(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/list",    12)==0) {
		forumlist(sid, 1);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/msglist", 15)==0) {
		fmessagelist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/save",    12)==0) {
		forumsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/msgsave", 15)==0) {
		fmessagesave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/groupedit", 17)==0) {
		forumgroupedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/grouplist", 17)==0) {
		forumgrouplist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/forums/groupsave", 17)==0) {
		forumgroupsave(sid);
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
		"mod_forums",		// mod_name
		1,			// mod_submenu
		"FORUMS",		// mod_menuname
		"/forums/list",		// mod_menuuri
		"forums",		// mod_menuperm
		"mod_main",		// fn_name
		"/forums/",		// fn_uri
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
