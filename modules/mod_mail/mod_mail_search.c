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
#include "http_mod.h"
#include "mod_mail.h"

char *search_makestring(CONN *sid)
{
	char *searchstring=getbuffer(sid);
	char *ptemp;

	searchstring[0]='\0';
	if ((ptemp=getgetenv(sid, "C"))!=NULL) {
		if (strcmp(ptemp, "addr")==0) {
			if ((ptemp=getgetenv(sid, "ADDR"))!=NULL) {
				snprintf(searchstring, sizeof(sid->dat->smallbuf[0])-1, "&c=addr&addr=%s", ptemp);
			}
		}
	}
	return searchstring;
}

int search_doquery(CONN *sid, const char *order_by, int folderid)
{
	char *ptemp;
	int sqr;

	if ((ptemp=getgetenv(sid, "C"))!=NULL) {
		if (strcmp(ptemp, "addr")==0) {
			if ((ptemp=getgetenv(sid, "ADDR"))!=NULL) {
				sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' OR hdr_to LIKE '%%%s%%' OR hdr_cc LIKE '%%%s%%' AND status != 'd' ORDER BY %s", sid->dat->user_uid, ptemp, ptemp, ptemp, order_by);
				return sqr;
			}
		}
	}
	sqr=sql_queryf("SELECT * FROM gw_mailheaders WHERE obj_uid = %d and accountid = %d AND folder = '%d' AND status != 'd' ORDER BY %s", sid->dat->user_uid, sid->dat->user_mailcurrent, folderid, order_by);
	return sqr;
}
