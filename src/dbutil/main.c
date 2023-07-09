/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#ifdef WIN32
#include "nullg/config-nt.h"
#else
#include "nullg/config.h"
#endif
#include <stdio.h>
#include <time.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
//#define snprintf _snprintf
//#define vsnprintf _vsnprintf
//#define strcasecmp stricmp
//#define strncasecmp strnicmp
#else
#include <ctype.h>
//#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
//#include <sys/stat.h>
#endif
#include "nullg/defines.h"
#include "nsp/nsp.h"
#include "nullg/nullg.h"

static NSP_FUNCTION(nsp_sql_escape)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	char *ss, *se;
	char *s2;
	int l2;

	if (!nsp_isstr(cobj1)) {
		printf("%s() expected a string for arg1\r\n", nsp_getstr(N, &N->context->l, "0"));
		return 0;
	}
	nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str == NULL) return 0;
	se = ss = cobj1->val->d.str;
	s2 = "'";
	l2 = 1;
	for (; *se; se++) {
		if (strncmp(se, s2, l2) != 0) continue;
		nsp_strcat(N, &N->r, ss, se - ss);
		nsp_strcat(N, &N->r, "''", 2);
		ss = se += l2;
		if (*se) { --se; continue; }
		nsp_strcat(N, &N->r, ss, se - ss);
		break;
	}
	if (se > ss) {
		nsp_strcat(N, &N->r, ss, se - ss);
	}
	return 0;
}

static NSP_FUNCTION(nsp_sql_getsequence)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int rc = -1;

	if (!nsp_isstr(cobj1)) {
		printf("%s() expected a string for arg1\r\n", nsp_getstr(N, &N->context->l, "0"));
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	rc = _sql_getsequence(N, cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	return rc;
}

static NSP_FUNCTION(nsp_sql_query)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *qobj = NULL;
	int rc;

	if (!nsp_isstr(cobj1)) {
		printf("%s() expected a string for arg1\r\n", nsp_getstr(N, &N->context->l, "0"));
		return 0;
	}
	rc = _sql_query(N, &qobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, qobj);
	nsp_unlinkval(N, qobj);
	free(qobj);
	return rc;
}

static NSP_FUNCTION(nsp_sql_update)
{
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t* qobj = NULL;
	int rc;

	if (!nsp_isstr(cobj1)) {
		printf("%s() expected a string for arg1\r\n", nsp_getstr(N, &N->context->l, "0"));
		return 0;
	}
	rc = _sql_update(N, &qobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, qobj);
	nsp_unlinkval(N, qobj);
	free(qobj);
	return rc;
}

int main(int argc, char* argv[])
{
	nsp_state *N;
	obj_t *tobj;

	setvbuf(stdout, NULL, _IONBF, 0);
	if ((N = nsp_newstate()) == NULL) {
		printf("nsp_newstate() error\r\n");
		return -1;
	}
	if (config_read(N, "", NULL) < 0) {
		printf("can't read the config file.\r\n");
		return 0;
	}
	tobj = nsp_settable(N, &N->g, "sql");
	nsp_setcfunc(N, tobj, "escape", (NSP_CFUNC)nsp_sql_escape);
	nsp_setcfunc(N, tobj, "getsequence", (NSP_CFUNC)nsp_sql_getsequence);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)nsp_sql_query);
	nsp_setcfunc(N, tobj, "update", (NSP_CFUNC)nsp_sql_update);
	nsp_exec(N, "include(\"../lib/scripts/db.schema.ns\");");
	nsp_exec(N, "include(\"../lib/scripts/db.util.ns\");");
	nsp_exec(N, "global dbutil = new db.dbutil();");
	tobj = nsp_getobj(N, nsp_getobj(N, &N->g, "dbutil"), "options");
	if (nsp_istable(tobj) && argc > 1) {
		char function[16];
		char *ptemp;

		memset(function, 0, sizeof(function));
		strncpy(function, argv[1], sizeof(function) - 1);
		ptemp = function;
		while (*ptemp) {
			*ptemp = tolower(*ptemp);
			ptemp++;
		}
		nsp_setstr(N, tobj, "command", function, -1);
		if (argc > 2) {
			nsp_setstr(N, tobj, "option", argv[2], -1);
			if (strcmp(function, "init") == 0) {
				char itoa64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
				char rootpass[40];
				char salt[10];
				int i;

				memset(rootpass, 0, sizeof(rootpass));
				memset(salt, 0, sizeof(salt));
				srand((int)time(NULL));
				for (i = 0; i < 8; i++) salt[i] = itoa64[(rand() % 64)];
				md5_crypt(rootpass, argv[2], salt);
				nsp_setstr(N, tobj, "rootpass", rootpass, -1);
			}
		}
		if (strcmp(function, "init") == 0) {
			nsp_exec(N, "dbutil.init();");
		}
		else if (strcmp(function, "dump") == 0) {
			nsp_exec(N, "dbutil.dump();");
		}
		else if (strcmp(function, "restore") == 0) {
			nsp_exec(N, "dbutil.restore();");
		}
		else {
			nsp_exec(N, "dbutil.usage();");
		}
	}
	else {
		nsp_exec(N, "dbutil.usage();");
	}
	nsp_exec(N, "io.flush();");
	if (N->err) {
		printf("\r\nerrno=%d :: \"%s\"\r\n", N->err, N->errbuf);
	}
	N = nsp_endstate(N);
	return 0;
}
