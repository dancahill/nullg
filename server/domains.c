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
#include "main.h"

char *domain_getname(char *outstring, int outlen, int domainid)
{
	int sqr;

	if ((sqr=sql_queryf("SELECT domainname FROM gw_domains WHERE domainid = %d", domainid))<0) return NULL;
	if (sql_numtuples(sqr)==1) {
		snprintf(outstring, outlen, "%s", sql_getvalue(sqr, 0, 0));
		outstring[outlen-1]='\0';
		sql_freeresult(sqr);
		return outstring;
	}
	sql_freeresult(sqr);
	return NULL;
}

int domain_getid(char *domainname)
{
	int domainid;
	int sqr;

	domainid=-1;
	if ((sqr=sql_queryf("SELECT domainid FROM gw_domains WHERE domainname = '%s'", domainname))<0) return -1;
	if (sql_numtuples(sqr)==1) {
		domainid=atoi(sql_getvalue(sqr, 0, 0));
		sql_freeresult(sqr);
		return domainid;
	}
	sql_freeresult(sqr);
	return -1;
}
