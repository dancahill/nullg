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
#include "mod_mail.h"

static const char *order_by[]={
	"hdr_date DESC", "hdr_date ASC", "hdr_from DESC", "hdr_from ASC",
	"hdr_subject DESC", "hdr_subject ASC", "size DESC", "size ASC"
};

void wmnotice(CONN *sid)
{
	int i;
	int newmessages;
	int nummessages;
	int sqr1, sqr2;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
		return;
	}
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Groupware E-Mail Notice</FONT></B>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=left WIDTH=100%% STYLE='border-style:solid'>Account Name</TH><TH STYLE='border-style:solid'>New</TH><TH STYLE='border-style:solid'>Total</TH></TR>\n");
	if ((sqr1=sql_queryf("SELECT mailaccountid, accountname, poppassword FROM gw_mailaccounts where obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sql_numtuples(sqr1);i++) {
		sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr1, i, 0));
		if ((sqr2=sql_queryf("SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND folder = 1", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) continue;
		nummessages=atoi(sql_getvalue(sqr2, 0, 0));
		sql_freeresult(sqr2);
		if ((sqr2=sql_queryf("SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND folder = 1 AND status = 'n'", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) continue;
		newmessages=atoi(sql_getvalue(sqr2, 0, 0));
		sql_freeresult(sqr2);
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.opener.top.gwmain.location.href='%s/mail/main?accountid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/mail/main?accountid=%d TARGET=gwmain>%-.25s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), sql_getvalue(sqr1, i, 1));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s%d%s</TD><TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%d</TD></TR>\n", newmessages?"<FONT COLOR=BLUE><B>":"", newmessages, newmessages?"</B></FONT>":"", nummessages);
	}
	sql_freeresult(sqr1);
	prints(sid, "</TABLE></CENTER>\n");
	prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
	return;
}

void wmloginform(CONN *sid)
{
	char msgto[512];
	char *ptemp;
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	if ((ptemp=getgetenv(sid, "MSGTO"))!=NULL) {
		strncpy(msgto, ptemp, sizeof(msgto)-1);
	}
	if ((sqr=sql_queryf("SELECT popusername, poppassword FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)==1) {
		strncpy(sid->dat->wm->username, sql_getvalue(sqr, 0, 0), sizeof(sid->dat->wm->username)-1);
		strncpy(sid->dat->wm->password, DecodeBase64string(sid, sql_getvalue(sqr, 0, 1)), sizeof(sid->dat->wm->password)-1);
	}
	sql_freeresult(sqr);
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/sync AUTOCOMPLETE=OFF NAME=wmlogin>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE CELLPADDING=0 CELLSPACING=0 BORDER=0>\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='padding:1px'>Webmail Login</TH></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=wmusername SIZE=25 VALUE='%s'></TD></TR>\n", sid->dat->wm->username);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=wmpassword SIZE=25 VALUE='%s'></TD></TR>\n", sid->dat->wm->password);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Login'></CENTER></TD></TR>\n");
	prints(sid, "</TABLE></FORM></CENTER>\n");
	if (strlen(sid->dat->wm->username)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.wmusername.focus();\n// -->\n</SCRIPT>\n");
	} else {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.wmlogin.wmpassword.focus();\n// -->\n</SCRIPT>\n");
	}
}

void wmlogout(CONN *sid)
{
	char timebuffer[32];

	time_unix2sql(timebuffer, sizeof(timebuffer)-1, time(NULL));
	sql_updatef("UPDATE gw_mailaccounts SET obj_mtime = '%s',  poppassword = '' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->user_uid, sid->dat->user_mailcurrent);
	wmloginform(sid);
	return;
}

void webmailraw(CONN *sid)
{
	FILE *fp;
	char inbuffer[1024];
	char msgfilename[512];
	char *ptemp;
	int folderid;
	int msgnum=0;
	int sqr;

	send_header(sid, 1, 200, "OK", "1", "text/plain", -1, -1);
	if ((ptemp=getgetenv(sid, "MSG"))!=NULL) msgnum=atoi(ptemp);
	if ((sqr=sql_queryf("SELECT mailheaderid, folder, status FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d AND mailheaderid = %d and status != 'd'", sid->dat->user_uid, sid->dat->user_mailcurrent, msgnum))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "No such message.<BR>");
		sql_freeresult(sqr);
		return;
	} else {
		folderid=atoi(sql_getvalue(sqr, 0, 1));
	}
	sql_freeresult(sqr);
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->user_did, sid->dat->user_mailcurrent, folderid, msgnum);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "r");
	if (fp==NULL) {
		prints(sid, "Message body not found.<BR>");
		return;
	}
	prints(sid, "<PRE>\r\n");
	prints(sid, "<HR>\r\n");
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (strcmp(inbuffer, "")==0) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "<HR>\r\n");
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (fp==NULL) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "<HR>\r\n");
	prints(sid, "</PRE>\r\n");
	return;
}

void webmaillist(CONN *sid)
{
	wmheader header;
	char *ptemp;
	char curdate[40];
	char searchstring[256];
	short int neworder;
	short int order=0;
	int folderid;
	int nummessages;
	int offset=0;
	int i;
	int sqr;
	time_t unixdate;

	if ((ptemp=getgetenv(sid, "ORDER"))!=NULL) order=atoi(ptemp);
	if (order<0) order=0;
	if (order>7) order=7;
	if ((sqr=sql_queryf("SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/mail/accounts/editnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n");
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/accounts/editnew\"></NOSCRIPT>\n", sid->dat->in_ScriptName);
		sql_freeresult(sqr);
		return;
	}
	sql_freeresult(sqr);
	if ((ptemp=getgetenv(sid, "FOLDERID"))!=NULL) {
		folderid=atoi(ptemp);
	} else {
		folderid=1;
	}
	wmfolder_testcreate(sid, sid->dat->user_mailcurrent, folderid);
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sid->dat->user_mailcurrent!=atoi(ptemp)) {
			sql_updatef("UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username);
			sid->dat->user_mailcurrent=atoi(ptemp);
			folderid=1;
		}
	}
/*
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailjump ACTION=%s/mail/list onChange=\"mailjump()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	htselect_mailjump(sid, sid->dat->user_mailcurrent, folderid);
	prints(sid, "</TD></FORM>\r\n");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;");
	prints(sid, "</TD></TR></TABLE>\r\n");
*/
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailmbox ACTION=%s/mail/list onChange=\"mailmbox()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	htselect_mailfolderjump(sid, folderid);
	prints(sid, "</TD></FORM>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailjump ACTION=%s/mail/list onChange=\"mailjump()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TD ALIGN=RIGHT>\r\n");
	htselect_mailjump(sid, sid->dat->user_mailcurrent);
	prints(sid, "</TD></FORM></TR></TABLE>\r\n");

	flushbuffer(sid);
	snprintf(searchstring, sizeof(searchstring)-1, "%s", wmsearch_makestring(sid));
	if ((sqr=wmsearch_doquery(sid, order_by[order], folderid))<0) return;
	nummessages=sql_numtuples(sqr);
	if (nummessages<1) {
		prints(sid, "<BR><CENTER><B>You have no messages in this mailbox</B></CENTER><BR>\n");
		sql_freeresult(sqr);
		return;
	}
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if (offset>nummessages-sid->dat->user_maxlist) offset=nummessages-sid->dat->user_maxlist;
	if (offset<0) offset=0;
	prints(sid, "<script language='JavaScript'>\n<!--\n");
	prints(sid, "function CheckAll(x)\n{\n");
	prints(sid, "	for (var i=0;i<document.webmail.elements.length;i++) {\n");
	prints(sid, "		var e=document.webmail.elements[i];\n");
	prints(sid, "		if ((x==1)&&(e.name!='allbox1')) {\n");
	prints(sid, "			e.checked=document.webmail.allbox1.checked;\n");
	prints(sid, "		} else if ((x==2)&&(e.name!='allbox2')) {\n");
	prints(sid, "			e.checked=document.webmail.allbox2.checked;\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n//-->\n</script>\n");
	sql_updatef("UPDATE gw_mailheaders SET status = 'o' WHERE accountid = %d AND obj_uid = %d AND status = 'n'", sid->dat->user_mailcurrent, sid->dat->user_uid);
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if (searchstring[0]=='\0') {
		prints(sid, "<FORM METHOD=POST NAME=webmail ACTION=%s/mail/move%s>\n", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=wmread":"");
		prints(sid, "<INPUT TYPE=hidden NAME=offset VALUE=%d>\n", offset);
		prints(sid, "<INPUT TYPE=hidden NAME=order VALUE=%d>\n", order);
	}
	if (searchstring[0]=='\0') {
		prints(sid, "<TR><TD ALIGN=LEFT NOWRAP><NOBR><INPUT TYPE=checkbox NAME=allbox1 onclick='CheckAll(1);'>");
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=delete VALUE=\"%s\">\n", FORM_DELETE);
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=move VALUE=\"%s\"><SELECT NAME=dest1>\n", MOD_MAIL_MOVETO);
		htselect_mailfolder(sid, 0, 1, 0);
		prints(sid, "</SELECT></NOBR></TD>");
	} else {
		prints(sid, "<TR><TD ALIGN=LEFT NOWRAP>&nbsp;</TD>");
	}
	prints(sid, "<TD ALIGN=RIGHT NOWRAP><B>");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			i=offset-sid->dat->user_maxlist;
			if (i<0) i=0;
			prints(sid, "<A HREF=%s/mail/list?folderid=%d&offset=%d&order=%d%s>&lt;&lt;</A>", sid->dat->in_ScriptName, folderid, i, order, searchstring);
		} else {
			prints(sid, "&lt;&lt;");
		}
		prints(sid, " Listing %d-%d of %d ", offset+1, offset+sid->dat->user_maxlist, nummessages);
		if (offset+sid->dat->user_maxlist<nummessages) {
			i=offset+sid->dat->user_maxlist;
			if (i>nummessages-sid->dat->user_maxlist) i=nummessages-sid->dat->user_maxlist;
			prints(sid, "<A HREF=%s/mail/list?folderid=%d&offset=%d&order=%d%s>&gt;&gt;</A>", sid->dat->in_ScriptName, folderid, i, order, searchstring);
		} else {
			prints(sid, "&gt;&gt;");
		}
	} else {
		prints(sid, "Listing %d-%d of %d", offset+1, nummessages, nummessages);
	}
	prints(sid, "</B></TD></TR>\n");
	prints(sid, "<TR><TD COLSPAN=2>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR>\n");
	if (searchstring[0]=='\0') {
		prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH>");
	}
	neworder=(order==3?2:3);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&order=%d%s'\">", sid->dat->in_ScriptName, folderid, neworder, searchstring);
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&order=%d%s\">%s</A>&nbsp;</TH>\r\n", folderid, neworder, searchstring, MOD_MAIL_FROM);
	neworder=(order==5?4:5);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&order=%d%s'\" WIDTH=100%%>", sid->dat->in_ScriptName, folderid, neworder, searchstring);
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&order=%d%s\">%s</A>&nbsp;</TH>\r\n", folderid, neworder, searchstring, MOD_MAIL_SUBJECT);
	neworder=(order==0?1:0);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&order=%d%s'\">", sid->dat->in_ScriptName, folderid, neworder, searchstring);
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&order=%d%s\">%s</A>&nbsp;</TH>\r\n", folderid, neworder, searchstring, MOD_MAIL_DATE);
	neworder=(order==6?7:6);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&order=%d%s'\">", sid->dat->in_ScriptName, folderid, neworder, searchstring);
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&order=%d%s\">%s</A>&nbsp;</TH>\r\n", folderid, neworder, searchstring, MOD_MAIL_SIZE);
	prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH>");
	prints(sid, "</TR>\n");
	for (i=offset;(i<nummessages)&&(i<offset+sid->dat->user_maxlist);i++) {
		dbread_getheader(sid, sqr, i, &header);
		prints(sid, "<TR BGCOLOR=\"%s\">", strcmp(header.status, "r")!=0?"#D0D0FF":"#F0F0F0");
		if (searchstring[0]=='\0') {
			prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid'><INPUT TYPE=checkbox NAME=%d VALUE=\"%s\"></TD>\r\n", header.localid, header.uidl);
		}
		prints(sid, "<TD NOWRAP TITLE=\"%s\" STYLE='border-style:solid'>", str2html(sid, header.From));
		prints(sid, "<SPAN STYLE='width:120px;overflow:hidden'>%s&nbsp;</SPAN></TD>\r\n", str2html(sid, header.FromName));
		prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"");
		if (sid->dat->user_menustyle>0) {
			prints(sid, "window.parent.wmread.location.href='%s/mail/read?msg=%d&order=%d%s'\" ", sid->dat->in_ScriptName, header.localid, order, searchstring);
		} else {
			prints(sid, "window.location.href='%s/mail/read?msg=%d&order=%d%s'\" ", sid->dat->in_ScriptName, header.localid, order, searchstring);
		}
		prints(sid, "TITLE=\"%s\"><SPAN STYLE='width:315px;overflow:hidden'>", str2html(sid, header.Subject));
		prints(sid, "<A HREF=%s/mail/read?msg=%d&order=%d%s", sid->dat->in_ScriptName, header.localid, order, searchstring);
		prints(sid, "%s TITLE=\"%s\">", sid->dat->user_menustyle>0?" TARGET=wmread":"", str2html(sid, header.Subject));
		prints(sid, "%s</A>&nbsp;</SPAN></TD>\r\n", str2html(sid, header.Subject));
		unixdate=time_sql2unix(header.Date);
		unixdate+=time_tzoffset(sid, unixdate);
		if (unixdate<0) unixdate=0;
		strftime(curdate, 30, "%b %d, %Y %I:%M%p", gmtime(&unixdate));
		curdate[18]=tolower(curdate[18]);
		curdate[19]=tolower(curdate[19]);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s</TD>\r\n", curdate);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>");
		if (header.size>=1048576) {
			prints(sid, "%1.1f M", (float)header.size/1048576.0);
		} else {
			prints(sid, "%1.1f K", (float)header.size/1024.0);
		}
		prints(sid, "</TD>\r\n<TD STYLE='border-style:solid'>");
		if (p_strcasestr(header.contenttype, "multipart/mixed")!=NULL) {
			prints(sid, "<IMG BORDER=0 SRC=/groupware/images/paperclip.png HEIGHT=16 WIDTH=11 ALT='File Attachments'>");
		} else {
			prints(sid, "&nbsp;&nbsp;&nbsp;");
		}
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n</TD></TR>\r\n");
	if (searchstring[0]=='\0') {
		prints(sid, "<TR><TD ALIGN=LEFT NOWRAP><NOBR><INPUT TYPE=checkbox NAME=allbox2 onclick='CheckAll(2);'>");
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=delete VALUE=\"%s\">\n", FORM_DELETE);
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=move VALUE=\"%s\"><SELECT NAME=dest2>\n", MOD_MAIL_MOVETO);
		htselect_mailfolder(sid, 0, 1, 0);
		prints(sid, "</SELECT></NOBR></TD>");
		prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD></TR>\n");
		prints(sid, "</FORM>\n");
	}
	prints(sid, "</TABLE>\n");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			i=offset-sid->dat->user_maxlist;
			if (i<0) i=0;
			prints(sid, "[<A HREF=%s/mail/list?folderid=%d&offset=%d&order=%d%s>%s</A>]", sid->dat->in_ScriptName, folderid, i, order, searchstring, MOD_MAIL_PREVPAGE);
		} else {
			prints(sid, "[%s]", MOD_MAIL_PREVPAGE);
		}
		if (offset+sid->dat->user_maxlist<nummessages) {
			i=offset+sid->dat->user_maxlist;
			if (i>nummessages-sid->dat->user_maxlist) i=nummessages-sid->dat->user_maxlist;
			prints(sid, "[<A HREF=%s/mail/list?folderid=%d&offset=%d&order=%d%s>%s</A>]", sid->dat->in_ScriptName, folderid, i, order, searchstring, MOD_MAIL_NEXTPAGE);
		} else {
			prints(sid, "[%s]", MOD_MAIL_NEXTPAGE);
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void webmailread(CONN *sid)
{
	wmheader header;
	struct stat sb;
	FILE *fp;
	char *ptemp;
	char curdate[40];
	char inbuffer[512];
	char msgfilename[512];
	char oldfilename[512];
	char searchstring[256];
	short int order=0;
	int folderid=0;
	int nummessages;
	int localid=0;
	int remoteid=-1;
	int i;
	int sqr;
	time_t unixdate;

	memset((char *)&header, 0, sizeof(header));
	if ((ptemp=getgetenv(sid, "MSG"))!=NULL) localid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "ORDER"))!=NULL) order=atoi(ptemp);
	if (order<0) order=0;
	if (order>7) order=7;
	if ((sqr=sql_queryf("SELECT mailheaderid, folder FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' and mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, localid))<0) return;
	if (sql_numtuples(sqr)==1) {
		folderid=atoi(sql_getvalue(sqr, 0, 1));
	}
	sql_freeresult(sqr);
	snprintf(searchstring, sizeof(searchstring)-1, "%s", wmsearch_makestring(sid));
	if ((sqr=wmsearch_doquery(sid, order_by[order], folderid))<0) return;
	nummessages=sql_numtuples(sqr);
	for (i=0;i<nummessages;i++) {
		if (localid==atoi(sql_getvalue(sqr, i, 0))) {
			dbread_getheader(sid, sqr, i, &header);
			remoteid=i+1;
			break;
		}
	}
	if (remoteid==-1) {
		prints(sid, "No such message.<BR>");
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this message?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ViewContact(name, email) {\r\n");
	prints(sid, "	window.open('%s/contacts/mailview?name='+name+'&email='+email,'emaillookup','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=470,height=375');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "function ReplyTo() {\r\n");
	prints(sid, "	window.open('%s/mail/write?replyto=%d&accountid=%d','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName, header.localid, header.accountid);
	prints(sid, "}\r\n");
	prints(sid, "function ReplyAll() {\r\n");
	prints(sid, "	window.open('%s/mail/write?replyall=%d&accountid=%d','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName, header.localid, header.accountid);
	prints(sid, "}\r\n");
	prints(sid, "function Forward() {\r\n");
	prints(sid, "	window.open('%s/mail/write?forward=%d&accountid=%d','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName, header.localid, header.accountid);
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (remoteid>1) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d&order=%d%s>%s</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid-2, 0)), order, searchstring, MOD_MAIL_PREV);
	} else {
		prints(sid, "[%s]\n", MOD_MAIL_PREV);
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo();>%s</A>]\n", MOD_MAIL_REPLY);
		prints(sid, "[<A HREF=javascript:ReplyAll();>%s</A>]\n", MOD_MAIL_REPLYALL);
		prints(sid, "[<A HREF=javascript:Forward();>%s</A>]\n", MOD_MAIL_FORWARD);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_REPLY);
		prints(sid, "[<A HREF=%s/mail/write?replyall=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_REPLYALL);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_FORWARD);
	}
	prints(sid, "[<A HREF=%s/mail/move?%d=%s onClick=\"return ConfirmDelete();\">%s</A>]\n", sid->dat->in_ScriptName, localid, header.uidl, MOD_MAIL_DELETE);
	if ((remoteid<nummessages)&&(remoteid>-1)) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d&order=%d%s>%s</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid, 0)), order, searchstring, MOD_MAIL_NEXT);
	} else {
		prints(sid, "[%s]\n", MOD_MAIL_NEXT);
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='padding:1px; border-style:solid'>");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_FROM);
	prints(sid, "<A HREF=\"javascript:ViewContact('%s','%s');\">%s</A> - ", encodeurl(sid, header.FromName), header.FromAddr, str2html(sid, header.From));
	prints(sid, "<A HREF=\"javascript:MsgTo('&quot;%s&quot; <%s>');\">Send Mail</A>", encodeurl(sid, header.FromName), header.ReplyTo);
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_TO);
	printht(sid, "%s", header.To);
	prints(sid, "&nbsp;</TD></TR>\n");
	if (strlen(header.CC)) {
		prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_CC);
		printht(sid, "%s", header.CC);
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	if (strlen(header.BCC)) {
		prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_BCC);
		printht(sid, "%s", header.BCC);
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_SUBJECT);
	printht(sid, "%s", header.Subject);
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP>&nbsp;%s&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;", MOD_MAIL_DATE);
	memset(curdate, 0, sizeof(curdate));
	unixdate=time_sql2unix(header.Date);
	unixdate+=time_tzoffset(sid, unixdate);
	if (unixdate<0) unixdate=0;
	strftime(curdate, 30, "%B %d, %Y %I:%M %p", gmtime(&unixdate));
	printht(sid, "%s", curdate);
	prints(sid, "&nbsp;</TD></TR>\n");
	if (strlen(header.scanresult)) {
		prints(sid, "<TR><TH ALIGN=LEFT VALIGN=TOP NOWRAP>&nbsp;Scan Result&nbsp;</TH><TD CLASS=\"FIELDVAL\" WIDTH=100%%>&nbsp;%s&nbsp;</TD></TR>\n", header.scanresult);
	}
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>[<A HREF=%s/mail/raw?msg=%d TARGET=_blank>%s</A>]</TD></TR>\n", sid->dat->in_ScriptName, localid, MOD_MAIL_VIEWSOURCE);
	prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>\n");
	sql_updatef("UPDATE gw_mailheaders SET status = 'r' WHERE uidl = '%s' AND obj_uid = %d AND accountid = %d", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.uidl), sid->dat->user_uid, sid->dat->user_mailcurrent);
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->user_did, header.accountid, header.folderid, localid);
	fixslashes(msgfilename);
	if (stat(msgfilename, &sb)!=0) {
		memset(oldfilename, 0, sizeof(oldfilename));
		snprintf(oldfilename, sizeof(oldfilename)-1, "%s/%04d/mail/%04d/%06d.msg", config->dir_var_domains, sid->dat->user_did, header.accountid, localid);
		fixslashes(oldfilename);
		if (stat(oldfilename, &sb)==0) {
			if (rename(oldfilename, msgfilename)==0) {
				prints(sid, "[stuff]");
			}
		}
	}
	fp=fopen(msgfilename, "r");
	if (fp!=NULL) {
		while (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
			striprn(inbuffer);
			if (strlen(inbuffer)==0) break;
		}
		webmailmime(sid, &fp, header.contenttype, header.encoding, header.boundary, localid, 0, 0);
	} else {
		prints(sid, "<B>Could not retrieve message body!</B>\n");
	}
	prints(sid, "</TD></TR></TABLE>\n");
	if (remoteid>1) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d&order=%d%s>%s</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid-2, 0)), order, searchstring, MOD_MAIL_PREV);
	} else {
		prints(sid, "[%s]\n", MOD_MAIL_PREV);
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo();>%s</A>]\n", MOD_MAIL_REPLY);
		prints(sid, "[<A HREF=javascript:ReplyAll();>%s</A>]\n", MOD_MAIL_REPLYALL);
		prints(sid, "[<A HREF=javascript:Forward();>%s</A>]\n", MOD_MAIL_FORWARD);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_REPLY);
		prints(sid, "[<A HREF=%s/mail/write?replyall=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_REPLYALL);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d&accountid=%d>%s</A>]\n", sid->dat->in_ScriptName, localid, header.accountid, MOD_MAIL_FORWARD);
	}
	prints(sid, "[<A HREF=%s/mail/move?%d=%s onClick=\"return ConfirmDelete();\">%s</A>]\n", sid->dat->in_ScriptName, localid, header.uidl, MOD_MAIL_DELETE);
	if ((remoteid<nummessages)&&(remoteid>-1)) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d&order=%d%s>%s</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid, 0)), order, searchstring, MOD_MAIL_NEXT);
	} else {
		prints(sid, "[%s]\n", MOD_MAIL_NEXT);
	}
	prints(sid, "<BR>\n");
	sql_freeresult(sqr);
	return;
}

void webmailwrite(CONN *sid)
{
	FILE *fp=NULL;
	wmheader header;
	char inbuffer[512];
	char msgfilename[512];
	char msgto[2048];
	char msgcc[2048];
	char msgbcc[2048];
	char subject[512];
	char *ptemp;
	short int accountid=sid->dat->user_mailcurrent;
	short int folderid=0;
	short int replyto=0;
	short int replyall=0;
	short int forward=0;
	short int msgnum=0;
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	memset(msgcc, 0, sizeof(msgcc));
	memset(msgbcc, 0, sizeof(msgbcc));
	memset(subject, 0, sizeof(subject));
	memset((char *)&header, 0, sizeof(header));
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) accountid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "REPLYTO"))!=NULL) replyto=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "REPLYALL"))!=NULL) { replyto=atoi(ptemp); replyall=1; }
	if ((ptemp=getgetenv(sid, "FORWARD"))!=NULL) forward=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "TO"))!=NULL) strncpy(msgto, ptemp, sizeof(msgto)-1);
	if ((ptemp=getgetenv(sid, "CC"))!=NULL) strncpy(msgcc, ptemp, sizeof(msgcc)-1);
	if ((ptemp=getgetenv(sid, "BCC"))!=NULL) strncpy(msgbcc, ptemp, sizeof(msgbcc)-1);
	if (replyto>0) {
		msgnum=replyto;
	} else if (forward>0) {
		msgnum=forward;
	} else {
		msgnum=0;
	}
	if (msgnum) {
		if ((sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->user_uid, accountid, msgnum))<0) return;
		if (sql_numtuples(sqr)!=1) {
			sql_freeresult(sqr);
			return;
		} else {
			folderid=atoi(sql_getvaluebyname(sqr, 0, "folder"));
		}
		dbread_getheader(sid, sqr, 0, &header);
		sql_freeresult(sqr);
		if (replyto>0) {
			snprintf(subject, sizeof(subject)-1, "%s%s", (strncasecmp(header.Subject, "RE:", 3)==0)?"":"Re: ", header.Subject);
		} else if (forward>0) {
			snprintf(subject, sizeof(subject)-1, "%s%s", (strncasecmp(header.Subject, "FWD:", 4)==0)?"":"Fwd: ", header.Subject);
		}
		if (replyto>0) {
			snprintf(msgto, sizeof(msgto)-1, "%s", header.ReplyTo);
		}
		if (replyall) {
			if ((strlen(msgto)>0)&&(strlen(header.To)>0)) strncat(msgto, ", ", sizeof(msgto)-strlen(msgto)-1);
			strncat(msgto, header.To, sizeof(msgto)-strlen(msgto)-1);
			strncpy(msgcc, header.CC, sizeof(msgcc)-strlen(msgto)-1);
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function AddressBook(field) {\r\n");
	prints(sid, "	window.open('%s/mail/addresses?field='+field,'wmaddrbook','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=440,height=440');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "function selOn(ctrl)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	ctrl.style.borderColor = '#000000';\r\n");
	prints(sid, "	ctrl.style.cursor = 'hand';\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function selOff(ctrl)\r\n");
	prints(sid, "{\r\n");
	prints(sid, "	ctrl.style.borderColor = '#F0F0F0';\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/save NAME=wmcompose ENCTYPE=multipart/form-data onSubmit=\"copy_submit()\">\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=inreplyto VALUE=\"%s\">\n", header.MessageID);
	if (forward>0) {
		prints(sid, "<INPUT TYPE=hidden NAME=forward VALUE=\"%d\">\n", forward);
		prints(sid, "<INPUT TYPE=hidden NAME=fwdacct VALUE=\"%d\">\n", accountid);
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%%>&nbsp;<B>%s</B>&nbsp;</TD><TD WIDTH=90%%>\n", MOD_MAIL_FROM);
	prints(sid, "<SELECT NAME=accountid style='width:433px'>\n");
	htselect_mailaccount(sid, accountid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('to');>&nbsp;<B><A HREF=javascript:AddressBook('to') TITLE='To'>%s</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgto SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", MOD_MAIL_TO, str2html(sid, msgto));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('cc');>&nbsp;<B><A HREF=javascript:AddressBook('cc') TITLE='Carbon Copy'>%s</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgcc SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", MOD_MAIL_CC, str2html(sid, msgcc));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('bcc');>&nbsp;<B><A HREF=javascript:AddressBook('bcc') TITLE='Blind Carbon Copy'>%s</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgbcc SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", MOD_MAIL_BCC, str2html(sid, msgbcc));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%%>&nbsp;<B>%s</B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgsubject SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", MOD_MAIL_SUBJECT, str2html(sid, subject));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%% STYLE='padding:0px'><B>&nbsp;Format&nbsp;</B></TD><TD STYLE='padding:0px'>");
	prints(sid, "<select NAME=ctype onChange=\"toggle_mode()\">\r\n<option value=plain>Plain Text</option>\r\n<option value=html>HTML</option>\r\n</select>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2>");
	prints(sid, "<DIV ID=MenuBar STYLE='display: none'>\r\n");
	prints(sid, "<TABLE BORDER=0><TR><TD NOWRAP STYLE='padding:0px'>\r\n");
	prints(sid, "	<select onchange=\"SetFont('FontName', this.value)\">\r\n");
	prints(sid, "	<option value='Arial'>Arial</option>\r\n");
	prints(sid, "	<option value='Courier'>Courier</option>\r\n");
	prints(sid, "	<option value='Helvetica'>Helvetica</option>\r\n");
	prints(sid, "	<option value='Times New Roman' SELECTED>Times New Roman</option>\r\n");
	prints(sid, "	<option value='Verdana'>Verdana</option>\r\n");
	prints(sid, "	</select>\r\n");
	prints(sid, "	<select onchange=\"SetFont('FontSize', this.value)\">\r\n");
	prints(sid, "	<option value='1'>1</option>\r\n");
	prints(sid, "	<option value='2'>2</option>\r\n");
	prints(sid, "	<option value='3' SELECTED>3</option>\r\n");
	prints(sid, "	<option value='4'>4</option>\r\n");
	prints(sid, "	<option value='5'>5</option>\r\n");
	prints(sid, "	<option value='6'>6</option>\r\n");
	prints(sid, "	<option value='7'>7</option>\r\n");
	prints(sid, "	</select></TD><TD BGCOLOR=#000000 STYLE='padding:1px'></TD><TD STYLE='padding:0px'><IMG\r\n");
	prints(sid, "	ALT='Bold'                   SRC='/groupware/images/wmedit_bold.png'      onClick=wmformat('Bold')                 BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Italic'                 SRC='/groupware/images/wmedit_italic.png'    onClick=wmformat('Italic')               BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Underline'              SRC='/groupware/images/wmedit_underline.png' onClick=wmformat('Underline')            BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Font Colour'            SRC='/groupware/images/wmedit_fontcol.png'   onClick=SetForeColour()                  BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)></TD><TD BGCOLOR=#000000 STYLE='padding:1px'></TD><TD STYLE='padding:0px'><IMG\r\n");
	prints(sid, "	ALT='Formatting Numbers'     SRC='/groupware/images/wmedit_numlist.png'   onClick=wmformat('InsertOrderedList')    BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Formatting Bullets'     SRC='/groupware/images/wmedit_bullist.png'   onClick=wmformat('InsertUnorderedList')  BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Decrease Indentation'   SRC='/groupware/images/wmedit_outdent.png'   onClick=wmformat('Outdent')              BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Increase Indentation'   SRC='/groupware/images/wmedit_indent.png'    onClick=wmformat('Indent')               BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)></TD><TD BGCOLOR=#000000 STYLE='padding:1px'></TD><TD STYLE='padding:0px'><IMG\r\n");
	prints(sid, "	ALT='Align Left'             SRC='/groupware/images/wmedit_left.png'      onClick=wmformat('JustifyLeft')          BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Center'                 SRC='/groupware/images/wmedit_center.png'    onClick=wmformat('JustifyCenter')        BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Align Right'            SRC='/groupware/images/wmedit_right.png'     onClick=wmformat('JustifyRight')         BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Justify'                SRC='/groupware/images/wmedit_justify.png'   onClick=wmformat('Justify')              BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)></TD><TD BGCOLOR=#000000 STYLE='padding:1px'></TD><TD STYLE='padding:0px' width=100%><IMG\r\n");
	prints(sid, "	ALT='Insert Horizontal Line' SRC='/groupware/images/wmedit_rule.png'      onClick=wmformat('InsertHorizontalRule') BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Create a Hyperlink'     SRC='/groupware/images/wmedit_link.png'      onClick=doLink()                         BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)><IMG\r\n");
	prints(sid, "	ALT='Insert Picture'         SRC='/groupware/images/wmedit_image.png'     onClick=doImage()                        BORDER=0 WIDTH=22 HEIGHT=22 CLASS=butClass onMouseOver=selOn(this) onMouseOut=selOff(this)></TD>\r\n");
	prints(sid, "</TR></TABLE>\r\n");
	prints(sid, "<CENTER><IFRAME id=\"wmeditor\" HEIGHT=236 WIDTH=646 SRC=\"javascript:return false;\"></IFRAME></CENTER>");
	prints(sid, "</DIV>");
	prints(sid, "<CENTER><TEXTAREA NAME=msgbody COLS=78 ROWS=16 WRAP=VIRTUAL>\n");
	if (msgnum) {
		if (forward>0) {
			prints(sid, "Note: forwarded message attached.\n");
		} else {
			prints(sid, "--- %s wrote:\n", header.From);
			memset(msgfilename, 0, sizeof(msgfilename));
			snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->user_did, accountid, folderid, msgnum);
			fixslashes(msgfilename);
			if ((fp=fopen(msgfilename, "r"))!=NULL) {
				while (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
					striprn(inbuffer);
					if (strlen(inbuffer)==0) break;
				}
				webmailmime(sid, &fp, header.contenttype, header.encoding, header.boundary, msgnum, 1, 0);
			} else {
				prints(sid, "\r\nCould not retrieve message body!\r\n");
			}
		}
	}
	prints(sid, "</TEXTAREA></CENTER>\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD WIDTH=10%%>&nbsp;<B>Attachment</B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=FILE NAME=fattach SIZE=70></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Send Mail'>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Save As Draft'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "document.wmcompose.msgto.focus();\r\n");
	prints(sid, "//-->\r\n</script>\r\n");
	return;
}

void webmailframeset(CONN *sid)
{
	char *ptemp;
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<HTML>\n<HEAD>\n<TITLE>NullLogic Groupware Webmail</TITLE>\n</HEAD>\n");
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sql_updatef("UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username)==0) {
			sid->dat->user_mailcurrent=atoi(ptemp);
		}
	}
	if ((sqr=sql_queryf("SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/mail/accounts/editnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/accounts/editnew\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
		prints(sid, "</HTML>\n");
	} else {
		prints(sid, "<FRAMESET ROWS=\"50%%,50%%\" BORDER=0 FRAMEBORDER=1 FRAMESPACING=1>\n");
		prints(sid, "<FRAME BORDER=0 NAME=\"wmlist\" SRC=%s/mail/list", sid->dat->in_ScriptName);
		if ((ptemp=getgetenv(sid, "FOLDERID"))!=NULL) {
			prints(sid, "?folderid=%d", atoi(ptemp));
		} else if ((ptemp=getgetenv(sid, "C"))!=NULL) {
			prints(sid, "?c=%s", ptemp);
			if ((ptemp=getgetenv(sid, "TEXT"))!=NULL) {
				prints(sid, "&text=%s", ptemp);
			}
		}
		prints(sid, " MARGINHEIGHT=1 MARGINWIDTH=1 SCROLLING=YES>\n", sid->dat->in_ScriptName);
		prints(sid, "<FRAME BORDER=0 NAME=\"wmread\" SRC=%s/mail/null MARGINHEIGHT=1 MARGINWIDTH=1 SCROLLING=YES>\n", sid->dat->in_ScriptName);
		prints(sid, "</FRAMESET>\n");
		prints(sid, "To view this page, you need a web browser capable of displaying frames.\n");
		prints(sid, "</HTML>\n");
	}
	sql_freeresult(sqr);
	return;
}

void mod_main(CONN *sid)
{
	char *suburi=sid->dat->in_RequestURI;

	if (strncmp(suburi, "/mail/", 6)!=0) return;
	suburi+=6;
	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(WEBMAIL)))==NULL) return;
	}
	if (strncmp(suburi, "main", 4)==0) {
		if (sid->dat->user_menustyle>0) {
			webmailframeset(sid);
			goto done;
		} else {
			/* HACK THE URI?  CRUDE, BUT EFFECTIVE */
			snprintf(sid->dat->in_RequestURI, sizeof(sid->dat->in_RequestURI)-1, "/mail/list");
		}
	}
	if (strncmp(suburi, "file", 4)==0) {
		webmailfiledl(sid);
		goto done;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(suburi, "notice", 6)==0) {
		htpage_header(sid, "E-Mail Notice");
		wmnotice(sid);
		htpage_footer(sid);
		goto done;
	}
	htpage_topmenu(sid, MENU_WEBMAIL);
	flushbuffer(sid);
	if (strncmp(suburi, "addresses", 9)==0) {
		wmaddr_list(sid);
	} else if (strncmp(suburi, "list", 4)==0) {
		webmaillist(sid);
	} else if (strncmp(suburi, "read", 4)==0) {
		webmailread(sid);
	} else if (strncmp(suburi, "write", 5)==0) {
		webmailwrite(sid);
	} else if (strncmp(suburi, "move", 4)==0) {
		webmailmove(sid);
	} else if (strncmp(suburi, "purge", 5)==0) {
		webmailpurge(sid);
	} else if (strncmp(suburi, "save", 4)==0) {
		webmailsave(sid);
	} else if (strncmp(suburi, "search", 6)==0) {
		wmsearch_form(sid);
	} else if (strncmp(suburi, "sync", 4)==0) {
		wmsync(sid, 1);
	} else if (strncmp(suburi, "raw", 3)==0) {
		webmailraw(sid);
	} else if (strncmp(suburi, "quit", 4)==0) {
		wmlogout(sid);
	} else if (strncmp(suburi, "accounts/", 9)==0) {
		suburi+=9;
		if (strncmp(suburi, "edit", 4)==0) {
			wmaccount_edit(sid);
		} else if (strncmp(suburi, "list", 4)==0) {
			wmaccount_list(sid);
		} else if (strncmp(suburi, "save", 4)==0) {
			wmaccount_save(sid);
		}
	} else if (strncmp(suburi, "filters/", 8)==0) {
		suburi+=8;
		if (strncmp(suburi, "edit", 4)==0) {
			wmfilter_edit(sid);
		} else if (strncmp(suburi, "save", 4)==0) {
			wmfilter_save(sid);
		}
	} else if (strncmp(suburi, "folders/", 8)==0) {
		suburi+=8;
		if (strncmp(suburi, "edit", 4)==0) {
			wmfolder_edit(sid);
		} else if (strncmp(suburi, "save", 4)==0) {
			wmfolder_save(sid);
		}
	}
	htpage_footer(sid);
done:
	if (sid->dat->wm!=NULL) {
		free(sid->dat->wm);
		sid->dat->wm=NULL;
	}
	return;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_mail",		// mod_name
		1,			// mod_submenu
		"E-MAIL",		// mod_menuname
		"/mail/main",		// mod_menuuri
		"webmail",		// mod_menuperm
		"mod_main",		// fn_name
		"/mail/",		// fn_uri
		mod_main		// fn_ptr
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	if (mod_export_main(&newmod)!=0) return -1;
	if (mod_export_function("mod_mail", "mod_mail_sync", wmsync)!=0) return -1;
	return 0;
}
