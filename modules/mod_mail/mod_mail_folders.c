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
#include "mod_substub.h"
#include "mod_mail.h"

void wmfolder_edit(CONN *sid)
{
	char foldername[51];
	int folderid;
	int accountid;
	int parentid;
	char *ptemp;
	int sqr;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
		if ((sqr=sql_queryf(sid, "SELECT mailfolderid, accountid, parentfolderid, foldername FROM gw_mailfolders WHERE obj_uid = %d and accountid = %d and mailfolderid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->user_uid, accountid, folderid))<0) return;
		if (sql_numtuples(sqr)==1) {
			folderid=atoi(sql_getvalue(sqr, 0, 0));
			accountid=atoi(sql_getvalue(sqr, 0, 1));
			parentid=atoi(sql_getvalue(sqr, 0, 2));
			snprintf(foldername, sizeof(foldername)-1, "%s", sql_getvalue(sqr, 0, 3));
		}
		sql_freeresult(sqr);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/folders/save NAME=folderedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n");
	prints(sid, "<INPUT TYPE=hidden NAME=accountid VALUE='%d'>\n", accountid);
	prints(sid, "<INPUT TYPE=hidden NAME=folderid VALUE='%d'>\n", folderid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (folderid!=0) {
		prints(sid, "Mail Folder '%s'</FONT></TH></TR>\n", foldername);
	} else {
		prints(sid, "New Mail Folder</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Parent Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=parentid style='width:217px'%s>\n", config->colour_editform, (folderid<1)||(folderid>5)?"":" DISABLED");
	htselect_mailfolder(sid, parentid, 1);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Folder Name  &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=foldername value=\"%s\" SIZE=30 style='width:217px'%s></TD></TR>\n", config->colour_editform, str2html(sid, foldername), (folderid<1)||(folderid>5)?"":" DISABLED");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	if ((folderid<1)||(folderid>5)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
		if (folderid>5) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
		}
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	if ((folderid<1)||(folderid>5)) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.folderedit.foldername.focus();\n// -->\n</SCRIPT>\n");
	}
	return;
}

void wmfolder_list(CONN *sid, int accountid)
{
	_btree *btree;
	_ptree *ptree;
	int base=0;
	int depth=1;
	int indent=0;
	int i, j, k;
	int x;
	int sqr1, sqr2;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr1=sql_queryf(sid, "SELECT mailfolderid, parentfolderid, foldername FROM gw_mailfolders WHERE obj_uid = %d and accountid = %d ORDER BY parentfolderid ASC, foldername ASC", sid->dat->user_uid, accountid))<0) return;
	if (sql_numtuples(sqr1)<1) {
		sql_freeresult(sqr1);
		return;
	}
	if ((sqr2=sql_queryf(sid, "SELECT folder, COUNT(mailheaderid), SUM(size) FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND status != 'd' GROUP BY folder", sid->dat->user_uid, accountid))<0) {
		sql_freeresult(sqr1);
		return;
	}
	btree=calloc(sql_numtuples(sqr1)+2, sizeof(_btree));
	ptree=calloc(sql_numtuples(sqr1)+2, sizeof(_ptree));
	j=0;
	widthloop:
	for (i=base;i<sql_numtuples(sqr1);i++) {
		if (btree[i].printed) continue;
		if (atoi(sql_getvalue(sqr1, i, 1))==btree[depth].lastref) {
			ptree[j].id=i;
			ptree[j].depth=depth-1;
			j++;
			btree[depth+1].lastref=atoi(sql_getvalue(sqr1, i, 0));
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
	if (base<sql_numtuples(sqr1)) {
		goto widthloop;
	}
	for (i=0; i<sql_numtuples(sqr1); i++) {
		for (j=i+1; j<sql_numtuples(sqr1); j++) {
			if (ptree[j].depth<ptree[i].depth+1) break;
			if (ptree[j].depth>ptree[i].depth+1) continue;
			ptree[i].numchildren++;
		}
	}
	prints(sid, "<CENTER><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>");
	for (i=0; i<sql_numtuples(sqr1); i++) {
		j=ptree[i].id;
		prints(sid, "<TR BGCOLOR=#F0F0F0><TD NOWRAP WIDTH=100%%>");
		prints(sid, "&nbsp;<A HREF=%s/mail/folders/edit?accountid=%d&folderid=%d>edit</A>&nbsp;", sid->dat->in_ScriptName, accountid, atoi(sql_getvalue(sqr1, j, 0)));
		for (indent=0;indent<ptree[i].depth;indent++) {
			x=0;
			for (k=i-1;k>-1;k--) {
				if (indent==ptree[i].depth-1) {
					if (ptree[k].numchildren>1) {
						prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-t.gif HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					} else if (ptree[k].numchildren==1) {
						prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-l.gif HEIGHT=21 WIDTH=9>");
						ptree[k].numchildren--;
						x=1;
						break;
					}
				} else if (indent<ptree[i].depth-1) {
					if (indent==ptree[k].depth) {
						if (ptree[k].numchildren>0) {
							prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-i.gif HEIGHT=21 WIDTH=9>");
							x=1;
							break;
						}
					}
				}
			}
			if (!x) {
				prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/blank.gif HEIGHT=21 WIDTH=9>");
			}
		}
		if (ptree[i].numchildren>0) {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-m.gif HEIGHT=21 WIDTH=9>");
		} else if (ptree[i].depth<1) {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-o.gif HEIGHT=21 WIDTH=9>");
		} else {
			prints(sid, "<IMG ALIGN=top BORDER=0 SRC=/groupware/images/forum-c.gif HEIGHT=21 WIDTH=9>");
		}
		prints(sid, "&nbsp;<A HREF=%s/mail/%s", sid->dat->in_ScriptName, (sid->dat->user_menustyle>0)?"main":"list");
		prints(sid, "?accountid=%d&folderid=%d><B>%s</B></A> ", accountid, atoi(sql_getvalue(sqr1, j, 0)), str2html(sid, sql_getvalue(sqr1, j, 2)));
		prints(sid, "</TD><TD ALIGN=right NOWRAP>");
		for (k=0;k<sql_numtuples(sqr2);k++) {
			if (atoi(sql_getvalue(sqr2, k, 0))==atoi(sql_getvalue(sqr1, j, 0))) {
				prints(sid, "(%d)", atoi(sql_getvalue(sqr2, k, 1)));
				prints(sid, "</TD><TD ALIGN=right NOWRAP>");
				prints(sid, "(%1.1f M)", atof(sql_getvalue(sqr2, k, 2))/1048576.0f);
				k=-1;
				break;
			}
		}
		if (k>-1) {
			prints(sid, "(0)</TD><TD ALIGN=right NOWRAP>(0.0 M)");
		}
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "<TR><TD></TD><TD ALIGN=right NOWRAP>");
	x=0;
	for (i=0;i<sql_numtuples(sqr2);i++) {
		x+=atoi(sql_getvalue(sqr2, i, 1));
	}
	prints(sid, "<B>(%d)</B></TD><TD ALIGN=right NOWRAP>", x);
	x=0;
	for (i=0;i<sql_numtuples(sqr2);i++) {
		x+=atoi(sql_getvalue(sqr2, i, 2));
	}
	prints(sid, "<B>(%1.1f M)</B></TD></TR>\n", x/1048576.0f);
	prints(sid, "</TABLE>\n");
	prints(sid, "<A HREF=%s/mail/folders/editnew?accountid=%d><B>New Folder</B></A>", sid->dat->in_ScriptName, accountid);
	prints(sid, "</CENTER>\n");
	free(ptree);
	free(btree);
	sql_freeresult(sqr2);
	sql_freeresult(sqr1);
	return;
}

void wmfolder_save(CONN *sid)
{
	char query[2048];
	char curdate[40];
	char foldername[51];
	char *ptemp;
	time_t t;
	int accountid;
	int folderid;
	int parentid;
	int sqr;

	prints(sid, "<BR>\n");
	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
			prints(sid, "<CENTER><B>This folder cannot be deleted.</B></CENTER>\n");
			return;
		}
		if ((sqr=sql_queryf(sid, "SELECT mailheaderid FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND folder = %d", sid->dat->user_uid, accountid, folderid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if ((sqr=sql_queryf(sid, "SELECT mailfolderid FROM gw_mailfolders WHERE obj_uid = %d AND accountid = %d AND parentfolderid = %d", sid->dat->user_uid, accountid, folderid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This folder cannot be deleted unless it is empty.</B></CENTER>\n");
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if (sql_updatef(sid, "DELETE FROM gw_mailfolders WHERE accountid = %d AND mailfolderid = %d", accountid, folderid)<0) return;
		prints(sid, "<CENTER>Mail Folder %d deleted successfully</CENTER><BR>\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	} else if (folderid==0) {
		if (strlen(foldername)<1) {
			prints(sid, "<CENTER>Folder name cannot be blank</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_queryf(sid, "SELECT max(mailfolderid) FROM gw_mailfolders where accountid = %d", accountid))<0) return;
		folderid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (folderid<11) folderid=11;
		strcpy(query, "INSERT INTO gw_mailfolders (mailfolderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountid, parentfolderid, foldername) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', '%d', ", folderid, curdate, curdate, sid->dat->user_uid, accountid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", parentid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, foldername));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Mail folder %d added successfully</CENTER><BR>\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	} else {
		if (folderid>5) {
			snprintf(query, sizeof(query)-1, "UPDATE gw_mailfolders SET obj_mtime = '%s', ", curdate);
			strncatf(query, sizeof(query)-strlen(query)-1, "parentfolderid = '%d', ", parentid);
			strncatf(query, sizeof(query)-strlen(query)-1, "foldername = '%s'", str2sql(sid, foldername));
			strncatf(query, sizeof(query)-strlen(query)-1, " WHERE mailfolderid = %d AND obj_uid = %d AND accountid = %d", folderid, sid->dat->user_uid, accountid);
			if (sql_update(sid, query)<0) return;
		}
		prints(sid, "<CENTER>Mail folder %d modified successfully</CENTER><BR>\n", folderid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	}
	return;
}
