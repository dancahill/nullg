/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2019 Dan Cahill

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
#include "nsp/nsplib.h"
#include "base.h"

NSP_FUNCTION(libnsp_base_rot13)
{
#define __FN__ __FILE__ ":libnsp_base_rot13()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *robj;
	char *p;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	robj = nsp_setstr(N, &N->r, "", cobj1->val->d.str, cobj1->val->size);
	if (robj->val->d.str == NULL) return 0;
	p = robj->val->d.str;
	while (*p) {
		if (nc_islower(*p)) {
			if (*p > 'm') *p -= 13; else *p += 13;
		}
		else if (nc_isupper(*p)) {
			if (*p > 'M') *p -= 13; else *p += 13;
		}
		p++;
	}
	return 0;
#undef __FN__
}
