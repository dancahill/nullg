/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

int sqldate2unix(char *sqldate)
{
	char *pdate;
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int year;
	int month;
	int day;
	int unixdate=0;
	int i;

	pdate=sqldate;
	year=atoi(pdate);
	while ((*pdate)&&(*pdate!='-')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	month=atoi(pdate)-1;
	while ((*pdate)&&(*pdate!='-')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	day=atoi(pdate);

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
	return unixdate;
}

int sqldatetime2unix(char *sqldate)
{
	char *ptime=sqldate;
	int unixdate=sqldate2unix(sqldate);

	while ((*ptime)&&(*ptime!=' ')) ptime++;
	while ((*ptime)&&(!isdigit(*ptime))) ptime++;
	if (*ptime=='0') ptime++;
	unixdate+=atoi(ptime)*3600;
	while ((*ptime)&&(*ptime!=':')) ptime++;
	while ((*ptime)&&(!isdigit(*ptime))) ptime++;
	unixdate+=atoi(ptime)*60;
	while ((*ptime)&&(*ptime!=':')) ptime++;
	while ((*ptime)&&(!isdigit(*ptime))) ptime++;
	unixdate+=atoi(ptime);
	return unixdate;
}

char *sqldate2text(char *olddate)
{
	char *months[]={
		"January", "February", "March",     "April",   "May",      "June",
		"July",    "August",   "September", "October", "November", "December"
	};
	int sid=getsid();
	char *newdate=conn[sid].dat->envbuf;
	char tempdate[20];
	char *pdate;
	int year=0;
	int month=0;
	int day=0;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	strncpy(tempdate, olddate, 10);
	pdate=tempdate;
	while (*pdate) {
		if (*pdate=='-') *pdate=' ';
		pdate++;
	}
	pdate=tempdate;
	year=atoi(pdate);
	while ((*pdate)&&(*pdate!=' ')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	month=atoi(pdate)-1;
	while ((*pdate)&&(*pdate!=' ')) pdate++;
	while ((*pdate)&&(!isdigit(*pdate))) pdate++;
	day=atoi(pdate);
	snprintf(newdate, sizeof(conn[sid].dat->envbuf)-1, "%s %d, %d", months[month], day, year);
	return newdate;
}

char *sqltime2text(char *oldtime)
{
	int sid=getsid();
	char *newtime=conn[sid].dat->envbuf;
	char *ptime=oldtime;
	char *ampm="AM";
	int hour;
	int minute;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	if (strlen(ptime)>12) {
		while ((*ptime)&&(*ptime!=' ')) ptime++;
		while ((*ptime)&&(!isdigit(*ptime))) ptime++;
	}
	if (*ptime=='0') ptime++;
	hour=atoi(ptime);
	while ((*ptime)&&(*ptime!=':')) ptime++;
	while ((*ptime)&&(!isdigit(*ptime))) ptime++;
	if (*ptime=='0') ptime++;
	minute=atoi(ptime);
	if (hour>11) {
		ampm="PM";
		if (hour>12) hour-=12;
	}
	if (hour==0) hour=12;
	snprintf(newtime, sizeof(conn[sid].dat->envbuf)-1, "%d:%02d %s", hour, minute, ampm);
	return newtime;
}

char *strcatf(char *dest, const char *format, ...)
{
	char catbuffer[1024];
	va_list ap;

	va_start(ap, format);
	memset(catbuffer, 0, sizeof(catbuffer));
	vsnprintf(catbuffer, sizeof(catbuffer)-1, format, ap);
	strcat(dest, catbuffer);
	return dest;
}

char *strcasestr(char *src, char *query)
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

void striprn(char *string)
{
	while ((string[strlen(string)-1]=='\r')||(string[strlen(string)-1]=='\n')) {
		string[strlen(string)-1]='\0';
	}
}

int printht(const char *format, ...)
{
	unsigned char buffer[1024];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	while (buffer[offset]) {
		if (buffer[offset]=='<') {
			prints("&lt;");
		} else if (buffer[offset]=='>') {
			prints("&gt;");
		} else if (buffer[offset]=='&') {
			prints("&amp;");
		} else {
			prints("%c", buffer[offset]);
		}
		offset++;
	}
	return 0;
}

void printline(char *msgtext)
{
	char *pTemp;
	char *pTemp2;
	char line[100];

	pTemp=msgtext;
	while (strlen(pTemp)>80) {
		memset(line, 0, sizeof(line));
		snprintf(line, 80, "%s", pTemp);
		pTemp2=strrchr(line, ' ');
		if (pTemp2!=NULL) *pTemp2='\0';
		printht("%s\r\n", line);
		pTemp+=strlen(line);
		if (pTemp2!=NULL) pTemp+=1;
	}
	printht("%s\r\n", pTemp);
}

char *getdate(char *src)
{
	char *days[]={ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *months[]={
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	char *pTemp=src;
	int i;
	int stat;
	int day, month, year;
	int sid=getsid();
	char *newdate=conn[sid].dat->envbuf;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	stat=0;
	for (i=0;i<7;i++) {
		if (strncasecmp(pTemp, days[i], 3)==0) stat=1;
	}
	if (stat==0) return "";
	while ((*pTemp)&&(!isdigit(*pTemp))) pTemp++;
	day=atoi(pTemp);
	while ((*pTemp)&&(!isalpha(*pTemp))) pTemp++;
	stat=0;
	for (i=0;i<12;i++) {
		if (strncasecmp(pTemp, months[i], 3)==0) stat=i;
	}
	if (stat==0) return "";
	month=stat;
	while ((*pTemp)&&(!isdigit(*pTemp))) pTemp++;
	year=atoi(pTemp);
	if ((day)&&(month)&&(year)) {
		snprintf(newdate, 20, "%s %02d, %04d", months[month], day, year);
	}
	return newdate;
}
