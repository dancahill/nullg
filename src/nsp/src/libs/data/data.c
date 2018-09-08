/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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

int nspdata_register_all(nsp_state *N)
{
	obj_t *tobj, *tobj2, *tobj3;

	tobj = nsp_settable(N, &N->g, "data");
	tobj->val->attr |= NST_HIDDEN;

	tobj2 = nsp_settable(N, tobj, "cdb");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "read", (NSP_CFUNC)libnsp_data_cdb_read);
	nsp_setcfunc(N, tobj2, "write", (NSP_CFUNC)libnsp_data_cdb_write);
	tobj2 = nsp_settable(N, tobj, "csv");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "reader", (NSP_CFUNC)libnsp_data_csv_reader);
	tobj2 = nsp_settable(N, tobj, "dbf");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "reader", (NSP_CFUNC)libnsp_data_dbf_reader);

	tobj2 = nsp_settable(N, tobj, "json");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "encode", (NSP_CFUNC)libnsp_data_json_encode);

	tobj2 = nsp_settable(N, tobj, "sql");
	tobj2->val->attr |= NST_HIDDEN;
	tobj3 = nsp_settable(N, tobj2, "common");
	tobj3->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj3, "escape", (NSP_CFUNC)libnsp_data_sql_common_escape);
	nsp_setcfunc(N, tobj3, "begin", (NSP_CFUNC)libnsp_data_sql_common_notimplemented);
	nsp_setcfunc(N, tobj3, "commit", (NSP_CFUNC)libnsp_data_sql_common_notimplemented);
	nsp_setcfunc(N, tobj3, "rollback", (NSP_CFUNC)libnsp_data_sql_common_notimplemented);

	tobj2 = nsp_settable(N, tobj, "txt");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "reader", (NSP_CFUNC)libnsp_data_txt_reader);
	tobj2 = nsp_settable(N, tobj, "xml");
	tobj2->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj2, "read", (NSP_CFUNC)libnsp_data_xml_read);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspdata_register_all(N);
	return 0;
}
#endif
