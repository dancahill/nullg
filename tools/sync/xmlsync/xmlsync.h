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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define DllExport __declspec(dllexport)
#else
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define closesocket close
#define DllExport
#endif

/* typedefs */
typedef struct {
	char host[255];
	unsigned short int port;
	char uri[255];
	char username[64];
	char password[64];
} CONFIG;
typedef struct {
	int contactid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char loginip[21];
	time_t logintime;
	char logintoken[51];
	char username[51];
	char password[51];
	int enabled;
	short int geozone;
	short int timezone;
	char surname[51];
	char givenname[51];
	char salutation[51];
	char contacttype[51];
	char referredby[51];
	char altcontact[51];
	char prefbilling[51];
	char email[51];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char mobilenumber[26];
	char jobtitle[51];
	char organization[51];
	char homeaddress[51];
	char homelocality[51];
	char homeregion[51];
	char homecountry[51];
	char homepostalcode[11];
	char workaddress[51];
	char worklocality[51];
	char workregion[51];
	char workcountry[51];
	char workpostalcode[11];
} REC_CONTACT;
typedef struct {
	unsigned int records;
	unsigned int last_record;
	REC_CONTACT **contact;
} RECLIST_CONTACT;

#ifdef __cplusplus
extern "C" {
#endif
	DllExport int xmlrpc_contact_listopen(char *host, short int port, char *uri, char *username, char *password, RECLIST_CONTACT *conlist);
	DllExport REC_CONTACT *xmlrpc_contact_listread(RECLIST_CONTACT *conlist);
	DllExport int xmlrpc_contact_listclose(RECLIST_CONTACT *conlist);
	DllExport int xmlrpc_contact_read(char *host, short int port, char *uri, char *username, char *password, int recordid, REC_CONTACT *contact);
	DllExport int xmlrpc_contact_write(char *host, short int port, char *uri, char *username, char *password, int recordid, REC_CONTACT *contact);
#ifdef __cplusplus
}
#endif
