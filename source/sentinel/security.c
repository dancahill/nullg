/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

char *MD5_salt(void)
{
	int sid=getsid();
	char *salt=conn[sid].dat->envbuf;
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int i;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	srand(time(NULL));
	for (i=0;i<8;i++) {
		salt[i]=itoa64[(rand()%64)];
	}
//	return "0hs2u/6B";
	return salt;
}

int check_pass(int sid, char *password)
{
	char cpassword[64];
	char SALT[10];
	int sqr;

	if ((strlen(conn[sid].dat->in_username)==0)||(strlen(password)==0)) return -1;
	if ((sqr=sqlQueryf("SELECT password FROM gw_users WHERE username = '%s' and enabled > 0", conn[sid].dat->in_username))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	strncpy(cpassword, sqlGetvalue(sqr, 0, 0), sizeof(cpassword)-1);
	sqlFreeconnect(sqr);
	memset(SALT, 0, sizeof(SALT));
	if (strncmp(cpassword, "$1$", 3)==0) {
		SALT[0]=cpassword[3];
		SALT[1]=cpassword[4];
		SALT[2]=cpassword[5];
		SALT[3]=cpassword[6];
		SALT[4]=cpassword[7];
		SALT[5]=cpassword[8];
		SALT[6]=cpassword[9];
		SALT[7]=cpassword[10];
		if (strcmp(cpassword, MD5_crypt(password, SALT))!=0) return -1;
	} else {
		return -1;
	}
	return 0;
}

char *setpass(char *rpassword)
{
	int sid=getsid();
	char *cpassword=conn[sid].dat->envbuf;

	if (!RunAsCGI) pthread_mutex_lock(&Lock.Crypt);
	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	strncpy(cpassword, MD5_crypt(rpassword, MD5_salt()), sizeof(conn[sid].dat->envbuf)-1);
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
	return cpassword;
}

int getcookie(int sid)
{
	char *ptemp;

	if (!RunAsCGI) pthread_mutex_lock(&Lock.Crypt);
	ptemp=strstr(conn[sid].dat->in_Cookie, "sentinel=");
	if (ptemp==NULL) {
		if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
		return -1;
	}
	ptemp+=9;
	while ((*ptemp)&&(*ptemp!=':')&&(*ptemp!=';')&&(strlen(conn[sid].dat->in_username)<sizeof(conn[sid].dat->in_username))) {
		conn[sid].dat->in_username[strlen(conn[sid].dat->in_username)]=*ptemp++;
	}
	ptemp=strstr(conn[sid].dat->in_Cookie, "token=");
	if (ptemp==NULL) {
		if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
		return -1;
	}
	ptemp+=6;
	while ((*ptemp)&&(*ptemp!=':')&&(*ptemp!=';')&&(strlen(conn[sid].dat->in_token)<sizeof(conn[sid].dat->in_token))) {
		conn[sid].dat->in_token[strlen(conn[sid].dat->in_token)]=*ptemp++;
	}
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
	return renewcookie(sid);
}

int setcookie(int sid)
{
	MD5_CTX c;
	unsigned char md[MD5_DIGEST_LENGTH];
	char timebuffer[100];
	char password[64];
	int result;
	struct timeval ttime;
	struct timezone tzone;
	int i;

	if (!RunAsCGI) pthread_mutex_lock(&Lock.Crypt);
	memset(password, 0, sizeof(password));
	gettimeofday(&ttime, &tzone);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&ttime.tv_sec));
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if ((getpostenv(sid, "USERNAME")!=NULL)&&(getpostenv(sid, "PASSWORD")!=NULL)) {
			strncpy(conn[sid].dat->in_username, getpostenv(sid, "USERNAME"), sizeof(conn[sid].dat->in_username)-1);
			strncpy(password, getpostenv(sid, "PASSWORD"), sizeof(password)-1);
			result=check_pass(sid, password);
			if (result==0) {
				MD5_Init(&c);
				MD5_Update(&c, conn[sid].dat->in_username, strlen(conn[sid].dat->in_username));
				MD5_Update(&c, timebuffer, strlen(timebuffer));
				MD5_Final(&(md[0]),&c);
				memset(conn[sid].dat->in_token, 0, sizeof(conn[sid].dat->in_token));
				for (i=0;i<MD5_DIGEST_LENGTH;i++) strcatf(conn[sid].dat->in_token, "%02x", md[i]);
				sqlUpdatef("UPDATE gw_users SET lastloginip='%s', lastlogintime='%s', lastlogintoken='%s' WHERE username = '%s'", conn[sid].dat->in_ClientIP, timebuffer, conn[sid].dat->in_token, conn[sid].dat->in_username);
				ttime.tv_sec+=604800;
				strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&ttime.tv_sec));
				snprintf(conn[sid].dat->out_SetCookieUser, sizeof(conn[sid].dat->out_SetCookieUser)-1, "sentinel=%s; expires=%s; path=/", conn[sid].dat->in_username, timebuffer);
				snprintf(conn[sid].dat->out_SetCookiePass, sizeof(conn[sid].dat->out_SetCookiePass)-1, "token=%s; path=/", conn[sid].dat->in_token);
			}
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
			return result;
		}
	}
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.Crypt);
	return renewcookie(sid);
}

int renewcookie(int sid)
{
	char timebuffer[100];
	time_t t;
	int sqr;

	if ((strlen(conn[sid].dat->in_username)==0)||(strlen(conn[sid].dat->in_token)!=32)) return -1;
	t=time((time_t*)0)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	if ((sqr=sqlQueryf("SELECT lastloginip, lastlogintoken FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return -1;
	if (sqlNumtuples(sqr)!=1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	if (strcmp(conn[sid].dat->in_ClientIP, sqlGetvalue(sqr, 0, 0))!=0) {
		sqlFreeconnect(sqr);
		return -1;
	}
	if (strcmp(conn[sid].dat->in_token, sqlGetvalue(sqr, 0, 1))!=0) {
		sqlFreeconnect(sqr);
		return -1;
	}
	sqlFreeconnect(sqr);
	snprintf(conn[sid].dat->out_SetCookieUser, sizeof(conn[sid].dat->out_SetCookieUser)-1, "sentinel=%s; expires=%s; path=/", conn[sid].dat->in_username, timebuffer);
	snprintf(conn[sid].dat->out_SetCookiePass, sizeof(conn[sid].dat->out_SetCookiePass)-1, "token=%s; path=/", conn[sid].dat->in_token);
	return 0;
}

void sentinellogout(int sid)
{
	time_t t;
	char timebuffer[100];

	sqlUpdatef("UPDATE gw_users SET lastlogintoken='NULL' WHERE username = '%s'", conn[sid].dat->in_username);
	t=time((time_t*)0)+604800;
	strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	snprintf(conn[sid].dat->out_SetCookieUser, sizeof(conn[sid].dat->out_SetCookieUser)-1, "sentinel=%s; expires=%s; path=/", conn[sid].dat->in_username, timebuffer);
	snprintf(conn[sid].dat->out_SetCookiePass, sizeof(conn[sid].dat->out_SetCookiePass)-1, "token=NULL; path=/");
	printlogout(sid);
}

int auth(int sid, char *service)
{
	int authlevel=-1;
	int sqr;

	if ((sqr=sqlQueryf("SELECT auth%s FROM gw_users WHERE username = '%s'", service, conn[sid].dat->in_username))<0) return -1;
	if (sqlNumtuples(sqr)<1) {
		sqlFreeconnect(sqr);
		return -1;
	}
	authlevel=atoi(sqlGetvalue(sqr, 0, 0));
	sqlFreeconnect(sqr);
	return authlevel;
}

void logaccess(int loglevel, const char *format, ...)
{
	char logbuffer[1024];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	if (loglevel>config.server_loglevel) return;
	snprintf(file, sizeof(file)-1, "%s/access.log", config.server_etc_dir);
	FixSlashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime(&ttime.tv_sec));
		fprintf(fp, "%s - [%d] %s\n", timebuffer, loglevel, logbuffer);
		fclose(fp);
	}
}

void logerror(const char *format, ...)
{
	char logbuffer[1024];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/error.log", config.server_etc_dir);
	FixSlashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime(&ttime.tv_sec));
		fprintf(fp, "%s - %s\n", timebuffer, logbuffer);
		fclose(fp);
	}
}

#ifdef DEBUG
void logdata(const char *format, ...)
{
	char logbuffer[1024];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/data.log", config.server_etc_dir);
	FixSlashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime(&ttime.tv_sec));
		fprintf(fp, "%s", logbuffer);
		fclose(fp);
	}
}
#endif
