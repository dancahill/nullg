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

int auth_checkpass(CONNECTION *sid, char *password)
{
	char cpassword[64];
	char salt[10];
	int sqr;

	DEBUG_IN(sid, "auth_checkpass()");
	if ((strlen(sid->dat->user_username)==0)||(strlen(password)==0)) {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	if ((sqr=sqlQueryf(sid, "SELECT password FROM gw_users WHERE username = '%s' and enabled > 0", sid->dat->user_username))<0) {
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		DEBUG_OUT(sid, "auth_checkpass()");
		return -1;
	}
	strncpy(cpassword, sqlGetvalue(sqr, 0, 0), sizeof(cpassword)-1);
	sqlFreeconnect(sqr);
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
		if (strcmp(cpassword, MD5Crypt(password, salt))!=0) {
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

char *auth_setpass(CONNECTION *sid, char *rpassword)
{
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char *cpassword=getbuffer(sid);
	char salt[10];
	int i;

	DEBUG_IN(sid, "auth_setpass()");
	memset(salt, 0, sizeof(salt));
	srand(time(NULL));
	for (i=0;i<8;i++) salt[i]=itoa64[(rand()%64)];
	strncpy(cpassword, MD5Crypt(rpassword, salt), sizeof(sid->dat->smallbuf[0])-1);
	DEBUG_OUT(sid, "auth_setpass()");
	return cpassword;
}

int auth_renewcookie(CONNECTION *sid, int settoken)
{
	char timebuffer[100];
	time_t t;
	int sqr;

	DEBUG_IN(sid, "auth_renewcookie()");
	if (strlen(sid->dat->user_username)==0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if ((settoken)&&(strlen(sid->dat->user_token)!=32)) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if ((sqr=sqlQueryf(sid, "SELECT loginip, logintoken, userid, groupid, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authfiles, authforums, authmessages, authorders, authprofile, authquery, authwebmail, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone FROM gw_users WHERE username = '%s'", sid->dat->user_username))<0) {
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		DEBUG_OUT(sid, "auth_renewcookie()");
		return -1;
	}
	if (settoken) {
		if (strcmp(sid->dat->in_RemoteAddr, sqlGetvalue(sqr, 0, 0))!=0) {
			sqlFreeconnect(sqr);
			DEBUG_OUT(sid, "auth_renewcookie()");
			return -1;
		}
		if (strcmp(sid->dat->user_token, sqlGetvalue(sqr, 0, 1))!=0) {
			sqlFreeconnect(sqr);
			DEBUG_OUT(sid, "auth_renewcookie()");
			return -1;
		}
	}
	sid->dat->user_uid = atoi(sqlGetvalue(sqr, 0, 2));
	sid->dat->user_gid = atoi(sqlGetvalue(sqr, 0, 3));
	sid->dat->user_authadmin     = atoi(sqlGetvalue(sqr, 0, 4));
	sid->dat->user_authbookmarks = atoi(sqlGetvalue(sqr, 0, 5));
	sid->dat->user_authcalendar  = atoi(sqlGetvalue(sqr, 0, 6));
	sid->dat->user_authcalls     = atoi(sqlGetvalue(sqr, 0, 7));
	sid->dat->user_authcontacts  = atoi(sqlGetvalue(sqr, 0, 8));
	sid->dat->user_authfiles     = atoi(sqlGetvalue(sqr, 0, 9));
	sid->dat->user_authforums    = atoi(sqlGetvalue(sqr, 0, 10));
	sid->dat->user_authmessages  = atoi(sqlGetvalue(sqr, 0, 11));
	sid->dat->user_authorders    = atoi(sqlGetvalue(sqr, 0, 12));
	sid->dat->user_authprofile   = atoi(sqlGetvalue(sqr, 0, 13));
	sid->dat->user_authqueries   = atoi(sqlGetvalue(sqr, 0, 14));
	sid->dat->user_authwebmail   = atoi(sqlGetvalue(sqr, 0, 15));
	sid->dat->user_daystart      = atoi(sqlGetvalue(sqr, 0, 16));
	sid->dat->user_daylength     = atoi(sqlGetvalue(sqr, 0, 17));
	sid->dat->user_mailcurrent   = atoi(sqlGetvalue(sqr, 0, 18));
	sid->dat->user_maildefault   = atoi(sqlGetvalue(sqr, 0, 19));
	sid->dat->user_maxlist       = atoi(sqlGetvalue(sqr, 0, 20));
	sid->dat->user_menustyle     = atoi(sqlGetvalue(sqr, 0, 21));
	if (strcasestr(sid->dat->in_UserAgent, "LYNX")!=NULL) {
		sid->dat->user_menustyle=0;
	}
	sid->dat->user_timezone      = atoi(sqlGetvalue(sqr, 0, 22));
	sqlFreeconnect(sqr);
	if (sid->dat->user_daystart<0) sid->dat->user_daystart=0;
	if (sid->dat->user_daystart>23) sid->dat->user_daystart=23;
	if (sid->dat->user_daylength<1) sid->dat->user_daylength=1;
	if (sid->dat->user_daylength>24) sid->dat->user_daylength=24;
	if (sid->dat->user_maxlist<5) sid->dat->user_maxlist=5;
	if (sid->dat->user_menustyle<0) sid->dat->user_menustyle=0;
//	if (sid->dat->user_tzoffset<-7200) sid->dat->user_tzoffset=-7200;
//	if (sid->dat->user_tzoffset>7200) sid->dat->user_tzoffset=7200;
	if (settoken) {
		t=time(NULL)+604800;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
//		snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=%s; path=/", sid->dat->user_token);
		t=time(NULL)+43200;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=%s; expires=%s; path=/", sid->dat->user_token, timebuffer);
	}
	DEBUG_OUT(sid, "auth_renewcookie()");
	return 0;
}

int auth_getcookie(CONNECTION *sid)
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

int auth_setcookie(CONNECTION *sid)
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
			MD5Init(&c);
			MD5Update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			MD5Update(&c, timebuffer, strlen(timebuffer));
			MD5Final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strcatf(sid->dat->user_token, "%02x", md[i]);
//			sqlUpdatef(sid, "UPDATE gw_users SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", sid->dat->in_RemoteAddr, timebuffer, sid->dat->user_token, sid->dat->user_username);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 0);
		}
	} else if ((getpostenv(sid, "USERNAME")!=NULL)&&(getpostenv(sid, "PASSWORD")!=NULL)) {
		strncpy(sid->dat->user_username, getpostenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getpostenv(sid, "PASSWORD"), sizeof(password)-1);
		result=auth_checkpass(sid, password);
		if (result==0) {
			MD5Init(&c);
			MD5Update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			MD5Update(&c, timebuffer, strlen(timebuffer));
			MD5Final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strcatf(sid->dat->user_token, "%02x", md[i]);
			sqlUpdatef(sid, "UPDATE gw_users SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", sid->dat->in_RemoteAddr, timebuffer, sid->dat->user_token, sid->dat->user_username);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 1);
		}
	} else if ((getgetenv(sid, "USERNAME")!=NULL)&&(getgetenv(sid, "PASSWORD")!=NULL)) {
		strncpy(sid->dat->user_username, getgetenv(sid, "USERNAME"), sizeof(sid->dat->user_username)-1);
		strncpy(password, getgetenv(sid, "PASSWORD"), sizeof(password)-1);
		result=auth_checkpass(sid, password);
		if (result==0) {
			MD5Init(&c);
			MD5Update(&c, sid->dat->user_username, strlen(sid->dat->user_username));
			MD5Update(&c, timebuffer, strlen(timebuffer));
			MD5Final(&(md[0]),&c);
			memset(sid->dat->user_token, 0, sizeof(sid->dat->user_token));
			for (i=0;i<MD5_SIZE;i++) strcatf(sid->dat->user_token, "%02x", md[i]);
			sqlUpdatef(sid, "UPDATE gw_users SET loginip='%s', logintime='%s', logintoken='%s' WHERE username = '%s'", sid->dat->in_RemoteAddr, timebuffer, sid->dat->user_token, sid->dat->user_username);
			memset(password, 0, sizeof(password));
			DEBUG_OUT(sid, "auth_setcookie()");
			return auth_renewcookie(sid, 1);
		}
	}
	DEBUG_OUT(sid, "auth_setcookie()");
	return -1;
}

void auth_logout(CONNECTION *sid)
{
	time_t t;
	char timebuffer[100];

	DEBUG_IN(sid, "auth_logout()");
	sqlUpdatef(sid, "UPDATE gw_users SET logintoken='NULL' WHERE username = '%s'", sid->dat->user_username);
	t=time(NULL)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(sid->dat->out_SetCookieUser, sizeof(sid->dat->out_SetCookieUser)-1, "gwuser=%s; expires=%s; path=/", sid->dat->user_username, timebuffer);
	snprintf(sid->dat->out_SetCookiePass, sizeof(sid->dat->out_SetCookiePass)-1, "gwtoken=NULL; path=/");
	htpage_logout(sid);
	DEBUG_OUT(sid, "auth_logout()");
}

int auth_priv(CONNECTION *sid, int service)
{
	int authlevel=0;

//	DEBUG_IN(sid, "auth_priv()");
	switch (service) {
		case AUTH_ADMIN:
			authlevel=sid->dat->user_authadmin;
			break;
		case AUTH_CALENDAR:
			authlevel=sid->dat->user_authcalendar;
			break;
		case AUTH_CALLS:
			authlevel=sid->dat->user_authcalls;
			break;
		case AUTH_CONTACTS:
			authlevel=sid->dat->user_authcontacts;
			break;
		case AUTH_FILES:
			authlevel=sid->dat->user_authfiles;
			break;
		case AUTH_FORUMS:
			authlevel=sid->dat->user_authforums;
			break;
		case AUTH_BOOKMARKS:
			authlevel=sid->dat->user_authbookmarks;
			break;
		case AUTH_MESSAGES:
			authlevel=sid->dat->user_authmessages;
			break;
		case AUTH_ORDERS:
			authlevel=sid->dat->user_authorders;
			break;
		case AUTH_PROFILE:
			authlevel=sid->dat->user_authprofile;
			break;
		case AUTH_QUERIES:
			authlevel=sid->dat->user_authqueries;
			break;
		case AUTH_WEBMAIL:
			authlevel=sid->dat->user_authwebmail;
			break;
		default:
			authlevel=0;
	}
	if (authlevel&A_ADMIN) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
//	DEBUG_OUT(sid, "auth_priv()");
	return authlevel;
}
