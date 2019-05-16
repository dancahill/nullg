/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2019 Dan Cahill

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
#include "nsp/nsplib.h"
#include "base.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <direct.h>
#include <io.h>
#elif !defined( __TURBOC__)
#include <unistd.h>
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

/*
* file functions
*/
NSP_FUNCTION(libnsp_base_file_chdir)
{
#define __FN__ __FILE__ ":libnsp_base_file_chdir()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int rc;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	rc = chdir(cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	return rc;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_mkdir)
{
#define __FN__ __FILE__ ":libnsp_base_file_mkdir()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	mode_t umask = 0755;
#endif
	int rc;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
#if !defined(_MSC_VER) && !defined(__BORLANDC__) && !defined( __TURBOC__)
	if (cobj2->val->type == NT_NUMBER) umask = (mode_t)cobj2->val->d.num;
	rc = mkdir(cobj1->val->d.str, umask & 0777);
#else
	rc = mkdir(cobj1->val->d.str);
#endif
	nsp_setnum(N, &N->r, "", rc);
	return rc;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_readall)
{
#define __FN__ __FILE__ ":libnsp_base_file_readall()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *robj;
	struct stat sb;
	char *p;
	int bl;
	int fd;
	int r;
	int offset = 0;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (stat(cobj1->val->d.str, &sb) != 0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd = open(cobj1->val->d.str, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type == NT_NUMBER) {
		offset = (int)cobj2->val->d.num;
		lseek(fd, offset, SEEK_SET);
	}
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	bl = sb.st_size - offset;
	robj->val->d.str = (char *)n_alloc(N, bl + 2, 0);
	robj->val->size = bl;
	p = (char *)robj->val->d.str;
	while (bl > 0) {
		if ((r = read(fd, p, bl)) < 0) break;
		p += r;
		bl -= r;
	}
	close(fd);
	robj->val->d.str[sb.st_size - offset] = '\0';
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_rename)
{
#define __FN__ __FILE__ ":libnsp_base_file_rename()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	struct stat sb;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING || cobj2->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (stat(cobj1->val->d.str, &sb) != 0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	rename(cobj1->val->d.str, cobj2->val->d.str);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_stat)
{
#define __FN__ __FILE__ ":libnsp_base_file_stat()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t tobj;
	struct stat sb;
	int rc;
	int sym = 0;
	char *file;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	file = cobj1->val->d.str;
#if defined(WIN32) || defined(__TURBOC__)
	rc = stat(file, &sb);
	if (rc != 0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
#else
	rc = lstat(file, &sb);
	if (rc != 0) {
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	if (!(~sb.st_mode&S_IFLNK)) {
		sym = 1;
		if (stat(file, &sb) != 0) sym = 2;
	}
#endif
	tobj.val = n_newval(N, NT_TABLE);
	nsp_setnum(N, &tobj, "mtime", (num_t)sb.st_mtime);
	if (sym == 2) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", "broken", 6);
	}
	else if ((sb.st_mode&S_IFDIR)) {
		nsp_setnum(N, &tobj, "size", 0);
		nsp_setstr(N, &tobj, "type", sym ? "dirp" : "dir", sym ? 4 : 3);
	}
	else {
		nsp_setnum(N, &tobj, "size", sb.st_size);
		nsp_setstr(N, &tobj, "type", sym ? "filep" : "file", sym ? 5 : 4);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_unlink)
{
#define __FN__ __FILE__ ":libnsp_base_file_unlink()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	int rc = -1;
	int i;

	settrace();
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	for (i = 1;; i++) {
		cobj1 = nsp_getobj(N, &N->context->l, n_ntoa(N, N->numbuf, i, 10, 0));
		if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) break;
		rc = unlink(cobj1->val->d.str);
		if (rc) break;
	}
	nsp_setnum(N, &N->r, "", rc);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_base_file_writeall)
{
#define __FN__ __FILE__ ":libnsp_base_file_writeall()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->context->l, "3");
	char *fname = nsp_getstr(N, &N->context->l, "0");
	int fd = -1;
	int w = 0;
	int offset = 0;

	settrace();
	/* umask(022); */
	if (cobj1->val->type != NT_STRING || cobj1->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (nc_strcmp(fname, "writeall") == 0) {
		fd = open(cobj1->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	}
	else if (nc_strcmp(fname, "append") == 0) {
		fd = open(cobj1->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
	}
	if (fd == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if (cobj2->val->type == NT_STRING) {
		if (cobj3->val->type == NT_NUMBER) {
			offset = (int)cobj3->val->d.num;
			lseek(fd, offset, SEEK_SET);
		}
		w = write(fd, cobj2->val->d.str, cobj2->val->size);
	}
	close(fd);
	nsp_setnum(N, &N->r, "", w);
	return 0;
#undef __FN__
}
