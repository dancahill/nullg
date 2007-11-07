/*
    NESLA NullLogic Embedded Scripting Language - Copyright (C) 2007 Dan Cahill

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
#include "nesla/libnesla.h"
#include "opcodes.h"

#include <stdio.h>

void dumpsyms(nes_state *N, char *file, long line, uchar *ptr, long count)
{
#define __FUNCTION__ "dumpsyms"
	long i=0;

	n_warn(N, __FUNCTION__, "%s:%d next %d ops 0x%08x [%d]", file, line, count, N->readptr, *N->readptr);
	do {
		switch (*ptr) {
		case 0          : n_warn(N, __FUNCTION__, "EOF"); return;
		case OP_LABEL   : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][label][%s]", i, ptr, *ptr, ptr+2); break;
		case OP_NUMDATA : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][num  ][%s]", i, ptr, *ptr, ptr+2); break;
		case OP_STRDATA : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][str  ][%s]", i, ptr, *ptr, ptr+5); break;
		default         : n_warn(N, __FUNCTION__, " %d = 0x%08x [%d][op   ][%s]", i, ptr, *ptr, n_getsym(N, *ptr));
		}
		N->warnings--;
		count--;
		i++;
		ptr=n_seekop(N, ptr, 1, 0);
	} while (count);
#undef __FUNCTION__
}

void n_decompile(nes_state *N)
{
#define __FUNCTION__ "n_decompile"
	uchar *p;
	long len;
	uchar *offset=NULL;
	short showbold;

	nl_flush(N);
	if (N->blockptr==NULL) { printf(" N->blockptr is NULL\n"); return; }
	if (N->readptr==NULL) { printf(" N->readptr is NULL\n"); return; }
	if (N->blockend==NULL) { printf(" N->blockend is NULL\n"); return; }
	printf("\n----\nrecomposed source is:\n\n");
	printf(" 0x%08lX\n 0x%08lX <-you are here\n 0x%08lX\n\n", (unsigned long)N->blockptr, (unsigned long)N->readptr, (unsigned long)N->blockend);
	if (N->readptr>N->blockend) {
		printf(" N->readptr is %ld bytes past the end of the block\n\n", (unsigned long)(N->readptr-N->blockend));
		N->blockptr=N->readptr;
	} else if (N->readptr<N->blockptr) {
		printf(" N->readptr is %ld bytes before the block\n\n", (unsigned long)(N->blockptr-N->readptr));
		N->blockptr=N->readptr;
	} else {
		offset=N->readptr;
	}
	p=N->blockptr+readi4((N->blockptr+12));
	for (;*p;p++) {
		showbold=0;
		if ((offset)&&(p>=offset)) {
			showbold=1;
			printf("[01;33;40m[->");
		}
		switch (*p) {
		case OP_STRDATA:
			len=readi4((p+1));
			p+=5;
			printf("\"%s\" ", p);
			p+=len;
			break;
		case OP_NUMDATA:
			len=(long)(p[1]);
			p+=2;
			printf("%s ", p);
			p+=len;
			break;
		case OP_LABEL  :
			len=(long)(p[1]);
			p+=2;
			printf("%s ", p);
			p+=len;
			break;
		default:
			printf("%s ", n_getsym(N, *p));
		}
		if (showbold) {
			printf("<-][00m");
			offset=NULL;
		}
	}
	printf("\n\n----\n");
	return;
#undef __FUNCTION__
}
