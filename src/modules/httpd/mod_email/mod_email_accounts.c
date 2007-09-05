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

void wmaccount_edit(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *p;
	int id, pid;
	int priv=auth_priv(sid, "email");
	int i, n;

	if (!(priv&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/mail/accounts/editnew", 22)==0) {
		id=0;
		pid=sid->dat->uid;
		if (dbread_mailaccount(sid, 2, 0, &qobj)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
	} else {
		if ((p=getgetenv(sid, "ID"))==NULL) return;
		id=atoi(p);
		if (dbread_mailaccount(sid, 2, id, &qobj)!=0) {
			prints(sid, "<CENTER>%s</CENTER>\r\n", lang_gets(sid, "common", "err_norecord"));
			return;
		}
		pid=sid->dat->uid;
	}
	if (qobj==NULL) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n");
	prints(sid, "<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"%s\");\r\n", lang_gets(sid, "common", "warn_delconfirm"));
	prints(sid, "}\r\n");
	prints(sid, "function PortUpdatePOP() {\r\n");
	prints(sid, "	if (document.profilemailedit.hosttype.value=='POP3') {\r\n");
	prints(sid, "		if (document.profilemailedit.popssl.value=='0') {\r\n");
	prints(sid, "			document.profilemailedit.popport.value='%d';\r\n", POP3_PORT);
	prints(sid, "		} else if (document.profilemailedit.popssl.value=='1') {\r\n");
	prints(sid, "			document.profilemailedit.popport.value='%d';\r\n", POP3S_PORT);
	prints(sid, "		}\r\n");
	prints(sid, "	} else if (document.profilemailedit.hosttype.value=='IMAP') {\r\n");
	prints(sid, "		if (document.profilemailedit.popssl.value=='0') {\r\n");
	prints(sid, "			document.profilemailedit.popport.value='%d';\r\n", IMAP_PORT);
	prints(sid, "		} else if (document.profilemailedit.popssl.value=='1') {\r\n");
	prints(sid, "			document.profilemailedit.popport.value='%d';\r\n", IMAPS_PORT);
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	document.getElementById('hosttype1').innerHTML=document.profilemailedit.hosttype.value;\r\n");
	prints(sid, "	document.getElementById('hosttype2').innerHTML=document.profilemailedit.hosttype.value;\r\n");
	prints(sid, "	return;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function PortUpdateSMTP() {\r\n");
	prints(sid, "	if (document.profilemailedit.smtpssl.value=='0') {\r\n");
	prints(sid, "		document.profilemailedit.smtpport.value='%d';\r\n", SMTP_PORT);
	prints(sid, "	} else if (document.profilemailedit.smtpssl.value=='1') {\r\n");
	prints(sid, "		document.profilemailedit.smtpport.value='%d';\r\n", SMTPS_PORT);
	prints(sid, "	}\r\n");
	prints(sid, "	return;\r\n");
	prints(sid, "}\r\n");
	if (id!=0) {
		htscript_showpage(sid, 6);
	} else {
		htscript_showpage(sid, 4);
	}
	prints(sid, "// -->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<BR /><CENTER>\r\n");
	if (id!=0) {
		prints(sid, "<B>Mail Account %d - (%s:%d)</B>\r\n", id, sid->dat->username, id);
	} else {
		prints(sid, "<B>New Mail Account</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=430>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/accounts/save NAME=profilemailedit>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<INPUT TYPE=hidden NAME=id VALUE='%d'>\r\n", id);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_gen"));
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_ser"));
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_adv"));
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_sig"));
	if (id!=0) {
		prints(sid, "<TD ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=5 HREF=javascript:showpage(5)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_fld"));
		prints(sid, "<TD ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=6 HREF=javascript:showpage(6)>%s</A>&nbsp;</TD>\r\n", lang_gets(sid, "mod_email", "tab_fil"));
	}
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Account Name   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=accountname  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "cn")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Real Name      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=realname     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "realname")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Organization   </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "organization")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;E-Mail Address </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address      VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "address")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Reply Address  </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=replyto      VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "replyto")));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Host Type      </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	p=ldir_getval(&qobj, 0, "hosttype");
	prints(sid, "<SELECT NAME=hosttype style='width:217px' onchange=PortUpdatePOP();>\r\n");
	prints(sid, "<OPTION VALUE='POP3'%s>POP3\r\n", strcasecmp(p, "POP3")==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='IMAP'%s>IMAP\r\n", strcasecmp(p, "IMAP")==0?" SELECTED":"");
	prints(sid, "</SELECT>\r\n</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<SPAN ID=hosttype1>%s</SPAN> Host </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pophost VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", strcasecmp(p, "IMAP")==0?"IMAP":"POP3", str2html(sid, ldir_getval(&qobj, 0, "pophost")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Host      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtphost     VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "smtphost")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Username       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popusername  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "popusername")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password       </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=poppassword  VALUE=\"%s\" SIZE=30 style='width:217px'></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "poppassword")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Auth      </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=smtpauth style='width:217px'>\r\n");
	p=ldir_getval(&qobj, 0, "smtpauth");
	prints(sid, "<OPTION VALUE='n'%s>No\r\n", p[0]=='n'?" SELECTED":"");
	prints(sid, "<OPTION VALUE='y'%s>Yes\r\n", p[0]!='n'?" SELECTED":"");
	prints(sid, "</SELECT>\r\n</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;<SPAN ID=hosttype2>%s</SPAN> Port </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=popport      VALUE=\"%d\" SIZE=30 style='width:217px'></TD></TR>\r\n", strcasecmp(ldir_getval(&qobj, 0, "hosttype"), "IMAP")==0?"IMAP":"POP3", atoi(ldir_getval(&qobj, 0, "popport")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Use SSL        </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	n=atoi(ldir_getval(&qobj, 0, "popssl"));
	prints(sid, "<SELECT NAME=popssl style='width:217px'onchange=PortUpdatePOP();>\r\n<OPTION VALUE='0'%s>No\r\n<OPTION VALUE='1'%s>Yes\r\n</SELECT>\r\n", n==0?" SELECTED":"", n==1?" SELECTED":"");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;SMTP Port      </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=smtpport     VALUE=\"%d\" SIZE=30 style='width:217px'></TD></TR>\r\n", atoi(ldir_getval(&qobj, 0, "smtpport")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Use SSL        </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	n=atoi(ldir_getval(&qobj, 0, "smtpssl"));
	prints(sid, "<SELECT NAME=smtpssl style='width:217px' onchange=PortUpdateSMTP();>\r\n<OPTION VALUE='0'%s>No\r\n<OPTION VALUE='1'%s>Yes\r\n</SELECT>\r\n", n==0?" SELECTED":"", n==1?" SELECTED":"");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Check for new mail</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=notify style='width:217px'>\r\n");
	n=atoi(ldir_getval(&qobj, 0, "notify"));
	prints(sid, "<OPTION VALUE='0'%s>never\r\n", n==0?" SELECTED":"");
	for (i=1;i<61;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>every %d minute%s\r\n", i, i==n?" SELECTED":"", i, i!=1?"s":"");
	}
	prints(sid, "</SELECT>\r\n</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Remove messages from server</B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	n=atoi(ldir_getval(&qobj, 0, "remove"));
	prints(sid, "<SELECT NAME=remove style='width:217px'>\r\n");
	prints(sid, "<OPTION VALUE='0'%s>never\r\n", n==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='1'%s>when retrieved\r\n",  n==1?" SELECTED":"");
	prints(sid, "<OPTION VALUE='2'%s>when deleted from 'Trash'\r\n", n==2?" SELECTED":"");
	prints(sid, "</SELECT>\r\n</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Show Debug    </B>&nbsp;</TD><TD ALIGN=RIGHT>\r\n");
	n=atoi(ldir_getval(&qobj, 0, "showdebug"));
	prints(sid, "<SELECT NAME=showdebug style='width:217px'>\r\n");
	prints(sid, "<OPTION VALUE='0'%s>No\r\n", n==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='1'%s>Yes\r\n", n==1?" SELECTED":"");
	prints(sid, "</SELECT>\r\n</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;Signature&nbsp;</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=VIRTUAL NAME=signature ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "signature")));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	if (id!=0) {
		prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
		prints(sid, "<TR><TD>");
		wmfolder_list(sid, id);
		prints(sid, "</TD></TR></TABLE>\r\n");
		prints(sid, "</DIV>\r\n");
		prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
		prints(sid, "<TR><TD>");
		wmfilter_list(sid, id);
		prints(sid, "</TD></TR></TABLE>\r\n");
		prints(sid, "</DIV>\r\n");
	}
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='%s'>\r\n", lang_gets(sid, "common", "form_save"));
	if (id>0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='%s' onClick=\"return ConfirmDelete();\">\r\n", lang_gets(sid, "common", "form_delete"));
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='%s'>\r\n", lang_gets(sid, "common", "form_reset"));
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "showpage(1);\r\n");
	prints(sid, "document.profilemailedit.accountname.focus();\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	if (qobj) ldir_freeresult(&qobj);
	return;
}

void wmaccount_list(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	int id;
	int i;

	if (!(auth_priv(sid, "email")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	prints(sid, "<BR /><CENTER>\r\n");
	if ((qobj=ldir_getlist(sid->N, "emailaccount", sid->dat->uid, sid->dat->did))==NULL) return;
	if (ldir_numentries(&qobj)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=300 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH COLSPAN=2 NOWRAP STYLE='border-style:solid'>&nbsp;Mail Accounts</TH></TR>\r\n");
		for (i=0;i<ldir_numentries(&qobj);i++) {
			id=atoi(ldir_getval(&qobj, i, "id"));
			prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP WIDTH=100%% style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/mail/accounts/edit?id=%d'\">", ScriptName, id);
			prints(sid, "<A HREF=%s/mail/accounts/edit?id=%d>%s</A>&nbsp;</TD>", ScriptName, id, str2html(sid, ldir_getval(&qobj, i, "cn")));
			prints(sid, "<TD STYLE='border-style:solid'><A HREF=%s/mail/purge?accountid=%d onClick=\"location.replace('%s/mail/purge?accountid=%d');return false;\">purge</A></TD>", ScriptName, id, ScriptName, id);
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
	}
	if (ldir_numentries(&qobj)<10) {
		prints(sid, "[<A HREF=%s/mail/accounts/editnew>Add Mail Account</A>]\r\n", ScriptName);
	}
	ldir_freeresult(&qobj);
	prints(sid, "</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	return;
}

void wmaccount_save(CONN *sid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
#ifdef WIN32
//	struct	direct *dentry;
#else
//	struct	dirent *dentry;
#endif
	struct stat sb;
//	DIR  *handle;
	char dirname[512];
//	char tmpname[512];
	char *ptemp;
	int id;
	int priv=auth_priv(sid, "email");

	if (!(priv&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_mailaccount(sid, 2, id, &qobj)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
//	nes_setnum(sid->N, tobj, "pid", sid->dat->uid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	if ((ptemp=getpostenv(sid, "ACCOUNTNAME"))!=NULL)  nes_setstr(sid->N, tobj, "cn", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "REALNAME"))!=NULL)     nes_setstr(sid->N, tobj, "realname", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "ORGANIZATION"))!=NULL) nes_setstr(sid->N, tobj, "organization", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "ADDRESS"))!=NULL)      nes_setstr(sid->N, tobj, "address", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "REPLYTO"))!=NULL)      nes_setstr(sid->N, tobj, "replyto", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOSTTYPE"))!=NULL)     nes_setstr(sid->N, tobj, "hosttype", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "POPHOST"))!=NULL)      nes_setstr(sid->N, tobj, "pophost", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "POPPORT"))!=NULL)      nes_setnum(sid->N, tobj, "popport", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "POPSSL"))!=NULL)       nes_setnum(sid->N, tobj, "popssl", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SMTPHOST"))!=NULL)     nes_setstr(sid->N, tobj, "smtphost", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "SMTPPORT"))!=NULL)     nes_setnum(sid->N, tobj, "smtpport", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SMTPSSL"))!=NULL)      nes_setnum(sid->N, tobj, "smtpssl", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "POPUSERNAME"))!=NULL)  nes_setstr(sid->N, tobj, "popusername", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "POPPASSWORD"))!=NULL)  nes_setstr(sid->N, tobj, "poppassword", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "SMTPAUTH"))!=NULL)     nes_setstr(sid->N, tobj, "smtpauth", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "NOTIFY"))!=NULL)       nes_setnum(sid->N, tobj, "notify", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "REMOVE"))!=NULL)       nes_setnum(sid->N, tobj, "remove", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SHOWDEBUG"))!=NULL)    nes_setnum(sid->N, tobj, "showdebug", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SIGNATURE"))!=NULL)    nes_setstr(sid->N, tobj, "signature", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		memset(dirname, 0, sizeof(dirname));
		snprintf(dirname, sizeof(dirname)-1, "%s/domains/%04d/mail/%04d", nes_getstr(proc->N, confobj, "var_path"), sid->dat->did, id);
		if (stat(dirname, &sb)!=0) return;
		/* Purge the maildir with extreme prejudice - This should be recursive */
/*
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
*/
		if (sql_updatef(proc->N, "DELETE FROM gw_email_headers WHERE accountid = %d AND obj_uid = %d AND obj_did = %d", id, sid->dat->uid, sid->dat->did)<0) return;
		if (sql_updatef(proc->N, "DELETE FROM gw_email_folders WHERE accountid = %d AND obj_uid = %d AND obj_did = %d", id, sid->dat->uid, sid->dat->did)<0) return;
		if (sql_updatef(proc->N, "DELETE FROM gw_email_accounts WHERE mailaccountid = %d AND obj_uid = %d AND obj_did = %d", id, sid->dat->uid, sid->dat->did)<0) return;
		rmdir(dirname);
		prints(sid, "<BR /><CENTER>E-Mail account %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "mailaccounts", id, "delete", "%s - %s deleted mail account %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else if (id==0) {
		if (!(priv&A_INSERT)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_mailaccount(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		wmfolder_makedefaults(sid, id);
		prints(sid, "<BR /><CENTER>E-Mail account %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "emailaccount", id, "insert", "%s - %s added mail account %d", sid->socket.RemoteAddr, sid->dat->username, id);
	} else {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if ((id=dbwrite_mailaccount(sid, id, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<BR /><CENTER>E-mail account %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "mailaccounts", id, "modify", "%s - %s modified mail account %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/mail/accounts/list\");\r\n// -->\r\n</SCRIPT>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/list\"></NOSCRIPT>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}
