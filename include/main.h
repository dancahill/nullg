/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
	#define HAVE_MYSQL
	#define HAVE_ODBC
	#define HAVE_PGSQL
	#define HAVE_SQLITE
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
	#include <io.h>
	#include <direct.h>
	#include "resource.h"
#ifdef BCC
	#define _setmode setmode
#endif
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#define HAVE_MYSQL
	#define HAVE_PGSQL
	#define HAVE_SQLITE
	#include <dirent.h>
	#include <dlfcn.h>
	#include <netdb.h>
	#include <paths.h>
	#include <pthread.h>
	#include <signal.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#define closesocket close
#endif
#ifdef HAVE_MYSQL
	#include "mysql.h"
#endif
#ifdef HAVE_ODBC
	#include <sql.h>
	#include <sqlext.h>
#endif
#ifdef HAVE_PGSQL
	#include "pgsql.h"
#endif
#ifdef HAVE_SQLITE
	#include "sqlite.h"
#endif

#include "language-en.h"
#include "defines.h"
#include "typedefs.h"
#include "functions.h"

#ifdef MAIN_GLOBALS
	LOCKS   Lock;
	_PROC   proc;
	CONN   *conn;
	SQLRES *sqlreply;
#else
	extern LOCKS   Lock;
	extern _PROC   proc;
	extern CONN   *conn;
	extern SQLRES *sqlreply;
#endif
