/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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

char *getbuffer(CONN *sid)
{
	sid->dat->lastbuf++;
	if (sid->dat->lastbuf>3) sid->dat->lastbuf=0;
	memset(sid->dat->smallbuf[sid->dat->lastbuf], 0, sizeof(sid->dat->smallbuf[sid->dat->lastbuf]));
	return sid->dat->smallbuf[sid->dat->lastbuf];
}

void fixslashes(char *string)
{
 	while (*string) {
#ifdef WIN32
 		if (*string=='/') *string='\\';
#else
 		if (*string=='\\') *string='/';
#endif
		string++;
	}
}

void striprn(char *string)
{
	int i;

	while (1) {
		i=strlen(string);
		if (i<1) break;
		if (string[i-1]=='\r') { string[i-1]='\0'; continue; }
		if (string[i-1]=='\n') { string[i-1]='\0'; continue; }
		break;
	};
}

void swapchar(char *string, char oldchar, char newchar)
{
	while (*string) {
		if (*string==oldchar) *string=newchar;
		string++;
	}
}

char *p_strcasestr(char *src, char *query)
{
	char *pToken;
	char Buffer[8192];
	char Query[64];
	int loop;

	if (strlen(src)==0) return NULL;
	memset(Buffer, 0, sizeof(Buffer));
	strncpy(Buffer, src, sizeof(Buffer)-1);
	strncpy(Query, query, sizeof(Query)-1);
	loop=0;
	while (Buffer[loop]) {
		Buffer[loop]=toupper(Buffer[loop]);
		loop++;
	}
	loop=0;
	while (Query[loop]) {
		Query[loop]=toupper(Query[loop]);
		loop++;
	}
	pToken=strstr(Buffer, Query);
	if (pToken!=NULL) {
		return src+(pToken-(char *)&Buffer);
	}
	return NULL;
}

char *strncatf(char *dest, int maxlen, const char *format, ...)
{
	char buffer[2048];
	va_list ap;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	strncat(dest, buffer, maxlen);
	return dest;
}

char *str2sql(CONN *sid, char *instring)
{
	char *buffer=getbuffer(sid);
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	while ((instring[i])&&(i<sizeof(sid->dat->smallbuf[0])-1)) {
		ch=instring[i];
		if (ch==0) break;
		if (ch=='\'') {
			buffer[bufferlength]='\'';
			buffer[bufferlength+1]=ch;
			bufferlength+=2;
		} else if (ch=='\\') {
			buffer[bufferlength]='\\';
			buffer[bufferlength+1]=ch;
			bufferlength+=2;
		} else {
			buffer[bufferlength]=ch;
			bufferlength++;
		}
		i++;
	}
	return buffer;
}

char *str2sqlbuf(CONN *sid, char *instring, char *outstring, int outsize)
{
	int srcindex=0;
	int dstindex=0;

	while ((dstindex<outsize)&&(instring[srcindex])) {
		if (instring[srcindex]=='\'') {
			outstring[dstindex]='\'';
			outstring[dstindex+1]=instring[srcindex];
			dstindex+=2;
		} else if (instring[srcindex]=='\\') {
			outstring[dstindex]='\\';
			outstring[dstindex+1]=instring[srcindex];
			dstindex+=2;
		} else {
			outstring[dstindex]=instring[srcindex];
			dstindex++;
		}
		srcindex++;
	}
	if (dstindex<outsize) {
		outstring[dstindex]='\0';
	}
	return outstring;
}

char *time_unix2sql(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *DecodeBase64string(CONN *sid, char *src)
{
	static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static char dest[1024];
	int destidx, state, ch;
	int szdest;
	char *pos;

	memset(dest, 0, sizeof(dest));
	szdest=sizeof(dest)-1;
	state=0;
	destidx=0;
	while ((ch=*src++)!='\0') {
		if (isspace(ch)) continue;
		if (ch=='=') break;
		pos=strchr(Base64, ch);
		if (pos==0) return "";
		switch (state) {
			case 0:
				if (dest) {
					if (destidx>=szdest) return dest;
					dest[destidx]=(pos-Base64)<<2;
				}
				state=1;
				break;
			case 1:
				if (dest) {
					if (destidx+1>=szdest) return dest;
					dest[destidx]|=(pos-Base64)>>4;
					dest[destidx+1]=((pos-Base64)&0x0f)<<4;
				}
				destidx++;
				state=2;
				break;
			case 2:
				if (dest) {
					if (destidx+1>=szdest) return dest;
					dest[destidx]|=(pos-Base64)>>2;
					dest[destidx+1]=((pos-Base64)&0x03)<<6;
				}
				destidx++;
				state=3;
				break;
			case 3:
				if (dest) {
					if (destidx>=szdest) return dest;
					dest[destidx]|=(pos-Base64);
				}
				destidx++;
				state=0;
				break;
		}
	}
	return dest;
}
