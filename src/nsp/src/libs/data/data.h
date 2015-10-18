/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
#elif !defined( __TURBOC__)
#include <unistd.h>
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* cdb.c */
NSP_FUNCTION(libnsp_data_cdb_read);
NSP_FUNCTION(libnsp_data_cdb_write);
/* csv.c */
NSP_FUNCTION(libnsp_data_csv_reader);
/* dbf.c */
NSP_CLASS(libnsp_data_dbf_reader);
/* sql.c */
NSP_FUNCTION(libnsp_data_sql_common_escape);
NSP_FUNCTION(libnsp_data_sql_common_notimplemented);
/* txt.c */
NSP_FUNCTION(libnsp_data_txt_reader);
/* xml.c */
NSP_FUNCTION(libnsp_data_xml_read);

int nspdata_register_all(nsp_state *N);
