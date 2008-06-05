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
#include "pop3d_main.h"

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "interface")==0) {
		strncpy(mod_config.pop3_interface, val, sizeof(mod_config.pop3_interface)-1);
	} else if (strcmp(var, "port")==0) {
		mod_config.pop3_port=atoi(val);
	} else if (strcmp(var, "ssl_port")==0) {
		mod_config.pop3_sslport=atoi(val);
	} else if (strcmp(var, "max_connections")==0) {
		mod_config.pop3_maxconn=atoi(val);
	} else if (strcmp(var, "max_idle")==0) {
		mod_config.pop3_maxidle=atoi(val);
	} else {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "unknown configuration directive '%s''%s'", var, val);
	}
	return;
}

int conf_read()
{
	memset((char *)&mod_config, 0, sizeof(mod_config));
	strncpy(mod_config.pop3_interface, "INADDR_ANY", sizeof(mod_config.pop3_interface)-1);
	mod_config.pop3_port         = 110;
	mod_config.pop3_sslport      = 995;
	mod_config.pop3_maxconn      = 50;
	mod_config.pop3_maxidle      = 120;
	config_read(proc->N, "pop3d", conf_callback);
	return 0;
}
