/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "main.h"

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "interface")==0) {
		strncpy(mod_config.http_interface, val, sizeof(mod_config.http_interface)-1);
	} else if (strcmp(var, "port")==0) {
		mod_config.http_port=atoi(val);
	} else if (strcmp(var, "ssl port")==0) {
		mod_config.http_sslport=atoi(val);
	} else if (strcmp(var, "max connections")==0) {
		mod_config.http_maxconn=atoi(val);
	} else if (strcmp(var, "max keepalive")==0) {
		mod_config.http_maxkeepalive=atoi(val);
	} else if (strcmp(var, "max idle")==0) {
		mod_config.http_maxidle=atoi(val);
	} else if (strcmp(var, "max post size")==0) {
		mod_config.http_maxpostsize=atoi(val);
	} else if (strcmp(var, "load module")==0) {
	} else {
		log_error("httpd", __FILE__, __LINE__, 1, "unknown configuration directive '%s'", var);
	}
	return;
}

static void conf_callback_modules(char *var, char *val)
{
	if (strcmp(var, "load module")==0) {
		module_load(val);
	}
	return;
}

int conf_read()
{
	memset((char *)&mod_config, 0, sizeof(mod_config));
	strncpy(mod_config.http_interface, "INADDR_ANY", sizeof(mod_config.http_interface)-1);
	mod_config.http_port         = 4110;
	mod_config.http_sslport      = 4112;
	mod_config.http_maxconn      = 50;
	mod_config.http_maxkeepalive = 15;
	mod_config.http_maxidle      = 120;
	mod_config.http_maxpostsize  = 32*1024*1024;
	config_read("srv_httpd", conf_callback);
	return 0;
}

int conf_read_modules()
{
	config_read("srv_httpd", conf_callback_modules);
	return 0;
}
