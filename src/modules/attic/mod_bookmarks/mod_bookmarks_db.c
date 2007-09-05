/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "mod_bookmarks.h"

int dbread_bookmark(CONN *sid, short int perm, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "bookmarks");
	obj_t *cobj, *tobj;

	if (auth<1) return -1;
	if (!(auth&A_MODIFY)&&(perm==2)) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Bookmark',labeleduri='http://'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return 0;
	}
	if ((*qobj=ldir_getentry(sid->N, "bookmark", NULL, index, sid->dat->did))==NULL) return -1;
	if (ldir_numentries(qobj)!=1) { ldir_freeresult(qobj); return -2; }
	/* this would be a good time to check record permissions..... */
	return 0;
}

int dbread_bookmarkfolder(CONN *sid, short int perm, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "bookmarks");
	obj_t *cobj, *tobj;

	if (auth<1) return -1;
	if (!(auth&A_MODIFY)&&(perm==2)) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Folder'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return 0;
	}
	if ((*qobj=ldir_getentry(sid->N, "bookmarkfolder", NULL, index, sid->dat->did))==NULL) return -1;
	if (ldir_numentries(qobj)!=1) { ldir_freeresult(qobj); return -2; }
	return 0;
}

int dbwrite_bookmark(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "bookmarks");

	if (auth<2) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	return ldir_saveentry(sid, index, "bookmark", qobj);
}

int dbwrite_bookmarkfolder(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "bookmarks");

	if (auth<2) return -1;
	if (!(auth&A_INSERT)&&(index==0)) return -1;
	return ldir_saveentry(sid, index, "bookmarkfolder", qobj);
}
