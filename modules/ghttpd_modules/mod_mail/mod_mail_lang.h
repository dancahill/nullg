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
#include "ghttpd/lang.h"

typedef struct {
	/* common strings */
	COMMON_LANG_TYPES
	/* this module */
	char tab_gen[STR_LEN];
	char tab_ser[STR_LEN];
	char tab_adv[STR_LEN];
	char tab_sig[STR_LEN];
	char tab_fld[STR_LEN];
	char tab_fil[STR_LEN];
	char folder1_name[STR_LEN];
	char folder2_name[STR_LEN];
	char folder3_name[STR_LEN];
	char folder4_name[STR_LEN];
	char folder5_name[STR_LEN];
	char folder6_name[STR_LEN];
	char folder7_name[STR_LEN];
	char mail_moveto[STR_LEN];
	char mail_from[STR_LEN];
	char mail_to[STR_LEN];
	char mail_cc[STR_LEN];
	char mail_bcc[STR_LEN];
	char mail_subject[STR_LEN];
	char mail_date[STR_LEN];
	char mail_size[STR_LEN];
	char mail_viewsource[STR_LEN];
	char mail_prev[STR_LEN];
	char mail_next[STR_LEN];
	char mail_prevpage[STR_LEN];
	char mail_nextpage[STR_LEN];
	char mail_reply[STR_LEN];
	char mail_replyall[STR_LEN];
	char mail_forward[STR_LEN];
	char mail_delete[STR_LEN];
	char mail_checking[STR_LEN];
	char mail_ok[STR_LEN];
	char mail_sent[STR_LEN];
	char mail_err_dns_pop3[STR_LEN];
	char mail_err_dns_smtp[STR_LEN];
	char mail_err_con_pop3[STR_LEN];
	char mail_err_con_smtp[STR_LEN];
	char mail_err_connreset[STR_LEN];
	char mail_err_box_lock[STR_LEN];
	char mail_err_nomsg[STR_LEN];
} STRINGS;
STRINGS lang;
