/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#define DEFAULT_COLOUR_TH		"#0000A0" // "#00A5D0"
#define DEFAULT_COLOUR_THTEXT		"#FFFFFF" // "#000000"
#define DEFAULT_COLOUR_THLINK		"#E0E0FF" // "#0000FF"
#define DEFAULT_COLOUR_LINKS		"#0000FF" // "#0000FF"

#define SERVER_NAME	"NullLogic Groupware 1.2.1"

#define MAX_POSTSIZE	33554432 /* arbitrary 32 MB limit for POST request sizes */
#define MAX_REPLYSIZE	16384 /* arbitrary 16 KB limit for reply buffering */

#define MAX_MOD_MENUITEMS	25
#define MAX_MOD_FUNCTIONS	50
/* SQL limits */
#define MAX_TUPLE_SIZE	16384
#define MAX_FIELD_SIZE	12288

#define IMAP_PORT	143
#define POP3_PORT	110
#define SMTP_PORT	25

#define AUTH_ADMIN	1
#define AUTH_BOOKMARKS	2
#define AUTH_CALENDAR	3
#define AUTH_CALLS	4
#define AUTH_CONTACTS	5
#define AUTH_FILES	6
#define AUTH_FORUMS	7
#define AUTH_MESSAGES	8
#define AUTH_ORDERS	9
#define AUTH_PROFILE	10
#define AUTH_QUERIES	11
#define AUTH_WEBMAIL	12

#define A_READ		1
#define A_MODIFY	2
#define A_INSERT	4
#define A_DELETE	8
#define A_ADMIN		16

#define DB_DBINFO		1
#define DB_BOOKMARKFOLDERS	2
#define DB_BOOKMARKS		3
#define DB_CALLS		4
#define DB_CONTACTS		5
#define DB_EVENTS		6
#define DB_FILES		7
#define DB_FORUMS		8
#define DB_FORUMGROUPS		9
#define DB_FORUMPOSTS		10
#define DB_GROUPS		11
#define DB_MAILACCOUNTS		12
#define DB_MAILHEADERS		13
#define DB_MESSAGES		14
#define DB_NOTES		15
#define DB_ORDERS		16
#define DB_ORDERITEMS		17
#define DB_PRODUCTS		18
#define DB_PROFILE		19
#define DB_QUERIES		20
#define DB_TASKS		21
#define DB_USERS		22
#define DB_ZONES		23

#define MENU_ADMIN	1
#define MENU_BOOKMARKS	2
#define MENU_CALENDAR	3
#define MENU_CALLS	4
#define MENU_CONTACTS	5
#define MENU_FILES	6
#define MENU_FORUMS	7
#define MENU_MAIN	8
#define MENU_MESSAGES	9
#define MENU_NOTES	10
#define MENU_ORDERS	11
#define MENU_PROFILE	12
#define MENU_SEARCHES	13
#define MENU_TASKS	14
#define MENU_WEBMAIL	15
#define MENU_XMLRPC	16

#define ACTIVITYFIELDS		13
#define BOOKMARKFOLDERFIELDS	9
#define BOOKMARKFIELDS		10
#define CALLACTIONFIELDS	8
#define CALLFIELDS		16
#define CONTACTFIELDS		39
#define EVENTCLOSINGFIELDS	8
#define EVENTFIELDS		20
#define EVENTTYPEFIELDS		8
#define FILEFIELDS		14
#define FORUMFIELDS		12
#define FORUMGROUPFIELDS	9
#define FORUMPOSTFIELDS		13
#define GROUPFIELDS		11
#define MAILACCTFIELDS		23
#define MAILHEADFIELDS		22
#define MESSAGEFIELDS		11
#define NOTEFIELDS		11
#define ORDERFIELDS		15
#define ORDERITEMFIELDS		15
#define PRODUCTFIELDS		15
#define QUERYFIELDS		9
#define TASKFIELDS		15
#define USERFIELDS		55
#define ZONEFIELDS		8

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

	#define BUG() logerror(NULL, __FILE__, __LINE__, "[%s][%d]", __FILE__, __LINE__)
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
