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
#include "mod_mail.h"

void wmnotice(CONN *sid)
{
	int i;
	int newmessages;
	int nummessages;
	int sqr1, sqr2;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<BR><CENTER>\n<B><FONT COLOR=#808080 SIZE=3>Groupware E-Mail Notice</FONT></B>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=left WIDTH=100%% STYLE='border-style:solid'>Account Name</TH><TH STYLE='border-style:solid'>New</TH><TH STYLE='border-style:solid'>Total</TH></TR>\n", config->colour_th);
	if ((sqr1=sql_queryf(sid, "SELECT mailaccountid, accountname, poppassword FROM gw_mailaccounts where obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid))<0) return;
	for (i=0;i<sql_numtuples(sqr1);i++) {
		sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr1, i, 0));
		if ((sqr2=sql_queryf(sid, "SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND folder = 1", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) continue;
		nummessages=atoi(sql_getvalue(sqr2, 0, 0));
		sql_freeresult(sqr2);
		if ((sqr2=sql_queryf(sid, "SELECT count(mailheaderid) FROM gw_mailheaders WHERE obj_uid = %d AND accountid = %d AND folder = 1 AND status = 'n'", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) continue;
		newmessages=atoi(sql_getvalue(sqr2, 0, 0));
		sql_freeresult(sqr2);
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.opener.top.gwmain.location.href='%s/mail/main?accountid=%d'\">", config->colour_fieldval, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
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
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	if (getgetenv(sid, "MSGTO")!=NULL) {
		strncpy(msgto, getgetenv(sid, "MSGTO"), sizeof(msgto)-1);
	}
	/* don't ask...  heh..  it was either this or a goto */
	for (;;) {
		if ((sqr=sql_queryf(sid, "SELECT popusername, poppassword FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) break;
		if (sql_numtuples(sqr)==1) {
			strncpy(sid->dat->wm->username, sql_getvalue(sqr, 0, 0), sizeof(sid->dat->wm->username)-1);
			strncpy(sid->dat->wm->password, DecodeBase64string(sid, sql_getvalue(sqr, 0, 1)), sizeof(sid->dat->wm->password)-1);
		}
		sql_freeresult(sqr);
		break;
	}
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/sync AUTOCOMPLETE=OFF NAME=wmlogin>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE CELLPADDING=0 CELLSPACING=0 BORDER=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 STYLE='padding:1px'><FONT COLOR=%s>Webmail Login</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Username&nbsp;</B></TD><TD><INPUT TYPE=TEXT NAME=wmusername SIZE=25 VALUE='%s'></TD></TR>\n", config->colour_editform, sid->dat->wm->username);
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;Password&nbsp;</B></TD><TD><INPUT TYPE=PASSWORD NAME=wmpassword SIZE=25 VALUE='%s'></TD></TR>\n", config->colour_editform, sid->dat->wm->password);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><CENTER><INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Login'></CENTER></TD></TR>\n", config->colour_editform);
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

	memset(timebuffer, 0, sizeof(timebuffer));
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, time(NULL)));
	sql_updatef(sid, "UPDATE gw_mailaccounts SET obj_mtime = '%s',  poppassword = '' WHERE obj_uid = %d AND mailaccountid = %d", timebuffer, sid->dat->user_uid, sid->dat->user_mailcurrent);
	return;
}

void webmailraw(CONN *sid)
{
	FILE *fp;
	char inbuffer[1024];
	char msgfilename[512];
	char *ptemp;
	int msgnum=0;
	int sqr;

	send_header(sid, 1, 200, "OK", "1", "text/plain", -1, -1);
	if ((ptemp=getgetenv(sid, "MSG"))!=NULL) msgnum=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, status, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, msgnum))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "No such message.<BR>");
		sql_freeresult(sqr);
		return;
	}
	sql_freeresult(sqr);
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, msgnum);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "r");
	if (fp==NULL) {
		return;
	}
	prints(sid, "<PRE>\r\n");
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (strcmp(inbuffer, "")==0) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "\r\n");
	for (;;) {
		wmffgets(sid, inbuffer, sizeof(inbuffer)-1, &fp);
		if (fp==NULL) break;
		printht(sid, "%s\r\n", inbuffer);
	}
	prints(sid, "</PRE>\r\n");
	return;
}

void webmaillist(CONN *sid)
{
	char *ptemp;
	char curdate[40];
	char fromtemp[26];
	char msgsize[20];
	char sortval[40];
	char sortorder[40];
	int folderid;
	int msize;
	int nummessages;
	int offset=0;
	int i;
	int sqr;
	time_t unixdate;

	memset(sortorder, 0, sizeof(sortorder));
	memset(sortval, 0, sizeof(sortval));
	if ((ptemp=getgetenv(sid, "ORDERBY"))!=NULL) {
		snprintf(sortval, sizeof(sortval)-1, "%s", ptemp);
	}
	if (strcasecmp(sortval, "from_a")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_from ASC");
	} else if (strcasecmp(sortval, "from_d")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_from DESC");
	} else if (strcasecmp(sortval, "sub_a")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_subject ASC");
	} else if (strcasecmp(sortval, "sub_d")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_subject DESC");
	} else if (strcasecmp(sortval, "date_a")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_date ASC");
	} else if (strcasecmp(sortval, "date_d")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_date DESC");
	} else if (strcasecmp(sortval, "size_a")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "size ASC");
	} else if (strcasecmp(sortval, "size_d")==0) {
		snprintf(sortorder, sizeof(sortorder)-1, "size DESC");
	} else {
		snprintf(sortorder, sizeof(sortorder)-1, "hdr_date ASC");
		snprintf(sortval, sizeof(sortval)-1, "date_a");
	}
	if ((sqr=sql_queryf(sid, "SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/profile/maileditnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/maileditnew\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
		sql_freeresult(sqr);
		return;
	}
	sql_freeresult(sqr);
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sql_updatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username)==0) {
			sid->dat->user_mailcurrent=atoi(ptemp);
		}
	}
	if ((ptemp=getgetenv(sid, "FOLDERID"))!=NULL) {
		folderid=atoi(ptemp);
	} else {
		folderid=1;
	}
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if (offset<0) offset=0;
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailmbox ACTION=%s/mail/list onChange=\"mailmbox()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	htselect_mailfolder(sid, folderid);
	prints(sid, "</TD></FORM>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailjump ACTION=%s/mail/list onChange=\"mailjump()\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "<TD ALIGN=RIGHT>\r\n");
	htselect_mailjump(sid, sid->dat->user_mailcurrent);
	prints(sid, "</TD></FORM></TR></TABLE>\r\n");
	flushbuffer(sid);
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d AND folder = '%d' AND status != 'd' ORDER BY %s", sid->dat->user_uid, sid->dat->user_mailcurrent, folderid, sortorder))<0) return;
	nummessages=sql_numtuples(sqr);
	if (nummessages<1) {
		prints(sid, "<CENTER><B>You have no messages in this mailbox</B></CENTER><BR>\n");
		sql_freeresult(sqr);
		return;
	}
	sql_updatef(sid, "UPDATE gw_mailheaders SET status = 'o' WHERE accountid = %d AND obj_uid = %d AND status = 'n'", sid->dat->user_mailcurrent, sid->dat->user_uid);
	prints(sid, "<CENTER>\n");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<nummessages) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Next Page</A>]", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		} else {
			prints(sid, "[Next Page]");
		}
		prints(sid, "<BR>\r\n");
	}
	prints(sid, "<script language='JavaScript'>\n<!--\n");
	prints(sid, "function CheckAll()\n{\n");
	prints(sid, "	for (var i=0;i<document.webmail.elements.length;i++) {\n");
	prints(sid, "		var e=document.webmail.elements[i];\n");
	prints(sid, "		if (e.name!='allbox') {\n");
	prints(sid, "			e.checked=document.webmail.allbox.checked;\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "function CheckAll2()\n{\n");
	prints(sid, "	for (var i=0;i<document.webmail.elements.length;i++) {\n");
	prints(sid, "		var e=document.webmail.elements[i];\n");
	prints(sid, "		if (e.name!='allbox2') {\n");
	prints(sid, "			e.checked=document.webmail.allbox2.checked;\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n//-->\n</script>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<FORM METHOD=POST NAME=webmail ACTION=%s/mail/move%s>\n", sid->dat->in_ScriptName, sid->dat->user_menustyle>0?" TARGET=wmread":"");
	prints(sid, "<TR><TD ALIGN=LEFT NOWRAP><NOBR><INPUT TYPE=checkbox NAME=allbox onclick='CheckAll();'>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=delete VALUE=\"Delete\">\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=move VALUE=\"Move to\"><SELECT NAME=dest1>\n");
	htselect_mailfolder2(sid, 0);
	prints(sid, "</SELECT></NOBR></TD>");
	i=nummessages-offset-sid->dat->user_maxlist+1;
	if (i<1) i=1;
	prints(sid, "<TD ALIGN=RIGHT NOWRAP>Listing %d-%d of %d</TD></TR>\n", nummessages-offset, i, nummessages);
	prints(sid, "<TR><TD COLSPAN=2>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_th);
	prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&orderby=%s'\">", sid->dat->in_ScriptName, folderid, strcasecmp(sortval, "from_d")==0?"from_a":"from_d");
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&orderby=%s\"><FONT COLOR=%s>From</FONT></A>&nbsp;</TH>\r\n", folderid, strcasecmp(sortval, "from_d")==0?"from_a":"from_d", config->colour_thtext);
	prints(sid, "<TH ALIGN=LEFT WIDTH=100%% NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&orderby=%s'\">", sid->dat->in_ScriptName, folderid, strcasecmp(sortval, "sub_d")==0?"sub_a":"sub_d");
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&orderby=%s\"><FONT COLOR=%s>Subject</FONT></A>&nbsp;</TH>\r\n", folderid, strcasecmp(sortval, "sub_d")==0?"sub_a":"sub_d", config->colour_thtext);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&orderby=%s'\">", sid->dat->in_ScriptName, folderid, strcasecmp(sortval, "date_a")==0?"date_d":"date_a");
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&orderby=%s\"><FONT COLOR=%s>Date</FONT></A>&nbsp;</TH>\r\n", folderid, strcasecmp(sortval, "date_a")==0?"date_d":"date_a", config->colour_thtext);
	prints(sid, "<TH ALIGN=LEFT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/list?folderid=%d&orderby=%s'\">", sid->dat->in_ScriptName, folderid, strcasecmp(sortval, "size_a")==0?"size_d":"size_a");
	prints(sid, "&nbsp;<A HREF=\"list?folderid=%d&orderby=%s\"><FONT COLOR=%s>Size</FONT></A>&nbsp;</TH>\r\n", folderid, strcasecmp(sortval, "size_a")==0?"size_d":"size_a", config->colour_thtext);
	prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH>");
	prints(sid, "</TR>\n");
	for (i=nummessages-offset-1;(i>-1)&&(i>nummessages-offset-sid->dat->user_maxlist-1);i--) {
		msize=atoi(sql_getvalue(sqr, i, 3));
		if (msize>=1048576) {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f M", (float)msize/1048576.0);
		} else {
			snprintf(msgsize, sizeof(msgsize)-1, "%1.1f K", (float)msize/1024.0);
		}
		memset(fromtemp, 0, sizeof(fromtemp));
		snprintf(fromtemp, sizeof(fromtemp)-1, "%s", DecodeRFC2047(sid, sql_getvalue(sqr, i, 5)));
		if ((ptemp=strchr(fromtemp, '<'))!=NULL) *ptemp='\0';
		if (strcmp(sql_getvalue(sqr, i, 2), "r")!=0) {
			prints(sid, "<TR BGCOLOR=#D0D0FF>");
		} else {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		}
		prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid'><INPUT TYPE=checkbox NAME=%d VALUE=\"%s\"></TD>\r\n", atoi(sql_getvalue(sqr, i, 0)), sql_getvalue(sqr, i, 4));
		prints(sid, "<TD NOWRAP TITLE=\"%s\" STYLE='border-style:solid'>", str2html(sid, DecodeRFC2047(sid, sql_getvalue(sqr, i, 5))));
		prints(sid, "%s&nbsp;</TD>\r\n", str2html(sid, fromtemp));
		if (sid->dat->user_menustyle>0) {
			prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.parent.wmread.location.href='%s/mail/read?msg=%d'\" ", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		} else {
			prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/read?msg=%d'\" ", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		}
		prints(sid, "TITLE=\"%s\">", str2html(sid, DecodeRFC2047(sid, sql_getvalue(sqr, i, 9))));
		prints(sid, "<A HREF=%s/mail/read?msg=%d%s TITLE=\"%s\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), sid->dat->user_menustyle>0?" TARGET=wmread":"", str2html(sid, DecodeRFC2047(sid, sql_getvalue(sqr, i, 9))));
		prints(sid, "%.40s</A>&nbsp;</TD>\r\n", str2html(sid, DecodeRFC2047(sid, sql_getvalue(sqr, i, 9))));
		unixdate=time_sql2unix(sql_getvalue(sqr, i, 8));
		unixdate+=time_tzoffset(sid, unixdate);
		if (unixdate<0) unixdate=0;
		strftime(curdate, 30, "%b %d, %Y %I:%M%p", gmtime(&unixdate));
		curdate[18]=tolower(curdate[18]);
		curdate[19]=tolower(curdate[19]);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s</TD>\r\n", curdate);
		prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s</TD>\r\n", msgsize);
		if (p_strcasestr(sql_getvalue(sqr, i, 11), "multipart/mixed")!=NULL) {
			prints(sid, "<TD STYLE='border-style:solid'><IMG BORDER=0 SRC=/groupware/images/paperclip.gif HEIGHT=16 WIDTH=11 ALT='File Attachments'></TD>");
		} else {
			prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;</TD>\r\n");
		}
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=LEFT NOWRAP><NOBR><INPUT TYPE=checkbox NAME=allbox2 onclick='CheckAll2();'>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=delete VALUE=\"Delete\">\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=move VALUE=\"Move to\"><SELECT NAME=dest2>\n");
	htselect_mailfolder2(sid, 0);
	prints(sid, "</SELECT></NOBR></TD>");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP>&nbsp;</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (nummessages>sid->dat->user_maxlist) {
		if (offset>0) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist);
		} else {
			prints(sid, "[Previous Page]");
		}
		if (offset+sid->dat->user_maxlist<nummessages) {
			prints(sid, "[<A HREF=%s/mail/list?offset=%d>Next Page</A>]", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist);
		} else {
			prints(sid, "[Next Page]");
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void webmailread(CONN *sid)
{
	wmheader header;
	char folder[20];
	char inbuffer[512];
	char msgfilename[512];
	char uidl[100];
	int i;
	int sqr;
	short int nummessages;
	short int localid;
	short int remoteid=-1;
	FILE *fp;
	char curdate[40];
	time_t unixdate;

	memset(folder, 0, sizeof(folder));
	memset(uidl, 0, sizeof(uidl));
	memset((char *)&header, 0, sizeof(header));
	localid=atoi(getgetenv(sid, "MSG"));
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, folder FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' and mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, localid))<0) return;
	if (sql_numtuples(sqr)==1) {
		snprintf(folder, sizeof(folder)-1, "%s", sql_getvalue(sqr, 0, 1));
	}
	sql_freeresult(sqr);
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, folder, uidl, hdr_from, hdr_replyto, hdr_to, hdr_cc, hdr_bcc, hdr_subject, hdr_date, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and folder = '%s' and status != 'd' ORDER BY hdr_date ASC", sid->dat->user_uid, sid->dat->user_mailcurrent, folder))<0) return;
	nummessages=sql_numtuples(sqr);
	for (i=0;i<nummessages;i++) {
		if (localid==atoi(sql_getvalue(sqr, i, 0))) {
			snprintf(uidl, sizeof(uidl)-1, "%s", sql_getvalue(sqr, i, 2));
			snprintf(header.From, sizeof(header.From)-1, "%s", sql_getvalue(sqr, i, 3));
			snprintf(header.ReplyTo, sizeof(header.ReplyTo)-1, "%s", sql_getvalue(sqr, i, 4));
			snprintf(header.To, sizeof(header.To)-1, "%s", sql_getvalue(sqr, i, 5));
			snprintf(header.CC, sizeof(header.CC)-1, "%s", sql_getvalue(sqr, i, 6));
			snprintf(header.BCC, sizeof(header.BCC)-1, "%s", sql_getvalue(sqr, i, 7));
			snprintf(header.Subject, sizeof(header.Subject)-1, "%s", sql_getvalue(sqr, i, 8));
			snprintf(header.Date, sizeof(header.Date)-1, "%s", sql_getvalue(sqr, i, 9));
			snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", sql_getvalue(sqr, i, 10));
			snprintf(header.boundary, sizeof(header.boundary)-1, "%s", sql_getvalue(sqr, i, 11));
			snprintf(header.encoding, sizeof(header.encoding)-1, "%s", sql_getvalue(sqr, i, 12));
			remoteid=i+1;
			break;
		}
	}
	if (remoteid==-1) {
		prints(sid, "No such message.<BR>");
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this message?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (remoteid>1) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Previous</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid-2, 0)));
	} else {
		prints(sid, "[Previous]\n");
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo(%d)>Reply</A>]\n", localid);
		prints(sid, "[<A HREF=javascript:ReplyAll(%d)>Reply All</A>]\n", localid);
		prints(sid, "[<A HREF=javascript:Forward(%d)>Forward</A>]\n", localid);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d>Reply</A>]\n", sid->dat->in_ScriptName, localid);
		prints(sid, "[<A HREF=%s/mail/write?replyall=%d>Reply All</A>]\n", sid->dat->in_ScriptName, localid);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d>Forward</A>]\n", sid->dat->in_ScriptName, localid);
	}
	prints(sid, "[<A HREF=%s/mail/move?%d=%s onClick=\"return ConfirmDelete();\">Delete</A>]\n", sid->dat->in_ScriptName, localid, uidl);
	if ((remoteid<nummessages)&&(remoteid>-1)) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Next</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid, 0)));
	} else {
		prints(sid, "[Next]\n");
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:1px; border-style:solid'><TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n", config->colour_fieldval);
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;From   &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;<A HREF=javascript:MsgTo(\"%s\");>", config->colour_th, config->colour_thtext, config->colour_fieldval, header.ReplyTo);
	printht(sid, "%s", DecodeRFC2047(sid, header.From));
	prints(sid, "</A>&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;To     &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", config->colour_th, config->colour_thtext, config->colour_fieldval);
	printht(sid, "%s", DecodeRFC2047(sid, header.To));
	prints(sid, "&nbsp;</TD></TR>\n");
	if (strlen(header.CC)) {
		prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;CC     &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", config->colour_th, config->colour_thtext, config->colour_fieldval);
		printht(sid, "%s", DecodeRFC2047(sid, header.CC));
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	if (strlen(header.BCC)) {
		prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;BCC     &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", config->colour_th, config->colour_thtext, config->colour_fieldval);
		printht(sid, "%s", DecodeRFC2047(sid, header.BCC));
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;Subject&nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", config->colour_th, config->colour_thtext, config->colour_fieldval);
	printht(sid, "%s", DecodeRFC2047(sid, header.Subject));
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "<TR><TH ALIGN=LEFT BGCOLOR=%s VALIGN=TOP><FONT COLOR=%s>&nbsp;Date   &nbsp;</FONT></TH><TD BGCOLOR=%s WIDTH=100%%>&nbsp;", config->colour_th, config->colour_thtext, config->colour_fieldval);
	memset(curdate, 0, sizeof(curdate));
	unixdate=time_sql2unix(header.Date);
	unixdate+=time_tzoffset(sid, unixdate);
	if (unixdate<0) unixdate=0;
	strftime(curdate, 30, "%B %d, %Y %I:%M %p", gmtime(&unixdate));
	printht(sid, "%s", curdate);
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'>[<A HREF=%s/mail/raw?msg=%d TARGET=_blank>View Source</A>]</TD></TR>\n", config->colour_fieldval, sid->dat->in_ScriptName, localid);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'>\n", config->colour_fieldval);
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, localid);
	fixslashes(msgfilename);
	sql_updatef(sid, "UPDATE gw_mailheaders SET status = 'r' WHERE uidl = '%s' AND obj_uid = %d AND accountid = %d", str2sql(sid, uidl), sid->dat->user_uid, sid->dat->user_mailcurrent);
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
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Previous</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid-2, 0)));
	} else {
		prints(sid, "[Previous]\n");
	}
	if (sid->dat->user_menustyle>0) {
		prints(sid, "[<A HREF=javascript:ReplyTo(%d)>Reply</A>]\n", localid);
		prints(sid, "[<A HREF=javascript:ReplyAll(%d)>Reply All</A>]\n", localid);
		prints(sid, "[<A HREF=javascript:Forward(%d)>Forward</A>]\n", localid);
	} else {
		prints(sid, "[<A HREF=%s/mail/write?replyto=%d>Reply</A>]\n", sid->dat->in_ScriptName, localid);
		prints(sid, "[<A HREF=%s/mail/write?replyall=%d>Reply All</A>]\n", sid->dat->in_ScriptName, localid);
		prints(sid, "[<A HREF=%s/mail/write?forward=%d>Forward</A>]\n", sid->dat->in_ScriptName, localid);
	}
	prints(sid, "[<A HREF=%s/mail/move?%d=%s onClick=\"return ConfirmDelete();\">Delete</A>]\n", sid->dat->in_ScriptName, localid, uidl);
	if ((remoteid<nummessages)&&(remoteid>-1)) {
		prints(sid, "[<A HREF=%s/mail/read?msg=%d>Next</A>]\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, remoteid, 0)));
	} else {
		prints(sid, "[Next]\n");
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
	short int replyto=0;
	short int replyall=0;
	short int forward=0;
	short int msgnum=0;
	int sqr;

	memset(msgto, 0, sizeof(msgto));
	memset(msgcc, 0, sizeof(msgcc));
	memset(msgbcc, 0, sizeof(msgbcc));
	memset(subject, 0, sizeof(subject));
	if ((ptemp=getgetenv(sid, "REPLYTO"))!=NULL) replyto=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "REPLYALL"))!=NULL) { replyto=atoi(ptemp); replyall=1; }
	if ((ptemp=getgetenv(sid, "FORWARD"))!=NULL) forward=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "TO"))!=NULL) strncpy(msgto, ptemp, sizeof(msgto)-1);
	if ((ptemp=getgetenv(sid, "CC"))!=NULL) strncpy(msgcc, ptemp, sizeof(msgcc)-1);
	if ((ptemp=getgetenv(sid, "BCC"))!=NULL) strncpy(msgbcc, getgetenv(sid, "BCC"), sizeof(msgbcc)-1);
	if (replyto>0) {
		msgnum=replyto;
	} else if (forward>0) {
		msgnum=forward;
	} else {
		msgnum=0;
	}
	if (msgnum) {
		memset((char *)&header, 0, sizeof(header));
		if ((sqr=sql_queryf(sid, "SELECT mailheaderid, status, uidl, hdr_from, hdr_replyto, hdr_to, hdr_date, hdr_subject, hdr_cc, hdr_contenttype, hdr_boundary, hdr_encoding FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->user_uid, sid->dat->user_mailcurrent, msgnum))<0) return;
		if (sql_numtuples(sqr)!=1) {
			sql_freeresult(sqr);
			return;
		}
		snprintf(header.From, sizeof(header.From)-1, "%s", sql_getvalue(sqr, 0, 3));
		snprintf(header.ReplyTo, sizeof(header.ReplyTo)-1, "%s", sql_getvalue(sqr, 0, 4));
		snprintf(header.To, sizeof(header.To)-1, "%s", sql_getvalue(sqr, 0, 5));
		snprintf(header.Date, sizeof(header.Date)-1, "%s", sql_getvalue(sqr, 0, 6));
		snprintf(header.Subject, sizeof(header.Subject)-1, "%s", sql_getvalue(sqr, 0, 7));
		snprintf(header.CC, sizeof(header.CC)-1, "%s", sql_getvalue(sqr, 0, 8));
		snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", sql_getvalue(sqr, 0, 9));
		snprintf(header.boundary, sizeof(header.boundary)-1, "%s", sql_getvalue(sqr, 0, 10));
		snprintf(header.encoding, sizeof(header.encoding)-1, "%s", sql_getvalue(sqr, 0, 11));
		sql_freeresult(sqr);
		if (replyto>0) {
			snprintf(subject, sizeof(subject)-1, "%s%s", (strncasecmp(DecodeRFC2047(sid, header.Subject), "RE:", 3)==0)?"":"Re: ", DecodeRFC2047(sid, header.Subject));
		} else if (forward>0) {
			snprintf(subject, sizeof(subject)-1, "%s%s", (strncasecmp(DecodeRFC2047(sid, header.Subject), "FWD:", 4)==0)?"":"Fwd: ", DecodeRFC2047(sid, header.Subject));
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
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%%>&nbsp;<B>From</B>&nbsp;</TD><TD WIDTH=90%%>\n", config->colour_editform);
	prints(sid, "<SELECT NAME=accountid style='width:433px'>\n");
	htselect_mailaccount(sid, sid->dat->user_mailcurrent);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('to');>&nbsp;<B><A HREF=javascript:AddressBook('to') TITLE='To'>TO</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgto SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", config->colour_editform, str2html(sid, msgto));
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('cc');>&nbsp;<B><A HREF=javascript:AddressBook('cc') TITLE='Carbon Copy'>CC</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgcc SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", config->colour_editform, str2html(sid, msgcc));
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%% STYLE=\"cursor:hand\" onClick=AddressBook('bcc');>&nbsp;<B><A HREF=javascript:AddressBook('bcc') TITLE='Blind Carbon Copy'>BCC</A></B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgbcc SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", config->colour_editform, str2html(sid, msgbcc));
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%%>&nbsp;<B>Subject</B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=TEXT NAME=msgsubject SIZE=80 STYLE='width:433px' VALUE=\"%s\"></TD></TR>\n", config->colour_editform, str2html(sid, subject));
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%% STYLE='padding:0px'><B>&nbsp;Format&nbsp;</B></TD><TD STYLE='padding:0px'>", config->colour_editform);
	prints(sid, "<select NAME=ctype onChange=\"toggle_mode()\">\r\n<option value=plain>Plain Text</option>\r\n<option value=html>HTML</option>\r\n</select>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>", config->colour_editform);
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
	prints(sid, "<CENTER><IFRAME id=\"wmeditor\" HEIGHT=236 width=646></IFRAME></CENTER>");
	prints(sid, "</DIV>");
	prints(sid, "<CENTER><TEXTAREA NAME=msgbody COLS=78 ROWS=16 WRAP=VIRTUAL>\n");
	if (msgnum) {
		prints(sid, "--- %s wrote:\n", header.From);
		if (forward>0) {
			prints(sid, "> From:    %s\n", header.From);
			prints(sid, "> Subject: %s\n", header.Subject);
			prints(sid, "> Date:    %s\n", header.Date);
			prints(sid, ">\n");
		}
		memset(msgfilename, 0, sizeof(msgfilename));
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, msgnum);
		fixslashes(msgfilename);
		fp=fopen(msgfilename, "r");
		if (fp!=NULL) {
			while (fgets(inbuffer, sizeof(inbuffer)-1, fp)!=NULL) {
				striprn(inbuffer);
				if (strlen(inbuffer)==0) break;
			}
			webmailmime(sid, &fp, header.contenttype, header.encoding, header.boundary, msgnum, 1, 0);
		} else {
			prints(sid, "\r\nCould not retrieve message body!\r\n");
		}
	}
	prints(sid, "</TEXTAREA></CENTER>\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TD WIDTH=10%%>&nbsp;<B>Attachment</B>&nbsp;</TD><TD WIDTH=90%%><INPUT TYPE=FILE NAME=fattach SIZE=70></TD></TR>\n", config->colour_editform);
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton VALUE='Send Mail'>\n");
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "document.wmcompose.msgto.focus();\r\n");
	prints(sid, "//-->\r\n</script>\r\n");
	return;
}

void webmailsave(CONN *sid)
{
	struct stat sb;
	FILE *fp;
	wmheader header;
	char *filebody=NULL;
	char *msgbody=NULL;
	char *pmsgbody;
	char *ptemp;
	char query[4096];
	char filename[512];
	char cfilesize[10];
	char msgctype[16];
	char datebuf[40];
	char date_tz[10];
	char line[1024];
	char msgfilename[512];
	int filesize=0;
	struct timeval ttime;
	struct timezone tzone;
	unsigned int mimesize;
	int sqr;
	int headerid;

	if (strcasecmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	prints(sid, "Saving File");
	memset(datebuf, 0, sizeof(datebuf));
	if ((ptemp=getmimeenv(sid, "ACCOUNTID", &mimesize))!=NULL) {
		strncpy(datebuf, ptemp, sizeof(datebuf)-1);
		if (mimesize<strlen(datebuf)) datebuf[mimesize]='\0';
		sid->dat->user_mailcurrent=atoi(datebuf);
	}
	if ((sqr=sql_queryf(sid, "SELECT realname, organization, popusername, poppassword, hosttype, pophost, popport, smtphost, smtpport, address, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		if ((sqr=sql_queryf(sid, "SELECT mailaccountid FROM gw_mailaccounts where obj_uid = %d", sid->dat->user_uid))<0) return;
		if (sql_numtuples(sqr)<1) {
			sql_freeresult(sqr);
			return;
		}
		sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr, 0, 0));
		if (sql_updatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", sid->dat->user_mailcurrent, sid->dat->user_username)<0) return;
		sql_freeresult(sqr);
		if ((sqr=sql_queryf(sid, "SELECT realname, organization, popusername, poppassword, hosttype, pophost, popport, smtphost, smtpport, address, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid))<0) return;
	}
	if (sql_numtuples(sqr)==1) {
		strncpy(sid->dat->wm->realname, sql_getvalue(sqr, 0, 0), sizeof(sid->dat->wm->realname)-1);
		strncpy(sid->dat->wm->organization, sql_getvalue(sqr, 0, 1), sizeof(sid->dat->wm->organization)-1);
		strncpy(sid->dat->wm->username, sql_getvalue(sqr, 0, 2), sizeof(sid->dat->wm->username)-1);
		strncpy(sid->dat->wm->password, DecodeBase64string(sid, sql_getvalue(sqr, 0, 3)), sizeof(sid->dat->wm->password)-1);
		strncpy(sid->dat->wm->servertype, sql_getvalue(sqr, 0, 4), sizeof(sid->dat->wm->servertype)-1);
		strncpy(sid->dat->wm->popserver, sql_getvalue(sqr, 0, 5), sizeof(sid->dat->wm->popserver)-1);
		sid->dat->wm->popport=atoi(sql_getvalue(sqr, 0, 6));
		strncpy(sid->dat->wm->smtpserver, sql_getvalue(sqr, 0, 7), sizeof(sid->dat->wm->smtpserver)-1);
		sid->dat->wm->smtpport=atoi(sql_getvalue(sqr, 0, 8));
		strncpy(sid->dat->wm->replyto, sql_getvalue(sqr, 0, 9), sizeof(sid->dat->wm->replyto)-1);
		strncpy(sid->dat->wm->signature, sql_getvalue(sqr, 0, 10), sizeof(sid->dat->wm->signature)-1);
	}
	sql_freeresult(sqr);
	prints(sid, ".");
	memset((char *)&header, 0, sizeof(header));
	memset(msgctype, 0, sizeof(msgctype));
	memset(filename, 0, sizeof(filename));
	memset(cfilesize, 0, sizeof(cfilesize));
	if (msgbody!=NULL) {
		free(msgbody);
		msgbody=NULL;
	}
	gettimeofday(&ttime, &tzone);
	snprintf(header.boundary, sizeof(header.boundary)-1, "------------NGW%d", (int)ttime.tv_sec);
	msgbody=calloc(sid->dat->in_ContentLength+1024, sizeof(char));
	if (getmimeenv(sid, "MSGTO", &mimesize)!=NULL) {
		strncpy(header.To, getmimeenv(sid, "MSGTO", &mimesize), sizeof(header.To)-1);
		if (mimesize<strlen(header.To)) header.To[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGCC", &mimesize)!=NULL) {
		strncpy(header.CC, getmimeenv(sid, "MSGCC", &mimesize), sizeof(header.CC)-1);
		if (mimesize<strlen(header.CC)) header.CC[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGBCC", &mimesize)!=NULL) {
		strncpy(header.BCC, getmimeenv(sid, "MSGBCC", &mimesize), sizeof(header.BCC)-1);
		if (mimesize<strlen(header.BCC)) header.BCC[mimesize]='\0';
	}
	if (getmimeenv(sid, "MSGSUBJECT", &mimesize)!=NULL) {
		strncpy(header.Subject, getmimeenv(sid, "MSGSUBJECT", &mimesize), sizeof(header.Subject)-1);
		if (mimesize<strlen(header.Subject)) header.Subject[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "CTYPE", &mimesize))!=NULL) {
		snprintf(msgctype, sizeof(msgctype)-1, "text/%s", ptemp);
		if (mimesize+5<strlen(msgctype)) msgctype[mimesize+5]='\0';
	} else {
		snprintf(msgctype, sizeof(msgctype)-1, "text/plain");
	}
	if (getmimeenv(sid, "MSGBODY", &mimesize)!=NULL) {
		strncpy(msgbody, getmimeenv(sid, "MSGBODY", &mimesize), sid->dat->in_ContentLength+1023);
		if (mimesize<strlen(msgbody)) msgbody[mimesize]='\0';
	}
	if (getmimeenv(sid, "FATTACH", &mimesize)!=NULL) {
		filebody=webmailfileul(sid, filename, cfilesize);
		filesize=atoi(cfilesize);
		if (strlen(filename)==0) filesize=0;
	}
	gettimeofday(&ttime, &tzone);
	ttime.tv_sec+=time_tzoffset(sid, ttime.tv_sec);
	strftime(header.Date, sizeof(header.Date), "%a, %d %b %Y %H:%M:%S", gmtime(&ttime.tv_sec));
	snprintf(date_tz, sizeof(date_tz)-1, " %+.4d", +time_tzoffset(sid, ttime.tv_sec)/36);
	strncat(header.Date, date_tz, sizeof(header.Date)-strlen(header.Date)-1);
	if (filesize>0) {
		snprintf(header.contenttype, sizeof(header.contenttype)-1, "multipart/mixed; boundary=\"%s\"", header.boundary);
	} else {
		snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", msgctype);
	}
	if ((sqr=sql_queryf(sid, "SELECT max(mailheaderid) FROM gw_mailheaders where accountid = %d", sid->dat->user_mailcurrent))<0) return;
	headerid=atoi(sql_getvalue(sqr, 0, 0))+1;
	sql_freeresult(sqr);
	if (headerid<1) headerid=1;
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", gmtime(&ttime.tv_sec));
	memset(query, 0, sizeof(query));
	strcpy(query, "INSERT INTO gw_mailheaders (mailheaderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_cc, hdr_bcc, hdr_subject, hdr_date, hdr_contenttype, hdr_boundary, hdr_encoding) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", headerid, datebuf, datebuf, sid->dat->user_uid, 0, 0, 0);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d", sid->dat->user_mailcurrent);
	strncatf(query, sizeof(query)-strlen(query)-1, "', '2"); /* folder */
	strncatf(query, sizeof(query)-strlen(query)-1, "', 'r");
	strncatf(query, sizeof(query)-strlen(query)-1, "', '0"); // MSG SIZE !!!
	strncatf(query, sizeof(query)-strlen(query)-1, "', 'uidl");
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s <%s>", str2sql(sid, sid->dat->wm->realname), str2sql(sid, sid->dat->wm->replyto));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, sid->dat->wm->replyto));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, header.To));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, header.CC));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, header.BCC));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, header.Subject));
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", gmtime(&ttime.tv_sec));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, datebuf));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(sid, header.contenttype));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '--%s", str2sql(sid, header.boundary));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s')", str2sql(sid, header.encoding));
	if (sql_update(sid, query)<0) return;
	prints(sid, ".");
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d", config->server_dir_var_mail, sid->dat->user_mailcurrent);
	fixslashes(msgfilename);
	if ((stat(msgfilename, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(msgfilename)!=0) {
#else
		if (mkdir(msgfilename, 0755)!=0) {
#endif
			logerror(NULL, __FILE__, __LINE__, "ERROR: Maildir '%s' is not accessible!", msgfilename);
			return;
		}
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, headerid);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "wb");
	if (fp==NULL) {
		logerror(NULL, __FILE__, __LINE__, "ERROR: Could not open message '%s'!", msgfilename);
		return;
	}
	fprintf(fp, "From: \"%s\" <%s>\r\n", sid->dat->wm->realname, sid->dat->wm->replyto);
	fprintf(fp, "Reply-To: <%s>\r\n", sid->dat->wm->replyto);
	if (strlen(sid->dat->wm->organization)>0) {
		fprintf(fp, "Organization: %s\r\n", sid->dat->wm->organization);
	}
	fprintf(fp, "To: %s\r\n", header.To);
	if (strlen(header.CC)) {
		fprintf(fp, "Cc: %s\r\n", header.CC);
	}
	fprintf(fp, "Subject: %s\r\n", header.Subject);
	fprintf(fp, "Date: %s\r\n", header.Date);
	if (filesize>0) {
		fprintf(fp, "MIME-Version: 1.0\r\n");
		fprintf(fp, "Content-Type: multipart/mixed; boundary=\"%s\"\r\n", header.boundary);
	} else {
		fprintf(fp, "Content-Type: %s\r\n", header.contenttype);
	}
	fprintf(fp, "X-Mailer: %s %s\r\n", SERVER_NAME, SERVER_VERSION);
	fprintf(fp, "\r\n");
	if (filesize>0) {
		fprintf(fp, "This is a multi-part message in MIME format.\r\n\r\n");
		fprintf(fp, "--%s\r\n", header.boundary);
		fprintf(fp, "Content-Type: %s\r\n", msgctype);
		fprintf(fp, "Content-Transfer-Encoding: 8bit\r\n\r\n");
	}
	pmsgbody=msgbody;
	while (strlen(pmsgbody)>78) {
		memset(line, 0, sizeof(line));
		snprintf(line, 79, "%s", pmsgbody);
		line[78]='\0';
		if (strchr(line, '\r')||strchr(line, '\n')) {
			if ((ptemp=strchr(line, '\r'))!=NULL) *ptemp='\0';
			if ((ptemp=strchr(line, '\n'))!=NULL) *ptemp='\0';
			fprintf(fp, "%s\r\n", line);
			pmsgbody+=strlen(line);
			if (*pmsgbody=='\r') pmsgbody++;
			if (*pmsgbody=='\n') pmsgbody++;
		} else if ((ptemp=strrchr(line, ' '))!=NULL) {
			*ptemp='\0';
			fprintf(fp, "%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else {
			fprintf(fp, "%s", line);
			pmsgbody+=strlen(line);
		}
	}
	memset(line, 0, sizeof(line));
	snprintf(line, 78, "%s", pmsgbody);
	if (strlen(line)) fprintf(fp, "%s\r\n", line);
	free(msgbody);
	if (strlen(sid->dat->wm->signature)>0) {
		fprintf(fp, "\r\n");
		if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "<PRE>");
		fprintf(fp, "%s", sid->dat->wm->signature);
		if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "</PRE>");
		fprintf(fp, "\r\n");
	}
	if (filesize>0) {
		fprintf(fp, "\r\n--%s\r\n", header.boundary);
		fprintf(fp, "Content-Type: application/octet-stream; name=\"%s\"\r\n", filename);
		fprintf(fp, "Content-Transfer-Encoding: base64\r\n");
		fprintf(fp, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", filename);
		EncodeBase64file(fp, filebody, filesize);
		fprintf(fp, "\r\n--%s--\r\n\r\n", header.boundary);
	}
	fclose(fp);
	prints(sid, ".");
	if ((stat(msgfilename, &sb)!=0)||(sb.st_mode&S_IFDIR)) return;
	sql_updatef(sid, "UPDATE gw_mailheaders SET size = '%d' WHERE mailheaderid = %d AND accountid = %d", sb.st_size, headerid, sid->dat->user_mailcurrent);
	prints(sid, "OK<BR>\r\n");
	prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]<BR>\n", sid->dat->in_ScriptName);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\nwindow.close();\n// -->\n</SCRIPT>\n");
	return;
}

void webmailmove(CONN *sid)
{
	char *ptemp1;
	char *ptemp2;
	char msgnum[8];
//	char uidl[100];
	int folderid=0;
	int oldfolderid=1;
	int nummessages;
//	int deleted=0;
	int i;
	int sqr;
	int move=0;
	int purge=0;

	if (((ptemp1=getpostenv(sid, "MOVE"))!=NULL)&&(strcasecmp(ptemp1, "Move to")==0)) {
		if ((ptemp1=getpostenv(sid, "DEST1"))!=NULL) {
			folderid=atoi(ptemp1);
		}
		if (folderid<1) {
			if ((ptemp1=getpostenv(sid, "DEST2"))!=NULL) {
				folderid=atoi(ptemp1);
			}
		}
		move=1;
	} else {
		folderid=4;
	}
	if ((sqr=sql_queryf(sid, "SELECT mailheaderid, uidl, status, folder FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status != 'd'", sid->dat->user_uid, sid->dat->user_mailcurrent))<0) return;
	nummessages=sql_numtuples(sqr);
	if (nummessages<1) {
		prints(sid, "<CENTER><B>You have no messages in this mailbox</B></CENTER><BR>\n");
	}
	for (i=0;i<nummessages;i++) {
		memset(msgnum, 0, sizeof(msgnum));
		snprintf(msgnum, sizeof(msgnum)-1, "%d", atoi(sql_getvalue(sqr, i, 0)));
		ptemp1=getpostenv(sid, msgnum);
		ptemp2=getgetenv(sid, msgnum);
		if (ptemp1==NULL) ptemp1=ptemp2;
		if (ptemp1==NULL) continue;
		if (strcmp(ptemp1, sql_getvalue(sqr, i, 1))!=0) continue;
		if (folderid>0) {
			prints(sid, "%s message %d...", move?"Moving":"Deleting", atoi(sql_getvalue(sqr, i, 0)));
			oldfolderid=atoi(sql_getvalue(sqr, i, 3));
			if ((oldfolderid==4)&&(folderid==4)) {
				sql_updatef(sid, "UPDATE gw_mailheaders SET status = 'd' WHERE accountid = %d AND obj_uid = %d and mailheaderid = %d", sid->dat->user_mailcurrent, sid->dat->user_uid, atoi(sql_getvalue(sqr, i, 0)));
				purge=1;
			} else {
				sql_updatef(sid, "UPDATE gw_mailheaders SET folder = '%d' WHERE accountid = %d AND obj_uid = %d and mailheaderid = %d", folderid, sid->dat->user_mailcurrent, sid->dat->user_uid, atoi(sql_getvalue(sqr, i, 0)));
			}
			prints(sid, "success.<BR>\n");
		}
//		deleted=i;
		flushbuffer(sid);
	}
	if (purge) wmserver_purge(sid, 2);
//	snprintf(msgnum, sizeof(msgnum)-1, "%d", deleted);
//	if (deleted>=nummessages) deleted=nummessages-1;
	if (sid->dat->user_menustyle>0) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "parent.wmlist.location=\"%s/mail/list?folderid=%d\";\n", sid->dat->in_ScriptName, oldfolderid);
		prints(sid, "// -->\n</SCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/null\">\n", sid->dat->in_ScriptName);
	} else {
//		if ((getgetenv(sid, msgnum)==NULL)||(nummessages<2)) {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/list\">\n", sid->dat->in_ScriptName);
//		} else {
//			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/read?msg=%d\">\n", sid->dat->in_ScriptName, deleted);
//		}
	}
	sql_freeresult(sqr);
	return;
}

void webmailpurge(CONN *sid)
{
	struct stat sb;
	char msgfilename[512];
	char newfilename[512];
	char *ptemp;
	int accountid;
	int err;
	int i;
	int j;
	int sqr;

	if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
	prints(sid, "<B>Purging...</B>");
	flushbuffer(sid);
	accountid=atoi(ptemp);
	if ((sqr=sql_queryf(sid, "SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status = 'd'", sid->dat->user_uid, accountid))<0) return;
	err=0;
	memset(msgfilename, 0, sizeof(msgfilename));
	memset(newfilename, 0, sizeof(newfilename));
	for (i=0;i<sql_numtuples(sqr);i++) {
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, accountid, atoi(sql_getvalue(sqr, i, 0)));
		if (stat(msgfilename, &sb)==0) {
			if (unlink(msgfilename)!=0) {
				prints(sid, "<B>'%s' could not be deleted.</B><BR>\n", msgfilename);
				err++;
			}
		}
	}
	sql_freeresult(sqr);
	if (err==0) {
		sql_updatef(sid, "DELETE FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d and status = 'd'", sid->dat->user_uid, accountid);
	}
	if ((sqr=sql_queryf(sid, "SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d", sid->dat->user_uid, accountid))<0) return;
	for (i=0,j=1;i<sql_numtuples(sqr);i++) {
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, accountid, atoi(sql_getvalue(sqr, i, 0)));
		snprintf(newfilename, sizeof(newfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, accountid, j);
		if (rename(msgfilename, newfilename)==0) {
			if (sql_updatef(sid, "UPDATE gw_mailheaders SET mailheaderid = '%d' WHERE obj_uid = %d and accountid = %d and mailheaderid = %d", j, sid->dat->user_uid, accountid, atoi(sql_getvalue(sqr, i, 0)))<0) {
				rename(newfilename, msgfilename);
				break;
			}
			j++;
		}
	}
	sql_freeresult(sqr);
	prints(sid, "<B>done.</B><BR>\n");
	return;
}

void webmailframeset(CONN *sid)
{
	char *ptemp;
	int sqr;

	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<HTML>\n<HEAD>\n<TITLE>NullLogic Groupware Webmail</TITLE>\n</HEAD>\n");
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))!=NULL) {
		if (sql_updatef(sid, "UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", atoi(ptemp), sid->dat->user_username)==0) {
			sid->dat->user_mailcurrent=atoi(ptemp);
		}
	}
	if ((sqr=sql_queryf(sid, "SELECT mailaccountid FROM gw_mailaccounts WHERE obj_uid = %d", sid->dat->user_uid))<0) return;
	if (sql_numtuples(sqr)<1) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "location.replace(\"%s/profile/maileditnew\");\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/maileditnew\">\n", sid->dat->in_ScriptName);
		prints(sid, "</NOSCRIPT>\n");
		prints(sid, "</HTML>\n");
	} else {
		prints(sid, "<FRAMESET ROWS=\"50%%,50%%\" BORDER=0 FRAMEBORDER=1 FRAMESPACING=1>\n");
		prints(sid, "<FRAME BORDER=0 NAME=\"wmlist\" SRC=%s/mail/list MARGINHEIGHT=1 MARGINWIDTH=1 SCROLLING=YES>\n", sid->dat->in_ScriptName);
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
	if (sid->dat->wm==NULL) {
		if ((sid->dat->wm=calloc(1, sizeof(WEBMAIL)))==NULL) return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/main", 10)==0) {
		if (sid->dat->user_menustyle>0) {
			webmailframeset(sid);
			goto done;
		} else {
			/* HACK THE URI?  CRUDE, BUT EFFECTIVE */
			snprintf(sid->dat->in_RequestURI, sizeof(sid->dat->in_RequestURI)-1, "/mail/list");
		}
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/file", 10)==0) {
		webmailfiledl(sid);
		goto done;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/mail/notice", 12)==0) {
		htpage_header(sid, "E-Mail Notice");
		wmnotice(sid);
		htpage_footer(sid);
		goto done;
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/quit", 10)==0) {
		wmlogout(sid);
	}
	htpage_topmenu(sid, MENU_WEBMAIL);
	flushbuffer(sid);
	if (strncmp(sid->dat->in_RequestURI, "/mail/addresses", 15)==0) {
		wmaddr_list(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/quit", 10)==0) {
		wmloginform(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/read", 10)==0) {
		webmailread(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/write", 11)==0) {
		webmailwrite(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/list", 10)==0) {
		webmaillist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/move", 10)==0) {
		webmailmove(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/purge", 11)==0) {
		webmailpurge(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/save", 10)==0) {
		webmailsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/sync", 10)==0) {
		wmsync(sid, 1);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/raw", 9)==0) {
		webmailraw(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/folders/edit", 18)==0) {
		wmfolder_edit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/folders/save", 18)==0) {
		wmfolder_save(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/mail/folders", 13)==0) {
		wmfolder_list(sid);
	}
	htpage_footer(sid);
done:
	if (sid->dat->wm!=NULL) {
		free(sid->dat->wm);
		sid->dat->wm=NULL;
	}
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod;

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	memset((char *)&newmod, 0, sizeof(newmod));
	newmod.mod_submenu=1;
	snprintf(newmod.mod_name,     sizeof(newmod.mod_name)-1,     "mod_mail");
	snprintf(newmod.mod_menuname, sizeof(newmod.mod_menuname)-1, "E-MAIL");
	snprintf(newmod.mod_menuperm, sizeof(newmod.mod_menuperm)-1, "webmail");
	snprintf(newmod.mod_menuuri,  sizeof(newmod.mod_menuuri)-1,  "/mail/main");
	snprintf(newmod.fn_name,      sizeof(newmod.fn_name)-1,      "mod_main");
	snprintf(newmod.fn_uri,       sizeof(newmod.fn_uri)-1,       "/mail/");
	newmod.fn_ptr=mod_main;
	if (mod_export_main(&newmod)!=0) return -1;
	if (mod_export_function("mod_mail", "mod_mail_sync", wmsync)!=0) return -1;
	return 0;
}
