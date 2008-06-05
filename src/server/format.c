/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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

char *decode_base64(char *dest, int szdest, char *src)
{
	static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int destidx, state, ch;
	char *pos;

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
	dest[szdest]='\0';
	return dest;
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
	char *ptemp;
	va_list ap;

	ptemp=dest+strlen(dest);
	va_start(ap, format);
	vsnprintf(ptemp, maxlen, format, ap);
	va_end(ap);
	ptemp[maxlen-1]='\0';
	return dest;
}

time_t time_sql2unix(char *sqldate)
{
	char *pdate=sqldate;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int year;
	int month;
	int day;
	int i;
	int unixdate=0;

	if (atoi(pdate)>=1970) {
		year=atoi(pdate);
		while ((*pdate)&&(*pdate!='-')) pdate++;
		while ((*pdate)&&(!isdigit(*pdate))) pdate++;
		month=atoi(pdate)-1;
		while ((*pdate)&&(*pdate!='-')) pdate++;
		while ((*pdate)&&(!isdigit(*pdate))) pdate++;
		day=atoi(pdate)-1;
		for (i=1970;i<year;i++) {
			unixdate+=365;
			if ((i/4.0f)==(int)(i/4)) {
				if ((i/400.0f)==(int)(i/400)) {
				unixdate++;
					} else if ((i/100.0f)!=(int)(i/100)) {
					unixdate++;
				}
			}
		}
		for (i=0;i<month;i++) {
			unixdate+=dim[i];
			if (i!=1) continue;
			if ((year/4.0f)==(int)(year/4)) {
				if ((year/400.0f)==(int)(year/400)) {
					unixdate++;
				} else if ((year/100.0f)!=(int)(year/100)) {
					unixdate++;
				}
			}
		}
		unixdate+=day;
		unixdate*=86400;
		while ((*pdate)&&(*pdate!=' ')) pdate++;
	}
	if (unixdate<0) unixdate=0;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	if (*pdate=='0') pdate++;
	unixdate+=atoi(pdate)*3600;
	while ((*pdate)&&(*pdate!=':')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	unixdate+=atoi(pdate)*60;
	while ((*pdate)&&(*pdate!=':')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	unixdate+=atoi(pdate);
	return unixdate;
}

char *time_unix2sql(char *outstring, int outlen, time_t unixdate)
{
	strftime(outstring, outlen-1, "%Y-%m-%d %H:%M:%S", gmtime(&unixdate));
	return outstring;
}
