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
#include "http_main.h"

/* TZ STRUCTURE USED FOR time_*() FUNCTIONS */
static tzentry timezones[]={
	{ -720, 0, "[GMT -12:00] Eniwetok, Kwajalein" },
	{ -660, 0, "[GMT -11:00] Midway Island, Samoa" },
	{ -600, 0, "[GMT -10:00] Hawaii" },
	{ -540, 1, "[GMT -9:00] Alaska" },
	{ -480, 1, "[GMT -8:00] Pacific Time" },
	{ -420, 0, "[GMT -7:00] Arizona" },
	{ -420, 1, "[GMT -7:00] Mountain Time" },
	{ -360, 1, "[GMT -6:00] Central Time" },
	{ -360, 0, "[GMT -6:00] Saskatchewan" },
	{ -300, 0, "[GMT -5:00] Bogota, Lima, Quito" },
	{ -300, 1, "[GMT -5:00] Eastern Time" },
	{ -300, 0, "[GMT -5:00] Indiana (East)" },
	{ -240, 1, "[GMT -4:00] Atlantic Time" },
	{ -240, 0, "[GMT -4:00] Caracas, La Paz" },
	{ -240, 1, "[GMT -4:00] Santiago" },
	{ -210, 1, "[GMT -3:30] Newfoundland" },
	{ -180, 1, "[GMT -3:00] Brasilia" },
	{ -180, 0, "[GMT -3:00] Buenos Aires, Georgetown" },
	{ -120, 1, "[GMT -2:00] Mid-Atlantic" },
	{  -60, 1, "[GMT -1:00] Azores, Cape Verde Is." },
	{    0, 0, "[GMT] Casablanca, Monrovia" },
	{    0, 1, "[GMT] Greenwich Mean Time" },
	{   60, 1, "[GMT +1:00] Amsterdam, Berlin, Rome" },
	{   60, 1, "[GMT +1:00] Belgrade, Budapest, Prague" },
	{   60, 1, "[GMT +1:00] Brussels, Madrid, Paris" },
	{   60, 1, "[GMT +1:00] Sarajevo, Warsaw" },
	{  120, 1, "[GMT +2:00] Athens, Istanbul, Minsk" },
	{  120, 1, "[GMT +2:00] Bucharest" },
	{  120, 1, "[GMT +2:00] Cairo" },
	{  120, 0, "[GMT +2:00] Harare, Pretoria" },
	{  120, 1, "[GMT +2:00] Helsinki, Riga, Tallinn" },
	{  120, 0, "[GMT +2:00] Jerusalem" },
	{  180, 0, "[GMT +3:00] Baghdad, Kuwait, Riyadh" },
	{  180, 1, "[GMT +3:00] Moscow, St. Petersburg" },
	{  180, 0, "[GMT +3:00] Nairobi" },
	{  210, 1, "[GMT +3:30] Tehran" },
	{  240, 0, "[GMT +4:00] Abu Dhabi, Muscat" },
	{  240, 0, "[GMT +4:00] Baku, Tbilisi" },
	{  270, 0, "[GMT +4:30] Kabul" },
	{  300, 1, "[GMT +5:00] Ekaterinburg" },
	{  300, 0, "[GMT +5:00] Islamabad, Karachi, Tashkent" },
	{  330, 0, "[GMT +5:30] Bombay, Calcutta, New Delhi" },
	{  360, 0, "[GMT +6:00] Astana, Almaty, Dhaka" },
	{  360, 0, "[GMT +6:00] Colombo" },
	{  420, 0, "[GMT +7:00] Bangkok, Hanoi, Jakarta" },
	{  480, 0, "[GMT +8:00] Beijing, Hong Kong, Urumqi" },
	{  480, 0, "[GMT +8:00] Perth" },
	{  480, 0, "[GMT +8:00] Singapore" },
	{  480, 0, "[GMT +8:00] Taipei" },
	{  540, 0, "[GMT +9:00] Osaka, Sapporo, Tokyo" },
	{  540, 0, "[GMT +9:00] Seoul" },
	{  540, 1, "[GMT +9:00] Yakutsk" },
	{  570, 1, "[GMT +9:30] Adelaide" },
	{  570, 0, "[GMT +9:30] Darwin" },
	{  600, 0, "[GMT +10:00] Brisbane" },
	{  600, 1, "[GMT +10:00] Canberra, Melbourne, Sydney" },
	{  600, 0, "[GMT +10:00] Guam, Port Moresby" },
	{  600, 1, "[GMT +10:00] Hobart" },
	{  600, 1, "[GMT +10:00] Vladivostok" },
	{  660, 0, "[GMT +11:00] Magadan, Solomon Is." },
	{  720, 1, "[GMT +12:00] Auckland, Wellington" },
	{  720, 0, "[GMT +12:00] Fiji, Kamchatka" }
};

char *getbuffer(CONN *sid)
{
	sid->dat->lastbuf++;
	if (sid->dat->lastbuf>3) sid->dat->lastbuf=0;
	memset(sid->dat->smallbuf[sid->dat->lastbuf], 0, sizeof(sid->dat->smallbuf[sid->dat->lastbuf]));
	return sid->dat->smallbuf[sid->dat->lastbuf];
}

void decodeurl(unsigned char *src)
{
	char *dest=src;

	while (*src) {
		if (*src=='+') {
			*dest++=' ';
			src++;
		} else if (*src=='%') {
			src++;
			if (isxdigit(src[0])&&isxdigit(src[1])) {
				*dest++=(char)hex2int(src);
				src+=2;
			}
		} else {
			*dest++=*src++;
		}
	}
	*dest='\0';
	return;
}

char *encodeurl(CONN *sid, unsigned char *src)
{
	unsigned char *buffer=getbuffer(sid);
	unsigned char *dest=buffer;
//	char *unsafe="`'!@#$%^&*(){}<>~|\\\";? ,/";
	char *unsafe="`'!@#$^&*(){}<>~|\\\";?,";
	char *hex="0123456789ABCDEF";
	short int dstlen=0;

	while (*src) {
		if (dstlen+4>sizeof(sid->dat->smallbuf[0])) break;
		if ((*src>32)&&(*src<128)&&(!strchr(unsafe, *src))) {
			*dest++=*src++;
			dstlen++;
		} else {
			*dest++='%';
			*dest++=hex[(short int)*src/16];
			*dest++=hex[(short int)*src++&15];
			dstlen+=3;
		}
	}
	*dest='\0';
	return buffer;
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

int hex2int(char *pChars)
{
	int Hi;
	int Lo;
	int Result;

	Hi=pChars[0];
	if ('0'<=Hi&&Hi<='9') {
		Hi-='0';
	} else if ('a'<=Hi&&Hi<='f') {
		Hi-=('a'-10);
	} else if ('A'<=Hi&&Hi<='F') {
		Hi-=('A'-10);
	}
	Lo = pChars[1];
	if ('0'<=Lo&&Lo<='9') {
		Lo-='0';
	} else if ('a'<=Lo&&Lo<='f') {
		Lo-=('a'-10);
	} else if ('A'<=Lo&&Lo<='F') {
		Lo-=('A'-10);
	}
	Result=Lo+(16*Hi);
	return (Result);
}

void striprn(char *string)
{
	int i=strlen(string)-1;

	while (1) {
		if (i<0) break;
		if (string[i]=='\r') { string[i]='\0'; i--; continue; }
		if (string[i]=='\n') { string[i]='\0'; i--; continue; }
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
	char *ptemp;
	va_list ap;

	ptemp=dest+strlen(dest);
	va_start(ap, format);
	vsnprintf(ptemp, maxlen, format, ap);
	va_end(ap);
	ptemp[maxlen-1]='\0';
	return dest;
}

/* HTML CODE CONVERSION TABLE */
typedef struct {
	unsigned char symbol;
	char *code;
} _htmltags;
static _htmltags htmltags[]={
	{  '"', "&quot;" },
	{  '&', "&amp;" },
	{  '<', "&lt;" },
	{  '>', "&gt;" }
};

char *str2html(CONN *sid, char *instring)
{
	unsigned char *buffer=getbuffer(sid);
	unsigned char *inptr=instring;
	unsigned char *outptr=buffer;
	short int destleft=sizeof(sid->dat->smallbuf[0])-1;
	short int i;
	short int match;
	short int str_len;

	while ((*inptr)&&(destleft>0)) {
		match=0;
		for (i=0;i<4;i++) {
			if (*inptr==htmltags[i].symbol) {
				match=1;
				str_len=strlen(htmltags[i].code);
				if (destleft<str_len) goto done;
				strcpy(outptr, htmltags[i].code);
				destleft-=str_len;
				outptr+=str_len;
				inptr++;
				break;
			}
		}
		if (!match) {
			if (destleft<1) goto done;
			*outptr++=*inptr++;
			destleft--;
		}
	}
done:
	*outptr='\0';
	buffer[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return buffer;
}

char *str2sqlbuf(CONN *sid, char *instring, char *outstring, int outsize)
{
	unsigned char ch;
	int srcindex=0;
	int dstindex=0;

	while ((dstindex<outsize)&&(instring[srcindex])) {
		ch=instring[srcindex];
		if (ch=='\'') {
			outstring[dstindex]='\'';
			outstring[dstindex+1]=ch;
			dstindex+=2;
		} else if ((ch=='\\')&&(strcasecmp(proc.config.sql_type, "SQLITE")!=0)) {
			outstring[dstindex]='\\';
			outstring[dstindex+1]=ch;
			dstindex+=2;
		} else {
			outstring[dstindex]=ch;
			dstindex++;
		}
		srcindex++;
	}
	if (dstindex<outsize) {
		outstring[dstindex]='\0';
	}
	outstring[outsize-1]='\0';
	return outstring;
}

char *str2sql(CONN *sid, char *instring)
{
	char *buffer=getbuffer(sid);

	return str2sqlbuf(sid, instring, buffer, sizeof(sid->dat->smallbuf[0])-1);
}

int printhex(CONN *sid, const char *format, ...)
{
	unsigned char *buffer=getbuffer(sid);
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, format, ap);
	va_end(ap);
	buffer[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return prints(sid, "%s", encodeurl(sid, buffer));
}

int printht(CONN *sid, const char *format, ...)
{
	unsigned char *buffer=getbuffer(sid);
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(sid->dat->smallbuf[0])-1, format, ap);
	va_end(ap);
	buffer[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return prints(sid, "%s", str2html(sid, buffer));
}

void printline(CONN *sid, short int reply, char *msgtext)
{
	char *pmsgbody;
	char *ptemp;
	char line[120];
	unsigned int width=78;

	pmsgbody=msgtext;
	if (reply) width=76;
	while (strlen(pmsgbody)>width) {
		memset(line, 0, sizeof(line));
		snprintf(line, width+1, "%s", pmsgbody);
		line[width]='\0';
		if (reply) prints(sid, "> ");
		if (strchr(line, '\r')||strchr(line, '\n')) {
			if ((ptemp=strchr(line, '\r'))!=NULL) *ptemp='\0';
			if ((ptemp=strchr(line, '\n'))!=NULL) *ptemp='\0';
			printht(sid, "%s\r\n", line);
			pmsgbody+=strlen(line);
			if (*pmsgbody=='\r') pmsgbody++;
			if (*pmsgbody=='\n') pmsgbody++;
		} else if ((ptemp=strrchr(line, ' '))!=NULL) {
			*ptemp='\0';
			printht(sid, "%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else {
			printht(sid, "%s", line);
			pmsgbody+=strlen(line);
		}
	}
	if (reply) prints(sid, "> ");
	printht(sid, "%s\r\n", pmsgbody);
}

void printline2(CONN *sid, int dowrap, char *msgtext)
{
	unsigned char buffer[1024];
	char lastchar='\0';
	char *ptemp1;
	char *ptemp2;
	int destleft;

	ptemp1=msgtext;
	while (strlen(ptemp1)>0) {
		memset(buffer, 0, sizeof(buffer));
		destleft=sizeof(buffer)-6;
		ptemp2=buffer;
		while ((*ptemp1)&&(destleft>0)) {
			if (*ptemp1=='\n') {
				if (dowrap) {
					strcat(ptemp2, "\n<BR>");
					ptemp2+=5;
					destleft-=5;
				} else {
					strcat(ptemp2, "\n");
					ptemp2+=1;
					destleft-=1;
				}
			} else if (*ptemp1=='<') {
				strcat(ptemp2, "&lt;");
				ptemp2+=4;
				destleft-=4;
			} else if (*ptemp1=='>') {
				strcat(ptemp2, "&gt;");
				ptemp2+=4;
				destleft-=4;
			} else if (*ptemp1=='&') {
				strcat(ptemp2, "&amp;");
				ptemp2+=5;
				destleft-=5;
			} else if (*ptemp1=='"') {
				strcat(ptemp2, "&quot;");
				ptemp2+=6;
				destleft-=6;
			} else if (*ptemp1==' ') {
				if (dowrap) {
					if (!isalnum(ptemp1[1])||!isalnum(lastchar)) {
						strcat(ptemp2, "&nbsp;");
						ptemp2+=6;
						destleft-=6;
					} else {
						*ptemp2=*ptemp1;
						ptemp2++;
						destleft--;
					}
				} else {
					strcat(ptemp2, " ");
					ptemp2+=1;
					destleft-=1;
				}
			} else {
				*ptemp2=*ptemp1;
				ptemp2++;
				destleft--;
			}
			lastchar=*ptemp1;
			ptemp1++;
		}
		prints(sid, "%s", buffer);
	}
	return;
}

void htselect_timezone(CONN *sid, short int selected)
{
	int i;

	for (i=0;i<62;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, (i==selected)?" SELECTED":"", timezones[i].name);
	}
	return;
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

char *time_sql2text(CONN *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2datetext(CONN *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_sql2timetext(CONN *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2lotimetext(CONN *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	char *ptemp=buffer;
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	while (*ptemp) *ptemp++=tolower(*ptemp);
	return buffer;
}

char *time_unix2sql(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *time_unix2sqldate(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%Y-%m-%d", gmtime(&unixdate));
	return buffer;
}

char *time_unix2sqltime(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *time_unix2text(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 50, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2datetext(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_unix2timetext(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2lotimetext(CONN *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);
	char *ptemp=buffer;

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	// GCC 3.2.2 was incrementing ptemp prematurely
	while (*ptemp) { *ptemp=tolower(*ptemp); ptemp++; }
	return buffer;
}

int time_tzoffset(CONN *sid, time_t unixdate)
{
	struct tm *today;
	time_t tzoffset;

	if (sid==NULL) return 0;
	if ((sid->dat->user_timezone<0)||(sid->dat->user_timezone>61)) return 0;
	tzoffset=timezones[sid->dat->user_timezone].minutes*60;
	today=localtime(&unixdate);
	if ((timezones[sid->dat->user_timezone].dst)&&(today->tm_isdst)) {
		tzoffset+=3600;
	}
	return tzoffset;
}

int time_tzoffset2(CONN *sid, time_t unixdate, int userid)
{
	int sqr;
	int tz=-1;
	time_t tzoffset;
	struct tm *today;

	if ((sqr=sql_queryf(sid, "SELECT preftimezone FROM gw_users where userid = %d", userid))<0) return 0;
	if (sql_numtuples(sqr)==1) {
		tz=atoi(sql_getvalue(sqr, 0, 0));
	}
	sql_freeresult(sqr);
	if ((tz<0)||(tz>61)) return 0;
	tzoffset=timezones[tz].minutes*60;
	today=localtime(&unixdate);
	if ((timezones[tz].dst)&&(today->tm_isdst)) {
		tzoffset+=3600;
	}
	return tzoffset;
}

int time_tzoffsetcon(CONN *sid, time_t unixdate, int contactid)
{
	int sqr;
	int tz=-1;
	time_t tzoffset;
	struct tm *today;

	if ((sqr=sql_queryf(sid, "SELECT timezone FROM gw_contacts where contactid = %d", contactid))<0) return 0;
	if (sql_numtuples(sqr)==1) {
		tz=atoi(sql_getvalue(sqr, 0, 0));
	}
	sql_freeresult(sqr);
	if ((tz<0)||(tz>61)) return 0;
	tzoffset=timezones[tz].minutes*60;
	today=localtime(&unixdate);
	if ((timezones[tz].dst)&&(today->tm_isdst)) {
		tzoffset+=3600;
	}
	return tzoffset;
}

time_t time_wmgetdate(char *src)
{
	int dim[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	char *months[]={
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	char *ptemp=src;
	int i;
	int stat;
	int day, month, year;
	int hours, minutes, seconds;
	int tzoffset;
	time_t unixdate=-1;

	while ((*ptemp)&&(!isdigit(*ptemp))) ptemp++;
	if(!isdigit(*ptemp)) return 0;
	if (*ptemp=='0') ptemp++;
	day=atoi(ptemp);
	while ((*ptemp)&&(!isalpha(*ptemp))) ptemp++;
	stat=-1;
	for (i=0;i<12;i++) {
		if (strncasecmp(ptemp, months[i], 3)==0) stat=i;
	}
	if (stat==-1) return 0;
	month=stat;
	while ((*ptemp)&&(!isdigit(*ptemp))) ptemp++;
	year=atoi(ptemp);
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
	if (unixdate<1) return 0;
	unixdate*=86400;
	while ((*ptemp)&&(isdigit(*ptemp))) ptemp++;
	while ((*ptemp)&&(!isdigit(*ptemp))) ptemp++;
	hours=atoi(ptemp);
	while ((*ptemp)&&(isdigit(*ptemp))) ptemp++;
	while ((*ptemp)&&(!isdigit(*ptemp))) ptemp++;
	minutes=atoi(ptemp);
	while ((*ptemp)&&(isdigit(*ptemp))) ptemp++;
	while ((*ptemp)&&(!isdigit(*ptemp))) ptemp++;
	seconds=atoi(ptemp);
	while ((*ptemp)&&(isdigit(*ptemp))) ptemp++;
	while ((*ptemp)&&(!isdigit(*ptemp))&&(*ptemp!='-')) ptemp++;
	tzoffset=atoi(ptemp);
	unixdate+=hours*3600;
	unixdate+=minutes*60;
	unixdate+=seconds;
	unixdate-=(int)(tzoffset/100)*3600;
	return unixdate;
}

char *DecodeBase64string(CONN *sid, char *src)
{
	static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *dest=getbuffer(sid);
	int destidx, state, ch;
	int szdest;
	char *pos;

	szdest=sizeof(sid->dat->smallbuf[0])-1;
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
	dest[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return dest;
}
