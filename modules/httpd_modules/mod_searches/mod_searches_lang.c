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
#include "http_mod.h"
#include "mod_searches.h"

typedef struct {
	char *key;
	char *val;
} LANGPAIR;

static LANGPAIR langpair[]={
	/* common strings */
	COMMON_LANG_SYMS
	/* this module */
	{ NULL, NULL }
};

static void lang_callback(char *var, char *val)
{
	int found=0;
	int i;

	for (i=0;;i++) {
		if (langpair[i].key==NULL) break;
		if (strcmp(var, langpair[i].key)==0) {
			if (langpair[i].val!=NULL) {
				strncpy(langpair[i].val, val, STR_LEN-1);
			}
			found=1;
		}
	}
	if (!found) {
		log_error("lang", __FILE__, __LINE__, 1, "unknown language string '%s'", var);
	}
	return;
}

int lang_read()
{
	memset((char *)&lang, 0, sizeof(lang));
	language_read(config->langcode, "mod_searches", lang_callback);
	return 0;
}
