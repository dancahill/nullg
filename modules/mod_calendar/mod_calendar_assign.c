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
#include "mod_substub.h"
#include "mod_calendar.h"

char *db_avail_getweek(CONN *sid, int userid, char *availability)
{
	char availbuff[170];
	char gavailability[673];
	char uavailability[673];
	int sqr;
	int groupid;
	int i;

	if ((sqr=sql_queryf(sid, "SELECT groupid, availability FROM gw_users WHERE userid = %d", userid))<0) return NULL;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return NULL;
	}
	memset(gavailability, 0, sizeof(gavailability));
	memset(uavailability, 0, sizeof(uavailability));
	groupid=atoi(sql_getvalue(sqr, 0, 0));
	memset(availbuff, 0, sizeof(availbuff));
	strncpy(availbuff, sql_getvalue(sqr, 0, 1), sizeof(availbuff)-1);
	sql_freeresult(sqr);
	if (strlen(availbuff)!=168) {
		for (i=0;i<672;i++) {
			uavailability[i]='0';
		}
	} else {
		for (i=0;i<672;i++) {
			uavailability[i]=availbuff[(int)(i/4)];
		}
	}
	if ((sqr=sql_queryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", groupid))<0) return NULL;
	if (sql_numtuples(sqr)!=1) {
		sql_freeresult(sqr);
		return NULL;
	}
	memset(availbuff, 0, sizeof(availbuff));
	strncpy(availbuff, sql_getvalue(sqr, 0, 0), sizeof(availbuff)-1);
	sql_freeresult(sqr);
	if (strlen(availbuff)!=168) {
		for (i=0;i<672;i++) {
			gavailability[i]='0';
		}
	} else {
		for (i=0;i<672;i++) {
			gavailability[i]=availbuff[(int)(i/4)];
		}
	}
	for (i=0;i<672;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='0';
		}
	}
	snprintf(availability, 673, "%s", uavailability);
	return availability;
}

char *db_avail_getfullweek(CONN *sid, int userid, int record, time_t eventstart, char *availability)
{
	char timebuf1[40];
	char timebuf2[40];
	int i, i2, j, k;
	int sqr;
	struct tm t1;
	struct tm t2;
	time_t eventfinish;

//	You MUST call db_avail_getweek() first!
	if (strlen(availability)!=672) return NULL;
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&eventstart));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	eventstart=((int)(eventstart/86400)*86400)-(t1.tm_wday*86400);
	eventstart-=time_tzoffset2(sid, eventstart, userid);
	eventfinish=eventstart+604800;
	strftime(timebuf1, sizeof(timebuf1)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventstart));
	strftime(timebuf2, sizeof(timebuf2)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventfinish));
	if (strcmp(config->sql_type, "ODBC")==0) {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= #%s# and eventstart < #%s#) or (eventfinish > #%s# and eventfinish < #%s#) or (eventstart < #%s# and eventfinish >= #%s#)) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return NULL;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT eventid, eventstart, eventfinish FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= '%s' and eventstart < '%s') or (eventfinish > '%s' and eventfinish < '%s') or (eventstart < '%s' and eventfinish >= '%s')) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return NULL;
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		eventstart=time_sql2unix(sql_getvalue(sqr, i, 1));
		eventstart+=time_tzoffset2(sid, eventstart, userid);
		eventfinish=time_sql2unix(sql_getvalue(sqr, i, 2));
		eventfinish+=time_tzoffset2(sid, eventfinish, userid);
		strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&eventstart));
		sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
		strftime(timebuf2, sizeof(timebuf2), "%w %H %M", gmtime(&eventfinish));
		sscanf(timebuf2, "%d %d %d", &t2.tm_wday, &t2.tm_hour, &t2.tm_min);
		j=(t1.tm_wday*96)+(t1.tm_hour*4)+(t1.tm_min/15);
		k=(t2.tm_wday*96)+(t2.tm_hour*4)+(t2.tm_min/15);
		if (k<j) k=j;
		if (k==j) k++;
		for (i2=j;i2<k;i2++) availability[i2]='0';
	}
	sql_freeresult(sqr);
//	logerror(sid, __FILE__, __LINE__, "================================================================================================");
//	logerror(sid, __FILE__, __LINE__, "000011112222333344445555666677778888999900001111222233334444555566667777888899990000111122223333");
//	for (i=0;i<7;i++) { ptemp=availability+(i*96); logerror(sid, __FILE__, __LINE__, "%.96s", ptemp); }
//	logerror(sid, __FILE__, __LINE__, "================================================================================================");
	return availability;
}

int db_availcheck(CONN *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	char availability[673];
	char timebuf1[40];
	char timebuf2[40];
	char *ptemp;
	int i, j, k;
	int ret;
	int sqr;
	time_t ts, tf;
	struct tm t1;
	struct tm t2;

	if (!busy) return 0;
	memset(availability, 0, sizeof(availability));
	ts=eventstart+time_tzoffset2(sid, eventstart, userid);
	tf=eventfinish+time_tzoffset2(sid, eventfinish, userid);
	if ((ptemp=db_avail_getweek(sid, userid, availability))==NULL) return -1;
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&ts));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	strftime(timebuf2, sizeof(timebuf2), "%w %H %M", gmtime(&tf));
	sscanf(timebuf2, "%d %d %d", &t2.tm_wday, &t2.tm_hour, &t2.tm_min);
	j=(t1.tm_wday*96)+(t1.tm_hour*4)+(t1.tm_min/15);
	k=(t2.tm_wday*96)+(t2.tm_hour*4)+(t2.tm_min/15);
	if (k<j) k=j;
	if (k==j) k++;
	for (i=j;i<k;i++) {
		if (availability[i]!='1') return -1;
	}
	if ((ptemp=db_avail_getfullweek(sid, userid, record, eventstart, availability))==NULL) return -2;
	for (i=j;i<k;i++) {
		if (availability[i]=='1') continue;
		strftime(timebuf1, sizeof(timebuf1)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventstart));
		strftime(timebuf2, sizeof(timebuf2)-1, "%Y-%m-%d %H:%M:%S", gmtime(&eventfinish));
		if (strcmp(config->sql_type, "ODBC")==0) {
			if ((sqr=sql_queryf(sid, "SELECT eventid FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= #%s# and eventstart < #%s#) or (eventfinish > #%s# and eventfinish < #%s#) or (eventstart < #%s# and eventfinish >= #%s#)) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return -1;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT eventid FROM gw_events where eventid <> %d and busy <> 0 and ((eventstart >= '%s' and eventstart < '%s') or (eventfinish > '%s' and eventfinish < '%s') or (eventstart < '%s' and eventfinish >= '%s')) and assignedto = %d", record, timebuf1, timebuf2, timebuf1, timebuf2, timebuf1, timebuf2, userid))<0) return -1;
		}
		if (sql_numtuples(sqr)>0) {
			ret=atoi(sql_getvalue(sqr, 0, 0));
			sql_freeresult(sqr);
			return ret;
		}
	}
	return 0;
}

int db_autoschedule(CONN *sid, int userid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	char availability[673];
	char availweekbuf[673];
	char timeblock[100];
	char timebuf1[40];
	char *ptemp;
	int i, j;
	int week=0;
	struct tm t1;
	time_t ts;
	time_t newtime;
//	int lasttz=time_tzoffset2(sid, eventstart, userid);

	if (!busy) return eventstart;
	memset(timeblock, 0, sizeof(timeblock));
	memset(availability, 0, sizeof(availability));
	if ((ptemp=db_avail_getweek(sid, userid, availability))==NULL) return -1;
	memcpy(availweekbuf, availability, sizeof(availweekbuf));
	i=(int)((eventfinish-eventstart)/900);
	if (i<0) i=0;
	if (i>96) return -1;
	for (j=0;j<i;j++) timeblock[j]='1';
	if ((ptemp=strstr(availability, timeblock))==NULL) return -1;
	ts=eventstart+time_tzoffset2(sid, eventstart, userid);
	strftime(timebuf1, sizeof(timebuf1), "%w %H %M", gmtime(&ts));
	sscanf(timebuf1, "%d %d %d", &t1.tm_wday, &t1.tm_hour, &t1.tm_min);
	newtime=((int)(ts/86400)*86400)-(t1.tm_wday*86400);
	do {
/*
		if (lasttz!=time_tzoffset(sid, newtime+10800)) {
			lasttz=time_tzoffset(sid, newtime+10800);
//			memset(availability, 0, sizeof(availability));
//			if ((ptemp=db_avail_getweek(sid, userid, availability))==NULL) return -1;
		}
*/
		memcpy(availweekbuf, availability, sizeof(availweekbuf));
		if ((ptemp=db_avail_getfullweek(sid, userid, record, newtime, availweekbuf))==NULL) return -1;
		if (week==0) {
			if ((ptemp=strstr(availweekbuf+((ts/900)-(newtime/900)), timeblock))==NULL) {
				newtime+=604800;
				week++;
				continue;
			}
		} else {
			if ((ptemp=strstr(availweekbuf, timeblock))==NULL) {
				newtime+=604800;
				week++;
				continue;
			}
		}
		newtime-=time_tzoffset2(sid, newtime, userid);
		newtime+=(strlen(availweekbuf)-strlen(ptemp))*900;
		return newtime;
	} while (1);
	return newtime;
}

int db_autoassign(CONN *sid, u_avail *uavail, int groupid, int zoneid, int record, int busy, time_t eventstart, time_t eventfinish)
{
	int i;
	int j;
	int rc;
	int sqr;

	uavail->userid=0;
	uavail->time=0;
	if (zoneid==0) {
		if ((sqr=sql_queryf(sid, "SELECT userid FROM gw_users where groupid = %d", groupid))<0) return -1;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT userid FROM gw_users where groupid = %d and prefgeozone = %d", groupid, zoneid))<0) return -1;
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		if ((rc=db_autoschedule(sid, j, record, busy, eventstart, eventfinish))<0) continue;
		if ((rc<uavail->time)||(uavail->time==0)) {
			uavail->userid=j;
			uavail->time=rc;
		}
	}
	sql_freeresult(sqr);
	if (uavail->userid<=0) return -1;
	if (uavail->time<=0) return -1;
	return 0;
}
