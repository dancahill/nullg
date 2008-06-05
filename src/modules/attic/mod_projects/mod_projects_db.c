/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_projects.h"

int dbread_project(CONN *sid, short int perm, int index, obj_t **qobj)
{
	short int approved;
	int auth=auth_priv(sid, "projects");
	obj_t *cobj, *tobj;

	if (auth<1) return EC_NOPERM;
	if (!(auth&A_MODIFY)&&(perm==2)) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	if (index==0) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
/*
		project->projectadmin=sid->dat->uid;
		snprintf(project->projectname, sizeof(project->projectname)-1, "New Project");
		project->projectstart=time(NULL);
		project->projectfinish=time(NULL);
		return 0;
*/
		cobj=nes_evalf(sid->N, "{_tuples=1,_rows={[0]={id=0,pid=%d,did=%d,_data={cn='New Project'}}}}", sid->dat->did, sid->dat->did);
		nes_linkval(sid->N, tobj, cobj);
		return EC_NOERROR;
	}
	if ((tobj=ldir_getentry(sid->N, "project", NULL, index, sid->dat->did))==NULL) return EC_NORECORD;
	if (ldir_numentries(&tobj)!=1) { ldir_freeresult(&tobj); return EC_NORECORD; }
	if (*qobj) {
		cobj=*qobj;
		cobj->val->d.table=tobj->val->d.table;
	} else {
		*qobj=tobj;
	}
	/* this would be a good time to check record permissions..... */
	approved=0;
	return approved;
}

int dbwrite_project(CONN *sid, int index, obj_t **qobj)
{
	int auth=auth_priv(sid, "projects");

	if (auth<2) return EC_NOPERM;
	if (!(auth&A_INSERT)&&(index==0)) return EC_NOPERM;
	return ldir_saveentry(sid, index, "project", qobj);
}
