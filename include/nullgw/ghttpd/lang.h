/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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

#define STR_LEN 256
#define COMMON_LANG_TYPES \
	char charset_html[STR_LEN];\
	char charset_mime[STR_LEN];\
	char err_noaccess[STR_LEN];\
	char err_norecord[STR_LEN];\
	char warn_delconfirm[STR_LEN];\
	char form_delete[STR_LEN];\
	char form_reset[STR_LEN];\
	char form_save[STR_LEN];
#define COMMON_LANG_SYMS \
	{ "HTML_CHARSET",    lang.charset_html    },\
	{ "MIME_CHARSET",    lang.charset_mime    },\
	{ "ERR_NOACCESS",    lang.err_noaccess    },\
	{ "ERR_NORECORD",    lang.err_norecord    },\
	{ "WARN_DELCONFIRM", lang.warn_delconfirm },\
	{ "FORM_DELETE",     lang.form_delete     },\
	{ "FORM_RESET",      lang.form_reset      },\
	{ "FORM_SAVE",       lang.form_save       },
