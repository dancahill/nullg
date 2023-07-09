/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "xmlsync.h"

#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

static struct sockaddr_in server;
static struct hostent *hp;
static short int cli_socket;
// TCP INPUT BUFFER
static short int recvbufsize;
static short int recvbufoffset;
static char recvbuf[4096];

#ifdef WIN32
static WSADATA wsaData;
#endif

static void striprn(char *string)
{
	int i;

	while (1) {
		i=strlen(string);
		if (i<1) break;
		if (string[i-1]=='\r') { string[i-1]='\0'; continue; }
		if (string[i-1]=='\n') { string[i-1]='\0'; continue; }
		break;
	};
}
/*
static void logdata(const char *format, ...)
{
	FILE *fp;
	va_list ap;
	char buffer[1024];
	char *file="xmlsync.log";

	fp=fopen(file, "a");
	if (fp!=NULL) {
		memset(buffer, 0, sizeof(buffer));
		va_start(ap, format);
		vsnprintf(buffer, sizeof(buffer)-1, format, ap);
		va_end(ap);
		striprn(buffer);
		fprintf(fp, "%s\r\n", buffer);
		fclose(fp);
	}
}
*/
static int wmprintf(const char *format, ...)
{
	char buffer[8192];
	va_list ap;
	int len;

	memset(buffer, 0, sizeof(buffer));
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	send(cli_socket, buffer, strlen(buffer), 0);
	len=strlen(buffer);
//striprn(buffer);
//logdata(">> %s", buffer);
	return len;
}
/*
static int wmfgets(char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char temp[2];
	int n=0;

	memset(temp, 0, sizeof(temp));
	while (n<max) {
		if (recv(fd, temp, 1, 0)<1) {
		}
		n++;
		if (temp[0]==13) continue;
		*buffer=temp[0];
		buffer++;
		if (temp[0]==10) break;
	}
	*buffer=0;
	striprn(pbuffer);
	return n;
}
*/
static int wmfgets(char *buffer, int max, int fd)
{
	char *pbuffer=buffer;
	char *obuffer;
	short int lf=0;
	short int n=0;
	short int rc;
	short int x;
	short int retries=10;

retry:
	if (!recvbufsize) {
		x=sizeof(recvbuf)-recvbufoffset-recvbufsize-2;
		obuffer=recvbuf+recvbufoffset+recvbufsize;
		if ((rc=recv(fd, obuffer, x, 0))<0) {
			return -1;
		} else if (rc<1) {
			msleep(1);
			if (retries-->0) goto retry;
		}
		recvbufsize+=rc;
	}
	obuffer=recvbuf+recvbufoffset;
	while ((n<max)&&(recvbufsize>0)) {
		recvbufoffset++;
		recvbufsize--;
		n++;
		if (*obuffer=='\n') lf=1;
		*pbuffer++=*obuffer++;
		if ((lf)||(*obuffer=='\0')) break;
	}
	*pbuffer='\0';
	if (n>max-1) return n;
	if (!lf) {
		if (recvbufsize>0) {
			memmove(recvbuf, recvbuf+recvbufoffset, recvbufsize);
			memset(recvbuf+recvbufsize, 0, sizeof(recvbuf)-recvbufsize);
			recvbufoffset=0;
		} else {
			memset(recvbuf, 0, sizeof(recvbuf));
			recvbufoffset=0;
			recvbufsize=0;
		}
		if (retries>0) goto retry;
	}
	striprn(buffer);
//logdata("<< %s", buffer);
	return n;
}

static int xmlrpc_connect(char *host, short int port)
{
#ifdef WIN32
	if (WSAStartup(0x101, &wsaData)) {
		printf("Winsock init error\r\n");
		return -1;
	}
#endif
	if ((hp=gethostbyname(host))==NULL) {
		printf("Host lookup error for %s\r\n", host);
		return -1;
	}
	memset((char *)&server, 0, sizeof(server));
	memmove((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family=hp->h_addrtype;
	server.sin_port=htons(port);
	if ((cli_socket=socket(AF_INET, SOCK_STREAM, 0))<0) return -1;
	if (connect(cli_socket, (struct sockaddr *)&server, sizeof(server))<0) {
		printf("Connect error for %s:%d\r\n", host, port);
		return -1;
	}
	recvbufsize=0;
	recvbufoffset=0;
	memset(recvbuf, 0, sizeof(recvbuf));
	return 0;
}

char *xmlrpc_addmember(char *name, char *type, const char *format, ...)
{
	char value[1024];
	static char outbuf[2048];
	char timebuf[20];
	va_list ap;
	time_t t;
	char *ptemp;

	memset(value, 0, sizeof(value));
	memset(outbuf, 0, sizeof(outbuf));
	va_start(ap, format);
	vsnprintf(value, sizeof(value)-1, format, ap);
	va_end(ap);
	if (strncasecmp(type, "dateTime", 8)==0) type="dateTime.iso8601";
	if (strcasecmp("dateTime.iso8601", type)==0) {
		t=atoi(value);
		strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&t));
		ptemp=timebuf;
	} else {
		ptemp=value;
	}
	snprintf(outbuf, sizeof(outbuf)-1, "<member><name>%s</name><value><%s>%s</%s></value></member>\r\n", name, type, ptemp, type);
	return outbuf;
}

int xmlrpc_contact_setlval(REC_CONTACT *contact, char *name, char *val)
{
	if (!strlen(name)) return 0;
	if (strcasecmp(name, "contactid")==0) {
		contact->contactid=atoi(val);
	} else if (strcasecmp(name, "obj_ctime")==0) {
		contact->obj_ctime=atoi(val);
	} else if (strcasecmp(name, "obj_mtime")==0) {
		contact->obj_mtime=atoi(val);
	} else if (strcasecmp(name, "obj_uid")==0) {
		contact->obj_uid=atoi(val);
	} else if (strcasecmp(name, "obj_gid")==0) {
		contact->obj_gid=atoi(val);
	} else if (strcasecmp(name, "obj_did")==0) {
		contact->obj_did=atoi(val);
	} else if (strcasecmp(name, "obj_gperm")==0) {
		contact->obj_gperm=atoi(val);
	} else if (strcasecmp(name, "obj_operm")==0) {
		contact->obj_operm=atoi(val);
	} else if (strcasecmp(name, "loginip")==0) {
		strncpy(contact->loginip, val, sizeof(contact->loginip)-1);
	} else if (strcasecmp(name, "logintime")==0) {
		contact->logintime=atoi(val);
	} else if (strcasecmp(name, "logintoken")==0) {
		strncpy(contact->logintoken, val, sizeof(contact->logintoken)-1);
	} else if (strcasecmp(name, "username")==0) {
		strncpy(contact->username, val, sizeof(contact->username)-1);
	} else if (strcasecmp(name, "password")==0) {
		strncpy(contact->password, val, sizeof(contact->password)-1);
	} else if (strcasecmp(name, "enabled")==0) {
		contact->enabled=atoi(val);
	} else if (strcasecmp(name, "geozone")==0) {
		contact->geozone=atoi(val);
	} else if (strcasecmp(name, "timezone")==0) {
		contact->timezone=atoi(val);
	} else if (strcasecmp(name, "surname")==0) {
		strncpy(contact->surname, val, sizeof(contact->surname)-1);
	} else if (strcasecmp(name, "givenname")==0) {
		strncpy(contact->givenname, val, sizeof(contact->givenname)-1);
	} else if (strcasecmp(name, "salutation")==0) {
		strncpy(contact->salutation, val, sizeof(contact->salutation)-1);
	} else if (strcasecmp(name, "contacttype")==0) {
		strncpy(contact->contacttype, val, sizeof(contact->contacttype)-1);
	} else if (strcasecmp(name, "referredby")==0) {
		strncpy(contact->referredby, val, sizeof(contact->referredby)-1);
	} else if (strcasecmp(name, "altcontact")==0) {
		strncpy(contact->altcontact, val, sizeof(contact->altcontact)-1);
	} else if (strcasecmp(name, "prefbilling")==0) {
		strncpy(contact->prefbilling, val, sizeof(contact->prefbilling)-1);
	} else if (strcasecmp(name, "email")==0) {
		strncpy(contact->email, val, sizeof(contact->email)-1);
	} else if (strcasecmp(name, "homenumber")==0) {
		strncpy(contact->homenumber, val, sizeof(contact->homenumber)-1);
	} else if (strcasecmp(name, "worknumber")==0) {
		strncpy(contact->worknumber, val, sizeof(contact->worknumber)-1);
	} else if (strcasecmp(name, "faxnumber")==0) {
		strncpy(contact->faxnumber, val, sizeof(contact->faxnumber)-1);
	} else if (strcasecmp(name, "mobilenumber")==0) {
		strncpy(contact->mobilenumber, val, sizeof(contact->mobilenumber)-1);
	} else if (strcasecmp(name, "jobtitle")==0) {
		strncpy(contact->jobtitle, val, sizeof(contact->jobtitle)-1);
	} else if (strcasecmp(name, "organization")==0) {
		strncpy(contact->organization, val, sizeof(contact->organization)-1);
	} else if (strcasecmp(name, "homeaddress")==0) {
		strncpy(contact->homeaddress, val, sizeof(contact->homeaddress)-1);
	} else if (strcasecmp(name, "homelocality")==0) {
		strncpy(contact->homelocality, val, sizeof(contact->homelocality)-1);
	} else if (strcasecmp(name, "homeregion")==0) {
		strncpy(contact->homeregion, val, sizeof(contact->homeregion)-1);
	} else if (strcasecmp(name, "homecountry")==0) {
		strncpy(contact->homecountry, val, sizeof(contact->homecountry)-1);
	} else if (strcasecmp(name, "homepostalcode")==0) {
		strncpy(contact->homepostalcode, val, sizeof(contact->homepostalcode)-1);
	} else if (strcasecmp(name, "workaddress")==0) {
		strncpy(contact->workaddress, val, sizeof(contact->workaddress)-1);
	} else if (strcasecmp(name, "worklocality")==0) {
		strncpy(contact->worklocality, val, sizeof(contact->worklocality)-1);
	} else if (strcasecmp(name, "workregion")==0) {
		strncpy(contact->workregion, val, sizeof(contact->workregion)-1);
	} else if (strcasecmp(name, "workcountry")==0) {
		strncpy(contact->workcountry, val, sizeof(contact->workcountry)-1);
	} else if (strcasecmp(name, "workpostalcode")==0) {
		strncpy(contact->workpostalcode, val, sizeof(contact->workpostalcode)-1);
	}
	return 0;
}

DllExport int xmlrpc_contact_listopen(char *host, short int port, char *uri, char *username, char *password, RECLIST_CONTACT *conlist)
{
	char *ptemp;
	char inbuffer[1024];
	char outbuffer[1024];
	char name[64];
	char val[1024];
	unsigned int tuple=0;
	unsigned int rowsalloc=0;

	if (xmlrpc_connect(host, port)<0) return -1;
	snprintf(outbuffer, sizeof(outbuffer)-1,
		"<?xml version=\"1.0\"?>\r\n<methodCall>\r\n<methodName>contacts.list</methodName>\r\n<params>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n"
		"</params>\r\n</methodCall>\r\n", username, password
	);
	wmprintf("POST %s HTTP/1.0\r\n", uri);
	wmprintf("Host: %s\r\n", host);
	wmprintf("Connection: close\r\n");
	wmprintf("Content-Length: %d\r\n", strlen(outbuffer));
	wmprintf("\r\n");
	wmprintf("%s", outbuffer);
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
	}
	rowsalloc=50;
	conlist->contact=(REC_CONTACT **)calloc(rowsalloc, sizeof(REC_CONTACT *));
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
		ptemp=inbuffer;
		if (strncasecmp(ptemp, "<struct>", 8)==0) {
			conlist->contact[tuple]=calloc(1, sizeof(REC_CONTACT));
			for (;;) {
				if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
				if (!strlen(inbuffer)) continue;
				ptemp=inbuffer;
				if (strncasecmp(ptemp, "<member>", 8)==0) {
					memset(name, 0, sizeof(name));
					memset(val, 0, sizeof(val));
					ptemp+=8;
					if (strncasecmp(ptemp, "<name>", 6)==0) {
						ptemp+=6;
						while (*ptemp!='<') name[strlen(name)]=*ptemp++;
					}
					if (strncasecmp(ptemp, "</name>", 7)==0) ptemp+=7;
					if (strncasecmp(ptemp, "<value>", 7)==0) ptemp+=7;
					if (strncasecmp(ptemp, "<", 1)!=0) ptemp+=1;
					while (*ptemp!='<') ptemp++;
					while (*ptemp!='>') ptemp++;
					if (*ptemp=='>') ptemp++;
					while (*ptemp!='<') val[strlen(val)]=*ptemp++;
					xmlrpc_contact_setlval(conlist->contact[tuple], name, val);
				}
				if (strncasecmp(ptemp, "</struct>", 9)==0) {
					tuple++;
					break;
				}
			}
			if (tuple+2>rowsalloc) {
				rowsalloc+=5;
				conlist->contact=(REC_CONTACT **)realloc(conlist->contact, rowsalloc*sizeof(REC_CONTACT *));
			}
		}
	}
	conlist->records=tuple;
	closesocket(cli_socket);
	return 0;
}

DllExport REC_CONTACT *xmlrpc_contact_listread(RECLIST_CONTACT *conlist)
{
	return NULL;
}

DllExport int xmlrpc_contact_listclose(RECLIST_CONTACT *conlist)
{
	unsigned int tuple;

	if (conlist->contact!=NULL) {
		for (tuple=0;tuple<conlist->records;tuple++) {
			free(conlist->contact[tuple]);
		}
		free(conlist->contact);
		conlist->contact=NULL;
	}
	conlist->records=0;
	conlist->last_record=0;
	return 0;
}

DllExport int xmlrpc_contact_read(char *host, short int port, char *uri, char *username, char *password, int recordid, REC_CONTACT *contact)
{
	char *ptemp;
	char inbuffer[1024];
	char outbuffer[1024];
	char name[64];
	char val[1024];

	memset(contact, 0, sizeof(REC_CONTACT));
	if (xmlrpc_connect(host, port)<0) return -1;
	snprintf(outbuffer, sizeof(outbuffer)-1,
		"<?xml version=\"1.0\"?>\r\n<methodCall>\r\n<methodName>contacts.read</methodName>\r\n<params>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n"
		"<param>\r\n<value><int>%d</int></value>\r\n</param>\r\n"
		"</params>\r\n</methodCall>\r\n", username, password, recordid
	);
	wmprintf("POST %s HTTP/1.0\r\n", uri);
	wmprintf("Host: %s\r\n", host);
	wmprintf("Connection: close\r\n");
	wmprintf("Content-Length: %d\r\n", strlen(outbuffer));
	wmprintf("\r\n");
	wmprintf("%s", outbuffer);
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
	}
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
		ptemp=inbuffer;
		if (strncasecmp(ptemp, "<member>", 8)==0) {
			memset(name, 0, sizeof(name));
			memset(val, 0, sizeof(val));
			ptemp+=8;
			if (strncasecmp(ptemp, "<name>", 6)==0) {
				ptemp+=6;
				while (*ptemp!='<') name[strlen(name)]=*ptemp++;
			}
			if (strncasecmp(ptemp, "</name>", 7)==0) ptemp+=7;
			if (strncasecmp(ptemp, "<value>", 7)==0) ptemp+=7;
			if (strncasecmp(ptemp, "<", 1)!=0) ptemp+=1;
			while (*ptemp!='<') ptemp++;
			while (*ptemp!='>') ptemp++;
			if (*ptemp=='>') ptemp++;
			while (*ptemp!='<') val[strlen(val)]=*ptemp++;
			xmlrpc_contact_setlval(contact, name, val);
		}
	}
	closesocket(cli_socket);
	return 0;
}

DllExport int xmlrpc_contact_write(char *host, short int port, char *uri, char *username, char *password, int recordid, REC_CONTACT *contact)
{
	char *ptemp;
	char inbuffer[1024];
	char outbuffer[16384];
	char name[64];
	char val[1024];

	if (xmlrpc_connect(host, port)<0) return -1;
	snprintf(outbuffer, sizeof(outbuffer)-1,
		"<?xml version=\"1.0\"?>\r\n<methodCall>\r\n<methodName>contacts.write</methodName>\r\n<params>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n"
		"<param>\r\n<value><string>%s</string></value>\r\n</param>\r\n", username, password
	);
	strncat(outbuffer, "<param>\r\n<value><struct>\r\n", sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("contactid",      "int",      "%d", contact->contactid);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_ctime",      "dateTime", "%d", contact->obj_ctime);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_mtime",      "dateTime", "%d", contact->obj_mtime);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_uid",        "int",      "%d", contact->obj_uid);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_gid",        "int",      "%d", contact->obj_gid);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_did",        "int",      "%d", contact->obj_did);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_gperm",      "int",      "%d", contact->obj_gperm);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("obj_operm",      "int",      "%d", contact->obj_operm);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("loginip",        "string",   "%s", contact->loginip);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("logintime",      "dateTime", "%d", contact->logintime);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("logintoken",     "string",   "%s", contact->logintoken);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("username",       "string",   "%s", contact->username);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("password",       "string",   "%s", contact->password);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("enabled",        "int",      "%d", contact->enabled);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("geozone",        "int",      "%d", contact->geozone);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("timezone",       "int",      "%d", contact->timezone);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("surname",        "string",   "%s", contact->surname);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("givenname",      "string",   "%s", contact->givenname);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("salutation",     "string",   "%s", contact->salutation);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("contacttype",    "string",   "%s", contact->contacttype);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("referredby",     "string",   "%s", contact->referredby);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("altcontact",     "string",   "%s", contact->altcontact);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("prefbilling",    "string",   "%s", contact->prefbilling);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("email",          "string",   "%s", contact->email);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homenumber",     "string",   "%s", contact->homenumber);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("worknumber",     "string",   "%s", contact->worknumber);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("faxnumber",      "string",   "%s", contact->faxnumber);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("mobilenumber",   "string",   "%s", contact->mobilenumber);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("jobtitle",       "string",   "%s", contact->jobtitle);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("organization",   "string",   "%s", contact->organization);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homeaddress",    "string",   "%s", contact->homeaddress);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homelocality",   "string",   "%s", contact->homelocality);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homeregion",     "string",   "%s", contact->homeregion);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homecountry",    "string",   "%s", contact->homecountry);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("homepostalcode", "string",   "%s", contact->homepostalcode);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("workaddress",    "string",   "%s", contact->workaddress);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("worklocality",   "string",   "%s", contact->worklocality);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("workregion",     "string",   "%s", contact->workregion);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("workcountry",    "string",   "%s", contact->workcountry);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	ptemp=xmlrpc_addmember("workpostalcode", "string",   "%s", contact->workpostalcode);
	strncat(outbuffer, ptemp, sizeof(outbuffer)-strlen(outbuffer)-1);
	strncat(outbuffer, "</struct></value>\r\n", sizeof(outbuffer)-strlen(outbuffer)-1);
	strncat(outbuffer, "</param>\r\n</params>\r\n</methodCall>\r\n", sizeof(outbuffer)-strlen(outbuffer)-1);
	wmprintf("POST %s HTTP/1.0\r\n", uri);
	wmprintf("Host: %s\r\n", host);
	wmprintf("Connection: close\r\n");
	wmprintf("Content-Length: %d\r\n", strlen(outbuffer));
	wmprintf("\r\n");
	wmprintf("%s", outbuffer);
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
	}
	for (;;) {
		if (wmfgets(inbuffer, sizeof(inbuffer)-1, cli_socket)<0) break;
		if (!strlen(inbuffer)) break;
		ptemp=inbuffer;
		if (strncasecmp(ptemp, "<member>", 8)==0) {
			memset(name, 0, sizeof(name));
			memset(val, 0, sizeof(val));
			ptemp+=8;
			if (strncasecmp(ptemp, "<name>", 6)==0) {
				ptemp+=6;
				while (*ptemp!='<') name[strlen(name)]=*ptemp++;
			}
			if (strncasecmp(ptemp, "</name>", 7)==0) ptemp+=7;
			if (strncasecmp(ptemp, "<value>", 7)==0) ptemp+=7;
			if (strncasecmp(ptemp, "<", 1)!=0) ptemp+=1;
			while (*ptemp!='<') ptemp++;
			while (*ptemp!='>') ptemp++;
			if (*ptemp=='>') ptemp++;
			while (*ptemp!='<') val[strlen(val)]=*ptemp++;
			xmlrpc_contact_setlval(contact, name, val);
		}
	}
	closesocket(cli_socket);
	return 0;
}

#ifdef WIN32
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpReserved)
{
	switch (dwFunction) {
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
#endif
