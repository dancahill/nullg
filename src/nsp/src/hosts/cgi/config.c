/*
    nsp.cgi -- simple Nesla CGI host
    Copyright (C) 2007-2023 Dan Cahill

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

int config_read()
{
	obj_t *confobj = nsp_settable(N, &N->g, "_CONFIG");
	obj_t *cobj;

	htnsp_runscript(CONFIG_FILENAME);
	/* fill in values for missing vars */
	cobj = nsp_getobj(N, confobj, "language");
	if (cobj->val->type == NT_NULL) nsp_setstr(N, confobj, "language", CONFIG_LANGUAGE, -1);
	cobj = nsp_getobj(N, confobj, "max_runtime");
	if (cobj->val->type == NT_NULL) nsp_setnum(N, confobj, "max_runtime", CONFIG_MAX_RUNTIME);
	cobj = nsp_getobj(N, confobj, "max_postsize");
	if (cobj->val->type == NT_NULL) nsp_setnum(N, confobj, "max_postsize", CONFIG_MAX_POSTSIZE);
	cobj = nsp_getobj(N, confobj, "use_syslog");
#ifndef WIN32
#ifdef CONFIG_USE_SYSLOG
	if (cobj->val->type == NT_NULL) nsp_setstr(N, confobj, "use_syslog", "y", -1);
#else
	if (cobj->val->type == NT_NULL) nsp_setstr(N, confobj, "use_syslog", "n", -1);
#endif
#endif
	return 0;
}
