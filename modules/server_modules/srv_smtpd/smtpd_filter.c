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
#include "smtpd_main.h"

int filter_scan(CONN *sid, char *msgfilename)
{
	short int err;

	if (strlen(mod_config.filter_program)<1) return 0;
	err=sys_system("%s %s", mod_config.filter_program, msgfilename);
	if (err>255) err=err>>8;
	if ((err==1)||(err==2)||(err==3)) {
		if (err==1) {
			log_error("smtpd", __FILE__, __LINE__, 1, "[%s][SPAM]", sid->dat->user_RemoteAddr);
		} else if (err==2) {
			log_error("smtpd", __FILE__, __LINE__, 1, "[%s][VIRUS]", sid->dat->user_RemoteAddr);
		} else if (err==3) {
			log_error("smtpd", __FILE__, __LINE__, 1, "[%s][VIRUS+SPAM]", sid->dat->user_RemoteAddr);
		}
		return err;
	}
	return 0;
}
