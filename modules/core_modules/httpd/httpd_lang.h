/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "httpd/lang.h"

typedef struct {
	/* common strings */
	COMMON_LANG_TYPES
	/* this module */
	char menu_adm_config[STR_LEN];
	char menu_adm_logs[STR_LEN];
	char menu_adm_check[STR_LEN];
	char menu_adm_users[STR_LEN];
	char menu_adm_groups[STR_LEN];
	char menu_adm_zones[STR_LEN];
	char menu_top_help[STR_LEN];
	char menu_top_print[STR_LEN];
	char menu_top_logout[STR_LEN];
} STRINGS;
STRINGS lang;
