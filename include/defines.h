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
#define SERVER_NAME	"NullLogic Groupware"
#define SERVER_VERSION	"1.2.3"
#define SERVER_BASENAME	"groupware"

#ifdef WIN32
#define DEFAULT_BASE_DIR "C:\\Program Files\\NullLogic Groupware"
#else
#define DEFAULT_BASE_DIR "/usr/local/groupware"
#endif
#define DEFAULT_COLOUR_TOPMENU		"#C0C0C0" // "#C0C0C0"
#define DEFAULT_COLOUR_EDITFORM		"#F0F0F0" // "#F0F0F0"
#define DEFAULT_COLOUR_FIELDNAME	"#E0E0E0" // "#E0E0E0"
#define DEFAULT_COLOUR_FIELDNAMETEXT	"#000000" // "#000000"
#define DEFAULT_COLOUR_FIELDVAL		"#F0F0F0" // "#F0F0F0"
#define DEFAULT_COLOUR_FIELDVALTEXT	"#000000" // "#000000"
#define DEFAULT_COLOUR_TABLETRIM	"#000000" // "#000000"
#define DEFAULT_COLOUR_TH		"#0000A0" // "#00A5D0"
#define DEFAULT_COLOUR_THTEXT		"#FFFFFF" // "#000000"
#define DEFAULT_COLOUR_THLINK		"#E0E0FF" // "#0000FF"
#define DEFAULT_COLOUR_LINKS		"#0000FF" // "#0000FF"

#define MAX_POSTSIZE	33554432 /* arbitrary 32 MB limit for POST request sizes */
#define MAX_REPLYSIZE	16384 /* arbitrary 16 KB limit for reply buffering */

#define MAX_MOD_MENUITEMS	25
#define MAX_MOD_FUNCTIONS	50

#define MAX_AUTH_FIELDS		16
#define MAX_PREF_FIELDS		16

/* SQL limits */
#define MAX_TUPLE_SIZE	16384
#define MAX_FIELD_SIZE	12288

#define IMAP_PORT	143
#define POP3_PORT	110
#define SMTP_PORT	25

#define A_READ		1
#define A_MODIFY	2
#define A_INSERT	4
#define A_DELETE	8
#define A_ADMIN		16

#define MENU_ADMIN	1
#define MENU_ACCOUNTING	2
#define MENU_BOOKMARKS	3
#define MENU_CALENDAR	4
#define MENU_CALLS	5
#define MENU_CONTACTS	6
#define MENU_FILES	7
#define MENU_FORUMS	8
#define MENU_MAIN	9
#define MENU_MESSAGES	10
#define MENU_NOTES	11
#define MENU_ORDERS	12
#define MENU_PROFILE	13
#define MENU_SEARCHES	14
#define MENU_TASKS	15
#define MENU_WEBMAIL	16
#define MENU_XMLRPC	17

#define ACTIVITYFIELDS		14
#define BOOKMARKFOLDERFIELDS	10
#define BOOKMARKFIELDS		11
#define CALLACTIONFIELDS	9
#define CALLFIELDS		17
#define CONTACTFIELDS		40
#define EVENTCLOSINGFIELDS	9
#define EVENTFIELDS		21
#define EVENTTYPEFIELDS		9
#define FILEFIELDS		15
#define FORUMFIELDS		13
#define FORUMGROUPFIELDS	10
#define FORUMPOSTFIELDS		14
#define GROUPFIELDS		12
#define MAILACCTFIELDS		24
#define MAILHEADFIELDS		23
#define MAILFOLDERFIELDS	11
#define MESSAGEFIELDS		12
#define NOTEFIELDS		12
#define ORDERFIELDS		16
#define ORDERITEMFIELDS		16
#define PRODUCTFIELDS		16
#define QUERYFIELDS		10
#define TASKFIELDS		16
#define USERFIELDS		56
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

#ifdef WIN32
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

/* MD5 */
#define MD5_SIZE 16
