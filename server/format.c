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
#include "main.h"

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

void decodeurl(unsigned char *pEncoded)
{
	char *pDecoded;

	pDecoded=pEncoded;
	while (*pDecoded) {
		if (*pDecoded=='+') *pDecoded=' ';
		pDecoded++;
	};
	pDecoded=pEncoded;
	while (*pEncoded) {
		if (*pEncoded=='%') {
			pEncoded++;
			if (isxdigit(pEncoded[0])&&isxdigit(pEncoded[1])) {
				*pDecoded++=(char)hex2int(pEncoded);
				pEncoded+=2;
			}
		} else {
			*pDecoded++=*pEncoded++;
		}
	}
	*pDecoded='\0';
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

int printhex(CONN *sid, const char *format, ...)
{
	char *hex="0123456789ABCDEF";
	unsigned char buffer[2048];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if ((buffer[offset]>32)&&(buffer[offset]<128)&&(buffer[offset]!='<')&&(buffer[offset]!='>')&&(buffer[offset]!='+')) {
			prints(sid, "%c", buffer[offset]);
		} else {
			prints(sid, "%%%c%c", hex[(unsigned int)buffer[offset]/16], hex[(unsigned int)buffer[offset]&15]);
		}
		offset++;
	}
	return 0;
}

int printht(CONN *sid, const char *format, ...)
{
	unsigned char buffer1[2048];
	unsigned char buffer2[2048];
	char *ptemp1;
	char *ptemp2;
	int destleft=sizeof(buffer2)-6;
	va_list ap;

	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
	ptemp1=buffer1;
	ptemp2=buffer2;
	va_start(ap, format);
	vsnprintf(buffer1, sizeof(buffer1)-1, format, ap);
	va_end(ap);
	while ((*ptemp1)&&(destleft>0)) {
		if (*ptemp1=='<') {
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
		} else {
			*ptemp2=*ptemp1;
			ptemp2++;
			destleft--;
		}
		ptemp1++;
	}
	prints(sid, "%s", buffer2);
	return 0;
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

char *str2html(CONN *sid, char *instring)
{
	unsigned char *buffer=getbuffer(sid);
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	while ((instring[i])&&(i<sizeof(sid->dat->smallbuf[0])-1)) {
		ch=instring[i];
		if (ch==0) break;
		if ((ch!=10)&&(ch!=13)) {
			if ((ch<32)||(ch>255)) { i++; continue; }
		}
		if (ch=='\"') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='q';
			buffer[bufferlength+2]='u';
			buffer[bufferlength+3]='o';
			buffer[bufferlength+4]='t';
			buffer[bufferlength+5]=';';
			bufferlength+=6;
			i++;
			continue;
		}
		if (ch=='&') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='a';
			buffer[bufferlength+2]='m';
			buffer[bufferlength+3]='p';
			buffer[bufferlength+4]=';';
			bufferlength+=5;
			i++;
			continue;
		}
		if (ch=='<') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='l';
			buffer[bufferlength+2]='t';
			buffer[bufferlength+3]=';';
			bufferlength+=4;
			i++;
			continue;
		}
		if (ch=='>') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='g';
			buffer[bufferlength+2]='t';
			buffer[bufferlength+3]=';';
			bufferlength+=4;
			i++;
			continue;
		}
		buffer[bufferlength]=ch;
		bufferlength++;
		i++;
	}
	return buffer;
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
	while (*ptemp) *ptemp++=tolower(*ptemp);
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
