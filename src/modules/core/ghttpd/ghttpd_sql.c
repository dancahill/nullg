/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "ghttpd_main.h"

int sql_permprune(CONN *sid, SQLRES *sqr, char *permtype)
{
	SQLRES sqr1;
	unsigned int field;
	unsigned int tuple1;
	unsigned int tuple2;
	unsigned int numtuples;
	int i;
	int authlevel;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;

	if (sqr->NumFields<8) return -1;
	if (sql_queryf(&sqr1, "SELECT groupid FROM gw_groups_members where userid = %d AND obj_did = %d", sid->dat->user_uid, sid->dat->user_did)<0) return -1;
	numtuples=sqr->NumTuples;
	for (tuple1=0,tuple2=0;tuple1<numtuples;tuple1++) {
		obj_uid  =atoi(sqr->row[tuple1].field[3]);
		obj_gid  =atoi(sqr->row[tuple1].field[4]);
		obj_did  =atoi(sqr->row[tuple1].field[5]);
		obj_gperm=atoi(sqr->row[tuple1].field[6]);
		obj_operm=atoi(sqr->row[tuple1].field[7]);
		if (obj_did!=sid->dat->user_did) goto nomatch;
		if (permtype!=NULL) {
			authlevel=auth_priv(sid, permtype);
			if (authlevel<1) goto nomatch;
			if (authlevel&A_ADMIN) goto match;
		}
		if (obj_uid==sid->dat->user_uid) goto match;
		/*
		 * if there's a group match, we don't check "other".
		 * this lets us explicitly block access to specific groups.
		 */
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			if (obj_gid==atoi(sql_getvalue(&sqr1, i, 0))) {
				if (obj_gperm<1) {
					goto nomatch;
				} else {
					goto match;
				}
			}
		}
		/* end group check */
		if (obj_operm>0) goto match;
nomatch:
		for (field=0;field<sqr->NumFields;field++) {
			if (sqr->row[tuple1].field[field]!=NULL) {
				free(sqr->row[tuple1].field[field]);
				sqr->row[tuple1].field[field]=NULL;
			}
		}
//		free(sqr->row[tuple1]);
//		sqr->row[tuple1]=NULL;
		sqr->NumTuples--;
		continue;
match:
//		sqr->row[tuple2]=sqr->row[tuple1];
		if (tuple1!=tuple2) {
			memmove((char *)&sqr->row[tuple2], (char *)&sqr->row[tuple1], sizeof(SQLROW));
		}
		tuple2++;
		continue;

	}
	sql_freeresult(&sqr1);
	return 0;
}
