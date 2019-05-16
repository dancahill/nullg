/*
    nsp.cgi -- simple Nesla CGI host
    Copyright (C) 2007-2019 Dan Cahill

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
#include "nsp/nsplib.h"
/*
#include "libbase.h"
#include "libcrypt.h"
#include "libnet.h"
#ifdef HAVE_REGEX
#include "libregex.h"
#endif
*/
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
	#include <io.h>
#ifdef BCC
	#define _setmode setmode
#endif
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
	#define sleep(x) Sleep(x*1000)
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <syslog.h>
	#include <unistd.h>
#endif
#include "config.h"

extern nsp_state *N;

/* cgi.c functions */
int   printhex(const char *format, ...);
int   printht(const char *format, ...);
char *b64decode(char *src);
char *b64encode(char *src);
int   IntFromHex(char *pChars);
void  URLDecode(char *pEncoded);
char *str2html(char *instring);
char *get_mime_type(char *name);
char *strcasestr(const char *src, const char *query);
void  cgi_readenv(void);
char *getgetenv(char *query);
char *getpostenv(char *query);
char *fileul(char *xfilename, char *xfilesize);
void  send_header(int cacheable, char *mime_type, int length, time_t mod);
/* config.c functions */
int   config_read(void);
/* main.c functions */
int   htnsp_runscript(char *file);
