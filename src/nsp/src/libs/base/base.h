/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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

#if defined(_WIN32)
#include "nsp/config-win.h"
#else
#include "nsp/config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_HAVE_THREADS) && !defined(HAVE_THREADS)
#define HAVE_THREADS
#endif

#ifdef HAVE_THREADS

#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#endif

typedef struct {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
				/* now begin the stuff that's mutex-specific */
#ifdef _WIN32
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
} OS_MUTEX;

typedef struct {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
				/* now begin the stuff that's thread-specific */
#ifdef _WIN32
	HANDLE handle;
	unsigned int id;
#else
	pthread_t handle;
	pthread_t id;
#endif
	nsp_state *N;
	nsp_state *parentN;
	obj_t this;
} OS_THREAD;
#endif

/* base64.c */
NSP_FUNCTION(libnsp_base_base64_decode);
NSP_FUNCTION(libnsp_base_base64_encode);
/* dir.c */
NSP_FUNCTION(libnsp_base_dirlist);
/* file.c */
NSP_FUNCTION(libnsp_base_file_chdir);
NSP_FUNCTION(libnsp_base_file_mkdir);
NSP_FUNCTION(libnsp_base_file_readall);
NSP_FUNCTION(libnsp_base_file_rename);
NSP_FUNCTION(libnsp_base_file_stat);
NSP_FUNCTION(libnsp_base_file_unlink);
NSP_FUNCTION(libnsp_base_file_writeall);
/* pipe.c */
#ifdef HAVE_PIPE
NSP_FUNCTION(libnsp_base_pipe_open);
NSP_FUNCTION(libnsp_base_pipe_read);
NSP_FUNCTION(libnsp_base_pipe_write);
NSP_FUNCTION(libnsp_base_pipe_close);
#endif
/* regex.c */
NSP_FUNCTION(libnsp_regex_match);
NSP_FUNCTION(libnsp_regex_replace);
/* rot13.c */
NSP_FUNCTION(libnsp_base_rot13);
/* sort.c */
NSP_FUNCTION(libnsp_base_sort_byname);
NSP_FUNCTION(libnsp_base_sort_bykey);
/* thread.c */
#ifdef HAVE_THREADS
NSP_FUNCTION(libnsp_base_thread_mutex_mutex);
NSP_FUNCTION(libnsp_base_thread_mutex_lock);
NSP_FUNCTION(libnsp_base_thread_mutex_unlock);
NSP_FUNCTION(libnsp_base_thread_mutex_free);
NSP_FUNCTION(libnsp_base_thread_finish);
NSP_FUNCTION(libnsp_base_thread_kill);
NSP_FUNCTION(libnsp_base_thread_start);
NSP_FUNCTION(libnsp_base_thread_thread);
#endif
/* winapi.c */
#ifdef _WIN32
NSP_FUNCTION(libnsp_winapi_beep);
NSP_FUNCTION(libnsp_winapi_createprocess);
NSP_FUNCTION(libnsp_winapi_messagebox);
NSP_FUNCTION(libnsp_winapi_playsound);
NSP_FUNCTION(libnsp_winapi_shellexecute);
#endif

int nspbase_register_all(nsp_state *N);

#ifdef __cplusplus
}
#endif
