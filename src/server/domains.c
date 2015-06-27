/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

char *domain_getname(char *outstring, int outlen, int domainid)
{
	obj_t *qobj=NULL;

//	if (sql_queryf(proc.N, &qobj, "SELECT did FROM nullsd_entries WHERE class = 'organization' AND did = %d", domainid)<0) return NULL;
	if (sql_queryf(proc.N, &qobj, "SELECT domainname FROM gw_domains WHERE domainid = %d", domainid)<0) return NULL;
	if (sql_numtuples(proc.N, &qobj)>0) {
		strncpy(outstring, sql_getvalue(proc.N, &qobj, 0, 0), outlen-1);
		sql_freeresult(proc.N, &qobj);
		return outstring;
	}
	sql_freeresult(proc.N, &qobj);
	return NULL;
}

int domain_getid(char *domainname)
{
	int domainid=-1;
	obj_t *qobj=NULL;

//	if (sql_queryf(proc.N, &qobj, "SELECT did FROM nullsd_entries WHERE class = 'associateddomain' AND name = '%s'", domainname)<0) return -1;
	if (sql_queryf(proc.N, &qobj, "SELECT domainid FROM gw_domains WHERE domainname = '%s'", domainname)<0) return -1;
	if (sql_numtuples(proc.N, &qobj)>0) domainid=atoi(sql_getvalue(proc.N, &qobj, 0, 0));
	sql_freeresult(proc.N, &qobj);
	if (domainid>0) return domainid;

//	if (sql_queryf(proc.N, &qobj, "SELECT did FROM nullsd_entries WHERE class = 'associateddomain' AND name = '%s'", domainname)<0) return -1;
	if (sql_queryf(proc.N, &qobj, "SELECT domainid FROM gw_domains_aliases WHERE domainname = '%s'", domainname)<0) return -1;
	if (sql_numtuples(proc.N, &qobj)>0) domainid=atoi(sql_getvalue(proc.N, &qobj, 0, 0));
	sql_freeresult(proc.N, &qobj);
	return domainid;
}
