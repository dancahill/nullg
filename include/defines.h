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
#define SERVER_NAME	"NullLogic Groupware"
#define SERVER_BASENAME	"groupware"
#define SERVICE_NAME	"nullgroupware"

#ifdef WIN32
#define DEFAULT_BASE_DIR "C:\\Program Files\\NullLogic Groupware"
#else
#define DEFAULT_BASE_DIR "/usr/local/nullgroupware"
#endif
#define DEFAULT_SERVER_USERNAME		"nullgw"

#define MAX_REPLYSIZE		16384 /* arbitrary 16 KB limit for reply buffering */

#define MAX_MOD_MENUITEMS	25
#define MAX_MOD_FUNCTIONS	50

/* SQL limits */
#define MAX_TUPLE_SIZE		16384
#define MAX_FIELD_SIZE		12288

#define HTTP_PORT		4110
#define HTTPS_PORT		4112
#define IMAP_PORT		143
#define POP3_PORT		110
#define SMTP_PORT		25
#define IMAPS_PORT		993
#define POP3S_PORT		995
#define SMTPS_PORT		465

#define ACTIVITYFIELDS		14
#define BOOKMARKFOLDERFIELDS	10
#define BOOKMARKFIELDS		11
#define CALLACTIONFIELDS	9
#define CALLFIELDS		17
#define DOMAINFIELDS		9
#define DOMAINALIASFIELDS	10
#define CONTACTFIELDS		42
#define EVENTCLOSINGFIELDS	9
#define EVENTFIELDS		22
#define EVENTTYPEFIELDS		9
#define FILEFIELDS		15
#define FORUMFIELDS		13
#define FORUMGROUPFIELDS	10
#define FORUMPOSTFIELDS		14
#define GROUPFIELDS		12
#define MAILACCTFIELDS		27
#define MAILFILTERFIELDS	15
#define MAILFOLDERFIELDS	11
#define MAILHEADFIELDS		27
#define MESSAGEFIELDS		12
#define NOTEFIELDS		12
#define ORDERFIELDS		17
#define ORDERITEMFIELDS		16
#define PRODUCTFIELDS		16
#define PROJECTFIELDS		14
#define QUERYFIELDS		10
#define TASKFIELDS		18
#define USERFIELDS		61
#define ZONEFIELDS		9

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
#endif

#ifdef DEBUG
	#define BUG() logerror(NULL, __FILE__, __LINE__, "[%s][%d]", __FILE__, __LINE__)
	#define DEBUG_IN(SID, a) logdebug(SID, __FILE__, __LINE__, "Entering %s.", a)
	#define DEBUG_OUT(SID, a) logdebug(SID, __FILE__, __LINE__, "Exiting  %s.", a)
#else
	#define DEBUG_IN(SID, a)
	#define DEBUG_OUT(SID, a)
#endif

#define HAVE_MYSQL
#ifdef WIN32
#define HAVE_ODBC
#endif
#define HAVE_PGSQL
#define HAVE_SQLITE

#define HAVE_SSL

#ifdef WIN32
#define DllExport __declspec(dllexport)
#define OS_WIN9X 1
#define OS_WINNT 2
#define OS_WIN2K 3
#else
#define DllExport
#define O_BINARY 0
#endif

#define SVC_HTTPD 1
#define SVC_POP3D 2
#define SVC_SMTPD 3
