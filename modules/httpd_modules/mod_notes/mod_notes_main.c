/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_notes.h"

void htselect_notefilter(CONN *sid, int selected, char *baseuri)
{
	char *option[]={ "All", "Personal", "Calls", "Contacts", "Events", "Notes", "Orders", "Projects", "Tasks", "Users" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	int table;

	if (selected<1) {
		selected=sid->dat->user_uid;
	}
	if ((ptemp=getgetenv(sid, "TABLE"))!=NULL) {
		table=atoi(ptemp);
	} else {
		table=1;
	}
	prints(sid, "<FORM METHOD=GET NAME=notefilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE domainid = %d ORDER BY username ASC", sid->dat->user_did))<0) return;
	prints(sid, "function go1() {\r\n");
	prints(sid, "	location=document.notefilter.userid.options[document.notefilter.userid.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&table=%d", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, i, 0)), table);
		prints(sid, "\"%s>%s');\n", atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "function go2() {\r\n");
	prints(sid, "	location=document.notefilter.table.options[document.notefilter.table.selectedIndex].value\r\n");
	prints(sid, "}\r\n");
	prints(sid, "document.write('<SELECT NAME=table onChange=\"go2()\">');\r\n");
	for (i=0;i<9;i++) {
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&table=%d", sid->dat->in_ScriptName, baseuri, selected, i);
		prints(sid, "\"%s>%s');\n", i==table?" SELECTED":"", option[i]);
	}
	prints(sid, "document.write('</SELECT>');\r\n");
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=table>\r\n");
	prints(sid, "<OPTION VALUE='0'%s>%s\n", table==0?" SELECTED":"", option[0]);
	prints(sid, "<OPTION VALUE='1'%s>%s\n", table==1?" SELECTED":"", option[1]);
	prints(sid, "<OPTION VALUE='2'%s>%s\n", table==2?" SELECTED":"", option[2]);
	prints(sid, "<OPTION VALUE='3'%s>%s\n", table==3?" SELECTED":"", option[3]);
	prints(sid, "<OPTION VALUE='4'%s>%s\n", table==4?" SELECTED":"", option[4]);
	prints(sid, "<OPTION VALUE='5'%s>%s\n", table==5?" SELECTED":"", option[5]);
	prints(sid, "<OPTION VALUE='6'%s>%s\n", table==6?" SELECTED":"", option[6]);
	prints(sid, "<OPTION VALUE='7'%s>%s\n", table==7?" SELECTED":"", option[7]);
	prints(sid, "<OPTION VALUE='8'%s>%s\n", table==8?" SELECTED":"", option[8]);
	prints(sid, "<OPTION VALUE='9'%s>%s\n", table==9?" SELECTED":"", option[9]);
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(sqr);
	return;
}

void notes_sublist(CONN *sid, char *table, int index, int colspan)
{
	int i;
	int sqr;

	if (auth_priv(sid, "admin")&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->user_did))<0) return;
	} else {
		if ((sqr=sql_queryf("SELECT noteid, notetitle, notetext FROM gw_notes WHERE tablename = '%s' AND tableindex = %d AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid ASC", table, index, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT COLSPAN=%d NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/notes/view?noteid=%d'\">", colspan, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/notes/view?noteid=%d>%s</A>&nbsp;</TD></TR>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
		}
	} else {
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD COLSPAN=%d NOWRAP STYLE='border-style:solid'>&nbsp;</TD></TR>\n", colspan);
	}
	sql_freeresult(sqr);
	return;
}

void notesedit(CONN *sid)
{
	REC_NOTE note;
	char *ptemp;
	int noteid;

	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/notes/editnew", 14)==0) {
		noteid=0;
		if (dbread_note(sid, 2, 0, &note)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "TABLE"))!=NULL) {
			snprintf(note.tablename, sizeof(note.tablename)-1, "%s", ptemp);
		} else {
			note.obj_gperm=1;
		}
		if ((ptemp=getgetenv(sid, "INDEX"))!=NULL) note.tableindex=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "NOTEID"))==NULL) return;
		noteid=atoi(ptemp);
		if (dbread_note(sid, 2, noteid, &note)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", noteid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/notes/save NAME=notesedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=noteid VALUE='%d'>\n", note.noteid);
	prints(sid, "<INPUT TYPE=hidden NAME=tablename VALUE='%s'>\n", note.tablename);
	prints(sid, "<INPUT TYPE=hidden NAME=tableindex VALUE='%d'>\n", note.tableindex);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	if (noteid>0) {
		prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/notes/view?noteid=%d>Note %d</TH></TR>\n", sid->dat->in_ScriptName, note.noteid, note.noteid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Note</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Note Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=notetitle  value=\"%s\" SIZE=50 MAXLENGTH=%d STYLE='width:100%%'></TD></TR>\n", str2html(sid, note.notetitle), sizeof(note.notetitle)-1);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP COLSPAN=2><B>&nbsp;Note&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=VIRTUAL NAME=notetext ROWS=15 COLS=70>");
	printline2(sid, 0, note.notetext);
	prints(sid, "</TEXTAREA></TD></TR>\n");
	if ((note.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<TR><TH ALIGN=center COLSPAN=2>Permissions</TH></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "admin")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, note.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "admin")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "admin"), note.obj_gid, sid->dat->user_did);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", note.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", note.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", note.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", note.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", note.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", note.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if (note.noteid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.notesedit.notetitle.focus();\n// -->\n</SCRIPT>\n");
}

void notesview(CONN *sid)
{
	REC_NOTE note;
	char *ptemp;
	int noteid;
	int sqr;
	time_t mdate;

	if ((ptemp=getgetenv(sid, "NOTEID"))==NULL) return;
	noteid=atoi(ptemp);
	prints(sid, "<BR>\r\n");
	if (dbread_note(sid, 1, noteid, &note)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", noteid);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Note %d", noteid);
	if (auth_priv(sid, "orders")&A_ADMIN) {
		prints(sid, " [<A HREF=%s/notes/edit?noteid=%d>edit</A>]", sid->dat->in_ScriptName, noteid);
	} else if ((note.obj_uid==sid->dat->user_uid)||((note.obj_gid==sid->dat->user_gid)&&(note.obj_gperm>1))||(note.obj_operm>1)) {
		prints(sid, " [<A HREF=%s/notes/edit?noteid=%d>edit</A>]", sid->dat->in_ScriptName, noteid);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Note Title</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\n", str2html(sid, note.notetitle));
	if (strcmp(note.tablename, "calls")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Call </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT callid, callname FROM gw_calls WHERE callid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/calls/view?callid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, 0, 0)));
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "contacts")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Contact </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/contacts/view?contactid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, 0, 0));
			prints(sid, "%s", str2html(sid, sql_getvalue(sqr, 0, 1)));
			if (strlen(sql_getvalue(sqr, 0, 1))&&strlen(sql_getvalue(sqr, 0, 2))) prints(sid, ", ");
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 2)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "events")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Event </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT eventid, eventname FROM gw_events WHERE eventid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/calendar/view?eventid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "notes")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Note </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT noteid, notetitle FROM gw_notes WHERE noteid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/notes/view?noteid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "orders")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Order </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT orderid, contactid FROM gw_orders WHERE orderid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/orders/view?orderid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", htview_contact(sid, atoi(sql_getvalue(sqr, 0, 1))));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "projects")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Task </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT projectid, projectname FROM gw_projects WHERE projectid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/projects/view?projectid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "tasks")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Task </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT taskid, taskname FROM gw_tasks WHERE taskid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/tasks/view?taskid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(note.tablename, "users")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>User </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE userid = %d", note.tableindex))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, note.tableindex);
			prints(sid, "%s</A>", str2html(sid, sql_getvalue(sqr, 0, 1)));
		}
		sql_freeresult(sqr);
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	mdate=note.obj_mtime;
	mdate+=time_tzoffset(sid, mdate);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Last Modified</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s (%s)</TD></TR>\n", time_unix2timetext(sid, mdate), time_unix2datetext(sid, mdate));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Note</B></TD></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'>");
	printline2(sid, 1, note.notetext);
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>Notes");
	prints(sid, " [<A HREF=%s/notes/editnew?table=notes&index=%d>new</A>]", sid->dat->in_ScriptName, note.noteid);
	prints(sid, "</FONT></TH></TR>\n");
	notes_sublist(sid, "notes", note.noteid, 2);
	prints(sid, "</TABLE>\n</CENTER>\n");
}

void noteslist(CONN *sid)
{
	char tablename[20];
	char *ptemp;
	int i;
	int userid;
	int offset=0;
	int sqr;
	int table;
	time_t mdate;

	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "TABLE"))!=NULL) {
		table=atoi(ptemp);
	} else {
		table=1;
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->user_uid;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_notefilter(sid, userid, "/notes/list");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\n");
	memset(tablename, 0, sizeof(tablename));
	switch(table) {
		case 0: { snprintf(tablename, sizeof(tablename)-1, "%%"); break; }
		case 1: { break; }
		case 2: { snprintf(tablename, sizeof(tablename)-1, "calls"); break; }
		case 3: { snprintf(tablename, sizeof(tablename)-1, "contacts"); break; }
		case 4: { snprintf(tablename, sizeof(tablename)-1, "events"); break; }
		case 5: { snprintf(tablename, sizeof(tablename)-1, "notes"); break; }
		case 6: { snprintf(tablename, sizeof(tablename)-1, "orders"); break; }
		case 7: { snprintf(tablename, sizeof(tablename)-1, "projects"); break; }
		case 8: { snprintf(tablename, sizeof(tablename)-1, "tasks"); break; }
		case 9: { snprintf(tablename, sizeof(tablename)-1, "users"); break; }
	}
	if (auth_priv(sid, "admin")&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT noteid, notetitle, obj_mtime, tablename, tableindex FROM gw_notes WHERE obj_uid = %d AND tablename like '%s' AND obj_did = %d ORDER BY noteid DESC", userid, tablename, sid->dat->user_did))<0) return;
	} else {
		if ((sqr=sql_queryf("SELECT noteid, notetitle, obj_mtime, tablename, tableindex FROM gw_notes WHERE obj_uid = %d AND tablename like '%s' AND (obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d ORDER BY noteid DESC", userid, tablename, sid->dat->user_uid, sid->dat->user_gid, sid->dat->user_did))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<BR>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=450 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Note Title&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Reference&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Last Modified&nbsp;</TH></TR>\n");
		for (i=offset;(i<sql_numtuples(sqr))&&(i<offset+sid->dat->user_maxlist);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/notes/view?noteid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/notes/view?noteid=%d>%s</A></TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
			if (strcasecmp(sql_getvalue(sqr, i, 3), "calls")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/calls/view?callid=%d>Call</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "contacts")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%d>Contact</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "events")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/calendar/view?eventid=%d>Event</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "notes")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/notes/view?noteid=%d>Note</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "orders")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/orders/view?orderid=%d>Order</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "projects")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/projects/view?projectid=%d>Project</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "tasks")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/tasks/view?taskid=%d>Task</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 3), "users")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/admin/useredit?userid=%d>User</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 4)));
			} else {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>None</TD>");
			}
			mdate=time_sql2unix(sql_getvalue(sqr, i, 2));
			mdate+=time_tzoffset(sid, mdate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s</TD></TR>\n", time_unix2datetext(sid, mdate));
		}
		prints(sid, "</TABLE>\n");
		if (sql_numtuples(sqr)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/notes/list?offset=%d&table=%d&userid=%d>Previous Page</A>]\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, table, userid);
			} else {
				prints(sid, "[Previous Page]\n");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(sqr)) {
				prints(sid, "[<A HREF=%s/notes/list?offset=%d&table=%d&userid=%d>Next Page</A>]\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, table, userid);
			} else {
				prints(sid, "[Next Page]\n");
			}
		}
	} else {
		prints(sid, "<B>No Notes found</B>\n");
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void notessave(CONN *sid)
{
	REC_NOTE note;
	char notebuffer[8193];
	char *ptemp;
	int noteid;

	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "NOTEID"))==NULL) return;
	memset(notebuffer, 0, sizeof(notebuffer));
	noteid=atoi(ptemp);
	prints(sid, "<BR>\r\n");
	if (dbread_note(sid, 2, noteid, &note)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "admin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) note.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) note.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "admin")&A_ADMIN)||(note.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) note.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) note.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "TABLENAME"))!=NULL) snprintf(note.tablename, sizeof(note.tablename)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "TABLEINDEX"))!=NULL) note.tableindex=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "NOTETITLE"))!=NULL) snprintf(note.notetitle, sizeof(note.notetitle)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "NOTETEXT"))!=NULL) {
		strncpy(notebuffer, ptemp, sizeof(notebuffer)-1);
		str2sql(note.notetext, sizeof(note.notetext)-1, notebuffer);
	}
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_notes WHERE noteid = %d", note.noteid)<0) return;
		prints(sid, "<CENTER>Note %d deleted successfully</CENTER><BR>\n", note.noteid);
		db_log_activity(sid, 1, "notes", note.noteid, "delete", "%s - %s deleted note %d", sid->dat->in_RemoteAddr, sid->dat->user_username, note.noteid);
		if (strcmp(note.tablename, "calls")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calls/view?callid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "contacts")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "events")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendar/view?eventid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "notes")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/notes/view?noteid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "orders")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/orders/view?orderid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "projects")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/projects/view?projectid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "tasks")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/tasks/view?taskid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else if (strcmp(note.tablename, "users")==0) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/useredit?userid=%d\">\n", sid->dat->in_ScriptName, note.tableindex);
		} else {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/notes/list\">\n", sid->dat->in_ScriptName);
		}
	} else if (note.noteid==0) {
		if ((note.noteid=dbwrite_note(sid, 0, &note))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Note %d added successfully</CENTER><BR>\n", note.noteid);
		db_log_activity(sid, 1, "notes", note.noteid, "insert", "%s - %s added note %d", sid->dat->in_RemoteAddr, sid->dat->user_username, note.noteid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/notes/view?noteid=%d\">\n", sid->dat->in_ScriptName, note.noteid);
	} else {
		if (dbwrite_note(sid, noteid, &note)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Note %d modified successfully</CENTER><BR>\n", note.noteid);
		db_log_activity(sid, 1, "notes", note.noteid, "modify", "%s - %s modified note %d", sid->dat->in_RemoteAddr, sid->dat->user_username, note.noteid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/notes/view?noteid=%d\">\n", sid->dat->in_ScriptName, note.noteid);
	}
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_NOTES);
	if (strncmp(sid->dat->in_RequestURI, "/notes/edit", 11)==0) {
		notesedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/notes/view", 11)==0) {
		notesview(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/notes/list", 11)==0) {
		noteslist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/notes/save", 11)==0) {
		notessave(sid);
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
		"mod_notes",		// mod_name
		3,			// mod_submenu
		"NOTEBOOK",		// mod_menuname
		"/notes/list",		// mod_menuuri
		"calendar",		// mod_menuperm
		"mod_main",		// fn_name
		"/notes/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	if (mod_export_function("mod_notes", "mod_notes_sublist", notes_sublist)!=0) return -1;
	return 0;
}
