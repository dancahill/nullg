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

int dbread_mailaccount(CONN *sid, short int perm, int index, REC_MAILACCT *mailacct)
{
	char *ptemp;
	int authlevel;
	int sqr;

	memset(mailacct, 0, sizeof(REC_MAILACCT));
	authlevel=auth_priv(sid, "webmail");
	if (authlevel<1) return -1;
	if (!(authlevel&A_MODIFY)&&(perm==2)) return -1;
	if (!(authlevel&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		mailacct->obj_uid=sid->dat->user_uid;
		mailacct->obj_gid=sid->dat->user_gid;
		mailacct->obj_did=sid->dat->user_did;
		mailacct->obj_gperm=1;
		mailacct->obj_operm=1;
		snprintf(mailacct->hosttype, sizeof(mailacct->hosttype)-1, "POP3");
		mailacct->popport=110;
		mailacct->smtpport=25;
		mailacct->smtpauth[0]='n';
		mailacct->lastcheck=time(NULL);
		mailacct->notify=10;
		mailacct->remove=2;
		return 0;
	}
	if (authlevel&A_ADMIN) {
		if ((sqr=sql_queryf("SELECT * FROM gw_mailaccounts where mailaccountid = %d", index))<0) return -1;
	} else {
		if ((sqr=sql_queryf("SELECT * FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", index, sid->dat->user_uid))<0) return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return -2;
	}
	mailacct->mailaccountid = atoi(sql_getvalue(sqr, 0, 0));
	mailacct->obj_ctime     = time_sql2unix(sql_getvalue(sqr, 0, 1));
	mailacct->obj_mtime     = time_sql2unix(sql_getvalue(sqr, 0, 2));
	mailacct->obj_uid       = atoi(sql_getvalue(sqr, 0, 3));
	mailacct->obj_gid       = atoi(sql_getvalue(sqr, 0, 4));
	mailacct->obj_did       = atoi(sql_getvalue(sqr, 0, 5));
	mailacct->obj_gperm     = atoi(sql_getvalue(sqr, 0, 6));
	mailacct->obj_operm     = atoi(sql_getvalue(sqr, 0, 7));
	strncpy(mailacct->accountname,	sql_getvalue(sqr, 0, 8), sizeof(mailacct->accountname)-1);
	strncpy(mailacct->realname,	sql_getvalue(sqr, 0, 9), sizeof(mailacct->realname)-1);
	strncpy(mailacct->organization,	sql_getvalue(sqr, 0, 10), sizeof(mailacct->organization)-1);
	strncpy(mailacct->address,	sql_getvalue(sqr, 0, 11), sizeof(mailacct->address)-1);
	strncpy(mailacct->replyto,	sql_getvalue(sqr, 0, 12), sizeof(mailacct->replyto)-1);
	strncpy(mailacct->hosttype,	sql_getvalue(sqr, 0, 13), sizeof(mailacct->hosttype)-1);
	strncpy(mailacct->pophost,	sql_getvalue(sqr, 0, 14), sizeof(mailacct->pophost)-1);
	mailacct->popport=atoi(sql_getvalue(sqr, 0, 15));
	strncpy(mailacct->smtphost,	sql_getvalue(sqr, 0, 16), sizeof(mailacct->smtphost)-1);
	mailacct->smtpport=atoi(sql_getvalue(sqr, 0, 17));
	strncpy(mailacct->popusername,	sql_getvalue(sqr, 0, 18), sizeof(mailacct->popusername)-1);
//	strncpy(mailacct->poppassword,	decode_b64s(sid, sql_getvalue(sqr, 0, 19)), sizeof(mailacct->poppassword)-1);
	strncpy(mailacct->poppassword,  DecodeBase64string(sid, sql_getvalue(sqr, 0, 19)), sizeof(mailacct->poppassword)-1);
	strncpy(mailacct->smtpauth,	sql_getvalue(sqr, 0, 20), sizeof(mailacct->smtpauth)-1);
	mailacct->lastcount=atoi(sql_getvalue(sqr, 0, 21));
	mailacct->notify=atoi(sql_getvalue(sqr, 0, 22));
	mailacct->remove=atoi(sql_getvalue(sqr, 0, 23));
	mailacct->lastcheck=time_sql2unix(sql_getvalue(sqr, 0, 24));
	ptemp=sql_getvalue(sqr, 0, 25);
	if (tolower(ptemp[0])=='y') {
		mailacct->showdebug=1;
	}
	strncpy(mailacct->signature,	sql_getvalue(sqr, 0, 26), sizeof(mailacct->signature)-1);
	sql_freeresult(sqr);
	return 0;
}

int dbread_mailcurrent(CONN *sid, int mailcurrent)
{
	char *ptemp;
	int sqr;

	if ((sqr=sql_queryf("SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, smtphost, smtpport, address, replyto, remove, showdebug, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", mailcurrent, sid->dat->user_uid))<0) return -1;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		if ((sqr=sql_queryf("SELECT mailaccountid FROM gw_mailaccounts where obj_uid = %d", sid->dat->user_uid))<0) return -1;
		if (sql_numtuples(sqr)<1) {
			sql_freeresult(sqr);
			return -1;
		}
		sid->dat->user_mailcurrent=atoi(sql_getvalue(sqr, 0, 0));
		if (sql_updatef("UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", sid->dat->user_mailcurrent, sid->dat->user_username)<0) return -1;
		sql_freeresult(sqr);
		if ((sqr=sql_queryf("SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, smtphost, smtpport, address, replyto, remove, showdebug, signature FROM gw_mailaccounts where mailaccountid = %d and obj_uid = %d", mailcurrent, sid->dat->user_uid))<0) return -1;
	}
	if (sql_numtuples(sqr)==1) {
		strncpy(sid->dat->wm->realname,     sql_getvalue(sqr, 0, 0), sizeof(sid->dat->wm->realname)-1);
		strncpy(sid->dat->wm->organization, sql_getvalue(sqr, 0, 1), sizeof(sid->dat->wm->organization)-1);
		strncpy(sid->dat->wm->username,     sql_getvalue(sqr, 0, 2), sizeof(sid->dat->wm->username)-1);
		strncpy(sid->dat->wm->password,     DecodeBase64string(sid, sql_getvalue(sqr, 0, 3)), sizeof(sid->dat->wm->password)-1);
		strncpy(sid->dat->wm->smtpauth,     sql_getvalue(sqr, 0, 4), sizeof(sid->dat->wm->smtpauth)-1);
		strncpy(sid->dat->wm->servertype,   sql_getvalue(sqr, 0, 5), sizeof(sid->dat->wm->servertype)-1);
		strncpy(sid->dat->wm->popserver,    sql_getvalue(sqr, 0, 6), sizeof(sid->dat->wm->popserver)-1);
		sid->dat->wm->popport=atoi(sql_getvalue(sqr, 0, 7));
		strncpy(sid->dat->wm->smtpserver,   sql_getvalue(sqr, 0, 8), sizeof(sid->dat->wm->smtpserver)-1);
		sid->dat->wm->smtpport=atoi(sql_getvalue(sqr, 0, 9));
		strncpy(sid->dat->wm->address,      sql_getvalue(sqr, 0, 10), sizeof(sid->dat->wm->address)-1);
		strncpy(sid->dat->wm->replyto,      sql_getvalue(sqr, 0, 11), sizeof(sid->dat->wm->replyto)-1);
		sid->dat->wm->remove=atoi(sql_getvalue(sqr, 0, 12));
		ptemp=sql_getvalue(sqr, 0, 13);
		if (tolower(ptemp[0])=='y') {
			sid->dat->wm->showdebug=1;
		}
		strncpy(sid->dat->wm->signature,    sql_getvalue(sqr, 0, 14), sizeof(sid->dat->wm->signature)-1);
	}
	sql_freeresult(sqr);
	return 0;
}

int dbread_getheader(CONN *sid, int sqr, int tuple, wmheader *header)
{
	char *ptemp;

	memset((char *)header, 0, sizeof(wmheader));
	if ((tuple<0)||(tuple>sql_numtuples(sqr))) return -1;
	header->localid=atoi(sql_getvalue(sqr, tuple, 0));
	snprintf(header->status,      sizeof(header->status)-1,      "%s", sql_getvalue(sqr, tuple, 10));
	header->accountid=atoi(sql_getvalue(sqr, tuple, 8));
	header->size=atoi(sql_getvalue(sqr, tuple, 11));
	snprintf(header->uidl,        sizeof(header->uidl)-1,        "%s", sql_getvalue(sqr, tuple, 12));
	snprintf(header->From,        sizeof(header->From)-1,        "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 13)));
	snprintf(header->ReplyTo,     sizeof(header->ReplyTo)-1,     "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 14)));
	snprintf(header->To,          sizeof(header->To)-1,          "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 15)));
	snprintf(header->CC,          sizeof(header->CC)-1,          "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 16)));
	snprintf(header->BCC,         sizeof(header->BCC)-1,         "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 17)));
	snprintf(header->Subject,     sizeof(header->Subject)-1,     "%s", DecodeRFC2047(sid, sql_getvalue(sqr, tuple, 18)));
	snprintf(header->Date,        sizeof(header->Date)-1,        "%s", sql_getvalue(sqr, tuple, 19));
	snprintf(header->MessageID,   sizeof(header->MessageID)-1,   "%s", sql_getvalue(sqr, tuple, 20));
	snprintf(header->InReplyTo,   sizeof(header->InReplyTo)-1,   "%s", sql_getvalue(sqr, tuple, 21));
	snprintf(header->contenttype, sizeof(header->contenttype)-1, "%s", sql_getvalue(sqr, tuple, 22));
	snprintf(header->boundary,    sizeof(header->boundary)-1,    "%s", sql_getvalue(sqr, tuple, 23));
	snprintf(header->encoding,    sizeof(header->encoding)-1,    "%s", sql_getvalue(sqr, tuple, 24));
	ptemp=header->From;
	while ((*ptemp)&&(strlen(header->FromName)<sizeof(header->FromName)-1)) {
		if (*ptemp=='<') break;
		if (*ptemp=='"') { ptemp++; continue; }
		header->FromName[strlen(header->FromName)]=*ptemp++;
	}
	while (strlen(header->FromName)&&(header->FromName[strlen(header->FromName)-1]==' ')) {
		header->FromName[strlen(header->FromName)-1]='\0';
	}
	ptemp=header->From;
	while ((*ptemp)&&(*ptemp!='<')) ptemp++;
	if (*ptemp=='<') ptemp++;
	while ((*ptemp)&&(*ptemp!='>')&&(strlen(header->FromAddr)<sizeof(header->FromAddr)-1)) {
		header->FromAddr[strlen(header->FromAddr)]=*ptemp;
		ptemp++;
	}
	if (strlen(header->FromAddr)==0) {
		strncpy(header->FromAddr, header->From, sizeof(header->FromAddr)-1);
	}
	if (strlen(header->ReplyTo)==0) {
		strncpy(header->ReplyTo, header->FromAddr, sizeof(header->ReplyTo)-1);
	}
	return 0;
}
