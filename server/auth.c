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
#include "main.h"

int auth_checkpass(CONN *sid, char *password)
{
	char cpassword[64];
	char salt[10];
	int sqr;

	DEBUG_IN(sid, "auth_checkpass()");
	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	if ((sqr=sql_queryf(sid, "SELECT userid, password FROM gw_users WHERE username = '%s' and enabled > 0", sid->dat->user_username))<0) {
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
	if (strncmp(cpassword, "$1$", 3)==0) {
		salt[0]=cpassword[3];
		salt[1]=cpassword[4];
		salt[2]=cpassword[5];
		salt[3]=cpassword[6];
		salt[4]=cpassword[7];
		salt[5]=cpassword[8];
		salt[6]=cpassword[9];
		salt[7]=cpassword[10];
		if (strcmp(cpassword, md5_crypt(sid, password, salt))!=0) {
			DEBUG_OUT(sid, "auth_checkpass()");
			return -1;
		}
	} else {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	DEBUG_OUT(sid, "auth_checkpass()");
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
	strncpy(cpassword, md5_crypt(sid, rpassword, salt), sizeof(sid->dat->smallbuf[0])-1);
	DEBUG_OUT(sid, "auth_setpass()");
	return cpassword;
}

int auth_renewcookie(CONN *sid, int settoken)
{
	char timebuffer[100];
	time_t t;
	int sqr;
	int i, j;

	DEBUG_IN(sid, "auth_renewcookie()");
	if (strlen(sid->dat->user_username)==0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if ((settoken)&&(strlen(sid->dat->user_token)!=32)) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if ((sqr=sql_queryf(sid, "SELECT * FROM gw_users WHERE username = '%s'", sid->dat->user_username))<0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (settoken) {
		if (strcmp(sid->dat->in_RemoteAddr, sql_getvaluebyname(sqr, 0, "loginip"))!=0) {
			sql_freeresult(sqr);
			DEBUG_OUT(sid, "auth_renewcookie()");
			return -1;
		}
		if (strcmp(sid->dat->user_token, sql_getvaluebyname(sqr, 0, "logintoken"))!=0) {
			sql_freeresult(sqr);
			DEBUG_OUT(sid, "auth_renewcookie()");
			return -1;
		}
	}
	sid->dat->user_uid = atoi(sql_getvaluebyname(sqr, 0, "userid"));
	sid->dat->user_gid = atoi(sql_getvaluebyname(sqr, 0, "groupid"));
//	memset((char *)&sid->dat->auth, 0, sizeof(sid->dat->auth));
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
	if (!module_exists(sid, "mod_mail")) {
		sid->dat->user_maildefault=0;
	}
	if (!module_exists(sid, "mod_html")) {
		sid->dat->user_menustyle=0;
	}
	if (strcasestr(sid->dat->in_UserAgent, "LYNX")!=NULL) {
		sid->dat->user_menustyle=0;
	}
	sql_freeresult(sqr);
	if (sid->dat->user_daystart<0)   sid->dat->user_daystart=0;
	if (sid->dat->user_daystart>23)  sid->dat->user_daystart=23;
	if (sid->dat->user_daylength<1)  sid->dat->user_daylength=1;
	if (sid->dat->user_daylength>24) sid->dat->user_daylength=24;
	if (sid->dat->user_maxlist<5)    sid->dat->user_maxlist=5;
	if (sid->dat->user_menustyle<0)  sid->dat->user_menustyle=0;
	if (settoken) {
		t=time(NULL)+604800;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
		t=time(NULL)+43200;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=%s; expires=%s; path=/", sid->dat->user_token, timebuffer);
	}
	DEBUG_OUT(sid, "auth_renewcookie()");
	return 0;
}

int auth_getcookie(CONN *sid)
{
	char *ptemp;

	DEBUG_IN(sid, "auth_getcookie()");
	ptemp=strstr(sid->dat->in_Cookie, "gwuser=");
	if (ptemp==NULL) {
		DEBUG_OUT(sid, "auth_getcookie()");
		return -1;
	}
	ptemp+=7;
	while ((*ptemp)&&(*ptemp!=':')&&(*ptemp!=';')&&(strlen(sid->dat->user_username)<sizeof(sid->dat->user_username))) {
		sid->dat->user_username[strlen(sid->dat->user_username)]=*ptemp++;
	}
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
	MD5_CTX c;
	unsigned char md[MD5_SIZE];
	char timebuffer[32];
	char password[64];
	int result;
	struct timeval ttime;
	struct timezone tzone;
	int i;

	DEBUG_IN(sid, "auth_setcookie()");
	memset(password, 0, sizeof(password));
	gettimeofday(&ttime, &tzone);
	snprintf(timebuffer, sizeof(timebuffer)-1, "%s", time_unix2sql(sid, ttime.tv_sec));
	if ((getxmlparam(sid, 1, "string")!=NULL)&&(getxmlparam(sid, 2, "string")!=NULL)) {
		strncpy(sid->dat->user_username, getxmlparam(sid, 1, "string"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getxmlparam(sid, 2, "string"), sizeof(password)-1);
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			md5_update(&c, timebuffer, strlen(timebuffer));
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 0);
		}
	} else if ((getpostenv(sid, "USERNAME")!=NULL)&&(getpostenv(sid, "PASSWORD")!=NULL)) {
		strncpy(sid->dat->user_username, getpostenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getpostenv(sid, "PASSWORD"), sizeof(password)-1);
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			md5_update(&c, timebuffer, strlen(timebuffer));
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			sql_updatef(sid, "UPDATE gw_users SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", sid->dat->in_RemoteAddr, timebuffer, sid->dat->user_token, sid->dat->user_username);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 1);
		}
	} else if ((getgetenv(sid, "USERNAME")!=NULL)&&(getgetenv(sid, "PASSWORD")!=NULL)) {
		strncpy(sid->dat->user_username, getgetenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getgetenv(sid, "PASSWORD"), sizeof(password)-1);
		result=auth_checkpass(sid, password);
		if (result==0) {
			md5_init(&c);
			md5_update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			md5_update(&c, timebuffer, strlen(timebuffer));
			md5_final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strncatf(sid->dat->user_token, sizeof(sid->dat->user_token)-strlen(sid->dat->user_token)-1, "%02x", md[i]);
			sql_updatef(sid, "UPDATE gw_users SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", sid->dat->in_RemoteAddr, timebuffer, sid->dat->user_token, sid->dat->user_username);
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
	sql_updatef(sid, "UPDATE gw_users SET logintoken='NULL' WHERE username = '%s'", sid->dat->user_username);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
	snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=NULL; path=/");
	htpage_logout(sid);
	DEBUG_OUT(sid, "auth_logout()");
}

int auth_priv(CONN *sid, char *service)
{
	int authlevel;
	int i;

//	DEBUG_IN(sid, "auth_priv()");
	authlevel=0;
	if (strlen(service)<4) return 0;
	for (i=0;i<MAX_AUTH_FIELDS;i++) {
		if (strcmp(service, sid->dat->auth[i].name)==0) {
			authlevel=sid->dat->auth[i].val;
			break;
		}
	}

	if (authlevel&A_ADMIN) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
//	DEBUG_OUT(sid, "auth_priv()");
	return authlevel;
}
