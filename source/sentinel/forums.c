/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

typedef struct {
	int lastref;
	int printed;
} _btree;
typedef struct {
	int id;
	int depth;
	int numchildren;
} _ptree;

void forumsave(int sid);
void fmessagesave(int sid);

void forumview(int sid, int forumid)
{
	int sqr;

	if (auth(sid, "forums")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQueryf("SELECT * FROM gw_forums WHERE forumid = %d", forumid))<0) return;
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP>[%s] <B>%s</B>", COLOR_TRIM, sqlGetvalue(sqr, 0, 0), sqlGetvalue(sqr, 0, 5));
	prints(" by <B>%s</B> on ", sqlGetvalue(sqr, 0, 3));
	prints("%s ", sqldate2text(sqlGetvalue(sqr, 0, 4)));
	prints("%s\n", sqltime2text(sqlGetvalue(sqr, 0, 4)));
	prints("</TD></TR>\n<TR><TD BGCOLOR=%s>\n", COLOR_FTEXT);
	prints("<PRE>");
	printline(sqlGetvalue(sqr, 0, 6));
	prints("&nbsp;</PRE>\n");
	prints("<FONT SIZE=2>\n[<A HREF=%s/forummsglist?forum=%d>Top</A>]", conn[sid].dat->in_ScriptName, forumid);
	prints("&nbsp;[<A HREF=%s/forummsgpost?forum=%d>Reply</A>]", conn[sid].dat->in_ScriptName, forumid);
	prints("</FONT>\n");
	prints("</TD></TR></TABLE><P>\n");
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void fmessageview(int sid, int forumid, int messageid)
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

	if (auth(sid, "forums")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQueryf("SELECT * FROM gw_forumposts WHERE forumid = %d ORDER BY messageid ASC", forumid))<0) return;
	if (sqlNumtuples(sqr)<1) return;
	btree=calloc(sqlNumtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sqlNumtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base; i<sqlNumtuples(sqr); i++) {
		if (btree[atoi(sqlGetvalue(sqr, i, 0))].printed) continue;
		if (atoi(sqlGetvalue(sqr, i, 4))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sqlGetvalue(sqr, i, 0));
			btree[atoi(sqlGetvalue(sqr, i, 0))].printed=1;
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
	if (base<sqlNumtuples(sqr)) {
		goto widthloop;
	}
	for (i=0; i<sqlNumtuples(sqr); i++) {
		if (ptree[i].id+1==messageid) {
			if (i>0) {
				previous=ptree[i-1].id+1;
			}
			if (i<sqlNumtuples(sqr)-1) {
				next=ptree[i+1].id+1;
			}
			break;
		}
	}
	free(ptree);
	free(btree);
	sqlFreeconnect(sqr);
	if ((sqr=sqlQueryf("SELECT * FROM gw_forumposts WHERE messageid = %d and forumid = %d", messageid, forumid))<0) return;
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%\n");
	prints("<TR><TD BGCOLOR=%s NOWRAP>", COLOR_TRIM);
	prints("<B>%s</B> by ", sqlGetvalue(sqr, 0, 7));
	prints("<B>%s</B> on ", sqlGetvalue(sqr, 0, 5));
	prints("%s ", sqldate2text(sqlGetvalue(sqr, 0, 6)));
	prints("%s", sqltime2text(sqlGetvalue(sqr, 0, 6)));
	prints("</TD></TR>\n<TR><TD BGCOLOR=%s>\n", COLOR_FTEXT);
	prints("<PRE>");
	printline(sqlGetvalue(sqr, 0, 8));
	prints("&nbsp;</PRE>\n");
	prints("<FONT SIZE=2>\n[<A HREF=%s/forummsglist?forum=%d>Top</A>]", conn[sid].dat->in_ScriptName, forumid);
	if (previous>0) {
		prints("&nbsp;[<A HREF=%s/forummsgread?forum=%d&message=%d>Previous</A>]", conn[sid].dat->in_ScriptName, forumid, previous);
	} else {
		prints("&nbsp;[Previous]");
	}
	if (next>0) {
		prints("&nbsp;[<A HREF=%s/forummsgread?forum=%d&message=%d>Next</A>]", conn[sid].dat->in_ScriptName, forumid, next);
	} else {
		prints("&nbsp;[Next]");
	}
//	prints("&nbsp;[<A HREF=%s/forummsgpost?forum=%d&reference=%d>Reply</A>]", conn[sid].dat->in_ScriptName, forumid, messageid);
	prints("</FONT>\n");
	prints("</TD></TR></TABLE><P>\n");
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void forumpost(int sid)
{
	char message[8192];
	char subject[64];
	char submit[10];
	char *pTemp;

	if (auth(sid, "forums")<3) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(submit, 0, sizeof(submit));
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if ((pTemp=getpostenv(sid, "SUBMIT"))!=NULL) {
			strncpy(submit, pTemp, sizeof(submit)-1);
			if (strcmp(submit, "Save")==0) {
				forumsave(sid);
				return;
			}
		}
		if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL)
			strncpy(message, pTemp, sizeof(message)-1);
		if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)
			strncpy(subject, pTemp, sizeof(subject)-1);
		if (strcmp(submit, "Preview")==0) {
			prints("<CENTER>\n");
			prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
			prints("<TR><TD BGCOLOR=%s NOWRAP>\n", COLOR_TRIM);
			prints("<B>%s </B> by <B>%s</B>\n", subject, conn[sid].dat->in_username);
			prints("</TD></TR><TR BGCOLOR=%s><TD>\n", COLOR_FTEXT);
			prints("<PRE>");
			printline(message);
			prints("&nbsp;</PRE>\n");
			prints("</TD></TR></TABLE><P>\n");
			prints("</CENTER>\n");
		}
	}
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/forumpost NAME=forumpost>\n", conn[sid].dat->in_ScriptName);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>New Forum</TD></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><B>Subject</B></TD><TD><INPUT TYPE=TEXT NAME=subject VALUE='%s' SIZE=50></TD></TR>\n", COLOR_EDITFORM, subject);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA WRAP=virtual NAME=message ROWS=10 COLS=50>", COLOR_EDITFORM);
	printline(message);
	prints("</TEXTAREA></TD></TR>\n");
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Preview'>\n");
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.forumpost.subject.focus();\n// -->\n</SCRIPT>\n");
}

void fmessagepost(int sid)
{
	char message[8192];
	char subject[64];
	char submit[10];
	int forumid=0;
	int referenceid=0;
	int sqr;
	char *pTemp;

	if (auth(sid, "forums")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(message, 0, sizeof(message));
	memset(subject, 0, sizeof(subject));
	memset(submit, 0, sizeof(submit));
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if ((pTemp=getpostenv(sid, "SUBMIT"))!=NULL) {
			strncpy(submit, pTemp, sizeof(submit)-1);
			if (strcmp(submit, "Save")==0) {
				fmessagesave(sid);
				return;
			}
		}
		if ((pTemp=getpostenv(sid, "FORUM"))!=NULL)
			forumid=atoi(pTemp);
		if ((pTemp=getpostenv(sid, "REFERENCE"))!=NULL)
			referenceid=atoi(pTemp);
		if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL)
			strncpy(message, pTemp, sizeof(message)-1);
		if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)
			strncpy(subject, pTemp, sizeof(subject)-1);
		if (strcmp(submit, "Preview")==0) {
			prints("<CENTER>\n");
			prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
			prints("<TR><TD BGCOLOR=%s NOWRAP>\n", COLOR_TRIM);
			prints("<B>%s</B> by <B>%s</B>\n", subject, conn[sid].dat->in_username);
			prints("</TD></TR><TR><TD BGCOLOR=%s>\n", COLOR_FTEXT);
			prints("<PRE>");
			printline(message);
			prints("&nbsp;</PRE>\n");
			prints("</TD></TR></TABLE><P>\n");
			prints("</CENTER>\n");
		}
	} else {
		if (getgetenv(sid, "FORUM")!=NULL) {
			forumid=atoi(getgetenv(sid, "FORUM"));
		}
		if (getgetenv(sid, "MESSAGE")!=NULL) {
			referenceid=atoi(getgetenv(sid, "MESSAGE"));
			fmessageview(sid, forumid, referenceid);
		} else if (getgetenv(sid, "REFERENCE")!=NULL) {
			referenceid=atoi(getgetenv(sid, "REFERENCE"));
			forumview(sid, forumid);
		}
	}
	if ((sqr=sqlQueryf("SELECT subject FROM gw_forumposts WHERE messageid = %d and forumid = %d", referenceid, forumid))<0) return;
	if (sqlNumtuples(sqr)>0) {
		if (strncasecmp(sqlGetvalue(sqr, 0, 0), "RE:", 3)!=0) {
			snprintf(subject, sizeof(subject)-1, "Re: %s", sqlGetvalue(sqr, 0, 0));
		} else {
			snprintf(subject, sizeof(subject)-1, "%s", sqlGetvalue(sqr, 0, 0));
		}
	}
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/forummsgpost NAME=msgpost>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=forum VALUE=%d>\n", forumid);
	prints("<INPUT TYPE=hidden NAME=reference VALUE=%d>\n", referenceid);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>New Message</TD></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><B>Subject</B></TD><TD><INPUT TYPE=TEXT NAME=subject VALUE='%s' SIZE=40></TD></TR>\n", COLOR_EDITFORM, subject);
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><TEXTAREA WRAP=virtual NAME=message ROWS=10 COLS=50>", COLOR_EDITFORM);
	printline(message);
	prints("</TEXTAREA></TD></TR>\n");
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Preview'>\n");
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.msgpost.subject.focus();\n// -->\n</SCRIPT>\n");
}

void forumlist(int sid, int longlist)
{
	int i;
	int sqr;

	if (auth(sid, "forums")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	prints("<CENTER>\n");
	if (longlist==0) {
		if ((sqr=sqlQuery("SELECT * FROM gw_forums ORDER BY forumid ASC"))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
			prints("<TR BGCOLOR=%s><TH ALIGN=left>Forum List</TD></TR>\n", COLOR_TRIM);
			for (i = 0;i<sqlNumtuples(sqr);i++) {
				prints("<TR BGCOLOR=%s><TD NOWRAP>", COLOR_FTEXT);
				prints("[%s] <B><A HREF=%s/forummsglist?forum=%s>", sqlGetvalue(sqr, i, 0), conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
				prints("%s&nbsp;</A></B> by ", sqlGetvalue(sqr, i, 5));
				prints("<B>%s</B> on ", sqlGetvalue(sqr, i, 3));
				prints("%s ", sqldate2text(sqlGetvalue(sqr, i, 4)));
				prints("%s\n</TD></TR>\n", sqltime2text(sqlGetvalue(sqr, i, 4)));
			}
			prints("</TABLE>\n");
		} else {
			prints("There are no forums\n");
		}
	} else {
		if ((sqr=sqlQuery("SELECT * FROM gw_forums ORDER BY forumid DESC"))<0) return;
		if (sqlNumtuples(sqr)>0) {
			for (i=0;i<sqlNumtuples(sqr);i++) {
				prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%\n");
				prints("<TR><TD BGCOLOR=%s NOWRAP>", COLOR_TRIM);
				prints("[%s] <B><A HREF=%s/forummsglist?forum=%s>", sqlGetvalue(sqr, i, 0), conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
				prints("%s&nbsp;</A> ", sqlGetvalue(sqr, i, 6));
				prints("[%s]</B> by ", sqlGetvalue(sqr, i, 5));
				prints("<B>%s</B> on ", sqlGetvalue(sqr, i, 3));
				prints("%s ", sqldate2text(sqlGetvalue(sqr, 0, 4)));
				prints("%s</TD></TR>\n", sqltime2text(sqlGetvalue(sqr, 0, 4)));
				prints("<TR><TD BGCOLOR=%s>", COLOR_FTEXT);
				prints("%s&nbsp;<BR><BR>\n", sqlGetvalue(sqr, i, 7));
				prints("[<A HREF=%s/forummsglist?forum=%s>Read More</A>]\n", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
				prints("</TD></TR></TABLE><P>\n");
			}
		} else {
			prints("There are no forums\n");
		}
	}
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

// heh..  if this code makes sense to you, please explain it to me..
// it's just a recursive depth first binary tree sorting algorithm, after all...
void fmessagelist(int sid)
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

	if (auth(sid, "forums")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (getgetenv(sid, "FORUM")==NULL) return;
	forumid=atoi(getgetenv(sid, "FORUM"));
	forumview(sid, forumid);
	if ((sqr=sqlQueryf("SELECT * FROM gw_forumposts WHERE forumid = %d ORDER BY messageid ASC", forumid))<0) return;
	if (sqlNumtuples(sqr)<1) return;
	btree=calloc(sqlNumtuples(sqr)+2, sizeof(_btree));
	ptree=calloc(sqlNumtuples(sqr)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base; i<sqlNumtuples(sqr); i++) {
		if (btree[atoi(sqlGetvalue(sqr, i, 0))].printed) continue;
		if (atoi(sqlGetvalue(sqr, i, 4))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sqlGetvalue(sqr, i, 0));
			btree[atoi(sqlGetvalue(sqr, i, 0))].printed=1;
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
	if (base<sqlNumtuples(sqr)) {
		goto widthloop;
	}
	for (i=0; i<sqlNumtuples(sqr); i++) {
		for (j=i+1; j<sqlNumtuples(sqr); j++) {
			if (ptree[j].depth<ptree[i].depth+1) break;
			if (ptree[j].depth>ptree[i].depth+1) continue;
			ptree[i].numchildren++;
		}
	}
	prints("<CENTER><BR>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=90%%>\n");
	prints("<TR BGCOLOR=%s><TH ALIGN=left>Topic</TH><TH ALIGN=left>Author</TH><TH ALIGN=left>Date</TH></TR>", COLOR_TRIM);
	for (i=0; i<sqlNumtuples(sqr); i++) {
		j=ptree[i].id;
		if (((float)i/2.0)==(int)(i/2))
			prints("<TR BGCOLOR=#F0F0F0>");
		else
			prints("<TR BGCOLOR=#E0E0E0>");
		prints("<TD NOWRAP ALIGN=TOP WIDTH=100%%>");
		prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>");
		for (indent=0;indent<ptree[i].depth;indent++) {
			x=0;
			for (k=i-1;k>-1;k--) {
				if (indent==ptree[i].depth-1) {
					if (ptree[k].numchildren>1) {
						prints("<IMG BORDER=0 SRC=/sentinel/images/forum-t.gif>");
						ptree[k].numchildren--;
						x=1;
						break;
					} else if (ptree[k].numchildren==1) {
						prints("<IMG BORDER=0 SRC=/sentinel/images/forum-l.gif>");
						ptree[k].numchildren--;
						x=1;
						break;
					}
				} else if (indent<ptree[i].depth-1) {
					if (indent==ptree[k].depth) {
						if (ptree[k].numchildren>0) {
							prints("<IMG BORDER=0 SRC=/sentinel/images/forum-i.gif>");
							x=1;
							break;
						}
					}
				}
			}
			if (!x) {
				prints("<IMG BORDER=0 SRC=/sentinel/images/blank.gif>");
			}
		}
		if (ptree[i].numchildren>0) {
			prints("<IMG BORDER=0 SRC=/sentinel/images/forum-m.gif>");
		} else if (ptree[i].depth<1) {
			prints("<IMG BORDER=0 SRC=/sentinel/images/forum-o.gif>");
		} else {
			prints("<IMG BORDER=0 SRC=/sentinel/images/forum-c.gif>");
		}
		prints("</TD><TD NOWRAP>");
		prints("<A HREF=%s/forummsgread?forum=%d&message=%s>", conn[sid].dat->in_ScriptName, forumid, sqlGetvalue(sqr, j, 0));
		prints("<B>&nbsp;%s&nbsp;</B></A>", sqlGetvalue(sqr, j, 7));
		prints("</TD></TR></TABLE></TD>");
		prints("<TD NOWRAP>%s&nbsp;&nbsp;</TD>", sqlGetvalue(sqr, j, 5));
		prints("<TD NOWRAP>%s ", sqldate2text(sqlGetvalue(sqr, j, 6)));
		prints("%s</TD></TR>\n", sqltime2text(sqlGetvalue(sqr, j, 6)));
	}
	prints("</TABLE>\n</CENTER>\n<BR>\n");
	free(ptree);
	free(btree);
	sqlFreeconnect(sqr);
	return;
}

void forumsave(int sid)
{
	char query[8192];
	time_t t;
	int forumid;
	char posttime[100];
	char subject[64];
	char message[8192];
	int sqr;
	char *pTemp;

	if (auth(sid, "forums")<3) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(subject, 0, sizeof(subject));
	memset(message, 0, sizeof(message));
	if ((sqr=sqlQuery("SELECT max(forumid) FROM gw_forums"))<0) return;
	forumid=atoi(sqlGetvalue(sqr, 0, 0))+1;
	sqlFreeconnect(sqr);
	if (forumid<1) forumid=1;
	if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)
		strncpy(subject, pTemp, sizeof(subject)-1);
	if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL)
		strncpy(message, pTemp, sizeof(message)-1);
	t=time((time_t*)0);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", localtime(&t));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forums (forumid,cdatetime,mdatetime,postername,posttime,subject,message) values ('%d','%s','%s','%s','%s','%s','", forumid, posttime, posttime, conn[sid].dat->in_username, posttime, str2sql(subject));
	strncat(query, str2sql(message), sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sqlUpdate(query)<0) return;
	prints("<CENTER>Forum %d added successfully</CENTER><BR>\n", forumid);
	forumview(sid, forumid);
	logaccess(1, "%s - %s added forum %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, forumid);
	return;
}

void fmessagesave(int sid)
{
	char query[8192];
	char message[8192];
	char subject[64];
	char posttime[100];
	int forumid=0;
	int messageid=0;
	int referenceid=0;
	time_t t;
	int sqr;
	char *pTemp;

	if (auth(sid, "forums")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(subject, 0, sizeof(subject));
	memset(message, 0, sizeof(message));
	if ((pTemp=getpostenv(sid, "FORUM"))!=NULL)
		forumid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "REFERENCE"))!=NULL)
		referenceid=atoi(pTemp);
	if ((pTemp=getpostenv(sid, "SUBJECT"))!=NULL)
		strncpy(subject, pTemp, sizeof(subject)-1);
	if ((pTemp=getpostenv(sid, "MESSAGE"))!=NULL)
		strncpy(message, pTemp, sizeof(message)-1);
	if ((sqr=sqlQueryf("SELECT max(messageid) FROM gw_forumposts where forumid = %d", forumid))<0) return;
	messageid=atoi(sqlGetvalue(sqr, 0, 0))+1;
	if (messageid<1) messageid=1;
	sqlFreeconnect(sqr);
	t=time((time_t*)0);
	strftime(posttime, sizeof(posttime), "%Y-%m-%d %H:%M:%S", localtime(&t));
	snprintf(query, sizeof(query)-1, "INSERT INTO gw_forumposts (messageid,cdatetime,mdatetime,forumid,referenceid,postername,posttime,subject,message) values (%d, '%s', '%s', %d, %d, '%s', '%s', '%s', '", messageid, posttime, posttime, forumid, referenceid, conn[sid].dat->in_username, posttime, str2sql(subject));
	strncat(query, str2sql(message), sizeof(query)-strlen(query)-3);
	strncat(query, "')", sizeof(query)-strlen(query)-1);
	if (sqlUpdate(query)<0) return;
	prints("<CENTER>Message %d added successfully</CENTER><BR>\n", messageid);
	fmessageview(sid, forumid, messageid);
	logaccess(1, "%s - %s added message %d on forum %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, messageid, forumid);
	return;
}

void forummain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Forums");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch06-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/forumlist>VIEW FORUMS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/forumpost>ADD FORUM</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	if (strncmp(conn[sid].dat->in_RequestURI, "/forummsgread",      13)==0)
		fmessagepost(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forumpost",    10)==0)
		forumpost(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forummsgpost", 13)==0)
		fmessagepost(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forumlist",    10)==0)
		forumlist(sid, 0);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forummsglist", 13)==0)
		fmessagelist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forumsave",    10)==0)
		forumsave(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forummsgsave", 13)==0)
		fmessagesave(sid);
	printfooter(sid);
	return;
}
