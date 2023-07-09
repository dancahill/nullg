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

int htpage_dirlist(CONN *conn)
{
	obj_t *confobj = nsp_getobj(conn->N, &conn->N->g, "_CONFIG");
	obj_t *htobj = nsp_getobj(conn->N, &conn->N->g, "_SERVER");
	obj_t *cobj1 = NULL, *cobj2 = NULL, *cobj3 = NULL;
	char  *RequestURI;
	char   dir[512];
	char   path[512];
	char   index[512];
	struct stat sb;

	if (htobj->val->type != NT_TABLE) return -1;
	RequestURI = nsp_getstr(conn->N, htobj, "REQUEST_URI");
	if (strncmp(RequestURI, "/", 1) != 0) return -1;
	if (strstr(RequestURI, "..") != NULL) return -1;
	snprintf(dir, sizeof(dir) - 1, "%s", RequestURI + 1);
	while (strlen(dir) > 0 && (dir[strlen(dir) - 1] == '\\' || dir[strlen(dir) - 1] == '/')) dir[strlen(dir) - 1] = '\0';
	decodeurl(dir);
	fixslashes(dir);

	cobj1 = nsp_getobj(conn->N, nsp_settable(conn->N, confobj, "paths"), "var");
	snprintf(path, sizeof(path) - 1, "%s/domains/%04d/htdocs/%s", nsp_tostr(conn->N, cobj1), conn->dat->did, dir);
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htpage_dirlist: path=[%s]", path);
	while ((path[strlen(path) - 1] == '\\') || (path[strlen(path) - 1] == '/')) { path[strlen(path) - 1] = '\0'; };
	fixslashes(path);
	if (stat(path, &sb) == 0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(conn, path);
		/* it exists, and it's a dir */
		snprintf(index, sizeof(index) - 1, "%s/index.html", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return filesend(conn, index);
		snprintf(index, sizeof(index) - 1, "%s/index.nsp", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.nsp");
		snprintf(index, sizeof(index) - 1, "%s/index.ns", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.ns");
	}
	else {
		cobj1 = NULL;
	}
	cobj2 = nsp_getobj(conn->N, nsp_settable(proc->N, confobj, "paths"), "var");
	snprintf(path, sizeof(path) - 1, "%s/share/htdocs/%s", nsp_tostr(conn->N, cobj2), dir);
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htpage_dirlist: path=[%s]", path);
	while ((path[strlen(path) - 1] == '\\') || (path[strlen(path) - 1] == '/')) { path[strlen(path) - 1] = '\0'; };
	fixslashes(path);
	if (stat(path, &sb) == 0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(conn, path);
		/* it exists, and it's a dir */
		snprintf(index, sizeof(index) - 1, "%s/index.html", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return filesend(conn, index);
		snprintf(index, sizeof(index) - 1, "%s/index.nsp", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.nsp");
		snprintf(index, sizeof(index) - 1, "%s/index.ns", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.ns");
	}
	else {
		cobj2 = NULL;
	}
	cobj3 = nsp_getobj(conn->N, nsp_settable(conn->N, confobj, "paths"), "lib");
	snprintf(path, sizeof(path) - 1, "%s/htdocs/%s", nsp_tostr(conn->N, cobj3), dir);
	//log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "htpage_dirlist: path=[%s]", path);
	while ((path[strlen(path) - 1] == '\\') || (path[strlen(path) - 1] == '/')) { path[strlen(path) - 1] = '\0'; };
	fixslashes(path);
	if (stat(path, &sb) == 0) {
		if (!(sb.st_mode&S_IFDIR)) return filesend(conn, path);
		/* it exists, and it's a dir */
		snprintf(index, sizeof(index) - 1, "%s/index.html", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return filesend(conn, index);
		snprintf(index, sizeof(index) - 1, "%s/index.nsp", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.nsp");
		snprintf(index, sizeof(index) - 1, "%s/index.ns", path);
		fixslashes(index);
		if (stat(index, &sb) == 0) return htnsp_doscript_htdocs(conn, dir, "index.ns");
	}
	else {
		cobj3 = NULL;
	}
	if (cobj1 == NULL&&cobj2 == NULL&&cobj3 == NULL) return -1;
	/* or just list the dir */
	send_header(conn, 0, 200, "1", "text/html", -1, -1);
	//htpage_header(conn, RequestURI);
	//htpage_topmenu(conn, "dir");
	if (htnsp_dotemplate(conn, "html", "dir.ns") != 0) {
		prints(conn, "<center>Access Denied - Directory listing not allowed.</center>\r\n");
	}
	//htpage_footer(conn);
	return 0;
}
