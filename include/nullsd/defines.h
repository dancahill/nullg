/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

#define SERVER_NAME	"NullLogic GroupServer"
#define SERVER_BASENAME	"nullsd"
#define SERVICE_NAME	"nullsd"

#define DEFAULT_SERVER_USERNAME	"nullsd"
#define DEFAULT_SERVER_LANGUAGE	"en"

#define MAX_MOD_FUNCTIONS	50
#define MAXLISTENERS 16

/* 64k stacks seem to be too small for select() + SQLite - does this pair make my local buffer look fat? */
/* #define PTHREAD_STACK_SIZE	65536L */
#define PTHREAD_STACK_SIZE	98304L
/* #define PTHREAD_STACK_SIZE	131072L */

/* SQL limits */
#define SQL_MAXFIELDS		64
#define MAX_TUPLE_SIZE		65536
#define MAX_FIELD_SIZE		32768

#define ACTIVITYFIELDS		14
#define BOOKMARKFOLDERFIELDS	10
#define BOOKMARKFIELDS		11
#define CALLACTIONFIELDS	9
#define CALLFIELDS		17
#define DOMAINFIELDS		9
#define DOMAINALIASFIELDS	10
#define CONTACTFIELDS		39
#define CONTACTFOLDERFIELDS	10
#define CONTACTSESSIONFIELDS	12
#define EVENTCLOSINGFIELDS	9
#define EVENTFIELDS		22
#define EVENTTYPEFIELDS		9
#define FILEFIELDS		15
#define FORUMFIELDS		13
#define FORUMGROUPFIELDS	10
#define FORUMPOSTFIELDS		14
#define GROUPFIELDS		11
#define INVENTORYFIELDS		16
#define INVOICEFIELDS		17
#define INVOICEITEMFIELDS	16
#define MAILACCTFIELDS		29
#define MAILFILTERFIELDS	15
#define MAILFOLDERFIELDS	11
#define MAILHEADFIELDS		27
#define MESSAGEFIELDS		12
#define NOTEFIELDS		12
#define PROJECTFIELDS		14
#define QUERYFIELDS		10
#define TASKFIELDS		18
#define USERFIELDS		58
#define USERSESSIONFIELDS	12
#define ZONEFIELDS		9



#define HTTP_PORT		80
#define HTTPS_PORT		443
#define IMAP_PORT		143
#define IMAPS_PORT		993
#define POP3_PORT		110
#define POP3S_PORT		995
#define SMTP_PORT		25
#define SMTPS_PORT		465

#define HAVE_FBSQL
#define HAVE_MYSQL
#ifdef WIN32
/* #define HAVE_ODBC */
#endif
#define HAVE_PGSQL
#define HAVE_SQLITE2
#define HAVE_SQLITE3

#define HAVE_SSL

#ifdef WIN32
#ifndef BCC
	#define S_IFMT		_S_IFMT   
	#define S_IFDIR		_S_IFDIR  
	#define S_IFCHR		_S_IFCHR  
	#define S_IFIFO		_S_IFIFO  
	#define S_IFREG		_S_IFREG  
	#define S_IREAD		_S_IREAD  
	#define S_IWRITE	_S_IWRITE 
	#define S_IEXEC		_S_IEXEC 
	#define S_IFBLK		0000000
	#define S_ISDIR(x)	(x & _S_IFDIR)
#endif
#define S_IFLNK		0000000
#define S_IFSOCK	0000000
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001
#define S_ISLNK(x)	0
#define ATTRIBUTES	(_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_SUBDIR)
#define	MAXNAMLEN	255
#define PATH_MAX	512 /* from limits.h, but _POSIX_ breaks stuff */
#define DllExport __declspec(dllexport)
#define OS_WIN9X 1
#define OS_WINNT 2
#define OS_WIN2K 3
#else
#define DllExport
#define O_BINARY 0
#endif

#ifndef socklen_t
#define socklen_t int
#endif

#define BUG() logerror(NULL, __FILE__, __LINE__, "[%s][%d]", __FILE__, __LINE__)
#define logruntime() { \
	struct timeval ttime; \
	struct timezone tzone; \
	num_t totaltime; \
	char namebuf[128]; \
	char linebuf[128]; \
	gettimeofday(&ttime, &tzone); \
	totaltime=(float)(((ttime.tv_sec-conn->dat->runtime.tv_sec)*1000000)+(ttime.tv_usec-conn->dat->runtime.tv_usec))/1000000; \
	snprintf(namebuf, 127, "_debug:%.6f", totaltime); \
	snprintf(linebuf, 127, "%s:%d", __FILE__, __LINE__); \
	nsp_setstr(conn->N, &conn->N->g, namebuf, linebuf, strlen(linebuf)); \
}
