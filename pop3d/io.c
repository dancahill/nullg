/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "pop3_main.h"

int prints(CONN *sid, const char *format, ...)
{
	unsigned char buffer[2048];
	va_list ap;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	return send(sid->socket, buffer, strlen(buffer), 0);
}

int sgets(CONN *sid, char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int rc;
	short int x;

	if (sid==NULL) return -1;
	sid->atime=time(NULL);
retry:
	if (!sid->dat->recvbufsize) {
		x=sizeof(sid->dat->recvbuf)-sid->dat->recvbufoffset-sid->dat->recvbufsize-2;
		obuffer=sid->dat->recvbuf+sid->dat->recvbufoffset+sid->dat->recvbufsize;
		if ((rc=recv(fd, obuffer, x, 0))<0) {
			return -1;
		}
		sid->dat->recvbufsize+=rc;
	}
	obuffer=sid->dat->recvbuf+sid->dat->recvbufoffset;
	while ((n<max)&&(sid->dat->recvbufsize>0)) {
		sid->dat->recvbufoffset++;
		sid->dat->recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) return n;
	if (!lf) {
		if (sid->dat->recvbufsize>0) {
			memmove(sid->dat->recvbuf, sid->dat->recvbuf+sid->dat->recvbufoffset, sid->dat->recvbufsize);
			memset(sid->dat->recvbuf+sid->dat->recvbufsize, 0, sizeof(sid->dat->recvbuf)-sid->dat->recvbufsize);
			sid->dat->recvbufoffset=0;
		} else {
			memset(sid->dat->recvbuf, 0, sizeof(sid->dat->recvbuf));
			sid->dat->recvbufoffset=0;
			sid->dat->recvbufsize=0;
		}
		goto retry;
	}
	return n;
}
