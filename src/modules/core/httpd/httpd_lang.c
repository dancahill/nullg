/*
    NullLogic Groupware - Copyright (C) 2000-2007 Dan Cahill

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
#include "httpd_main.h"

char *lang_gets(CONN *sid, char *sect, char *label)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *cobj, *tobj;
	struct stat sb;
	char fname[512];
//	jmp_buf *savjmp;

	if ((sid==NULL)||(sid->N==NULL)) return "";
	if (strlen(sid->dat->language)==0) return "";
	if (strlen(sect)==0) return "";
	tobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	tobj=nes_settable(sid->N, tobj, "LANGUAGE");
	tobj->val->attr|=NST_HIDDEN;
	/*
	 * it's hidden because it's ugly.  not because knowing the
	 * french word for 'inbox' would be a security issue.
	 */
	tobj=nes_settable(sid->N, tobj, sid->dat->language);
	tobj=nes_settable(sid->N, tobj, sect);
	cobj=nes_getobj(sid->N, tobj, label);
	if (cobj->val->type!=NT_NULL) return nes_tostr(sid->N, cobj);
	/* sym not found, so let's go hunting */
	snprintf(fname, sizeof(fname)-1, "%s/share/locale/%s/%s.ns", nes_getstr(proc->N, confobj, "var_path"), sid->dat->language, sect);
	fixslashes(fname);
	if (stat(fname, &sb)==0) {
//		savjmp=sid->N->savjmp;
//		sid->N->savjmp=calloc(1, sizeof(jmp_buf));
//		if (setjmp(*sid->N->savjmp)==0) {
//			sid->N->jmpset=1;
			nes_execfile(sid->N, fname);
//		}
//		sid->N->jmpset=0;
//		free(sid->N->savjmp);
//		sid->N->savjmp=savjmp;
	}
	cobj=nes_getobj(sid->N, tobj, label);
	return nes_tostr(sid->N, cobj);
}
