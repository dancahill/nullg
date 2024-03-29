/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#include "data.h"

NSP_FUNCTION(libnsp_data_sql_common_escape)
{
#define __FN__ __FILE__ ":libnsp_data_sql_common_escape()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");

	char *ss, *se;
	char *s2;
	int l2;

	if (!nsp_isstr(cobj1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nsp_setstr(N, &N->r, "", NULL, 0);
	if (cobj1->val->d.str == NULL) return 0;
	se = ss = cobj1->val->d.str;
	s2 = "'";
	l2 = 1;
	for (; *se; se++) {
		if (nc_strncmp(se, s2, l2) != 0) continue;
		nsp_strcat(N, &N->r, ss, (unsigned long)(se - ss));
		nsp_strcat(N, &N->r, "''", 2);
		ss = se += l2;
		if (*se) { --se; continue; }
		nsp_strcat(N, &N->r, ss, (unsigned long)(se - ss));
		break;
	}
	if (se > ss) {
		nsp_strcat(N, &N->r, ss, (unsigned long)(se - ss));
	}
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_data_sql_common_notimplemented)
{
#define __FN__ __FILE__ ":libnsp_data_sql_common_notimplemented()"
	n_error(N, NE_SYNTAX, __FN__, "method is not implemented");
	return 0;
#undef __FN__
}
