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
#include "http_mod.h"
#include "mod_mail.h"

void wmaccount_edit(CONN *sid)
{
	REC_MAILACCT mailacct;
	char *ptemp;
	int accountid;
	int i;

	if (!(auth_priv(sid, "webmail")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/mail/accounts/editnew", 22)==0) {
		accountid=0;
		if (dbread_mailaccount(sid, 2, 0, &mailacct)!=0) {
			prints(sid, "<CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
			return;
		}
	} else {
		if ((ptemp=getgetenv(sid, "ACCOUNT"))==NULL) return;
		accountid=atoi(ptemp);
		if (dbread_mailaccount(sid, 2, accountid, &mailacct)!=0) {
			prints(sid, "<CENTER>" ERR_NORECORD "</CENTER>\n", accountid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"" WARN_DELCONFIRM "\");\n");
	prints(sid, "}\n");
	prints(sid, "function PortUpdate() {\n");
	prints(sid, "	if (document.profilemailedit.hosttype.value=='POP3') {\n");
	prints(sid, "		document.profilemailedit.popport.value='110';\n");
	prints(sid, "	}\n");
	prints(sid, "	if (document.profilemailedit.hosttype.value=='IMAP') {\n");
	prints(sid, "		document.profilemailedit.popport.value='143';\n");
	prints(sid, "	}\n");
	prints(sid, "	document.getElementById('hosttype1').innerHTML=document.profilemailedit.hosttype.value;\n");
	prints(sid, "	document.getElementById('hosttype2').innerHTML=document.profilemailedit.hosttype.value;\n");
	prints(sid, "	return;\n");
	prints(sid, "}\n");
	if (mailacct.mailaccountid!=0) {
		htscript_showpage(sid, 6);
	} else {
		htscript_showpage(sid, 4);
	}
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<BR><CENTER>\n");
	if (mailacct.mailaccountid!=0) {
		prints(sid, "<B>Mail Account %d - (%s:%d)</B>\n", mailacct.mailaccountid, sid->dat->user_username, mailacct.mailaccountid);
	} else {
		prints(sid, "<B>New Mail Account</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=430>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/accounts/save NAME=profilemailedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=mailaccountid VALUE='%d'>\n", mailacct.mailaccountid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>" MOD_MAIL_TAB_GEN "</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>" MOD_MAIL_TAB_SER "</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>" MOD_MAIL_TAB_ADV "</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>" MOD_MAIL_TAB_SIG "</A>&nbsp;</TD>\n");
	if (mailacct.mailaccountid!=0) {
		prints(sid, "<TD ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=5 HREF=javascript:showpage(5)>" MOD_MAIL_TAB_FLD "</A>&nbsp;</TD>\n");
		prints(sid, "<TD ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=6 HREF=javascript:showpage(6)>" MOD_MAIL_TAB_FIL "</A>&nbsp;</TD>\n");
	}
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Account Name   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=accountname  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.accountname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Real Name      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=realname     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.realname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Organization   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.organization));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;E-Mail Address </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address      VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.address));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Reply Address  </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=replyto      VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.replyto));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Host Type      </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=hosttype style='width:217px' onchange=PortUpdate();>\n");
	prints(sid, "<OPTION VALUE='POP3'%s>POP3\n", strcasecmp(mailacct.hosttype, "POP3")==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='IMAP'%s>IMAP\n", strcasecmp(mailacct.hosttype, "IMAP")==0?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<SPAN ID=hosttype1>%s</SPAN> Host </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pophost VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", strcasecmp(mailacct.hosttype, "IMAP")==0?"IMAP":"POP3", str2html(sid, mailacct.pophost));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Host      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtphost     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.smtphost));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Username       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popusername  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.popusername));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=poppassword  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", str2html(sid, mailacct.poppassword));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Auth      </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=smtpauth style='width:217px'>\n");
	prints(sid, "<OPTION VALUE='n'%s>No\n", mailacct.smtpauth[0]=='n'?" SELECTED":"");
	prints(sid, "<OPTION VALUE='y'%s>Yes\n", mailacct.smtpauth[0]!='n'?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<SPAN ID=hosttype2>%s</SPAN> Port </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popport      VALUE=\"%d\" SIZE=30 style='width:217px'></TD></TR>\n", strcasecmp(mailacct.hosttype, "IMAP")==0?"IMAP":"POP3", mailacct.popport);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Port      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtpport     VALUE=\"%d\" SIZE=30 style='width:217px'></TD></TR>\n", mailacct.smtpport);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Check for new mail</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=notify style='width:217px'>\n");
	prints(sid, "<OPTION VALUE='0'%s>never\n", mailacct.notify==0?" SELECTED":"");
	for (i=1;i<61;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>every %d minute%s\n", i, i==mailacct.notify?" SELECTED":"", i, i!=1?"s":"");
	}
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Remove messages from server</B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=remove style='width:217px'>\n");
	prints(sid, "<OPTION VALUE='0'%s>never\n", mailacct.remove==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='1'%s>when retrieved\n",  mailacct.remove==1?" SELECTED":"");
	prints(sid, "<OPTION VALUE='2'%s>when deleted from 'Trash'\n", mailacct.remove==2?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Show Debug    </B>&nbsp;</TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=showdebug style='width:217px'>\n");
	prints(sid, "<OPTION VALUE='0'%s>No\n", mailacct.showdebug==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='1'%s>Yes\n", mailacct.showdebug==1?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Signature&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=VIRTUAL NAME=signature ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", str2html(sid, mailacct.signature));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	if (mailacct.mailaccountid!=0) {
		prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD>");
		wmfolder_list(sid, mailacct.mailaccountid);
		prints(sid, "</TD></TR></TABLE>\n");
		prints(sid, "</DIV>\r\n");
		prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD>");
		wmfilter_list(sid, mailacct.mailaccountid);
		prints(sid, "</TD></TR></TABLE>\n");
		prints(sid, "</DIV>\r\n");
	}
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='%s'>\n", FORM_SAVE);
	if (accountid>0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='%s' onClick=\"return ConfirmDelete();\">\n", FORM_DELETE);
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='%s'>\n", FORM_RESET);
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "showpage(1);\n");
	prints(sid, "document.profilemailedit.accountname.focus();\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}

void wmaccount_list(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "profile")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
		return;
	}
	prints(sid, "<BR><CENTER>\n");
	if (auth_priv(sid, "webmail")>0) {
		if ((sqr=sql_queryf("SELECT mailaccountid, accountname, address FROM gw_mailaccounts WHERE obj_uid = %d ORDER BY accountname ASC", sid->dat->user_uid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300 STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;Mail Accounts</TH></TR>\n");
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/accounts/edit?account=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "<A HREF=%s/mail/accounts/edit?account=%d>%s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), str2html(sid, sql_getvalue(sqr, i, 1)));
				prints(sid, "<TD STYLE='border-style:solid'><A HREF=%s/mail/purge?accountid=%d>purge</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				prints(sid, "</TR>\n");
			}
			prints(sid, "</TABLE>\n");
		}
		if (sql_numtuples(sqr)<10) {
			prints(sid, "[<A HREF=%s/mail/accounts/editnew>Add Mail Account</A>]\n", sid->dat->in_ScriptName);
		}
		sql_freeresult(sqr);
	}
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void wmaccount_save(CONN *sid)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	struct stat sb;
	DIR  *handle;
	char dirname[512];
	char tmpname[512];
	REC_MAILACCT mailacct;
	char query[2048];
	char curdate[40];
	char *ptemp;
	int accountid;
	int sqr;

	if (!(auth_priv(sid, "webmail")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "MAILACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if (dbread_mailaccount(sid, 2, accountid, &mailacct)!=0) {
		prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
		return;
	}
	if ((ptemp=getpostenv(sid, "ACCOUNTNAME"))!=NULL) snprintf(mailacct.accountname, sizeof(mailacct.accountname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REALNAME"))!=NULL) snprintf(mailacct.realname, sizeof(mailacct.realname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ORGANIZATION"))!=NULL) snprintf(mailacct.organization, sizeof(mailacct.organization)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ADDRESS"))!=NULL) snprintf(mailacct.address, sizeof(mailacct.address)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REPLYTO"))!=NULL) snprintf(mailacct.replyto, sizeof(mailacct.replyto)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOSTTYPE"))!=NULL) snprintf(mailacct.hosttype, sizeof(mailacct.hosttype)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPHOST"))!=NULL) snprintf(mailacct.pophost, sizeof(mailacct.pophost)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPPORT"))!=NULL) mailacct.popport=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SMTPHOST"))!=NULL) snprintf(mailacct.smtphost, sizeof(mailacct.smtphost)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SMTPPORT"))!=NULL) mailacct.smtpport=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "POPUSERNAME"))!=NULL) snprintf(mailacct.popusername, sizeof(mailacct.popusername)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPPASSWORD"))!=NULL) snprintf(mailacct.poppassword, sizeof(mailacct.poppassword)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SMTPAUTH"))!=NULL) snprintf(mailacct.smtpauth, sizeof(mailacct.smtpauth)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "NOTIFY"))!=NULL) mailacct.notify=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "REMOVE"))!=NULL) mailacct.remove=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SHOWDEBUG"))!=NULL) mailacct.showdebug=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SIGNATURE"))!=NULL) snprintf(mailacct.signature, sizeof(mailacct.signature)-1, "%s", ptemp);
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "webmail")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
			return;
		}
		memset(dirname, 0, sizeof(dirname));
		snprintf(dirname, sizeof(dirname)-1, "%s/%04d/mail/%04d", config->server_dir_var_domains, sid->dat->user_did, mailacct.mailaccountid);
		if (stat(dirname, &sb)!=0) return;
		/* Purge the maildir with extreme prejudice - This should be recursive */
		handle=opendir(dirname);
		while ((dentry=readdir(handle))!=NULL) {
			if (strcmp(".", dentry->d_name)==0) continue;
			if (strcmp("..", dentry->d_name)==0) continue;
			memset(tmpname, 0, sizeof(tmpname));
			snprintf(tmpname, sizeof(tmpname)-1, "%s/%s", dirname, dentry->d_name);
			fixslashes(tmpname);
			if (stat(tmpname, &sb)!=0) continue;
			if (sb.st_mode&S_IFDIR) continue;
			unlink(tmpname);
		}
		closedir(handle);
		if (sql_updatef("DELETE FROM gw_mailheaders WHERE accountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid)<0) return;
		if (sql_updatef("DELETE FROM gw_mailfolders WHERE accountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid)<0) return;
		if (sql_updatef("DELETE FROM gw_mailaccounts WHERE mailaccountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid)<0) return;
		rmdir(dirname);
		prints(sid, "<BR><CENTER>E-Mail account %d deleted successfully</CENTER><BR>\n", mailacct.mailaccountid);
		db_log_activity(sid, 1, "mailaccounts", mailacct.mailaccountid, "delete", "%s - %s deleted mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	} else if (mailacct.mailaccountid==0) {
		if ((sqr=sql_query("SELECT max(mailaccountid) FROM gw_mailaccounts"))<0) return;
		mailacct.mailaccountid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (mailacct.mailaccountid<1) mailacct.mailaccountid=1;
		strcpy(query, "INSERT INTO gw_mailaccounts (mailaccountid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountname, realname, organization, address, replyto, hosttype, pophost, popport, smtphost, smtpport, popusername, poppassword, smtpauth, lastcount, notify, remove, showdebug, signature) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', ", mailacct.mailaccountid, curdate, curdate, sid->dat->user_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.accountname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.realname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.organization));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.replyto));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.hosttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.pophost));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", mailacct.popport);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.smtphost));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", mailacct.smtpport);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.popusername));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, EncodeBase64string(sid, mailacct.poppassword)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.smtpauth));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", mailacct.lastcount);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", mailacct.notify);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", mailacct.remove);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%c', ", mailacct.showdebug==0?'n':'y');
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.signature));
		if (sql_update(query)<0) return;
		wmfolder_makedefaults(sid, mailacct.mailaccountid);
		prints(sid, "<BR><CENTER>E-Mail account %d added successfully</CENTER><BR>\n", mailacct.mailaccountid);
		db_log_activity(sid, 1, "mailaccounts", mailacct.mailaccountid, "insert", "%s - %s added mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	} else {
		if (!(auth_priv(sid, "webmail")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>" ERR_NOACCESS "</CENTER><BR>\n");
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_mailaccounts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, mailacct.obj_uid);
		strncatf(query, sizeof(query)-strlen(query)-1, "accountname = '%s', ",  str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.accountname));
		strncatf(query, sizeof(query)-strlen(query)-1, "realname = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.realname));
		strncatf(query, sizeof(query)-strlen(query)-1, "organization = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.organization));
		strncatf(query, sizeof(query)-strlen(query)-1, "address = '%s', ",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "replyto = '%s', ",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.replyto));
		strncatf(query, sizeof(query)-strlen(query)-1, "hosttype = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.hosttype));
		strncatf(query, sizeof(query)-strlen(query)-1, "pophost = '%s', ",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.pophost));
		strncatf(query, sizeof(query)-strlen(query)-1, "popport = '%d', ",      mailacct.popport);
		strncatf(query, sizeof(query)-strlen(query)-1, "smtphost = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.smtphost));
		strncatf(query, sizeof(query)-strlen(query)-1, "smtpport = '%d', ",     mailacct.smtpport);
		strncatf(query, sizeof(query)-strlen(query)-1, "popusername = '%s', ",  str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.popusername));
		strncatf(query, sizeof(query)-strlen(query)-1, "poppassword = '%s', ",  str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, EncodeBase64string(sid, mailacct.poppassword)));
		strncatf(query, sizeof(query)-strlen(query)-1, "smtpauth = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.smtpauth));
		strncatf(query, sizeof(query)-strlen(query)-1, "lastcount = '%d', ",    mailacct.lastcount);
		strncatf(query, sizeof(query)-strlen(query)-1, "notify = '%d', ",       mailacct.notify);
		strncatf(query, sizeof(query)-strlen(query)-1, "remove = '%d', ",       mailacct.remove);
		strncatf(query, sizeof(query)-strlen(query)-1, "showdebug = '%c', ",    mailacct.showdebug==0?'n':'y');
		strncatf(query, sizeof(query)-strlen(query)-1, "signature = '%s'",      str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, mailacct.signature));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE mailaccountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid);
		if (sql_update(query)<0) return;
		prints(sid, "<BR><CENTER>E-mail account %d modified successfully</CENTER><BR>\n", mailacct.mailaccountid);
		db_log_activity(sid, 1, "mailaccounts", mailacct.mailaccountid, "modify", "%s - %s modified mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/mail/accounts/list\");\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/accounts/list\">\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
