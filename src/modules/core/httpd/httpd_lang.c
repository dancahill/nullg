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
#include "httpd_main.h"

char *lang_gets(CONN *conn, char *sect, char *label)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj, *tobj;
	struct stat sb;
	char fname[512];

	if ((conn == NULL) || (conn->N == NULL)) return "";
	if (strlen(conn->dat->language) == 0) return "";
	if (strlen(sect) == 0) return "";
	tobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	tobj = nsp_settable(conn->N, tobj, "LANGUAGE");
	tobj->val->attr |= NST_HIDDEN;
	/*
	 * it's hidden because it's ugly.  not because knowing the
	 * french word for 'inbox' would be a security issue.
	 */
	tobj = nsp_settable(conn->N, tobj, conn->dat->language);
	tobj = nsp_settable(conn->N, tobj, sect);
	cobj = nsp_getobj(conn->N, tobj, label);
	if (cobj->val->type != NT_NULL) return nsp_tostr(conn->N, cobj);
	/* sym not found, so let's go hunting */
	snprintf(fname, sizeof(fname) - 1, "%s/locale/%s/%s.ns", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"), conn->dat->language, sect);
	fixslashes(fname);
	if (stat(fname, &sb) == 0) {
		nsp_execfile(conn->N, fname);
	}
	cobj = nsp_getobj(conn->N, tobj, label);
	return nsp_tostr(conn->N, cobj);
}
