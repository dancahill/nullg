/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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

int dbread_mailaccount(CONN *sid, short int perm, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "email");
	obj_t *cobj, *tobj;

	if (auth<1) return -1;
	if (!(auth&A_MODIFY)&&(perm==2)) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Account',hosttype='POP3',popport=110,smtpport=25,smtpauth='n',notify=10,remove=2}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return 0;
	}
	if ((*qobj=ldir_getentry(sid->N, "emailaccount", NULL, index, sid->dat->did))==NULL) return -1;
	if (ldir_numentries(qobj)!=1) { ldir_freeresult(qobj); return -2; }
	/* this would be a good time to check record permissions..... */
	/* DECODE THE PASSWORD !!! */
	return 0;
}

int dbwrite_mailaccount(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "email");

	if (auth<2) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	/* ENCODE THE PASSWORD !!! */
//	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, EncodeBase64string(sid, mailacct.poppassword)));
	return ldir_saveentry(sid, index, "emailaccount", qobj);
}

int dbread_mailcurrent(CONN *sid, int mailcurrent)
{
	char *ptemp;
	obj_t *qobj;
//	SQLRES sqr;

	if ((qobj=ldir_getentry(sid->N, "emailaccount", NULL, mailcurrent, sid->dat->did))==NULL) return -1;
/*
	if (sql_queryf(&sqr, "SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, popssl, smtphost, smtpport, smtpssl, address, replyto, remove, showdebug, signature FROM gw_email_accounts where mailaccountid = %d and obj_uid = %d", mailcurrent, sid->dat->uid)<0) return -1;
	if (sql_numtuples(&sqr)!=1) {
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT mailaccountid FROM gw_email_accounts where obj_uid = %d", sid->dat->uid)<0) return -1;
		if (sql_numtuples(&sqr)<1) {
			sql_freeresult(&sqr);
			return -1;
		}
		sid->dat->mailcurrent=atoi(sql_getvalue(&sqr, 0, 0));
		if (sql_updatef("UPDATE gw_users SET prefmailcurrent = '%d' WHERE username = '%s'", sid->dat->mailcurrent, sid->dat->username)<0) return -1;
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT realname, organization, popusername, poppassword, smtpauth, hosttype, pophost, popport, popssl, smtphost, smtpport, smtpssl, address, replyto, remove, showdebug, signature FROM gw_email_accounts where mailaccountid = %d and obj_uid = %d", mailcurrent, sid->dat->uid)<0) return -1;
	}
*/
	if (ldir_numentries(&qobj)==1) {
		strncpy(sid->dat->wm->realname,     ldir_getval(&qobj, 0, "realname"),     sizeof(sid->dat->wm->realname)-1);
		strncpy(sid->dat->wm->organization, ldir_getval(&qobj, 0, "organization"), sizeof(sid->dat->wm->organization)-1);
		strncpy(sid->dat->wm->username,     ldir_getval(&qobj, 0, "popusername"),  sizeof(sid->dat->wm->username)-1);
		strncpy(sid->dat->wm->password,     DecodeBase64string(sid, ldir_getval(&qobj, 0, "poppassword")), sizeof(sid->dat->wm->password)-1);
		strncpy(sid->dat->wm->smtpauth,     ldir_getval(&qobj, 0, "smtpauth"),     sizeof(sid->dat->wm->smtpauth)-1);
		strncpy(sid->dat->wm->servertype,   ldir_getval(&qobj, 0, "hosttype"),     sizeof(sid->dat->wm->servertype)-1);
		strncpy(sid->dat->wm->popserver,    ldir_getval(&qobj, 0, "pophost"),      sizeof(sid->dat->wm->popserver)-1);
		sid->dat->wm->popport=atoi(ldir_getval(&qobj, 0, "popport"));
		sid->dat->wm->popssl=atoi(ldir_getval(&qobj, 0, "popssl"));
		strncpy(sid->dat->wm->smtpserver,   ldir_getval(&qobj, 0, "smtphost"),     sizeof(sid->dat->wm->smtpserver)-1);
		sid->dat->wm->smtpport=atoi(ldir_getval(&qobj, 0, "smtpport"));
		sid->dat->wm->smtpssl=atoi(ldir_getval(&qobj, 0, "smtpssl"));
		strncpy(sid->dat->wm->address,      ldir_getval(&qobj, 0, "address"),      sizeof(sid->dat->wm->address)-1);
		strncpy(sid->dat->wm->replyto,      ldir_getval(&qobj, 0, "replyto"),      sizeof(sid->dat->wm->replyto)-1);
		sid->dat->wm->remove=atoi(ldir_getval(&qobj, 0, "remove"));
		ptemp=ldir_getval(&qobj, 0, "showdebug");
		if (tolower(ptemp[0])=='y') sid->dat->wm->showdebug=1;
		strncpy(sid->dat->wm->signature,    ldir_getval(&qobj, 0, "signature"),    sizeof(sid->dat->wm->signature)-1);
	}
	ldir_freeresult(&qobj);
	return 0;
}

int dbread_mailheader(CONN *sid, short int perm, int index, obj_t **qobj)
{
	short int approved;
	int auth=auth_priv(sid, "email");
	obj_t *cobj, *tobj;

	if (auth<1) return EC_NOPERM;
//	if (!(auth&A_MODIFY)&&(perm==2)) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return EC_NOERROR;
	}
	if ((tobj=ldir_getentry(sid->N, "emailheader", NULL, index, sid->dat->did))==NULL) return EC_NORECORD;
	if (ldir_numentries(&tobj)!=1) { ldir_freeresult(&tobj); return EC_NORECORD; }
	if (*qobj) {
		cobj=*qobj;
		cobj->val->d.table=tobj->val->d.table;
	} else {
		*qobj=tobj;
	}
	approved=0;
	return approved;
/*
	char *ptemp;

	memset((char *)header, 0, sizeof(wmheader));
	if ((tuple<0)||(tuple>sql_numtuples(sqr))) return -1;
	header->localid=atoi(sql_getvalue(sqr, tuple, 0));
	snprintf(header->status,      sizeof(header->status)-1,      "%s", sql_getvalue(sqr, tuple, 10));
	header->accountid=atoi(sql_getvalue(sqr, tuple, 8));
	header->folderid=atoi(sql_getvalue(sqr, tuple, 9));
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
	snprintf(header->scanresult,  sizeof(header->scanresult)-1,  "%s", sql_getvalue(sqr, tuple, 25));
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
*/
	return 0;
}

int dbwrite_mailheader(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "email");

	if (auth<2) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	return ldir_saveentry(sid, index, "emailheader", qobj);
}
