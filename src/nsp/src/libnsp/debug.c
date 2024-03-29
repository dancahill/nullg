/*
    NESLA NullLogic Embedded Scripting Language
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
#ifndef _NSPLIB_H
#include "nsp/nsplib.h"
#endif
#include "opcodes.h"

#include <stdio.h>

void dumpsyms(nsp_state *N, char *file, long line, uchar *ptr, long count)
{
#define __FN__ __FILE__ ":dumpsyms()"
	long i = 0;

	settrace();
	n_warn(N, __FN__, "%s:%d next %d ops 0x%08x [%d]", file, line, count, n_context_readptr, *n_context_readptr);
	do {
		switch (*ptr) {
		case 0: n_warn(N, __FN__, "EOF"); return;
		case OP_LABEL: n_warn(N, __FN__, " %d = 0x%08x [%d][label][%s]", i, ptr, *ptr, ptr + 2); break;
		case OP_NUMDATA: n_warn(N, __FN__, " %d = 0x%08x [%d][num  ][%s]", i, ptr, *ptr, ptr + 2); break;
		case OP_STRDATA: n_warn(N, __FN__, " %d = 0x%08x [%d][str  ][%s]", i, ptr, *ptr, ptr + 5); break;
		case OP_ESTRDATA: n_warn(N, __FN__, " %d = 0x%08x [%d][estr ][%s]", i, ptr, *ptr, ptr + 5); break;
		default: n_warn(N, __FN__, " %d = 0x%08x [%d][op   ][%s]", i, ptr, *ptr, n_getsym(N, *ptr));
		}
		N->warnings--;
		count--;
		i++;
		ptr = n_seekop(N, ptr, 0);
	} while (count);
#undef __FN__
}

void n_decompile(nsp_state *N, uchar *start, uchar *end, char *errbuf, unsigned short errmax)
{
#define __FN__ __FILE__ ":n_decompile()"
	uchar *oldbptr = n_context_blockptr;
	uchar *oldrptr = n_context_readptr;
	uchar *oldbend = n_context_blockend;
	uchar *p;
	uchar *offset = NULL;
	long len;
	short showbold;
	short subchunk;

	settrace();
	subchunk = (start && end) ? 1 : 0;
	nl_flush(N);
	if (n_context_blockptr == NULL) { nc_printf(N, " N->blockptr is NULL\n"); return; }
	if (n_context_blockend == NULL) { nc_printf(N, " N->blockend is NULL\n"); return; }
	if (n_context_readptr == NULL) { nc_printf(N, " N->readptr is NULL\n"); return; }
	if (subchunk) {
		n_context_blockptr = start;
		n_context_blockend = end;
		n_context_readptr = start;
	}
	if (!subchunk) nc_printf(N, "\n----\nrecomposed source is:\n\n");
	if (!subchunk) nc_printf(N, " 0x%08lX\n 0x%08lX <-you are here\n 0x%08lX\n\n", (uint64)n_context_blockptr, (uint64)n_context_readptr, (uint64)n_context_blockend);
	if (n_context_readptr > n_context_blockend) {
		nc_printf(N, " N->readptr is %ld bytes past the end of the block\n\n", (uint64)(n_context_readptr - n_context_blockend));
		n_context_blockptr = n_context_readptr;
	} else if (n_context_readptr < n_context_blockptr) {
		nc_printf(N, " N->readptr is %ld bytes before the block\n\n", (uint64)(n_context_blockptr - n_context_readptr));
		n_context_blockptr = n_context_readptr;
	} else {
		offset = n_context_readptr;
	}
	if (subchunk) {
		p = n_context_blockptr;
	} else {
		p = n_context_blockptr + readi4((n_context_blockptr + 12));
	}
	for (; *p; p++) {
		if (subchunk && p > end) break;
		showbold = 0;
		if ((offset) && (p >= offset)) {
			showbold = 1;
			if (!subchunk) nc_printf(N, "[01;33;40m[->");
		}
		switch (*p) {
		case OP_STRDATA:
		case OP_ESTRDATA:
			len = readi4((p + 1));
			p += 5;
			if (errbuf) nc_snprintf(N, errbuf + nc_strlen(errbuf), errmax - nc_strlen(errbuf), "\"%s\"", p);
			else nc_printf(N, "\"%s\"", p);
			p += len;
			break;
		case OP_NUMDATA:
			len = (long)(p[1]);
			p += 2;
			if (errbuf) nc_snprintf(N, errbuf + nc_strlen(errbuf), errmax - nc_strlen(errbuf), "%s", p);
			else nc_printf(N, "%s", p);
			p += len;
			break;
		case OP_LABEL:
			len = (long)(p[1]);
			p += 2;
			if (errbuf) nc_snprintf(N, errbuf + nc_strlen(errbuf), errmax - nc_strlen(errbuf), "%s", p);
			else nc_printf(N, "%s", p);
			p += len;
			break;
		default:
			if (OP_ISPUNC(*p) || OP_ISMATH(*p) || OP_ISEND(*p)) {
				if (errbuf) nc_snprintf(N, errbuf + nc_strlen(errbuf), errmax - nc_strlen(errbuf), "%s", n_getsym(N, *p));
				else nc_printf(N, "%s ", n_getsym(N, *p));
			} else if (OP_ISKEY(*p)) {
				if (errbuf) nc_snprintf(N, errbuf + nc_strlen(errbuf), errmax - nc_strlen(errbuf), "%s ", n_getsym(N, *p));
				else nc_printf(N, "%s ", n_getsym(N, *p));
			}
		}
		if (showbold) {
			if (!subchunk) nc_printf(N, "<-][00m");
			offset = NULL;
		}
	}
	if (!subchunk) nc_printf(N, "\n\n----\n");
	if (subchunk) {
		n_context_blockptr = oldbptr;
		n_context_blockend = oldbend;
		n_context_readptr = oldrptr;
	}
	if (errbuf == NULL) nc_printf(N, "\n");
	return;
#undef __FN__
}
