/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

void profileedit(CONNECTION *sid)
{
	REC_USER user;
	int i;
	int sqr;

	if (!(auth_priv(sid, AUTH_PROFILE)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (db_read(sid, 2, DB_PROFILE, sid->dat->user_uid, &user)!=0) {
		prints(sid, "<CENTER>Profile information not found</CENTER>\n");
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR><TD>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/save NAME=profileedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>User Profile for %s</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, sid->dat->user_username);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password             </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password  VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, user.password));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Start       </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefdaystart style='width:182px'>\n");
	htselect_hour(sid, user.prefdaystart);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Length      </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefdaylength style='width:182px'>\n");
	htselect_number(sid, user.prefdaylength, 0, 24);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Current Mail Account </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefmailcurrent style='width:182px'>\n");
	htselect_mailaccount(sid, user.prefmailcurrent);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Default Mail Account </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefmaildefault style='width:182px'>\n");
	prints(sid, "<OPTION VALUE=0%s>External Mail Client\n", user.prefmaildefault!=1?" SELECTED":"");
	prints(sid, "<OPTION VALUE=1%s>Web E-Mail\n", user.prefmaildefault==1?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Maximum Results/Page </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefmaxlist style='width:182px'>\n");
	htselect_number(sid, user.prefmaxlist, 5, 50);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Menu and Frame Style </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefmenustyle style='width:182px'>\n");
	htselect_layout(sid, user.prefmenustyle);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Time Zone            </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=preftimezone style='width:182px'>\n");
	htselect_timezone(sid, user.preftimezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=prefgeozone style='width:182px'>\n");
	htselect_zone(sid, user.prefgeozone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD></TR>");
	prints(sid, "<TR><TD ALIGN=CENTER>\n");
	if (auth_priv(sid, AUTH_WEBMAIL)&A_READ) {
		if ((sqr=sqlQueryf(sid, "SELECT mailaccountid, accountname, address FROM gw_mailaccounts WHERE obj_uid = %d ORDER BY mailaccountid ASC", sid->dat->user_uid))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
			prints(sid, "<TR BGCOLOR=%s><TH NOWRAP><FONT COLOR=%s>Mail Account Name</FONT></TH><TH NOWRAP><FONT COLOR=%s>E-Mail Address</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT);
			for (i=0;i<sqlNumtuples(sqr);i++) {
				prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
				prints(sid, "<TD NOWRAP><A HREF=%s/profile/mailedit?account=%d>%s</A>&nbsp;</TD>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), str2html(sid, sqlGetvalue(sqr, i, 1)));
				prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr, i, 2)));
				prints(sid, "</TR>\n");
			}
			prints(sid, "</TABLE>\n");
		}
		if (sqlNumtuples(sqr)<5) {
			prints(sid, "[<A HREF=%s/profile/maileditnew>Add Mail Account</A>]\n", sid->dat->in_ScriptName);
		}
		sqlFreeconnect(sqr);
	}
	prints(sid, "[<A HREF=%s/profile/timeedit>Edit Availability</A>]\n", sid->dat->in_ScriptName);
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.profileedit.password.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void profilesave(CONNECTION *sid)
{
	REC_USER user;
	char curdate[32];
	char opassword[50];
	char query[4096];
	char *ptemp;
	int userid;

	if (!(auth_priv(sid, AUTH_PROFILE)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	userid=sid->dat->user_uid;
	if (db_read(sid, 2, DB_PROFILE, userid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", user.password);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL) user.prefdaystart=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL) user.prefdaylength=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAILCURRENT"))!=NULL) user.prefmailcurrent=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAILDEFAULT"))!=NULL) user.prefmaildefault=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL) user.prefmaxlist=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMENUSTYLE"))!=NULL) user.prefmenustyle=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL) user.preftimezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL) user.prefgeozone=atoi(ptemp);
	memset(curdate, 0, sizeof(curdate));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (strcmp(opassword, user.password)!=0) {
		snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
	}
	snprintf(query, sizeof(query)-1, "UPDATE gw_users SET obj_mtime = '%s', ", curdate);
	strcatf(query, "password = '%s', ", user.password);
	strcatf(query, "prefdaystart = '%d', ", user.prefdaystart);
	strcatf(query, "prefdaylength = '%d', ", user.prefdaylength);
	strcatf(query, "prefmailcurrent = '%d', ", user.prefmailcurrent);
	strcatf(query, "prefmaildefault = '%d', ", user.prefmaildefault);
	strcatf(query, "prefmaxlist = '%d', ", user.prefmaxlist);
	strcatf(query, "prefmenustyle = '%d', ", user.prefmenustyle);
	strcatf(query, "preftimezone = '%d', ", user.preftimezone);
	strcatf(query, "prefgeozone = '%d'", user.prefgeozone);
	strcatf(query, " WHERE userid = %d", user.userid);
	if (sqlUpdate(sid, query)<0) return;
	prints(sid, "<CENTER>User %d modified successfully</CENTER><BR>\n", userid);
	logaccess(sid, 1, "%s - %s modified profile", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\n", sid->dat->in_ScriptName);
	return;
}

void profilemailedit(CONNECTION *sid)
{
	REC_MAILACCT mailacct;
	int accountid;

	if (!(auth_priv(sid, AUTH_WEBMAIL)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/profile/maileditnew", 20)==0) {
		accountid=0;
		if (db_read(sid, 2, DB_MAILACCOUNTS, 0, &mailacct)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	} else {
		if (getgetenv(sid, "ACCOUNT")==NULL) return;
		accountid=atoi(getgetenv(sid, "ACCOUNT"));
		if (db_read(sid, 2, DB_MAILACCOUNTS, accountid, &mailacct)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", accountid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "function PortUpdate() {\n");
	prints(sid, "	if (document.profilemailedit.hosttype.value=='POP3') {\n");
	prints(sid, "		document.profilemailedit.popport.value='110';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "	if (document.profilemailedit.hosttype.value=='IMAP') {\n");
	prints(sid, "		document.profilemailedit.popport.value='143';\n");
	prints(sid, "		return;\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/mailsave NAME=profilemailedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=mailaccountid VALUE='%d'>\n", mailacct.mailaccountid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (mailacct.mailaccountid!=0) {
		prints(sid, "Mail Account %d</FONT></TH></TR>\n", mailacct.mailaccountid);
	} else {
		prints(sid, "New Mail Account</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Account Name   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=accountname  VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.accountname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Real Name      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=realname     VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.realname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Organization   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.organization));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;E-Mail Address </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address      VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.address));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Host Type      </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=hosttype style='width:182px' onchange=PortUpdate();>\n");
	prints(sid, "<OPTION VALUE='POP3'%s>POP3\n", strcasecmp(mailacct.hosttype, "POP3")==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='IMAP'%s>IMAP\n", strcasecmp(mailacct.hosttype, "IMAP")==0?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;POP3/IMAP Host </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pophost      VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.pophost));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;POP3/IMAP Port </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popport      VALUE=\"%d\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, mailacct.popport);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;SMTP Host      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtphost     VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.smtphost));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;SMTP Port      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtpport     VALUE=\"%d\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, mailacct.smtpport);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Username       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popusername  VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.popusername));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=poppassword  VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.poppassword));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Notify         </B>&nbsp;</TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints(sid, "<SELECT NAME=notify style='width:182px'>\n");
	prints(sid, "<OPTION VALUE='0'%s>No\n",  mailacct.notify==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='1'%s>Yes\n", mailacct.notify==1?" SELECTED":"");
	prints(sid, "</SELECT>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>&nbsp;Signature&nbsp;</B></TD></TR>\n", COLOR_EDITFORM);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=signature ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, str2html(sid, mailacct.signature));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", COLOR_EDITFORM);
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.profilemailedit.accountname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void profilemailsave(CONNECTION *sid)
{
	REC_MAILACCT mailacct;
	char query[2048];
	char curdate[40];
	char *ptemp;
	int accountid;
	int sqr;

	if (!(auth_priv(sid, AUTH_WEBMAIL)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "MAILACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if (db_read(sid, 2, DB_MAILACCOUNTS, accountid, &mailacct)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "ACCOUNTNAME"))!=NULL) snprintf(mailacct.accountname, sizeof(mailacct.accountname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REALNAME"))!=NULL) snprintf(mailacct.realname, sizeof(mailacct.realname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ORGANIZATION"))!=NULL) snprintf(mailacct.organization, sizeof(mailacct.organization)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ADDRESS"))!=NULL) snprintf(mailacct.address, sizeof(mailacct.address)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOSTTYPE"))!=NULL) snprintf(mailacct.hosttype, sizeof(mailacct.hosttype)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPHOST"))!=NULL) snprintf(mailacct.pophost, sizeof(mailacct.pophost)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPPORT"))!=NULL) mailacct.popport=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SMTPHOST"))!=NULL) snprintf(mailacct.smtphost, sizeof(mailacct.smtphost)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SMTPPORT"))!=NULL) mailacct.smtpport=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "POPUSERNAME"))!=NULL) snprintf(mailacct.popusername, sizeof(mailacct.popusername)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POPPASSWORD"))!=NULL) snprintf(mailacct.poppassword, sizeof(mailacct.poppassword)-1, "%s", ptemp);
//	if ((ptemp=getpostenv(sid, "LASTCOUNT"))!=NULL) mailacct.lastcount=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "NOTIFY"))!=NULL) mailacct.notify=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SIGNATURE"))!=NULL) snprintf(mailacct.signature, sizeof(mailacct.signature)-1, "%s", ptemp);
	memset(curdate, 0, sizeof(curdate));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_WEBMAIL)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_mailaccounts WHERE mailaccountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid)<0) return;
		prints(sid, "<CENTER>E-Mail account %d deleted successfully</CENTER><BR>\n", mailacct.mailaccountid);
		logaccess(sid, 1, "%s - %s deleted e-mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	} else if (mailacct.mailaccountid==0) {
		if ((sqr=sqlQuery(sid, "SELECT max(mailaccountid) FROM gw_mailaccounts"))<0) return;
		mailacct.mailaccountid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		sqlFreeconnect(sqr);
		if (mailacct.mailaccountid<1) mailacct.mailaccountid=1;
		strcpy(query, "INSERT INTO gw_mailaccounts (mailaccountid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountname, realname, organization, address, hosttype, pophost, popport, smtphost, smtpport, popusername, poppassword, lastcount, notify, signature) values (");
		strcatf(query, "'%d', '%s', '%s', '%d', '0', '0', '0', ", mailacct.mailaccountid, curdate, curdate, sid->dat->user_uid);
		strcatf(query, "'%s', ", str2sql(sid, mailacct.accountname));
		strcatf(query, "'%s', ", str2sql(sid, mailacct.realname));
		strcatf(query, "'%s', ", str2sql(sid, mailacct.organization));
		strcatf(query, "'%s', ", str2sql(sid, mailacct.address));
		strcatf(query, "'%s', ", str2sql(sid, mailacct.hosttype));
		strcatf(query, "'%s', ", str2sql(sid, mailacct.pophost));
		strcatf(query, "'%d', ", mailacct.popport);
		strcatf(query, "'%s', ", str2sql(sid, mailacct.smtphost));
		strcatf(query, "'%d', ", mailacct.smtpport);
		strcatf(query, "'%s', ", str2sql(sid, mailacct.popusername));
		strcatf(query, "'%s', ", str2sql(sid, EncodeBase64string(sid, mailacct.poppassword)));
		strcatf(query, "'%d', ", mailacct.lastcount);
		strcatf(query, "'%d', ", mailacct.notify);
		strcatf(query, "'%s')", str2sql(sid, mailacct.signature));
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>E-Mail account %d added successfully</CENTER><BR>\n", mailacct.mailaccountid);
		logaccess(sid, 1, "%s - %s added e-mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	} else {
		if (!(auth_priv(sid, AUTH_WEBMAIL)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_mailaccounts SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '0', obj_gperm = '0', obj_operm = '0', ", curdate, mailacct.obj_uid);
		strcatf(query, "accountname = '%s', ", str2sql(sid, mailacct.accountname));
		strcatf(query, "realname = '%s', ", str2sql(sid, mailacct.realname));
		strcatf(query, "organization = '%s', ", str2sql(sid, mailacct.organization));
		strcatf(query, "address = '%s', ", str2sql(sid, mailacct.address));
		strcatf(query, "hosttype = '%s', ", str2sql(sid, mailacct.hosttype));
		strcatf(query, "pophost = '%s', ", str2sql(sid, mailacct.pophost));
		strcatf(query, "popport = '%d', ", mailacct.popport);
		strcatf(query, "smtphost = '%s', ", str2sql(sid, mailacct.smtphost));
		strcatf(query, "smtpport = '%d', ", mailacct.smtpport);
		strcatf(query, "popusername = '%s', ", str2sql(sid, mailacct.popusername));
		strcatf(query, "poppassword = '%s', ", str2sql(sid, EncodeBase64string(sid, mailacct.poppassword)));
		strcatf(query, "lastcount = '%d', ", mailacct.lastcount);
		strcatf(query, "notify = '%d', ", mailacct.notify);
		strcatf(query, "signature = '%s'", str2sql(sid, mailacct.signature));
		strcatf(query, " WHERE mailaccountid = %d and obj_uid = %d", mailacct.mailaccountid, sid->dat->user_uid);
		if (sqlUpdate(sid, query)<0) return;
		prints(sid, "<CENTER>E-mail account %d modified successfully</CENTER><BR>\n", mailacct.mailaccountid);
		logaccess(sid, 1, "%s - %s modified e-mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/profile/edit\");\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void profiletimeedit(CONNECTION *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	int i;
	int j;
	int sqr;

	if (!(auth_priv(sid, AUTH_PROFILE)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sqlQueryf(sid, "SELECT availability FROM gw_users WHERE userid = %d", sid->dat->user_uid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\n", sid->dat->user_uid);
		sqlFreeconnect(sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sqlGetvalue(sqr, 0, 0), sizeof(uavailability)-1);
	sqlFreeconnect(sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if ((sqr=sqlQueryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", sid->dat->user_gid))<0) return;
	if (sqlNumtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\n", sid->dat->user_gid);
		sqlFreeconnect(sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sqlGetvalue(sqr, 0, 0), sizeof(gavailability)-1);
	sqlFreeconnect(sqr);
	if (strlen(gavailability)!=168) {
		for (i=0;i<168;i++) {
			gavailability[i]='0';
		}
	}
	for (i=0;i<168;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='X';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/profile/timesave NAME=availability>\n", sid->dat->in_ScriptName);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=25><FONT COLOR=%s>Availability for %s</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, sid->dat->user_username);
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", COLOR_FNAME);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=#F0F0F0>\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", COLOR_FNAME, i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
			} else {
				prints(sid, "<TD><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void profiletimesave(CONNECTION *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	char *ptemp;
	char *pdest;
	int i;
	int j;

	if (!(auth_priv(sid, AUTH_PROFILE)&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	memset(curdate, 0, sizeof(curdate));
	snprintf(curdate, sizeof(curdate)-1, "%s", time_unix2sql(sid, time(NULL)));
	if (sqlUpdatef(sid, "UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, sid->dat->user_uid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	logaccess(sid, 1, "%s - %s modified availability", sid->dat->in_RemoteAddr, sid->dat->user_username);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/profile/edit\");\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/profile/edit\">\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void profilemain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_PROFILE);
	prints(sid, "<BR>\r\n");
	if (strncmp(sid->dat->in_RequestURI, "/profile/save", 13)==0) {
		profilesave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/edit", 13)==0) {
		profileedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/mailedit", 17)==0) {
		profilemailedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/mailsave", 17)==0) {
		profilemailsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/timeedit", 17)==0) {
		profiletimeedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/profile/timesave", 17)==0) {
		profiletimesave(sid);
	}
	htpage_footer(sid);
	return;
}
