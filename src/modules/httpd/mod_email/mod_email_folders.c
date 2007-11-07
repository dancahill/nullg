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
#include "mod_email.h"

void wmfolder_edit(CONN *sid)
{
/*
	char foldername[51];
	int folderid;
	int accountid;
	int parentid;
	char *ptemp;
	SQLRES sqr;

	if (!(auth_priv(sid, "email")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	memset(foldername, 0, sizeof(foldername));
	parentid=0;
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if (strncmp(sid->dat->in_RequestURI, "/mail/folders/editnew", 21)==0) {
		folderid=0;
		parentid=0;
		snprintf(foldername, sizeof(foldername)-1, "New Folder");
	} else {
		if ((ptemp=getgetenv(sid, "FOLDERID"))==NULL) return;
		folderid=atoi(ptemp);
		if (sql_queryf(&sqr, "SELECT mailfolderid, accountid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d and mailfolderid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, accountid, folderid)<0) return;
		if (sql_numtuples(&sqr)==1) {
			folderid=atoi(sql_getvalue(&sqr, 0, 0));
			accountid=atoi(sql_getvalue(&sqr, 0, 1));
			parentid=atoi(sql_getvalue(&sqr, 0, 2));
			snprintf(foldername, sizeof(foldername)-1, "%s", sql_getvalue(&sqr, 0, 3));
		}
		sql_freeresult(&sqr);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<BR /><CENTER>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/folders/save NAME=folderedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\r\n");
	prints(sid, "<INPUT TYPE=hidden NAME=accountid VALUE='%d'>\r\n", accountid);
	prints(sid, "<INPUT TYPE=hidden NAME=folderid VALUE='%d'>\r\n", folderid);
	if (folderid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Mail Folder '%s'</TH></TR>\r\n", foldername);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Mail Folder</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP COLSPAN=2>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=parentid style='width:217px'%s>\r\n", (folderid<1)||(folderid>5)?"":" DISABLED");
	htselect_mailfolder(sid, parentid, 1, 1);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=30 style='width:217px'%s></TD></TR>\r\n", str2html(sid, foldername), (folderid<1)||(folderid>5)?"":" DISABLED");
	prints(sid, "</TABLE></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	if ((folderid<1)||(folderid>5)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
		if (folderid>5) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
		}
	}
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
	if ((folderid<1)||(folderid>5)) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.folderedit.foldername.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	}
*/
	return;
}

void wmfolder_list(CONN *sid, int accountid)
{
/*
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j, k;
	int x;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "email")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (sql_queryf(&sqr1, "SELECT mailfolderid, parentfolderid, foldername FROM gw_email_folders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->uid, accountid)<0) return;
	if (sql_numtuples(&sqr1)<1) {
		sql_freeresult(&sqr1);
		return;
	}
	if (sql_queryf(&sqr2, "SELECT folder, COUNT(mailheaderid), SUM(size) FROM gw_email_headers WHERE obj_uid = %d AND accountid = %d AND status != 'd' GROUP BY folder", sid->dat->uid, accountid)<0) {
		sql_freeresult(&sqr1);
		return;
	}
	btree=calloc(sql_numtuples(&sqr1)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(&sqr1)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(&sqr1);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(&sqr1, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(&sqr1, i, 0));
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
	if (base<sql_numtuples(&sqr1)) {
		goto widthloop;
	}
	for (i=0; i<sql_numtuples(&sqr1); i++) {
		for (j=i+1; j<sql_numtuples(&sqr1); j++) {
			if (ptree[j].depth<ptree[i].depth+1) break;
			if (ptree[j].depth>ptree[i].depth+1) continue;
			ptree[i].numchildren++;
		}
	}
	prints(sid, "<CENTER><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>");
	for (i=0; i<sql_numtuples(&sqr1); i++) {
		j=ptree[i].id;
		prints(sid, "<TR><TD NOWRAP WIDTH=100%%>");
		prints(sid, "&nbsp;<A HREF=%s/mail/folders/edit?accountid=%d&folderid=%d>edit</A>&nbsp;", sid->dat->in_ScriptName, accountid, atoi(sql_getvalue(&sqr1, j, 0)));
		for (indent=0;indent<ptree[i].depth;indent++) {
			x=0;
			for (k=i-1;k>-1;k--) {
				if (indent==ptree[i].depth-1) {
					if (ptree[k].numchildren>1) {
						prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-t.png HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					} else if (ptree[k].numchildren==1) {
						prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-l.png HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					}
				} else if (indent<ptree[i].depth-1) {
					if (indent==ptree[k].depth) {
						if (ptree[k].numchildren>0) {
							prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-i.png HEIGHT=21 WIDTH=9>");
							x=1;
							break;
						}
					}
				}
			}
			if (!x) {
				prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/blank.png HEIGHT=21 WIDTH=9>");
			}
		}
		if (ptree[i].numchildren>0) {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-m.png HEIGHT=21 WIDTH=9>");
		} else if (ptree[i].depth<1) {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-o.png HEIGHT=21 WIDTH=9>");
		} else {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/nullgs/images/tree-c.png HEIGHT=21 WIDTH=9>");
		}
		prints(sid, "&nbsp;<A HREF=%s/mail/%s", sid->dat->in_ScriptName, (sid->dat->menustyle>0)?"main":"list");
		prints(sid, "?accountid=%d&folderid=%d><B>%s</B></A> ", accountid, atoi(sql_getvalue(&sqr1, j, 0)), str2html(sid, sql_getvalue(&sqr1, j, 2)));
		prints(sid, "</TD><TD ALIGN=right NOWRAP>");
		for (k=0;k<sql_numtuples(&sqr2);k++) {
			if (atoi(sql_getvalue(&sqr2, k, 0))==atoi(sql_getvalue(&sqr1, j, 0))) {
				prints(sid, "(%d)", atoi(sql_getvalue(&sqr2, k, 1)));
				prints(sid, "</TD><TD ALIGN=right NOWRAP>");
				prints(sid, "(%1.1f M)", atof(sql_getvalue(&sqr2, k, 2))/1048576.0f);
				k=-1;
				break;
			}
		}
		if (k>-1) {
			prints(sid, "(0)</TD><TD ALIGN=right NOWRAP>(0.0 M)");
		}
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD></TD><TD ALIGN=right NOWRAP>");
	x=0;
	for (i=0;i<sql_numtuples(&sqr2);i++) {
		x+=atoi(sql_getvalue(&sqr2, i, 1));
	}
	prints(sid, "<B>(%d)</B></TD><TD ALIGN=right NOWRAP>", x);
	x=0;
	for (i=0;i<sql_numtuples(&sqr2);i++) {
		x+=atoi(sql_getvalue(&sqr2, i, 2));
	}
	prints(sid, "<B>(%1.1f M)</B></TD></TR>\r\n", x/1048576.0f);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<A HREF=%s/mail/folders/editnew?accountid=%d><B>New Folder</B></A>", sid->dat->in_ScriptName, accountid);
	prints(sid, "</CENTER>\r\n");
	free(ptree);
	free(btree);
	sql_freeresult(&sqr2);
	sql_freeresult(&sqr1);
*/
	return;
}

void wmfolder_save(CONN *sid)
{
/*
	char query[2048];
	char curdate[40];
	char foldername[51];
	char *ptemp;
	time_t t;
	int accountid;
	int folderid;
	int parentid;
	SQLRES sqr;

	prints(sid, "<BR />\r\n");
	if (!(auth_priv(sid, "email")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	memset(foldername, 0, sizeof(foldername));
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "FOLDERID"))==NULL) return;
	folderid=atoi(ptemp);
	parentid=0;
	if ((folderid<1)||(folderid>5)) {
		if ((ptemp=getpostenv(sid, "PARENTID"))!=NULL) parentid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "FOLDERNAME"))!=NULL) snprintf(foldername, sizeof(foldername)-1, "%s", ptemp);
	} else {
		parentid=0;
	}
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (parentid==folderid) parentid=0;
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (folderid<6) {
			prints(sid, "<CENTER><B>This folder cannot be deleted.</B></CENTER>\r\n");
			return;
		}
		if (sql_queryf(&sqr, "SELECT mailheaderid FROM gw_email_headers WHERE obj_uid = %d AND accountid = %d AND folder = %d", sid->dat->uid, accountid, folderid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\r\n");
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT mailfolderid FROM gw_email_folders WHERE obj_uid = %d AND accountid = %d AND parentfolderid = %d", sid->dat->uid, accountid, folderid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\r\n");
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (sql_updatef("DELETE FROM gw_email_folders WHERE accountid = %d AND mailfolderid = %d", accountid, folderid)<0) return;
		prints(sid, "<CENTER>Mail Folder %d deleted successfully</CENTER><BR />\r\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\r\n", sid->dat->in_ScriptName, accountid);
	} else if (folderid==0) {
		if (strlen(foldername)<1) {
			prints(sid, "<CENTER>Folder name cannot be blank</CENTER><BR />\r\n");
			return;
		}
		if (sql_queryf(&sqr, "SELECT max(mailfolderid) FROM gw_email_folders where accountid = %d", accountid)<0) return;
		folderid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (folderid<11) folderid=11;
		strcpy(query, "INSERT INTO gw_email_folders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '%d', '0', '0', '%d', ", folderid, curdate, curdate, sid->dat->uid, sid->dat->did, accountid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, foldername));
		if (sql_update(query)<0) return;
		if (wmfolder_testcreate(sid, accountid, folderid)<0) return;
		prints(sid, "<CENTER>Mail folder %d added successfully</CENTER><BR />\r\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\r\n", sid->dat->in_ScriptName, accountid);
	} else {
		if (folderid>5) {
			snprintf(query, sizeof(query)-1, "UPDATE gw_email_folders SET obj_mtime = '%s', ", curdate);
			strncatf(query, sizeof(query)-strlen(query)-1, "parentfolderid = '%d', ", parentid);
			strncatf(query, sizeof(query)-strlen(query)-1, "foldername = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, foldername));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE mailfolderid = %d AND obj_uid = %d AND accountid = %d", folderid, sid->dat->uid, accountid);
			if (sql_update(query)<0) return;
		}
		if (wmfolder_testcreate(sid, accountid, folderid)<0) return;
		prints(sid, "<CENTER>Mail folder %d modified successfully</CENTER><BR />\r\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\r\n", sid->dat->in_ScriptName, accountid);
	}
*/
	return;
}