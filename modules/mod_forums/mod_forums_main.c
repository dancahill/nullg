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
#include "mod_forums.h"

typedef struct {
	int lastref;
	int printed;
} _btree;
typedef struct {
	int id;
	int depth;
	int numchildren;
} _ptree;

void forumsave(CONN *sid);
void fmessagesave(CONN *sid);

void htselect_forumgroup(CONN *sid, int selected)
{
	int i, j;
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT forumgroupid, title FROM gw_forumgroups ORDER BY title ASC"))<0) return;
	prints(sid, "<OPTION VALUE='0'>\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	return;
}

char *htview_forumgroup(CONN *sid, int selected)
{
	char *buffer=getbuffer(sid);
	int sqr;

	if ((sqr=sql_queryf(sid, "SELECT title FROM gw_forumgroups WHERE forumgroupid = %d", selected))<0) return buffer;
	if (sql_numtuples(sqr)==1) {
		snprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, "%s", str2html(sid, sql_getvalue(sqr, 0, 0)));
	}
	sql_freeresult(sqr);
	return buffer;
}

void forumgroupedit(CONN *sid)
{
	REC_FORUMGROUP forumgroup;
	int forumgroupid;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/forums/groupeditnew")==0) {
		forumgroupid=0;
		dbread_forumgroup(sid, 2, 0, &forumgroup);
	} else {
		if (getgetenv(sid, "FORUMGROUPID")==NULL) return;
		forumgroupid=atoi(getgetenv(sid, "FORUMGROUPID"));
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (forumgroupid!=0) {
		prints(sid, "Forum Group %d</FONT></TH></TR>\n", forumgroupid);
	} else {
		prints(sid, "New Forum Group</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Title</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=title value=\"%s\" SIZE=50></TD></TR>\n", config->colour_editform, str2html(sid, forumgroup.title));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>Description</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=description ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, forumgroup.description));
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
//	if ((auth_priv(sid, "forums")&A_ADMIN)&&(forumgroupid>1)) {
//		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
//	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", config->colour_editform);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.groupedit.title.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void forumgrouplist(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_query(sid, "SELECT forumgroupid, title FROM gw_forumgroups ORDER BY title ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", config->colour_tabletrim);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=150><FONT COLOR=%s>&nbsp;Forum Group Name&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/forums/groupedit?forumgroupid=%d'\">", config->colour_fieldval, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/forums/groupedit?forumgroupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no forum groups<BR>\n");
	}
	sql_freeresult(sqr);
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FORUMGROUPID"))==NULL) return;
	forumgroupid=atoi(ptemp);
	if (dbread_forumgroup(sid, 2, forumgroupid, &forumgroup)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "TITLE"))!=NULL) snprintf(forumgroup.title, sizeof(forumgroup.title)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DESCRIPTION"))!=NULL) snprintf(forumgroup.description, sizeof(forumgroup.description)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef(sid, "DELETE FROM gw_forumgroups WHERE forumgroupid = %d", forumgroup.forumgroupid)<0) return;
		prints(sid, "<CENTER>Forum Group %d deleted successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, 1, "forumgroups", forumgroup.forumgroupid, "delete", "%s - %s deleted forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	} else if (forumgroup.forumgroupid==0) {
		if ((forumgroup.forumgroupid=dbwrite_forumgroup(sid, 0, &forumgroup))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Forum Group %d added successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, 1, "forumgroups", forumgroup.forumgroupid, "insert", "%s - %s added forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	} else {
		if (dbwrite_forumgroup(sid, forumgroupid, &forumgroup)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Forum Group %d modified successfully</CENTER><BR>\n", forumgroup.forumgroupid);
		db_log_activity(sid, 1, "forumgroups", forumgroup.forumgroupid, "modify", "%s - %s modified forum group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumgroup.forumgroupid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/forums/grouplist\">\n", sid->dat->in_ScriptName);
	return;
}

void forumview(CONN *sid, int forumid)
{
	int forumgroupid;
	int sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_queryf(sid, "SELECT forumid, forumgroupid, postername, subject, message FROM gw_forums WHERE forumid = %d", forumid))<0) return;
	prints(sid, "<CENTER>\n");
	forumgroupid=atoi(sql_getvalue(sqr, 0, 1));
	if (forumgroupid) {
		prints(sid, "<B><FONT COLOR=%s STYLE='font-size: 14px'>", config->colour_links);
		prints(sid, "<A HREF=\"%s/forums/list?forumgroupid=%d\">%s</A></FONT></B><BR><BR>\n", sid->dat->in_ScriptName, forumgroupid, htview_forumgroup(sid, forumgroupid));
	}
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s NOWRAP><FONT COLOR=%s>&nbsp;<B>%s</B>", config->colour_th, config->colour_thtext, str2html(sid, sql_getvalue(sqr, 0, 3)));
	prints(sid, " by <B>%s</B>", str2html(sid, sql_getvalue(sqr, 0, 2)));
	prints(sid, "</FONT></TH></TR>\n<TR><TD BGCOLOR=%s>\n", config->colour_fieldval);
	printline2(sid, 1, sql_getvalue(sqr, 0, 4));
	prints(sid, "<FONT SIZE=2>\n<BR><BR>[<A HREF=%s/forums/msglist?forum=%d>Top</A>]&nbsp;[<A HREF=%s/forums/msgpost?forum=%d>Reply</A>]</FONT>\n", sid->dat->in_ScriptName, forumid, sid->dat->in_ScriptName, forumid);
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
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
	int sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_queryf(sid, "SELECT messageid, referenceid FROM gw_forumposts WHERE forumid = %d ORDER BY messageid ASC", forumid))<0) return;
	if (sql_numtuples(sqr)<1) return;
	btree=calloc(sql_numtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base; i<sql_numtuples(sqr); i++) {
		if (btree[atoi(sql_getvalue(sqr, i, 0))].printed) continue;
		if (atoi(sql_getvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr, i, 0));
			btree[atoi(sql_getvalue(sqr, i, 0))].printed=1;
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
	for (i=0; i<sql_numtuples(sqr); i++) {
		if (ptree[i].id+1==messageid) {
			if (i>0) {
				previous=ptree[i-1].id+1;
			}
			if (i<sql_numtuples(sqr)-1) {
				next=ptree[i+1].id+1;
			}
			break;
		}
	}
	free(ptree);
	free(btree);
	sql_freeresult(sqr);
	if ((sqr=sql_queryf(sid, "SELECT messageid, postername, posttime, subject, message FROM gw_forumposts WHERE messageid = %d and forumid = %d", messageid, forumid))<0) return;
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s NOWRAP><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	prints(sid, "&nbsp;<B>%s</B><BR>", str2html(sid, sql_getvalue(sqr, 0, 3)));
	prints(sid, "&nbsp;By <B>%s</B> on ", str2html(sid, sql_getvalue(sqr, 0, 1)));
	prints(sid, "%s ", time_sql2datetext(sid, sql_getvalue(sqr, 0, 2)));
	prints(sid, "%s", time_sql2timetext(sid, sql_getvalue(sqr, 0, 2)));
	prints(sid, "</FONT></TH></TR>\n<TR><TD BGCOLOR=%s>\n", config->colour_fieldval);
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
	printline2(sid, 1, sql_getvalue(sqr, 0, 4));
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
	sql_freeresult(sqr);
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
			prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
			prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s NOWRAP><FONT COLOR=%s>\n", config->colour_th, config->colour_thtext);
			prints(sid, "<B>%s</B> by ", str2html(sid, subject));
			prints(sid, "<B>%s</B>\n", str2html(sid, sid->dat->user_username));
			prints(sid, "</FONT></TH></TR><TR BGCOLOR=%s><TD>\n", config->colour_fieldval);
			printline2(sid, 1, message);
			prints(sid, "&nbsp;\n");
			prints(sid, "</TD></TR></TABLE><P>\n");
			prints(sid, "</CENTER>\n");
		}
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/forums/post NAME=forumpost>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>New Forum</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Subject&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject VALUE=\"%s\" SIZE=60 style='width:427px'></TD></TR>\n", config->colour_editform, str2html(sid, subject));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Forum Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=forumgroupid style='width:427px'>\n", config->colour_editform);
	htselect_forumgroup(sid, forumgroupid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=10 COLS=70>", config->colour_editform);
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
	char message[12288];
	char subject[64];
	char submit[10];
	int forumid=0;
	int referenceid=0;
	int sqr;
	char *pTemp;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(submit, 0, sizeof(submit));
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		if ((pTemp=getpostenv(sid, "SUBMIT"))!=NULL) {
			strncpy(submit, pTemp, sizeof(submit)-1);
			if (strcmp(submit, "Save")==0) {
				fmessagesave(sid);
				return;
			}
		}
		if ((pTemp=getpostenv(sid, "FORUM"))!=NULL)     forumid=atoi(pTemp);
		if ((pTemp=getpostenv(sid, "REFERENCE"))!=NULL) referenceid=atoi(pTemp);
		if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL)   strncpy(message, pTemp, sizeof(message)-1);
		if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)   strncpy(subject, pTemp, sizeof(subject)-1);
		if (strcmp(submit, "Preview")==0) {
			prints(sid, "<CENTER>\n");
			prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
			prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s NOWRAP><FONT COLOR=%s>\n", config->colour_th, config->colour_thtext);
			prints(sid, "<B>%s</B> by ", str2html(sid, subject));
			prints(sid, "<B>%s</B>\n", str2html(sid, sid->dat->user_username));
			prints(sid, "</FONT></TH></TR><TR><TD BGCOLOR=%s>\n", config->colour_fieldval);
			printline2(sid, 1, message);
			prints(sid, "&nbsp;\n");
			prints(sid, "</TD></TR></TABLE><P>\n");
			prints(sid, "</CENTER>\n");
		}
	} else {
		if ((pTemp=getgetenv(sid, "FORUM"))!=NULL) forumid=atoi(pTemp);
		if ((pTemp=getgetenv(sid, "MESSAGE"))!=NULL) {
			referenceid=atoi(pTemp);
		} else if ((pTemp=getgetenv(sid, "REFERENCE"))!=NULL) {
			referenceid=atoi(pTemp);
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
	if ((sqr=sql_queryf(sid, "SELECT subject FROM gw_forumposts WHERE messageid = %d and forumid = %d", referenceid, forumid))<0) return;
	if (sql_numtuples(sqr)>0) {
		if (strncasecmp(sql_getvalue(sqr, 0, 0), "RE:", 3)!=0) {
			snprintf(subject, sizeof(subject)-1, "Re: %s", sql_getvalue(sqr, 0, 0));
		} else {
			snprintf(subject, sizeof(subject)-1, "%s", sql_getvalue(sqr, 0, 0));
		}
	}
	sql_freeresult(sqr);
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/forums/msgpost NAME=msgpost>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=forum VALUE=%d>\n", forumid);
	prints(sid, "<INPUT TYPE=hidden NAME=reference VALUE=%d>\n", referenceid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>New Message</TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD><B>Subject</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=subject VALUE=\"%s\" SIZE=70></TD></TR>\n", config->colour_editform, str2html(sid, subject));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=message ROWS=10 COLS=70>", config->colour_editform);
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
	int sqr;
	int sqr2;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "FORUMGROUPID"))!=NULL) { selected=1; forumgroupid=atoi(ptemp); }
	if ((sqr=sql_query(sid, "SELECT distinct forumgroupid FROM gw_forums ORDER BY forumgroupid ASC"))<0) return;
	if (sql_numtuples(sqr)>1) {
		for (i=0;i<sql_numtuples(sqr);i++) {
			if (forumgroupid==atoi(sql_getvalue(sqr, i, 0))) {
				if (selected==1) selected=2;
				break;
			}
		}
		if (selected!=2) {
			prints(sid, "<CENTER>\n");
			prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=250>\r\n", config->colour_tabletrim);
			prints(sid, "<TR><TH BGCOLOR=%s COLSPAN=2 NOWRAP><FONT COLOR=%s>Forum Groups</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
				prints(sid, "<TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=window.location.href=\"%s/forums/list?forumgroupid=%d\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "<A HREF=%s/forums/list?forumgroupid=%d", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, ">%s</A>&nbsp;</TD>", htview_forumgroup(sid, atoi(sql_getvalue(sqr, i, 0))));
				if ((sqr2=sql_queryf(sid, "SELECT count(forumid) FROM gw_forums WHERE forumgroupid=%d", atoi(sql_getvalue(sqr, i, 0))))<0) {
					sql_freeresult(sqr);
					return;
				}
				prints(sid, "<TD NOWRAP>%d forum%s</TD>", atoi(sql_getvalue(sqr2, 0, 0)), atoi(sql_getvalue(sqr2, 0, 0))==1?"":"s");
				sql_freeresult(sqr2);
				prints(sid, "</TR>\n");
			}
			prints(sid, "</TABLE>\n");
			prints(sid, "</CENTER>\n");
			sql_freeresult(sqr);
			return;
		}
	} else if (sql_numtuples(sqr)==1) {
		forumgroupid=atoi(sql_getvalue(sqr, 0, 0));
	}
	sql_freeresult(sqr);
	prints(sid, "<CENTER>\n");
	if ((selected==2)&&(forumgroupid)) {
		prints(sid, "<B><FONT COLOR=%s STYLE='font-size: 14px'>%s</FONT></B><BR><BR>\n", config->colour_links, htview_forumgroup(sid, forumgroupid));
	}
	if (longlist==0) {
		if ((sqr=sql_queryf(sid, "SELECT forumid, postername, posttime, subject FROM gw_forums WHERE forumgroupid = %d ORDER BY forumid ASC", forumgroupid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
			prints(sid, "<TR BGCOLOR=%s><TH ALIGN=left><FONT COLOR=%s>Forum List</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>", config->colour_fieldval);
				prints(sid, "[%d] <B><A HREF=%s/forums/msglist?forum=%d>", atoi(sql_getvalue(sqr, i, 0)), sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "%s</A></B> by ", str2html(sid, sql_getvalue(sqr, i, 3)));
				prints(sid, "<B>%s</B>", str2html(sid, sql_getvalue(sqr, i, 1)));
				if ((sqr2=sql_queryf(sid, "SELECT count(messageid) FROM gw_forumposts where forumid = %d", atoi(sql_getvalue(sqr, i, 0))))<0) return;
				prints(sid, " (%d posts)", atoi(sql_getvalue(sqr2, 0, 0)));
				sql_freeresult(sqr2);
				prints(sid, "</TD></TR>\n");
			}
			prints(sid, "</TABLE>\n");
		} else {
			prints(sid, "There are no forums\n");
		}
	} else {
		if ((sqr=sql_queryf(sid, "SELECT forumid, postername, posttime, subject, message FROM gw_forums WHERE forumgroupid = %d ORDER BY forumid DESC", forumgroupid))<0) return;
		if (sql_numtuples(sqr)>0) {
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=90%%>\r\n", config->colour_tabletrim);
				prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s NOWRAP><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
				prints(sid, "&nbsp;%s by ", str2html(sid, sql_getvalue(sqr, i, 3)));
				prints(sid, "%s", str2html(sid, sql_getvalue(sqr, i, 1)));
				if ((sqr2=sql_queryf(sid, "SELECT count(messageid) FROM gw_forumposts where forumid = %d", atoi(sql_getvalue(sqr, i, 0))))<0) return;
				prints(sid, " (%d posts)", atoi(sql_getvalue(sqr2, 0, 0)));
				sql_freeresult(sqr2);
				prints(sid, "</FONT></TD></TR>\n");
				prints(sid, "<TR><TD BGCOLOR=%s>", config->colour_fieldval);
				memset(body, 0, sizeof(body));
				snprintf(body, sizeof(body)-1, "%s", sql_getvalue(sqr, i, 4));
//				printline2(sid, 1, sql_getvalue(sqr, 0, 4));
				printline2(sid, 1, body);
				prints(sid, "\n<BR><BR>[<A HREF=%s/forums/msglist?forum=%s>Read More</A>]\n", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
				prints(sid, "</TD></TR></TABLE><P>\n");
			}
		} else {
			prints(sid, "There are no forums\n");
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

// heh..  if this code makes sense to you, please explain it to me..
// it's just a recursive depth first binary tree sorting algorithm, after all...
void fmessagelist(CONN *sid)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int forumid=0;
	int indent=0;
	int i;
	int j;
	int k;
	int x;
	int sqr;

	if (!(auth_priv(sid, "forums")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "FORUM")==NULL) return;
	forumid=atoi(getgetenv(sid, "FORUM"));
	forumview(sid, forumid);
	if ((sqr=sql_queryf(sid, "SELECT messageid, referenceid, postername, posttime, subject FROM gw_forumposts WHERE forumid = %d ORDER BY messageid ASC", forumid))<0) return;
	if (sql_numtuples(sqr)<1) {
		sql_freeresult(sqr);
		return;
	}
	btree=calloc(sql_numtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base; i<sql_numtuples(sqr); i++) {
		if (btree[atoi(sql_getvalue(sqr, i, 0))].printed) continue;
		if (atoi(sql_getvalue(sqr, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr, i, 0));
			btree[atoi(sql_getvalue(sqr, i, 0))].printed=1;
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
	for (i=0; i<sql_numtuples(sqr); i++) {
		for (j=i+1; j<sql_numtuples(sqr); j++) {
			if (ptree[j].depth<ptree[i].depth+1) break;
			if (ptree[j].depth>ptree[i].depth+1) continue;
			ptree[i].numchildren++;
		}
	}
	prints(sid, "<CENTER><BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=90%%>\n");
	prints(sid, "<TR BGCOLOR=%s STYLE='padding:2px'><TH ALIGN=left><FONT COLOR=%s>&nbsp;Topic&nbsp;</FONT></TH><TH ALIGN=left><FONT COLOR=%s>&nbsp;Author&nbsp;</FONT></TH><TH ALIGN=left><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext, config->colour_thtext);
	for (i=0; i<sql_numtuples(sqr); i++) {
		j=ptree[i].id;
		if (((float)i/2.0)==(int)(i/2))
			prints(sid, "<TR BGCOLOR=#F0F0F0>");
		else
			prints(sid, "<TR BGCOLOR=#E0E0E0>");
		prints(sid, "<TD NOWRAP ALIGN=TOP WIDTH=100%%>");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>");
		for (indent=0;indent<ptree[i].depth;indent++) {
			x=0;
			for (k=i-1;k>-1;k--) {
				if (indent==ptree[i].depth-1) {
					if (ptree[k].numchildren>1) {
						prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-t.gif HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					} else if (ptree[k].numchildren==1) {
						prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-l.gif HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					}
				} else if (indent<ptree[i].depth-1) {
					if (indent==ptree[k].depth) {
						if (ptree[k].numchildren>0) {
							prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-i.gif HEIGHT=21 WIDTH=9>");
							x=1;
							break;
						}
					}
				}
			}
			if (!x) {
				prints(sid, "<IMG BORDER=0 SRC=/groupware/images/blank.gif HEIGHT=21 WIDTH=9>");
			}
		}
		if (ptree[i].numchildren>0) {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-m.gif HEIGHT=21 WIDTH=9>");
		} else if (ptree[i].depth<1) {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-o.gif HEIGHT=21 WIDTH=9>");
		} else {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/forum-c.gif HEIGHT=21 WIDTH=9>");
		}
		prints(sid, "</TD><TD NOWRAP WIDTH=100%% style=\"cursor:hand\" onClick=\"window.location.href='%s/forums/msgread?forum=%d&message=%d'\">", sid->dat->in_ScriptName, forumid, atoi(sql_getvalue(sqr, j, 0)));
		prints(sid, "<A HREF=%s/forums/msgread?forum=%d&message=%d>", sid->dat->in_ScriptName, forumid, atoi(sql_getvalue(sqr, j, 0)));
		prints(sid, "<B>&nbsp;%s</B></A>&nbsp;", str2html(sid, sql_getvalue(sqr, j, 4)));
		prints(sid, "</TD></TR></TABLE></TD>");
		prints(sid, "<TD NOWRAP>%s&nbsp;&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, j, 2)));
		prints(sid, "<TD NOWRAP>%s ", time_sql2datetext(sid, sql_getvalue(sqr, j, 3)));
		prints(sid, "%s</TD></TR>\n", time_sql2timetext(sid, sql_getvalue(sqr, j, 3)));
	}
	prints(sid, "</TABLE>\n</CENTER>\n");
	free(ptree);
	free(btree);
	sql_freeresult(sqr);
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
	int sqr;
	char *pTemp;

	if (!(auth_priv(sid, "forums")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(query, 0, sizeof(query));
	if ((sqr=sql_query(sid, "SELECT max(forumid) FROM gw_forums"))<0) return;
	forumid=atoi(sql_getvalue(sqr, 0, 0))+1;
	sql_freeresult(sqr);
	if (forumid<1) forumid=1;
	if ((pTemp=getpostenv(sid, "FORUMGROUPID"))!=NULL) forumgroupid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)   strncpy(subject, pTemp, sizeof(subject)-1);
	if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL) {
		strncpy(query, pTemp, sizeof(query)-1);
		str2sqlbuf(sid, query, message, sizeof(message)-1);
	}
	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forums (forumid,obj_ctime,obj_mtime,obj_uid,obj_gid,obj_gperm,obj_operm,forumgroupid,postername,posttime,subject,message) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d','%s','%s',0,0,0,0,'%d','%s','%s','%s','", forumid, posttime, posttime, forumgroupid, sid->dat->user_username, posttime, str2sql(sid, subject));
	strncat(query, message, sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sql_update(sid, query)<0) return;
	prints(sid, "<CENTER>Forum %d added successfully</CENTER>\n", forumid);
	forumview(sid, forumid);
	db_log_activity(sid, 1, "forums", forumid, "insert", "%s - %s added forum %d", sid->dat->in_RemoteAddr, sid->dat->user_username, forumid);
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
	int sqr;
	char *pTemp;

	if (!(auth_priv(sid, "forums")&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
		str2sqlbuf(sid, query, message, sizeof(message)-1);
	}
	memset(query, 0, sizeof(query));
	if ((sqr=sql_queryf(sid, "SELECT max(messageid) FROM gw_forumposts where forumid = %d", forumid))<0) return;
	messageid=atoi(sql_getvalue(sqr, 0, 0))+1;
	if (messageid<1) messageid=1;
	sql_freeresult(sqr);
	t=time(NULL);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", gmtime(&t));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forumposts (messageid,obj_ctime,obj_mtime,obj_uid,obj_gid,obj_gperm,obj_operm,forumid,referenceid,postername,posttime,subject,message) values (%d, '%s', '%s', 0, 0, 0, 0, %d, %d, '%s', '%s', '%s', '", messageid, posttime, posttime, forumid, referenceid, sid->dat->user_username, posttime, str2sql(sid, subject));
	strncat(query, message, sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sql_update(sid, query)<0) return;
	prints(sid, "<CENTER>Message %d added successfully</CENTER>\n", messageid);
	fmessageview(sid, forumid, messageid);
	db_log_activity(sid, 1, "forumposts", forumid, "insert", "%s - %s added message %d on forum %d", sid->dat->in_RemoteAddr, sid->dat->user_username, messageid, forumid);
	return;
}

void mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
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
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_forums", "FORUMS", "/forums/list", "mod_main", "/forums/", mod_main)!=0) return -1;
	return 0;
}
