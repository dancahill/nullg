/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include "httpd_main.h"

static void auth_dumpvars(nsp_state *N, obj_t *tobj, char *buf, int len)
{
	obj_t *cobj=tobj;
	short b;
	char *l;
	int x;

	for (;cobj;cobj=cobj->next) {
		if ((cobj->val->attr&NST_HIDDEN)||(cobj->val->attr&NST_SYSTEM)) continue;
		l=(cobj->next)?",":"";
		if (isdigit(cobj->name[0])) b=1; else b=0;
		if ((cobj->val->type==NT_NULL)||(cobj->val->type==NT_BOOLEAN)||(cobj->val->type==NT_NUMBER)) {
			x=snprintf(buf, len, "%s%s%s=%s%s ", b?"[":"", cobj->name, b?"]":"", nsp_tostr(N, cobj), l);
			len-=x; buf+=x;
		} else if (cobj->val->type==NT_STRING) {
			x=snprintf(buf, len, "%s%s%s=\"%s\"%s ", b?"[":"", cobj->name, b?"]":"", nsp_tostr(N, cobj), l);
			len-=x; buf+=x;
		} else if (cobj->val->type==NT_TABLE) {
			if (strcmp(cobj->name, "_globals_")==0) continue;
			x=snprintf(buf, len, "%s%s%s={ ", b?"[":"", cobj->name, b?"]":"");
			len-=x; buf+=x;
			auth_dumpvars(N, cobj->val->d.table.f, buf, len);
			len-=strlen(buf); buf+=strlen(buf);
			x=snprintf(buf, len, "}%s ", l);
			len-=x; buf+=x;
		}
	}
	return;
}

void auth_savesession(CONN *conn)
{
	obj_t *tobj=nsp_getobj(conn->N, &conn->N->g, "_SESSION");
	char *data=calloc(MAX_FIELD_SIZE, sizeof(char));
	char timebuffer[32];

	time_unix2sql(timebuffer, sizeof(timebuffer)-1, time(NULL));
	if (tobj->val->type==NT_TABLE) {
		auth_dumpvars(conn->N, tobj->val->d.table.f, data, MAX_FIELD_SIZE-1);
	}
	/* log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "[%s]", data); */
	sql_updatef(proc->N, "UPDATE nullsd_sessions SET mtime = '%s', data = '{ %s}' WHERE token = '%s' AND uid = %d AND did = %d", timebuffer, str2sql(conn->dat->largebuf, sizeof(conn->dat->largebuf)-1, data), conn->dat->token, conn->dat->uid, conn->dat->did);
	free(data);
	return;
}

static int auth_checkpass(CONN *conn, char *password)
{
	char cpassword[64];
	char cpass[64];
	char salt[10];
	short int i;
	obj_t *qobj=NULL;
	int rc;

	if ((strlen(conn->dat->username)==0)||(strlen(password)==0)) {
		return -1;
	}
	if ((qobj=ldir_getentry(conn->N, "person", conn->dat->username, 0, conn->dat->did))==NULL) {
		return -1;
	}
	if (ldir_numentries(&qobj)!=1) {
		ldir_freeresult(&qobj);
		return -1;
	}
	conn->dat->uid=atoi(ldir_getval(&qobj, 0, "uidNumber"));
	strncpy(cpassword, ldir_getval(&qobj, 0, "userPassword"), sizeof(cpassword)-1);
	ldir_freeresult(&qobj);
	memset(salt, 0, sizeof(salt));
	memset(cpass, 0, sizeof(cpass));
	rc=-1;
	if (strncmp(cpassword, "$1$", 3)==0) {
		for (i=0;i<8;i++) salt[i]=cpassword[i+3];
		md5_crypt(cpass, password, salt);
		if (strcmp(cpassword, cpass)==0) rc=0;
	}
	memset(salt, 0, sizeof(salt));
	memset(cpass, 0, sizeof(cpass));
	memset(cpassword, 0, sizeof(cpassword));
	return rc;
}

static int auth_renewcookie(CONN *conn, int settoken)
{
	obj_t *confobj=nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *htobj=nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj, *tobj, *uobj;
	obj_t *qobj=NULL;
	char timebuffer[100];
	char CookieToken[128];
	int userid=0;
	int domainid=0;
	int rc=-1;
	time_t t;
	char *p;
	int n;

	snprintf(conn->dat->language, sizeof(conn->dat->language)-1, nsp_getstr(proc->N, confobj, "default_language"));
	snprintf(conn->dat->theme, sizeof(conn->dat->theme)-1, "default");
	if ((settoken)&&(strlen(conn->dat->token)!=32)) {
		return -1;
	}
	rc=sql_queryf(proc->N, &qobj, "SELECT * FROM nullsd_sessions WHERE token = '%s' AND remoteaddr = '%s'", conn->dat->token, conn->socket.RemoteAddr);
	if (rc<0) return -1;
	if (sql_numtuples(proc->N, &qobj)!=1) {
		sql_freeresult(proc->N, &qobj);
		return -1;
	}
	userid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "uid"));
	domainid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "did"));
	cobj=nsp_getobj(proc->N, qobj, "_rows");
	if (cobj->val->type!=NT_TABLE) goto blah;
	cobj=nsp_getiobj(proc->N, cobj, 0);
	if (cobj->val->type!=NT_TABLE) goto blah;
	cobj=nsp_getobj(proc->N, cobj, "data");
	if ((cobj->val->type==NT_STRING)&&(cobj->val->d.str!=NULL)) {
		tobj=nsp_settable(conn->N, &conn->N->g, "_SESSION");
		nsp_linkval(conn->N, tobj, nsp_eval(conn->N, cobj->val->d.str));
	}
blah:
	sql_freeresult(proc->N, &qobj);
	if ((userid==0)||(domainid==0)) {
		return -1;
	}
	if ((qobj=ldir_getentry(conn->N, "person", NULL, userid, domainid))==NULL) {
		return -1;
	}
	if (ldir_numentries(&qobj)!=1) {
		ldir_freeresult(&qobj);
		return -1;
	}

	conn->dat->uid = atoi(ldir_getval(&qobj, 0, "uidNumber"));
	conn->dat->gid = atoi(ldir_getval(&qobj, 0, "groupid"));
	conn->dat->did = atoi(ldir_getval(&qobj, 0, "domainid"));
	snprintf(conn->dat->username, sizeof(conn->dat->username)-1, "%s", ldir_getval(&qobj, 0, "name"));
	nsp_setstr(conn->N, htobj, "REMOTE_USER", conn->dat->username, strlen(conn->dat->username));
	tobj=nsp_getobj(proc->N, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	/* steal the results and plant the tree in the user's back yard */
	cobj=nsp_getobj(conn->N, qobj, "_rows");
	if (cobj->val->type==NT_TABLE) {
		cobj=nsp_getiobj(conn->N, cobj, 0);
		if (cobj->val->type==NT_TABLE) {
			cobj=nsp_getobj(conn->N, cobj, "_data");
			if (cobj->val->type==NT_TABLE) {
				tobj=nsp_settable(conn->N, &conn->N->g, "_USER");
				tobj->val->d.table.f=cobj->val->d.table.f;
				cobj->val->d.table.f=NULL;
			}
		}
	}
	ldir_freeresult(&qobj);
	if (strlen(conn->dat->username)==0) return -1;

	uobj=nsp_settable(conn->N, &conn->N->g, "_USER");
	tobj=nsp_settable(conn->N, uobj, "pref");
	if (tobj->val->type==NT_TABLE) {
		/* maxlist */
		cobj=nsp_getobj(conn->N, tobj, "maxlist");
		if (cobj->val->type!=NT_NUMBER) {
			n=nsp_isnull(cobj)?5:(int)nsp_tonum(conn->N, cobj);
			cobj=nsp_setnum(conn->N, tobj, "maxlist", n);
		}
		conn->dat->maxlist=(int)cobj->val->d.num;
		/* timezone */
		cobj=nsp_getobj(conn->N, tobj, "timezone");
		if (cobj->val->type!=NT_NUMBER) {
			n=nsp_isnull(cobj)?10:(int)nsp_tonum(conn->N, cobj);
			cobj=nsp_setnum(conn->N, tobj, "timezone", n);
		}
		conn->dat->timezone=(int)cobj->val->d.num;
		/* theme */
		cobj=nsp_getobj(conn->N, tobj, "theme");
		if (cobj->val->type!=NT_STRING) cobj=nsp_setstr(conn->N, tobj, "theme", "default", strlen("default"));
		snprintf(conn->dat->theme, sizeof(conn->dat->theme)-1, "%s", cobj->val->d.str?cobj->val->d.str:"default");
		/* language */
		cobj=nsp_getobj(conn->N, tobj, "language");
		if (cobj->val->type!=NT_STRING) {
			p=nsp_getstr(proc->N, confobj, "default_language");
			cobj=nsp_setstr(conn->N, tobj, "language", p, strlen(p));
		}
		snprintf(conn->dat->language, sizeof(conn->dat->language)-1, "%s", cobj->val->d.str?cobj->val->d.str:"en");
	}
	if (settoken) {
		t=time(NULL)+86400;
		memset(timebuffer, 0, sizeof(timebuffer));
		strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		tobj=nsp_getobj(conn->N, &conn->N->g, "_HEADER");
		if (tobj->val->type==NT_TABLE) {
			snprintf(CookieToken, sizeof(CookieToken)-1, "gstoken=%s; expires=%s; path=/", conn->dat->token, timebuffer);
			nsp_setstr(conn->N, tobj, "SET_COOKIE", CookieToken, strlen(CookieToken));
		}
	}
	return 0;
}

char *auth_setpass(CONN *conn, char *rpassword)
{
	char itoa64[]="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char salt[10];
	char *cpassword=getbuffer(conn);
	int i;

	memset(salt, 0, sizeof(salt));
	srand(time(NULL));
	for (i=0;i<8;i++) salt[i]=itoa64[(rand()%64)];
	md5_crypt(cpassword, rpassword, salt);
	return cpassword;
}

int auth_getcookie(CONN *conn)
{
	obj_t *confobj=nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *ctobj=nsp_getobj(conn->N, &conn->N->g, "_COOKIE");

	snprintf(conn->dat->language, sizeof(conn->dat->language)-1, nsp_getstr(proc->N, confobj, "default_language"));
	snprintf(conn->dat->theme, sizeof(conn->dat->theme)-1, "default");
	snprintf(conn->dat->token, sizeof(conn->dat->token)-1, "%s", nsp_getstr(conn->N, ctobj, "gstoken"));
	return auth_renewcookie(conn, 1);
}

int auth_setcookie(CONN *conn)
{
	obj_t *confobj=nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *hobj=nsp_settable(proc->N, confobj, "httpd");
	obj_t *qobj=NULL;
	MD5_CONTEXT c;
	unsigned char md[MD5_SIZE];
	char *ptemp;
	char timebuffer[32];
	char password[64];
	char domain[64];
	struct timeval ttime;
	int i;
	int limit;
	int result;
	short int xml=0;

	memset(password, 0, sizeof(password));
	gettimeofday(&ttime, NULL);
	time_unix2sql(timebuffer, sizeof(timebuffer)-1, ttime.tv_sec);
	if (strcmp(nsp_getstr(conn->N, nsp_settable(conn->N, &conn->N->g, "_SERVER"), "REQUEST_METHOD"), "POST")!=0) {
		/* it's a GET */
		if ((ptemp=getgetenv(conn, "USERNAME"))!=NULL) strncpy(conn->dat->username, ptemp, sizeof(conn->dat->username)-1);
		if ((ptemp=getgetenv(conn, "PASSWORD"))!=NULL) strncpy(password, ptemp, sizeof(password)-1);
		if ((ptemp=getgetenv(conn, "DOMAIN"))!=NULL) conn->dat->did=domain_getid(ptemp);
	} else {
		/* either POST or XML */
		if ((ptemp=getxmlparam(conn, 1, "string"))==NULL) ptemp=getpostenv(conn, "USERNAME"); else xml++;
		if (ptemp) strncpy(conn->dat->username, ptemp, sizeof(conn->dat->username)-1);
		if ((ptemp=getxmlparam(conn, 2, "string"))==NULL) ptemp=getpostenv(conn, "PASSWORD"); else xml++;
		if (ptemp) strncpy(password, ptemp, sizeof(password)-1);
		if ((ptemp=getxmlparam(conn, 3, "string"))==NULL) ptemp=getpostenv(conn, "DOMAIN"); else xml++;
		conn->dat->did=domain_getid(ptemp);
	}
	if (conn->dat->did<0) conn->dat->did=1;
	ptemp=domain_getname(domain, sizeof(domain)-1, conn->dat->did);
	strncpy(conn->dat->domainname, ptemp?ptemp:"NULL", sizeof(conn->dat->domainname)-1);
	result=auth_checkpass(conn, password);
	if (result==0) {
		md5_init(&c);
		md5_update(&c, (unsigned char *)conn->dat->username, strlen(conn->dat->username));
		if (strcmp(conn->dat->username, "guest")!=0) {
			md5_update(&c, (unsigned char *)timebuffer, strlen(timebuffer));
		}
		md5_final(&(md[0]),&c);
		memset(conn->dat->token, 0, sizeof(conn->dat->token));
		for (i=0;i<MD5_SIZE;i++) strncatf(conn->dat->token, sizeof(conn->dat->token)-strlen(conn->dat->token)-1, "%02x", md[i]);
		memset(password, 0, sizeof(password));
		if (xml) return auth_renewcookie(conn, 0);
		if (sql_queryf(proc->N, &qobj, "SELECT id FROM nullsd_sessions WHERE uid = %d AND did = %d ORDER BY mtime DESC", conn->dat->uid, conn->dat->did)<0) return -1;
		limit=(int)nsp_getnum(proc->N, hobj, "session_limit");
		for (i=limit-1;i<sql_numtuples(proc->N, &qobj);i++) {
			if (i<0) continue;
			sql_updatef(proc->N, "DELETE FROM nullsd_sessions WHERE id = %d AND uid = %d AND did = %d", atoi(sql_getvalue(proc->N, &qobj, i, 0)), conn->dat->uid, conn->dat->did);
		}
		sql_freeresult(proc->N, &qobj);
		sql_updatef(proc->N, "INSERT INTO nullsd_sessions (ctime, mtime, uid, did, token, remoteaddr, data) VALUES ('%s', '%s', %d, %d, '%s', '%s', '')", timebuffer, timebuffer, conn->dat->uid, conn->dat->did, conn->dat->token, conn->socket.RemoteAddr);
		return auth_renewcookie(conn, 1);
	} else {
		ptemp=nsp_getstr(conn->N, nsp_getobj(conn->N, &conn->N->g, "_SERVER"), "SERVER_NAME");
		conn->dat->did=domain_getid(ptemp);
//		prints(conn, "[%s][%d]\r\n", ptemp, conn->dat->did);
	}
	return -1;
}

void auth_logout(CONN *conn)
{
	obj_t *tobj;
	char CookieToken[128];

	sql_updatef(proc->N, "DELETE FROM nullsd_sessions WHERE token = '%s' AND uid = %d AND did = %d", conn->dat->token, conn->dat->uid, conn->dat->did);
	tobj=nsp_getobj(conn->N, &conn->N->g, "_HEADER");
	if (tobj->val->type==NT_TABLE) {
		snprintf(CookieToken, sizeof(CookieToken)-1, "gstoken=NULL; path=/");
		nsp_setstr(conn->N, tobj, "SET_COOKIE", CookieToken, strlen(CookieToken));
	}
	htpage_logout(conn);
}

int auth_priv(CONN *conn, char *service)
{
	obj_t *uobj=nsp_getobj(conn->N, &conn->N->g, "_USER");
	obj_t *tobj;
	int authlevel;

	if (strlen(service)<4) return 0;
	if (uobj->val->type!=NT_TABLE) return 0;
	tobj=nsp_getobj(conn->N, uobj, "auth");
	if (tobj->val->type!=NT_TABLE) return 0;
	tobj=nsp_getobj(conn->N, tobj, service);
	if (tobj->val->type==NT_NULL) return 0;
	authlevel=(int)nsp_tonum(conn->N, tobj);
	if (authlevel&A_ADMIN) authlevel=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	return authlevel;
}
