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
#include "ghttpd_main.h"

static int auth_checkpass(CONN *sid, char *password)
{
	char cpassword[64];
	char cpass[64];
	char salt[10];
	short int i;
	int sqr;

	DEBUG_IN(sid, "auth_checkpass()");
	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	if ((sqr=sql_queryf("SELECT userid, password FROM gw_users WHERE username = '%s' AND domainid = %d AND enabled > 0", sid->dat->user_username, sid->dat->user_did))<0) {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	sid->dat->user_uid=atoi(sql_getvalue(sqr, 0, 0));
	strncpy(cpassword, sql_getvalue(sqr, 0, 1), sizeof(cpassword)-1);
	sql_freeresult(sqr);
	memset(salt, 0, sizeof(salt));
	memset(cpass, 0, sizeof(cpass));
	if (strncmp(cpassword, "$1$", 3)==0) {
		for (i=0;i<8;i++) salt[i]=cpassword[i+3];
		md5_crypt(cpass, password, salt);
		if (strcmp(cpassword, cpass)!=0) {
			memset(salt, 0, sizeof(salt));
			memset(cpass, 0, sizeof(cpass));
			memset(cpassword, 0, sizeof(cpassword));
			DEBUG_OUT(sid, "auth_checkpass()");
			return -1;
		}
	} else {
		memset(salt, 0, sizeof(salt));
		memset(cpass, 0, sizeof(cpass));
		memset(cpassword, 0, sizeof(cpassword));
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	memset(salt, 0, sizeof(salt));
	memset(cpass, 0, sizeof(cpass));
	memset(cpassword, 0, sizeof(cpassword));
	DEBUG_OUT(sid, "auth_checkpass()");
	return 0;
}

static int auth_renewcookie(CONN *sid, int settoken)
{
	char timebuffer[100];
	time_t t;
	int sqr;
	int i, j;

	int userid=0;
	int domainid=0;

	DEBUG_IN(sid, "auth_renewcookie()");
	snprintf(sid->dat->user_language, sizeof(sid->dat->user_language)-1, config->langcode);
	snprintf(sid->dat->user_theme, sizeof(sid->dat->user_theme)-1, "default");
	if ((settoken)&&(strlen(sid->dat->user_token)!=32)) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if ((sqr=sql_queryf("SELECT userid, domainid FROM gw_usersessions WHERE remoteip = '%s' AND token = '%s'", sid->dat->in_RemoteAddr, sid->dat->user_token))<0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	userid   = atoi(sql_getvalue(sqr, 0, 0));
	domainid = atoi(sql_getvalue(sqr, 0, 1));
	sql_freeresult(sqr);
	if ((sqr=sql_queryf("SELECT * FROM gw_users WHERE userid = %d AND domainid = %d AND enabled > 0", userid, domainid))<0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	sid->dat->user_uid = atoi(sql_getvaluebyname(sqr, 0, "userid"));
	sid->dat->user_gid = atoi(sql_getvaluebyname(sqr, 0, "groupid"));
	sid->dat->user_did = atoi(sql_getvaluebyname(sqr, 0, "domainid"));
	snprintf(sid->dat->user_username, sizeof(sid->dat->user_username)-1, "%s", sql_getvaluebyname(sqr, 0, "username"));
	/* memset((char *)&sid->dat->auth, 0, sizeof(sid->dat->auth)); */
	for (i=0,j=0;i<sql_numfields(sqr);i++) {
		if (strncmp(sql_getname(sqr, i), "auth", 4)!=0) continue;
		strncpy(sid->dat->auth[j].name, sql_getname(sqr, i)+4, sizeof(sid->dat->auth[j].name)-1);
		sid->dat->auth[j].val=atoi(sql_getvalue(sqr, 0, i));
		j++;
		if (j>=MAX_AUTH_FIELDS) break;
	}
	sid->dat->user_daystart      = atoi(sql_getvaluebyname(sqr, 0, "prefdaystart"));
	sid->dat->user_daylength     = atoi(sql_getvaluebyname(sqr, 0, "prefdaylength"));
	sid->dat->user_mailcurrent   = atoi(sql_getvaluebyname(sqr, 0, "prefmailcurrent"));
	sid->dat->user_maildefault   = atoi(sql_getvaluebyname(sqr, 0, "prefmaildefault"));
	sid->dat->user_maxlist       = atoi(sql_getvaluebyname(sqr, 0, "prefmaxlist"));
	sid->dat->user_menustyle     = atoi(sql_getvaluebyname(sqr, 0, "prefmenustyle"));
	sid->dat->user_timezone      = atoi(sql_getvaluebyname(sqr, 0, "preftimezone"));
	snprintf(sid->dat->user_language, sizeof(sid->dat->user_language)-1, "%s", sql_getvaluebyname(sqr, 0, "preflanguage"));
	snprintf(sid->dat->user_theme, sizeof(sid->dat->user_theme)-1, "%s", sql_getvaluebyname(sqr, 0, "preftheme"));
	if (!module_exists("mod_mail")) {
		sid->dat->user_maildefault=0;
	}
	if (!module_exists("mod_html")) {
		sid->dat->user_menustyle=0;
	}
	if (p_strcasestr(sid->dat->in_UserAgent, "LYNX")!=NULL) {
		sid->dat->user_menustyle=0;
	}
	sql_freeresult(sqr);
//	if (strlen(sid->dat->user_username)==0) {
//		DEBUG_OUT(sid, "auth_renewcookie()");
//		return -1;
//	}
	if (sid->dat->user_daystart<0)   sid->dat->user_daystart=0;
	if (sid->dat->user_daystart>23)  sid->dat->user_daystart=23;
	if (sid->dat->user_daylength<1)  sid->dat->user_daylength=1;
	if (sid->dat->user_daylength>24) sid->dat->user_daylength=24;
	if (sid->dat->user_maxlist<5)    sid->dat->user_maxlist=5;
	if (sid->dat->user_menustyle<0)  sid->dat->user_menustyle=0;
	if (strlen(sid->dat->user_language)==0) {
		snprintf(sid->dat->user_language, sizeof(sid->dat->user_language)-1, config->langcode);
	}
	if (strlen(sid->dat->user_theme)==0) {
		snprintf(sid->dat->user_theme, sizeof(sid->dat->user_theme)-1, "default");
	}
	if (settoken) {
		t=time(NULL)+86400;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		snprintf(sid->dat->out_SetCookieToken, sizeof(sid->dat->out_SetCookieToken)-1, "gwtoken=%s; expires=%s; path=/", sid->dat->user_token, timebuffer);
	}
	DEBUG_OUT(sid, "auth_renewcookie()");
	return 0;
}

char *auth_setpass(CONN *sid, char *rpassword)
{
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char *cpassword=getbuffer(sid);
	char salt[10];
	int i;

	DEBUG_IN(sid, "auth_setpass()");
	memset(salt, 0, sizeof(salt));
	srand(time(NULL));
	for (i=0;i<8;i++) salt[i]=itoa64[(rand()%64)];
	md5_crypt(cpassword, rpassword, salt);
	DEBUG_OUT(sid, "auth_setpass()");
	return cpassword;
}

int auth_getcookie(CONN *sid)
{
	char *ptemp;

	DEBUG_IN(sid, "auth_getcookie()");
	snprintf(sid->dat->user_language, sizeof(sid->dat->user_language)-1, config->langcode);
	snprintf(sid->dat->user_theme, sizeof(sid->dat->user_theme)-1, "default");
	ptemp=strstr(sid->dat->in_Cookie, "gwtoken=");
	if (ptemp==NULL) {
		DEBUG_OUT(sid, "auth_getcookie()");
		return -1;
	}
	ptemp+=8;
	while ((*ptemp)&&(*ptemp!=':')&&(*ptemp!=';')&&(strlen(sid->dat->user_token)<sizeof(sid->dat->user_token))) {
		sid->dat->user_token[strlen(sid->dat->user_token)]=*ptemp++;
	}
	DEBUG_OUT(sid, "auth_getcookie()");
	return auth_renewcookie(sid, 1);
}

int auth_setcookie(CONN *sid)
{
	MD5_CONTEXT c;
	unsigned char md[MD5_SIZE];
	char *ptemp;
	char timebuffer[32];
	char password[64];
	char domain[64];
	int result;
	struct timeval ttime;
	struct timezone tzone;
	int sqr;
	int domainid;
	int i;

	DEBUG_IN(sid, "auth_setcookie()");
	memset(password, 0, sizeof(password));
	gettimeofday(&ttime, &tzone);
	time_unix2sql(timebuffer, sizeof(timebuffer)-1, ttime.tv_sec);
	if ((getxmlparam(sid, 1, "string")!=NULL)&&(getxmlparam(sid, 2, "string")!=NULL)&&(getxmlparam(sid, 3, "string")!=NULL)) {
		strncpy(sid->dat->user_username, getxmlparam(sid, 1, "string"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getxmlparam(sid, 2, "string"), sizeof(password)-1);
		strncpy(domain, getxmlparam(sid, 3, "string"), sizeof(domain)-1);
		domainid=domain_getid(domain);
		if (domainid<0) {
			sid->dat->user_did=1;
		} else {
			sid->dat->user_did=domainid;
		}
		if ((ptemp=domain_getname(domain, sizeof(domain)-1, sid->dat->user_did))!=NULL) {
			strncpy(sid->dat->user_domainname, ptemp, sizeof(sid->dat->user_domainname)-1);
		} else {
			strncpy(sid->dat->user_domainname, "NULL", sizeof(sid->dat->user_domainname)-1);
		}
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			if (strcmp(sid->dat->user_username, "guest")!=0) {
				md5_update(&c, timebuffer, strlen(timebuffer));
			}
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 0);
		}
		DEBUG_OUT(sid, "auth_setcookie()");
		return -1;
	}
	if ((getpostenv(sid, "USERNAME")!=NULL)&&(getpostenv(sid, "PASSWORD")!=NULL)&&(getpostenv(sid, "DOMAIN")!=NULL)) {
		strncpy(sid->dat->user_username, getpostenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getpostenv(sid, "PASSWORD"), sizeof(password)-1);
		ptemp=getpostenv(sid, "DOMAIN");
		domainid=domain_getid(ptemp);
		if (domainid<0) {
			sid->dat->user_did=1;
		} else {
			sid->dat->user_did=domainid;
		}
		if ((ptemp=domain_getname(domain, sizeof(domain)-1, sid->dat->user_did))!=NULL) {
			strncpy(sid->dat->user_domainname, ptemp, sizeof(sid->dat->user_domainname)-1);
		} else {
			strncpy(sid->dat->user_domainname, "NULL", sizeof(sid->dat->user_domainname)-1);
		}
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			if (strcmp(sid->dat->user_username, "guest")!=0) {
				md5_update(&c, timebuffer, strlen(timebuffer));
			}
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			if ((sqr=sql_queryf("SELECT sessionid FROM gw_usersessions WHERE userid = %d AND domainid = %d ORDER BY obj_mtime DESC", sid->dat->user_uid, sid->dat->user_did))<0) return -1;
			for (i=http_proc.config.http_sesslimit-1;i<sql_numtuples(sqr);i++) {
				if (i<0) continue;
				sql_updatef("DELETE FROM gw_usersessions WHERE sessionid = %d AND userid = %d AND domainid = %d", atoi(sql_getvalue(sqr, i, 0)), sid->dat->user_uid, sid->dat->user_did);
			}
			sql_freeresult(sqr);
			sql_updatef("INSERT INTO gw_usersessions (obj_ctime, obj_mtime, obj_uid, obj_did, remoteip, token, userid, domainid) VALUES ('%s', '%s', %d, %d, '%s', '%s', %d, %d)", timebuffer, timebuffer, sid->dat->user_uid, sid->dat->user_did, sid->dat->in_RemoteAddr, sid->dat->user_token, sid->dat->user_uid, sid->dat->user_did);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 1);
		}
	} else if ((getgetenv(sid, "USERNAME")!=NULL)&&(getgetenv(sid, "PASSWORD")!=NULL)&&(getgetenv(sid, "DOMAIN")!=NULL)) {
		strncpy(sid->dat->user_username, getgetenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getgetenv(sid, "PASSWORD"), sizeof(password)-1);
		ptemp=getgetenv(sid, "DOMAIN");
		domainid=domain_getid(ptemp);
		if (domainid<0) {
			sid->dat->user_did=1;
		} else {
			sid->dat->user_did=domainid;
		}
		if ((ptemp=domain_getname(domain, sizeof(domain)-1, sid->dat->user_did))!=NULL) {
			strncpy(sid->dat->user_domainname, ptemp, sizeof(sid->dat->user_domainname)-1);
		} else {
			strncpy(sid->dat->user_domainname, "NULL", sizeof(sid->dat->user_domainname)-1);
		}
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			if (strcmp(sid->dat->user_username, "guest")!=0) {
				md5_update(&c, timebuffer, strlen(timebuffer));
			}
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			if ((sqr=sql_queryf("SELECT sessionid FROM gw_usersessions WHERE userid = %d AND domainid = %d ORDER BY obj_mtime DESC", sid->dat->user_uid, sid->dat->user_did))<0) return -1;
			for (i=http_proc.config.http_sesslimit-1;i<sql_numtuples(sqr);i++) {
				if (i<0) continue;
				sql_updatef("DELETE FROM gw_usersessions WHERE sessionid = %d AND userid = %d AND domainid = %d", atoi(sql_getvalue(sqr, i, 0)), sid->dat->user_uid, sid->dat->user_did);
			}
			sql_freeresult(sqr);
			sql_updatef("INSERT INTO gw_usersessions (obj_ctime, obj_mtime, obj_uid, obj_did, remoteip, token, userid, domainid) VALUES ('%s', '%s', %d, %d, '%s', '%s', %d, %d)", timebuffer, timebuffer, sid->dat->user_uid, sid->dat->user_did, sid->dat->in_RemoteAddr, sid->dat->user_token, sid->dat->user_uid, sid->dat->user_did);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 1);
		}
	}
	DEBUG_OUT(sid, "auth_setcookie()");
	return -1;
}

void auth_logout(CONN *sid)
{
	time_t t;
	char timebuffer[100];

	DEBUG_IN(sid, "auth_logout()");
	sql_updatef("DELETE FROM gw_usersessions WHERE token = '%s' AND userid = %d AND domainid = %d", sid->dat->user_token, sid->dat->user_uid, sid->dat->user_did);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieToken, sizeof(sid->dat->out_SetCookieToken)-1, "gwtoken=NULL; path=/");
	htpage_logout(sid);
	DEBUG_OUT(sid, "auth_logout()");
}

int auth_priv(CONN *sid, char *service)
{
	int authlevel;
	int i;

	authlevel=0;
	if (strlen(service)<4) return 0;
	for (i=0;i<MAX_AUTH_FIELDS;i++) {
		if (strcmp(service, sid->dat->auth[i].name)==0) {
			authlevel=sid->dat->auth[i].val;
			break;
		}
	}
	if (authlevel&A_ADMIN) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	return authlevel;
}
