/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

int filesend(CONNECTION *sid, unsigned char *file)
{
	struct stat sb;
	FILE *fp;
	char fileblock[2048];
	int blocksize;
	int ich;

	decodeurl(file);
	fixslashes(file);
	if (strstr(file, "..")!=NULL) return -1;
	if (stat(file, &sb)!=0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
/*	if (strlen(sid->dat->in_IfModifiedSince)) {
		send_fileheader(sid, 1, 304, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
		sid->dat->out_headdone=1;
		sid->dat->out_bodydone=1;
		sid->dat->out_flushed=1;
		sid->dat->out_ReplyData[0]='\0';
		flushbuffer(sid);
		return 0;
	}
*/
	sid->dat->out_ContentLength=sb.st_size;
	send_fileheader(sid, 1, 200, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
	fp=fopen(file, "rb");
	if (fp==NULL) return -1;
	blocksize=0;
	while ((ich=getc(fp))!=EOF) {
		if (blocksize>sizeof(fileblock)-1) {
			if (RunAsCGI) {
				fwrite(fileblock, sizeof(char), blocksize, stdout);
			} else {
				send(sid->socket, fileblock, blocksize, 0);
			}
			blocksize=0;
		}
		fileblock[blocksize]=ich;
		blocksize++;
	}
	if (blocksize) {
		if (RunAsCGI) {
			fwrite(fileblock, sizeof(char), blocksize, stdout);
		} else {
			send(sid->socket, fileblock, blocksize, 0);
		}
		blocksize=0;
	}
	fclose(fp);
	sid->dat->out_headdone=1;
	sid->dat->out_bodydone=1;
	sid->dat->out_flushed=1;
	sid->dat->out_ReplyData[0]='\0';
	flushbuffer(sid);
	return 0;
}
