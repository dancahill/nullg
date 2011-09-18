/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include "smtpd_main.h"

static void conf_callback(char *var, char *val)
{
	if (strcmp(var, "interface")==0) {
		strncpy(mod_config.smtp_interface, val, sizeof(mod_config.smtp_interface)-1);
	} else if (strcmp(var, "relay_host")==0) {
		strncpy(mod_config.smtp_relayhost, val, sizeof(mod_config.smtp_relayhost)-1);
	} else if (strcmp(var, "host_name")==0) {
		strncpy(mod_config.smtp_hostname, val, sizeof(mod_config.smtp_hostname)-1);
	} else if (strcmp(var, "port")==0) {
		mod_config.smtp_port=atoi(val);
	} else if (strcmp(var, "ssl_port")==0) {
		mod_config.smtp_sslport=atoi(val);
	} else if (strcmp(var, "max_connections")==0) {
		mod_config.smtp_maxconn=atoi(val);
	} else if (strcmp(var, "max_idle")==0) {
		mod_config.smtp_maxidle=atoi(val);
	} else if (strcmp(var, "retry_delay")==0) {
		mod_config.smtp_retrydelay=atoi(val);
	} else if (strcmp(var, "pop_auth_window")==0) {
		mod_config.popauth_window=atoi(val);
	} else if (strcmp(var, "filter_program")==0) {
		strncpy(mod_config.filter_program, val, sizeof(mod_config.filter_program)-1);
	} else {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "unknown configuration directive '%s''%s'", var, val);
	}
	return;
}

int conf_read()
{
	memset((char *)&mod_config, 0, sizeof(mod_config));
	strncpy(mod_config.smtp_interface, "INADDR_ANY", sizeof(mod_config.smtp_interface)-1);
	strncpy(mod_config.smtp_relayhost, "",           sizeof(mod_config.smtp_relayhost)-1);
	mod_config.smtp_port       = 25;
	mod_config.smtp_sslport    = 465;
	mod_config.smtp_maxconn    = 50;
	mod_config.smtp_maxidle    = 120;
	mod_config.smtp_retrydelay = 300;
	mod_config.popauth_window  = 1800;
	config_read(proc->N, "smtpd", conf_callback);
	if (mod_config.smtp_retrydelay<60) {
		mod_config.smtp_retrydelay=60;
	}
	return 0;
}
