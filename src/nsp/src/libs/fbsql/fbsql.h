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

#if defined(_WIN32)
#include "nsp/config-win.h"
#else
#include "nsp/config.h"
#endif

#if defined(CONFIG_HAVE_FBSQL) && !defined(HAVE_FBSQL)
#define HAVE_FBSQL
#endif

#ifdef HAVE_FBSQL
#if defined(_WIN32)
#pragma comment(lib, "fbclient_ms.lib")
#pragma comment(lib, "ib_util_ms.lib")
#endif
int nspfbsql_register_all(nsp_state *N);
#endif /* HAVE_FBSQL */
