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

void htselect_timezone(CONNECTION *sid, short int selected)
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

char *time_sql2text(CONNECTION *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2datetext(CONNECTION *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_sql2timetext(CONNECTION *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2lotimetext(CONNECTION *sid, char *sqldate)
{
	char *buffer=getbuffer(sid);
	char *ptemp=buffer;
	time_t unixdate=time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	while (*ptemp) *ptemp++=tolower(*ptemp);
	return buffer;
}

char *time_unix2sql(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *time_unix2sqldate(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%Y-%m-%d", gmtime(&unixdate));
	return buffer;
}

char *time_unix2sqltime(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *time_unix2text(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 50, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2datetext(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_unix2timetext(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2lotimetext(CONNECTION *sid, time_t unixdate)
{
	char *buffer=getbuffer(sid);
	char *ptemp=buffer;

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	while (*ptemp) *ptemp++=tolower(*ptemp);
	return buffer;
}

int time_tzoffset(CONNECTION *sid, time_t unixdate)
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

int time_tzoffset2(CONNECTION *sid, time_t unixdate, int userid)
{
	int sqr;
	int tz=-1;
	time_t tzoffset;
	struct tm *today;

	if ((sqr=sqlQueryf(sid, "SELECT preftimezone FROM gw_users where userid = %d", userid))<0) return 0;
	if (sqlNumtuples(sqr)==1) {
		tz=atoi(sqlGetvalue(sqr, 0, 0));
	}
	sqlFreeconnect(sqr);
	if ((tz<0)||(tz>61)) return 0;
	tzoffset=timezones[tz].minutes*60;
	today=localtime(&unixdate);
	if ((timezones[tz].dst)&&(today->tm_isdst)) {
		tzoffset+=3600;
	}
	return tzoffset;
}

time_t wmgetdate(char *src)
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
