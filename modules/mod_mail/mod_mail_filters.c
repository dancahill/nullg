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

#ifdef WIN32
int winsystem(const char *format, ...)
{
	DWORD exitcode=0;
	HANDLE hMyProcess=GetCurrentProcess();
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char Command[512];
	va_list ap;
	int pid;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	memset(Command, 0, sizeof(Command));
	va_start(ap, format);
	vsnprintf(Command, sizeof(Command)-1, format, ap);
	va_end(ap);
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdInput=NULL;
	si.hStdOutput=NULL;
	si.hStdError=NULL;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		return -1;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
stuff:
	GetExitCodeProcess(pi.hProcess, &exitcode);
	if (exitcode==STILL_ACTIVE) goto stuff;
//      if (exitcode==STILL_ACTIVE) TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return exitcode;
}
#endif

int wmfilter_apply(CONN *sid, wmheader *header, int accountid, int messageid)
{
	char cmdline[512];
	char msgfilename[512];
	char *hptr;
	int dstmbox;
	int i;
	int match;
	int sqr1;
	short int err=0;
	struct stat sb;

	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/%06d.msg", config->server_dir_var_mail, sid->dat->user_mailcurrent, messageid);
	fixslashes(msgfilename);
	if (strlen(config->util_scanmail)>0) {
		memset(cmdline, 0, sizeof(cmdline));
		snprintf(cmdline, sizeof(cmdline)-1, "%s %s", config->util_scanmail, msgfilename);
//		logerror(sid, __FILE__, __LINE__, 0, "[%s]", cmdline);
#ifdef WIN32
		err=winsystem(cmdline);
#else
		err=system(cmdline);
#endif
		if (err>255) err=err>>8;
		if ((stat(msgfilename, &sb)==0)&&(sb.st_size>0)) {
			sql_updatef("UPDATE gw_mailheaders SET size = %d WHERE mailheaderid = %d AND accountid = %d", (int)sb.st_size, messageid, accountid);
		}
	}
//	prints(sid, "[%d]", err);
	if ((err==1)||(err==2)||(err==3)) {
		if (err==1) prints(sid, "[SPAM]");
		if (err==2) prints(sid, "[VIRUS]");
		if (err==3) prints(sid, "[VIRUS+SPAM]");
		sql_updatef("UPDATE gw_mailheaders SET folder = 7, status = 'o' WHERE mailheaderid = %d AND accountid = %d", messageid, accountid);
		return 7;
	}
	dstmbox=1;
	if ((sqr1=sql_queryf("SELECT header, string, rule, action, dstfolderid FROM gw_mailfilters WHERE obj_uid = %d AND accountid = %d ORDER BY mailfilterid ASC", sid->dat->user_uid, accountid))<0) return -1;
	if (sql_numtuples(sqr1)<1) {
		sql_freeresult(sqr1);
		return 1;
	}
	for (i=0;i<sql_numtuples(sqr1);i++) {
		if (strlen(sql_getvalue(sqr1, i, 0))<1) continue;
		if (strlen(sql_getvalue(sqr1, i, 1))<1) continue;
		hptr=NULL;
		if (strcasecmp(sql_getvalue(sqr1, i, 0), "from")==0) {
			hptr=header->From;
		} else if (strcasecmp(sql_getvalue(sqr1, i, 0), "reply-to")==0) {
			hptr=header->ReplyTo;
		} else if (strcasecmp(sql_getvalue(sqr1, i, 0), "to")==0) {
			hptr=header->To;
		} else if (strcasecmp(sql_getvalue(sqr1, i, 0), "cc")==0) {
			hptr=header->CC;
		} else if (strcasecmp(sql_getvalue(sqr1, i, 0), "bcc")==0) {
			hptr=header->BCC;
		} else if (strcasecmp(sql_getvalue(sqr1, i, 0), "subject")==0) {
			hptr=header->Subject;
		}
		if (hptr==NULL) continue;
		match=0;
		if (strcasecmp(sql_getvalue(sqr1, i, 2), "exact")==0) {
			if (strcasecmp(hptr, sql_getvalue(sqr1, i, 1))==0) {
				match=1;
			}
		} else if (strcasecmp(sql_getvalue(sqr1, i, 2), "substr")==0) {
			if (p_strcasestr(hptr, sql_getvalue(sqr1, i, 1))!=NULL) {
				match=1;
			}
		}
		if (match) {
			if (strcasecmp(sql_getvalue(sqr1, i, 3), "delete")==0) {
				sql_freeresult(sqr1);
				return -2;
			} else if (strcasecmp(sql_getvalue(sqr1, i, 3), "move")==0) {
				dstmbox=atoi(sql_getvalue(sqr1, i, 4));
				goto testrule;
			}
		}
	}
testrule:
	sql_freeresult(sqr1);
	if ((sqr1=sql_queryf("SELECT mailfolderid FROM gw_mailfolders WHERE obj_uid = %d AND accountid = %d AND mailfolderid = %d", sid->dat->user_uid, accountid, dstmbox))<0) return -1;
	if (sql_numtuples(sqr1)<1) {
		dstmbox=-1;
	}
	sql_freeresult(sqr1);
	sql_updatef("UPDATE gw_mailheaders SET folder = %d WHERE mailheaderid = %d AND accountid = %d", dstmbox, messageid, accountid);
	return dstmbox;
}

void wmfilter_edit(CONN *sid)
{
	char *ptemp;
	char filtername[51];
	char header[51];
	char string[51];
	char rule[51];
	char action[51];
	int accountid;
	int dstfolderid;
	int filterid;
	int hdrfound;
	int sqr;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(filtername, 0, sizeof(filtername));
	memset(header, 0, sizeof(header));
	memset(string, 0, sizeof(string));
	memset(rule,   0, sizeof(rule));
	memset(action, 0, sizeof(action));
	dstfolderid=0;
	if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if (strncmp(sid->dat->in_RequestURI, "/mail/filters/editnew", 21)==0) {
		filterid=0;
		snprintf(filtername, sizeof(filtername)-1, "New Filter");
		snprintf(header,     sizeof(filtername)-1, "subject");
//		snprintf(string,     sizeof(filtername)-1, "");
		snprintf(rule,       sizeof(filtername)-1, "exact");
		snprintf(action,     sizeof(filtername)-1, "move");
		dstfolderid=1;
	} else {
		if ((ptemp=getgetenv(sid, "FILTERID"))==NULL) return;
		filterid=atoi(ptemp);
		if ((sqr=sql_queryf("SELECT mailfilterid, accountid, filtername, header, string, rule, action, dstfolderid FROM gw_mailfilters WHERE obj_uid = %d AND accountid = %d AND mailfilterid = %d ORDER BY mailfilterid ASC", sid->dat->user_uid, accountid, filterid))<0) return;
		if (sql_numtuples(sqr)==1) {
			filterid=atoi(sql_getvalue(sqr, 0, 0));
			accountid=atoi(sql_getvalue(sqr, 0, 1));
			snprintf(filtername, sizeof(filtername)-1, "%s", sql_getvalue(sqr, 0, 2));
			snprintf(header, sizeof(header)-1, "%s", sql_getvalue(sqr, 0, 3));
			snprintf(string, sizeof(string)-1, "%s", sql_getvalue(sqr, 0, 4));
			snprintf(rule,   sizeof(rule)-1,   "%s", sql_getvalue(sqr, 0, 5));
			snprintf(action, sizeof(action)-1, "%s", sql_getvalue(sqr, 0, 6));
			dstfolderid=atoi(sql_getvalue(sqr, 0, 7));
		}
		sql_freeresult(sqr);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<BR><CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/mail/filters/save NAME=filteredit>\n", sid->dat->in_ScriptName);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n");
	prints(sid, "<INPUT TYPE=hidden NAME=accountid VALUE='%d'>\n", accountid);
	prints(sid, "<INPUT TYPE=hidden NAME=filterid VALUE='%d'>\n", filterid);
	prints(sid, "<TR BGCOLOR=\"%s\"><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (filterid!=0) {
		prints(sid, "Mail Filter '%s'</FONT></TH></TR>\n", filtername);
	} else {
		prints(sid, "New Mail Filter</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=\"%s\"><TD VALIGN=TOP COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Filter Name   &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=filtername value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", config->colour_editform, str2html(sid, filtername));
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Filter Header &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=header style='width:217px'>\n", config->colour_editform);
	hdrfound=0;
	if (strcasecmp(header, "from")==0) hdrfound=1;
	else if (strcasecmp(header, "reply-to")==0) hdrfound=2;
	else if (strcasecmp(header, "to")==0) hdrfound=3;
	else if (strcasecmp(header, "cc")==0) hdrfound=4;
//	else if (strcasecmp(header, "bcc")==0) hdrfound=5;
	else if (strcasecmp(header, "subject")==0) hdrfound=6;
	if (hdrfound==0) {
		prints(sid, "<OPTION VALUE='%s'%s>%s:\n", header, hdrfound==0?" SELECTED":"", header);
	}
	prints(sid, "<OPTION VALUE='from'%s>From:\n",         hdrfound==1?" SELECTED":"");
	prints(sid, "<OPTION VALUE='reply-to'%s>Reply-To:\n", hdrfound==2?" SELECTED":"");
	prints(sid, "<OPTION VALUE='to'%s>To:\n",             hdrfound==3?" SELECTED":"");
	prints(sid, "<OPTION VALUE='cc'%s>CC:\n",             hdrfound==4?" SELECTED":"");
//	prints(sid, "<OPTION VALUE='bcc'%s>BCC:\n",           hdrfound==5?" SELECTED":"");
	prints(sid, "<OPTION VALUE='subject'%s>Subject:\n",   hdrfound==6?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Filter String &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><INPUT TYPE=TEXT NAME=string value=\"%s\" SIZE=30 style='width:217px'></TD></TR>\n", config->colour_editform, str2html(sid, string));
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Filter Rule &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=rule style='width:217px'>\n", config->colour_editform);
	prints(sid, "<OPTION VALUE='exact'%s>Exact match\n", strcasecmp(rule, "exact")==0?" SELECTED":"");
	prints(sid, "<OPTION VALUE='substr'%s>Substring match\n", strcasecmp(rule, "substr")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Filter Action &nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=action style='width:217px'>\n", config->colour_editform);
	prints(sid, "<OPTION VALUE='move'%s>Move to folder\n", strcasecmp(action, "move")==0?" SELECTED":"");
//	prints(sid, "<OPTION VALUE='delete'%s>Delete from server\n", strcasecmp(action, "delete")==0?" SELECTED":"");
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=\"%s\"><TD NOWRAP><B>&nbsp;Move to Folder&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=dstfolderid style='width:217px'>\n", config->colour_editform);
	htselect_mailfolder(sid, dstfolderid, 1, 0);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if (filterid>0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.filteredit.filtername.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void wmfilter_list(CONN *sid, int accountid)
{
	int i;
	int sqr1;

	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr1=sql_queryf("SELECT mailfilterid, filtername FROM gw_mailfilters WHERE obj_uid = %d and accountid = %d ORDER BY mailfilterid ASC", sid->dat->user_uid, accountid))<0) return;
	prints(sid, "<CENTER>");
	if (sql_numtuples(sqr1)>0) {
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>");
		for (i=0; i<sql_numtuples(sqr1); i++) {
			prints(sid, "<TR BGCOLOR=#F0F0F0><TD NOWRAP WIDTH=100%%>");
			prints(sid, "<A HREF=%s/mail/filters/edit?accountid=%d&filterid=%d>", sid->dat->in_ScriptName, accountid, atoi(sql_getvalue(sqr1, i, 0)));
			prints(sid, "<B>%s</B></A></TD></TR>\n", str2html(sid, sql_getvalue(sqr1, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/mail/filters/editnew?accountid=%d><B>New Filter</B></A>", sid->dat->in_ScriptName, accountid);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr1);
	return;
}

void wmfilter_save(CONN *sid)
{
	char *ptemp;
	char query[2048];
	char curdate[40];
	char filtername[51];
	char header[51];
	char string[51];
	char rule[51];
	char action[51];
	int accountid;
	int dstfolderid;
	int filterid;
	int sqr;
	time_t t;

	prints(sid, "<BR>\n");
	if (!(auth_priv(sid, "webmail")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	memset(filtername, 0, sizeof(filtername));
	memset(header, 0, sizeof(header));
	memset(string, 0, sizeof(string));
	memset(rule,   0, sizeof(rule));
	memset(action, 0, sizeof(action));
	dstfolderid=0;
	if (strcmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "FILTERID"))==NULL) return;
	filterid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "FILTERNAME"))!=NULL) snprintf(filtername, sizeof(filtername)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HEADER"))!=NULL)     snprintf(header,     sizeof(header)-1,     "%s", ptemp);
	if ((ptemp=getpostenv(sid, "STRING"))!=NULL)     snprintf(string,     sizeof(string)-1,     "%s", ptemp);
	if ((ptemp=getpostenv(sid, "RULE"))!=NULL)       snprintf(rule,       sizeof(rule)-1,       "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ACTION"))!=NULL)     snprintf(action,     sizeof(action)-1,     "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DSTFOLDERID"))!=NULL) dstfolderid=atoi(ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_mailfilters WHERE accountid = %d AND mailfilterid = %d", accountid, filterid)<0) return;
		prints(sid, "<CENTER>Mail Filter %d deleted successfully</CENTER><BR>\n", filterid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	} else if (filterid==0) {
		if (strlen(filtername)<1) {
			prints(sid, "<CENTER>Filter name cannot be blank</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_queryf("SELECT max(mailfilterid) FROM gw_mailfilters where accountid = %d", accountid))<0) return;
		filterid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (filterid<1) filterid=1;
		strcpy(query, "INSERT INTO gw_mailfilters (mailfilterid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, accountid, filtername, header, string, rule, action, dstfolderid) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '0', '0', '0', '%d', ", filterid, curdate, curdate, sid->dat->user_uid, accountid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filtername));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, rule));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, action));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d')", dstfolderid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Mail filter %d added successfully</CENTER><BR>\n", filterid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_mailfilters SET obj_mtime = '%s', ", curdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "filtername = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filtername));
		strncatf(query, sizeof(query)-strlen(query)-1, "header = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header));
		strncatf(query, sizeof(query)-strlen(query)-1, "string = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string));
		strncatf(query, sizeof(query)-strlen(query)-1, "rule = '%s', ",       str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, rule));
		strncatf(query, sizeof(query)-strlen(query)-1, "action = '%s', ",     str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, action));
		strncatf(query, sizeof(query)-strlen(query)-1, "dstfolderid = '%d'", dstfolderid);
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE mailfilterid = %d AND obj_uid = %d AND accountid = %d", filterid, sid->dat->user_uid, accountid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Mail filter %d modified successfully</CENTER><BR>\n", filterid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/mail/accounts/edit?account=%d\">\n", sid->dat->in_ScriptName, accountid);
	}
	return;
}
