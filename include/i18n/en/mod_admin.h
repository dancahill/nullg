/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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

#include "modules.h"

/* ADMINISTRATION STRINGS */
#define ADM_TITLE	"NullLogic Groupware Administration"
#define ADM_CFG_TITLE	"Server Configuration"
#define ADM_CFG_SRVBASE	"Server Base Directory"
#define ADM_CFG_SRVBIN	"Server Bin Directory"
#define ADM_CFG_SRVCGI	"Server CGI Directory"
#define ADM_CFG_SRVETC	"Server Etc Directory"
#define ADM_CFG_SRVLIB	"Server Lib Directory"
#define ADM_CFG_SRVVAR	"Server Var Directory"
#define ADM_CFG_SRVBAK	"Server Backup Directory"
#define ADM_CFG_SRVDB	"Server DB Directory"
#define ADM_CFG_SRVFILE	"Server Files Directory"
#define ADM_CFG_SRVHDOC	"Server Htdocs Directory"
#define ADM_CFG_SRVLOGS	"Server Log Directory"
#define ADM_CFG_SRVMAIL	"Server Mail Directory"
#define ADM_CFG_SRVLOG	"Server Log Level"
#define ADM_CFG_SRVHOST	"Server Host Address"
#define ADM_CFG_SRVPORT	"Server Port"
#define ADM_CFG_SQLTYPE	"SQL Server Type"
#define ADM_CFG_SQLHOST	"SQL Server Host Address"
#define ADM_CFG_SQLPORT	"SQL Server Port"
#define ADM_CFG_SQLNAME	"SQL Database Name"
#define ADM_CFG_SQLUSER	"SQL Database Username"
#define ADM_CFG_SQLPASS	"SQL Database Password"
#define ADM_CFG_SQLODBC	"SQL ODBC Data Source"
#define ADM_CFG_SAVED \
"Groupware configuration modified successfully.\n" \
"In order to make sure all settings take effect, you\n" \
"may need to restart the server.\n"
#define ADM_CFG_LOG	"%s - %s modified configuration"
#define ADM_CFG_NOFILE	"ERROR: Cannot write to configuration file %s."
