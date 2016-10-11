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
#include "main.h"
/*
#ifdef WIN32
#define LIBEXT "dll"
#else
#define LIBEXT "so"
#endif

static NSP_FUNCTION(libnsp_dl_load)
{
#define __FN__ __FILE__ ":libnsp_dl_load()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj, *tobj;
	NSP_CFUNC cfunc;
#ifdef WIN32
	HINSTANCE l;
#else
	void *l;
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
		snprintf(namebuf, sizeof(namebuf) - 1, "%s/libnsp_%s.%s", cobj->val->d.str, cobj1->val->d.str, LIBEXT);
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
	//log_error(proc.N, "core", __FILE__, __LINE__, 1, "can't open lib '%s' - %s", cobj1->val->d.str, lib_error());
	nsp_setstr(N, nsp_getobj(N, &N->g, "dl"), "last_error", lib_error(), -1);
	nsp_setbool(N, &N->r, "", 0);
	return 0;
#undef __FN__
}
*/

static NSP_FUNCTION(corensp_logaccess)
{
	log_access(proc.N, "core", "%s", nsp_getstr(N, &N->l, "1"));
	return 0;
}

void nsdevents_start(nsp_state *N)
{
	obj_t *confobj = nsp_getobj(N, &N->g, "CONFIG");
	char fname[255];
	struct stat sb;

	snprintf(fname, sizeof(fname) - 1, "%s/share/scripts/core/events.ns", nsp_getstr(N, nsp_getobj(N, confobj, "paths"), "var"));
	fixslashes(fname);
	if (stat(fname, &sb) != 0) {
		snprintf(fname, sizeof(fname) - 1, "%s/scripts/core/events.ns", nsp_getstr(N, nsp_getobj(N, confobj, "paths"), "lib"));
		fixslashes(fname);
		if (stat(fname, &sb) != 0) return;
	}

	nsp_setcfunc(N, &N->g, "logaccess", (NSP_CFUNC)corensp_logaccess);

	nsp_execfile(N, fname);
	if (N->err) {
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "errno=%d :: %s", N->err, N->errbuf);
		return;
	}
	nsp_exec(N, "if (typeof(nsd.core.events)=='table') global _core_events=new nsd.core.events();");
	if (N->err) {
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "errno=%d :: %s", N->err, N->errbuf);
		return;
	}
	nsp_exec(N, "_core_events.onstart();");
	if (N->err) {
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "errno=%d :: %s", N->err, N->errbuf);
		return;
	}
}

void nsdevents_timer()
{
	//char libbuf[80];
	//nsp_state *N;
	//obj_t *tobj, *tobj2;
	nsp_exec(proc.N, "_core_events.ontimer();");
	if (proc.N->err) {
		log_error(proc.N, "core", __FILE__, __LINE__, 1, "errno=%d :: %s", proc.N->err, proc.N->errbuf);
	}
	/*
	tobj = nsp_getobj(proc.N, nsp_getobj(proc.N, &proc.N->g, "CONFIG"), "cron_script");
	if (nsp_typeof(tobj) != NT_STRING || tobj->val->size < 1) {
	log_error(proc.N, "core", __FILE__, __LINE__, 1, "config.cron_script undefined");
	goto endrun;
	}
	if ((N = nsp_newstate()) == NULL) {
	log_error(proc.N, "core", __FILE__, __LINE__, 1, "nsp_newstate() failed");
	goto endrun;
	}
	#ifdef WIN32
	GetWindowsDirectory(libbuf, sizeof(libbuf));
	_snprintf(libbuf + strlen(libbuf), sizeof(libbuf) - strlen(libbuf) - 1, "\\NSP");
	#else
	snprintf(libbuf, sizeof(libbuf) - 1, "/usr/lib/nsp");
	#endif
	tobj2 = nsp_settable(N, &N->g, "dl");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "load", (NSP_CFUNC)libnsp_dl_load);
	tobj2 = nsp_settable(N, tobj2, "path");
	nsp_setstr(N, tobj2, "0", libbuf, -1);
	nsp_execfile(N, tobj->val->d.str);
	if (N->err) {
	log_error(proc.N, "core", __FILE__, __LINE__, 1, "errno=%d :: %s", N->err, N->errbuf);
	}
	N = nsp_endstate(N);
	*/
}
