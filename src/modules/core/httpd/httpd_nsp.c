/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

#ifdef WIN32
#define pthread_self() GetCurrentThreadId()
#endif
static CONN *get_conn()
{
	int i, maxconn;

	if (htproc.conn != NULL) {
		maxconn = (int)nsp_getnum(proc->N, nsp_getobj(proc->N, nsp_getobj(proc->N, nsp_getobj(proc->N, &proc->N->g, "CONFIG"), "modules"), "httpd"), "max_connections");
		for (i = 0;i < maxconn;i++) {
			if (htproc.conn[i].id == pthread_self()) return &htproc.conn[i];
		}
	}
	return NULL;
}

NSP_FUNCTION(htnsp_dl_load)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj, *tobj;
	NSP_CFUNC cfunc;
#ifdef WIN32
	HINSTANCE l;
	char *ext = "dll";
#else
	void *l;
	char *ext = "so";
#endif
	char namebuf[512];

	if (!nsp_isstr(cobj1) || cobj1->val->size < 1) {
		nsp_setbool(N, &N->r, "", 0);
		return 0;
	}
	tobj = nsp_getobj(N, nsp_getobj(N, &N->g, "dl"), "path");
	if (!nsp_istable(tobj)) {
		nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", "dl.path not found", -1);
		nsp_setbool(N, &N->r, "", 0);
		return 0;
	}
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) {
		if (!nsp_isstr(cobj)) continue;
		memset(namebuf, 0, sizeof(namebuf));
		snprintf(namebuf, sizeof(namebuf) - 1, "%s/libnsp_%s.%s", cobj->val->d.str, cobj1->val->d.str, ext);
		lib_error();
		if ((l = lib_open(namebuf)) != NULL) {
			lib_error();
			if ((cfunc = (NSP_CFUNC)lib_sym(l, "nsplib_init")) != NULL) {
				cfunc(N);
				nsp_setbool(N, &N->r, "", 1);
				return 0;
			}
			else {
				nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", lib_error(), -1);
				nsp_setbool(N, &N->r, "", 0);
				lib_close(l);
				return 0;
			}
		}
	}
	nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", lib_error(), -1);
	nsp_setbool(N, &N->r, "", 0);
	return 0;
}

int htnsp_flush(nsp_state *N)
{
	CONN *conn = get_conn();
	int b;
	int o = 0;

	do {
		conn->socket.atime = time(NULL);
		b = sizeof(conn->dat->replybuf) - conn->dat->replybuflen - 1;
		if (b < 512) { flushbuffer(conn); continue; }
		if (b > N->outbuflen) b = N->outbuflen;
		memcpy(conn->dat->replybuf + conn->dat->replybuflen, N->outbuffer + o, b);
		conn->dat->replybuflen += b;
		conn->dat->replybuf[conn->dat->replybuflen] = '\0';
		conn->dat->out_bytecount += b;
		N->outbuflen -= b;
		o += b;
	} while (N->outbuflen);
	if (conn->dat->replybuflen) flushbuffer(conn);
	return 0;
}

static NSP_FUNCTION(htnsp_lang_gets)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");

	nsp_setstr(N, &N->r, "", lang_gets(conn, nsp_tostr(N, cobj1), nsp_tostr(N, cobj2)), -1);
	return 0;
}

static NSP_FUNCTION(htnsp_auth_md5pass)
{
	//	CONN *conn=get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	char itoa64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char salt[10];
	char pass[40];
	int i;

	if (!nsp_isstr(cobj1) || cobj1->val->size < 1) {
		nsp_setbool(N, &N->r, "", 0);
		return 0;
	}
	memset(salt, 0, sizeof(salt));
	memset(pass, 0, sizeof(pass));
	srand(time(NULL));
	for (i = 0;i < 8;i++) salt[i] = itoa64[(rand() % 64)];
	md5_crypt(pass, cobj1->val->d.str, salt);
	nsp_setstr(N, &N->r, "", pass, -1);
	return 0;
}

static NSP_FUNCTION(htnsp_sqlquery)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *qobj = NULL;
	int rc;

	if (cobj1->val->type != NT_STRING) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	rc = sql_query(proc->N, &qobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, qobj);
	nsp_unlinkval(N, qobj);
	free(qobj);
	return rc;
}

static NSP_FUNCTION(htnsp_sqlupdate)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	int rc;

	if (cobj1->val->type != NT_STRING) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	rc = sql_update(proc->N, cobj1->val->d.str);
	return rc;
}

static NSP_FUNCTION(htnsp_sqlgetsequence)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	int rc = -1;

	if (cobj1->val->type != NT_STRING) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	rc = sql_getsequence(proc->N, cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	return rc;
}

static NSP_FUNCTION(htnsp_dirlist)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *tobj = NULL;
	int rc;

	if (cobj1->val->type != NT_STRING) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	rc = dir_list(N, &tobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, tobj);
	nsp_unlinkval(N, tobj);
	free(tobj);
	return rc;
}

static NSP_FUNCTION(htnsp_meminfo)
{
	obj_t tobj;

	//	tobj.val=n_newval(N, NT_TABLE);
	//	tobj.val->attr&=~NST_AUTOSORT;
	memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr &= ~NST_AUTOSORT;

	nsp_setnum(N, &tobj, "allocs", N->allocs);
	nsp_setnum(N, &tobj, "allocmem", N->allocmem);
	nsp_setnum(N, &tobj, "frees", N->frees);
	nsp_setnum(N, &tobj, "freemem", N->freemem);
	nsp_setnum(N, &tobj, "peakmem", N->peakmem);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
}

/* HTML CODE CONVERSION TABLE */
typedef struct {
	char symbol;
	char *code;
	short len;
} _htmltags;
static _htmltags htmltags[] = {
	{  '"', "&quot;", 6 },
	{  '&', "&amp;",  5 },
	{  '<', "&lt;",   4 },
	{  '>', "&gt;",   4 }
};

static NSP_FUNCTION(htnsp_str2html)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *robj;
	char *p, *p2;
	int i;
	unsigned int len;

	if (nsp_isnull(cobj1)) {
		nsp_setstr(N, &N->r, "", NULL, 0);
		return 0;
	}
	else if (cobj1->val->type != NT_STRING) {
		nsp_setstr(N, &N->r, "", nsp_tostr(N, cobj1), -1);
		return 0;
	}
	p = nsp_tostr(N, cobj1);
	len = 0;
	while (*p) {
		for (i = 0;i < 4;i++) {
			if (*p == htmltags[i].symbol) {
				len += htmltags[i].len;
				p++;
				goto match1;
			}
		}
		len++;
		p++;
	match1:
		continue;
	}
	if (len <= cobj1->val->size) {
		/*
		 * len less than object size?  it _is_ possible if the string
		 * isn't text, and has a '\0'.
		 */
		nsp_setstr(N, &N->r, "", cobj1->val->d.str, len);
		return 0;
	}
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	robj->val->d.str = malloc(len + 1);
	p = nsp_tostr(N, cobj1);
	p2 = robj->val->d.str;
	while (*p) {
		for (i = 0;i < 4;i++) {
			if (*p == htmltags[i].symbol) {
				strcpy(p2, htmltags[i].code);
				p2 += htmltags[i].len;
				p++;
				goto match2;
			}
		}
		*p2++ = *p++;
	match2:
		continue;
	}
	robj->val->d.str[len] = 0;
	return 0;
}

NSP_FUNCTION(htnsp_include_template)
{
	CONN *conn = get_conn();
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	uchar *p;
	struct stat sb;
	int n = -1;
	char fname[512];

	if (!nsp_isstr(cobj1)) {
		nsp_setnum(N, &N->r, "", n);
		return n;
	}
	snprintf(fname, sizeof(fname) - 1, "%s/domains/%04d/scripts/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did, nsp_tostr(N, cobj1));
	fixslashes(fname);
	if (stat(fname, &sb) == 0) {
		p = N->readptr;
		n = nsp_execfile(N, fname);
		N->readptr = p;
	}
	if (n) {
		snprintf(fname, sizeof(fname) - 1, "%s/scripts/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), nsp_tostr(N, cobj1));
		fixslashes(fname);
		if (stat(fname, &sb) == 0) {
			p = N->readptr;
			n = nsp_execfile(N, fname);
			N->readptr = p;
		}
	}
	if (n) {
		snprintf(fname, sizeof(fname) - 1, "%s/scripts/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), nsp_tostr(N, cobj1));
		fixslashes(fname);
		if (stat(fname, &sb) == 0) {
			p = N->readptr;
			n = nsp_execfile(N, fname);
			N->readptr = p;
		}
	}
	nsp_setnum(N, &N->r, "", n);
	return n;
}

static int htnsp_system(nsp_state *N)
{
	CONN *conn = get_conn();
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	char tempname[512];
	char line[512];
	FILE *fp;

	if (!nsp_isstr(cobj1)) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	snprintf(tempname, sizeof(tempname) - 1, "%s/tmp/exec-%d.tmp", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), (int)(time(NULL) % 999));
	fixslashes(tempname);
#ifdef WIN32
	snprintf(line, sizeof(line) - 1, "\"%s\" > \"%s\"", cobj1->val->d.str, tempname);
#else
	snprintf(line, sizeof(line) - 1, "%s > %s 2>&1", cobj1->val->d.str, tempname);
#endif
	htnsp_flush(conn->N);
	flushbuffer(conn);
	sys_system(line);
	if ((fp = fopen(tempname, "r")) != NULL) {
		while (fgets(line, sizeof(line) - 1, fp) != NULL) {
			prints(conn, "%s", line);
		}
		fclose(fp);
	}
	unlink(tempname);
	return 0;
}

NSP_FUNCTION(htnsp_convertnsp)
{
	CONN *conn = get_conn();
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	char *ss, *se;

	if (!nsp_isstr(cobj1)) {
		prints(conn, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	if (cobj1->val->d.str == NULL) return 0;
	nsp_setstr(N, &N->r, "", "print(\"", 7);
	se = ss = cobj1->val->d.str;
	for (;*se;se++) {
		if (strncmp(se, "?>", 2) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "print(\"", 7);
			ss = se += 2;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "\"", 1) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\\\"", 2);
			ss = se += 1;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "\\", 1) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\\\\", 2);
			ss = se += 1;
			if (*se) { --se; continue; }
			break;
		}
		else if (strncmp(se, "<?nsp", 5) == 0) {
			nsp_strcat(N, &N->r, ss, se - ss);
			nsp_strcat(N, &N->r, "\");", 3);
			ss = se += 5;
			while (*se && (se[0] != '?' || se[1] != '>')) ++se;
			if (*se) { --se; continue; }
			break;
		}
	}
	if (se > ss) nsp_strcat(N, &N->r, ss, se - ss);
	nsp_strcat(N, &N->r, "\");", 3);
	return 0;
}

static void preppath(nsp_state *N, char *name)
{
	char buf[512];
	char *p;
	unsigned int j;

	p = name;
	if ((name[0] == '/') || (name[0] == '\\') || (name[1] == ':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf) - 1);
	}
	else if (name[0] == '.') {
		/* looks relative... */
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	else {
		getcwd(buf, sizeof(buf) - strlen(name) - 2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	for (j = 0;j < strlen(buf);j++) {
		if (buf[j] == '\\') buf[j] = '/';
	}
	for (j = strlen(buf) - 1;j > 0;j--) {
		if (buf[j] == '/') { buf[j] = '\0'; p = buf + j + 1; break; }
	}
	nsp_setstr(N, &N->g, "_filename", p, strlen(p));
	nsp_setstr(N, &N->g, "_filepath", buf, strlen(buf));
	return;
}

static int htnsp_initenv(CONN *conn)
{
	char libbuf[80];
	obj_t *tobj, *tobj2;

	nsp_setcfunc(conn->N, &conn->N->g, "convertnsp", (NSP_CFUNC)htnsp_convertnsp);
	nsp_setcfunc(conn->N, &conn->N->g, "meminfo", (NSP_CFUNC)htnsp_meminfo);
	nsp_setcfunc(conn->N, &conn->N->g, "dirlist", (NSP_CFUNC)htnsp_dirlist);
	nsp_setcfunc(conn->N, &conn->N->g, "include_template", (NSP_CFUNC)htnsp_include_template);
	nsp_setcfunc(conn->N, &conn->N->g, "lang_gets", (NSP_CFUNC)htnsp_lang_gets);
	nsp_setcfunc(conn->N, &conn->N->g, "str2html", (NSP_CFUNC)htnsp_str2html);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlquery", (NSP_CFUNC)htnsp_sqlquery);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlupdate", (NSP_CFUNC)htnsp_sqlupdate);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlgetsequence", (NSP_CFUNC)htnsp_sqlgetsequence);
	nsp_setcfunc(conn->N, &conn->N->g, "system", (NSP_CFUNC)htnsp_system);
	tobj = nsp_settable(conn->N, &conn->N->g, "io");
	nsp_setcfunc(conn->N, tobj, "flush", (NSP_CFUNC)htnsp_flush);
	tobj = nsp_settable(conn->N, &conn->N->g, "dl");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(conn->N, tobj, "load", (NSP_CFUNC)htnsp_dl_load);
	tobj2 = nsp_settable(conn->N, tobj, "path");

	nsp_setcfunc(conn->N, &conn->N->g, "auth_md5pass", (NSP_CFUNC)htnsp_auth_md5pass);

#ifdef WIN32
	//	nsp_setstr(conn->N, tobj2, "0", "C:\\nullsd\\lib\\shared", -1);
	//	GetSystemWindowsDirectory(libbuf, sizeof(libbuf));
	//	GetEnvironmentVariable("SystemRoot", libbuf, sizeof(libbuf));
	GetWindowsDirectory(libbuf, sizeof(libbuf));
	_snprintf(libbuf + strlen(libbuf), sizeof(libbuf) - strlen(libbuf) - 1, "\\NSP");
	nsp_setstr(conn->N, tobj2, "0", libbuf, -1);
#else
	//nsp_setstr(conn->N, tobj2, "0", "/usr/lib/nsp", -1);
	snprintf(libbuf, sizeof(libbuf) - 1, "%s/nsp", nsp_getstr(proc->N, nsp_settable(proc->N, nsp_getobj(proc->N, &proc->N->g, "CONFIG"), "paths"), "lib"));
	nsp_setstr(conn->N, tobj2, "0", libbuf, -1);
#endif
	//	tobj=nsp_settable(conn->N, &conn->N->g, "ldir");
	//	tobj->val->attr|=NST_HIDDEN;
	//	nsp_setcfunc(conn->N, tobj,       "deleteentry",      (NSP_CFUNC)htnsp_ldir_deleteentry);
	//	nsp_setcfunc(conn->N, tobj,       "getentry",         (NSP_CFUNC)htnsp_ldir_getentry);
	//	nsp_setcfunc(conn->N, tobj,       "getlist",          (NSP_CFUNC)htnsp_ldir_getlist);
	//	nsp_setcfunc(conn->N, tobj,       "sortlist",         (NSP_CFUNC)htnsp_ldir_sortlist);
	//	nsp_setcfunc(conn->N, tobj,       "saveentry",        (NSP_CFUNC)htnsp_ldir_saveentry);
	return 0;
}

int htnsp_runinit(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	char filename[512];
	//char hackbuf[512];
	struct stat sb;

	snprintf(filename, sizeof(filename) - 1, "%s/scripts/http/init.ns", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"));
	fixslashes(filename);
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_runinit: filename=[%s]", filename);
	if (stat(filename, &sb) != 0) return 0;
	htnsp_initenv(conn);


	//snprintf(hackbuf, sizeof(hackbuf) - 1, "if (typeof(x=file.readall(\"%s\"))=='string') exec(x);", filename);
	//nsp_exec(conn->N, hackbuf);
	int n = nsp_execfile(conn->N, filename);
	if (n > 0) log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_runinit: n=%d, filename=[%s]", n, filename);


	if (conn->N->err) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_runinit exception: [%s]", conn->N->errbuf);
	}
	return conn->N->err;
}

static int htnsp_runscript(CONN *conn, char *file)
{
	char *p, *p2;

	htnsp_initenv(conn);
	preppath(conn->N, file);
	p = file;
	if ((p2 = strrchr(file, '/')) != NULL) p = p2 + 1;
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 2, "htnsp_runscript [%s]", p);
	p = strrchr(file, '.');
	if (p != NULL&&strcmp(p, ".nsp") == 0) {
		char hackbuf[512];

		snprintf(hackbuf, sizeof(hackbuf) - 1, "exec(convertnsp(file.readall(\"%s\")));", file);
		//snprintf(hackbuf, sizeof(hackbuf) - 1, "convertnsp(file.readall(\"%s\"));", file);
		nsp_exec(conn->N, hackbuf);
	}
	else {
		nsp_execfile(conn->N, file);
	}
	htnsp_flush(conn->N);
	if (conn->N->err) {
		p = file;
		if ((p2 = strrchr(file, '/')) != NULL) p = p2 + 1;
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_runscript exception: [%s][%s]", p, conn->N->errbuf);
	}
	if (conn->N->err) {
		prints(conn, "<HR><B>[Exception: %s]</B>\r\n", conn->N->errbuf);
		conn->N->err = 0;
		htnsp_dotemplate(conn, "html", "debug.ns");
	}
	return 0;
}

int htnsp_doscript(CONN *conn)
{
	obj_t *confobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	obj_t *htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	struct stat sb;
	char docroot[512];
	char filename[512];
	char *PathInfo = nsp_getstr(conn->N, htobj, "PATH_INFO");
	char *p;

	/* set the domain docroot */
	snprintf(docroot, sizeof(docroot) - 1, "%s/domains/%04d/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	nsp_setstr(conn->N, htobj, "DOCUMENT_ROOT", docroot, strlen(docroot));
	/* try the domain docroot first */
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename) - 1, "%s%s", docroot, PathInfo);
	fixslashes(filename);
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_doscript: filename=[%s]", filename);
	if (filename[0] != '\0') {
		p = filename + strlen(filename) - 1;
		if ((*p == '\\') || (*p == '/')) *p = '\0';
	}
	/* if it's not in the domain htdocs, try the shared */
	if (stat(filename, &sb) != 0) {
		memset(filename, 0, sizeof(filename));
		snprintf(filename, sizeof(filename) - 1, "%s/share/htdocs%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), PathInfo);
		fixslashes(filename);
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_doscript: filename=[%s]", filename);
		if (filename[0] != '\0') {
			p = filename + strlen(filename) - 1;
			if ((*p == '\\') || (*p == '/')) *p = '\0';
		}
	}
	/* last, try static lib htdocs */
	if (stat(filename, &sb) != 0) {
		memset(filename, 0, sizeof(filename));
		snprintf(filename, sizeof(filename) - 1, "%s/htdocs%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), PathInfo);
		fixslashes(filename);
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_doscript: filename=[%s]", filename);
		if (filename[0] != '\0') {
			p = filename + strlen(filename) - 1;
			if ((*p == '\\') || (*p == '/')) *p = '\0';
		}
	}
	if (stat(filename, &sb) != 0) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_doscript: [%s]", PathInfo);
		return -1;
	}
	if (sb.st_mode&S_IFDIR) {
		return htpage_dirlist(conn);
	}
	p = strrchr(PathInfo, '.');
	if (p == NULL) {
		return filesend(conn, filename);
	}
	if ((strcmp(p, ".ns") != 0) && (strcmp(p, ".nsp") != 0)) {
		return filesend(conn, filename);
	}
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	htnsp_runscript(conn, filename);
	return 0;
}

int htnsp_doscript_htdocs(CONN *conn, char *dir, char *file)
{
	obj_t *confobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	obj_t *htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	//obj_t *cobj;
	char docroot[512];
	char fullname[512];
	char dirname[512];
	char buf[512];
	struct stat sb;
	int rc;

	/* check domains */
//	cobj = nsp_getobj(conn->N, confobj, "private_htdocs_path");
//	if (!nsp_isstr(cobj)) goto tryshared;
//	snprintf(docroot, sizeof(docroot) - 1, "%s", nsp_tostr(conn->N, cobj));
	snprintf(docroot, sizeof(docroot) - 1, "%s/domains/%04d/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	nsp_setstr(conn->N, htobj, "DOCUMENT_ROOT", docroot, strlen(docroot));
	snprintf(dirname, sizeof(dirname) - 1, "%s/%s", docroot, dir);
	fixslashes(dirname);
	snprintf(fullname, sizeof(fullname) - 1, "%s/%s", dirname, file);
	decodeurl(fullname);
	fixslashes(fullname);
	//nsp_setstr(conn->N, htobj, "FULLNAME1", fullname, strlen(fullname));
	if (stat(fullname, &sb) == 0) {
		nsp_setstr(conn->N, htobj, "PWD", dirname, strlen(dirname));
		//nsp_setstr(conn->N, htobj, "SCRIPT_FILENAME", fullname, strlen(fullname));
		nsp_setstr(conn->N, htobj, "PATH_TRANSLATED", fullname, strlen(fullname));
		snprintf(buf, sizeof(buf) - 1, "/%s/%s", dir, file);
		//nsp_setstr(conn->N, htobj, "SCRIPT_NAME", buf, strlen(buf));
		send_header(conn, 0, 200, "1", "text/html", -1, -1);
		rc = htnsp_runscript(conn, fullname);
		return rc;
	}
	//tryshared:
		/* and now shared htdocs */
	//	cobj = nsp_getobj(conn->N, confobj, "shared_htdocs_path");
	//	if (!nsp_isstr(cobj)) return -1;
	//	snprintf(dirname, sizeof(dirname) - 1, "%s/%s", nsp_tostr(conn->N, cobj), dir);
	snprintf(dirname, sizeof(dirname) - 1, "%s/share/htdocs/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), dir);
	fixslashes(dirname);
	snprintf(fullname, sizeof(fullname) - 1, "%s/%s", dirname, file);
	decodeurl(fullname);
	fixslashes(fullname);
	//nsp_setstr(conn->N, htobj, "FULLNAME2", fullname, strlen(fullname));
	if (stat(fullname, &sb) == 0) {
		nsp_setstr(conn->N, htobj, "PWD", dirname, strlen(dirname));
		//nsp_setstr(conn->N, htobj, "SCRIPT_FILENAME", fullname, strlen(fullname));
		nsp_setstr(conn->N, htobj, "PATH_TRANSLATED", fullname, strlen(fullname));
		snprintf(buf, sizeof(buf) - 1, "/%s/%s", dir, file);
		//nsp_setstr(conn->N, htobj, "SCRIPT_NAME", buf, strlen(buf));
		send_header(conn, 0, 200, "1", "text/html", -1, -1);
		rc = htnsp_runscript(conn, fullname);
		return rc;
	}
	/* and now static lib shared htdocs */
//	cobj = nsp_getobj(conn->N, confobj, "shared_htdocs_path");
//	if (!nsp_isstr(cobj)) return -1;
//	snprintf(dirname, sizeof(dirname) - 1, "%s/%s", nsp_tostr(conn->N, cobj), dir);
	snprintf(dirname, sizeof(dirname) - 1, "%s/htdocs/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), dir);
	fixslashes(dirname);
	snprintf(fullname, sizeof(fullname) - 1, "%s/%s", dirname, file);
	decodeurl(fullname);
	fixslashes(fullname);
	//nsp_setstr(conn->N, htobj, "FULLNAME2", fullname, strlen(fullname));
	if (stat(fullname, &sb) == 0) {
		nsp_setstr(conn->N, htobj, "PWD", dirname, strlen(dirname));
		//nsp_setstr(conn->N, htobj, "SCRIPT_FILENAME", fullname, strlen(fullname));
		nsp_setstr(conn->N, htobj, "PATH_TRANSLATED", fullname, strlen(fullname));
		snprintf(buf, sizeof(buf) - 1, "/%s/%s", dir, file);
		//nsp_setstr(conn->N, htobj, "SCRIPT_NAME", buf, strlen(buf));
		send_header(conn, 0, 200, "1", "text/html", -1, -1);
		rc = htnsp_runscript(conn, fullname);
		return rc;
	}
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_doscript_htdocs: [%s]", "");
	return -1;
}

int htnsp_dotemplate(CONN *conn, char *dir, char *file)
{
	obj_t *confobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	obj_t *htobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	obj_t *tobj = nsp_settable(conn->N, &conn->N->g, "_TEMP");
	char docroot[512];
	char fullname[512];
	char buf[512];
	struct stat sb;
	char *p;
	int rc = -1;
	//char *PathInfo = nsp_getstr(conn->N, htobj, "PATH_INFO");

	snprintf(docroot, sizeof(docroot) - 1, "%s/domains/%04d/htdocs", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did);
	nsp_setstr(conn->N, htobj, "DOCUMENT_ROOT", docroot, -1);

	snprintf(fullname, sizeof(fullname) - 1, "%s/scripts/%s/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), dir, file);
	/* decodeurl(fullname); */
	fixslashes(fullname);
	if (stat(fullname, &sb) == 0) {
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_dotemplate: [%s]", fullname);
		snprintf(buf, sizeof(buf) - 1, "%s%s", docroot, nsp_getstr(conn->N, htobj, "REQUEST_URI"));
		if ((p = strchr(buf, '?')) != NULL) *p = '\0';
		nsp_setstr(conn->N, tobj, "PWD", buf, strlen(buf));
		rc = htnsp_runscript(conn, fullname);
		nsp_unlinkval(conn->N, tobj);
		return rc;
	}
	/* then check shared templates */
	snprintf(fullname, sizeof(fullname) - 1, "%s/scripts/%s/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), dir, file);
	/* decodeurl(fullname); */
	fixslashes(fullname);
	if (stat(fullname, &sb) == 0) {
		//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_dotemplate: [%s]", fullname);
		snprintf(buf, sizeof(buf) - 1, "%s%s", docroot, nsp_getstr(conn->N, htobj, "REQUEST_URI"));
		if ((p = strchr(buf, '?')) != NULL) *p = '\0';
		nsp_setstr(conn->N, tobj, "PWD", buf, strlen(buf));
		rc = htnsp_runscript(conn, fullname);
		nsp_unlinkval(conn->N, tobj);
		return rc;
	}
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htnsp_dotemplate: [%s]", PathInfo);
	return rc;
}
