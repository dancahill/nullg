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
#ifdef WIN32
#include "nullgw/config-nt.h"
#else
#include "nullgw/config.h"
#endif
/* #includes */
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
	//#define HAVE_FBSQL
	#define HAVE_MYSQL
	#define HAVE_ODBC
	#define HAVE_PGSQL
	#define HAVE_SQLITE2
	#define HAVE_SQLITE3
	#define WIN32_LEAN_AND_MEAN
	#define _MT 1
	#pragma comment(lib, "libcmt.lib")
	#pragma comment(lib, "ws2_32.lib")
	#include <winsock2.h>
	#include <windows.h>
	#include <process.h>
	#include <shellapi.h>
	#include <signal.h>
	#include <windowsx.h>
	#include <basetsd.h>
	#include <io.h>
	#include <direct.h>
#ifdef BCC
	#define _setmode setmode
#endif
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp _stricmp
	#define strncasecmp strnicmp
#else
	#define HAVE_FBSQL
	#define HAVE_MYSQL
	#define HAVE_PGSQL
	#define HAVE_SQLITE2
	#define HAVE_SQLITE3
	#include <dirent.h>
	#include <netdb.h>
#ifdef HAVE_PATHS_H
	#include <paths.h>
#endif
#ifdef HAVE_PTHREAD_MIT_PTHREAD_H
	#include <pthread/mit/pthread.h>
#else
	#include <pthread.h>
#endif
	#include <signal.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#define closesocket close
#endif
#include "nullgw/common/ssl.h"

#include "nullgw/common/defines.h"
#include "nullgw/common/typedefs.h"
#include "functions.h"

#ifdef MAIN_GLOBALS
	LOCKS   Lock;
	_PROC   proc;
	pthread_t CronThread;
	pthread_t ListenThread;
#else
	extern LOCKS   Lock;
	extern _PROC   proc;
	extern pthread_t CronThread;
	extern pthread_t ListenThread;
#endif
