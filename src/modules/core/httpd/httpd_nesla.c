/*
    NullLogic Groupware - Copyright (C) 2000-2007 Dan Cahill

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
static CONN *get_sid()
{
	int maxconn=(int)nes_getnum(proc->N, nes_getobj(proc->N, nes_getobj(proc->N, &proc->N->g, "CONFIG"), "httpd"), "max_connections");
	int i;

	if (htproc.conn==NULL) return NULL;
	for (i=0;i<maxconn;i++) {
		if (htproc.conn[i].id==pthread_self()) break;
	}
	if ((i<0)||(i>=maxconn)) return NULL;
	return &htproc.conn[i];
}

NES_FUNCTION(nesladl_loadlib)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
#ifdef WIN32
	HINSTANCE l;
#else
	void *l;
#endif
	NES_CFUNC cfunc;
	int rc=0;

	if ((cobj1->val->type==NT_STRING)&&(cobj1->val->size>0)) {
		lib_error();
		l=lib_open(cobj1->val->d.str);
		if (l==NULL) {
			prints(sid, "can't open lib '%s'<BR>%s<BR>\n", cobj1->val->d.str, lib_error());
			rc=-1;
		} else {
			lib_error();
			cfunc=(NES_CFUNC)lib_sym(l, "neslalib_init");
			if (cfunc==NULL) {
				prints(sid, "can't find entry point for lib<BR>\n");
				if (l!=NULL) lib_close(l);
				rc=-1;
			} else {
				rc=cfunc(N);
			}
		}
	} else {
		rc=-1;
	}
	nes_setnum(N, &N->r, "", rc);
	return 0;
}

int htnes_flush(nes_state *N)
{
	CONN *sid=get_sid();
	int len=N->outbuflen;
	int b, o, slen;
/*
	N->outbuf[N->outbuflen]='\0';
	len=prints(sid, "%s", N->outbuf);
	N->outbuflen=0;
*/
	o=0;
	do {
		sid->socket.atime=time(NULL);
		slen=strlen(sid->dat->replybuf);
		b=sizeof(sid->dat->replybuf)-slen-1;
		if (b<512) { flushbuffer(sid); continue; }
		if (b>N->outbuflen) b=N->outbuflen;
		memcpy(sid->dat->replybuf+slen, N->outbuf+o, b);
		sid->dat->replybuf[slen+b]='\0';
		sid->dat->out_bytecount+=b;
		slen+=b;
		o+=b;
	} while (o<N->outbuflen);
	N->outbuflen=0;
	return len;
}

static NES_FUNCTION(htnes_lang_gets)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);

	nes_setstr(N, &N->r, "", lang_gets(sid, nes_tostr(N, cobj1), nes_tostr(N, cobj2)), -1);
	return 0;
}

/* LDIR */

static NES_FUNCTION(htnes_ldir_deleteentry)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	int rc;

	if (cobj1->val->type!=NT_STRING) { prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	if (cobj2->val->type!=NT_NUMBER) { prints(sid, "%s() expected a number for arg2\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	rc=ldir_deleteentry(cobj1->val->d.str, (int)cobj2->val->d.num, sid->dat->did);
	nes_setnum(N, &N->r, "", rc);
	return 0;
}

static NES_FUNCTION(htnes_ldir_saveentry)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	int rc;

	if (cobj1->val->type!=NT_STRING) { prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	if (cobj2->val->type!=NT_NUMBER) { prints(sid, "%s() expected a number for arg2\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	if (cobj3->val->type!=NT_TABLE)  { prints(sid, "%s() expected a table for arg3\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	rc=ldir_saveentry(sid, (int)cobj2->val->d.num, cobj1->val->d.str, &cobj3);
	nes_setnum(N, &N->r, "", rc);
	return 0;
}

static NES_FUNCTION(htnes_ldir_getlist)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *qobj=NULL;

	if (cobj1->val->type!=NT_STRING) { prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	if (cobj2->val->type!=NT_NUMBER) { prints(sid, "%s() expected a number for arg2\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	qobj=ldir_getlist(proc->N, cobj1->val->d.str, (int)cobj2->val->d.num, sid->dat->did);
	nes_linkval(N, &N->r, qobj);
	nes_unlinkval(N, qobj);
	free(qobj);
	return 0;
}

static NES_FUNCTION(htnes_ldir_sortlist)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *cobj3=nes_getiobj(N, &N->l, 3);
	obj_t *cobj4=nes_getiobj(N, &N->l, 4);

	nes_unlinkval(N, &N->r);
//	ldir_sortlist(N, qobj1, "sn", "_data", 1);
	ldir_sortlist(N, cobj1, nes_tostr(N, cobj2), nes_isnull(cobj3)?NULL:nes_tostr(N, cobj3), (int)nes_tonum(N, cobj4));
//	ldir_sortlist(N, cobj1, nes_tostr(N, cobj2), nes_isnull(cobj3)?NULL:nes_tostr(N, cobj3), -1);
	return 0;
}

static NES_FUNCTION(htnes_ldir_getentry)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj2=nes_getiobj(N, &N->l, 2);
	obj_t *qobj=NULL;

//	nes_unlinkval(N, &N->r);
	if (cobj1->val->type!=NT_STRING) { prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	if (cobj2->val->type==NT_STRING) {
		qobj=ldir_getentry(proc->N, cobj1->val->d.str, cobj2->val->d.str, 0, sid->dat->did);
	} else if (cobj2->val->type==NT_NUMBER) {
		qobj=ldir_getentry(proc->N, cobj1->val->d.str, NULL, (int)cobj2->val->d.num, sid->dat->did);
	} else { prints(sid, "%s() expected a string or number for arg2\r\n", nes_getstr(N, &N->l, "0")); return 0; }
	
	nes_linkval(N, &N->r, qobj);
	nes_unlinkval(N, qobj);
	free(qobj);
	return 0;
}

static NES_FUNCTION(htnes_sqlquery)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *qobj=NULL;
	int rc;

	if (cobj1->val->type!=NT_STRING) {
		prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0"));
		return 0;
	}
	rc=sql_query(proc->N, &qobj, cobj1->val->d.str);
	nes_linkval(N, &N->r, qobj);
	nes_unlinkval(N, qobj);
	free(qobj);
	return rc;
}

static NES_FUNCTION(htnes_dirlist)
{
	CONN *sid=get_sid();
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *tobj=NULL;
	int rc;

	if (cobj1->val->type!=NT_STRING) {
		prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0"));
		return 0;
	}
	rc=dir_list(N, &tobj, cobj1->val->d.str);
	nes_linkval(N, &N->r, tobj);
	nes_unlinkval(N, tobj);
	free(tobj);
	return rc;
}

/* HTML CODE CONVERSION TABLE */
typedef struct {
	char symbol;
	char *code;
	short len;
} _htmltags;
static _htmltags htmltags[]={
	{  '"', "&quot;", 6 },
	{  '&', "&amp;",  5 },
	{  '<', "&lt;",   4 },
	{  '>', "&gt;",   4 }
};

static NES_FUNCTION(htnes_str2html)
{
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *robj;
	char *p, *p2;
	int i;
	int len;

	if (nes_isnull(cobj1)) {
		nes_setstr(N, &N->r, "", NULL, 0);
		return 0;
	} else if (cobj1->val->type!=NT_STRING) {
		nes_setstr(N, &N->r, "", nes_tostr(N, cobj1), -1);
		return 0;
	}
	p=nes_tostr(N, cobj1);
	len=0;
	while (*p) {
		for (i=0;i<4;i++) {
			if (*p==htmltags[i].symbol) {
				len+=htmltags[i].len;
				p++;
				goto match1;
			}
		}
		len++;
		p++;
match1:
		continue;
	}
	if (len<=cobj1->val->size) {
		/*
		 * len less than object size?  it _is_ possible if the string
		 * isn't text, and has a '\0'.
		 */
		nes_setstr(N, &N->r, "", cobj1->val->d.str, len);
		return 0;
	}
	robj=nes_setstr(N, &N->r, "", NULL, 0);
	robj->val->d.str=malloc(len+1);
	p=nes_tostr(N, cobj1);
	p2=robj->val->d.str;
	while (*p) {
		for (i=0;i<4;i++) {
			if (*p==htmltags[i].symbol) {
				strcpy(p2, htmltags[i].code);
				p2+=htmltags[i].len;
				p++;
				goto match2;
			}
		}
		*p2++=*p++;
match2:
		continue;
	}
	robj->val->d.str[len]=0;
	return 0;
}

NES_FUNCTION(htnes_include_template)
{
//	CONN *sid=get_sid();
/*
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	uchar *p;
	int n=0;

	if (!nes_isnull(cobj1)) {
		p=N->readptr;
		n=nes_execfile(N, (char *)cobj1->val->d.str);
		N->readptr=p;
	}
	nes_setnum(N, &N->r, "", n);
	return n;
*/
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	obj_t *cobj;
	uchar *p;
	struct stat sb;
	int n=-1;
	char fname[512];

	if (!nes_isstr(cobj1)) {
		nes_setnum(N, &N->r, "", n);
		return n;
	}
	cobj=nes_getobj(N, nes_settable(N, &N->g, "_CONFIG"), "private_template_path");
	if (nes_isstr(cobj)) {
		snprintf(fname, sizeof(fname)-1, "%s/%s", nes_tostr(N, cobj), nes_tostr(N, cobj1));
		fixslashes(fname);
		if (stat(fname, &sb)==0) {
			p=N->readptr;
			n=nes_execfile(N, fname);
			N->readptr=p;
		}
	}
//	prints(sid, "1[%s][%d]<BR>", fname, n);
	if (n) {
		cobj=nes_getobj(N, nes_settable(N, &N->g, "_CONFIG"), "shared_template_path");
		if (nes_isstr(cobj)) {
			snprintf(fname, sizeof(fname)-1, "%s/%s", nes_tostr(N, cobj), nes_tostr(N, cobj1));
			fixslashes(fname);
			if (stat(fname, &sb)==0) {
				p=N->readptr;
				n=nes_execfile(N, fname);
				N->readptr=p;
			}
		}
	}
//	prints(sid, "2[%s][%d]<BR>", fname, n);
	nes_setnum(N, &N->r, "", n);
	return n;
}

static int htnes_system(nes_state *N)
{
	CONN *sid=get_sid();
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj1=nes_getiobj(N, &N->l, 1);
	char tempname[255];
	char line[512];
	short int err;
	FILE *fp;

	if (cobj1->val->type!=NT_STRING) {
		prints(sid, "%s() expected a string for arg1\r\n", nes_getstr(N, &N->l, "0"));
		return 0;
	}
	snprintf(tempname, sizeof(tempname)-1, "%s/exec-%d.tmp", nes_getstr(proc->N, confobj, "var_tmp_path"), (int)(time(NULL)%999));
	fixslashes(tempname);
#ifdef WIN32
	snprintf(line, sizeof(line)-1, "\"%s\" > \"%s\"", cobj1->val->d.str, tempname);
#else
	snprintf(line, sizeof(line)-1, "%s > %s 2>&1", cobj1->val->d.str, tempname);
#endif
	htnes_flush(sid->N);
	flushbuffer(sid);
	err=sys_system(line);
	if ((fp=fopen(tempname, "r"))!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			prints(sid, "%s", line);
		}
		fclose(fp);
	}
	unlink(tempname);
	return 0;
}

static void preppath(nes_state *N, char *name)
{
	char buf[512];
	char *p;
	unsigned int j;

	p=name;
	if ((name[0]=='/')||(name[0]=='\\')||(name[1]==':')) {
		/* it's an absolute path.... probably... */
		strncpy(buf, name, sizeof(buf)-1);
	} else if (name[0]=='.') {
		/* looks relative... */
		getcwd(buf, sizeof(buf)-strlen(name)-2);
		strcat(buf, "/");
		strcat(buf, name);
	} else {
		getcwd(buf, sizeof(buf)-strlen(name)-2);
		strcat(buf, "/");
		strcat(buf, name);
	}
	for (j=0;j<strlen(buf);j++) {
		if (buf[j]=='\\') buf[j]='/';
	}
	for (j=strlen(buf)-1;j>0;j--) {
		if (buf[j]=='/') { buf[j]='\0'; p=buf+j+1; break; }
	}
	nes_setstr(N, &N->g, "_filename", p, strlen(p));
	nes_setstr(N, &N->g, "_filepath", buf, strlen(buf));
	return;
}

static int htnes_runscript(CONN *sid, char *file)
{
	obj_t *tobj=nes_getobj(sid->N, &sid->N->g, "io");
//	jmp_buf *savjmp;

	nes_setcfunc(sid->N, &sid->N->g, "dirlist",          (NES_CFUNC)htnes_dirlist);
	nes_setcfunc(sid->N, &sid->N->g, "include_template", (NES_CFUNC)htnes_include_template);
	nes_setcfunc(sid->N, &sid->N->g, "lang_gets",        (NES_CFUNC)htnes_lang_gets);
	nes_setcfunc(sid->N, &sid->N->g, "str2html",         (NES_CFUNC)htnes_str2html);
	nes_setcfunc(sid->N, &sid->N->g, "sqlquery",         (NES_CFUNC)htnes_sqlquery);
	nes_setcfunc(sid->N, &sid->N->g, "system",           (NES_CFUNC)htnes_system);
	nes_setcfunc(sid->N, tobj,       "flush",            (NES_CFUNC)htnes_flush);

	tobj=nes_settable(sid->N, &sid->N->g, "dl");
	tobj->val->attr|=NST_HIDDEN;
	nes_setcfunc(sid->N, tobj,       "loadlib",          (NES_CFUNC)nesladl_loadlib);

	tobj=nes_settable(sid->N, &sid->N->g, "ldir");
	tobj->val->attr|=NST_HIDDEN;
	nes_setcfunc(sid->N, tobj,       "deleteentry",      (NES_CFUNC)htnes_ldir_deleteentry);
	nes_setcfunc(sid->N, tobj,       "getentry",         (NES_CFUNC)htnes_ldir_getentry);
	nes_setcfunc(sid->N, tobj,       "getlist",          (NES_CFUNC)htnes_ldir_getlist);
	nes_setcfunc(sid->N, tobj,       "sortlist",         (NES_CFUNC)htnes_ldir_sortlist);
	nes_setcfunc(sid->N, tobj,       "saveentry",        (NES_CFUNC)htnes_ldir_saveentry);

	preppath(sid->N, file);
//	savjmp=sid->N->savjmp;
//	sid->N->savjmp=calloc(1, sizeof(jmp_buf));
//	if (setjmp(*sid->N->savjmp)==0) {
//		sid->N->jmpset=1;
		nes_execfile(sid->N, file);
		htnes_flush(sid->N);
//	}
//	sid->N->jmpset=0;
//	free(sid->N->savjmp);
//	sid->N->savjmp=savjmp;
	if (sid->N->err) {
		prints(sid, "<HR><B>[errno=%d :: %s]</B>\r\n", sid->N->err, sid->N->errbuf);
		sid->N->err=0;
		htnes_dotemplate(sid, "html", "debug.ns");
	}
	return 0;
}

int htnes_doscript(CONN *sid)
{
	obj_t *confobj=nes_settable(sid->N, &sid->N->g, "_CONFIG");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *cobj;
	struct stat sb;
	char docroot[512];
	char filename[512];
	char *PathInfo=nes_getstr(sid->N, htobj, "PATH_INFO");
	char *p;

	cobj=nes_getobj(sid->N, confobj, "private_htdocs_path");
	if (!nes_isstr(cobj)) goto tryshared;
	snprintf(docroot, sizeof(docroot)-1, "%s", nes_tostr(sid->N, cobj));
	nes_setstr(sid->N, htobj, "DOCUMENT_ROOT", docroot, strlen(docroot));
	/* if (strncmp(sid->dat->in_CGIScriptName, "/nesla/", 7)!=0) { return -1; } */
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename)-1, "%s%s", docroot, PathInfo);
	fixslashes(filename);
	if (filename[0]!='\0') {
		p=filename+strlen(filename)-1;
		if ((*p=='\\')||(*p=='/')) *p='\0';
	}
	/* if it's not in the domain htdocs, try the shared */
	if (stat(filename, &sb)!=0) {
tryshared:
		cobj=nes_getobj(sid->N, confobj, "shared_htdocs_path");
		if (!nes_isstr(cobj)) { return -1; }
		memset(filename, 0, sizeof(filename));
		snprintf(filename, sizeof(filename)-1, "%s%s", nes_tostr(sid->N, cobj), PathInfo);
		fixslashes(filename);
		if (filename[0]!='\0') {
			p=filename+strlen(filename)-1;
			if ((*p=='\\')||(*p=='/')) *p='\0';
		}
		if (stat(filename, &sb)!=0) { return -1; }
	}
	if (sb.st_mode&S_IFDIR) {
		return htpage_dirlist(sid);
	}
	p=strrchr(PathInfo, '.');
	if (p==NULL) {
		return filesend(sid, filename);
	}
	if ((strcmp(p, ".ns")!=0)&&(strcmp(p, ".nes")!=0)) {
		return filesend(sid, filename);
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htnes_runscript(sid, filename);
	return 0;
}

int htnes_doscript_htdocs(CONN *sid, char *dir, char *file)
{
	obj_t *confobj=nes_settable(sid->N, &sid->N->g, "_CONFIG");
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *cobj;
	char docroot[512];
	char fullname[512];
	char dirname[512];
	char buf[512];
	struct stat sb;
	int rc;

	/* check domains */
	cobj=nes_getobj(sid->N, confobj, "private_htdocs_path");
	if (!nes_isstr(cobj)) goto tryshared;
	snprintf(docroot, sizeof(docroot)-1, "%s", nes_tostr(sid->N, cobj));
	nes_setstr(sid->N, htobj, "DOCUMENT_ROOT", docroot, strlen(docroot));
	snprintf(dirname, sizeof(dirname)-1, "%s/%s", docroot, dir);
	fixslashes(dirname);
	snprintf(fullname, sizeof(fullname)-1, "%s/%s", dirname, file);
	decodeurl(fullname);
	fixslashes(fullname);
//	nes_setstr(sid->N, htobj, "FULLNAME1", fullname, strlen(fullname));
	if (stat(fullname, &sb)==0) {
		nes_setstr(sid->N, htobj, "PWD", dirname, strlen(dirname));
//		nes_setstr(sid->N, htobj, "SCRIPT_FILENAME", fullname, strlen(fullname));
		nes_setstr(sid->N, htobj, "PATH_TRANSLATED", fullname, strlen(fullname));
		snprintf(buf, sizeof(buf)-1, "/%s/%s", dir, file);
//		nes_setstr(sid->N, htobj, "SCRIPT_NAME", buf, strlen(buf));
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		rc=htnes_runscript(sid, fullname);
		return rc;
	}
tryshared:
	/* and now shared htdocs */
	cobj=nes_getobj(sid->N, confobj, "shared_htdocs_path");
	if (!nes_isstr(cobj)) return -1;
	snprintf(dirname, sizeof(dirname)-1, "%s/%s", nes_tostr(sid->N, cobj), dir);
	fixslashes(dirname);
	snprintf(fullname, sizeof(fullname)-1, "%s/%s", dirname, file);
	decodeurl(fullname);
	fixslashes(fullname);
//	nes_setstr(sid->N, htobj, "FULLNAME2", fullname, strlen(fullname));
	if (stat(fullname, &sb)==0) {
		nes_setstr(sid->N, htobj, "PWD", dirname, strlen(dirname));
//		nes_setstr(sid->N, htobj, "SCRIPT_FILENAME", fullname, strlen(fullname));
		nes_setstr(sid->N, htobj, "PATH_TRANSLATED", fullname, strlen(fullname));
		snprintf(buf, sizeof(buf)-1, "/%s/%s", dir, file);
//		nes_setstr(sid->N, htobj, "SCRIPT_NAME", buf, strlen(buf));
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		rc=htnes_runscript(sid, fullname);
		return rc;
	}
	return -1;
}

int htnes_dotemplate(CONN *sid, char *dir, char *file)
{
	obj_t *confobj=nes_settable(sid->N, &sid->N->g, "_CONFIG");
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	obj_t *tobj=nes_settable(sid->N, &sid->N->g, "_TEMP");
	obj_t *cobj;
	char docroot[512];
	char fullname[512];
	char buf[512];
	struct stat sb;
	char *p;
	int rc=-1;

	/* check domains */
	cobj=nes_getobj(sid->N, confobj, "private_htdocs_path");
	if (!nes_isstr(cobj)) {
		cobj=nes_getobj(sid->N, confobj, "shared_htdocs_path");
		if (!nes_isstr(cobj)) return -1;
		snprintf(docroot, sizeof(docroot)-1, "%s", nes_tostr(sid->N, cobj));
		nes_setstr(sid->N, htobj, "DOCUMENT_ROOT", docroot, -1);
	} else {
		snprintf(docroot, sizeof(docroot)-1, "%s", nes_tostr(sid->N, cobj));
		nes_setstr(sid->N, htobj, "DOCUMENT_ROOT", docroot, -1);
	}
	cobj=nes_getobj(sid->N, confobj, "private_template_path");
	if (!nes_isstr(cobj)) goto tryshared;
	snprintf(fullname, sizeof(fullname)-1, "%s/%s/%s", nes_tostr(sid->N, cobj), dir, file);
	/* decodeurl(fullname); */
	fixslashes(fullname);
	if (stat(fullname, &sb)==0) {
		snprintf(buf, sizeof(buf)-1, "%s%s", docroot, nes_getstr(sid->N, htobj, "REQUEST_URI"));
		if ((p=strchr(buf, '?'))!=NULL) *p='\0';
		nes_setstr(sid->N, tobj, "PWD", buf, strlen(buf));
		rc=htnes_runscript(sid, fullname);
		goto end;
	}
tryshared:
	cobj=nes_getobj(sid->N, confobj, "shared_template_path");
	if (!nes_isstr(cobj)) return -1;
	/* then check shared templates */
	snprintf(fullname, sizeof(fullname)-1, "%s/%s/%s", nes_tostr(sid->N, cobj), dir, file);
	/* decodeurl(fullname); */
	fixslashes(fullname);
	if (stat(fullname, &sb)==0) {
		snprintf(buf, sizeof(buf)-1, "%s%s", docroot, nes_getstr(sid->N, htobj, "REQUEST_URI"));
		if ((p=strchr(buf, '?'))!=NULL) *p='\0';
		nes_setstr(sid->N, tobj, "PWD", buf, strlen(buf));
		rc=htnes_runscript(sid, fullname);
		goto end;
	}
end:
	nes_unlinkval(sid->N, tobj);
	return rc;
}
