/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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
#include "data.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
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
 * This implementation of cdb is 100% original (except for cdb_hash())
 * reference - tinycdb written by Michael Tokarev.
 */

static unsigned long cdb_hash(const void *buf, unsigned long len)
{
	uchar *p = (uchar *)buf;
	uchar *end = p + len;
	unsigned long hash = 5381;

	while (p < end) hash = (hash + (hash << 5)) ^ *p++;
	return hash;
}

NSP_FUNCTION(libnsp_data_cdb_read)
{
#define __FN__ __FILE__ ":libnsp_data_cdb_read()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj;
	obj_t tobj;
	char namebuf[MAX_OBJNAMELEN + 1];
	char nbuf[8];
	uchar *cdb_toc;
	uchar *cdb_idx;
	uchar *hashptr;
	struct stat sb;
	int fd;
	//int r;
	int i, j;
	int count;
	int offset;
	int len;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (stat(cobj1->val->d.str, &sb) != 0) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	if ((fd = open(cobj1->val->d.str, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	cdb_toc = n_alloc(N, 2048, 1);
	lseek(fd, 0, SEEK_SET);
	read(fd, cdb_toc, 2048);
	for (i = 0;i < 256;i++) {
		hashptr = cdb_toc + i * 8;
		offset = readi4(hashptr);
		hashptr += 4;
		count = readi4(hashptr);
		if (!count) continue;
		cdb_idx = n_alloc(N, count * 8, 1);
		lseek(fd, offset, SEEK_SET);
		read(fd, cdb_idx, count * 8);
		for (j = 0;j < count;j++) {
			hashptr = cdb_idx + j * 8 + 4;
			offset = readi4(hashptr);
			if (!offset) continue;
			lseek(fd, offset, SEEK_SET);
			read(fd, nbuf, 8);
			len = readi4(nbuf);
			if (!len) continue;
			if (len > MAX_OBJNAMELEN) {
				read(fd, namebuf, MAX_OBJNAMELEN);
				namebuf[MAX_OBJNAMELEN] = 0;
				lseek(fd, len - MAX_OBJNAMELEN, SEEK_CUR);
			}
			else {
				read(fd, namebuf, len);
				namebuf[len] = 0;
			}
			len = readi4((nbuf + 4));
			cobj = nsp_setstr(N, &tobj, namebuf, NULL, 0);
			if (!len) continue;
			cobj->val->size = len;
			cobj->val->d.str = n_alloc(N, cobj->val->size + 1, 0);
			read(fd, cobj->val->d.str, cobj->val->size);
			cobj->val->d.str[cobj->val->size] = 0;
		}
		n_free(N, (void *)&cdb_idx, count * 8);
	}
	n_free(N, (void *)&cdb_toc, 2048);
	close(fd);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

/*
 * dump a table into a named cdb file
 *
 * this took me all day to write, and i'm tired
 * consider this code broken and dangerous
 *
 * if you can't guess, i've never touched cdb before today
 * e-mail comments to that effect to /dev/null
 */
NSP_FUNCTION(libnsp_data_cdb_write)
{
#define __FN__ __FILE__ ":libnsp_data_cdb_write()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	obj_t *cobj;
	char nbuf[4];
	char *p;
	uchar *cdb_toc = n_alloc(N, 2048, 1);
	uchar *cdb_idx = NULL;
	uchar *hashptr;
	uchar hashmod;
	unsigned int kl, vl;
	unsigned long hash;
	unsigned long datasize;
	int fd;
	int w;
	int i, j;
	int numkeys = 0;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg2");
	if ((fd = open(cobj1->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	w = write(fd, cdb_toc, 2048);
	for (cobj = cobj2->val->d.table.f;cobj;cobj = cobj->next) {
		kl = nc_strlen(cobj->name);
		if (cobj->val->type == NT_STRING) {
			p = cobj->val->d.str ? cobj->val->d.str : "";
			vl = cobj->val->size;
		}
		else {
			p = nsp_tostr(N, cobj);
			vl = nc_strlen(p);
		}
		writei4(kl, nbuf);
		w += write(fd, nbuf, 4);
		writei4(vl, nbuf);
		w += write(fd, nbuf, 4);
		w += write(fd, cobj->name, kl);
		w += write(fd, p, vl);
		hash = cdb_hash(cobj->name, kl);
		hashmod = (uchar)hash % 255;
		hashptr = cdb_toc + (hashmod * 8 + 4);
		writei4((readi4(hashptr) + 1), hashptr);
		numkeys++;
	}
	datasize = w - 2048;
	w = 0;
	for (i = 0;i < 256;i++) {
		hashptr = cdb_toc + (i * 8 + 4);
		j = readi4(hashptr);
		hashptr = cdb_toc + (i * 8);
		writei4((2048 + datasize + w), hashptr);
		w += j * 8;
	}
	lseek(fd, 0, SEEK_SET);
	w = write(fd, cdb_toc, 2048);
	if (numkeys > 0) {
		cdb_idx = n_alloc(N, numkeys * 8, 1);
		w = 2048;
		for (cobj = cobj2->val->d.table.f;cobj;cobj = cobj->next) {
			kl = nc_strlen(cobj->name);
			if (cobj->val->type == NT_STRING) {
				p = cobj->val->d.str ? cobj->val->d.str : "";
				vl = cobj->val->size;
			}
			else {
				p = nsp_tostr(N, cobj);
				vl = nc_strlen(p);
			}
			hash = cdb_hash(cobj->name, nc_strlen(cobj->name));
			hashmod = (uchar)hash % 255;
			hashptr = cdb_toc + (hashmod * 8);
			j = readi4(hashptr);
			hashptr = cdb_idx + (j - 2048 - datasize);
			j = readi4(hashptr);
			while ((j = readi4(hashptr)) != 0) hashptr += 8;
			writei4(hash, hashptr);
			hashptr += 4;
			writei4(w, hashptr);
			w += 8 + kl + vl;
		}
		lseek(fd, 2048 + datasize, SEEK_SET);
		if (write(fd, cdb_idx, numkeys * 8) != numkeys * 8) {
			n_warn(N, __FN__, "write() wrote less bytes than expected");
		}
	}
	close(fd);
	nsp_setnum(N, &N->r, "", w);
	if (cdb_idx) n_free(N, (void *)&cdb_idx, numkeys * 8);
	n_free(N, (void *)&cdb_toc, 2048);
	return 0;
#undef __FN__
}
