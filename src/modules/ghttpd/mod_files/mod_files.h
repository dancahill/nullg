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
#include "nullgw/ghttpd/mod.h"
#include "mod_files_lang.h"

/* mod_files_conf.c */
int conf_read(void);
/* mod_files_db.c */
int dbread_file(CONN *sid, short int perm, int index, REC_FILE *file);
int fileperm(CONN *sid, int perm, char *dir, char *file);
/* mod_files_dir.c */
int dirlist(CONN *sid);
int filemkdir(CONN *sid);
int filedirsave(CONN *sid);
/* mod_files_file.c */
int fileul(CONN *sid);
int filerecv(CONN *sid);
void fileedit(CONN *sid);
void filesave(CONN *sid);
/* mod_files_lang.c */
int lang_read(void);

typedef struct {
	char filter_program[256];
} MOD_CONFIG;

#ifdef SRVMOD_MAIN
	MOD_CONFIG mod_config;
#else
	extern MOD_CONFIG mod_config;
#endif
